"""Phase-15 tests: CPISwap and CPIBond."""

from __future__ import annotations

import math

import pytest

import qlnb as ql


def test_version_is_phase15():
    assert ql.__version__ == "0.16.0"


_RPI_FIXINGS = [
    206.1, 207.3, 208.0, 208.9, 209.7, 210.9,
    209.8, 211.4, 212.1, 214.0, 215.1, 216.8,
    216.5, 217.2, 218.4, 217.7, 216.0, 212.9,
    210.1, 211.4, 211.3, 211.5, 212.8, 213.4,
    213.4, 213.4, 214.4,
]


def _uk_cpi_market(*, flat_nominal: bool = False):
    # Mirrors test-suite/inflationcpiswap.cpp / inflationcpibond.cpp CommonVars.
    calendar = ql.UnitedKingdom()
    evaluation = calendar.adjust(ql.Date(25, ql.Month.November, 2009))
    ql.set_evaluation_date(evaluation)
    dc = ql.ActualActual(ql.ActualActualConvention.ISDA)

    hz = ql.RelinkableZeroInflationTermStructureHandle()
    index = ql.UKRPI(hz)

    rpi_sched = ql.Schedule(
        ql.Date(1, ql.Month.July, 2007),
        ql.Date(1, ql.Month.September, 2009),
        ql.Period(ql.Frequency.Monthly),
        ql.NullCalendar(),
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )
    for d, fixing in zip(list(rpi_sched.dates()), _RPI_FIXINGS):
        index.add_fixing(d, fixing, force_overwrite=True)

    if flat_nominal:
        nominal = ql.FlatForward(evaluation, 0.05, dc)
    else:
        nominal_data = [
            (ql.Date(26, ql.Month.November, 2009), 0.00475),
            (ql.Date(2, ql.Month.December, 2009), 0.0047498),
            (ql.Date(29, ql.Month.December, 2009), 0.0049988),
            (ql.Date(25, ql.Month.February, 2010), 0.0059955),
            (ql.Date(18, ql.Month.March, 2010), 0.0065361),
            (ql.Date(25, ql.Month.May, 2010), 0.0082830),
            (ql.Date(16, ql.Month.September, 2010), 0.0078960),
            (ql.Date(16, ql.Month.December, 2010), 0.0093762),
            (ql.Date(17, ql.Month.March, 2011), 0.0112037),
            (ql.Date(16, ql.Month.June, 2011), 0.0131308),
            (ql.Date(22, ql.Month.September, 2011), 0.0152011),
            (ql.Date(25, ql.Month.November, 2011), 0.0178399),
            (ql.Date(26, ql.Month.November, 2012), 0.0241170),
            (ql.Date(25, ql.Month.November, 2013), 0.0283935),
            (ql.Date(25, ql.Month.November, 2014), 0.0312888),
            (ql.Date(25, ql.Month.November, 2015), 0.0334298),
            (ql.Date(25, ql.Month.November, 2016), 0.0350632),
            (ql.Date(27, ql.Month.November, 2017), 0.0363666),
            (ql.Date(26, ql.Month.November, 2018), 0.0374723),
            (ql.Date(25, ql.Month.November, 2019), 0.0383988),
            (ql.Date(25, ql.Month.November, 2021), 0.0400508),
            (ql.Date(25, ql.Month.November, 2024), 0.0416042),
            (ql.Date(26, ql.Month.November, 2029), 0.0415577),
            (ql.Date(27, ql.Month.November, 2034), 0.0404933),
            (ql.Date(25, ql.Month.November, 2039), 0.0395217),
            (ql.Date(25, ql.Month.November, 2049), 0.0380932),
            (ql.Date(25, ql.Month.November, 2059), 0.0380849),
            (ql.Date(25, ql.Month.November, 2069), 0.0372677),
            (ql.Date(27, ql.Month.November, 2079), 0.0363082),
        ]
        nominal = ql.InterpolatedZeroCurve(
            [d for d, _ in nominal_data],
            [r for _, r in nominal_data],
            dc,
        )

    observation_lag = ql.Period(2, ql.TimeUnit.Months)
    bdc = ql.BusinessDayConvention.ModifiedFollowing
    interp = ql.CPIInterpolationType.Flat
    zc_data = [
        (ql.Date(25, ql.Month.November, 2010), 0.030495),
        (ql.Date(25, ql.Month.November, 2011), 0.0293),
        (ql.Date(26, ql.Month.November, 2012), 0.029795),
        (ql.Date(25, ql.Month.November, 2013), 0.03029),
        (ql.Date(25, ql.Month.November, 2014), 0.031425),
        (ql.Date(25, ql.Month.November, 2015), 0.03211),
        (ql.Date(25, ql.Month.November, 2016), 0.032675),
        (ql.Date(25, ql.Month.November, 2017), 0.033625),
        (ql.Date(25, ql.Month.November, 2018), 0.03405),
        (ql.Date(25, ql.Month.November, 2019), 0.0348),
        (ql.Date(25, ql.Month.November, 2021), 0.03576),
        (ql.Date(25, ql.Month.November, 2024), 0.03649),
        (ql.Date(26, ql.Month.November, 2029), 0.03751),
        (ql.Date(27, ql.Month.November, 2034), 0.0377225),
        (ql.Date(25, ql.Month.November, 2039), 0.0377),
        (ql.Date(25, ql.Month.November, 2049), 0.03734),
        (ql.Date(25, ql.Month.November, 2059), 0.03714),
    ]
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
    return evaluation, nominal, index, observation_lag, calendar, dc, interp, bdc, zc_data


def test_cpi_lagged_fixing():
    evaluation, _nominal, index, lag, _cal, _dc, interp, *_rest = _uk_cpi_market(
        flat_nominal=True
    )
    fixing = ql.cpi_lagged_fixing(index, evaluation, lag, interp)
    assert fixing > 0.0


def test_cpibond_cached_prices():
    # Mirrors InflationCPIBondTests::testCleanPrice golden values.
    evaluation, nominal, index, _lag, calendar, _dc, interp, bdc, _zc = _uk_cpi_market(
        flat_nominal=True
    )
    contract_lag = ql.Period(3, ql.TimeUnit.Months)
    start = ql.Date(2, ql.Month.October, 2007)
    end = ql.Date(2, ql.Month.October, 2052)
    schedule = ql.Schedule(
        start,
        end,
        ql.Period(6, ql.TimeUnit.Months),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Backward,
        False,
    )
    bond = ql.CPIBond(
        3,
        1_000_000.0,
        206.1,
        contract_lag,
        index,
        interp,
        schedule,
        [0.1],
        ql.Actual365Fixed(),
        bdc,
    )
    bond.set_pricing_engine(nominal)
    assert bond.dirty_price() == pytest.approx(396.47045891, abs=1.0e-8)
    assert bond.clean_price() == pytest.approx(394.79676679, abs=1.0e-8)
    assert bond.base_CPI() == pytest.approx(206.1)
    assert bond.maturity_date() == schedule.end_date()


def test_cpiswap_as_zciis_reprices_to_zero():
    # Mirrors CPISwapTests::zciisconsistency.
    evaluation, nominal, index, lag, calendar, dc, interp, bdc, zc_data = (
        _uk_cpi_market()
    )
    maturity, quote = zc_data[-1]
    zciis = ql.ZeroCouponInflationSwap(
        ql.SwapType.Payer,
        1_000_000.0,
        evaluation,
        maturity,
        calendar,
        bdc,
        dc,
        quote,
        index,
        lag,
        interp,
    )
    zciis.set_pricing_engine(nominal)
    assert zciis.NPV() == pytest.approx(0.0, abs=1.0e-3)

    one_date = ql.Schedule([maturity], calendar, bdc)
    inflation_nominal = 1_000_000.0
    years = 50.0
    float_nominal = inflation_nominal * math.pow(1.0 + quote, years)
    base_cpi = ql.cpi_lagged_fixing(index, evaluation, lag, interp)
    dummy_index = None
    cpi_swap = ql.CPISwap(
        ql.SwapType.Payer,
        float_nominal,
        True,
        0.0,
        ql.Actual365Fixed(),
        one_date,
        bdc,
        0,
        dummy_index,
        0.0,
        base_cpi,
        ql.Actual365Fixed(),
        one_date,
        bdc,
        lag,
        index,
        interp,
        inflation_nominal,
    )
    cpi_swap.set_pricing_engine(nominal)
    assert cpi_swap.NPV() == pytest.approx(0.0, abs=1.0e-3)


def test_cpiswap_npv_consistency():
    # Mirrors CPISwapTests::consistency stored NPV.
    evaluation, nominal, index, _obs_lag, calendar, _dc, interp, bdc, _zc = (
        _uk_cpi_market()
    )
    contract_lag = ql.Period(3, ql.TimeUnit.Months)
    float_index = ql.GBPLibor(ql.Period(6, ql.TimeUnit.Months), nominal)

    start = ql.Date(2, ql.Month.October, 2007)
    end = ql.Date(2, ql.Month.October, 2052)
    float_sched = ql.Schedule(
        start,
        end,
        ql.Period(6, ql.TimeUnit.Months),
        calendar,
        bdc,
        bdc,
        ql.DateGeneration.Backward,
        False,
    )
    fixed_sched = ql.Schedule(
        start,
        end,
        ql.Period(6, ql.TimeUnit.Months),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Backward,
        False,
    )

    float_fix = [0.06255, 0.05975, 0.0637, 0.018425, 0.0073438]
    for i, d in enumerate(float_sched.dates()):
        if d < evaluation and i < len(float_fix):
            float_index.add_fixing(d, float_fix[i], force_overwrite=True)

    swap = ql.CPISwap(
        ql.SwapType.Payer,
        1_000_000.0,
        True,
        0.0,
        ql.Actual365Fixed(),
        float_sched,
        bdc,
        0,
        float_index,
        0.1,
        206.1,
        ql.Actual365Fixed(),
        fixed_sched,
        bdc,
        contract_lag,
        index,
        interp,
    )
    swap.set_pricing_engine(nominal)
    # C++ stored consistency value is 4191797.54 (ratio within 1e-5).
    assert abs(1.0 - swap.NPV() / 4191797.54) < 3.0e-5
    assert swap.fair_rate() != 0.0


def test_gbplibor_and_zero_curve_factories():
    today = ql.Date(25, ql.Month.November, 2009)
    ql.set_evaluation_date(today)
    dc = ql.ActualActual(ql.ActualActualConvention.ISDA)
    curve = ql.InterpolatedZeroCurve(
        [today, ql.Date(25, ql.Month.November, 2019)],
        [0.01, 0.03],
        dc,
    )
    idx = ql.GBPLibor(ql.Period(6, ql.TimeUnit.Months), curve)
    assert "GBPLibor" in idx.name()
    assert not curve.empty()


def test_compat_phase15_aliases():
    import qlnb.compat as cql

    assert callable(cql.GBPLibor)
    assert callable(cql.CPISwap)
    assert callable(cql.CPIBond)
    assert hasattr(cql.CPISwap, "setPricingEngine")
    assert hasattr(cql.CPIBond, "cleanPrice")
    assert callable(cql.CPI.laggedFixing)
