"""Phase-25 tests: double-barrier options (Haug / AnalyticDoubleBarrierEngine)."""

from __future__ import annotations

import math

import pytest

import qlnb as ql


def test_version_is_phase25():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 26)


def _bsm_process(today: ql.Date, spot: float, q: float, r: float, vol: float):
    dc = ql.Actual360()
    return ql.BlackScholesMertonProcess(
        ql.make_quote_handle(spot),
        ql.FlatForward(today, q, dc),
        ql.FlatForward(today, r, dc),
        ql.BlackConstantVol(today, ql.TARGET(), vol, dc),
    )


def _time_to_days(t: float) -> int:
    # Mirrors test-suite/utilities.hpp timeToDays(t) with dayPerYear=360.
    return int(math.floor(t * 360 + 0.5))


# Subset of Haug values from test-suite/doublebarrieroption.cpp
# (spot=100, strike=100, q=0, r=0.1, rebate=0; tol=1e-4).
_HAUG_CASES = [
    # KnockOut Call
    (ql.DoubleBarrierType.KnockOut, ql.OptionType.Call, 50, 150, 0.25, 0.15, 4.3515),
    (ql.DoubleBarrierType.KnockOut, ql.OptionType.Call, 50, 150, 0.25, 0.25, 6.1644),
    (ql.DoubleBarrierType.KnockOut, ql.OptionType.Call, 80, 120, 0.25, 0.25, 2.6387),
    (ql.DoubleBarrierType.KnockOut, ql.OptionType.Call, 90, 110, 0.50, 0.15, 0.5537),
    # KnockOut Put
    (ql.DoubleBarrierType.KnockOut, ql.OptionType.Put, 50, 150, 0.25, 0.15, 1.8825),
    (ql.DoubleBarrierType.KnockOut, ql.OptionType.Put, 80, 120, 0.25, 0.25, 2.6866),
    (ql.DoubleBarrierType.KnockOut, ql.OptionType.Put, 90, 110, 0.50, 0.35, 0.0013),
    # KnockIn Call
    (ql.DoubleBarrierType.KnockIn, ql.OptionType.Call, 50, 150, 0.25, 0.15, 0.0000),
    (ql.DoubleBarrierType.KnockIn, ql.OptionType.Call, 70, 130, 0.50, 0.25, 5.5818),
    (ql.DoubleBarrierType.KnockIn, ql.OptionType.Call, 90, 110, 0.50, 0.35, 12.2398),
]


@pytest.mark.parametrize(
    "barrier_type,option_type,barrier_lo,barrier_hi,t,vol,expected",
    _HAUG_CASES,
)
def test_double_barrier_haug_values(
    barrier_type, option_type, barrier_lo, barrier_hi, t, vol, expected
):
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    maturity = today + _time_to_days(t)
    process = _bsm_process(today, 100.0, 0.0, 0.1, vol)
    option = ql.DoubleBarrierOption(
        barrier_type,
        barrier_lo,
        barrier_hi,
        0.0,
        ql.PlainVanillaPayoff(option_type, 100.0),
        ql.EuropeanExercise(maturity),
    )
    option.set_pricing_engine(ql.AnalyticDoubleBarrierEngine(process))
    assert option.NPV() == pytest.approx(expected, abs=1.0e-4)


def test_double_barrier_set_pricing_engine_process_alias():
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    maturity = today + _time_to_days(0.25)
    process = _bsm_process(today, 100.0, 0.0, 0.1, 0.15)
    option = ql.DoubleBarrierOption(
        ql.DoubleBarrierType.KnockOut,
        50.0,
        150.0,
        0.0,
        ql.PlainVanillaPayoff(ql.OptionType.Call, 100.0),
        ql.EuropeanExercise(maturity),
    )
    option.set_pricing_engine(process)
    assert option.NPV() == pytest.approx(4.3515, abs=1.0e-4)


def test_compat_phase25_aliases():
    import qlnb.compat as cql

    assert cql.DoubleBarrierOption is not None
    assert hasattr(cql.DoubleBarrierOption, "setPricingEngine")
    assert cql.DoubleBarrierType is not None
    assert cql.AnalyticDoubleBarrierEngine is not None
