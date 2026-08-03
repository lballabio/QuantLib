"""Phase-9 tests: CDS bootstrap, Asian analytics, FD Hull–White swaption."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_at_least_phase9():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 10)


def test_piecewise_hazard_bootstrap_fair_spreads():
    # Mirrors test-suite/defaultprobabilitycurves.cpp::testBootstrapFromSpread
    # (HazardRate / BackwardFlat).
    today = ql.Date(15, ql.Month.May, 2007)
    ql.set_evaluation_date(today)
    settings = ql.Settings.instance()
    settings.include_todays_cash_flows = True

    calendar = ql.TARGET()
    discount = ql.FlatForward(today, 0.06, ql.Actual360())
    quotes = [0.005, 0.006, 0.007, 0.009]
    tenors = [1, 2, 3, 5]
    day_counter = ql.Thirty360(ql.Thirty360Convention.BondBasis)
    recovery = 0.4
    frequency = ql.Frequency.Quarterly
    convention = ql.BusinessDayConvention.Following
    rule = ql.DateGeneration.TwentiethIMM

    helpers = [
        ql.SpreadCdsHelper(
            quote,
            ql.Period(n, ql.TimeUnit.Years),
            1,
            calendar,
            frequency,
            convention,
            rule,
            day_counter,
            recovery,
            discount,
        )
        for quote, n in zip(quotes, tenors)
    ]
    curve = ql.PiecewiseHazardRateCurve(today, helpers, day_counter)

    # Survival should be strictly decreasing with maturity.
    sps = [
        curve.survival_probability(today + ql.Period(n, ql.TimeUnit.Years))
        for n in tenors
    ]
    assert all(0.0 < sp < 1.0 for sp in sps)
    assert sps == sorted(sps, reverse=True)

    settlement_days = 1
    for quote, n in zip(quotes, tenors):
        protection_start = today + settlement_days
        start = calendar.adjust(protection_start, convention)
        end = today + ql.Period(n, ql.TimeUnit.Years)
        schedule = ql.Schedule(
            start,
            end,
            ql.Period(frequency),
            calendar,
            convention,
            ql.BusinessDayConvention.Unadjusted,
            rule,
            False,
        )
        # CreditDefaultSwap ctor doesn't take protectionStart in our binding;
        # use the default schedule-based protection. Fair spread still recovers
        # the bootstrap quote under include_todays_cash_flows.
        cds = ql.CreditDefaultSwap(
            ql.ProtectionSide.Buyer,
            1.0,
            quote,
            schedule,
            convention,
            day_counter,
        )
        cds.set_pricing_engine(curve, recovery, discount)
        assert cds.fair_spread() == pytest.approx(quote, abs=1.0e-6)

    settings.include_todays_cash_flows = None


def test_continuous_geometric_asian_cached_npv():
    # Haug p.96-97 — asianoptions.cpp::testAnalyticContinuousGeometricAveragePrice
    today = ql.get_evaluation_date()
    dc = ql.Actual360()
    process = ql.BlackScholesMertonProcess(
        ql.make_quote_handle(80.0),
        ql.FlatForward(today, -0.03, dc),
        ql.FlatForward(today, 0.05, dc),
        ql.BlackConstantVol(today, ql.NullCalendar(), 0.20, dc),
    )
    payoff = ql.PlainVanillaPayoff(ql.OptionType.Put, 85.0)
    exercise = ql.EuropeanExercise(today + 90)
    option = ql.ContinuousAveragingAsianOption(
        ql.AverageType.Geometric, payoff, exercise
    )
    option.set_pricing_engine(process)
    assert option.NPV() == pytest.approx(4.6922, abs=1.0e-4)
    assert option.delta() != 0.0


def test_discrete_geometric_asian_cached_npv():
    # Clewlow/Strickland — asianoptions.cpp::testAnalyticDiscreteGeometricAveragePrice
    today = ql.get_evaluation_date()
    dc = ql.Actual360()
    process = ql.BlackScholesMertonProcess(
        ql.make_quote_handle(100.0),
        ql.FlatForward(today, 0.03, dc),
        ql.FlatForward(today, 0.06, dc),
        ql.BlackConstantVol(today, ql.NullCalendar(), 0.20, dc),
    )
    payoff = ql.PlainVanillaPayoff(ql.OptionType.Call, 100.0)
    exercise = ql.EuropeanExercise(today + 360)
    future_fixings = 10
    dt = round(360.0 / future_fixings)
    fixing_dates = [today + dt]
    for _ in range(1, future_fixings):
        fixing_dates.append(fixing_dates[-1] + dt)

    option = ql.DiscreteAveragingAsianOption(
        ql.AverageType.Geometric,
        1.0,
        0,
        fixing_dates,
        payoff,
        exercise,
    )
    option.set_pricing_engine(process)
    assert option.NPV() == pytest.approx(5.3425606635, abs=1.0e-10)


def test_fd_hullwhite_bermudan_near_tree():
    # Same ATM Bermudan case as phase-7 tree cache (~12.9).
    today = ql.Date(15, ql.Month.February, 2002)
    ql.set_evaluation_date(today)
    settlement = ql.Date(19, ql.Month.February, 2002)
    curve = ql.FlatForward(settlement, 0.04875825, ql.Actual365Fixed())
    index = ql.Euribor6M(curve)
    calendar = index.fixing_calendar()

    start = calendar.advance(settlement, 1, ql.TimeUnit.Years)
    maturity = calendar.advance(start, 5, ql.TimeUnit.Years)
    fixed_schedule = ql.Schedule(
        start,
        maturity,
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )
    float_schedule = ql.Schedule(
        start,
        maturity,
        ql.Period(ql.Frequency.Semiannual),
        calendar,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.DateGeneration.Forward,
        False,
    )
    probe = ql.VanillaSwap(
        ql.SwapType.Payer,
        1000.0,
        fixed_schedule,
        0.05,
        ql.Thirty360(ql.Thirty360Convention.BondBasis),
        float_schedule,
        index,
        0.0,
        index.day_counter(),
    )
    probe.set_pricing_engine(curve)
    atm = probe.fair_rate()
    swap = ql.VanillaSwap(
        ql.SwapType.Payer,
        1000.0,
        fixed_schedule,
        atm,
        ql.Thirty360(ql.Thirty360Convention.BondBasis),
        float_schedule,
        index,
        0.0,
        index.day_counter(),
    )
    exercise_dates = list(fixed_schedule.dates())[:-1]
    model = ql.HullWhite(curve, 0.048696, 0.0058904)

    tree = ql.Swaption(swap, ql.BermudanExercise(exercise_dates))
    tree.set_tree_pricing_engine(model, 50)

    fd = ql.Swaption(swap, ql.BermudanExercise(exercise_dates))
    fd.set_fd_hullwhite_pricing_engine(model, t_grid=100, x_grid=100)

    assert tree.NPV() == pytest.approx(12.9069, abs=0.05)
    # FD cache in bermudanswaption.cpp is ~12.8864 (at-par).
    assert fd.NPV() == pytest.approx(12.8864, abs=0.15)
    assert fd.NPV() == pytest.approx(tree.NPV(), abs=0.25)


def test_compat_phase9_aliases():
    import qlnb.compat as cql

    assert hasattr(cql.Swaption, "setFdHullWhitePricingEngine")
    assert callable(cql.SpreadCdsHelper)
    assert callable(cql.PiecewiseHazardRateCurve)
    assert hasattr(cql.ContinuousAveragingAsianOption, "setPricingEngine")
