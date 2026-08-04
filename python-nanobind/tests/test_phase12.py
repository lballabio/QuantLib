"""Phase-12 tests: zero inflation curves and ZCIS."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase12():
    assert ql.__version__ == "0.13.0"


_RPI_FIXINGS = [
    189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
    192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
    194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
    198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
    202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
    207.3,
]


def _uk_zero_inflation_market():
    # Mirrors test-suite/inflation.cpp::testZeroTermStructure.
    calendar = ql.UnitedKingdom()
    evaluation = calendar.adjust(ql.Date(13, ql.Month.August, 2007))
    ql.set_evaluation_date(evaluation)
    nominal = ql.FlatForward(evaluation, 0.05, ql.Actual360())

    hz = ql.RelinkableZeroInflationTermStructureHandle()
    index = ql.UKRPI(hz)

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
        index.add_fixing(d, fixing)

    zc_data = [
        (ql.Date(13, ql.Month.August, 2008), 0.0293),
        (ql.Date(13, ql.Month.August, 2009), 0.0295),
        (ql.Date(13, ql.Month.August, 2010), 0.02965),
        (ql.Date(15, ql.Month.August, 2011), 0.0298),
        (ql.Date(13, ql.Month.August, 2012), 0.0300),
    ]
    observation_lag = ql.Period(3, ql.TimeUnit.Months)
    dc = ql.Thirty360(ql.Thirty360Convention.BondBasis)
    bdc = ql.BusinessDayConvention.ModifiedFollowing
    interp = ql.CPIInterpolationType.Flat

    helpers = [
        ql.ZeroCouponInflationSwapHelper(
            ql.make_quote_handle(rate),
            observation_lag,
            maturity,
            calendar,
            bdc,
            dc,
            index,
            interp,
        )
        for maturity, rate in zc_data
    ]
    curve = ql.PiecewiseZeroInflationCurve(
        evaluation,
        index.last_fixing_date(),
        ql.Frequency.Monthly,
        dc,
        helpers,
    )
    hz.link_to(curve)
    return evaluation, nominal, index, curve, zc_data, observation_lag, calendar, bdc, dc, interp


def test_flat_zero_inflation_curve_constant_rate():
    today = ql.Date(15, ql.Month.May, 2007)
    ql.set_evaluation_date(today)
    base = ql.Date(1, ql.Month.January, 2007)
    end = ql.Date(1, ql.Month.January, 2012)
    curve = ql.FlatZeroInflationCurve(
        today,
        base,
        end,
        0.025,
        ql.Frequency.Monthly,
        ql.Actual365Fixed(),
    )
    assert not curve.empty()
    assert curve.zero_rate(ql.Date(1, ql.Month.January, 2009)) == pytest.approx(
        0.025, abs=1.0e-12
    )
    assert curve.frequency() == ql.Frequency.Monthly


def test_ukrpi_fixings_and_last_fixing_date():
    _evaluation, _nominal, index, curve, *_rest = _uk_zero_inflation_market()
    assert "RPI" in index.name()
    assert index.last_fixing_date() == ql.Date(1, ql.Month.July, 2007)
    assert index.fixing(ql.Date(1, ql.Month.July, 2007)) == pytest.approx(207.3)
    assert curve.base_date() == index.last_fixing_date()


def test_zcis_bootstrap_reprices_to_zero():
    (
        evaluation,
        nominal,
        index,
        _curve,
        zc_data,
        observation_lag,
        calendar,
        bdc,
        dc,
        interp,
    ) = _uk_zero_inflation_market()

    for maturity, rate in zc_data:
        zcis = ql.ZeroCouponInflationSwap(
            ql.SwapType.Payer,
            1_000_000.0,
            evaluation,
            maturity,
            calendar,
            bdc,
            dc,
            rate,
            index,
            observation_lag,
            interp,
        )
        zcis.set_pricing_engine(nominal)
        assert zcis.NPV() == pytest.approx(0.0, abs=1.0e-7)
        assert zcis.fair_rate() == pytest.approx(rate, abs=1.0e-7)
        assert zcis.fixed_rate() == rate
        assert zcis.maturity_date() == maturity


def test_euhicp_factory():
    idx = ql.EUHICP()
    assert "HICP" in idx.name()
    assert idx.frequency() == ql.Frequency.Monthly


def test_compat_phase12_aliases():
    import qlnb.compat as cql

    assert callable(cql.UKRPI)
    assert callable(cql.ZeroCouponInflationSwapHelper)
    assert hasattr(cql.ZeroCouponInflationSwap, "setPricingEngine")
    assert hasattr(cql.ZeroCouponInflationSwap, "fairRate")
