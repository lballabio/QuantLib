"""Phase-7 tests: CDS, Bermudan tree swaption, FD mesher NumPy."""

from __future__ import annotations

import numpy as np
import pytest

import qlnb as ql


def test_version_is_at_least_phase7():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 8)


def test_cds_midpoint_cached_npv_and_fair_spread():
    # Mirrors test-suite/creditdefaultswap.cpp::testCachedValue (mid-point).
    today = ql.Date(9, ql.Month.June, 2006)
    ql.set_evaluation_date(today)
    calendar = ql.TARGET()

    probability = ql.FlatHazardRate(0, calendar, 0.01234, ql.Actual360())
    discount = ql.FlatForward(today, 0.06, ql.Actual360())

    issue = calendar.advance(today, -1, ql.TimeUnit.Years)
    maturity = calendar.advance(issue, 10, ql.TimeUnit.Years)
    convention = ql.BusinessDayConvention.ModifiedFollowing
    schedule = ql.Schedule(
        issue,
        maturity,
        ql.Period(ql.Frequency.Semiannual),
        calendar,
        convention,
        convention,
        ql.DateGeneration.Forward,
        False,
    )

    cds = ql.CreditDefaultSwap(
        ql.ProtectionSide.Seller,
        10_000.0,
        0.0120,
        schedule,
        convention,
        ql.Actual360(),
    )
    cds.set_pricing_engine(probability, 0.4, discount)

    assert cds.NPV() == pytest.approx(295.0153398, abs=1.0e-7)
    assert cds.fair_spread() == pytest.approx(0.007517539081, abs=1.0e-7)
    assert cds.side() == ql.ProtectionSide.Seller
    assert cds.notional() == 10_000.0
    assert cds.running_spread() == pytest.approx(0.0120)
    assert not cds.is_expired()
    assert cds.default_leg_NPV() != 0.0
    assert cds.coupon_leg_NPV() != 0.0


def test_flat_hazard_rate_survival_probability():
    # Lehman guide case cited in Examples/CDS/CDS.cpp: flat 3% hazard →
    # 1Y survival ≈ 97.04%, 2Y ≈ 94.18%.
    today = ql.Date(15, ql.Month.May, 2007)
    ql.set_evaluation_date(today)
    hazard = ql.FlatHazardRate(today, 0.03, ql.Actual365Fixed())
    sp1 = hazard.survival_probability(today + ql.Period(1, ql.TimeUnit.Years))
    sp2 = hazard.survival_probability(today + ql.Period(2, ql.TimeUnit.Years))
    assert sp1 == pytest.approx(0.9704, abs=5.0e-4)
    assert sp2 == pytest.approx(0.9418, abs=5.0e-4)
    assert hazard.hazard_rate(today + ql.Period(1, ql.TimeUnit.Years)) == pytest.approx(
        0.03, abs=1.0e-12
    )


def test_cds_optional_swig_comparison():
    QuantLib = pytest.importorskip("QuantLib")

    today = ql.Date(9, ql.Month.June, 2006)
    ql.set_evaluation_date(today)
    calendar = ql.TARGET()
    probability = ql.FlatHazardRate(0, calendar, 0.01234, ql.Actual360())
    discount = ql.FlatForward(today, 0.06, ql.Actual360())
    issue = calendar.advance(today, -1, ql.TimeUnit.Years)
    maturity = calendar.advance(issue, 10, ql.TimeUnit.Years)
    convention = ql.BusinessDayConvention.ModifiedFollowing
    schedule = ql.Schedule(
        issue,
        maturity,
        ql.Period(ql.Frequency.Semiannual),
        calendar,
        convention,
        convention,
        ql.DateGeneration.Forward,
        False,
    )
    cds = ql.CreditDefaultSwap(
        ql.ProtectionSide.Seller,
        10_000.0,
        0.0120,
        schedule,
        convention,
        ql.Actual360(),
    )
    cds.set_pricing_engine(probability, 0.4, discount)

    QuantLib.Settings.instance().evaluationDate = QuantLib.Date(9, QuantLib.June, 2006)
    cal_s = QuantLib.TARGET()
    hazard_quote = QuantLib.QuoteHandle(QuantLib.SimpleQuote(0.01234))
    hazard_s = QuantLib.FlatHazardRate(0, cal_s, hazard_quote, QuantLib.Actual360())
    disc_s = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(QuantLib.Date(9, QuantLib.June, 2006), 0.06, QuantLib.Actual360())
    )
    prob_s = QuantLib.DefaultProbabilityTermStructureHandle(hazard_s)
    issue_s = cal_s.advance(QuantLib.Date(9, QuantLib.June, 2006), -1, QuantLib.Years)
    mat_s = cal_s.advance(issue_s, 10, QuantLib.Years)
    sched_s = QuantLib.Schedule(
        issue_s,
        mat_s,
        QuantLib.Period(QuantLib.Semiannual),
        cal_s,
        QuantLib.ModifiedFollowing,
        QuantLib.ModifiedFollowing,
        QuantLib.DateGeneration.Forward,
        False,
    )
    cds_s = QuantLib.CreditDefaultSwap(
        QuantLib.Protection.Seller,
        10_000.0,
        0.0120,
        sched_s,
        QuantLib.ModifiedFollowing,
        QuantLib.Actual360(),
    )
    cds_s.setPricingEngine(QuantLib.MidPointCdsEngine(prob_s, 0.4, disc_s))
    assert cds.NPV() == pytest.approx(cds_s.NPV(), abs=1.0e-8)
    assert cds.fair_spread() == pytest.approx(cds_s.fairSpread(), abs=1.0e-10)


def _bermudan_hw_case():
    """Simple HW Bermudan vs European setup (test-suite/bermudanswaption style)."""
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

    # ATM fixed rate via a temporary swap.
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
    # Exercise on fixed-leg accrual starts (all schedule dates except maturity).
    exercise_dates = list(fixed_schedule.dates())[:-1]
    model = ql.HullWhite(curve, 0.048696, 0.0058904)
    return curve, swap, exercise_dates, model, atm


def test_bermudan_tree_npv_ge_jamshidian_european():
    _, swap, exercise_dates, model, _ = _bermudan_hw_case()

    bermudan = ql.Swaption(swap, ql.BermudanExercise(exercise_dates))
    bermudan.set_tree_pricing_engine(model, 50)

    european = ql.Swaption(swap, ql.EuropeanExercise(exercise_dates[0]))
    european.set_jamshidian_pricing_engine(model)

    assert bermudan.NPV() > 0.0
    assert european.NPV() > 0.0
    # Early exercise premium: Bermudan >= European on the same HW model.
    assert bermudan.NPV() + 1.0e-8 >= european.NPV()


def test_bermudan_tree_cached_atm_approx():
    # Cached ATM tree value from bermudanswaption.cpp (at-par coupons).
    _, swap, exercise_dates, model, _ = _bermudan_hw_case()
    swaption = ql.Swaption(swap, ql.BermudanExercise(exercise_dates))
    swaption.set_tree_pricing_engine(model, 50)
    # Tolerance loose vs exact cache (12.9069) — numerical tree + construction.
    assert swaption.NPV() == pytest.approx(12.9069, abs=0.05)


def test_bermudan_optional_swig_comparison():
    QuantLib = pytest.importorskip("QuantLib")

    curve, swap, exercise_dates, model, atm = _bermudan_hw_case()
    bermudan = ql.Swaption(swap, ql.BermudanExercise(exercise_dates))
    bermudan.set_tree_pricing_engine(model, 50)

    QuantLib.Settings.instance().evaluationDate = QuantLib.Date(
        15, QuantLib.February, 2002
    )
    settlement_s = QuantLib.Date(19, QuantLib.February, 2002)
    ts = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(settlement_s, 0.04875825, QuantLib.Actual365Fixed())
    )
    index_s = QuantLib.Euribor6M(ts)
    cal = index_s.fixingCalendar()
    start = cal.advance(settlement_s, 1, QuantLib.Years)
    maturity = cal.advance(start, 5, QuantLib.Years)
    fixed_s = QuantLib.Schedule(
        start,
        maturity,
        QuantLib.Period(QuantLib.Annual),
        cal,
        QuantLib.Unadjusted,
        QuantLib.Unadjusted,
        QuantLib.DateGeneration.Forward,
        False,
    )
    float_s = QuantLib.Schedule(
        start,
        maturity,
        QuantLib.Period(QuantLib.Semiannual),
        cal,
        QuantLib.ModifiedFollowing,
        QuantLib.ModifiedFollowing,
        QuantLib.DateGeneration.Forward,
        False,
    )
    swap_s = QuantLib.VanillaSwap(
        QuantLib.Swap.Payer,
        1000.0,
        fixed_s,
        atm,
        QuantLib.Thirty360(QuantLib.Thirty360.BondBasis),
        float_s,
        index_s,
        0.0,
        index_s.dayCounter(),
    )
    dates_s = list(fixed_s)[:-1]
    model_s = QuantLib.HullWhite(ts, 0.048696, 0.0058904)
    swaption_s = QuantLib.Swaption(swap_s, QuantLib.BermudanExercise(dates_s))
    swaption_s.setPricingEngine(QuantLib.TreeSwaptionEngine(model_s, 50))
    assert bermudan.NPV() == pytest.approx(swaption_s.NPV(), abs=1.0e-6)


def test_uniform_1d_mesher_locations_shape_and_monotonic():
    locs = ql.uniform_1d_mesher_locations(0.0, 100.0, 51)
    assert isinstance(locs, np.ndarray)
    assert locs.shape == (51,)
    assert locs[0] == pytest.approx(0.0)
    assert locs[-1] == pytest.approx(100.0)
    assert np.all(np.diff(locs) > 0.0)
    assert locs[1] - locs[0] == pytest.approx(2.0, abs=1.0e-12)


def test_fdm_black_scholes_mesher_locations():
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    settlement = ql.TARGET().advance(today, 2, ql.TimeUnit.Days)
    dc = ql.Actual365Fixed()
    process = ql.BlackScholesMertonProcess(
        ql.make_quote_handle(100.0),
        ql.FlatForward(settlement, 0.0, dc),
        ql.FlatForward(settlement, 0.06, dc),
        ql.BlackConstantVol(settlement, ql.TARGET(), 0.20, dc),
    )
    locs = ql.fdm_black_scholes_mesher_locations(101, process, 1.0, 100.0)
    assert isinstance(locs, np.ndarray)
    assert locs.shape == (101,)
    assert np.all(np.diff(locs) > 0.0)
    # ln(S) grid should straddle ln(spot) ≈ ln(100).
    assert locs[0] < np.log(100.0) < locs[-1]


def test_compat_phase7_aliases():
    import qlnb.compat as cql

    assert cql.Protection.Seller == ql.ProtectionSide.Seller
    assert cql.Protection.Buyer == ql.ProtectionSide.Buyer
    assert callable(cql.uniform1dMesherLocations)
    locs = cql.uniform1dMesherLocations(1.0, 2.0, 5)
    assert len(locs) == 5
