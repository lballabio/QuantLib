"""Phase-1 market-stack tests for qlnb."""

from __future__ import annotations

import math

import pytest

import qlnb as ql


def test_period_and_date_arithmetic():
    p = ql.Period(6, ql.TimeUnit.Months)
    assert p.length() == 6
    assert p.units() == ql.TimeUnit.Months
    assert (p * 2).length() == 12

    d = ql.Date(15, ql.Month.May, 1998)
    assert d + p == ql.Date(15, ql.Month.November, 1998)


def test_schedule_generation():
    calendar = ql.TARGET()
    schedule = ql.Schedule(
        ql.Date(15, ql.Month.May, 1998),
        ql.Date(15, ql.Month.May, 2000),
        ql.Period(ql.Frequency.Semiannual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Backward,
        False,
    )
    assert len(schedule) == 5
    assert schedule.start_date() == ql.Date(15, ql.Month.May, 1998)
    assert schedule.end_date() == ql.Date(15, ql.Month.May, 2000)
    assert schedule[0] == schedule.start_date()
    assert schedule.dates()[0] == schedule[0]


def test_calendar_advance_and_day_counters():
    cal = ql.UnitedStates(ql.UnitedStatesMarket.GovernmentBond)
    d = ql.Date(4, ql.Month.July, 2024)  # US Independence Day observed context
    assert cal.is_holiday(d) or not cal.is_business_day(d)

    dc = ql.Thirty360()
    assert "360" in dc.name()
    yf = ql.Actual360().year_fraction(
        ql.Date(1, ql.Month.January, 2024), ql.Date(1, ql.Month.July, 2024)
    )
    assert yf == pytest.approx(182.0 / 360.0)


def test_deposit_bootstrap_zero_rate():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    calendar = ql.TARGET()
    settlement = calendar.advance(todays, 2, ql.TimeUnit.Days)
    dc = ql.Actual360()

    helpers = [
        ql.DepositRateHelper(
            0.04,
            ql.Period(3, ql.TimeUnit.Months),
            2,
            calendar,
            ql.BusinessDayConvention.ModifiedFollowing,
            True,
            dc,
        ),
        ql.DepositRateHelper(
            0.045,
            ql.Period(6, ql.TimeUnit.Months),
            2,
            calendar,
            ql.BusinessDayConvention.ModifiedFollowing,
            True,
            dc,
        ),
        ql.DepositRateHelper(
            0.05,
            ql.Period(1, ql.TimeUnit.Years),
            2,
            calendar,
            ql.BusinessDayConvention.ModifiedFollowing,
            True,
            dc,
        ),
    ]
    curve = ql.PiecewiseLogLinearDiscountCurve(
        settlement, helpers, ql.Actual365Fixed()
    )
    assert not curve.empty()
    z = curve.zero_rate(
        settlement + ql.Period(6, ql.TimeUnit.Months),
        ql.Actual365Fixed(),
        ql.Compounding.Continuous,
    )
    assert 0.03 < float(z) < 0.07
    df = curve.discount(settlement + ql.Period(3, ql.TimeUnit.Months))
    assert 0.95 < df < 1.0


def test_fixed_rate_bond_clean_price():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    calendar = ql.TARGET()
    settlement = calendar.advance(todays, 3, ql.TimeUnit.Days)
    issue = ql.Date(15, ql.Month.May, 1998)
    maturity = ql.Date(15, ql.Month.May, 2003)

    schedule = ql.Schedule(
        issue,
        maturity,
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Backward,
        False,
    )
    bond = ql.FixedRateBond(
        3,
        100.0,
        schedule,
        [0.05],
        ql.ActualActual(ql.ActualActualConvention.ISMA),
        ql.BusinessDayConvention.Following,
        100.0,
        issue,
    )
    curve = ql.FlatForward(settlement, 0.05, ql.Actual365Fixed())
    bond.set_pricing_engine(curve)

    # Par-ish coupon close to discount rate => clean price near 100.
    assert bond.clean_price() == pytest.approx(100.0, abs=1.5)
    assert bond.NPV() > 0.0
    assert bond.maturity_date() == maturity


def test_vanilla_swap_fair_rate_matches_flat_curve():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    calendar = ql.TARGET()
    settlement = calendar.advance(todays, 2, ql.TimeUnit.Days)
    flat = ql.FlatForward(settlement, 0.05, ql.Actual365Fixed())

    index = ql.Euribor6M(flat)
    fixed_schedule = ql.Schedule(
        settlement,
        calendar.advance(settlement, ql.Period(5, ql.TimeUnit.Years)),
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )
    float_schedule = ql.Schedule(
        settlement,
        calendar.advance(settlement, ql.Period(5, ql.TimeUnit.Years)),
        ql.Period(ql.Frequency.Semiannual),
        calendar,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.DateGeneration.Forward,
        True,
    )
    swap = ql.VanillaSwap(
        ql.SwapType.Payer,
        1_000_000.0,
        fixed_schedule,
        0.05,
        ql.Thirty360(),
        float_schedule,
        index,
        0.0,
        ql.Actual360(),
    )
    swap.set_pricing_engine(flat)
    # On a flat curve, fair fixed rate should be near the curve rate.
    assert swap.fair_rate() == pytest.approx(0.05, abs=0.005)
    # A swap struck at the fair rate has ~0 NPV.
    fair = swap.fair_rate()
    swap2 = ql.VanillaSwap(
        ql.SwapType.Payer,
        1_000_000.0,
        fixed_schedule,
        fair,
        ql.Thirty360(),
        float_schedule,
        index,
        0.0,
        ql.Actual360(),
    )
    swap2.set_pricing_engine(flat)
    assert swap2.NPV() == pytest.approx(0.0, abs=1.0)


def test_phase1_matches_swig_bond_if_available():
    QuantLib = pytest.importorskip("QuantLib")

    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    calendar = ql.TARGET()
    settlement = calendar.advance(todays, 3, ql.TimeUnit.Days)
    issue = ql.Date(15, ql.Month.May, 1998)
    maturity = ql.Date(15, ql.Month.May, 2003)
    schedule = ql.Schedule(
        issue,
        maturity,
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Backward,
        False,
    )
    bond = ql.FixedRateBond(
        3,
        100.0,
        schedule,
        [0.05],
        ql.ActualActual(ql.ActualActualConvention.ISMA),
    )
    curve = ql.FlatForward(settlement, 0.04, ql.Actual365Fixed())
    bond.set_pricing_engine(curve)
    qlnb_price = bond.clean_price()

    QuantLib.Settings.instance().evaluationDate = QuantLib.Date(15, QuantLib.May, 1998)
    ql_calendar = QuantLib.TARGET()
    ql_settlement = ql_calendar.advance(QuantLib.Date(15, QuantLib.May, 1998), 3, QuantLib.Days)
    ql_schedule = QuantLib.Schedule(
        QuantLib.Date(15, QuantLib.May, 1998),
        QuantLib.Date(15, QuantLib.May, 2003),
        QuantLib.Period(QuantLib.Annual),
        ql_calendar,
        QuantLib.Unadjusted,
        QuantLib.Unadjusted,
        QuantLib.DateGeneration.Backward,
        False,
    )
    ql_bond = QuantLib.FixedRateBond(
        3,
        100.0,
        ql_schedule,
        [0.05],
        QuantLib.ActualActual(QuantLib.ActualActual.ISMA),
    )
    ql_curve = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(ql_settlement, 0.04, QuantLib.Actual365Fixed())
    )
    ql_bond.setPricingEngine(QuantLib.DiscountingBondEngine(ql_curve))
    assert qlnb_price == pytest.approx(ql_bond.cleanPrice(), rel=1e-10, abs=1e-10)
