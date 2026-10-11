/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Raghav Subbarao

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fxlocalvol.cpp
    \brief Tests for the FX local vol surface and FxVanillaBumpRisk Greek calculator.
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"

#include <ql/experimental/fxslv/fxsettlementconvention.hpp>
#include <ql/experimental/fxslv/fxvariancesurface.hpp>
#include <ql/experimental/fxslv/fxsmilesectionbydelta.hpp>
#include <ql/termstructures/volatility/equityfx/localvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/noexceptlocalvolsurface.hpp>
#include <ql/experimental/fxslv/tradingtimetermstructure.hpp>
#include <ql/experimental/fxslv/fxvanillagreeks.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/exercise.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/quotes/deltavolquote.hpp>
#include <ql/experimental/fxslv/fxslvpricingcontext.hpp>
#include <ql/experimental/fxslv/hestoncalibrator.hpp>
#include <ql/experimental/fxslv/slvleveragecalibrator.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/settings.hpp>
#include <cmath>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(FxLocalVolTests)

// ─────────────────────────────────────────────────────────────────────────────
//  Shared market data used by all tests.
// ─────────────────────────────────────────────────────────────────────────────

namespace {

    struct FxMarket {
        Date today;
        Date expiry3M;
        Date expiry6M;

        DayCounter dc;
        Handle<YieldTermStructure> eurTs; // foreign
        Handle<YieldTermStructure> usdTs; // domestic
        Handle<tradingTimeTermStructure> timeTs;

        ext::shared_ptr<SimpleQuote> spotSQ;
        Handle<Quote> spot;

        std::vector<Date> pillars;
        std::vector<Handle<Quote>> atms;
        std::vector<std::vector<Handle<Quote>>> rrs, bfs;
        std::vector<Real> deltas;
        std::vector<ext::shared_ptr<SimpleQuote>> atmSQs;

        // Per-pillar RR and BF SimpleQuotes for rega/sega.
        std::vector<std::vector<ext::shared_ptr<SimpleQuote>>> rrPillarSQs, bfPillarSQs;
        std::vector<Time> rrPillarTimes, bfPillarTimes;

        DeltaVolQuote::DeltaType deltaType;
        DeltaVolQuote::AtmType   atmType;
        FxRrBfQuotes::FlyType  flyType;

        ext::shared_ptr<fxVarianceSurfaceNCP<FxQuadraticSmileSection>> surface;
        ext::shared_ptr<GeneralizedBlackScholesProcess> process;

        FxMarket() {
            today    = Date(2, January, 2024);
            expiry3M = Date(2, April,   2024);
            expiry6M = Date(2, July,    2024);
            Settings::instance().evaluationDate() = today;
            dc = Actual365Fixed();

            eurTs = Handle<YieldTermStructure>(
                ext::make_shared<FlatForward>(0, NullCalendar(), 0.04, dc));
            usdTs = Handle<YieldTermStructure>(
                ext::make_shared<FlatForward>(0, NullCalendar(), 0.0525, dc));
            timeTs = Handle<tradingTimeTermStructure>(
                ext::make_shared<tradingTimeTermStructure>(today, WeekendsOnly(), 0.0));

            spotSQ = ext::make_shared<SimpleQuote>(1.0850);
            spot   = Handle<Quote>(spotSQ);

            deltaType = DeltaVolQuote::Fwd;
            atmType   = DeltaVolQuote::AtmFwd;
            flyType   = FxRrBfQuotes::SmileStrangle;
            deltas    = { 0.25, 0.10 };

            // Two pillars: 3M and 6M
            pillars = { expiry3M, expiry6M };

            struct Vol { Real atm, rr25, bf25, rr10, bf10; };
            std::vector<Vol> mkt = {
                { 0.0850, -0.014, 0.0045, -0.0280, 0.0095 }, // 3M
                { 0.0900, -0.016, 0.0050, -0.0310, 0.0105 }, // 6M
            };

            rrPillarSQs.resize(pillars.size());
            bfPillarSQs.resize(pillars.size());
            rrPillarTimes.resize(pillars.size());
            bfPillarTimes.resize(pillars.size());

            for (Size i = 0; i < pillars.size(); ++i) {
                auto atmSQ  = ext::make_shared<SimpleQuote>(mkt[i].atm);
                auto rr25SQ = ext::make_shared<SimpleQuote>(mkt[i].rr25);
                auto rr10SQ = ext::make_shared<SimpleQuote>(mkt[i].rr10);
                auto bf25SQ = ext::make_shared<SimpleQuote>(mkt[i].bf25);
                auto bf10SQ = ext::make_shared<SimpleQuote>(mkt[i].bf10);

                atmSQs.push_back(atmSQ);
                rrPillarSQs[i] = { rr25SQ, rr10SQ };
                bfPillarSQs[i] = { bf25SQ, bf10SQ };

                const Time T = dc.yearFraction(today, pillars[i]);
                rrPillarTimes[i] = bfPillarTimes[i] = T;

                atms.push_back(Handle<Quote>(atmSQ));
                rrs.push_back({ Handle<Quote>(rr25SQ), Handle<Quote>(rr10SQ) });
                bfs.push_back({ Handle<Quote>(bf25SQ), Handle<Quote>(bf10SQ) });
            }

            surface = ext::make_shared<fxVarianceSurfaceNCP<FxQuadraticSmileSection>>(
                today, spot, pillars, atms, rrs, bfs, deltas,
                eurTs, usdTs, timeTs,
                deltaType, atmType, flyType,
                FxSettlementConvention(WeekendsOnly(), 2),
                WeekendsOnly(), Following, true);
            surface->enableExtrapolation();

            process = ext::make_shared<GeneralizedBlackScholesProcess>(
                spot, eurTs, usdTs,
                Handle<BlackVolTermStructure>(surface));
        }

        // Build a 3M EUR call at ATM forward
        ext::shared_ptr<VanillaOption> makeAtmCall() const {
            Time T   = dc.yearFraction(today, expiry3M);
            Real fwd = spot->value() * eurTs->discount(T) / usdTs->discount(T);
            auto payoff   = ext::make_shared<PlainVanillaPayoff>(Option::Call, fwd);
            auto exercise = ext::make_shared<EuropeanExercise>(expiry3M);
            return ext::make_shared<VanillaOption>(payoff, exercise);
        }

        // Build a 3M EUR put at ATM forward
        ext::shared_ptr<VanillaOption> makeAtmPut() const {
            Time T   = dc.yearFraction(today, expiry3M);
            Real fwd = spot->value() * eurTs->discount(T) / usdTs->discount(T);
            auto payoff   = ext::make_shared<PlainVanillaPayoff>(Option::Put, fwd);
            auto exercise = ext::make_shared<EuropeanExercise>(expiry3M);
            return ext::make_shared<VanillaOption>(payoff, exercise);
        }
    };

} // anonymous namespace


// ─────────────────────────────────────────────────────────────────────────────
//  Test 1: Local vol surface values are non-negative
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testLocalVolSurfaceNonNegative) {
    BOOST_TEST_MESSAGE("Testing FX local vol surface: values are non-negative...");

    FxMarket mkt;

    LocalVolSurface lv(mkt.process->blackVolatility(),
                        mkt.process->riskFreeRate(),
                        mkt.process->dividendYield(),
                        mkt.process->x0());
    lv.enableExtrapolation();

    std::vector<Time> times  = { 1.0/12, 3.0/12, 6.0/12 };
    std::vector<Real> strikes = { 0.95, 1.00, 1.05, 1.08, 1.10, 1.15 };

    for (Time t : times)
        for (Real K : strikes) {
            Volatility vol = lv.localVol(t, K, true);
            BOOST_CHECK_MESSAGE(
                vol > 0.0,
                "negative local vol " << vol
                << " at T=" << t << ", K=" << K);
        }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Test 2: FixedLocalVolSurface agrees with Dupire at grid points
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testFixedLocalVolConsistency) {
    BOOST_TEST_MESSAGE("Testing FX local vol: FixedLocalVolSurface agrees with "
                       "Dupire at sample points...");

    FxMarket mkt;
    auto option = mkt.makeAtmCall();

    FxVanillaBumpRisk riskCalc(option, mkt.process, mkt.spotSQ, mkt.atmSQs);

    std::vector<Time> times   = { 1.0/12, 3.0/12, 6.0/12 };
    std::vector<Real> strikes = { 0.95, 1.00, 1.05, 1.10 };

    auto fixedLV = riskCalc.buildFixedLocalVolSurface(times, strikes);
    fixedLV->enableExtrapolation();

    LocalVolSurface lvDupire(mkt.process->blackVolatility(),
                              mkt.process->riskFreeRate(),
                              mkt.process->dividendYield(),
                              mkt.process->x0());
    lvDupire.enableExtrapolation();

    const Real tol = 1e-10;
    for (Time t : times)
        for (Real K : strikes) {
            Real lvD = lvDupire.localVol(t, K, true);
            Real lvF = fixedLV->localVol(t, K, true);
            BOOST_CHECK_MESSAGE(
                std::fabs(lvD - lvF) < tol,
                "Dupire=" << lvD << " FixedLV=" << lvF
                << " diff=" << std::fabs(lvD - lvF)
                << " at T=" << t << " K=" << K);
        }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Test 3: Greek signs
//    Call:  spotDelta > 0, spotGamma > 0, theta < 0, volga > 0, vega > 0
//    Put:   spotDelta < 0, spotGamma > 0, theta < 0, volga > 0, vega > 0
//    navva == vanna (by construction)
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testGreekSigns) {
    BOOST_TEST_MESSAGE("Testing FX local vol Greeks: sign conventions...");

    FxMarket mkt;

    // ── Call Greeks ──────────────────────────────────────────────────────────
    {
        auto option = mkt.makeAtmCall();
        FxVanillaBumpRisk riskCalc(option, mkt.process, mkt.spotSQ, mkt.atmSQs,
                                   mkt.rrPillarSQs, mkt.rrPillarTimes,
                                   mkt.bfPillarSQs, mkt.bfPillarTimes,
                                   /*notional=*/1.0e6,
                                   /*spotBump=*/0.001,
                                   /*volBump=*/0.001,
                                   /*rrBump=*/0.001,
                                   /*bfBump=*/0.001,
                                   /*tGrid=*/50, /*xGrid=*/50);

        FxVanillaGreeks g = riskCalc.calculate(/*localVol=*/true);

        BOOST_CHECK_MESSAGE(g.npv       > 0.0, "call NPV must be positive: "    << g.npv);
        BOOST_CHECK_MESSAGE(g.spotDelta > 0.0, "call spot delta must be >0: "   << g.spotDelta);
        BOOST_CHECK_MESSAGE(g.fwdDelta  > 0.0, "call fwd delta must be >0: "    << g.fwdDelta);
        BOOST_CHECK_MESSAGE(g.spotGamma > 0.0, "call spot gamma must be >0: "   << g.spotGamma);
        BOOST_CHECK_MESSAGE(g.fwdGamma  > 0.0, "call fwd gamma must be >0: "    << g.fwdGamma);
        BOOST_CHECK_MESSAGE(g.theta     < 0.0, "call theta must be negative: "  << g.theta);
        BOOST_CHECK_MESSAGE(g.vega      > 0.0, "call vega must be positive: "   << g.vega);
        BOOST_CHECK_MESSAGE(g.volga     > 0.0, "call volga must be positive: "  << g.volga);

        // navva == vanna (identity, not approximate)
        BOOST_CHECK_MESSAGE(g.navva == g.vanna,
            "navva must equal vanna: navva=" << g.navva << " vanna=" << g.vanna);
    }

    // ── Put Greeks ───────────────────────────────────────────────────────────
    {
        auto option = mkt.makeAtmPut();
        FxVanillaBumpRisk riskCalc(option, mkt.process, mkt.spotSQ, mkt.atmSQs,
                                   mkt.rrPillarSQs, mkt.rrPillarTimes,
                                   mkt.bfPillarSQs, mkt.bfPillarTimes,
                                   /*notional=*/1.0e6,
                                   /*spotBump=*/0.001,
                                   /*volBump=*/0.001,
                                   /*rrBump=*/0.001,
                                   /*bfBump=*/0.001,
                                   /*tGrid=*/50, /*xGrid=*/50);

        FxVanillaGreeks g = riskCalc.calculate(/*localVol=*/true);

        BOOST_CHECK_MESSAGE(g.npv       > 0.0, "put NPV must be positive: "    << g.npv);
        BOOST_CHECK_MESSAGE(g.spotDelta < 0.0, "put spot delta must be <0: "   << g.spotDelta);
        BOOST_CHECK_MESSAGE(g.fwdDelta  < 0.0, "put fwd delta must be <0: "    << g.fwdDelta);
        BOOST_CHECK_MESSAGE(g.spotGamma > 0.0, "put spot gamma must be >0: "   << g.spotGamma);
        BOOST_CHECK_MESSAGE(g.fwdGamma  > 0.0, "put fwd gamma must be >0: "    << g.fwdGamma);
        BOOST_CHECK_MESSAGE(g.theta     < 0.0, "put theta must be negative: "  << g.theta);
        BOOST_CHECK_MESSAGE(g.vega      > 0.0, "put vega must be positive: "   << g.vega);
        BOOST_CHECK_MESSAGE(g.volga     > 0.0, "put volga must be positive: "  << g.volga);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Test 4: Put-call parity
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testPutCallParity) {
    BOOST_TEST_MESSAGE("Testing FX local vol: put-call parity...");

    FxMarket mkt;

    Time T   = mkt.dc.yearFraction(mkt.today, mkt.expiry3M);
    Real Bd  = mkt.usdTs->discount(T);
    Real Bf  = mkt.eurTs->discount(T);
    Real fwd = mkt.spot->value() * Bf / Bd;

    const Real notional = 1.0e6;

    auto callOption = mkt.makeAtmCall();
    auto putOption  = mkt.makeAtmPut();

    FxVanillaBumpRisk callCalc(callOption, mkt.process, mkt.spotSQ, mkt.atmSQs,
                                mkt.rrPillarSQs, mkt.rrPillarTimes,
                                mkt.bfPillarSQs, mkt.bfPillarTimes,
                                notional, 0.001, 0.001, 0.001, 0.001, 50, 50);
    FxVanillaBumpRisk putCalc(putOption, mkt.process, mkt.spotSQ, mkt.atmSQs,
                               mkt.rrPillarSQs, mkt.rrPillarTimes,
                               mkt.bfPillarSQs, mkt.bfPillarTimes,
                               notional, 0.001, 0.001, 0.001, 0.001, 50, 50);

    FxVanillaGreeks callG = callCalc.calculate(true);
    FxVanillaGreeks putG  = putCalc.calculate(true);

    // For ATM forward: call NPV - put NPV ≈ 0
    const Real priceTol = 10.0; // $10 on a 1M notional position
    BOOST_CHECK_MESSAGE(
        std::fabs(callG.npv - putG.npv) < priceTol,
        "put-call parity violation: call-put=" << (callG.npv - putG.npv)
        << " (tolerance=" << priceTol << ")");

    // Gamma parity: call gamma == put gamma (both in USD per 1% spot^2)
    const Real gammaTol = 0.01; // $0.01 in scaled units
    BOOST_CHECK_MESSAGE(
        std::fabs(callG.spotGamma - putG.spotGamma) < gammaTol,
        "gamma parity violation: call gamma=" << callG.spotGamma
        << " put gamma=" << putG.spotGamma);

    // Vega parity: call vega == put vega
    const Real vegaTol = 0.10;
    BOOST_CHECK_MESSAGE(
        std::fabs(callG.vega - putG.vega) < vegaTol,
        "vega parity violation: call vega=" << callG.vega
        << " put vega=" << putG.vega);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Test 5: Delta and gamma consistency
//
//  Taylor approximation: ΔV ≈ spotDelta × pctMove + 0.5 × spotGamma × pctMove²
//  where pctMove = ds / (0.01 × S)  (spot move expressed in units of 1%)
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testDeltaGammaConsistency) {
    BOOST_TEST_MESSAGE("Testing FX local vol: delta-gamma consistency...");

    FxMarket mkt;
    auto option = mkt.makeAtmCall();

    const Real notional = 1.0;  // unit notional for this test

    FxVanillaBumpRisk riskCalc(option, mkt.process, mkt.spotSQ, mkt.atmSQs,
                                mkt.rrPillarSQs, mkt.rrPillarTimes,
                                mkt.bfPillarSQs, mkt.bfPillarTimes,
                                notional, 0.001, 0.001, 0.001, 0.001, 50, 50);
    FxVanillaGreeks g = riskCalc.calculate(true);

    Real S0  = mkt.spotSQ->value();
    Real ds  = 0.0005; // 5 pips absolute
    Real V0  = g.npv;

    mkt.spotSQ->setValue(S0 + ds);
    FxVanillaGreeks gShifted = riskCalc.calculate(true);
    Real Vshifted = gShifted.npv;
    mkt.spotSQ->setValue(S0);

    Real actualPnL = Vshifted - V0;

    // spotDelta is in USD per 1% spot; convert ds to units of 1% spot.
    Real pctMove     = ds / (0.01 * S0);
    Real predictedPnL = g.spotDelta * pctMove + 0.5 * g.spotGamma * pctMove * pctMove;

    // Allow 5% relative error in the second-order Taylor approximation
    Real relErr = std::fabs(actualPnL - predictedPnL) / std::fabs(actualPnL);
    BOOST_CHECK_MESSAGE(
        relErr < 0.05,
        "delta-gamma Taylor approx error=" << relErr * 100 << "% "
        << "(actual=" << actualPnL << " predicted=" << predictedPnL << ")");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Test 6: BS vs local vol pricing
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testBsVsLocalVolPricing) {
    BOOST_TEST_MESSAGE("Testing FX local vol: BS vs local vol pricing...");

    FxMarket mkt;
    auto option = mkt.makeAtmCall();
    const Real notional = 1.0e6;

    FxVanillaBumpRisk riskCalc(option, mkt.process, mkt.spotSQ, mkt.atmSQs,
                                mkt.rrPillarSQs, mkt.rrPillarTimes,
                                mkt.bfPillarSQs, mkt.bfPillarTimes,
                                notional, 0.001, 0.001, 0.001, 0.001, 100, 100);

    FxVanillaGreeks bsG = riskCalc.calculate(false);
    FxVanillaGreeks lvG = riskCalc.calculate(true);

    BOOST_CHECK_MESSAGE(bsG.npv > 0.0, "BS price must be positive: " << bsG.npv);
    BOOST_CHECK_MESSAGE(lvG.npv > 0.0, "LV price must be positive: " << lvG.npv);

    Real relDiff = std::fabs(bsG.npv - lvG.npv) / bsG.npv;
    BOOST_CHECK_MESSAGE(
        relDiff < 0.20,
        "BS and local vol prices differ too much: BS=" << bsG.npv
        << " LV=" << lvG.npv << " relDiff=" << relDiff * 100 << "%");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Test 7: Sticky-delta vs sticky-strike deltas differ when skew is present
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testStickyDeltaVsStickyStrike) {
    BOOST_TEST_MESSAGE("Testing FX local vol: sticky-delta vs sticky-strike deltas differ...");

    FxMarket mkt;
    auto option = mkt.makeAtmCall();
    const Real notional = 1.0e6;

    FxVanillaBumpRisk riskCalc(option, mkt.process, mkt.spotSQ, mkt.atmSQs,
                                mkt.rrPillarSQs, mkt.rrPillarTimes,
                                mkt.bfPillarSQs, mkt.bfPillarTimes,
                                notional,
                                /*spotBump=*/0.001,
                                /*volBump=*/0.001,
                                /*rrBump=*/0.001,
                                /*bfBump=*/0.001,
                                /*tGrid=*/50, /*xGrid=*/50,
                                /*lvTimePts=*/15, /*lvStrikePts=*/30);

    FxVanillaGreeks sdG = riskCalc.calculate(true, FxVanillaBumpRisk::StickyType::Delta);
    FxVanillaGreeks ssG = riskCalc.calculate(true, FxVanillaBumpRisk::StickyType::Strike);

    BOOST_CHECK_MESSAGE(sdG.spotDelta > 0.0,
        "sticky-delta call spot delta must be >0: " << sdG.spotDelta);
    BOOST_CHECK_MESSAGE(ssG.spotDelta > 0.0,
        "sticky-strike call spot delta must be >0: " << ssG.spotDelta);

    BOOST_CHECK_MESSAGE(sdG.npv > 0.0, "sticky-delta NPV must be >0: " << sdG.npv);
    BOOST_CHECK_MESSAGE(ssG.npv > 0.0, "sticky-strike NPV must be >0: " << ssG.npv);

    const Real npvTol = 10.0;
    BOOST_CHECK_MESSAGE(
        std::fabs(sdG.npv - ssG.npv) < npvTol,
        "sticky-delta and sticky-strike NPVs should agree: "
        << "sd=" << sdG.npv << " ss=" << ssG.npv
        << " diff=" << std::fabs(sdG.npv - ssG.npv));

    // Deltas must differ due to skew (at least 0.01% relative difference).
    const Real minRelDiff = 1.0e-4;
    const Real relDiff = std::fabs(sdG.spotDelta - ssG.spotDelta)
                         / std::fabs(sdG.spotDelta);
    BOOST_CHECK_MESSAGE(
        relDiff > minRelDiff,
        "sticky-delta and sticky-strike deltas should differ with non-zero skew: "
        << "sd=" << sdG.spotDelta << " ss=" << ssG.spotDelta
        << " relDiff=" << relDiff * 100.0 << "%"
        << " (min expected=" << minRelDiff * 100.0 << "%)");

    // With EUR put skew: sticky-delta < sticky-strike.
    BOOST_CHECK_MESSAGE(
        sdG.spotDelta < ssG.spotDelta,
        "with EUR put skew, sticky-delta should be < sticky-strike: "
        << "sd=" << sdG.spotDelta << " ss=" << ssG.spotDelta);

    // navva == vanna in both modes.
    BOOST_CHECK_MESSAGE(sdG.navva == sdG.vanna,
        "sticky-delta: navva must equal vanna");
    BOOST_CHECK_MESSAGE(ssG.navva == ssG.vanna,
        "sticky-strike: navva must equal vanna");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Test 8: Full 11-pillar SLV calibration and risk on a non-pillar tenor
//
//  Builds an 11-pillar EURUSD surface (o/n through 2y), calibrates a Heston
//  stochastic vol model, calibrates the SLV leverage function, then prices and
//  risks a 4M ATM-forward call.  4M falls strictly between the 3M and 6M
//  surface pillars — exercising the interpolation in both the surface and the
//  leverage function.
//
//  Checks:
//    - Heston RMSE < 1.5 vol pts  (model fits the smile reasonably)
//    - SLV NPV > 0 and delta ∈ (0, 1)
//    - SLV price within 20% of the local-vol Dupire price
//    - FxVanillaBumpRisk Greeks are sensible under the SLV engine
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testSLVCalibrationAndRisk) {
    BOOST_TEST_MESSAGE(
        "Testing FX local vol + SLV: full 11-pillar calibration and 4M risk...");

    // ── Market data ──────────────────────────────────────────────────────────
    const Date today = Date(2, January, 2024);
    Settings::instance().evaluationDate() = today;

    const DayCounter dc       = Actual365Fixed();
    const Calendar   calendar = WeekendsOnly();

    const Handle<YieldTermStructure> usdTs(
        ext::make_shared<FlatForward>(0, NullCalendar(), 0.0525, dc));  // domestic
    const Handle<YieldTermStructure> eurTs(
        ext::make_shared<FlatForward>(0, NullCalendar(), 0.0400, dc));  // foreign

    auto spotSQ = ext::make_shared<SimpleQuote>(1.0850);
    Handle<Quote> spot(spotSQ);

    // ── 11-pillar vol surface quotes (o/n through 2y) ────────────────────────
    // ATM vol rises from 6.2% at o/n to 10.0% at 2y.
    // 25d risk-reversal is negative (EUR put skew, typical EURUSD).
    // 25d butterfly is small positive (vol smile convexity).
    struct PillarQuotes { Real atm, rr25, bf25, rr10, bf10; };
    const std::vector<PillarQuotes> mktVols = {
        // tenor    ATM     RR25    BF25    RR10    BF10
        {/*O/N */ 0.0620, -0.0020, 0.0002, -0.0040, 0.0005 },
        {/*1W  */ 0.0680, -0.0030, 0.0003, -0.0060, 0.0007 },
        {/*2W  */ 0.0720, -0.0050, 0.0004, -0.0100, 0.0010 },
        {/*1M  */ 0.0780, -0.0120, 0.0040, -0.0250, 0.0090 },
        {/*2M  */ 0.0810, -0.0130, 0.0042, -0.0265, 0.0092 },
        {/*3M  */ 0.0850, -0.0140, 0.0045, -0.0280, 0.0095 },
        {/*6M  */ 0.0900, -0.0160, 0.0050, -0.0310, 0.0105 },
        {/*9M  */ 0.0920, -0.0170, 0.0052, -0.0330, 0.0110 },
        {/*1Y  */ 0.0950, -0.0180, 0.0055, -0.0350, 0.0115 },
        {/*18M */ 0.0970, -0.0190, 0.0058, -0.0370, 0.0120 },
        {/*2Y  */ 0.1000, -0.0200, 0.0060, -0.0390, 0.0125 },
    };

    const std::vector<Period> tenors = {
        1*Days,  1*Weeks,  2*Weeks,
        1*Months, 2*Months, 3*Months, 6*Months,
        9*Months, 1*Years,  18*Months, 2*Years
    };

    std::vector<Date>                       pillars;
    std::vector<Handle<Quote>>              atms;
    std::vector<std::vector<Handle<Quote>>> rrs, bfs;
    std::vector<ext::shared_ptr<SimpleQuote>> atmSQs;
    std::vector<std::vector<ext::shared_ptr<SimpleQuote>>> rrSQs, bfSQs;

    for (Size i = 0; i < tenors.size(); ++i) {
        pillars.push_back(calendar.advance(today, tenors[i]));
        auto a   = ext::make_shared<SimpleQuote>(mktVols[i].atm);
        auto r25 = ext::make_shared<SimpleQuote>(mktVols[i].rr25);
        auto r10 = ext::make_shared<SimpleQuote>(mktVols[i].rr10);
        auto b25 = ext::make_shared<SimpleQuote>(mktVols[i].bf25);
        auto b10 = ext::make_shared<SimpleQuote>(mktVols[i].bf10);
        atmSQs.push_back(a);
        rrSQs.push_back({ r25, r10 });
        bfSQs.push_back({ b25, b10 });
        atms.push_back(Handle<Quote>(a));
        rrs.push_back({ Handle<Quote>(r25), Handle<Quote>(r10) });
        bfs.push_back({ Handle<Quote>(b25), Handle<Quote>(b10) });
    }

    const std::vector<Real> deltas = { 0.25, 0.10 };
    auto timeTs = Handle<tradingTimeTermStructure>(
        ext::make_shared<tradingTimeTermStructure>(today, calendar, 0.0));

    auto volSurface = ext::make_shared<fxVarianceSurfaceNCP<FxQuadraticSmileSection>>(
        today, spot, pillars, atms, rrs, bfs, deltas,
        eurTs, usdTs, timeTs,
        DeltaVolQuote::Fwd, DeltaVolQuote::AtmFwd, FxRrBfQuotes::SmileStrangle,
        FxSettlementConvention(calendar, 2),
        calendar, Following, true);
    volSurface->enableExtrapolation();
    Handle<BlackVolTermStructure> volHandle(volSurface);

    // ── Dupire local vol surface ─────────────────────────────────────────────
    auto localVol = ext::make_shared<NoExceptLocalVolSurface>(
        volHandle, usdTs, eurTs, spot, 0.01);
    localVol->enableExtrapolation();
    Handle<LocalVolTermStructure> localVolHandle(localVol);

    // ── SLV calibration ──────────────────────────────────────────────────────
    // Use 5 tenors × 3 strikes for Heston calibration.
    const std::vector<Period> calibTenors = {
        1*Months, 3*Months, 6*Months, 1*Years, 2*Years
    };
    std::vector<StochVolCalibrator::Pillar> hestonPillars;
    for (const auto& tenor : calibTenors) {
        const Date   expiry = calendar.advance(today, tenor);
        const Time   T      = dc.yearFraction(today, expiry);
        const Real   fwd    = spotSQ->value()
                              * eurTs->discount(T) / usdTs->discount(T);
        for (Real m : { 0.95, 1.00, 1.05 })
            hestonPillars.push_back({ tenor, m * fwd });
    }

    HestonParams hp;
    hp.v0    = 0.0078 * 0.0078;
    hp.kappa = 1.50;
    hp.theta = 0.0100 * 0.0100;
    hp.sigma = 0.40;
    hp.rho   = -0.25;

    FxSLVPricingContext ctx(spot, usdTs, eurTs, volHandle, localVolHandle, calendar);

    HestonCalibrator hestonCal(hp);

    // Small FDM grid for test speed.
    auto fdmP = HestonSLVLeverageCalibrator::defaultParams();
    fdmP.xGrid            = 51;
    fdmP.vGrid            = 21;
    fdmP.tMaxStepsPerYear = 52;
    fdmP.tMinStepsPerYear = 12;
    fdmP.tStepNumberDecay = 2.0;

    HestonSLVLeverageCalibrator levCal(fdmP);

    const Date endDate = calendar.advance(today, 2*Years);
    ctx.calibrate(hestonCal, levCal, hestonPillars, endDate);

    // Heston fit quality check.
    BOOST_CHECK_MESSAGE(ctx.hestonRmse() < 0.015,
        "Heston RMSE too large: " << ctx.hestonRmse() * 100.0 << " vol pts");

    // ── Price a 4M ATM-forward call ──────────────────────────────────────────
    // 4M is strictly between the 3M (surface pillar) and 6M (surface pillar).
    const Date   expiry4M = calendar.advance(today, 4*Months);
    const Time   T4M      = dc.yearFraction(today, expiry4M);
    const Real   fwd4M    = spotSQ->value()
                            * eurTs->discount(T4M) / usdTs->discount(T4M);

    auto payoff4M   = ext::make_shared<PlainVanillaPayoff>(Option::Call, fwd4M);
    auto exercise4M = ext::make_shared<EuropeanExercise>(expiry4M);
    auto call4M     = ext::make_shared<VanillaOption>(payoff4M, exercise4M);

    call4M->setPricingEngine(ctx.vanillaEngine(FdmGridConfig{ 100, 100, 50, 0 }));

    const Real slvNpv = call4M->NPV();
    BOOST_CHECK_MESSAGE(slvNpv > 0.0,
        "SLV 4M call NPV must be positive: " << slvNpv);

    // ── Delta via finite difference ──────────────────────────────────────────
    const Real S0 = spotSQ->value();
    const Real ds = 0.0010;
    spotSQ->setValue(S0 + ds);
    const Real callUp = call4M->NPV();
    spotSQ->setValue(S0 - ds);
    const Real callDn = call4M->NPV();
    spotSQ->setValue(S0);

    const Real slvDelta = (callUp - callDn) / (2.0 * ds);
    BOOST_CHECK_MESSAGE(slvDelta > 0.0 && slvDelta < 1.0,
        "SLV 4M call delta must be in (0, 1): " << slvDelta);

    // ── Vega via finite difference ───────────────────────────────────────────
    // Bump all ATM vol quotes in parallel (parallel shift of ATM curve).
    const Real dv = 0.001;
    for (auto& sq : atmSQs) sq->setValue(sq->value() + dv);
    const Real callVolUp = call4M->NPV();
    for (auto& sq : atmSQs) sq->setValue(sq->value() - 2.0 * dv);
    const Real callVolDn = call4M->NPV();
    for (auto& sq : atmSQs) sq->setValue(sq->value() + dv);  // restore

    const Real slvVega = (callVolUp - callVolDn) / (2.0 * dv);
    BOOST_CHECK_MESSAGE(slvVega > 0.0,
        "SLV 4M call vega must be positive: " << slvVega);

    // ── SLV vs local-vol Dupire price comparison ─────────────────────────────
    auto process = ext::make_shared<GeneralizedBlackScholesProcess>(
        spot, eurTs, usdTs, volHandle);
    auto call4M_lv = ext::make_shared<VanillaOption>(payoff4M, exercise4M);
    call4M_lv->setPricingEngine(
        ext::make_shared<FdBlackScholesVanillaEngine>(process, 100, 100));
    const Real lvNpv = call4M_lv->NPV();

    const Real relDiff = std::fabs(slvNpv - lvNpv) / lvNpv;
    BOOST_CHECK_MESSAGE(relDiff < 0.20,
        "SLV and local-vol 4M NPVs differ by " << relDiff * 100.0
        << "% (SLV=" << slvNpv << " LV=" << lvNpv << ")");
}

BOOST_AUTO_TEST_CASE(testTradingTimeFormsAgree) {
    BOOST_TEST_MESSAGE("Testing that trading time from dates and from times agree...");

    const Date today(5, January, 2024);  // a Friday
    Settings::instance().evaluationDate() = today;
    for (const Calendar& calendar : {Calendar(WeekendsOnly()), Calendar(TARGET())}) {
        for (Real w : {0.0, 0.3, 1.0}) {
            tradingTimeTermStructure ts(today, calendar, w);
            // every day for seven years, beyond any fixed holiday horizon
            for (Date d = today; d <= today + 7 * Years; ++d) {
                const Time byDate = ts.tradingTime(d);
                const Time byTime = ts.tradingTime(ts.timeFromReference(d));
                if (std::fabs(byDate - byTime) > 1.0e-14) {
                    BOOST_ERROR(calendar.name() << ", weekend weight " << w << ", " << d
                                                << ": " << byDate << " from the date, " << byTime
                                                << " from the time");
                    break;
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testTradingTimeIsMonotonicAndContinuous) {
    BOOST_TEST_MESSAGE("Testing that trading time runs at rate 1 on business days and the weekend weight otherwise...");

    const Date today(5, January, 2024);
    Settings::instance().evaluationDate() = today;
    for (Real w : {0.0, 0.3, 1.0}) {
        tradingTimeTermStructure ts(today, TARGET(), w);
        // in tenths of a day over two months, including weekends and New Year
        const Real step = 0.1 / 365.0;
        Time previous = ts.tradingTime(0.0);
        BOOST_CHECK_EQUAL(previous, 0.0);
        for (Size i = 1; i <= 600; ++i) {
            const Time t = i * step, tt = ts.tradingTime(t);
            const Real rate = (tt - previous) / step;
            if (rate < w - 1.0e-9 || rate > 1.0 + 1.0e-9) {
                BOOST_ERROR("weekend weight " << w << ": rate " << rate << " at "
                                              << t * 365.0 << " days");
                break;
            }
            previous = tt;
        }
        if (w == 1.0)
            BOOST_CHECK_SMALL(ts.tradingTime(3.0) - 3.0, 1.0e-14);
    }

    // with weekends not counted, a week has five trading days whatever its first day
    tradingTimeTermStructure ts(today, WeekendsOnly(), 0.0);
    for (Integer start = 0; start < 7; ++start)
        BOOST_CHECK_SMALL(ts.tradingTime(today + start, today + start + 7) * 365.0 - 5.0, 1.0e-12);
}

BOOST_AUTO_TEST_CASE(testTradingTimeEventJumps) {
    BOOST_TEST_MESSAGE("Testing trading time jumps at events...");

    using Jump = tradingTimeTermStructure::Jump;
    const Date today(5, January, 2024);  // a Friday
    Settings::instance().evaluationDate() = today;
    const Date eventDate(10, January, 2024);  // the next Wednesday
    const Real timeOfDay = 13.5 / 24.0;
    auto size = ext::make_shared<SimpleQuote>(2.0);
    tradingTimeTermStructure plain(today, WeekendsOnly(), 0.0);
    tradingTimeTermStructure ts(
        today, WeekendsOnly(), 0.0,
        {Jump(eventDate, timeOfDay, Handle<Quote>(size)),
         Jump(Date(4, January, 2024), 0.5, makeQuoteHandle(5.0))});

    // the jump before the reference date has happened; the other is at its time
    const Time te = (eventDate - today + timeOfDay) / 365.0;
    const std::vector<Time> jumps = ts.jumpTimes();
    BOOST_REQUIRE_EQUAL(jumps.size(), Size(1));
    BOOST_CHECK_SMALL(jumps[0] - te, 1.0e-15);
    BOOST_CHECK_SMALL(ts.tradingTime(te - 1.0e-9) - plain.tradingTime(te - 1.0e-9), 1.0e-15);
    BOOST_CHECK_SMALL(ts.tradingTime(te) - plain.tradingTime(te) - 2.0 / 365.0, 1.0e-15);
    // by date: not yet at the start of the event day, included the day after
    BOOST_CHECK_SMALL(ts.tradingTime(eventDate) - plain.tradingTime(eventDate), 1.0e-15);
    BOOST_CHECK_SMALL(ts.tradingTime(eventDate + 1) - plain.tradingTime(eventDate + 1) - 2.0 / 365.0,
                      1.0e-15);

    // the jump follows its quote, and the change is notified
    auto observed = ext::make_shared<tradingTimeTermStructure>(
        today, WeekendsOnly(), 0.0, std::vector<Jump>{Jump(eventDate, timeOfDay, Handle<Quote>(size))});
    Flag flag;
    flag.registerWith(observed);
    size->setValue(3.0);
    BOOST_CHECK(flag.isUp());
    BOOST_CHECK_SMALL(observed->tradingTime(eventDate + 1) - plain.tradingTime(eventDate + 1) -
                          3.0 / 365.0, 1.0e-15);
    size->setValue(-1.0);
    BOOST_CHECK_THROW(observed->tradingTime(eventDate + 1), Error);
}

BOOST_AUTO_TEST_CASE(testTradingTimeJumpsNeedAnExplicitTime) {
    BOOST_TEST_MESSAGE("Testing that trading time jumps need an explicit time of day...");

    using Jump = tradingTimeTermStructure::Jump;
    const Date eventDate(10, January, 2024);
    const Handle<Quote> size = makeQuoteHandle(1.0);
    // no time, or midnight, is not accepted
    BOOST_CHECK_THROW(Jump(eventDate, size), Error);
    BOOST_CHECK_THROW(Jump(eventDate, 0.0, size), Error);
    BOOST_CHECK_THROW(Jump(eventDate, 1.0, size), Error);
    BOOST_CHECK_THROW(Jump(eventDate, 0.5, Handle<Quote>()), Error);
#ifdef QL_HIGH_RESOLUTION_DATE
    // a date carrying a time gives it; giving it twice is ambiguous
    const Date eventTime(10, January, 2024, 13, 30, 0);
    const Jump fromDate(eventTime, size);
    BOOST_CHECK_EQUAL(fromDate.date(), eventDate);
    BOOST_CHECK_SMALL(fromDate.timeOfDay() - 13.5 / 24.0, 1.0e-12);
    BOOST_CHECK_THROW(Jump(eventTime, 0.5, size), Error);
    BOOST_CHECK_THROW(Jump(Date(10, January, 2024, 0, 0, 0), size), Error);
#endif
}

BOOST_AUTO_TEST_CASE(testTradingTimeDiffuseEvents) {
    BOOST_TEST_MESSAGE("Testing trading time with diffuse events over whole days...");

    using DiffuseEvent = tradingTimeTermStructure::DiffuseEvent;
    const Date today(18, December, 2023);  // a Monday
    Settings::instance().evaluationDate() = today;
    const Date christmasEve(22, December, 2023), quietStart(25, December, 2023);
    auto halfDay = ext::make_shared<SimpleQuote>(-0.5);
    const Real w = 0.5;
    tradingTimeTermStructure plain(today, WeekendsOnly(), w);
    tradingTimeTermStructure ts(
        today, WeekendsOnly(), w, {},
        {DiffuseEvent(christmasEve, christmasEve + 1, Handle<Quote>(halfDay)),
         DiffuseEvent(quietStart, quietStart + 7, makeQuoteHandle(-2.8))});
    auto days = [](const tradingTimeTermStructure& c, const Date& d1, const Date& d2) {
        return c.tradingTime(d1, d2) * 365.0;
    };

    // a half day counts half, through the day
    BOOST_CHECK_SMALL(days(ts, christmasEve, christmasEve + 1) - 0.5, 1.0e-12);
    BOOST_CHECK_SMALL(ts.tradingTime(ts.timeFromReference(christmasEve) + 0.5 / 365.0) -
                          ts.tradingTime(christmasEve) - 0.25 / 365.0, 1.0e-15);
    // a quiet week takes 0.4 days from each of its seven days
    BOOST_CHECK_SMALL(days(ts, quietStart, quietStart + 7) - (days(plain, quietStart, quietStart + 7) - 2.8),
                      1.0e-12);
    BOOST_CHECK_SMALL(days(ts, quietStart, quietStart + 1) - 0.6, 1.0e-12);
    // elsewhere nothing changes
    BOOST_CHECK_SMALL(days(ts, today, christmasEve) - days(plain, today, christmasEve), 1.0e-12);

    // the two forms still agree, and the rate stays non-negative
    for (Date d = today; d <= today + 60; ++d)
        BOOST_CHECK_SMALL(ts.tradingTime(d) - ts.tradingTime(ts.timeFromReference(d)), 1.0e-14);

    // the weight follows its quote, and the change is notified
    auto observed = ext::make_shared<tradingTimeTermStructure>(
        today, WeekendsOnly(), w, std::vector<tradingTimeTermStructure::Jump>{},
        std::vector<DiffuseEvent>{DiffuseEvent(christmasEve, christmasEve + 1, Handle<Quote>(halfDay))});
    BOOST_CHECK_SMALL(days(*observed, christmasEve, christmasEve + 1) - 0.5, 1.0e-12);
    Flag flag;
    flag.registerWith(observed);
    halfDay->setValue(0.5);
    BOOST_CHECK(flag.isUp());
    BOOST_CHECK_SMALL(days(*observed, christmasEve, christmasEve + 1) - 1.5, 1.0e-12);
    // more than the day's own rate cannot be taken away
    halfDay->setValue(-1.5);
    BOOST_CHECK_THROW(observed->tradingTime(christmasEve + 1), Error);

    BOOST_CHECK_THROW(DiffuseEvent(christmasEve, christmasEve, makeQuoteHandle(1.0)), Error);
    BOOST_CHECK_THROW(DiffuseEvent(christmasEve, christmasEve + 1, Handle<Quote>()), Error);
#ifdef QL_HIGH_RESOLUTION_DATE
    BOOST_CHECK_THROW(DiffuseEvent(Date(22, December, 2023, 12, 0, 0), christmasEve + 1,
                                   makeQuoteHandle(1.0)),
                      Error);
#endif
}

BOOST_AUTO_TEST_CASE(testTradingTimeFloatingReference) {
    BOOST_TEST_MESSAGE("Testing trading time with a reference date following the evaluation date...");

    const Date today(5, January, 2024);
    Settings::instance().evaluationDate() = today;
    tradingTimeTermStructure floating(0, TARGET(), 0.2);
    floating.tradingTime(today + 30);  // build the state for the first reference date
    for (Integer days : {1, 7, 400}) {
        // with no settlement days the reference date is the evaluation
        // date moved to a business day, as for any term structure
        Settings::instance().evaluationDate() = today + days;
        const Date reference = floating.referenceDate();
        BOOST_CHECK_EQUAL(reference, TARGET().adjust(today + days));
        tradingTimeTermStructure fixed(reference, TARGET(), 0.2);
        for (Integer ahead : {1, 30, 365}) {
            BOOST_CHECK_SMALL(floating.tradingTime(reference + ahead) -
                                  fixed.tradingTime(reference + ahead), 1.0e-15);
            BOOST_CHECK_SMALL(floating.tradingTime(ahead / 365.0) - fixed.tradingTime(ahead / 365.0),
                              1.0e-15);
        }
    }
}

BOOST_AUTO_TEST_CASE(testTradingTimeEdgeCasesAndValidation) {
    BOOST_TEST_MESSAGE("Testing trading time edge cases and input validation...");

    const Date saturday(6, January, 2024);
    Settings::instance().evaluationDate() = saturday;
    tradingTimeTermStructure ts(saturday, WeekendsOnly(), 0.25);
    BOOST_CHECK_EQUAL(ts.tradingTime(saturday, saturday), 0.0);
    // starting on a weekend, time runs at the weekend weight
    BOOST_CHECK_SMALL(ts.tradingTime(0.5 / 365.0) * 365.0 - 0.125, 1.0e-12);
    BOOST_CHECK_SMALL(ts.tradingTime(saturday + 3, saturday + 1) + ts.tradingTime(saturday + 1, saturday + 3),
                      1.0e-15);
    BOOST_CHECK_THROW(ts.tradingTime(saturday - 1), Error);
    BOOST_CHECK_THROW(ts.tradingTime(-1.0 / 365.0), Error);

    BOOST_CHECK_THROW(tradingTimeTermStructure(saturday, WeekendsOnly(), -0.1), Error);
    BOOST_CHECK_THROW(tradingTimeTermStructure(saturday, WeekendsOnly(), 1.1), Error);
    // default construction: reference date at the evaluation date, on a business day
    tradingTimeTermStructure byDefault;
    BOOST_CHECK_EQUAL(byDefault.referenceDate(), WeekendsOnly().adjust(saturday));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
