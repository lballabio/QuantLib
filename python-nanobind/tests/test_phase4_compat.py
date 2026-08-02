"""Phase-4 SWIG compatibility shim tests."""

from __future__ import annotations

import pytest

import qlnb as ql_native
import qlnb.compat as ql


def test_compat_version():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 4)
    assert ql.__version__ == ql_native.__version__


def test_month_module_aliases():
    d = ql.Date(15, ql.May, 1998)
    assert d.dayOfMonth() == 15
    assert d.month() == ql.Month.May
    assert d.year() == 1998
    assert d.serialNumber() == ql_native.Date(15, ql_native.Month.May, 1998).serial_number()


def test_option_put_call_namespace():
    assert ql.Option.Put == ql_native.OptionType.Put
    assert ql.Option.Call == ql_native.OptionType.Call
    payoff = ql.PlainVanillaPayoff(ql.Option.Put, 40.0)
    assert payoff.strike() == 40.0
    assert payoff.optionType() == ql.Option.Put


def test_settings_evaluation_date_camelcase():
    d = ql.Date(17, ql.May, 1998)
    ql.setEvaluationDate(d)
    assert ql.getEvaluationDate() == d
    assert ql.evaluationDate() == d

    settings = ql.Settings.instance()
    settings.evaluationDate = ql.Date(1, ql.June, 1998)
    assert settings.evaluationDate == ql.Date(1, ql.June, 1998)
    assert settings.evaluation_date == ql.Date(1, ql.June, 1998)


def test_bond_camelcase_aliases():
    todays = ql.Date(15, ql.May, 1998)
    ql.set_evaluation_date(todays)
    calendar = ql.TARGET()
    settlement = calendar.advance(todays, 2, ql.TimeUnit.Days)
    maturity = calendar.advance(settlement, ql.Period(5, ql.TimeUnit.Years))
    schedule = ql.Schedule(
        settlement,
        maturity,
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Following,
        ql.BusinessDayConvention.Following,
        ql.DateGeneration.Backward,
        False,
    )
    curve = ql.FlatForward(settlement, 0.05, ql.Actual365Fixed())
    bond = ql.FixedRateBond(
        2,
        100.0,
        schedule,
        [0.05],
        ql.ActualActual(ql.ActualActualConvention.ISDA),
    )
    bond.setPricingEngine(curve)
    assert bond.cleanPrice() == pytest.approx(bond.clean_price(), abs=0.0)
    assert bond.dirtyPrice() == pytest.approx(bond.dirty_price(), abs=0.0)
    assert bond.NPV() > 0.0


def test_option_set_pricing_engine_alias():
    todays = ql.Date(15, ql.May, 1998)
    ql.set_evaluation_date(todays)
    settlement = ql.Date(17, ql.May, 1998)
    maturity = ql.Date(17, ql.May, 1999)
    process = ql.BlackScholesMertonProcess(
        ql.make_quote_handle(36.0),
        ql.FlatForward(settlement, 0.0, ql.Actual365Fixed()),
        ql.FlatForward(settlement, 0.06, ql.Actual365Fixed()),
        ql.BlackConstantVol(settlement, ql.TARGET(), 0.20, ql.Actual365Fixed()),
    )
    option = ql.EuropeanOption(
        ql.PlainVanillaPayoff(ql.Option.Put, 40.0),
        ql.EuropeanExercise(maturity),
    )
    option.setPricingEngine(process)
    assert option.NPV() == pytest.approx(3.844307791471182, rel=1e-8, abs=1e-8)


def test_compat_is_not_full_swig_parity_documented():
    import qlnb.compat as compat_mod

    assert "not full" in compat_mod.__doc__.lower() or "best-effort" in compat_mod.__doc__.lower()
