"""Phase-20 tests: capped/floored YoY inflation coupons."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase20():
    assert ql.__version__ == "0.21.0"


_RPI_FIXINGS = [
    189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
    192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
    194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
    198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
    202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
    207.3,
]


def _uk_yoy_capfloor_coupon_market():
    # Mirrors InflationCapFlooredCouponTests::CommonVars (2M lag, not the
    # uninitialized member used by C++ leg builders).
    calendar = ql.UnitedKingdom()
    evaluation = calendar.adjust(ql.Date(13, ql.Month.August, 2007))
    ql.set_evaluation_date(evaluation)
    nominal = ql.FlatForward(
        evaluation, 0.05, ql.ActualActual(ql.ActualActualConvention.ISDA)
    )

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
    return evaluation, nominal, yoy, observation_lag, calendar, bdc, dc, interp


def _yoy_schedule(evaluation, calendar, length_years=7):
    end = calendar.advance(
        evaluation, length_years, ql.TimeUnit.Years,
        ql.BusinessDayConvention.Unadjusted,
    )
    return ql.Schedule(
        evaluation,
        end,
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )


def test_capped_floored_coupon_type_and_strikes():
    (
        evaluation,
        nominal,
        yoy,
        lag,
        calendar,
        bdc,
        dc,
        interp,
    ) = _uk_yoy_capfloor_coupon_market()
    sched = _yoy_schedule(evaluation, calendar)
    vol = ql.ConstantYoYOptionletVolatility(
        0.01, 0, calendar, bdc, dc, lag, ql.Frequency.Annual,
    )
    leg = ql.make_yoy_inflation_leg(
        sched, calendar, yoy, lag, interp, dc,
        notional=1_000_000.0, payment_convention=bdc, cap=0.10,
    )
    assert isinstance(leg[0], ql.CappedFlooredYoYInflationCoupon)
    assert isinstance(leg[0], ql.YoYInflationCoupon)
    assert isinstance(leg[0], ql.CashFlow)
    assert leg[0].is_capped()
    assert not leg[0].is_floored()
    assert leg[0].cap() == pytest.approx(0.10)

    pricer = ql.BlackYoYInflationCouponPricer(nominal, vol)
    ql.set_yoy_coupon_pricer(leg, pricer)
    assert leg[0].rate() <= leg[0].underlying_rate() + 1.0e-12


def test_capped_leg_decomposition_vs_cap():
    # Mirrors InflationCapFlooredCouponTests::testDecomposition (g=1, s=0).
    (
        evaluation,
        nominal,
        yoy,
        lag,
        calendar,
        bdc,
        dc,
        interp,
    ) = _uk_yoy_capfloor_coupon_market()
    sched = _yoy_schedule(evaluation, calendar)
    vol = ql.ConstantYoYOptionletVolatility(
        0.01, 0, calendar, bdc, dc, lag, ql.Frequency.Annual,
    )
    pricer = ql.BlackYoYInflationCouponPricer(nominal, vol)
    cap_strike = 0.10
    floor_strike = 0.05

    vanilla = ql.make_yoy_inflation_leg(
        sched, calendar, yoy, lag, interp, dc,
        notional=1_000_000.0, payment_convention=bdc,
    )
    ql.set_yoy_coupon_pricer(vanilla, pricer)

    capped = ql.make_yoy_inflation_leg(
        sched, calendar, yoy, lag, interp, dc,
        notional=1_000_000.0, payment_convention=bdc, cap=cap_strike,
    )
    ql.set_yoy_coupon_pricer(capped, pricer)

    floored = ql.make_yoy_inflation_leg(
        sched, calendar, yoy, lag, interp, dc,
        notional=1_000_000.0, payment_convention=bdc, floor=floor_strike,
    )
    ql.set_yoy_coupon_pricer(floored, pricer)

    collared = ql.make_yoy_inflation_leg(
        sched, calendar, yoy, lag, interp, dc,
        notional=1_000_000.0, payment_convention=bdc,
        cap=cap_strike, floor=floor_strike,
    )
    ql.set_yoy_coupon_pricer(collared, pricer)

    settlement = evaluation
    npv_vanilla = ql.cashflows_npv(vanilla, nominal, settlement)
    npv_capped = ql.cashflows_npv(capped, nominal, settlement)
    npv_floored = ql.cashflows_npv(floored, nominal, settlement)
    npv_collared = ql.cashflows_npv(collared, nominal, settlement)

    cap = ql.YoYInflationCapFloor(
        ql.YoYInflationCapFloorType.Cap, vanilla, cap_strike,
    )
    cap.set_pricing_engine(yoy, vol, nominal, "black")
    floor = ql.YoYInflationCapFloor(
        ql.YoYInflationCapFloorType.Floor, vanilla, floor_strike,
    )
    floor.set_pricing_engine(yoy, vol, nominal, "black")
    collar = ql.YoYInflationCapFloor(
        ql.YoYInflationCapFloorType.Collar,
        vanilla,
        cap_strike,
        floor_strike,
    )
    collar.set_pricing_engine(yoy, vol, nominal, "black")

    tol = 1.0e-10
    assert npv_capped == pytest.approx(npv_vanilla - cap.NPV(), abs=tol)
    assert npv_floored == pytest.approx(npv_vanilla + floor.NPV(), abs=tol)
    assert npv_collared == pytest.approx(npv_vanilla - collar.NPV(), abs=tol)


def test_compat_phase20_aliases():
    import qlnb.compat as cql

    assert cql.CappedFlooredYoYInflationCoupon is not None
    assert hasattr(cql.CappedFlooredYoYInflationCoupon, "effectiveCap")
    assert hasattr(cql.CappedFlooredYoYInflationCoupon, "isCapped")
    assert hasattr(cql.CappedFlooredYoYInflationCoupon, "underlyingRate")
