"""Phase-21 tests: Indexed / CPI / ZeroInflation cash flows."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase21():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 22)


_RPI_FIXINGS = [
    206.1, 207.3, 208.0, 208.9, 209.7, 210.9,
    209.8, 211.4, 212.1, 214.0, 215.1, 216.8,
    216.5, 217.2, 218.4, 217.7, 216.0, 212.9,
    210.1, 211.4, 211.3, 211.5, 212.8, 213.4,
    213.4, 213.4, 214.4,
]


def _uk_cpi_cashflow_market():
    # Same UKRPI setup as phase-18 CPI coupon tests.
    calendar = ql.UnitedKingdom()
    evaluation = calendar.adjust(ql.Date(25, ql.Month.November, 2009))
    ql.set_evaluation_date(evaluation)
    dc = ql.ActualActual(ql.ActualActualConvention.ISDA)
    nominal = ql.FlatForward(evaluation, 0.05, dc)

    hz = ql.RelinkableZeroInflationTermStructureHandle()
    index = ql.UKRPI(hz)
    index.clear_fixings()

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
    return evaluation, nominal, index, calendar, bdc, interp


def test_cpi_leg_terminal_is_cpi_cashflow_bond_style():
    _evaluation, _nominal, index, calendar, bdc, interp = _uk_cpi_cashflow_market()
    contract_lag = ql.Period(3, ql.TimeUnit.Months)
    notional = 1_000_000.0
    base_cpi = 206.1
    dc = ql.Actual365Fixed()
    schedule = ql.Schedule(
        ql.Date(2, ql.Month.October, 2007),
        ql.Date(2, ql.Month.October, 2052),
        ql.Period(6, ql.TimeUnit.Months),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Backward,
        False,
    )
    leg = ql.make_cpi_leg(
        schedule,
        index,
        contract_lag,
        dc,
        base_cpi=base_cpi,
        notional=notional,
        fixed_rate=0.1,
        payment_convention=bdc,
        payment_calendar=calendar,
        observation_interpolation=interp,
        subtract_inflation_nominal=False,
    )
    terminal = leg[-1]
    assert isinstance(terminal, ql.CashFlow)
    assert isinstance(terminal, ql.IndexedCashFlow)
    assert isinstance(terminal, ql.CPICashFlow)
    assert not terminal.growth_only()
    assert terminal.notional() == pytest.approx(notional)
    assert terminal.base_fixing() == pytest.approx(base_cpi)
    expected = notional * terminal.index_fixing() / terminal.base_fixing()
    assert terminal.amount() == pytest.approx(expected, rel=1.0e-12)


def test_cpi_leg_terminal_growth_only_swap_style():
    _evaluation, _nominal, index, calendar, bdc, interp = _uk_cpi_cashflow_market()
    contract_lag = ql.Period(3, ql.TimeUnit.Months)
    notional = 1_000_000.0
    base_cpi = 206.1
    dc = ql.Actual365Fixed()
    schedule = ql.Schedule(
        ql.Date(2, ql.Month.October, 2007),
        ql.Date(2, ql.Month.October, 2052),
        ql.Period(6, ql.TimeUnit.Months),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Backward,
        False,
    )
    leg = ql.make_cpi_leg(
        schedule,
        index,
        contract_lag,
        dc,
        base_cpi=base_cpi,
        notional=notional,
        fixed_rate=0.1,
        payment_convention=bdc,
        payment_calendar=calendar,
        observation_interpolation=interp,
        subtract_inflation_nominal=True,
    )
    terminal = leg[-1]
    assert isinstance(terminal, ql.CPICashFlow)
    assert terminal.growth_only()
    ratio = terminal.index_fixing() / terminal.base_fixing()
    assert terminal.amount() == pytest.approx(
        notional * (ratio - 1.0), rel=1.0e-12
    )


def test_zero_inflation_cashflow_and_zcis_leg():
    # Fixing-date golden from InflationTest::testZeroTermStructure.
    calendar = ql.UnitedKingdom()
    evaluation = calendar.adjust(ql.Date(13, ql.Month.August, 2007))
    ql.set_evaluation_date(evaluation)
    nominal = ql.FlatForward(evaluation, 0.05, ql.Actual360())

    hz = ql.RelinkableZeroInflationTermStructureHandle()
    index = ql.UKRPI(hz)
    index.clear_fixings()

    rpi_fixings = [
        189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
        192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
        194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
        198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
        202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
        207.3,
    ]
    rpi_sched = ql.Schedule(
        ql.Date(1, ql.Month.January, 2005),
        ql.Date(1, ql.Month.July, 2007),
        ql.Period(ql.Frequency.Monthly),
        ql.NullCalendar(),
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )
    for d, fixing in zip(list(rpi_sched.dates()), rpi_fixings):
        index.add_fixing(d, fixing, force_overwrite=True)

    observation_lag = ql.Period(3, ql.TimeUnit.Months)
    dc = ql.Thirty360(ql.Thirty360Convention.BondBasis)
    bdc = ql.BusinessDayConvention.ModifiedFollowing
    interp = ql.CPIInterpolationType.Flat
    zc_data = [
        (ql.Date(13, ql.Month.August, 2008), 0.0293),
        (ql.Date(13, ql.Month.August, 2009), 0.0295),
        (ql.Date(13, ql.Month.August, 2010), 0.02965),
        (ql.Date(15, ql.Month.August, 2011), 0.0298),
        (ql.Date(13, ql.Month.August, 2012), 0.0300),
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

    maturity, rate = zc_data[0]
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
    infl_leg = zcis.inflation_leg()
    assert len(infl_leg) == 1
    cf = infl_leg[0]
    assert isinstance(cf, ql.ZeroInflationCashFlow)
    assert isinstance(cf, ql.IndexedCashFlow)
    assert cf.fixing_date() == ql.Date(13, ql.Month.May, 2008)
    assert cf.growth_only()
    ratio = cf.index_fixing() / cf.base_fixing()
    assert cf.amount() == pytest.approx(1_000_000.0 * (ratio - 1.0), rel=1.0e-12)

    # Standalone constructor path.
    standalone = ql.ZeroInflationCashFlow(
        1_000_000.0,
        index,
        interp,
        evaluation,
        maturity,
        observation_lag,
        maturity,
        True,
    )
    assert standalone.fixing_date() == ql.Date(13, ql.Month.May, 2008)
    assert standalone.amount() == pytest.approx(cf.amount(), rel=1.0e-12)


def test_compat_phase21_aliases():
    import qlnb.compat as cql

    assert cql.IndexedCashFlow is not None
    assert cql.CPICashFlow is not None
    assert cql.ZeroInflationCashFlow is not None
    assert hasattr(cql.CPICashFlow, "cpiIndex")
    assert hasattr(cql.CPICashFlow, "observationDate")
    assert hasattr(cql.ZeroInflationCashFlow, "zeroInflationIndex")
    assert hasattr(cql.ZeroCouponInflationSwap, "inflationLeg")
