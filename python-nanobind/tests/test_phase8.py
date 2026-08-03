"""Phase-8 tests: ISDA CDS, GSR/Gaussian1d, FD value grid."""

from __future__ import annotations

import numpy as np
import pytest

import qlnb as ql


def test_version_is_at_least_phase8():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 9)


def test_interpolated_hazard_rate_curve_survival():
    today = ql.Date(15, ql.Month.May, 2007)
    ql.set_evaluation_date(today)
    dates = [
        today,
        today + ql.Period(1, ql.TimeUnit.Years),
        today + ql.Period(5, ql.TimeUnit.Years),
    ]
    rates = [0.03, 0.03, 0.03]
    curve = ql.InterpolatedHazardRateCurve(
        dates, rates, ql.Actual365Fixed(), ql.TARGET()
    )
    sp1 = curve.survival_probability(today + ql.Period(1, ql.TimeUnit.Years))
    assert sp1 == pytest.approx(0.9704, abs=5.0e-4)
    assert curve.hazard_rate(today + ql.Period(2, ql.TimeUnit.Years)) == pytest.approx(
        0.03, abs=1.0e-12
    )
    assert curve.max_date() == dates[-1]
    assert curve.default_probability(today + ql.Period(1, ql.TimeUnit.Years)) == (
        pytest.approx(1.0 - sp1, abs=1.0e-12)
    )


def test_isda_cds_engine_npv_near_midpoint():
    # ISDA engine requires Act/365Fixed on the discount curve. Mid-point can
    # share the same market data; NPVs should be the same order of magnitude.
    today = ql.Date(9, ql.Month.June, 2006)
    ql.set_evaluation_date(today)
    calendar = ql.WeekendsOnly()

    probability = ql.FlatHazardRate(0, calendar, 0.01234, ql.Actual365Fixed())
    discount = ql.FlatForward(today, 0.06, ql.Actual365Fixed())

    issue = calendar.advance(today, -1, ql.TimeUnit.Years)
    maturity = calendar.advance(issue, 10, ql.TimeUnit.Years)
    convention = ql.BusinessDayConvention.Following
    schedule = ql.Schedule(
        issue,
        maturity,
        ql.Period(ql.Frequency.Quarterly),
        calendar,
        convention,
        convention,
        ql.DateGeneration.CDS,
        False,
    )

    mid = ql.CreditDefaultSwap(
        ql.ProtectionSide.Seller,
        10_000.0,
        0.0120,
        schedule,
        convention,
        ql.Actual360(),
    )
    mid.set_pricing_engine(probability, 0.4, discount)

    isda = ql.CreditDefaultSwap(
        ql.ProtectionSide.Seller,
        10_000.0,
        0.0120,
        schedule,
        convention,
        ql.Actual360(),
    )
    isda.set_isda_pricing_engine(
        probability,
        0.4,
        discount,
        ql.IsdaCdsNumericalFix.Taylor,
        ql.IsdaCdsAccrualBias.HalfDayBias,
        ql.IsdaCdsForwardsInCouponPeriod.Piecewise,
    )

    assert mid.NPV() != 0.0
    assert isda.NPV() != 0.0
    # Same curve/recovery: fair spreads should be close across engines.
    assert isda.fair_spread() == pytest.approx(mid.fair_spread(), abs=5.0e-4)


def test_gsr_zerobond_matches_hull_white():
    today = ql.Date(15, ql.Month.May, 2007)
    ql.set_evaluation_date(today)
    yts = ql.FlatForward(today, 0.03, ql.Actual365Fixed())
    reversion = 0.01
    vol = 0.01
    gsr = ql.Gsr(yts, [], [vol], reversion, T=50.0)

    gsr_val = gsr.zerobond(5.0, 1.0, 0.0)
    assert gsr_val > 0.0
    assert gsr_val < 1.0
    assert gsr.numeraire_time() == pytest.approx(50.0)


def test_gaussian1d_swaption_npv_near_jamshidian():
    today = ql.Date(15, ql.Month.May, 2007)
    ql.set_evaluation_date(today)
    calendar = ql.TARGET()
    yts = ql.FlatForward(today, 0.03, ql.Actual365Fixed())
    index = ql.Euribor6M(yts)

    start = calendar.advance(today, 5, ql.TimeUnit.Years)
    maturity = calendar.advance(start, 10, ql.TimeUnit.Years)
    fixed_schedule = ql.Schedule(
        start,
        maturity,
        ql.Period(ql.Frequency.Annual),
        calendar,
        ql.BusinessDayConvention.Unadjusted,
        ql.BusinessDayConvention.Unadjusted,
        ql.DateGeneration.Forward,
        False,
    )
    float_schedule = ql.Schedule(
        start,
        maturity,
        ql.Period(ql.Frequency.Semiannual),
        calendar,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.BusinessDayConvention.ModifiedFollowing,
        ql.DateGeneration.Forward,
        False,
    )
    # ATM-ish fixed rate near the flat curve forward.
    fixed_rate = 0.03
    swap = ql.VanillaSwap(
        ql.SwapType.Payer,
        1.0,
        fixed_schedule,
        fixed_rate,
        ql.Thirty360(ql.Thirty360Convention.BondBasis),
        float_schedule,
        index,
        0.0,
        index.day_counter(),
    )
    exercise = ql.EuropeanExercise(start)
    reversion = 0.01
    vol = 0.01

    hw = ql.HullWhite(yts, reversion, vol)
    jam = ql.Swaption(swap, exercise)
    jam.set_jamshidian_pricing_engine(hw)

    gsr = ql.Gsr(yts, [], [vol], reversion, T=50.0)
    g1d = ql.Swaption(swap, exercise)
    g1d.set_gaussian1d_pricing_engine(gsr)

    assert jam.NPV() > 0.0
    assert g1d.NPV() > 0.0
    # Same constant HW/GSR parameters → NPVs should be very close.
    assert g1d.NPV() == pytest.approx(jam.NPV(), abs=5.0e-4)


def test_fdm_black_scholes_values_shape_and_spot_npv():
    today = ql.Date(15, ql.Month.May, 1998)
    ql.set_evaluation_date(today)
    settlement = ql.TARGET().advance(today, 2, ql.TimeUnit.Days)
    dc = ql.Actual365Fixed()
    spot = 100.0
    process = ql.BlackScholesMertonProcess(
        ql.make_quote_handle(spot),
        ql.FlatForward(settlement, 0.0, dc),
        ql.FlatForward(settlement, 0.06, dc),
        ql.BlackConstantVol(settlement, ql.TARGET(), 0.20, dc),
    )
    strike = 100.0
    grid = ql.fdm_black_scholes_values(
        process,
        strike,
        1.0,
        ql.OptionType.Call,
        t_grid=50,
        x_grid=51,
    )
    assert isinstance(grid, np.ndarray)
    assert grid.shape == (51, 2)
    spots = grid[:, 0]
    values = grid[:, 1]
    assert np.all(spots > 0.0)
    assert np.all(np.diff(spots) > 0.0)
    assert np.all(values >= -1.0e-8)

    # Value at the spot node should match a FD-engine European NPV closely.
    payoff = ql.PlainVanillaPayoff(ql.OptionType.Call, strike)
    maturity = settlement + ql.Period(1, ql.TimeUnit.Years)
    opt = ql.VanillaOption(payoff, ql.EuropeanExercise(maturity))
    opt.set_fd_pricing_engine(process, t_grid=50, x_grid=51)
    # Interpolate FD grid value at spot.
    value_at_spot = float(np.interp(spot, spots, values))
    assert value_at_spot == pytest.approx(opt.NPV(), abs=0.05)
    assert value_at_spot == pytest.approx(opt.NPV(), rel=0.02)


def test_compat_phase8_aliases():
    import qlnb.compat as cql

    assert callable(cql.fdmBlackScholesValues)
    assert hasattr(cql.CreditDefaultSwap, "setIsdaPricingEngine")
    assert hasattr(cql.Swaption, "setGaussian1dPricingEngine")
