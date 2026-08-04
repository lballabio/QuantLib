"""Phase-11 tests: CMS-spread indexes, LinearTsr, LognormalCmsSpreadPricer."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_at_least_phase11():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 12)


def _spread_market():
    # Mirrors test-suite/cmsspread.cpp::TestData (lognormal vol case).
    ref = ql.Date(23, ql.Month.February, 2018)
    ql.set_evaluation_date(ref)
    yts = ql.FlatForward(ref, 0.02, ql.Actual365Fixed())
    vol = ql.ConstantSwaptionVolatility(
        ref,
        ql.TARGET(),
        ql.BusinessDayConvention.Following,
        0.20,
        ql.Actual365Fixed(),
    )
    reversion = ql.make_quote_handle(0.01)
    correlation = ql.make_quote_handle(0.6)
    cms_pricer = ql.LinearTsrPricer(vol, reversion, yts)
    spread_pricer = ql.LognormalCmsSpreadPricer(
        cms_pricer, correlation, yts, integration_points=32
    )
    cms10 = ql.EuriborSwapIsdaFixA(
        ql.Period(10, ql.TimeUnit.Years), yts, yts
    )
    cms2 = ql.EuriborSwapIsdaFixA(ql.Period(2, ql.TimeUnit.Years), yts, yts)
    spread = ql.SwapSpreadIndex("cms10y2y", cms10, cms2)
    return ref, yts, cms10, cms2, spread, cms_pricer, spread_pricer


def test_swap_spread_index_fixings():
    ref, _, cms10, cms2, spread, *_ = _spread_market()
    assert spread.fixing_days() == 2
    assert spread.gearing1() == 1.0
    assert spread.gearing2() == -1.0

    # Forecast today before historic fixings.
    today_fixing = spread.fixing(ref)
    assert today_fixing == pytest.approx(
        cms10.fixing(ref) - cms2.fixing(ref), abs=1.0e-12
    )

    cms10.add_fixing(ref, 0.05, force_overwrite=True)
    cms2.add_fixing(ref, 0.04, force_overwrite=True)
    assert spread.fixing(ref) == pytest.approx(0.01, abs=1.0e-12)


def test_cms_spread_coupon_today_equals_cms_difference():
    ref, _, cms10, cms2, spread, cms_pricer, spread_pricer = _spread_market()
    value_date = spread.value_date(ref)
    pay = value_date + ql.Period(1, ql.TimeUnit.Years)

    cpn10 = ql.CmsCoupon(
        pay,
        10000.0,
        value_date,
        pay,
        cms10.fixing_days(),
        cms10,
        day_counter=ql.Actual360(),
    )
    cpn2 = ql.CmsCoupon(
        pay,
        10000.0,
        value_date,
        pay,
        cms2.fixing_days(),
        cms2,
        day_counter=ql.Actual360(),
    )
    cpn_sp = ql.CmsSpreadCoupon(
        pay,
        10000.0,
        value_date,
        pay,
        spread.fixing_days(),
        spread,
        day_counter=ql.Actual360(),
    )
    assert cpn_sp.fixing_date() == ref

    cpn10.set_pricer(cms_pricer)
    cpn2.set_pricer(cms_pricer)
    cpn_sp.set_pricer(spread_pricer)

    assert cpn_sp.rate() == pytest.approx(cpn10.rate() - cpn2.rate(), abs=1.0e-12)

    cms10.add_fixing(ref, 0.05, force_overwrite=True)
    cms2.add_fixing(ref, 0.03, force_overwrite=True)
    assert cpn_sp.rate() == pytest.approx(0.02, abs=1.0e-12)
    assert cpn_sp.rate() == pytest.approx(cpn10.rate() - cpn2.rate(), abs=1.0e-12)


def test_capped_floored_cms_spread_forward_rates():
    # Cached rates from QuantLib SWIG on the cmsspread.cpp TestData market.
    _, _, cms10, cms2, spread, cms_pricer, spread_pricer = _spread_market()
    pay = ql.Date(23, ql.Month.February, 2029)
    start = ql.Date(23, ql.Month.February, 2028)

    cpn10 = ql.CmsCoupon(
        pay, 10000.0, start, pay, 2, cms10, day_counter=ql.Actual360()
    )
    cpn2 = ql.CmsCoupon(
        pay, 10000.0, start, pay, 2, cms2, day_counter=ql.Actual360()
    )
    cpn10.set_pricer(cms_pricer)
    cpn2.set_pricer(cms_pricer)

    plain = ql.CappedFlooredCmsSpreadCoupon(
        pay, 10000.0, start, pay, 2, spread, day_counter=ql.Actual360()
    )
    capped = ql.CappedFlooredCmsSpreadCoupon(
        pay,
        10000.0,
        start,
        pay,
        2,
        spread,
        cap=0.03,
        day_counter=ql.Actual360(),
    )
    floored = ql.CappedFlooredCmsSpreadCoupon(
        pay,
        10000.0,
        start,
        pay,
        2,
        spread,
        floor=0.01,
        day_counter=ql.Actual360(),
    )
    collared = ql.CappedFlooredCmsSpreadCoupon(
        pay,
        10000.0,
        start,
        pay,
        2,
        spread,
        cap=0.03,
        floor=0.01,
        day_counter=ql.Actual360(),
    )
    for cpn in (plain, capped, floored, collared):
        cpn.set_pricer(spread_pricer)

    # Linearity of the uncapped spread vs single CMS legs.
    assert plain.rate() == pytest.approx(cpn10.rate() - cpn2.rate(), abs=1.0e-12)
    assert plain.rate() == pytest.approx(0.00075087870996, abs=1.0e-12)
    assert capped.rate() == pytest.approx(0.00042748705356, abs=1.0e-12)
    assert floored.rate() == pytest.approx(0.01177566933970, abs=1.0e-12)
    # Collar sits between floor and cap adjustments.
    assert floored.rate() >= plain.rate() - 1.0e-12
    assert capped.rate() <= plain.rate() + 1.0e-12
    assert floored.rate() >= collared.rate() - 1.0e-12
    assert collared.rate() >= capped.rate() - 1.0e-12


def test_compat_phase11_aliases():
    import qlnb.compat as cql

    assert callable(cql.LinearTsrPricer)
    assert callable(cql.LognormalCmsSpreadPricer)
    assert callable(cql.SwapSpreadIndex)
    assert hasattr(cql.CmsSpreadCoupon, "setPricer")
    assert hasattr(cql.SwapIndex, "addFixing")
