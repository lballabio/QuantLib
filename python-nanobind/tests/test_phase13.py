"""Phase-13 tests: YoY inflation curves and YYIIS."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase13():
    assert ql.__version__ == "0.14.0"


_RPI_FIXINGS = [
    189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
    192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
    194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
    198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
    202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
    207.3,
]


def _uk_yoy_inflation_market():
    # Mirrors test-suite/inflation.cpp::testYYTermStructure.
    calendar = ql.UnitedKingdom()
    evaluation = calendar.adjust(ql.Date(13, ql.Month.August, 2007))
    ql.set_evaluation_date(evaluation)
    nominal = ql.FlatForward(evaluation, 0.05, ql.Actual360())

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
    dates = list(schedule.dates())
    assert len(dates) == len(_RPI_FIXINGS)
    for d, fixing in zip(dates, _RPI_FIXINGS):
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
    return (
        evaluation,
        nominal,
        rpi,
        yoy,
        curve,
        yy_data,
        observation_lag,
        calendar,
        bdc,
        dc,
        interp,
    )


def test_flat_yoy_inflation_curve_constant_rate():
    today = ql.Date(15, ql.Month.May, 2007)
    ql.set_evaluation_date(today)
    base = ql.Date(1, ql.Month.January, 2007)
    end = ql.Date(1, ql.Month.January, 2012)
    curve = ql.FlatYoYInflationCurve(
        today,
        base,
        end,
        0.027,
        ql.Frequency.Monthly,
        ql.Actual365Fixed(),
    )
    assert not curve.empty()
    assert curve.yoy_rate(ql.Date(1, ql.Month.January, 2009)) == pytest.approx(
        0.027, abs=1.0e-12
    )
    assert curve.frequency() == ql.Frequency.Monthly


def test_yyukrpi_and_yyeuhicp_factories():
    yy = ql.YYUKRPI()
    assert "YY_RPI" in yy.name() or "RPI" in yy.name()
    assert yy.frequency() == ql.Frequency.Monthly
    assert not yy.ratio()

    eu = ql.YYEUHICP()
    assert "HICP" in eu.name()
    assert not eu.ratio()


def test_ratio_yoy_index_from_zero():
    _evaluation, _nominal, rpi, yoy, *_rest = _uk_yoy_inflation_market()
    assert yoy.ratio()
    assert rpi.last_fixing_date() == ql.Date(1, ql.Month.July, 2007)
    assert rpi.fixing(ql.Date(1, ql.Month.July, 2007)) == pytest.approx(207.3)


def test_yyiis_bootstrap_reprices_to_zero():
    (
        evaluation,
        nominal,
        _rpi,
        yoy,
        _curve,
        yy_data,
        observation_lag,
        calendar,
        _bdc,
        dc,
        interp,
    ) = _uk_yoy_inflation_market()

    # Skip the first pillar (used as base YoY rate); reprice the rest.
    for maturity, rate in yy_data[1:]:
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
        assert yyiis.fair_rate() == pytest.approx(rate, abs=1.0e-7)
        assert yyiis.fixed_rate() == rate
        assert yyiis.maturity_date() == sched.end_date()


def test_compat_phase13_aliases():
    import qlnb.compat as cql

    assert callable(cql.YYUKRPI)
    assert callable(cql.YoYInflationIndex)
    assert callable(cql.YearOnYearInflationSwapHelper)
    assert hasattr(cql.YearOnYearInflationSwap, "setPricingEngine")
    assert hasattr(cql.YearOnYearInflationSwap, "fairRate")
    assert hasattr(cql.YearOnYearInflationSwap, "yoyLegNPV")
