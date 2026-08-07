"""Phase-26 tests: cash-or-nothing double-barrier binaries (Haug)."""

from __future__ import annotations

import math

import pytest

import qlnb as ql


def test_version_is_phase26():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 27)


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


def _binary_option(barrier_type, barrier_lo, barrier_hi, today, maturity, spot, q, r, vol):
    process = _bsm_process(today, spot, q, r, vol)
    payoff = ql.CashOrNothingPayoff(ql.OptionType.Call, 0.0, 10.0)
    if barrier_type in (ql.DoubleBarrierType.KIKO, ql.DoubleBarrierType.KOKI):
        exercise = ql.AmericanExercise(today, maturity)
    else:
        exercise = ql.EuropeanExercise(maturity)
    option = ql.DoubleBarrierOption(
        barrier_type, barrier_lo, barrier_hi, 0.0, payoff, exercise,
    )
    option.set_binary_pricing_engine(
        ql.AnalyticDoubleBarrierBinaryEngine(process)
    )
    return option


# Subset of Haug values from test-suite/doublebinaryoption.cpp (tol=1e-4).
# cash=10, q=0.02, r=0.05, t=0.25 unless noted.
_HAUG_CASES = [
    (ql.DoubleBarrierType.KnockOut, 80, 120, 0.10, 100.0, 9.8716),
    (ql.DoubleBarrierType.KnockOut, 80, 120, 0.20, 100.0, 8.9307),
    (ql.DoubleBarrierType.KnockOut, 90, 110, 0.20, 100.0, 3.6752),
    (ql.DoubleBarrierType.KnockOut, 95, 105, 0.10, 100.0, 3.6323),
    (ql.DoubleBarrierType.KnockIn, 80, 120, 0.20, 100.0, 0.9450),
    (ql.DoubleBarrierType.KnockIn, 90, 110, 0.20, 100.0, 6.2006),
    (ql.DoubleBarrierType.KnockIn, 95, 105, 0.10, 100.0, 6.2434),
    (ql.DoubleBarrierType.KIKO, 80, 120, 0.30, 100.0, 1.4076),
    (ql.DoubleBarrierType.KIKO, 90, 110, 0.20, 100.0, 2.7954),
    (ql.DoubleBarrierType.KIKO, 95, 105, 0.10, 100.0, 2.6285),
    (ql.DoubleBarrierType.KOKI, 80, 120, 0.30, 100.0, 2.1581),
    (ql.DoubleBarrierType.KOKI, 90, 110, 0.20, 100.0, 3.4424),
    (ql.DoubleBarrierType.KOKI, 95, 105, 0.10, 100.0, 3.6524),
    # Degenerate cases
    (ql.DoubleBarrierType.KnockOut, 95, 105, 0.10, 80.0, 0.0000),
    (ql.DoubleBarrierType.KnockIn, 95, 105, 0.10, 110.0, 10.0000),
    (ql.DoubleBarrierType.KIKO, 95, 105, 0.10, 80.0, 10.0000),
]


@pytest.mark.parametrize(
    "barrier_type,barrier_lo,barrier_hi,vol,spot,expected",
    _HAUG_CASES,
)
def test_double_binary_haug_values(
    barrier_type, barrier_lo, barrier_hi, vol, spot, expected
):
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    maturity = today + _time_to_days(0.25)
    option = _binary_option(
        barrier_type, barrier_lo, barrier_hi, today, maturity, spot, 0.02, 0.05, vol,
    )
    assert option.NPV() == pytest.approx(expected, abs=1.0e-4)


def test_cash_or_nothing_payoff_inspectors():
    payoff = ql.CashOrNothingPayoff(ql.OptionType.Call, 0.0, 10.0)
    assert payoff.option_type() == ql.OptionType.Call
    assert payoff.strike() == pytest.approx(0.0)
    assert payoff.cash_payoff() == pytest.approx(10.0)


def test_compat_phase26_aliases():
    import qlnb.compat as cql

    assert cql.CashOrNothingPayoff is not None
    assert hasattr(cql.CashOrNothingPayoff, "cashPayoff")
    assert hasattr(cql.DoubleBarrierOption, "setBinaryPricingEngine")
    assert cql.AnalyticDoubleBarrierBinaryEngine is not None
