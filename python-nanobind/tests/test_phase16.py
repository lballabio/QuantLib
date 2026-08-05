"""Phase-16 tests: CPICapFloor and term price surface."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase16():
    assert ql.__version__ == "0.17.0"


_RPI_FIXINGS = [
    206.1, 207.3, 208.0, 208.9, 209.7, 210.9,
    209.8, 211.4, 212.1, 214.0, 215.1, 216.8,
    216.5, 217.2, 218.4, 217.7, 216.0, 212.9,
    210.1, 211.4, 211.3, 211.5, 212.8, 213.4,
    213.4, 214.4, 215.3, 216.0, 216.6, 218.0,
    217.9, 219.2, 220.7, 222.8,
]

# Raw market tables from InflationCPICapFloorTests (bps by maturity × strike).
_C_PRICE_BPS = [
    [227.6, 100.27, 38.8, 14.94],
    [345.32, 127.9, 40.59, 14.11],
    [477.95, 170.19, 50.62, 16.88],
    [757.81, 303.95, 107.62, 43.61],
    [1140.73, 481.89, 168.4, 63.65],
    [1537.6, 607.72, 172.27, 54.87],
    [2211.67, 839.24, 184.75, 45.03],
]
_F_PRICE_BPS = [
    [15.62, 28.38, 53.61, 104.6],
    [21.45, 36.73, 66.66, 129.6],
    [24.45, 42.08, 77.04, 152.24],
    [39.25, 63.52, 109.2, 203.44],
    [36.82, 63.62, 116.97, 232.73],
    [39.7, 67.47, 121.79, 238.56],
    [41.48, 73.9, 139.75, 286.75],
]


def _matrix_from_maturity_strike(table_bps):
    """Build Matrix(n_strikes, n_maturities) in absolute price units."""
    n_mat = len(table_bps)
    n_strike = len(table_bps[0])
    data = []
    for i in range(n_strike):
        for j in range(n_mat):
            data.append(table_bps[j][i] / 10000.0)
    return ql.Matrix(n_strike, n_mat, data)


def _uk_cpi_capfloor_market():
    # Mirrors test-suite/inflationcpicapfloor.cpp CommonVars.
    calendar = ql.UnitedKingdom()
    evaluation = calendar.adjust(ql.Date(1, ql.Month.June, 2010))
    ql.set_evaluation_date(evaluation)
    dc = ql.ActualActual(ql.ActualActualConvention.ISDA)

    hz = ql.RelinkableZeroInflationTermStructureHandle()
    index = ql.UKRPI(hz)

    rpi_sched = ql.Schedule(
        ql.Date(1, ql.Month.July, 2007),
        ql.Date(1, ql.Month.April, 2010),
        ql.Period(ql.Frequency.Monthly),
        ql.NullCalendar(),
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )
    for d, fixing in zip(list(rpi_sched.dates()), _RPI_FIXINGS):
        index.add_fixing(d, fixing, force_overwrite=True)

    nominal_data = [
        (ql.Date(2, ql.Month.June, 2010), 0.00499997),
        (ql.Date(3, ql.Month.June, 2010), 0.00524992),
        (ql.Date(8, ql.Month.June, 2010), 0.00524974),
        (ql.Date(15, ql.Month.June, 2010), 0.00549942),
        (ql.Date(22, ql.Month.June, 2010), 0.00549913),
        (ql.Date(1, ql.Month.July, 2010), 0.00574864),
        (ql.Date(2, ql.Month.August, 2010), 0.00624668),
        (ql.Date(1, ql.Month.September, 2010), 0.00724338),
        (ql.Date(16, ql.Month.September, 2010), 0.00769461),
        (ql.Date(1, ql.Month.December, 2010), 0.00997501),
        (ql.Date(17, ql.Month.March, 2011), 0.00916996),
        (ql.Date(16, ql.Month.June, 2011), 0.00984339),
        (ql.Date(22, ql.Month.September, 2011), 0.0106085),
        (ql.Date(22, ql.Month.December, 2011), 0.01141788),
        (ql.Date(1, ql.Month.June, 2012), 0.01504426),
        (ql.Date(3, ql.Month.June, 2013), 0.0192064),
        (ql.Date(2, ql.Month.June, 2014), 0.02290824),
        (ql.Date(1, ql.Month.June, 2015), 0.02614394),
        (ql.Date(1, ql.Month.June, 2016), 0.02887445),
        (ql.Date(1, ql.Month.June, 2017), 0.03122128),
        (ql.Date(1, ql.Month.June, 2018), 0.03322511),
        (ql.Date(3, ql.Month.June, 2019), 0.03483997),
        (ql.Date(1, ql.Month.June, 2020), 0.03616896),
        (ql.Date(1, ql.Month.June, 2022), 0.038281),
        (ql.Date(2, ql.Month.June, 2025), 0.040341),
        (ql.Date(3, ql.Month.June, 2030), 0.04070854),
        (ql.Date(1, ql.Month.June, 2035), 0.04023202),
        (ql.Date(1, ql.Month.June, 2040), 0.03954748),
        (ql.Date(1, ql.Month.June, 2050), 0.03870953),
        (ql.Date(1, ql.Month.June, 2060), 0.0385298),
        (ql.Date(2, ql.Month.June, 2070), 0.03757542),
        (ql.Date(3, ql.Month.June, 2080), 0.03651379),
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
        (ql.Date(1, ql.Month.June, 2011), 0.03087),
        (ql.Date(1, ql.Month.June, 2012), 0.0312),
        (ql.Date(1, ql.Month.June, 2013), 0.03059),
        (ql.Date(1, ql.Month.June, 2014), 0.0311),
        (ql.Date(1, ql.Month.June, 2015), 0.0315),
        (ql.Date(1, ql.Month.June, 2016), 0.03207),
        (ql.Date(1, ql.Month.June, 2017), 0.03253),
        (ql.Date(1, ql.Month.June, 2018), 0.03288),
        (ql.Date(1, ql.Month.June, 2019), 0.03314),
        (ql.Date(1, ql.Month.June, 2020), 0.03401),
        (ql.Date(1, ql.Month.June, 2022), 0.03458),
        (ql.Date(1, ql.Month.June, 2025), 0.0352),
        (ql.Date(1, ql.Month.June, 2030), 0.03655),
        (ql.Date(1, ql.Month.June, 2035), 0.03668),
        (ql.Date(1, ql.Month.June, 2040), 0.03695),
        (ql.Date(1, ql.Month.June, 2050), 0.03634),
        (ql.Date(1, ql.Month.June, 2060), 0.03629),
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

    cap_strikes = [0.03, 0.04, 0.05, 0.06]
    floor_strikes = [-0.01, 0.0, 0.01, 0.02]
    maturities = [
        ql.Period(n, ql.TimeUnit.Years) for n in (3, 5, 7, 10, 15, 20, 30)
    ]
    cap_prices = _matrix_from_maturity_strike(_C_PRICE_BPS)
    floor_prices = _matrix_from_maturity_strike(_F_PRICE_BPS)
    base_zero_rate = zc_data[0][1]
    surface = ql.InterpolatedCPICapFloorTermPriceSurface(
        1.0,
        base_zero_rate,
        observation_lag,
        calendar,
        bdc,
        dc,
        index,
        interp,
        nominal,
        cap_strikes,
        floor_strikes,
        maturities,
        cap_prices,
        floor_prices,
    )
    return (
        evaluation,
        nominal,
        index,
        observation_lag,
        calendar,
        dc,
        interp,
        bdc,
        surface,
        cap_strikes,
        floor_strikes,
        maturities,
        cap_prices,
        floor_prices,
    )


def test_matrix_roundtrip():
    m = ql.Matrix(2, 3, [1.0, 2.0, 3.0, 4.0, 5.0, 6.0])
    assert m.rows() == 2
    assert m.columns() == 3
    assert m.at(0, 0) == pytest.approx(1.0)
    assert m.at(1, 2) == pytest.approx(6.0)


def test_cpi_capfloor_surface_reproduces_quotes():
    (
        _evaluation,
        _nominal,
        _index,
        _lag,
        _calendar,
        _dc,
        _interp,
        _bdc,
        surface,
        cap_strikes,
        floor_strikes,
        maturities,
        cap_prices,
        floor_prices,
    ) = _uk_cpi_capfloor_market()

    for i, k in enumerate(floor_strikes):
        for j, t in enumerate(maturities):
            assert surface.floor_price(t, k) == pytest.approx(
                floor_prices.at(i, j), abs=1.0e-7
            )
    for i, k in enumerate(cap_strikes):
        for j, t in enumerate(maturities):
            assert surface.cap_price(t, k) == pytest.approx(
                cap_prices.at(i, j), abs=1.0e-7
            )

    # 3Y / 1% is below ATM → floor premium 53.61 bps.
    assert surface.price(ql.Period(3, ql.TimeUnit.Years), 0.01) == pytest.approx(
        floor_prices.at(2, 0), abs=1.0e-12
    )


def test_cpi_capfloor_engine_matches_surface():
    (
        evaluation,
        _nominal,
        index,
        lag,
        calendar,
        _dc,
        _interp,
        bdc,
        surface,
        _cap_strikes,
        _floor_strikes,
        _maturities,
        cap_prices,
        _floor_prices,
    ) = _uk_cpi_capfloor_market()

    maturity = evaluation + ql.Period(3, ql.TimeUnit.Years)
    obs_interp = ql.CPIInterpolationType.Linear
    base_cpi = ql.cpi_lagged_fixing(index, evaluation, lag, obs_interp)
    cap = ql.CPICapFloor(
        ql.OptionType.Call,
        1.0,
        evaluation,
        base_cpi,
        maturity,
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        calendar,
        bdc,
        0.03,
        index,
        lag,
        obs_interp,
    )
    cap.set_pricing_engine(surface)
    cached = cap_prices.at(0, 0)  # 227.6 bps
    assert cap.NPV() == pytest.approx(cached, abs=1.0e-10)
    assert cap.type() == ql.OptionType.Call
    assert cap.strike() == pytest.approx(0.03)
    assert not cap.is_expired()


def test_compat_phase16_aliases():
    import qlnb.compat as cql

    assert cql.Matrix is not None
    assert callable(cql.InterpolatedCPICapFloorTermPriceSurface)
    assert hasattr(cql.CPICapFloor, "setPricingEngine")
    assert hasattr(cql.CPICapFloor, "fixingDate")
    assert hasattr(cql.CPICapFloorTermPriceSurfaceHandle, "capPrice")
