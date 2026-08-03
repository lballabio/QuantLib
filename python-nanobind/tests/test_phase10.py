"""Phase-10 tests: CMS / SwapIndex / Hagan pricers."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_at_least_phase10():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 11)


def _cms_market():
    calendar = ql.TARGET()
    today = ql.Date(3, ql.Month.August, 2026)
    ref = calendar.adjust(today)
    ql.set_evaluation_date(ref)
    yts = ql.FlatForward(ref, 0.05, ql.Actual365Fixed())
    ibor = ql.Euribor6M(yts)
    swap_index = ql.EuriborSwapIsdaFixA(ql.Period(10, ql.TimeUnit.Years), yts)
    vol = ql.ConstantSwaptionVolatility(
        ref,
        calendar,
        ql.BusinessDayConvention.Following,
        0.20,
        ql.Actual365Fixed(),
    )
    mr = ql.make_quote_handle(0.0)
    return ref, yts, ibor, swap_index, vol, mr


def test_euribor_swap_isda_fix_a_inspectors():
    _, yts, _, swap_index, _, _ = _cms_market()
    assert "EuriborSwapIsdaFixA" in swap_index.name()
    assert swap_index.tenor() == ql.Period(10, ql.TimeUnit.Years)
    assert swap_index.fixing_days() == 2
    assert not swap_index.fixing_calendar().name() == ""


def test_constant_swaption_vol_handle():
    ref, _, _, _, vol, _ = _cms_market()
    assert not vol.empty()
    # ATM-ish strike; constant vol should return the input.
    v = vol.volatility(
        ref + ql.Period(1, ql.TimeUnit.Years),
        ql.Period(10, ql.TimeUnit.Years),
        0.05,
    )
    assert v == pytest.approx(0.20, abs=1.0e-12)


def test_cms_coupon_hagan_fair_rate_cached():
    # Mirrors test-suite/cms.cpp::testFairRate with ConstantSwaptionVolatility.
    # Golden analytic rate from QuantLib 1.43 SWIG on this market.
    ref, yts, ibor, swap_index, vol, mr = _cms_market()
    start = ref + ql.Period(20, ql.TimeUnit.Years)
    pay = start + ql.Period(1, ql.TimeUnit.Years)
    coupon = ql.CmsCoupon(
        pay,
        1.0,
        start,
        pay,
        swap_index.fixing_days(),
        swap_index,
        1.0,
        0.0,
        start,
        pay,
        ibor.day_counter(),
    )

    analytic = ql.AnalyticHaganPricer(vol, ql.YieldCurveModel.Standard, mr)
    numeric = ql.NumericHaganPricer(vol, ql.YieldCurveModel.Standard, mr)

    coupon.set_pricer(analytic)
    r_ana = coupon.rate()
    coupon.set_pricer(numeric)
    r_num = coupon.rate()

    assert r_ana == pytest.approx(0.063891583538, abs=1.0e-9)
    # cms.cpp uses 2e-4 analytic-vs-numeric tolerance.
    assert abs(r_ana - r_num) < 2.0e-4
    assert coupon.amount() != 0.0


def test_make_cms_npv_with_hagan_pricer():
    ref, yts, ibor, swap_index, vol, mr = _cms_market()
    pricer = ql.AnalyticHaganPricer(vol, ql.YieldCurveModel.Standard, mr)
    cms = ql.make_cms(
        ql.Period(5, ql.TimeUnit.Years),
        swap_index,
        ibor,
        ibor_spread=0.0,
        discount_curve=yts,
        pricer=pricer,
        nominal=1.0,
    )
    assert cms.number_of_legs() == 2
    assert not cms.is_expired()
    npv = cms.NPV()
    assert npv == pytest.approx(-0.012779842583, abs=5.0e-4)

    # Re-attach numeric pricer on CMS leg and require close NPV.
    numeric = ql.NumericHaganPricer(vol, ql.YieldCurveModel.Standard, mr)
    cms.set_cms_coupon_pricer(numeric)
    assert cms.NPV() == pytest.approx(npv, abs=2.0e-3)


def test_compat_phase10_aliases():
    import qlnb.compat as cql

    assert callable(cql.EuriborSwapIsdaFixA)
    assert callable(cql.makeCms)
    assert hasattr(cql.CmsCoupon, "setPricer")
    assert hasattr(cql.Swap, "setCmsCouponPricer")
