"""Phase-29 tests: soft barrier options (Haug p.166 / AnalyticSoftBarrierEngine)."""

from __future__ import annotations

import math

import pytest

import qlnb as ql


def test_version_is_phase29():
    assert ql.__version__ == "0.30.0"


def _bsm_process(today: ql.Date, spot: float, q: float, r: float, vol: float):
    dc = ql.Actual360()
    return ql.BlackScholesMertonProcess(
        ql.make_quote_handle(spot),
        ql.FlatForward(today, q, dc),
        ql.FlatForward(today, r, dc),
        ql.BlackConstantVol(today, ql.TARGET(), vol, dc),
    )


def _time_to_days(t: float) -> int:
    return int(math.floor(t * 360 + 0.5))


# Subset of Haug p.166 DownOut calls from test-suite/softbarrieroption.cpp.
# S=X=100, U=95, q=0.05, r=0.1; barrier_lo=L, barrier_hi=U.
_HAUG_CASES = [
    # L, t, vol, expected
    (95, 0.5, 0.1, 3.8075),
    (90, 0.5, 0.1, 4.0175),
    (50, 0.5, 0.1, 4.0808),
    (95, 0.5, 0.2, 4.5263),
    (80, 0.5, 0.2, 6.2594),
    (50, 0.5, 0.3, 8.8751),
    (95, 1.0, 0.1, 5.4187),
    (70, 1.0, 0.2, 9.0931),
    (95, 1.0, 0.3, 5.2300),
    (50, 1.0, 0.3, 12.2036),
]


@pytest.mark.parametrize("barrier_lo,t,vol,expected", _HAUG_CASES)
def test_soft_barrier_haug_down_out_call(barrier_lo, t, vol, expected):
    # Mirrors SoftBarrierOptionTests::testSoftBarrierHaug evaluation date.
    today = ql.Date(8, ql.Month.August, 2025)
    ql.set_evaluation_date(today)
    maturity = today + _time_to_days(t)
    process = _bsm_process(today, 100.0, 0.05, 0.1, vol)
    option = ql.SoftBarrierOption(
        ql.BarrierType.DownOut,
        barrier_lo,
        95.0,
        ql.PlainVanillaPayoff(ql.OptionType.Call, 100.0),
        ql.EuropeanExercise(maturity),
    )
    option.set_pricing_engine(ql.AnalyticSoftBarrierEngine(process))
    assert option.NPV() == pytest.approx(expected, abs=1.0e-4)


def test_soft_barrier_set_pricing_engine_process_alias():
    today = ql.Date(8, ql.Month.August, 2025)
    ql.set_evaluation_date(today)
    maturity = today + _time_to_days(0.5)
    process = _bsm_process(today, 100.0, 0.05, 0.1, 0.1)
    option = ql.SoftBarrierOption(
        ql.BarrierType.DownOut,
        95.0,
        95.0,
        ql.PlainVanillaPayoff(ql.OptionType.Call, 100.0),
        ql.EuropeanExercise(maturity),
    )
    option.set_pricing_engine(process)
    assert option.NPV() == pytest.approx(3.8075, abs=1.0e-4)


def test_compat_phase29_aliases():
    import qlnb.compat as cql

    assert cql.SoftBarrierOption is not None
    assert hasattr(cql.SoftBarrierOption, "setPricingEngine")
    assert cql.AnalyticSoftBarrierEngine is not None
