"""Phase-14 tests: YoY inflation caps / floors."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase14():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 15)


_RPI_FIXINGS = [
    189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
    192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
    194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
    198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
    202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
    207.3,
]


def _uk_yoy_market():
    # Mirrors test-suite/inflationcapfloor.cpp CommonVars.
    calendar = ql.UnitedKingdom()
    evaluation = calendar.adjust(ql.Date(13, ql.Month.August, 2007))
    ql.set_evaluation_date(evaluation)
    nominal = ql.FlatForward(
        evaluation, 0.05, ql.ActualActual(ql.ActualActualConvention.ISDA)
    )

    hy = ql.RelinkableYoYInflationTermStructureHandle()
    rpi = ql.UKRPI()
    yoy = ql.make_yoy_inflation_index(rpi, hy)

    from_date = ql.Date(1, ql.Month.January, 2005)
    to_date = ql.Date(1, ql.Month.July, 2007)
    schedule = ql.Schedule(
        from_date,
        to_date,
        ql.Period(ql.Frequency.Monthly),
        ql.NullCalendar(),
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )
    for d, fixing in zip(list(schedule.dates()), _RPI_FIXINGS):
        rpi.add_fixing(d, fixing)

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
    return evaluation, nominal, yoy, observation_lag, calendar, dc, interp, bdc


def test_constant_yoy_optionlet_vol():
    _evaluation, _nominal, _yoy, lag, calendar, dc, _interp, bdc = _uk_yoy_market()
    vol = ql.ConstantYoYOptionletVolatility(
        0.01,
        0,
        calendar,
        bdc,
        dc,
        lag,
        ql.Frequency.Annual,
    )
    assert not vol.empty()
    assert vol.volatility(ql.Date(13, ql.Month.August, 2010), 0.03) == pytest.approx(
        0.01, abs=1.0e-12
    )


def test_cap_floor_collar_consistency():
    evaluation, nominal, yoy, lag, calendar, dc, interp, bdc = _uk_yoy_market()
    length = 5
    strike_cap = 0.03
    strike_floor = 0.025
    vol_level = 0.01

    end = calendar.advance(evaluation, length, ql.TimeUnit.Years)
    sched = ql.Schedule(
        evaluation,
        end,
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )
    vol = ql.ConstantYoYOptionletVolatility(
        vol_level, 0, calendar, bdc, dc, lag, ql.Frequency.Annual
    )

    cap = ql.YoYInflationCapFloor(
        ql.YoYInflationCapFloorType.Cap,
        sched,
        yoy,
        lag,
        interp,
        strike_cap,
        calendar,
        dc,
    )
    floor = ql.YoYInflationCapFloor(
        ql.YoYInflationCapFloorType.Floor,
        sched,
        yoy,
        lag,
        interp,
        strike_floor,
        calendar,
        dc,
    )
    collar = ql.YoYInflationCapFloor(
        ql.YoYInflationCapFloorType.Collar,
        sched,
        yoy,
        lag,
        interp,
        strike_cap,
        calendar,
        dc,
        floor_strike=strike_floor,
    )
    for instrument in (cap, floor, collar):
        instrument.set_pricing_engine(yoy, vol, nominal, model="black")

    assert cap.type() == ql.YoYInflationCapFloorType.Cap
    assert floor.type() == ql.YoYInflationCapFloorType.Floor
    assert collar.type() == ql.YoYInflationCapFloorType.Collar
    assert (cap.NPV() - floor.NPV()) == pytest.approx(collar.NPV(), abs=1.0e-6)
    assert cap.NPV() > 0.0
    assert floor.NPV() > 0.0


def test_cap_floor_parity_vs_yyiis():
    evaluation, nominal, yoy, lag, calendar, dc, interp, bdc = _uk_yoy_market()
    length = 5
    strike = 0.03
    vol_level = 0.01

    end = calendar.advance(evaluation, length, ql.TimeUnit.Years)
    sched = ql.Schedule(
        evaluation,
        end,
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Backward,
        False,
    )
    vol = ql.ConstantYoYOptionletVolatility(
        vol_level, 0, calendar, bdc, dc, lag, ql.Frequency.Annual
    )

    cap = ql.YoYInflationCapFloor(
        ql.YoYInflationCapFloorType.Cap,
        sched,
        yoy,
        lag,
        interp,
        strike,
        calendar,
        dc,
    )
    floor = ql.YoYInflationCapFloor(
        ql.YoYInflationCapFloorType.Floor,
        sched,
        yoy,
        lag,
        interp,
        strike,
        calendar,
        dc,
    )
    for instrument in (cap, floor):
        instrument.set_pricing_engine(yoy, vol, nominal, model="black")

    swap = ql.YearOnYearInflationSwap(
        ql.SwapType.Payer,
        1_000_000.0,
        sched,
        strike,
        dc,
        sched,
        yoy,
        lag,
        interp,
        0.0,
        dc,
        calendar,
    )
    swap.set_pricing_engine(nominal)
    assert (cap.NPV() - floor.NPV()) == pytest.approx(swap.NPV(), abs=1.0e-6)


@pytest.mark.parametrize("model", ["black", "unit_displaced_black", "bachelier"])
def test_engine_models_price_positive_cap(model):
    evaluation, nominal, yoy, lag, calendar, dc, interp, bdc = _uk_yoy_market()
    end = calendar.advance(evaluation, 5, ql.TimeUnit.Years)
    sched = ql.Schedule(
        evaluation,
        end,
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )
    vol = ql.ConstantYoYOptionletVolatility(
        0.015, 0, calendar, bdc, dc, lag, ql.Frequency.Annual
    )
    cap = ql.YoYInflationCapFloor(
        ql.YoYInflationCapFloorType.Cap,
        sched,
        yoy,
        lag,
        interp,
        0.025,
        calendar,
        dc,
    )
    cap.set_pricing_engine(yoy, vol, nominal, model=model)
    assert cap.NPV() > 0.0


def test_make_yoy_inflation_capfloor_helper():
    evaluation, nominal, yoy, lag, calendar, dc, interp, bdc = _uk_yoy_market()
    cap = ql.make_yoy_inflation_capfloor(
        ql.YoYInflationCapFloorType.Cap,
        yoy,
        5,
        calendar,
        lag,
        interp,
        0.03,
        nominal=1_000_000.0,
        effective_date=evaluation,
        day_counter=dc,
        payment_convention=bdc,
    )
    vol = ql.ConstantYoYOptionletVolatility(
        0.01, 0, calendar, bdc, dc, lag, ql.Frequency.Annual
    )
    cap.set_pricing_engine(yoy, vol, nominal, model="black")
    assert cap.type() == ql.YoYInflationCapFloorType.Cap
    assert cap.NPV() > 0.0
    assert not cap.is_expired()


def test_compat_phase14_aliases():
    import qlnb.compat as cql

    assert callable(cql.ConstantYoYOptionletVolatility)
    assert callable(cql.MakeYoYInflationCapFloor)
    assert hasattr(cql.YoYInflationCapFloor, "setPricingEngine")
    assert hasattr(cql.YoYInflationCapFloor, "atmRate")
