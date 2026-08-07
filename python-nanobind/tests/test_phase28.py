"""Phase-28 tests: partial-time continuous lookbacks (Haug 2006)."""

from __future__ import annotations

import math

import pytest

import qlnb as ql


def test_version_is_phase28():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 29)


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


# Haug 2006 p.146 — floating partial (t=1, q=0, r=0.06, lambda=1).
_PARTIAL_FLOATING = [
    # type, minmax(=spot), vol, t1, expected
    (ql.OptionType.Call, 90, 0.1, 0.25, 8.6524),
    (ql.OptionType.Call, 90, 0.1, 0.75, 9.5567),
    (ql.OptionType.Call, 110, 0.2, 0.50, 17.737),
    (ql.OptionType.Call, 110, 0.3, 0.75, 25.4825),
    (ql.OptionType.Put, 90, 0.1, 0.25, 2.7189),
    (ql.OptionType.Put, 110, 0.2, 0.50, 11.7119),
    (ql.OptionType.Put, 90, 0.3, 0.75, 18.4071),
    (ql.OptionType.Put, 110, 0.3, 0.25, 16.4657),
]


@pytest.mark.parametrize(
    "option_type,spot,vol,t1,expected",
    _PARTIAL_FLOATING,
)
def test_partial_floating_lookback_haug(option_type, spot, vol, t1, expected):
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    maturity = today + _time_to_days(1.0)
    lookback_end = today + _time_to_days(t1)
    process = _bsm_process(today, spot, 0.0, 0.06, vol)
    option = ql.ContinuousPartialFloatingLookbackOption(
        spot,
        1.0,
        lookback_end,
        ql.FloatingTypePayoff(option_type),
        ql.EuropeanExercise(maturity),
    )
    option.set_pricing_engine(
        ql.AnalyticContinuousPartialFloatingLookbackEngine(process)
    )
    assert option.NPV() == pytest.approx(expected, abs=1.0e-4)


# Haug 2006 p.148 — fixed partial (s=100, q=0, r=0.06, t=1).
_PARTIAL_FIXED = [
    # type, strike, vol, t1, expected
    (ql.OptionType.Call, 90, 0.1, 0.25, 20.2845),
    (ql.OptionType.Call, 90, 0.1, 0.75, 18.6244),
    (ql.OptionType.Call, 110, 0.2, 0.50, 10.8995),
    (ql.OptionType.Call, 110, 0.3, 0.75, 16.2976),
    (ql.OptionType.Put, 90, 0.1, 0.25, 0.4973),
    (ql.OptionType.Put, 110, 0.1, 0.50, 10.9492),
    (ql.OptionType.Put, 90, 0.2, 0.75, 3.5831),
    (ql.OptionType.Put, 110, 0.3, 0.25, 25.2112),
]


@pytest.mark.parametrize(
    "option_type,strike,vol,t1,expected",
    _PARTIAL_FIXED,
)
def test_partial_fixed_lookback_haug(option_type, strike, vol, t1, expected):
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    maturity = today + _time_to_days(1.0)
    lookback_start = today + _time_to_days(t1)
    process = _bsm_process(today, 100.0, 0.0, 0.06, vol)
    option = ql.ContinuousPartialFixedLookbackOption(
        lookback_start,
        ql.PlainVanillaPayoff(option_type, strike),
        ql.EuropeanExercise(maturity),
    )
    option.set_pricing_engine(process)
    assert option.NPV() == pytest.approx(expected, abs=1.0e-4)


def test_compat_phase28_aliases():
    import qlnb.compat as cql

    assert cql.ContinuousPartialFloatingLookbackOption is not None
    assert hasattr(
        cql.ContinuousPartialFloatingLookbackOption, "setPricingEngine"
    )
    assert cql.ContinuousPartialFixedLookbackOption is not None
    assert cql.AnalyticContinuousPartialFixedLookbackEngine is not None
