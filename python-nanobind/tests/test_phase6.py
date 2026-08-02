"""Phase-6 tests: floating bonds, tree/FD pricing, overnight indexes."""

from __future__ import annotations

import pytest

import qlnb as ql


def test_version_is_phase6():
    assert ql.__version__ == "0.7.0"


def _flat_curves(today: ql.Date, forecast: float, discount: float):
    dc = ql.Actual365Fixed()
    return ql.FlatForward(today, forecast, dc), ql.FlatForward(today, discount, dc)


def test_floating_rate_bond_near_par_same_curves():
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    calendar = ql.TARGET()
    settlement = calendar.advance(today, 2, ql.TimeUnit.Days)
    forecast, discount = _flat_curves(settlement, 0.04, 0.04)
    index = ql.Euribor6M(forecast)
    schedule = ql.Schedule(
        settlement,
        calendar.advance(settlement, ql.Period(5, ql.TimeUnit.Years)),
        ql.Period(6, ql.TimeUnit.Months),
        calendar,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.DateGeneration.Forward,
        False,
    )
    bond = ql.FloatingRateBond(
        2,
        100.0,
        schedule,
        index,
        ql.ActualActual(ql.ActualActualConvention.ISDA),
        ql.BusinessDayConvention.ModifiedFollowing,
        2,
    )
    bond.set_pricing_engine(discount)
    # Same forecast/discount + zero spread → clean price near par.
    assert bond.clean_price() == pytest.approx(100.0, abs=0.5)
    assert bond.NPV() > 0.0
    assert bond.dirty_price() >= bond.clean_price() - 1.0e-10
    assert bond.maturity_date() == schedule.end_date()


def test_floating_rate_bond_discount_below_forecast_cheaper():
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    calendar = ql.TARGET()
    settlement = calendar.advance(today, 2, ql.TimeUnit.Days)
    forecast, _ = _flat_curves(settlement, 0.04, 0.04)
    discount_high = ql.FlatForward(settlement, 0.06, ql.Actual365Fixed())
    index = ql.Euribor6M(forecast)
    schedule = ql.Schedule(
        settlement,
        calendar.advance(settlement, ql.Period(3, ql.TimeUnit.Years)),
        ql.Period(6, ql.TimeUnit.Months),
        calendar,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.DateGeneration.Forward,
        False,
    )
    bond = ql.FloatingRateBond(
        2,
        100.0,
        schedule,
        index,
        ql.ActualActual(ql.ActualActualConvention.ISDA),
    )
    bond.set_pricing_engine(discount_high)
    assert bond.clean_price() < 99.0


def test_floating_rate_bond_optional_swig_comparison():
    QuantLib = pytest.importorskip("QuantLib")

    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    calendar = ql.TARGET()
    settlement = calendar.advance(today, 2, ql.TimeUnit.Days)
    forecast = ql.FlatForward(settlement, 0.04, ql.Actual365Fixed())
    discount = ql.FlatForward(settlement, 0.045, ql.Actual365Fixed())
    index = ql.Euribor6M(forecast)
    end = calendar.advance(settlement, ql.Period(4, ql.TimeUnit.Years))
    schedule = ql.Schedule(
        settlement,
        end,
        ql.Period(6, ql.TimeUnit.Months),
        calendar,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.DateGeneration.Forward,
        False,
    )
    bond = ql.FloatingRateBond(
        2,
        100.0,
        schedule,
        index,
        ql.ActualActual(ql.ActualActualConvention.ISDA),
        ql.BusinessDayConvention.ModifiedFollowing,
        2,
    )
    bond.set_pricing_engine(discount)
    qlnb_price = bond.clean_price()

    today_s = QuantLib.Date(15, QuantLib.May, 1998)
    QuantLib.Settings.instance().evaluationDate = today_s
    cal_s = QuantLib.TARGET()
    settlement_s = cal_s.advance(today_s, 2, QuantLib.Days)
    forecast_s = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(settlement_s, 0.04, QuantLib.Actual365Fixed())
    )
    discount_s = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(settlement_s, 0.045, QuantLib.Actual365Fixed())
    )
    index_s = QuantLib.Euribor6M(forecast_s)
    end_s = cal_s.advance(settlement_s, 4, QuantLib.Years)
    schedule_s = QuantLib.Schedule(
        settlement_s,
        end_s,
        QuantLib.Period(6, QuantLib.Months),
        cal_s,
        QuantLib.ModifiedFollowing,
        QuantLib.ModifiedFollowing,
        QuantLib.DateGeneration.Forward,
        False,
    )
    bond_s = QuantLib.FloatingRateBond(
        2,
        100.0,
        schedule_s,
        index_s,
        QuantLib.ActualActual(QuantLib.ActualActual.ISDA),
        QuantLib.ModifiedFollowing,
        2,
    )
    bond_s.setPricingEngine(QuantLib.DiscountingBondEngine(discount_s))
    QuantLib.setCouponPricer(
        bond_s.cashflows(), QuantLib.BlackIborCouponPricer()
    )
    assert qlnb_price == pytest.approx(bond_s.cleanPrice(), abs=1.0e-8)


def _bsm_process(todays, settlement, spot, q, r, vol, day_counter=None):
    if day_counter is None:
        day_counter = ql.Actual365Fixed()
    return ql.BlackScholesMertonProcess(
        ql.make_quote_handle(spot),
        ql.FlatForward(settlement, q, day_counter),
        ql.FlatForward(settlement, r, day_counter),
        ql.BlackConstantVol(settlement, ql.TARGET(), vol, day_counter),
    )


def test_american_put_binomial_between_european_and_intrinsic():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    settlement = ql.Date(17, ql.Month.May, 1998)
    maturity = ql.Date(17, ql.Month.May, 1999)
    spot, strike = 100.0, 100.0
    process = _bsm_process(todays, settlement, spot, 0.03, 0.06, 0.20)
    payoff = ql.PlainVanillaPayoff(ql.OptionType.Put, strike)

    european = ql.EuropeanOption(payoff, ql.EuropeanExercise(maturity))
    european.set_pricing_engine(process)
    eu_npv = european.NPV()

    american = ql.VanillaOption(payoff, ql.AmericanExercise(todays, maturity))
    american.set_binomial_pricing_engine(process, steps=401)
    am_npv = american.NPV()

    # Intrinsic (immediate exercise) lower bound for American put.
    intrinsic = max(strike - spot, 0.0)
    assert am_npv >= eu_npv - 1.0e-6
    assert am_npv >= intrinsic - 1.0e-8
    # Binomial American should be in a plausible premium over European.
    assert am_npv < eu_npv + 5.0


def test_american_put_fd_near_binomial():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    settlement = ql.Date(17, ql.Month.May, 1998)
    maturity = ql.Date(17, ql.Month.May, 1999)
    process = _bsm_process(todays, settlement, 100.0, 0.03, 0.06, 0.20)
    payoff = ql.PlainVanillaPayoff(ql.OptionType.Put, 100.0)
    exercise = ql.AmericanExercise(todays, maturity)

    tree = ql.VanillaOption(payoff, exercise)
    tree.set_binomial_pricing_engine(process, steps=401)
    fd = ql.VanillaOption(payoff, exercise)
    fd.set_fd_pricing_engine(process, t_grid=100, x_grid=100)

    european = ql.EuropeanOption(payoff, ql.EuropeanExercise(maturity))
    european.set_pricing_engine(process)

    assert fd.NPV() >= european.NPV() - 0.05
    assert fd.NPV() == pytest.approx(tree.NPV(), abs=0.15)


def test_overnight_indexes_construction_and_calendars():
    today = ql.Date(15, ql.Month.January, 2024)
    ql.set_evaluation_date(today)
    curve = ql.FlatForward(today, 0.05, ql.Actual365Fixed())

    sofr = ql.Sofr(curve)
    estr = ql.Estr(curve)
    eonia = ql.Eonia(curve)

    assert "SOFR" in sofr.name().upper()
    assert "ESTR" in estr.name().upper()
    assert "EONIA" in eonia.name().upper()
    assert sofr.tenor() == ql.Period(1, ql.TimeUnit.Days)
    assert sofr.fixing_days() == 0

    # SOFR uses the US SOFR calendar; ESTR/Eonia use TARGET.
    assert sofr.fixing_calendar().is_holiday(ql.Date(1, ql.Month.January, 2024))
    assert estr.fixing_calendar().is_holiday(ql.Date(1, ql.Month.January, 2024))
    assert eonia.fixing_calendar().name() == estr.fixing_calendar().name()

    # Past fixing round-trip on a business day.
    fixing_date = sofr.fixing_calendar().advance(
        today, -5, ql.TimeUnit.Days, ql.BusinessDayConvention.Preceding
    )
    sofr.add_fixing(fixing_date, 0.0531)
    assert sofr.fixing(fixing_date) == pytest.approx(0.0531, abs=1.0e-12)


def test_make_ois_fair_rate_near_flat_curve():
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    curve = ql.FlatForward(today, 0.04, ql.Actual360())
    index = ql.Estr(curve)
    off_market = ql.make_ois(
        ql.Period(2, ql.TimeUnit.Years),
        index,
        0.02,
        ql.Period(0, ql.TimeUnit.Days),
    )
    off_market.set_pricing_engine(curve)
    fair = off_market.fair_rate()
    # Flat 4% continuous/simple curve → OIS fair rate in the same neighborhood.
    assert 0.035 < fair < 0.045
    assert off_market.NPV() != 0.0

    at_market = ql.make_ois(
        ql.Period(2, ql.TimeUnit.Years),
        index,
        fair,
        ql.Period(0, ql.TimeUnit.Days),
    )
    at_market.set_pricing_engine(curve)
    assert abs(at_market.NPV()) < 1.0e-8


def test_compat_phase6_aliases():
    import qlnb.compat as ql_c

    assert hasattr(ql_c.FloatingRateBond, "setPricingEngine")
    assert hasattr(ql_c.FloatingRateBond, "cleanPrice")
    assert hasattr(ql_c.VanillaOption, "setBinomialPricingEngine")
    assert hasattr(ql_c.VanillaOption, "setFdPricingEngine")
    assert hasattr(ql_c.OvernightIndexedSwap, "fairRate")
    assert ql_c.makeOIS is ql.make_ois
