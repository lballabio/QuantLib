"""Phase-23 tests: callable / puttable fixed-rate bonds."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase23():
    assert ql.__version__ == "0.24.0"


def _callable_bond_market():
    # Mirrors CallableBondTests::testCached / Globals.
    calendar = ql.TARGET()
    today = ql.Date(3, ql.Month.June, 2004)
    ql.set_evaluation_date(today)
    settlement = calendar.advance(today, 3, ql.TimeUnit.Days)
    dc = ql.Actual365Fixed()
    curve = ql.FlatForward(settlement, 0.032, dc)
    model = ql.HullWhite(curve)  # a=0.1, sigma=0.01 defaults
    issue = calendar.adjust(today - 100)
    maturity = calendar.advance(issue, 10, ql.TimeUnit.Years)
    schedule = ql.Schedule(
        issue,
        maturity,
        ql.Period(ql.Frequency.Semiannual),
        calendar,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.DateGeneration.Backward,
        False,
    )
    return today, settlement, calendar, curve, model, issue, maturity, schedule


def _even_years(calendar, issue):
    return [
        calendar.advance(issue, i, ql.TimeUnit.Years)
        for i in range(2, 10, 2)
    ]


def _odd_years(calendar, issue):
    return [
        calendar.advance(issue, i, ql.TimeUnit.Years)
        for i in range(1, 10, 2)
    ]


def test_callable_bond_cached_prices():
    (
        _today,
        _settlement,
        calendar,
        curve,
        model,
        issue,
        _maturity,
        schedule,
    ) = _callable_bond_market()
    coupons = [0.05]
    bdc = ql.BusinessDayConvention.ModifiedFollowing
    accrual = ql.Thirty360(ql.Thirty360Convention.BondBasis)

    calls = [
        ql.make_callability(
            110.0, ql.BondPriceType.Clean, ql.CallabilityType.Call, d
        )
        for d in _even_years(calendar, issue)
    ]
    puts = [
        ql.make_callability(
            100.0, ql.BondPriceType.Clean, ql.CallabilityType.Put, d
        )
        for d in _odd_years(calendar, issue)
    ]

    callable_bond = ql.CallableFixedRateBond(
        3, 10000.0, schedule, coupons, accrual, bdc, 100.0, issue, calls,
    )
    callable_bond.set_tree_pricing_engine(model, 240, curve)
    assert callable_bond.clean_price() == pytest.approx(110.60975477, abs=1.0e-8)

    puttable_bond = ql.CallableFixedRateBond(
        3, 10000.0, schedule, coupons, accrual, bdc, 100.0, issue, puts,
    )
    puttable_bond.set_tree_pricing_engine(model, 240, curve)
    assert puttable_bond.clean_price() == pytest.approx(115.16559362, abs=1.0e-8)

    both = ql.CallableFixedRateBond(
        3, 10000.0, schedule, coupons, accrual, bdc, 100.0, issue, calls + puts,
    )
    both.set_tree_pricing_engine(model, 240, curve)
    assert both.clean_price() == pytest.approx(110.97509625, abs=1.0e-8)


def test_callable_vs_plain_ordering():
    (
        _today,
        _settlement,
        calendar,
        curve,
        model,
        issue,
        _maturity,
        schedule,
    ) = _callable_bond_market()
    coupons = [0.05]
    bdc = ql.BusinessDayConvention.ModifiedFollowing
    accrual = ql.Thirty360(ql.Thirty360Convention.BondBasis)

    plain = ql.FixedRateBond(3, 100.0, schedule, coupons, accrual, bdc, 100.0, issue)
    plain.set_pricing_engine(curve)

    calls = [
        ql.make_callability(
            110.0, ql.BondPriceType.Clean, ql.CallabilityType.Call, d
        )
        for d in _even_years(calendar, issue)
    ]
    puts = [
        ql.make_callability(
            90.0, ql.BondPriceType.Clean, ql.CallabilityType.Put, d
        )
        for d in _odd_years(calendar, issue)
    ]
    callable_bond = ql.CallableFixedRateBond(
        3, 100.0, schedule, coupons, accrual, bdc, 100.0, issue, calls,
    )
    callable_bond.set_tree_pricing_engine(model, 240, curve)
    puttable_bond = ql.CallableFixedRateBond(
        3, 100.0, schedule, coupons, accrual, bdc, 100.0, issue, puts,
    )
    puttable_bond.set_tree_pricing_engine(model, 240, curve)

    assert callable_bond.clean_price() < plain.clean_price()
    assert puttable_bond.clean_price() > plain.clean_price()


def test_compat_phase23_aliases():
    import qlnb.compat as cql

    assert callable(cql.Callability)
    assert cql.CallableFixedRateBond is not None
    assert hasattr(cql.CallableFixedRateBond, "cleanPrice")
    assert hasattr(cql.CallableFixedRateBond, "setTreePricingEngine")
    assert cql.BondPrice is not None
