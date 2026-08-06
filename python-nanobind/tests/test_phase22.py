"""Phase-22 tests: YoY cap/floor term price surface → ATM."""

from __future__ import annotations

import math

import pytest

import qlnb as ql


def test_version_is_phase22():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 23)


def _eur_nominal_cubic(evaluation: ql.Date) -> ql.YieldTermStructureHandle:
    # Mirrors inflationvolatility.cpp setup() EUR InterpolatedZeroCurve<Cubic>.
    times = [
        0.0109589, 0.0684932, 0.263014, 0.317808, 0.567123, 0.816438,
        1.06575, 1.31507, 1.56438, 2.0137, 3.01918, 4.01644,
        5.01644, 6.01644, 7.01644, 8.01644, 9.02192, 10.0192,
        12.0192, 15.0247, 20.0301, 25.0356, 30.0329, 40.0384,
        50.0466,
    ]
    rates = [
        0.0415600, 0.0426840, 0.0470980, 0.0458506, 0.0449550, 0.0439784,
        0.0431887, 0.0426604, 0.0422925, 0.0424591, 0.0421477, 0.0421853,
        0.0424016, 0.0426969, 0.0430804, 0.0435011, 0.0439368, 0.0443825,
        0.0452589, 0.0463389, 0.0472636, 0.0473401, 0.0470629, 0.0461092,
        0.0450794,
    ]
    dates = []
    for t in times:
        ys = int(math.floor(t))
        ds = int((t - ys) * 365)
        dates.append(
            evaluation
            + ql.Period(ys, ql.TimeUnit.Years)
            + ql.Period(ds, ql.TimeUnit.Days)
        )
    return ql.InterpolatedZeroCurve(
        dates, rates, ql.Actual365Fixed(), "cubic"
    )


def _eu_yoy_price_surface():
    # Mirrors InflationVolTests::testYoYPriceSurfaceToATM / setupPriceSurface.
    evaluation = ql.Date(23, ql.Month.November, 2007)
    ql.set_evaluation_date(evaluation)
    nominal = _eur_nominal_cubic(evaluation)

    hy = ql.RelinkableYoYInflationTermStructureHandle()
    yoy = ql.make_yoy_inflation_index(ql.EUHICP(), hy)

    # Optional side YoY curve (same as C++ setup); surface bootstraps its own.
    yoy_rates = [
        0.0237951,
        0.0238749, 0.0240334, 0.0241934, 0.0243567, 0.0245323,
        0.0247213, 0.0249348, 0.0251768, 0.0254337, 0.0257258,
        0.0260217, 0.0263006, 0.0265538, 0.0267803, 0.0269378,
        0.0270608, 0.0271363, 0.0272, 0.0272512, 0.0272927,
        0.027317, 0.0273615, 0.0273811, 0.0274063, 0.0274307,
        0.0274625, 0.027527, 0.0275952, 0.0276734, 0.027794,
    ]
    base_date = ql.inflation_period(
        evaluation - ql.Period(1, ql.TimeUnit.Months), ql.Frequency.Monthly
    )[0]
    yoy_dates = [base_date]
    yoy_curve_rates = [yoy_rates[0]]
    cal = ql.TARGET()
    bdc = ql.BusinessDayConvention.ModifiedFollowing
    cap_start = cal.advance(evaluation, -2, ql.TimeUnit.Months, bdc)
    for i in range(1, len(yoy_rates)):
        yoy_dates.append(cal.advance(cap_start, i, ql.TimeUnit.Years, bdc))
        yoy_curve_rates.append(yoy_rates[i])
    hy.link_to(
        ql.InterpolatedYoYInflationCurve(
            evaluation,
            yoy_dates,
            yoy_curve_rates,
            ql.Frequency.Monthly,
            ql.Actual365Fixed(),
        )
    )

    cap_strikes = [0.02, 0.025, 0.03, 0.035, 0.04, 0.05]
    floor_strikes = [-0.01, 0.00, 0.005, 0.01, 0.015, 0.02]
    maturities = [
        ql.Period(3, ql.TimeUnit.Years),
        ql.Period(5, ql.TimeUnit.Years),
        ql.Period(7, ql.TimeUnit.Years),
        ql.Period(10, ql.TimeUnit.Years),
        ql.Period(15, ql.TimeUnit.Years),
        ql.Period(20, ql.TimeUnit.Years),
        ql.Period(30, ql.TimeUnit.Years),
    ]
    cap_table = [
        [116.225, 204.945, 296.285, 434.29, 654.47, 844.775, 1132.33],
        [34.305, 71.575, 114.1, 184.33, 307.595, 421.395, 602.35],
        [6.37, 19.085, 35.635, 66.42, 127.69, 189.685, 296.195],
        [1.325, 5.745, 12.585, 26.945, 58.95, 94.08, 158.985],
        [0.501, 2.37, 5.38, 13.065, 31.91, 53.95, 96.97],
        [0.501, 0.695, 1.47, 4.415, 12.86, 23.75, 46.7],
    ]
    floor_table = [
        [0.501, 0.851, 2.44, 6.645, 16.23, 26.85, 46.365],
        [0.501, 2.236, 5.555, 13.075, 28.46, 44.525, 73.08],
        [1.025, 3.935, 9.095, 19.64, 39.93, 60.375, 96.02],
        [2.465, 7.885, 16.155, 31.6, 59.34, 86.21, 132.045],
        [6.9, 17.92, 32.085, 56.08, 95.95, 132.85, 194.18],
        [23.52, 47.625, 74.085, 114.355, 175.72, 229.565, 316.285],
    ]
    n_mat = len(maturities)
    cap_prices = ql.Matrix(
        len(cap_strikes), n_mat, [x for row in cap_table for x in row]
    )
    floor_prices = ql.Matrix(
        len(floor_strikes), n_mat, [x for row in floor_table for x in row]
    )

    surface = ql.InterpolatedYoYCapFloorTermPriceSurface(
        0,
        ql.Period(3, ql.TimeUnit.Months),
        yoy,
        ql.CPIInterpolationType.Linear,
        nominal,
        ql.Actual365Fixed(),
        cal,
        bdc,
        cap_strikes,
        floor_strikes,
        maturities,
        cap_prices,
        floor_prices,
    )
    return surface, cap_strikes, floor_strikes, maturities


def test_yoy_price_surface_to_atm():
    surface, _caps, _floors, mats = _eu_yoy_price_surface()
    assert len(surface.maturities()) == 7
    assert surface.observation_lag() == ql.Period(3, ql.TimeUnit.Months)
    assert surface.cap_price(mats[0], 0.02) == pytest.approx(116.225, abs=1.0e-8)

    crv = [
        0.024586, 0.0247575, 0.0249396, 0.0252596,
        0.0258498, 0.0262883, 0.0267915,
    ]
    swaps = list(crv)
    ayoy = [
        0.0247659, 0.0251437, 0.0255945, 0.0265015,
        0.0280457, 0.0285534, 0.0295884,
    ]
    eps = 2.0e-5

    times, time_rates = surface.atm_yoy_swap_time_rates()
    assert len(time_rates) == len(crv)
    for got, exp in zip(time_rates, crv):
        assert got == pytest.approx(exp, abs=eps)

    dates, _date_rates = surface.atm_yoy_swap_date_rates()
    assert len(dates) == len(swaps)
    for d, exp in zip(dates, swaps):
        assert surface.atm_yoy_swap_rate(d) == pytest.approx(exp, abs=eps)
    for d, exp in zip(dates, ayoy):
        assert surface.atm_yoy_rate(d) == pytest.approx(exp, abs=eps)

    yoy_ts = surface.yoy_ts()
    assert not yoy_ts.empty()


def test_compat_phase22_aliases():
    import qlnb.compat as cql

    assert callable(cql.InterpolatedYoYCapFloorTermPriceSurface)
    assert cql.YoYCapFloorTermPriceSurfaceHandle is not None
    assert hasattr(cql.YoYCapFloorTermPriceSurfaceHandle, "atmYoYSwapRate")
    assert hasattr(cql.YoYCapFloorTermPriceSurfaceHandle, "atmYoYRate")
    assert hasattr(cql.YoYCapFloorTermPriceSurfaceHandle, "capPrice")
