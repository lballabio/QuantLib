"""Phase-19 tests: YoY inflation coupons and yoyInflationLeg."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase19():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 20)


_RPI_FIXINGS = [
    189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
    192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
    194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
    198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
    202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
    207.3,
]


def _uk_yoy_coupon_market():
    # Mirrors test-suite/inflation.cpp::testYYTermStructure.
    calendar = ql.UnitedKingdom()
    evaluation = calendar.adjust(ql.Date(13, ql.Month.August, 2007))
    ql.set_evaluation_date(evaluation)
    nominal = ql.FlatForward(evaluation, 0.05, ql.Actual360())

    hy = ql.RelinkableYoYInflationTermStructureHandle()
    rpi = ql.UKRPI()
    rpi.clear_fixings()
    yoy = ql.make_yoy_inflation_index(rpi, hy)

    schedule = ql.Schedule(
        ql.Date(1, ql.Month.January, 2005),
        ql.Date(1, ql.Month.July, 2007),
        ql.Period(ql.Frequency.Monthly),
        ql.NullCalendar(),
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )
    for d, fixing in zip(list(schedule.dates()), _RPI_FIXINGS):
        rpi.add_fixing(d, fixing, force_overwrite=True)

    yy_data = [
        (ql.Date(13, ql.Month.August, 2008), 0.0295),
        (ql.Date(13, ql.Month.August, 2009), 0.0295),
        (ql.Date(13, ql.Month.August, 2010), 0.0293),
        (ql.Date(15, ql.Month.August, 2011), 0.02955),
        (ql.Date(13, ql.Month.August, 2012), 0.02945),
        (ql.Date(13, ql.Month.August, 2013), 0.02985),
        (ql.Date(13, ql.Month.August, 2014), 0.0301),
        (ql.Date(13, ql.Month.August, 2015), 0.03035),
        (ql.Date(13, ql.Month.August, 2016), 0.03055),
        (ql.Date(13, ql.Month.August, 2017), 0.03075),
        (ql.Date(13, ql.Month.August, 2019), 0.03105),
        (ql.Date(15, ql.Month.August, 2022), 0.03135),
        (ql.Date(13, ql.Month.August, 2027), 0.03155),
        (ql.Date(13, ql.Month.August, 2032), 0.03145),
        (ql.Date(13, ql.Month.August, 2037), 0.03145),
    ]
    observation_lag = ql.Period(2, ql.TimeUnit.Months)
    dc = ql.Thirty360(ql.Thirty360Convention.BondBasis)
    bdc = ql.BusinessDayConvention.ModifiedFollowing
    interp = ql.CPIInterpolationType.Flat

    helpers = [
        ql.YearOnYearInflationSwapHelper(
            ql.make_quote_handle(rate),
            observation_lag,
            maturity,
            calendar,
            bdc,
            dc,
            yoy,
            interp,
            nominal,
        )
        for maturity, rate in yy_data
    ]
    curve = ql.PiecewiseYoYInflationCurve(
        evaluation,
        rpi.last_fixing_date(),
        yy_data[0][1],
        ql.Frequency.Monthly,
        dc,
        helpers,
    )
    hy.link_to(curve)
    return (
        evaluation,
        nominal,
        yoy,
        yy_data,
        observation_lag,
        calendar,
        bdc,
        dc,
        interp,
    )


def test_yoy_coupon_fixing_date_matches_helper():
    # Golden from InflationTest::testYYTermStructure.
    (
        evaluation,
        _nominal,
        yoy,
        yy_data,
        observation_lag,
        calendar,
        bdc,
        dc,
        interp,
    ) = _uk_yoy_coupon_market()

    maturity = yy_data[0][0]
    sched = ql.Schedule(
        evaluation,
        maturity,
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Backward,
        False,
    )
    leg = ql.make_yoy_inflation_leg(
        sched,
        calendar,
        yoy,
        observation_lag,
        interp,
        dc,
        notional=1_000_000.0,
        payment_convention=bdc,
    )
    assert len(leg) > 0
    assert isinstance(leg[0], ql.CashFlow)
    assert isinstance(leg[0], ql.YoYInflationCoupon)
    assert leg[0].fixing_date() == ql.Date(13, ql.Month.June, 2008)


def test_yoy_leg_npv_matches_yyiis_yoy_leg():
    (
        evaluation,
        nominal,
        yoy,
        yy_data,
        observation_lag,
        calendar,
        _bdc,
        dc,
        interp,
    ) = _uk_yoy_coupon_market()

    maturity, rate = yy_data[1]  # second pillar (first is base YoY)
    sched = ql.Schedule(
        evaluation,
        maturity,
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Backward,
        False,
    )
    yyiis = ql.YearOnYearInflationSwap(
        ql.SwapType.Payer,
        1_000_000.0,
        sched,
        rate,
        dc,
        sched,
        yoy,
        observation_lag,
        interp,
        0.0,
        dc,
        calendar,
    )
    yyiis.set_pricing_engine(nominal)
    assert yyiis.NPV() == pytest.approx(0.0, abs=1.0e-6)

    leg = ql.make_yoy_inflation_leg(
        sched,
        calendar,
        yoy,
        observation_lag,
        interp,
        dc,
        notional=1_000_000.0,
    )
    ql.set_yoy_coupon_pricer(leg, ql.YoYInflationCouponPricer(nominal))
    settlement = evaluation
    leg_npv = ql.cashflows_npv(leg, nominal, settlement)
    assert leg_npv == pytest.approx(yyiis.yoy_leg_NPV(), rel=1.0e-10, abs=1.0e-6)


def test_standalone_yoy_coupon_rate():
    (
        evaluation,
        nominal,
        yoy,
        _yy_data,
        observation_lag,
        calendar,
        bdc,
        dc,
        interp,
    ) = _uk_yoy_coupon_market()
    start = evaluation
    end = calendar.advance(start, 1, ql.TimeUnit.Years, bdc)
    pay = calendar.adjust(end, bdc)
    cpn = ql.YoYInflationCoupon(
        pay,
        1_000_000.0,
        start,
        end,
        0,
        yoy,
        observation_lag,
        interp,
        dc,
        1.0,
        0.0,
    )
    cpn.set_pricer(ql.YoYInflationCouponPricer(nominal))
    assert cpn.gearing() == pytest.approx(1.0)
    assert cpn.spread() == pytest.approx(0.0)
    assert cpn.rate() == pytest.approx(cpn.adjusted_fixing(), abs=1.0e-12)
    assert cpn.index_fixing() == pytest.approx(cpn.adjusted_fixing(), abs=1.0e-12)


def test_compat_phase19_aliases():
    import qlnb.compat as cql

    assert callable(cql.yoyInflationLeg)
    assert callable(cql.YoYInflationCouponPricer)
    assert hasattr(cql.YoYInflationCoupon, "setPricer")
    assert hasattr(cql.YoYInflationCoupon, "indexFixing")
    assert hasattr(cql.YoYInflationCoupon, "adjustedFixing")
    assert callable(cql.setYoYCouponPricer)
    assert callable(cql.BlackYoYInflationCouponPricer)
