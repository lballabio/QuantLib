"""Phase-27 tests: continuous fixed / floating lookback options (Haug)."""

from __future__ import annotations

import math

import pytest

import qlnb as ql


def test_version_is_phase27():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 28)


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


_FLOATING_CASES = [
    # type, minmax, s, q, r, t, vol, expected
    (ql.OptionType.Call, 100, 120.0, 0.06, 0.10, 0.50, 0.30, 25.3533),
    (ql.OptionType.Call, 100, 100.0, 0.00, 0.05, 1.00, 0.30, 23.7884),
    (ql.OptionType.Call, 100, 100.0, 0.00, 0.05, 0.20, 0.30, 10.7190),
    (ql.OptionType.Call, 100, 110.0, 0.00, 0.05, 0.20, 0.30, 14.4597),
    (ql.OptionType.Put, 100, 100.0, 0.00, 0.10, 0.50, 0.30, 15.3526),
    (ql.OptionType.Put, 120, 100.0, 0.00, 0.10, 0.50, 0.30, 21.0645),
]


@pytest.mark.parametrize(
    "option_type,minmax,spot,q,r,t,vol,expected",
    _FLOATING_CASES,
)
def test_continuous_floating_lookback_haug(
    option_type, minmax, spot, q, r, t, vol, expected
):
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    maturity = today + _time_to_days(t)
    process = _bsm_process(today, spot, q, r, vol)
    option = ql.ContinuousFloatingLookbackOption(
        minmax,
        ql.FloatingTypePayoff(option_type),
        ql.EuropeanExercise(maturity),
    )
    option.set_pricing_engine(
        ql.AnalyticContinuousFloatingLookbackEngine(process)
    )
    assert option.NPV() == pytest.approx(expected, abs=1.0e-4)


_FIXED_CASES = [
    # type, strike, t, vol, expected  (s=100, q=0, r=0.10, minmax=100)
    (ql.OptionType.Call, 95, 0.50, 0.10, 13.2687),
    (ql.OptionType.Call, 100, 0.50, 0.20, 14.1702),
    (ql.OptionType.Call, 105, 0.50, 0.30, 15.8512),
    (ql.OptionType.Call, 95, 1.00, 0.30, 34.7116),
    (ql.OptionType.Put, 100, 0.50, 0.20, 8.3177),
    (ql.OptionType.Put, 105, 1.00, 0.30, 20.9130),
]


@pytest.mark.parametrize(
    "option_type,strike,t,vol,expected",
    _FIXED_CASES,
)
def test_continuous_fixed_lookback_haug(option_type, strike, t, vol, expected):
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    maturity = today + _time_to_days(t)
    process = _bsm_process(today, 100.0, 0.0, 0.10, vol)
    option = ql.ContinuousFixedLookbackOption(
        100.0,
        ql.PlainVanillaPayoff(option_type, strike),
        ql.EuropeanExercise(maturity),
    )
    option.set_pricing_engine(process)
    assert option.NPV() == pytest.approx(expected, abs=1.0e-4)


def test_floating_type_payoff_inspector():
    payoff = ql.FloatingTypePayoff(ql.OptionType.Put)
    assert payoff.option_type() == ql.OptionType.Put


def test_compat_phase27_aliases():
    import qlnb.compat as cql

    assert cql.FloatingTypePayoff is not None
    assert hasattr(cql.FloatingTypePayoff, "optionType")
    assert cql.ContinuousFloatingLookbackOption is not None
    assert hasattr(cql.ContinuousFloatingLookbackOption, "setPricingEngine")
    assert cql.ContinuousFixedLookbackOption is not None
    assert cql.AnalyticContinuousFixedLookbackEngine is not None
