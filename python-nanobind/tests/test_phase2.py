"""Phase-2 pricing-coverage tests for qlnb."""

from __future__ import annotations

import numpy as np
import pytest

import qlnb as ql


def _bsm_process(
    todays: ql.Date,
    settlement: ql.Date,
    spot: float,
    q: float,
    r: float,
    vol: float,
    day_counter: ql.DayCounter | None = None,
):
    if day_counter is None:
        day_counter = ql.Actual365Fixed()
    calendar = ql.TARGET()
    underlying_h = ql.make_quote_handle(spot)
    flat_q = ql.FlatForward(settlement, q, day_counter)
    flat_r = ql.FlatForward(settlement, r, day_counter)
    flat_vol = ql.BlackConstantVol(settlement, calendar, vol, day_counter)
    return ql.BlackScholesMertonProcess(underlying_h, flat_q, flat_r, flat_vol)


def test_version_is_phase2():
    assert ql.__version__ == "0.3.0"


def test_american_barone_adesi_whaley_haug_put():
    # Haug "Option pricing formulas" value from QuantLib test-suite
    # (type=Put, K=100, S=90, q=r=0.10, t=0.10, vol=0.15) => 10.0000
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    dc = ql.Actual360()
    # t=0.10 year ≈ 36 days on Actual360
    maturity = todays + 36
    process = _bsm_process(todays, todays, 90.0, 0.10, 0.10, 0.15, dc)
    payoff = ql.PlainVanillaPayoff(ql.OptionType.Put, 100.0)
    exercise = ql.AmericanExercise(todays, maturity)
    option = ql.VanillaOption(payoff, exercise)
    option.set_pricing_engine(ql.BaroneAdesiWhaleyEngine(process))
    assert option.NPV() == pytest.approx(10.0000, abs=3.0e-3)


def test_american_call_above_european():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    settlement = ql.Date(17, ql.Month.May, 1998)
    maturity = ql.Date(17, ql.Month.May, 1999)
    process = _bsm_process(todays, settlement, 100.0, 0.03, 0.06, 0.20)

    payoff = ql.PlainVanillaPayoff(ql.OptionType.Put, 100.0)
    american = ql.VanillaOption(payoff, ql.AmericanExercise(todays, maturity))
    american.set_pricing_engine(process)
    european = ql.EuropeanOption(payoff, ql.EuropeanExercise(maturity))
    european.set_pricing_engine(process)

    # American put with dividends/rates should be at least the European value.
    assert american.NPV() >= european.NPV() - 1e-8


def test_european_greeks_and_implied_vol():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    settlement = ql.Date(17, ql.Month.May, 1998)
    maturity = ql.Date(17, ql.Month.May, 1999)
    vol = 0.20
    process = _bsm_process(todays, settlement, 36.0, 0.0, 0.06, vol)
    option = ql.EuropeanOption(
        ql.PlainVanillaPayoff(ql.OptionType.Put, 40.0),
        ql.EuropeanExercise(maturity),
    )
    option.set_pricing_engine(process)
    npv = option.NPV()
    assert option.delta() < 0.0
    assert option.gamma() > 0.0
    assert option.vega() > 0.0

    implied = option.implied_volatility(npv, process)
    assert implied == pytest.approx(vol, rel=1e-4, abs=1e-4)


def test_mc_european_near_analytic():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    settlement = ql.Date(17, ql.Month.May, 1998)
    maturity = ql.Date(17, ql.Month.May, 1999)
    process = _bsm_process(todays, settlement, 36.0, 0.0, 0.06, 0.20)
    payoff = ql.PlainVanillaPayoff(ql.OptionType.Put, 40.0)
    exercise = ql.EuropeanExercise(maturity)

    analytic = ql.EuropeanOption(payoff, exercise)
    analytic.set_pricing_engine(process)
    target = analytic.NPV()

    mc = ql.EuropeanOption(payoff, exercise)
    mc.set_mc_pricing_engine(
        process,
        time_steps=2,
        required_samples=50000,
        seed=42,
        antithetic=True,
    )
    assert mc.NPV() == pytest.approx(target, rel=0.05, abs=0.25)


def test_simulate_gbm_paths_numpy_shape_and_start():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    settlement = ql.Date(17, ql.Month.May, 1998)
    spot = 36.0
    process = _bsm_process(todays, settlement, spot, 0.0, 0.06, 0.20)
    paths = ql.simulate_gbm_paths(
        process, length=1.0, time_steps=12, samples=128, seed=7
    )
    assert isinstance(paths, np.ndarray)
    assert paths.shape == (128, 13)
    assert paths.dtype == np.float64
    assert np.allclose(paths[:, 0], spot)
    assert np.all(np.isfinite(paths))
    # Terminal values should have positive dispersion under vol.
    assert float(np.std(paths[:, -1])) > 0.0


def test_forward_rate_agreement_atm_near_zero_npv():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    calendar = ql.TARGET()
    settlement = calendar.advance(todays, 2, ql.TimeUnit.Days)
    curve = ql.FlatForward(settlement, 0.05, ql.Actual365Fixed())
    index = ql.Euribor3M(curve)

    value_date = calendar.advance(
        settlement, ql.Period(3, ql.TimeUnit.Months),
        ql.BusinessDayConvention.ModifiedFollowing,
    )
    # Probe the forecast, then strike ATM so NPV/amount ~ 0.
    probe = ql.ForwardRateAgreement(
        index, value_date, ql.Position.Long, 0.05, 1_000_000.0, curve
    )
    atm = float(probe.forward_rate())
    assert 0.04 < atm < 0.06
    fra = ql.ForwardRateAgreement(
        index, value_date, ql.Position.Long, atm, 1_000_000.0, curve
    )
    assert fra.NPV() == pytest.approx(0.0, abs=1e-6)
    assert fra.amount() == pytest.approx(0.0, abs=1e-6)


def test_forward_rate_agreement_off_market_sign():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    calendar = ql.TARGET()
    settlement = calendar.advance(todays, 2, ql.TimeUnit.Days)
    curve = ql.FlatForward(settlement, 0.05, ql.Actual365Fixed())
    index = ql.Euribor3M(curve)
    value_date = calendar.advance(
        settlement, ql.Period(3, ql.TimeUnit.Months),
        ql.BusinessDayConvention.ModifiedFollowing,
    )
    # Long FRA struck below forward should have positive value.
    fra = ql.ForwardRateAgreement(
        index, value_date, ql.Position.Long, 0.04, 1_000_000.0, curve
    )
    assert fra.NPV() > 0.0


def test_phase2_american_matches_swig_if_available():
    QuantLib = pytest.importorskip("QuantLib")

    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    dc = ql.Actual360()
    maturity = todays + 36
    process = _bsm_process(todays, todays, 90.0, 0.10, 0.10, 0.15, dc)
    option = ql.VanillaOption(
        ql.PlainVanillaPayoff(ql.OptionType.Put, 100.0),
        ql.AmericanExercise(todays, maturity),
    )
    option.set_pricing_engine(process)
    qlnb_npv = option.NPV()

    QuantLib.Settings.instance().evaluationDate = QuantLib.Date(
        15, QuantLib.May, 1998
    )
    today_ql = QuantLib.Date(15, QuantLib.May, 1998)
    ex = today_ql + 36
    spot = QuantLib.QuoteHandle(QuantLib.SimpleQuote(90.0))
    q_ts = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(today_ql, 0.10, QuantLib.Actual360())
    )
    r_ts = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(today_ql, 0.10, QuantLib.Actual360())
    )
    vol_ts = QuantLib.BlackVolTermStructureHandle(
        QuantLib.BlackConstantVol(
            today_ql, QuantLib.TARGET(), 0.15, QuantLib.Actual360()
        )
    )
    ql_process = QuantLib.BlackScholesMertonProcess(spot, q_ts, r_ts, vol_ts)
    ql_option = QuantLib.VanillaOption(
        QuantLib.PlainVanillaPayoff(QuantLib.Option.Put, 100.0),
        QuantLib.AmericanExercise(today_ql, ex),
    )
    ql_option.setPricingEngine(
        QuantLib.BaroneAdesiWhaleyApproximationEngine(ql_process)
    )
    assert qlnb_npv == pytest.approx(ql_option.NPV(), rel=1e-10, abs=1e-10)


def test_phase2_fra_matches_swig_if_available():
    QuantLib = pytest.importorskip("QuantLib")

    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    calendar = ql.TARGET()
    settlement = calendar.advance(todays, 2, ql.TimeUnit.Days)
    curve = ql.FlatForward(settlement, 0.05, ql.Actual365Fixed())
    index = ql.Euribor3M(curve)
    value_date = calendar.advance(
        settlement, ql.Period(3, ql.TimeUnit.Months),
        ql.BusinessDayConvention.ModifiedFollowing,
    )
    fra = ql.ForwardRateAgreement(
        index, value_date, ql.Position.Long, 0.04, 1_000_000.0, curve
    )
    qlnb_npv = fra.NPV()

    QuantLib.Settings.instance().evaluationDate = QuantLib.Date(
        15, QuantLib.May, 1998
    )
    ql_cal = QuantLib.TARGET()
    ql_today = QuantLib.Date(15, QuantLib.May, 1998)
    ql_settlement = ql_cal.advance(ql_today, 2, QuantLib.Days)
    ql_curve = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(ql_settlement, 0.05, QuantLib.Actual365Fixed())
    )
    ql_index = QuantLib.Euribor3M(ql_curve)
    ql_value = ql_cal.advance(
        ql_settlement, QuantLib.Period(3, QuantLib.Months),
        QuantLib.ModifiedFollowing,
    )
    ql_fra = QuantLib.ForwardRateAgreement(
        ql_index, ql_value, QuantLib.Position.Long, 0.04, 1_000_000.0, ql_curve
    )
    assert qlnb_npv == pytest.approx(ql_fra.NPV(), rel=1e-10, abs=1e-10)
