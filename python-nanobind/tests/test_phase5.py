"""Phase-5 rates options + ergonomics tests (swaption, zero bond, NumPy helpers)."""

from __future__ import annotations

import numpy as np
import pytest

import qlnb as ql


def test_version_is_at_least_phase5():
    # Package version advances with later phases; keep this as a presence check.
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 5)


def test_european_swaption_black_cached_npv():
    # Mirrors test-suite/swaption.cpp::testCachedValue (at-par coupons path).
    today = ql.Date(13, ql.Month.March, 2002)
    settlement = ql.Date(15, ql.Month.March, 2002)
    ql.set_evaluation_date(today)
    curve = ql.FlatForward(settlement, 0.05, ql.Actual365Fixed())
    index = ql.Euribor6M(curve)
    calendar = index.fixing_calendar()

    exercise_date = calendar.advance(settlement, ql.Period(5, ql.TimeUnit.Years))
    start_date = calendar.advance(exercise_date, 2, ql.TimeUnit.Days)
    swap = ql.make_vanilla_swap(
        ql.Period(10, ql.TimeUnit.Years),
        index,
        0.06,
        start_date,
        fixed_leg_tenor=ql.Period(1, ql.TimeUnit.Years),
        fixed_day_count=ql.Thirty360(ql.Thirty360Convention.BondBasis),
        type=ql.SwapType.Payer,
        nominal=1.0,
    )
    swaption = ql.Swaption(swap, ql.EuropeanExercise(exercise_date))
    swaption.set_pricing_engine(curve, 0.20)

    # Cached at-par coupon NPV from QuantLib test-suite.
    assert swaption.NPV() == pytest.approx(0.036418158579, abs=1.0e-12)
    assert swaption.type() == ql.SwapType.Payer
    assert swaption.settlement_type() == ql.SettlementType.Physical
    assert not swaption.is_expired()


def test_zero_coupon_bond_cached_clean_price():
    # Mirrors test-suite/bonds.cpp::testCachedZero (bond1).
    today = ql.Date(22, ql.Month.November, 2004)
    ql.set_evaluation_date(today)
    curve = ql.FlatForward(today, 0.03, ql.Actual360())
    bond = ql.ZeroCouponBond(
        1,
        ql.UnitedStates(ql.UnitedStatesMarket.GovernmentBond),
        1_000_000.0,
        ql.Date(30, ql.Month.November, 2008),
        ql.BusinessDayConvention.ModifiedFollowing,
        100.0,
        ql.Date(30, ql.Month.November, 2004),
    )
    bond.set_pricing_engine(curve)
    assert bond.clean_price() == pytest.approx(88.551726, abs=1.0e-6)
    assert bond.NPV() > 0.0
    assert bond.maturity_date() == ql.Date(30, ql.Month.November, 2008)


def test_zero_coupon_bond_flat_curve_npv_positive():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    calendar = ql.TARGET()
    settlement = calendar.advance(todays, 2, ql.TimeUnit.Days)
    maturity = calendar.advance(settlement, ql.Period(5, ql.TimeUnit.Years))
    curve = ql.FlatForward(settlement, 0.04, ql.Actual365Fixed())
    bond = ql.ZeroCouponBond(2, calendar, 100.0, maturity)
    bond.set_pricing_engine(curve)
    # Clean price ≈ 100 × DF(maturity) for a zero (percent of par).
    df = curve.discount(bond.maturity_date())
    assert bond.clean_price() == pytest.approx(100.0 * df, abs=0.05)
    assert bond.NPV() > 0.0
    assert bond.dirty_price() >= bond.clean_price() - 1.0e-10


def test_discount_times_numpy_matches_scalar():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    curve = ql.FlatForward(todays, 0.05, ql.Actual365Fixed())
    times = np.array([0.25, 0.5, 1.0, 2.0, 5.0], dtype=np.float64)
    dfs = ql.discount_times(curve, times)
    assert isinstance(dfs, np.ndarray)
    assert dfs.shape == times.shape
    for t, df in zip(times, dfs, strict=True):
        assert df == pytest.approx(curve.discount(float(t)), abs=1.0e-14)


def test_discount_dates_numpy_matches_scalar():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    curve = ql.FlatForward(todays, 0.05, ql.Actual365Fixed())
    dates = [
        todays + ql.Period(3, ql.TimeUnit.Months),
        todays + ql.Period(1, ql.TimeUnit.Years),
        todays + ql.Period(5, ql.TimeUnit.Years),
    ]
    dfs = ql.discount_dates(curve, dates)
    assert isinstance(dfs, np.ndarray)
    assert len(dfs) == len(dates)
    for d, df in zip(dates, dfs, strict=True):
        assert df == pytest.approx(curve.discount(d), abs=1.0e-14)


def test_japan_germany_calendars():
    jp = ql.Japan()
    de = ql.Germany(ql.GermanyMarket.FrankfurtStockExchange)
    assert "Japan" in jp.name() or "japanese" in jp.name().lower()
    assert "Frankfurt" in de.name() or "German" in de.name()
    # New Year's Day is a holiday on both calendars.
    new_year = ql.Date(1, ql.Month.January, 2024)
    assert jp.is_holiday(new_year)
    assert de.is_holiday(new_year)


def test_fra_and_swap_rate_helpers_bootstrap_smoke():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    calendar = ql.TARGET()
    settlement = calendar.advance(todays, 2, ql.TimeUnit.Days)
    dc = ql.Actual360()
    # Dummy index without a curve; SwapRateHelper creates its own forecast handle.
    index = ql.Euribor6M()
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
        ql.FraRateHelper(
            0.045,
            3,
            6,
            2,
            calendar,
            ql.BusinessDayConvention.ModifiedFollowing,
            True,
            dc,
        ),
        ql.SwapRateHelper(
            0.05,
            ql.Period(2, ql.TimeUnit.Years),
            calendar,
            ql.Frequency.Annual,
            ql.BusinessDayConvention.Unadjusted,
            ql.Thirty360(ql.Thirty360Convention.BondBasis),
            index,
        ),
    ]
    curve = ql.PiecewiseLogLinearDiscountCurve(
        settlement, helpers, ql.Actual365Fixed()
    )
    assert not curve.empty()
    assert 0.9 < curve.discount(settlement + ql.Period(1, ql.TimeUnit.Years)) < 1.0


def test_compat_swaption_and_settlement_aliases():
    import qlnb.compat as ql_c

    assert ql_c.Settlement.Physical == ql.SettlementType.Physical
    assert ql_c.VanillaSwap.Payer == ql.SwapType.Payer
    assert hasattr(ql_c.Swaption, "setPricingEngine")
    assert hasattr(ql_c.ZeroCouponBond, "cleanPrice")


def test_swaption_optional_swig_comparison():
    QuantLib = pytest.importorskip("QuantLib")

    # Same cached scenario as test_european_swaption_black_cached_npv / QL test-suite.
    today = ql.Date(13, ql.Month.March, 2002)
    settlement = ql.Date(15, ql.Month.March, 2002)
    ql.set_evaluation_date(today)
    curve = ql.FlatForward(settlement, 0.05, ql.Actual365Fixed())
    index = ql.Euribor6M(curve)
    calendar = index.fixing_calendar()
    exercise_date = calendar.advance(settlement, ql.Period(5, ql.TimeUnit.Years))
    start_date = calendar.advance(exercise_date, 2, ql.TimeUnit.Days)
    swap = ql.make_vanilla_swap(
        ql.Period(10, ql.TimeUnit.Years),
        index,
        0.06,
        start_date,
        fixed_leg_tenor=ql.Period(1, ql.TimeUnit.Years),
        fixed_day_count=ql.Thirty360(ql.Thirty360Convention.BondBasis),
    )
    swaption = ql.Swaption(swap, ql.EuropeanExercise(exercise_date))
    swaption.set_pricing_engine(curve, 0.20)
    qlnb_npv = swaption.NPV()

    today_s = QuantLib.Date(13, QuantLib.March, 2002)
    settlement_s = QuantLib.Date(15, QuantLib.March, 2002)
    QuantLib.Settings.instance().evaluationDate = today_s
    curve_s = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(settlement_s, 0.05, QuantLib.Actual365Fixed())
    )
    index_s = QuantLib.Euribor6M(curve_s)
    cal_s = index_s.fixingCalendar()
    exercise_s = cal_s.advance(settlement_s, 5, QuantLib.Years)
    start_s = cal_s.advance(exercise_s, 2, QuantLib.Days)
    swap_s = QuantLib.MakeVanillaSwap(
        QuantLib.Period(10, QuantLib.Years),
        index_s,
        0.06,
        effectiveDate=start_s,
        fixedLegTenor=QuantLib.Period(1, QuantLib.Years),
        fixedLegDayCount=QuantLib.Thirty360(QuantLib.Thirty360.BondBasis),
    )
    swaption_s = QuantLib.Swaption(
        swap_s, QuantLib.EuropeanExercise(exercise_s)
    )
    vol_s = QuantLib.QuoteHandle(QuantLib.SimpleQuote(0.20))
    swaption_s.setPricingEngine(
        QuantLib.BlackSwaptionEngine(curve_s, vol_s, QuantLib.Actual365Fixed())
    )
    assert qlnb_npv == pytest.approx(swaption_s.NPV(), abs=1.0e-12)


def test_zero_coupon_optional_swig_comparison():
    QuantLib = pytest.importorskip("QuantLib")

    today = ql.Date(22, ql.Month.November, 2004)
    ql.set_evaluation_date(today)
    curve = ql.FlatForward(today, 0.03, ql.Actual360())
    bond = ql.ZeroCouponBond(
        1,
        ql.UnitedStates(ql.UnitedStatesMarket.GovernmentBond),
        1_000_000.0,
        ql.Date(30, ql.Month.November, 2008),
        ql.BusinessDayConvention.ModifiedFollowing,
        100.0,
        ql.Date(30, ql.Month.November, 2004),
    )
    bond.set_pricing_engine(curve)
    qlnb_price = bond.clean_price()

    today_s = QuantLib.Date(22, QuantLib.November, 2004)
    QuantLib.Settings.instance().evaluationDate = today_s
    curve_s = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(today_s, 0.03, QuantLib.Actual360())
    )
    bond_s = QuantLib.ZeroCouponBond(
        1,
        QuantLib.UnitedStates(QuantLib.UnitedStates.GovernmentBond),
        1_000_000.0,
        QuantLib.Date(30, QuantLib.November, 2008),
        QuantLib.ModifiedFollowing,
        100.0,
        QuantLib.Date(30, QuantLib.November, 2004),
    )
    bond_s.setPricingEngine(QuantLib.DiscountingBondEngine(curve_s))
    assert qlnb_price == pytest.approx(bond_s.cleanPrice(), abs=1.0e-8)
