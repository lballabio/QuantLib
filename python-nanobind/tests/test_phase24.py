"""Phase-24 tests: currencies, money, exchange rates, FX forward."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase24():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 25)


def test_currency_iso_codes():
    usd = ql.USDCurrency()
    eur = ql.EURCurrency()
    gbp = ql.GBPCurrency()
    sgd = ql.SGDCurrency()

    assert usd.code() == "USD"
    assert usd.numeric_code() == 840
    assert not usd.empty()

    assert eur.code() == "EUR"
    assert eur.numeric_code() == 978

    assert gbp.code() == "GBP"
    assert gbp.numeric_code() == 826

    assert sgd.code() == "SGD"
    assert sgd.numeric_code() == 702

    assert usd == ql.USDCurrency()
    assert usd != eur


def test_money_and_direct_exchange_rate():
    # Mirrors ExchangeRateTests::testDirect.
    ql.set_money_conversion(ql.MoneyConversionType.NoConversion)
    eur = ql.EURCurrency()
    usd = ql.USDCurrency()
    eur_usd = ql.ExchangeRate(eur, usd, 1.2042)

    m1 = 50000.0 * eur
    m2 = 100000.0 * usd

    assert m1.currency() == eur
    assert m1.value() == pytest.approx(50000.0)

    calculated = eur_usd.exchange(m1)
    assert calculated.currency() == usd
    assert calculated.value() == pytest.approx(60210.0)

    calculated = eur_usd.exchange(m2)
    assert calculated.currency() == eur
    assert calculated.value() == pytest.approx(100000.0 / 1.2042)


def test_derived_exchange_rate_chain():
    # Mirrors ExchangeRateTests::testDerived.
    ql.set_money_conversion(ql.MoneyConversionType.NoConversion)
    eur = ql.EURCurrency()
    usd = ql.USDCurrency()
    gbp = ql.GBPCurrency()

    eur_usd = ql.ExchangeRate(eur, usd, 1.2042)
    eur_gbp = ql.ExchangeRate(eur, gbp, 0.6612)
    derived = ql.ExchangeRate.chain(eur_usd, eur_gbp)

    assert derived.type() == ql.ExchangeRateType.Derived

    m1 = 50000.0 * gbp
    calculated = derived.exchange(m1)
    expected = 50000.0 * 1.2042 / 0.6612
    assert calculated.currency() == usd
    assert calculated.value() == pytest.approx(expected)


def test_exchange_rate_manager_lookup():
    ql.set_money_conversion(ql.MoneyConversionType.NoConversion)
    ql.exchange_rate_manager_clear()

    eur = ql.EURCurrency()
    usd = ql.USDCurrency()
    ql.exchange_rate_manager_add(
        ql.ExchangeRate(eur, usd, 1.1983),
        ql.Date(4, ql.Month.August, 2004),
    )
    ql.exchange_rate_manager_add(
        ql.ExchangeRate(usd, eur, 1.0 / 1.2042),
        ql.Date(5, ql.Month.August, 2004),
    )

    rate = ql.exchange_rate_manager_lookup(
        eur,
        usd,
        ql.Date(4, ql.Month.August, 2004),
        ql.ExchangeRateType.Direct,
    )
    calculated = rate.exchange(50000.0 * eur)
    assert calculated.value() == pytest.approx(50000.0 * 1.1983)

    rate = ql.exchange_rate_manager_lookup(
        eur,
        usd,
        ql.Date(5, ql.Month.August, 2004),
        ql.ExchangeRateType.Direct,
    )
    calculated = rate.exchange(50000.0 * eur)
    assert calculated.value() == pytest.approx(50000.0 * 1.2042)

    ql.exchange_rate_manager_clear()


def _fx_forward_market():
    # Mirrors FxForwardTests::CommonVars.
    today = ql.Date(15, ql.Month.March, 2024)
    ql.set_evaluation_date(today)
    maturity = today + ql.Period(6, ql.TimeUnit.Months)
    dc = ql.Actual365Fixed()
    usd_curve = ql.FlatForward(today, 0.05, dc)
    sgd_curve = ql.FlatForward(today, 0.035, dc)
    spot = 1.35
    return today, maturity, usd_curve, sgd_curve, spot


def test_fx_forward_construction():
    _today, maturity, _usd_curve, _sgd_curve, _spot = _fx_forward_market()
    usd = ql.USDCurrency()
    sgd = ql.SGDCurrency()

    fwd = ql.FxForward(
        1_000_000.0, usd, 1_350_000.0, sgd, maturity, True,
    )
    assert fwd.source_nominal() == pytest.approx(1_000_000.0)
    assert fwd.target_nominal() == pytest.approx(1_350_000.0)
    assert fwd.source_currency() == usd
    assert fwd.target_currency() == sgd
    assert fwd.maturity_date() == maturity
    assert fwd.pay_source_currency() is True
    assert fwd.forward_rate() == pytest.approx(1.35)
    assert fwd.is_expired() is False

    fwd_rate = ql.FxForward(1_000_000.0, usd, sgd, 1.36, maturity, True)
    assert fwd_rate.forward_rate() == pytest.approx(1.36)
    assert fwd_rate.target_nominal() == pytest.approx(1_360_000.0)


def test_fx_forward_fair_rate_and_atm_npv():
    # Mirrors FxForwardTests::testFairForwardRate.
    _today, maturity, usd_curve, sgd_curve, spot = _fx_forward_market()
    usd = ql.USDCurrency()
    sgd = ql.SGDCurrency()

    fwd = ql.FxForward(
        1_000_000.0, usd, 1_350_000.0, sgd, maturity, True,
    )
    fwd.set_pricing_engine(usd_curve, sgd_curve, spot)

    settlement = fwd.settlement_date()
    df_usd = usd_curve.discount(maturity) / usd_curve.discount(settlement)
    df_sgd = sgd_curve.discount(maturity) / sgd_curve.discount(settlement)
    expected_fair = spot * df_usd / df_sgd

    assert fwd.fair_forward_rate() == pytest.approx(expected_fair, rel=1.0e-6)
    assert fwd.NPV() != 0.0

    atm = ql.FxForward(
        1_000_000.0, usd, sgd, fwd.fair_forward_rate(), maturity, True,
    )
    atm.set_pricing_engine(usd_curve, sgd_curve, ql.make_quote_handle(spot))
    assert atm.NPV() == pytest.approx(0.0, abs=1.0e-4)


def test_compat_phase24_aliases():
    import qlnb.compat as cql

    assert cql.USDCurrency is not None
    assert cql.EURCurrency is not None
    assert hasattr(cql.FxForward, "fairForwardRate")
    assert hasattr(cql.FxForward, "setPricingEngine")
    assert hasattr(cql.FxForward, "sourceNominal")
    assert cql.ExchangeRate is not None
