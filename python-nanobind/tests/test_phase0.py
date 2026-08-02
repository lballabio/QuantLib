"""Phase-0 correctness tests for qlnb."""

from __future__ import annotations

import math

import pytest

import qlnb as ql


# EquityOption example (Black-Scholes European put) reference NPV.
EQUITY_OPTION_BS_PUT_NPV = 3.844307791471182


def test_date_roundtrip():
    d = ql.Date(15, ql.Month.May, 1998)
    assert d.day_of_month() == 15
    assert d.month() == ql.Month.May
    assert d.year() == 1998
    assert d.serial_number() > 0
    assert (d + 2).day_of_month() == 17


def test_settings_evaluation_date():
    d = ql.Date(17, ql.Month.May, 1998)
    ql.set_evaluation_date(d)
    assert ql.get_evaluation_date() == d

    settings = ql.Settings.instance()
    settings.evaluation_date = ql.Date(1, ql.Month.June, 1998)
    assert settings.evaluation_date == ql.Date(1, ql.Month.June, 1998)


def test_simple_quote_and_handle():
    q = ql.SimpleQuote(36.0)
    assert q.is_valid()
    assert q.value() == pytest.approx(36.0)

    h = ql.QuoteHandle(q)
    assert not h.empty()
    assert h.current_link().value() == pytest.approx(36.0)

    q.set_value(37.5)
    assert h.current_link().value() == pytest.approx(37.5)

    rh = ql.make_quote_handle(10.0)
    assert rh.current_link().value() == pytest.approx(10.0)


def test_flat_forward_discount():
    ref = ql.Date(17, ql.Month.May, 1998)
    ql.set_evaluation_date(ql.Date(15, ql.Month.May, 1998))
    dc = ql.Actual365Fixed()
    curve = ql.FlatForward(ref, 0.06, dc)
    t = dc.year_fraction(ref, ql.Date(17, ql.Month.May, 1999))
    expected = math.exp(-0.06 * t)
    assert curve.discount(ql.Date(17, ql.Month.May, 1999)) == pytest.approx(
        expected, rel=1e-12
    )


def _price_equity_option_european_put() -> float:
    calendar = ql.TARGET()
    todays_date = ql.Date(15, ql.Month.May, 1998)
    settlement_date = ql.Date(17, ql.Month.May, 1998)
    ql.set_evaluation_date(todays_date)

    underlying = 36.0
    strike = 40.0
    dividend_yield = 0.00
    risk_free_rate = 0.06
    volatility = 0.20
    maturity = ql.Date(17, ql.Month.May, 1999)
    day_counter = ql.Actual365Fixed()

    underlying_h = ql.make_quote_handle(underlying)
    flat_ts = ql.FlatForward(settlement_date, risk_free_rate, day_counter)
    flat_dividend_ts = ql.FlatForward(settlement_date, dividend_yield, day_counter)
    flat_vol_ts = ql.BlackConstantVol(
        settlement_date, calendar, volatility, day_counter
    )

    process = ql.BlackScholesMertonProcess(
        underlying_h, flat_dividend_ts, flat_ts, flat_vol_ts
    )
    payoff = ql.PlainVanillaPayoff(ql.OptionType.Put, strike)
    exercise = ql.EuropeanExercise(maturity)
    option = ql.EuropeanOption(payoff, exercise)
    option.set_pricing_engine(ql.AnalyticEuropeanEngine(process))
    return option.NPV()


def test_european_put_matches_equity_option_example():
    npv = _price_equity_option_european_put()
    assert npv == pytest.approx(EQUITY_OPTION_BS_PUT_NPV, rel=1e-10, abs=1e-10)


def test_european_put_matches_official_quantlib_if_available():
    npv = _price_equity_option_european_put()
    QuantLib = pytest.importorskip("QuantLib")

    calendar = QuantLib.TARGET()
    todays_date = QuantLib.Date(15, QuantLib.May, 1998)
    settlement_date = QuantLib.Date(17, QuantLib.May, 1998)
    QuantLib.Settings.instance().evaluationDate = todays_date

    underlying_h = QuantLib.QuoteHandle(QuantLib.SimpleQuote(36.0))
    day_counter = QuantLib.Actual365Fixed()
    flat_ts = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(settlement_date, 0.06, day_counter)
    )
    flat_dividend_ts = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(settlement_date, 0.00, day_counter)
    )
    flat_vol_ts = QuantLib.BlackVolTermStructureHandle(
        QuantLib.BlackConstantVol(settlement_date, calendar, 0.20, day_counter)
    )
    process = QuantLib.BlackScholesMertonProcess(
        underlying_h, flat_dividend_ts, flat_ts, flat_vol_ts
    )
    option = QuantLib.EuropeanOption(
        QuantLib.PlainVanillaPayoff(QuantLib.Option.Put, 40.0),
        QuantLib.EuropeanExercise(QuantLib.Date(17, QuantLib.May, 1999)),
    )
    option.setPricingEngine(QuantLib.AnalyticEuropeanEngine(process))
    assert npv == pytest.approx(option.NPV(), rel=1e-12, abs=1e-12)
