"""Phase-18 tests: CPI coupons and CPILeg."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase18():
    assert ql.__version__ == "0.19.0"


_RPI_FIXINGS = [
    206.1, 207.3, 208.0, 208.9, 209.7, 210.9,
    209.8, 211.4, 212.1, 214.0, 215.1, 216.8,
    216.5, 217.2, 218.4, 217.7, 216.0, 212.9,
    210.1, 211.4, 211.3, 211.5, 212.8, 213.4,
    213.4, 213.4, 214.4,
]


def _uk_cpi_coupon_market():
    # Mirrors InflationCPIBondTests CommonVars (flat 5% nominal).
    calendar = ql.UnitedKingdom()
    evaluation = calendar.adjust(ql.Date(25, ql.Month.November, 2009))
    ql.set_evaluation_date(evaluation)
    dc = ql.ActualActual(ql.ActualActualConvention.ISDA)
    nominal = ql.FlatForward(evaluation, 0.05, dc)

    hz = ql.RelinkableZeroInflationTermStructureHandle()
    index = ql.UKRPI(hz)
    # Drop leftover UKRPI history from other inflation tests (IndexManager is
    # process-global). A later fixing advances last_fixing_date past the
    # evaluation date and breaks the piecewise zero-inflation bootstrap.
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


def test_cpi_coupon_rate_matches_index_ratio():
    evaluation, nominal, index, calendar, bdc, interp = _uk_cpi_coupon_market()
    lag = ql.Period(3, ql.TimeUnit.Months)
    base_cpi = 206.1
    start = ql.Date(2, ql.Month.October, 2007)
    end = ql.Date(2, ql.Month.April, 2008)
    pay = calendar.adjust(end, bdc)
    cpn = ql.CPICoupon(
        base_cpi,
        pay,
        1_000_000.0,
        start,
        end,
        index,
        lag,
        interp,
        ql.Actual365Fixed(),
        0.1,
    )
    cpn.set_pricer(ql.CPICouponPricer(nominal))
    expected = 0.1 * (cpn.index_fixing() / base_cpi)
    assert cpn.rate() == pytest.approx(expected, abs=1.0e-10)
    assert cpn.fixed_rate() == pytest.approx(0.1)
    assert cpn.base_CPI() == pytest.approx(base_cpi)
    assert cpn.amount() == pytest.approx(
        cpn.rate() * cpn.nominal() * ql.Actual365Fixed().year_fraction(start, end),
        rel=1.0e-10,
    )


def test_cpi_leg_base_date_vs_base_cpi_clean_price():
    # Mirrors InflationCPIBondTests::testCPILegWithoutBaseCPI.
    evaluation, nominal, index, calendar, bdc, interp = _uk_cpi_coupon_market()
    contract_lag = ql.Period(3, ql.TimeUnit.Months)
    notional = 1_000_000.0
    fixed_rate = 0.1
    dc = ql.Actual365Fixed()
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

    leg_base_date = ql.make_cpi_leg(
        schedule,
        index,
        contract_lag,
        dc,
        base_date=ql.Date(1, ql.Month.July, 2007),
        notional=notional,
        fixed_rate=fixed_rate,
        payment_convention=bdc,
        payment_calendar=calendar,
        observation_interpolation=interp,
        subtract_inflation_nominal=False,
    )
    leg_base_cpi = ql.make_cpi_leg(
        schedule,
        index,
        contract_lag,
        dc,
        base_cpi=206.1,
        notional=notional,
        fixed_rate=fixed_rate,
        payment_convention=bdc,
        payment_calendar=calendar,
        observation_interpolation=interp,
        subtract_inflation_nominal=False,
    )
    assert len(leg_base_date) > 0
    assert len(leg_base_cpi) == len(leg_base_date)
    assert isinstance(leg_base_date[0], ql.CashFlow)
    assert isinstance(leg_base_date[0], ql.CPICoupon)

    pricer = ql.CPICouponPricer(nominal)
    ql.set_cpi_coupon_pricer(leg_base_date, pricer)
    ql.set_cpi_coupon_pricer(leg_base_cpi, pricer)

    settlement = calendar.advance(evaluation, 3, ql.TimeUnit.Days, bdc)
    npv_date = ql.cashflows_npv(leg_base_date, nominal, settlement)
    acc_date = ql.cashflows_accrued_amount(leg_base_date, settlement)
    npv_cpi = ql.cashflows_npv(leg_base_cpi, nominal, settlement)
    acc_cpi = ql.cashflows_accrued_amount(leg_base_cpi, settlement)

    clean_date = (npv_date - acc_date) * 100.0 / notional
    clean_cpi = (npv_cpi - acc_cpi) * 100.0 / notional
    assert clean_date == pytest.approx(clean_cpi, abs=1.0e-8)
    assert clean_date == pytest.approx(394.79676680, abs=1.0e-8)


def test_compat_phase18_aliases():
    import qlnb.compat as cql

    assert callable(cql.CPILeg)
    assert callable(cql.CPICouponPricer)
    assert hasattr(cql.CPICoupon, "setPricer")
    assert hasattr(cql.CPICoupon, "indexFixing")
    assert callable(cql.setCouponPricer)
    assert callable(cql.CashFlows_npv)
