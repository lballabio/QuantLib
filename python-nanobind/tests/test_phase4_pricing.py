"""Phase-4 experimental pricing tests (barrier + cap/floor)."""

from __future__ import annotations

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
    return ql.BlackScholesMertonProcess(
        ql.make_quote_handle(spot),
        ql.FlatForward(settlement, q, day_counter),
        ql.FlatForward(settlement, r, day_counter),
        ql.BlackConstantVol(settlement, calendar, vol, day_counter),
    )


def test_version_is_at_least_phase4():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 4)


def test_barrier_down_in_call_haug_value():
    # From QuantLib test-suite/barrieroption.cpp (Haug / Journal of Derivatives).
    # DownIn call: vol=0.20, K=100, barrier=90, S=100, q=0.02, r=0.05, T=360/360
    # => 1.64005
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    dc = ql.Actual360()
    maturity = todays + 360
    process = _bsm_process(todays, todays, 100.0, 0.02, 0.05, 0.20, dc)
    payoff = ql.PlainVanillaPayoff(ql.OptionType.Call, 100.0)
    exercise = ql.EuropeanExercise(maturity)
    option = ql.BarrierOption(
        ql.BarrierType.DownIn,
        90.0,
        0.0,
        payoff,
        exercise,
    )
    option.set_pricing_engine(ql.AnalyticBarrierEngine(process))
    assert option.NPV() == pytest.approx(1.64005, abs=1.0e-5)


def test_barrier_up_in_call_haug_value():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    dc = ql.Actual360()
    maturity = todays + 360
    process = _bsm_process(todays, todays, 100.0, 0.02, 0.05, 0.20, dc)
    option = ql.BarrierOption(
        ql.BarrierType.UpIn,
        110.0,
        0.0,
        ql.PlainVanillaPayoff(ql.OptionType.Call, 100.0),
        ql.EuropeanExercise(maturity),
    )
    option.set_pricing_engine(process)
    assert option.NPV() == pytest.approx(9.11008, abs=1.0e-5)


def test_cap_floor_cached_black_values():
    # Mirrors test-suite/capfloor.cpp::testCachedValue (par-coupon path).
    cached_today = ql.Date(14, ql.Month.March, 2002)
    cached_settlement = ql.Date(18, ql.Month.March, 2002)
    ql.set_evaluation_date(cached_today)
    curve = ql.FlatForward(cached_settlement, 0.05, ql.Actual360())
    index = ql.Euribor6M(curve)
    calendar = index.fixing_calendar()
    convention = ql.BusinessDayConvention.ModifiedFollowing
    start = curve.reference_date()
    end = calendar.advance(start, 20, ql.TimeUnit.Years, convention)
    schedule = ql.Schedule(
        start,
        end,
        ql.Period(ql.Frequency.Semiannual),
        calendar,
        convention,
        convention,
        ql.DateGeneration.Forward,
        False,
    )

    cap = ql.CapFloor(
        ql.CapFloorType.Cap, schedule, index, 0.07, nominal=100.0, fixing_days=2
    )
    floor = ql.CapFloor(
        ql.CapFloorType.Floor, schedule, index, 0.03, nominal=100.0, fixing_days=2
    )
    cap.set_pricing_engine(curve, 0.20, ql.Actual365Fixed())
    floor.set_pricing_engine(curve, 0.20, ql.Actual365Fixed())

    # Par-coupon cached NPVs from QuantLib test-suite.
    assert cap.NPV() == pytest.approx(6.87570026732, abs=1.0e-8)
    assert floor.NPV() == pytest.approx(2.65812927959, abs=1.0e-8)
    assert cap.type() == ql.CapFloorType.Cap
    assert floor.type() == ql.CapFloorType.Floor
    assert cap.maturity_date() > cap.start_date()


def test_make_cap_floor_helpers_smoke():
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    calendar = ql.TARGET()
    settlement = calendar.advance(todays, 2, ql.TimeUnit.Days)
    curve = ql.FlatForward(
        settlement, 0.05, ql.ActualActual(ql.ActualActualConvention.ISDA)
    )
    index = ql.Euribor6M(curve)
    cap = ql.make_cap(ql.Period(5, ql.TimeUnit.Years), index, 0.05, nominal=100.0)
    floor = ql.make_floor(ql.Period(5, ql.TimeUnit.Years), index, 0.05, nominal=100.0)
    cap.set_pricing_engine(curve, 0.20)
    floor.set_pricing_engine(curve, 0.20)
    assert cap.NPV() > 0.0
    assert floor.NPV() > 0.0
    atm = cap.atm_rate(curve)
    assert 0.04 < atm < 0.06


def test_barrier_optional_swig_comparison():
    QuantLib = pytest.importorskip("QuantLib")
    todays = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(todays)
    dc = ql.Actual360()
    maturity = todays + 360
    process = _bsm_process(todays, todays, 100.0, 0.02, 0.05, 0.20, dc)
    option = ql.BarrierOption(
        ql.BarrierType.DownIn,
        90.0,
        0.0,
        ql.PlainVanillaPayoff(ql.OptionType.Call, 100.0),
        ql.EuropeanExercise(maturity),
    )
    option.set_pricing_engine(process)
    qlnb_npv = option.NPV()

    today_swig = QuantLib.Date(15, QuantLib.May, 1998)
    QuantLib.Settings.instance().evaluationDate = today_swig
    spot = QuantLib.QuoteHandle(QuantLib.SimpleQuote(100.0))
    q_ts = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(today_swig, 0.02, QuantLib.Actual360())
    )
    r_ts = QuantLib.YieldTermStructureHandle(
        QuantLib.FlatForward(today_swig, 0.05, QuantLib.Actual360())
    )
    vol_ts = QuantLib.BlackVolTermStructureHandle(
        QuantLib.BlackConstantVol(
            today_swig, QuantLib.TARGET(), 0.20, QuantLib.Actual360()
        )
    )
    swig_process = QuantLib.BlackScholesMertonProcess(spot, q_ts, r_ts, vol_ts)
    swig_opt = QuantLib.BarrierOption(
        QuantLib.Barrier.DownIn,
        90.0,
        0.0,
        QuantLib.PlainVanillaPayoff(QuantLib.Option.Call, 100.0),
        QuantLib.EuropeanExercise(today_swig + 360),
    )
    swig_opt.setPricingEngine(QuantLib.AnalyticBarrierEngine(swig_process))
    assert qlnb_npv == pytest.approx(swig_opt.NPV(), abs=1.0e-8)
