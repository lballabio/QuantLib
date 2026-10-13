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
#include <ql/experimental/fxslv/fxlocalvolsurface.hpp>
#include <ql/experimental/fxslv/fxsettlementforeigncurve.hpp>
#include <ql/experimental/fxslv/fxvariancesurface.hpp>
#include <ql/experimental/fxslv/fxsmilesectionbystrike.hpp>
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
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancesurface.hpp>
#include <sstream>
#include <cmath>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(FxLocalVolTests)

// ─────────────────────────────────────────────────────────────────────────────
//  Shared market data used by all tests.
// ─────────────────────────────────────────────────────────────────────────────

namespace {

    // Smile sections for the local-vol and SLV tests.  Quadratic smiles are
    // a convenient approximation but fail in practice, notably in the wings
    // that Dupire and SLV depend on, so these tests use SABR (SVI also works).
    using TestSmileSection = FxSabrSmileSection;

    struct FxMarket {
        Date today;
        Date expiry3M;
        Date expiry6M;

        DayCounter dc;
        Handle<YieldTermStructure> eurTs; // foreign
        Handle<YieldTermStructure> usdTs; // domestic
        Handle<YieldTermStructure> eurFwdTs; // foreign, giving the process the settlement forward
        Handle<TradingTimeTermStructure> timeTs;

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

        ext::shared_ptr<fxVarianceSurfaceNCP<TestSmileSection>> surface;
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
            eurFwdTs = Handle<YieldTermStructure>(ext::make_shared<FxSettlementForeignCurve>(
                eurTs, usdTs, FxSettlementConvention(WeekendsOnly(), 2)));
            timeTs = Handle<TradingTimeTermStructure>(
                ext::make_shared<TradingTimeTermStructure>(today, WeekendsOnly(), 0.0));

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

            surface = ext::make_shared<fxVarianceSurfaceNCP<TestSmileSection>>(
                today, spot, pillars, atms, rrs, bfs, deltas,
                eurTs, usdTs, timeTs,
                deltaType, atmType, flyType,
                FxSettlementConvention(WeekendsOnly(), 2),
                WeekendsOnly(), Following, true);
            surface->enableExtrapolation();

            process = ext::make_shared<GeneralizedBlackScholesProcess>(
                spot, eurFwdTs, usdTs,
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

    FxLocalVolSurface lv(mkt.process->blackVolatility(),
                         mkt.process->riskFreeRate(),
                         mkt.process->dividendYield(),
                         mkt.process->stateVariable());
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

    FxLocalVolSurface lvDupire(mkt.process->blackVolatility(),
                               mkt.process->riskFreeRate(),
                               mkt.process->dividendYield(),
                               mkt.process->stateVariable());
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
    // the foreign curve giving processes the settlement forward
    const Handle<YieldTermStructure> eurFwdTs(ext::make_shared<FxSettlementForeignCurve>(
        eurTs, usdTs, FxSettlementConvention(calendar, 2)));

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
    auto timeTs = Handle<TradingTimeTermStructure>(
        ext::make_shared<TradingTimeTermStructure>(today, calendar, 0.0));

    auto volSurface = ext::make_shared<fxVarianceSurfaceNCP<TestSmileSection>>(
        today, spot, pillars, atms, rrs, bfs, deltas,
        eurTs, usdTs, timeTs,
        DeltaVolQuote::Fwd, DeltaVolQuote::AtmFwd, FxRrBfQuotes::SmileStrangle,
        FxSettlementConvention(calendar, 2),
        calendar, Following, true);
    volSurface->enableExtrapolation();
    Handle<BlackVolTermStructure> volHandle(volSurface);

    // ── Dupire local vol surface ─────────────────────────────────────────────
    auto localVol = ext::make_shared<FxLocalVolSurface>(volHandle, usdTs, eurFwdTs, spot);
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

    FxSLVPricingContext ctx(spot, usdTs, eurFwdTs, volHandle, localVolHandle, calendar);

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
        spot, eurFwdTs, usdTs, volHandle, localVolHandle);
    auto call4M_lv = ext::make_shared<VanillaOption>(payoff4M, exercise4M);
    call4M_lv->setPricingEngine(
        ext::make_shared<FdBlackScholesVanillaEngine>(process, 100, 100, 0,
                                                      FdmSchemeDesc::Douglas(), true));
    const Real lvNpv = call4M_lv->NPV();

    const Real relDiff = std::fabs(slvNpv - lvNpv) / lvNpv;
    BOOST_CHECK_MESSAGE(relDiff < 0.20,
        "SLV and local-vol 4M NPVs differ by " << relDiff * 100.0
        << "% (SLV=" << slvNpv << " LV=" << lvNpv << ")");
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceFollowsQuotes) {
    BOOST_TEST_MESSAGE("Testing that the variance surface follows its market quotes...");

    FxMarket m;
    // between the pillars, so that an interpolated smile is built and cached
    const Date d(15, May, 2024);
    const Time t = m.surface->timeFromReference(d);
    const Real strike = m.spot->value() * m.eurTs->discount(t) / m.usdTs->discount(t);
    const Volatility before = m.surface->blackVol(d, strike);

    Flag flag;
    flag.registerWith(m.surface);
    m.atmSQs[1]->setValue(m.atmSQs[1]->value() + 0.01);
    BOOST_CHECK(flag.isUp());
    const Volatility after = m.surface->blackVol(d, strike);
    BOOST_CHECK_MESSAGE(after > before + 0.001,
                        "vol did not follow the 6M ATM quote: " << before << " before, "
                                                                << after << " after");

    // the spot and the curves are observed too
    flag.lower();
    m.spotSQ->setValue(m.spotSQ->value() * 1.01);
    BOOST_CHECK(flag.isUp());
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceAtmVariance) {
    BOOST_TEST_MESSAGE("Testing ATM variance of the variance surface in trading time...");

    FxMarket m;
    auto tau = [&](const Date& d) { return m.timeTs->tradingTime(d); };
    const Real var3M = m.surface->atmVar(m.expiry3M), var6M = m.surface->atmVar(m.expiry6M);

    // at the pillars, the pillar sections' ATM vols over calendar time
    for (Size i = 0; i < m.pillars.size(); ++i) {
        TestSmileSection pillar(
            m.pillars[i], m.spot,
            ext::make_shared<FxRrBfQuotes>(m.atms[i], m.rrs[i], m.bfs[i], m.deltas, m.flyType),
            m.eurTs, m.usdTs, m.deltaType, m.atmType, Actual365Fixed(),
            FxSettlementConvention(WeekendsOnly(), 2), m.today);
        const Time T = m.dc.yearFraction(m.today, m.pillars[i]);
        BOOST_CHECK_SMALL(std::sqrt(m.surface->atmVar(m.pillars[i]) / T) - pillar.atmVol(), 1.0e-12);
    }

    // between them, linear in trading time
    const Date mid(15, May, 2024);
    const Real w = (tau(mid) - tau(m.expiry3M)) / (tau(m.expiry6M) - tau(m.expiry3M));
    BOOST_CHECK_SMALL(m.surface->atmVar(mid) - ((1.0 - w) * var3M + w * var6M), 1.0e-12);

    // before the first pillar, proportional to trading time, so flat over a weekend
    const Date saturday(3, February, 2024), monday(5, February, 2024);
    BOOST_CHECK_SMALL(m.surface->atmVar(saturday) - var3M * tau(saturday) / tau(m.expiry3M),
                      1.0e-12);
    BOOST_CHECK_SMALL(m.surface->atmVar(monday) - m.surface->atmVar(saturday), 1.0e-12);

    // after the last pillar, at the last pillar's rate per unit of trading time
    const Date later(2, October, 2024);
    BOOST_CHECK_SMALL(m.surface->atmVar(later) - var6M * tau(later) / tau(m.expiry6M), 1.0e-12);
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceReproducesPillars) {
    BOOST_TEST_MESSAGE("Testing that the variance surface reproduces its pillar smiles...");

    FxMarket m;
    for (Size i = 0; i < m.pillars.size(); ++i) {
        TestSmileSection pillar(
            m.pillars[i], m.spot,
            ext::make_shared<FxRrBfQuotes>(m.atms[i], m.rrs[i], m.bfs[i], m.deltas, m.flyType),
            m.eurTs, m.usdTs, m.deltaType, m.atmType, Actual365Fixed(),
            FxSettlementConvention(WeekendsOnly(), 2), m.today);
        for (Real strike : {1.00, 1.05, 1.085, 1.12, 1.17})
            BOOST_CHECK_SMALL(m.surface->blackVariance(m.pillars[i], strike) - pillar.variance(strike),
                              1.0e-12);
    }
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceInTradingTime) {
    BOOST_TEST_MESSAGE("Testing that the variance surface accrues variance in trading time...");

    FxMarket m;
    // at a given forward moneyness, nothing accrues over a weekend, which
    // has no trading time here, although the forward moves with the carry;
    // the two nodes are separate smile fits, equal up to fitting noise
    const Date saturday(3, February, 2024), monday(5, February, 2024);
    for (Real k : {0.93, 1.0, 1.07}) {
        const Real wSaturday = m.surface->blackVariance(saturday, k * m.surface->fwd(saturday));
        const Real wMonday = m.surface->blackVariance(monday, k * m.surface->fwd(monday));
        BOOST_CHECK_SMALL(wMonday - wSaturday, 1.0e-10);
    }

    // through the pillars and beyond, variance at a given forward moneyness
    // never decreases, up to the fitting noise between nodes, and grows at
    // a bounded rate; far in the wings of the first day's smile the rate
    // is high but finite
    const Time dt = 1.0e-4;
    for (Real k : {0.93, 1.0, 1.07}) {
        for (Time t = dt; t < 0.55; t += dt) {
            const Real w1 = m.surface->blackVariance(t, k * m.surface->fwd(t), true);
            const Real w2 = m.surface->blackVariance(t + dt, k * m.surface->fwd(t + dt), true);
            const Real rate = (w2 - w1) / dt;
            if (rate < -1.0e-7 || rate > 0.3) {
                BOOST_ERROR("moneyness " << k << ": variance rate " << rate << " at time " << t);
                break;
            }
        }
    }

    BOOST_CHECK_THROW(fxVarianceSurfaceNCP<TestSmileSection>(
                          m.today, m.spot, m.pillars, m.atms, m.rrs, m.bfs, m.deltas, m.eurTs,
                          m.usdTs, m.timeTs, m.deltaType, m.atmType, m.flyType,
                          FxSettlementConvention(WeekendsOnly(), 2), WeekendsOnly(), Following,
                          true, 0.0),
                      Error);
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceEventBetweenPillars) {
    BOOST_TEST_MESSAGE("Testing that the variance surface jumps at an event between pillars...");

    FxMarket m;
    const Date eventDate(10, April, 2024);  // a Wednesday between the pillars
    const Real timeOfDay = 13.0 / 24.0, size = 2.0;
    auto timeTs = Handle<TradingTimeTermStructure>(ext::make_shared<TradingTimeTermStructure>(
        m.today, WeekendsOnly(), 0.0,
        std::vector<ext::shared_ptr<TradingTimeEvent>>{ext::make_shared<JumpTradingTimeEvent>(
            eventDate, timeOfDay, makeQuoteHandle(size))}));
    fxVarianceSurfaceNCP<TestSmileSection> surface(
        m.today, m.spot, m.pillars, m.atms, m.rrs, m.bfs, m.deltas, m.eurTs, m.usdTs, timeTs,
        m.deltaType, m.atmType, m.flyType, FxSettlementConvention(WeekendsOnly(), 2));

    // the jump adds trading time at the rate variance accrues around it
    const Time te = (eventDate - m.today + timeOfDay) / 365.0, eps = 1.0e-7;
    auto w = [&](Time t) { return surface.blackVariance(t, surface.fwd(t)); };
    const Real jump = w(te + eps) - w(te - eps);
    const Time t0 = te - 0.5 / 365.0;  // earlier on the same day
    const Real rate = (w(te - eps) - w(t0)) / (timeTs->tradingTime(te - eps) - timeTs->tradingTime(t0));
    BOOST_CHECK_MESSAGE(std::fabs(jump / (rate * size / 365.0) - 1.0) < 1.0e-4,
                        "variance jump " << jump << " at the event, expected "
                                         << rate * size / 365.0);
    // and nothing elsewhere: just the ordinary accrual
    const Time t1 = te - 0.25 / 365.0;
    BOOST_CHECK_SMALL(w(t1) - w(t1 - eps) - rate * eps, 1.0e-12);
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceShortAndLongTenors) {
    BOOST_TEST_MESSAGE("Testing NCP interpolation at very short and very long tenors...");

    FxMarket m;
    auto q = [](Real v) { return makeQuoteHandle(v); };
    auto surfaceWith = [&](const std::vector<Date>& pillars, Real atm, Real rr25, Real bf25) {
        std::vector<Handle<Quote>> atms;
        std::vector<std::vector<Handle<Quote>>> rrs, bfs;
        for (Size i = 0; i < pillars.size(); ++i) {
            atms.push_back(q(atm));
            rrs.push_back({q(rr25), q(1.8 * rr25)});
            bfs.push_back({q(bf25), q(3.2 * bf25)});
        }
        return fxVarianceSurfaceNCP<TestSmileSection>(
            m.today, m.spot, pillars, atms, rrs, bfs, m.deltas, m.eurTs, m.usdTs, m.timeTs,
            m.deltaType, m.atmType, m.flyType, FxSettlementConvention(WeekendsOnly(), 2));
    };

    // between O/N and 1W at 7%, the std dev at the nodes is below 0.01
    const auto shortEnd = surfaceWith({m.today + 1, m.today + 7}, 0.07, -0.005, 0.002);
    for (Integer days : {3, 5}) {
        const Date d = m.today + days;
        const Volatility vol = shortEnd.blackVol(d, shortEnd.fwd(d));
        BOOST_CHECK_MESSAGE(vol > 0.05 && vol < 0.10, "vol " << vol << " after " << days << " days");
    }

    // between 1Y and 5Y at 35%, the std dev at the nodes is above 0.5
    const auto longEnd = surfaceWith({Date(2, January, 2025), Date(2, January, 2029)}, 0.35, -0.03, 0.01);
    const Date d(3, January, 2027);
    const Volatility vol = longEnd.blackVol(d, longEnd.fwd(d));
    BOOST_CHECK_MESSAGE(vol > 0.25 && vol < 0.45, "vol " << vol << " after three years");
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceTradingTimeReference) {
    BOOST_TEST_MESSAGE("Testing that the variance surface and its trading time share a reference date...");

    FxMarket m;
    auto surfaceWith = [&](const Handle<TradingTimeTermStructure>& timeTs) {
        return fxVarianceSurfaceNCP<TestSmileSection>(
            m.today, m.spot, m.pillars, m.atms, m.rrs, m.bfs, m.deltas, m.eurTs, m.usdTs, timeTs,
            m.deltaType, m.atmType, m.flyType, FxSettlementConvention(WeekendsOnly(), 2));
    };
    const Date d(15, May, 2024);

    // a fixed trading-time curve from another date
    const auto shifted = surfaceWith(Handle<TradingTimeTermStructure>(
        ext::make_shared<TradingTimeTermStructure>(m.today - 3, WeekendsOnly(), 0.0)));
    BOOST_CHECK_THROW(shifted.blackVariance(d, m.spot->value()), Error);

    // a floating one agrees until the evaluation date moves
    const auto floating = surfaceWith(Handle<TradingTimeTermStructure>(
        ext::make_shared<TradingTimeTermStructure>(WeekendsOnly(), 0.0)));
    const Real before = floating.blackVariance(d, m.spot->value());
    BOOST_CHECK_SMALL(before - m.surface->blackVariance(d, m.spot->value()), 1.0e-15);
    // moving the evaluation date must not throw; using the surface then does
    BOOST_CHECK_NO_THROW(Settings::instance().evaluationDate() = m.today + 1);
    BOOST_CHECK_THROW(floating.blackVariance(d, m.spot->value()), Error);
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceSettlementForward) {
    BOOST_TEST_MESSAGE("Testing that the variance surface uses settlement forwards throughout...");

    FxMarket m;
    const FxSettlementConvention settlement(WeekendsOnly(), 2);

    // at the pillars, the pillar sections' own forwards
    for (Size i = 0; i < m.pillars.size(); ++i) {
        TestSmileSection pillar(
            m.pillars[i], m.spot,
            ext::make_shared<FxRrBfQuotes>(m.atms[i], m.rrs[i], m.bfs[i], m.deltas, m.flyType),
            m.eurTs, m.usdTs, m.deltaType, m.atmType, Actual365Fixed(), settlement, m.today);
        BOOST_CHECK_CLOSE(m.surface->fwd(m.pillars[i]), pillar.forward(), 1.0e-12);
    }

    // a Friday expiry delivers on Tuesday, two days later than the time
    // to expiry alone would give
    const Date friday(12, April, 2024);
    const Date spotDate = settlement.spotDate(m.today), delivery = settlement.deliveryDate(friday);
    BOOST_CHECK_EQUAL(delivery, Date(16, April, 2024));
    const Real expected = m.spot->value() *
                          (m.eurTs->discount(delivery) / m.eurTs->discount(spotDate)) /
                          (m.usdTs->discount(delivery) / m.usdTs->discount(spotDate));
    BOOST_CHECK_CLOSE(m.surface->fwd(friday), expected, 1.0e-12);
    const Time tFriday = m.surface->timeFromReference(friday);
    BOOST_CHECK_CLOSE(m.surface->fwd(tFriday), expected, 1.0e-12);
    // every option expiring on Friday delivers on Tuesday: the forward
    // holds through the day and steps at midnight
    BOOST_CHECK_CLOSE(m.surface->fwd(tFriday + 0.5 / 365.0), expected, 1.0e-12);
    BOOST_CHECK_CLOSE(m.surface->fwd(tFriday + 1.0 / 365.0), m.surface->fwd(friday + 1), 1.0e-12);

    // curves referenced to the spot date give the same surface
    const Handle<YieldTermStructure> eurFromSpot(
        ext::make_shared<FlatForward>(2, WeekendsOnly(), 0.04, m.dc));
    const Handle<YieldTermStructure> usdFromSpot(
        ext::make_shared<FlatForward>(2, WeekendsOnly(), 0.0525, m.dc));
    BOOST_REQUIRE_EQUAL(eurFromSpot->referenceDate(), spotDate);
    fxVarianceSurfaceNCP<TestSmileSection> fromSpot(
        m.today, m.spot, m.pillars, m.atms, m.rrs, m.bfs, m.deltas, eurFromSpot, usdFromSpot,
        m.timeTs, m.deltaType, m.atmType, m.flyType, settlement);
    for (const Date& d : {Date(20, February, 2024), friday, Date(15, May, 2024)}) {
        BOOST_CHECK_CLOSE(fromSpot.fwd(d), m.surface->fwd(d), 1.0e-12);
        for (Real k : {0.95, 1.0, 1.05})
            BOOST_CHECK_CLOSE(fromSpot.blackVol(d, k * fromSpot.fwd(d)),
                              m.surface->blackVol(d, k * m.surface->fwd(d)), 1.0e-9);
    }
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceNodeSpacing) {
    BOOST_TEST_MESSAGE("Testing variance surface nodes within a day...");

    FxMarket m;
    auto surfaceWith = [&](Time spacing) {
        return fxVarianceSurfaceNCP<TestSmileSection>(
            m.today, m.spot, m.pillars, m.atms, m.rrs, m.bfs, m.deltas, m.eurTs, m.usdTs,
            m.timeTs, m.deltaType, m.atmType, m.flyType, FxSettlementConvention(WeekendsOnly(), 2),
            WeekendsOnly(), Following, true, spacing);
    };
    BOOST_CHECK_NO_THROW(surfaceWith(2.0 / 365.0));
#ifdef QL_HIGH_RESOLUTION_DATE
    // nodes every six hours: a same-day expiry, and agreement with daily
    // nodes up to the noise of refitting a smile at every node
    const auto sixHourly = surfaceWith(0.25 / 365.0);
    const Date sameDay(2, January, 2024, 16, 0, 0);
    const Volatility sameDayVol = sixHourly.blackVol(sameDay, sixHourly.fwd(sameDay));
    BOOST_CHECK_MESSAGE(sameDayVol > 0.05 && sameDayVol < 0.20,
                        "same-day vol " << sameDayVol);
    const Date midday(15, May, 2024, 12, 0, 0);
    for (Real k : {0.95, 1.0, 1.05})
        BOOST_CHECK_CLOSE(sixHourly.blackVariance(midday, k * sixHourly.fwd(midday)),
                          m.surface->blackVariance(midday, k * m.surface->fwd(midday)), 0.1);
#else
    // a node within a day needs a date with a time
    BOOST_CHECK_THROW(surfaceWith(0.25 / 365.0), Error);
#endif
}

BOOST_AUTO_TEST_CASE(testFxSettlementForeignCurve) {
    BOOST_TEST_MESSAGE("Testing that the settlement foreign curve gives processes the settlement forward...");

    FxMarket m;
    // the process forward is the surface's settlement forward, through the
    // day and across weekends
    auto processForward = [&](Time t) {
        return m.spot->value() * m.process->dividendYield()->discount(t) /
               m.process->riskFreeRate()->discount(t);
    };
    BOOST_CHECK_CLOSE(processForward(0.0), m.spot->value(), 1.0e-12);
    for (const Date& d : {Date(4, January, 2024), Date(12, April, 2024), Date(13, April, 2024),
                          Date(15, April, 2024), m.expiry3M, Date(2, January, 2025)}) {
        const Time t = m.surface->timeFromReference(d);
        for (Real hours : {0.0, 9.0, 23.0}) {
            const Time th = t + hours / 24.0 / 365.0;
            BOOST_CHECK_CLOSE(processForward(th), m.surface->fwd(th), 1.0e-12);
        }
        BOOST_CHECK_CLOSE(processForward(t), m.surface->fwd(d), 1.0e-12);
    }

    // it follows its curves
    RelinkableHandle<YieldTermStructure> foreign(m.eurTs.currentLink());
    auto curve = ext::make_shared<FxSettlementForeignCurve>(foreign, m.usdTs,
                                                           FxSettlementConvention(WeekendsOnly(), 2));
    const Time t1Y = m.surface->timeFromReference(Date(2, January, 2025));
    const Real before = curve->discount(t1Y);
    Flag flag;
    flag.registerWith(curve);
    foreign.linkTo(ext::make_shared<FlatForward>(0, NullCalendar(), 0.05, m.dc));
    BOOST_CHECK(flag.isUp());
    BOOST_CHECK(curve->discount(t1Y) < before);
}

BOOST_AUTO_TEST_CASE(testFxLocalVolSurfaceWithoutWeekendVariance) {
    BOOST_TEST_MESSAGE("Testing Dupire local vol where trading time stops over weekends...");

    FxMarket m;
    FxLocalVolSurface lv(m.process->blackVolatility(), m.process->riskFreeRate(),
                         m.process->dividendYield(), m.process->stateVariable());
    lv.enableExtrapolation();

    // defined everywhere, zero exactly over weekends, positive otherwise
    Size weekend = 0, weekday = 0;
    for (Real strike : {1.03, 1.06, 1.085, 1.11, 1.14}) {
        for (Size i = 1; i < 365; ++i) {
            const Time t = (i + 0.5) / 4.0 / 365.0;
            const Date d = m.today + Integer(std::floor(t * 365.0));
            Volatility vol = Null<Volatility>();
            BOOST_REQUIRE_NO_THROW(vol = lv.localVol(t, strike, true));
            if (d.weekday() == Saturday || d.weekday() == Sunday) {
                BOOST_CHECK_EQUAL(vol, 0.0);
                ++weekend;
            } else {
                BOOST_CHECK(vol > 0.0);
                ++weekday;
            }
        }
    }
    BOOST_CHECK(weekend > 0 && weekday > 0);

    // on weekdays, QuantLib's Dupire on the same inputs agrees
    LocalVolSurface upstream(m.process->blackVolatility(), m.process->riskFreeRate(),
                             m.process->dividendYield(), m.process->stateVariable());
    upstream.enableExtrapolation();
    for (Real strike : {1.06, 1.085, 1.11}) {
        const Time t = m.surface->timeFromReference(Date(14, February, 2024)) + 0.3 / 365.0;
        BOOST_CHECK_CLOSE(lv.localVol(t, strike, true), upstream.localVol(t, strike, true), 1.0e-10);
    }

    // a floor, where given, applies over weekends too
    FxLocalVolSurface floored(m.process->blackVolatility(), m.process->riskFreeRate(),
                              m.process->dividendYield(), m.process->stateVariable(), 1.0e-7, 0.01);
    floored.enableExtrapolation();
    const Time saturday = m.surface->timeFromReference(Date(3, February, 2024)) + 0.5 / 365.0;
    BOOST_CHECK_EQUAL(floored.localVol(saturday, 1.085, true), 0.01);
}

BOOST_AUTO_TEST_CASE(testFxLocalVolSurfaceArbitrage) {
    BOOST_TEST_MESSAGE("Testing that Dupire local vol fails on arbitrage, saying so...");

    const Date today(2, January, 2024);
    Settings::instance().evaluationDate() = today;
    const DayCounter dc = Actual365Fixed();
    const Handle<YieldTermStructure> rates(ext::make_shared<FlatForward>(today, 0.0, dc));
    const Handle<Quote> spot = makeQuoteHandle(1.0);

    // total variance falling between two expiries
    auto falling = ext::make_shared<BlackVarianceCurve>(
        today, std::vector<Date>{today + 90, today + 180},
        std::vector<Volatility>{0.20, 0.10}, dc, false);
    FxLocalVolSurface calendar(Handle<BlackVolTermStructure>(falling), rates, rates, spot);
    try {
        calendar.localVol(120.0 / 365.0, 1.0, true);
        BOOST_ERROR("no error on calendar arbitrage");
    } catch (Error& e) {
        BOOST_CHECK_MESSAGE(std::string(e.what()).find("calendar arbitrage") != std::string::npos,
                            "unexpected error: " << e.what());
    }

    // a smile too peaked to have a positive density
    Matrix vols(3, 2);
    vols[0][0] = vols[0][1] = 0.05;
    vols[1][0] = vols[1][1] = 0.60;
    vols[2][0] = vols[2][1] = 0.05;
    auto peaked = ext::make_shared<BlackVarianceSurface>(
        today, NullCalendar(), std::vector<Date>{today + 90, today + 180},
        std::vector<Real>{0.9, 1.0, 1.1}, vols, dc);
    FxLocalVolSurface butterfly(Handle<BlackVolTermStructure>(peaked), rates, rates, spot);
    try {
        butterfly.localVol(120.0 / 365.0, 1.0, true);
        BOOST_ERROR("no error on butterfly arbitrage");
    } catch (Error& e) {
        BOOST_CHECK_MESSAGE(std::string(e.what()).find("butterfly arbitrage") != std::string::npos,
                            "unexpected error: " << e.what());
    }
}

BOOST_AUTO_TEST_CASE(testFxVanillaBumpRiskLocalVolDefined) {
    BOOST_TEST_MESSAGE("Testing that the bump-risk local vol needs no substitute values...");

    FxMarket m;
    FxVanillaBumpRisk riskCalc(m.makeAtmCall(), m.process, m.spotSQ, m.atmSQs);
    std::vector<Time> times;
    std::vector<Real> strikes;
    for (Size i = 1; i <= 20; ++i)
        times.push_back(0.25 * i / 20.0);
    for (Size j = 0; j < 15; ++j)
        strikes.push_back(1.00 + 0.012 * j);
    std::ostringstream out;
    riskCalc.printLocalVolComparison(times, strikes, out);
    BOOST_CHECK_MESSAGE(out.str().find("defined on the whole grid") != std::string::npos,
                        "Dupire failed somewhere:\n" << out.str());
}

BOOST_AUTO_TEST_CASE(testTradingTimeFormsAgree) {
    BOOST_TEST_MESSAGE("Testing that trading time from dates and from times agree...");

    const Date today(5, January, 2024);  // a Friday
    Settings::instance().evaluationDate() = today;
    for (const Calendar& calendar : {Calendar(WeekendsOnly()), Calendar(TARGET())}) {
        for (Real w : {0.0, 0.3, 1.0}) {
            TradingTimeTermStructure ts(today, calendar, w);
            // every day for seven years, beyond any fixed holiday horizon
            for (Date d = today; d <= today + 7 * Years; ++d) {
                const Time byDate = ts.tradingTime(d);
                const Time byTime = ts.tradingTime(ts.timeFromReference(d));
                if (std::fabs(byDate - byTime) > 1.0e-14) {
                    BOOST_ERROR(calendar.name() << ", holiday weight " << w << ", " << d
                                                << ": " << byDate << " from the date, " << byTime
                                                << " from the time");
                    break;
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testTradingTimeIsMonotonicAndContinuous) {
    BOOST_TEST_MESSAGE("Testing that trading time runs at rate 1 on business days and the holiday weight otherwise...");

    const Date today(5, January, 2024);
    Settings::instance().evaluationDate() = today;
    for (Real w : {0.0, 0.3, 1.0}) {
        TradingTimeTermStructure ts(today, TARGET(), w);
        // in tenths of a day over two months, including weekends and New Year
        const Real step = 0.1 / 365.0;
        Time previous = ts.tradingTime(0.0);
        BOOST_CHECK_EQUAL(previous, 0.0);
        for (Size i = 1; i <= 600; ++i) {
            const Time t = i * step, tt = ts.tradingTime(t);
            const Real rate = (tt - previous) / step;
            if (rate < w - 1.0e-9 || rate > 1.0 + 1.0e-9) {
                BOOST_ERROR("holiday weight " << w << ": rate " << rate << " at "
                                              << t * 365.0 << " days");
                break;
            }
            previous = tt;
        }
        if (w == 1.0)
            BOOST_CHECK_SMALL(ts.tradingTime(3.0) - 3.0, 1.0e-14);
    }

    // with weekends not counted, a week has five trading days whatever its first day
    TradingTimeTermStructure ts(today, WeekendsOnly(), 0.0);
    for (Integer start = 0; start < 7; ++start)
        BOOST_CHECK_SMALL(ts.tradingTime(today + start, today + start + 7) * 365.0 - 5.0, 1.0e-12);
}

BOOST_AUTO_TEST_CASE(testTradingTimeEventJumps) {
    BOOST_TEST_MESSAGE("Testing trading time jumps at events...");

    const Date today(5, January, 2024);  // a Friday
    Settings::instance().evaluationDate() = today;
    const Date eventDate(10, January, 2024);  // the next Wednesday
    const Real timeOfDay = 13.5 / 24.0;
    auto size = ext::make_shared<SimpleQuote>(2.0);
    TradingTimeTermStructure plain(today, WeekendsOnly(), 0.0);
    TradingTimeTermStructure ts(
        today, WeekendsOnly(), 0.0,
        {ext::make_shared<JumpTradingTimeEvent>(eventDate, timeOfDay, Handle<Quote>(size)),
         ext::make_shared<JumpTradingTimeEvent>(Date(4, January, 2024), 0.5,
                                                makeQuoteHandle(5.0))});

    // the jump before the reference date has happened; the other is at its time
    const Time te = (eventDate - today + timeOfDay) / 365.0;
    const auto intervals = ts.eventIntervals();
    BOOST_REQUIRE_EQUAL(intervals.size(), Size(1));
    BOOST_CHECK_SMALL(intervals[0].first - te, 1.0e-12);
    BOOST_CHECK_EQUAL(intervals[0].first, intervals[0].second);
    const Time justBefore = te - 1.0e-9, justAfter = te + 1.0e-9;
    BOOST_CHECK_SMALL(ts.tradingTime(justBefore) - plain.tradingTime(justBefore), 1.0e-15);
    BOOST_CHECK_SMALL(ts.tradingTime(justAfter) - plain.tradingTime(justAfter) - 2.0 / 365.0,
                      1.0e-15);
    // by date: not yet at the start of the event day, included the day after
    BOOST_CHECK_SMALL(ts.tradingTime(eventDate) - plain.tradingTime(eventDate), 1.0e-15);
    BOOST_CHECK_SMALL(ts.tradingTime(eventDate + 1) - plain.tradingTime(eventDate + 1) - 2.0 / 365.0,
                      1.0e-15);

    // the jump follows its quote, and the change is notified
    auto observed = ext::make_shared<TradingTimeTermStructure>(
        today, WeekendsOnly(), 0.0,
        std::vector<ext::shared_ptr<TradingTimeEvent>>{
            ext::make_shared<JumpTradingTimeEvent>(eventDate, timeOfDay, Handle<Quote>(size))});
    Flag flag;
    flag.registerWith(observed);
    size->setValue(3.0);
    BOOST_CHECK(flag.isUp());
    BOOST_CHECK_SMALL(observed->tradingTime(eventDate + 1) - plain.tradingTime(eventDate + 1) -
                          3.0 / 365.0, 1.0e-15);
    // a jump cannot take trading time away
    size->setValue(-1.0);
    BOOST_CHECK_THROW(observed->tradingTime(eventDate + 1), Error);
    size->setValue(1.0);
    BOOST_CHECK_NO_THROW(observed->tradingTime(eventDate + 1));
}

BOOST_AUTO_TEST_CASE(testTradingTimeEventsNeedExplicitTimes) {
    BOOST_TEST_MESSAGE("Testing that trading-time events need explicit times of day...");

    const Date eventDate(10, January, 2024);
    const Handle<Quote> w = makeQuoteHandle(1.0);
    // a jump: no time, or midnight, is not accepted
    BOOST_CHECK_THROW(JumpTradingTimeEvent(eventDate, w), Error);
    BOOST_CHECK_THROW(JumpTradingTimeEvent(eventDate, 0.0, w), Error);
    BOOST_CHECK_THROW(JumpTradingTimeEvent(eventDate, 1.0, w), Error);
    BOOST_CHECK_THROW(JumpTradingTimeEvent(eventDate, 0.5, Handle<Quote>()), Error);
    const JumpTradingTimeEvent jump(eventDate, 0.25, w);
    BOOST_CHECK_EQUAL(jump.start(), eventDate.serialNumber() + 0.25);
    BOOST_CHECK_EQUAL(jump.end(), jump.start());

    // a diffuse event: midnight given explicitly is fine, and it must end after it starts
    const DiffuseTradingTimeEvent wholeDay(eventDate, 0.0, eventDate + 1, 0.0, w);
    BOOST_CHECK_EQUAL(wholeDay.end() - wholeDay.start(), 1.0);
    BOOST_CHECK_THROW(DiffuseTradingTimeEvent(eventDate, 0.5, eventDate, 0.5, w), Error);
    BOOST_CHECK_THROW(DiffuseTradingTimeEvent(eventDate, 0.5, eventDate, 0.4, w), Error);
    BOOST_CHECK_THROW(DiffuseTradingTimeEvent(eventDate, 1.0, eventDate + 1, 0.0, w), Error);
    BOOST_CHECK_THROW(DiffuseTradingTimeEvent(eventDate, 0.0, eventDate, -0.1, w), Error);
    BOOST_CHECK_THROW(DiffuseTradingTimeEvent(eventDate, 0.5, eventDate, 0.6, Handle<Quote>()),
                      Error);
    BOOST_CHECK_THROW(DiffuseTradingTimeEvent(eventDate, eventDate + 1, w), Error);

    BOOST_CHECK_THROW(TradingTimeTermStructure(eventDate, WeekendsOnly(), 0.0,
                                               {ext::shared_ptr<TradingTimeEvent>()}),
                      Error);
#ifdef QL_HIGH_RESOLUTION_DATE
    // a date carrying a time gives it; giving it twice is ambiguous
    const Date at1330(10, January, 2024, 13, 30, 0), at1500(10, January, 2024, 15, 0, 0);
    const JumpTradingTimeEvent fromDate(at1330, w);
    BOOST_CHECK_SMALL(fromDate.start() - (eventDate.serialNumber() + 13.5 / 24.0), 1.0e-9);
    BOOST_CHECK_THROW(JumpTradingTimeEvent(at1330, 0.5, w), Error);
    BOOST_CHECK_THROW(JumpTradingTimeEvent(Date(10, January, 2024, 0, 0, 0), w), Error);
    const DiffuseTradingTimeEvent fromDates(at1330, at1500, w);
    BOOST_CHECK_SMALL(fromDates.end() - fromDates.start() - 1.5 / 24.0, 1.0e-9);
    BOOST_CHECK_THROW(DiffuseTradingTimeEvent(at1500, at1330, w), Error);
    BOOST_CHECK_THROW(DiffuseTradingTimeEvent(at1330, 0.5, eventDate + 1, 0.0, w), Error);
    BOOST_CHECK_THROW(DiffuseTradingTimeEvent(eventDate, at1500, w), Error);
#endif
}

BOOST_AUTO_TEST_CASE(testTradingTimeRateDecisionWithPressConference) {
    BOOST_TEST_MESSAGE("Testing trading time over a rate decision and its press conference...");

    const Date today(5, January, 2024);  // a Friday
    Settings::instance().evaluationDate() = today;
    const Date meeting(10, January, 2024);  // a Wednesday
    const Real decision = 14.0 / 24.0, pressStart = 14.5 / 24.0, pressEnd = 15.5 / 24.0;
    auto jumpWeight = ext::make_shared<SimpleQuote>(1.0);
    auto pressWeight = ext::make_shared<SimpleQuote>(0.5);
    TradingTimeTermStructure plain(today, WeekendsOnly(), 0.0);
    auto ts = ext::make_shared<TradingTimeTermStructure>(
        today, WeekendsOnly(), 0.0,
        std::vector<ext::shared_ptr<TradingTimeEvent>>{
            ext::make_shared<JumpTradingTimeEvent>(meeting, decision, Handle<Quote>(jumpWeight)),
            ext::make_shared<DiffuseTradingTimeEvent>(meeting, pressStart, meeting, pressEnd,
                                                      Handle<Quote>(pressWeight))});

    // extra trading days over the plain curve, at a time of the meeting day
    auto extra = [&](Real timeOfDay) {
        const Time t = (meeting - today + timeOfDay) / 365.0;
        return (ts->tradingTime(t) - plain.tradingTime(t)) * 365.0;
    };
    const Real eps = 1.0e-9;
    BOOST_CHECK_SMALL(extra(decision - eps), 1.0e-9);
    BOOST_CHECK_SMALL(extra(decision + eps) - 1.0, 1.0e-9);
    BOOST_CHECK_SMALL(extra(pressStart) - 1.0, 1.0e-9);
    BOOST_CHECK_SMALL(extra(15.0 / 24.0) - 1.25, 1.0e-9);
    BOOST_CHECK_SMALL(extra(pressEnd) - 1.5, 1.0e-9);
    BOOST_CHECK_SMALL(extra(20.0 / 24.0) - 1.5, 1.0e-9);
    BOOST_CHECK_SMALL((ts->tradingTime(meeting + 30) - plain.tradingTime(meeting + 30)) * 365.0 - 1.5,
                      1.0e-9);

    const auto intervals = ts->eventIntervals();
    BOOST_REQUIRE_EQUAL(intervals.size(), Size(2));
    BOOST_CHECK_SMALL(intervals[1].first * 365.0 - (meeting - today + pressStart), 1.0e-9);
    BOOST_CHECK_SMALL(intervals[1].second * 365.0 - (meeting - today + pressEnd), 1.0e-9);

    // the two forms agree with events present
    for (Date d = today; d <= today + 30; ++d)
        BOOST_CHECK_SMALL(ts->tradingTime(d) - ts->tradingTime(ts->timeFromReference(d)), 1.0e-14);

    // both weights follow their quotes, with notification
    Flag flag;
    flag.registerWith(ts);
    pressWeight->setValue(1.0);
    BOOST_CHECK(flag.isUp());
    BOOST_CHECK_SMALL(extra(15.0 / 24.0) - 1.5, 1.0e-9);
    flag.lower();
    jumpWeight->setValue(2.0);
    BOOST_CHECK(flag.isUp());
    BOOST_CHECK_SMALL(extra(15.0 / 24.0) - 2.5, 1.0e-9);
}

BOOST_AUTO_TEST_CASE(testTradingTimeDiffuseEvents) {
    BOOST_TEST_MESSAGE("Testing trading time with diffuse events over whole days...");

    const Date today(18, December, 2023);  // a Monday
    Settings::instance().evaluationDate() = today;
    const Date christmasEve(22, December, 2023), quietStart(25, December, 2023);
    auto halfDay = ext::make_shared<SimpleQuote>(-0.5);
    const Real w = 0.5;
    auto wholeDays = [](const Date& start, const Date& end, const Handle<Quote>& weight) {
        return ext::make_shared<DiffuseTradingTimeEvent>(start, 0.0, end, 0.0, weight);
    };
    TradingTimeTermStructure plain(today, WeekendsOnly(), w);
    TradingTimeTermStructure ts(
        today, WeekendsOnly(), w,
        {wholeDays(christmasEve, christmasEve + 1, Handle<Quote>(halfDay)),
         wholeDays(quietStart, quietStart + 7, makeQuoteHandle(-2.8))});
    auto days = [](const TradingTimeTermStructure& c, const Date& d1, const Date& d2) {
        return c.tradingTime(d1, d2) * 365.0;
    };

    // a half day counts half, through the day
    BOOST_CHECK_SMALL(days(ts, christmasEve, christmasEve + 1) - 0.5, 1.0e-9);
    BOOST_CHECK_SMALL(ts.tradingTime(ts.timeFromReference(christmasEve) + 0.5 / 365.0) -
                          ts.tradingTime(christmasEve) - 0.25 / 365.0, 1.0e-12);
    // a quiet week takes 0.4 days from each of its seven days
    BOOST_CHECK_SMALL(days(ts, quietStart, quietStart + 7) - (days(plain, quietStart, quietStart + 7) - 2.8),
                      1.0e-9);
    BOOST_CHECK_SMALL(days(ts, quietStart, quietStart + 1) - 0.6, 1.0e-9);
    // elsewhere nothing changes
    BOOST_CHECK_SMALL(days(ts, today, christmasEve) - days(plain, today, christmasEve), 1.0e-9);
    for (Date d = today; d <= today + 60; ++d)
        BOOST_CHECK_SMALL(ts.tradingTime(d) - ts.tradingTime(ts.timeFromReference(d)), 1.0e-14);

    // the weight follows its quote, and the change is notified
    auto observed = ext::make_shared<TradingTimeTermStructure>(
        today, WeekendsOnly(), w,
        std::vector<ext::shared_ptr<TradingTimeEvent>>{
            wholeDays(christmasEve, christmasEve + 1, Handle<Quote>(halfDay))});
    BOOST_CHECK_SMALL(days(*observed, christmasEve, christmasEve + 1) - 0.5, 1.0e-9);
    Flag flag;
    flag.registerWith(observed);
    halfDay->setValue(0.5);
    BOOST_CHECK(flag.isUp());
    BOOST_CHECK_SMALL(days(*observed, christmasEve, christmasEve + 1) - 1.5, 1.0e-9);
    // more than the day's own rate cannot be taken away
    halfDay->setValue(-1.5);
    BOOST_CHECK_THROW(observed->tradingTime(christmasEve + 1), Error);
    // the whole day exactly can
    halfDay->setValue(-1.0);
    BOOST_CHECK_SMALL(days(*observed, christmasEve, christmasEve + 1), 1.0e-9);
}

BOOST_AUTO_TEST_CASE(testTradingTimeNegativeWeightsNeedCover) {
    BOOST_TEST_MESSAGE("Testing that negative event weights need trading time to take from...");

    const Date today(5, January, 2024);  // a Friday
    Settings::instance().evaluationDate() = today;
    const Date saturday(6, January, 2024);
    const auto negative = ext::make_shared<DiffuseTradingTimeEvent>(
        saturday, 10.0 / 24.0, saturday, 12.0 / 24.0, makeQuoteHandle(-0.05));
    // alone on a weekend not counted, it would run trading time backwards
    TradingTimeTermStructure alone(today, WeekendsOnly(), 0.0, {negative});
    BOOST_CHECK_THROW(alone.tradingTime(today + 7), Error);
    // overlapped by an event giving at least as much, it is accepted
    const auto cover = ext::make_shared<DiffuseTradingTimeEvent>(
        saturday, 9.0 / 24.0, saturday, 13.0 / 24.0, makeQuoteHandle(0.1));
    TradingTimeTermStructure covered(today, WeekendsOnly(), 0.0, {negative, cover});
    BOOST_CHECK_SMALL(covered.tradingTime(today + 7) * 365.0 - 5.05, 1.0e-9);
    // a cover that is too small, or misses part of it, is not enough
    TradingTimeTermStructure tooSmall(
        today, WeekendsOnly(), 0.0,
        {negative, ext::make_shared<DiffuseTradingTimeEvent>(saturday, 9.0 / 24.0, saturday,
                                                             13.0 / 24.0, makeQuoteHandle(0.09))});
    BOOST_CHECK_THROW(tooSmall.tradingTime(today + 7), Error);
    TradingTimeTermStructure partial(
        today, WeekendsOnly(), 0.0,
        {negative, ext::make_shared<DiffuseTradingTimeEvent>(saturday, 11.0 / 24.0, saturday,
                                                             13.0 / 24.0, makeQuoteHandle(1.0))});
    BOOST_CHECK_THROW(partial.tradingTime(today + 7), Error);
    // a negative event already over by the reference date is not checked
    TradingTimeTermStructure past(saturday + 1, WeekendsOnly(), 0.0, {negative});
    BOOST_CHECK_NO_THROW(past.tradingTime(saturday + 8));
}

BOOST_AUTO_TEST_CASE(testTradingTimeEventInProgress) {
    BOOST_TEST_MESSAGE("Testing trading time with an event in progress at the reference date...");

    const Date today(10, January, 2024);  // a Wednesday
    Settings::instance().evaluationDate() = today;
    // released evenly from yesterday to tomorrow, so half of it by today
    TradingTimeTermStructure plain(today, WeekendsOnly(), 0.0);
    TradingTimeTermStructure ts(today, WeekendsOnly(), 0.0,
                                {ext::make_shared<DiffuseTradingTimeEvent>(
                                    today - 1, 0.0, today + 1, 0.0, makeQuoteHandle(1.0))});
    BOOST_CHECK_SMALL((ts.tradingTime(today + 1) - plain.tradingTime(today + 1)) * 365.0 - 0.5,
                      1.0e-9);
    BOOST_CHECK_SMALL((ts.tradingTime(0.5 / 365.0) - plain.tradingTime(0.5 / 365.0)) * 365.0 - 0.25,
                      1.0e-9);
    const auto intervals = ts.eventIntervals();
    BOOST_REQUIRE_EQUAL(intervals.size(), Size(1));
    BOOST_CHECK_EQUAL(intervals[0].first, 0.0);
    BOOST_CHECK_SMALL(intervals[0].second * 365.0 - 1.0, 1.0e-9);

    // once over, it is gone
    TradingTimeTermStructure later(today + 1, WeekendsOnly(), 0.0, ts.events());
    BOOST_CHECK(later.eventIntervals().empty());
}

#ifdef QL_HIGH_RESOLUTION_DATE
BOOST_AUTO_TEST_CASE(testTradingTimeIntradayReference) {
    BOOST_TEST_MESSAGE("Testing trading time from a reference date carrying a time of day...");

    const Date day(10, January, 2024);  // a Wednesday
    const Date now(10, January, 2024, 14, 0, 0);
    const std::vector<ext::shared_ptr<TradingTimeEvent>> events = {
        ext::make_shared<JumpTradingTimeEvent>(day, 10.0 / 24.0, makeQuoteHandle(1.0)),
        ext::make_shared<JumpTradingTimeEvent>(day, 16.0 / 24.0, makeQuoteHandle(2.0)),
        ext::make_shared<DiffuseTradingTimeEvent>(day, 13.0 / 24.0, day, 15.0 / 24.0,
                                                  makeQuoteHandle(4.0))};

    Settings::instance().evaluationDate() = now;
    TradingTimeTermStructure fixed(now, WeekendsOnly(), 0.0, events);
    TradingTimeTermStructure floating(WeekendsOnly(), 0.0, events);
    TradingTimeTermStructure fromDay(day, WeekendsOnly(), 0.0, events);
    for (const TradingTimeTermStructure* ts : {&fixed, &floating}) {
        BOOST_CHECK_EQUAL(ts->referenceDate(), now);
        // the 10:00 jump has happened; half the diffuse event is left, and the 16:00 jump
        BOOST_CHECK_EQUAL(ts->eventIntervals().size(), Size(2));
        BOOST_CHECK_SMALL(ts->tradingTime(day + 1) * 365.0 - (10.0 / 24.0 + 2.0 + 2.0), 1.0e-9);
        BOOST_CHECK_SMALL(ts->tradingTime(1.0 / 24.0 / 365.0) * 365.0 - (1.0 / 24.0 + 2.0),
                          1.0e-9);
        BOOST_CHECK_SMALL(ts->tradingTime(Date(10, January, 2024, 16, 0, 1)) * 365.0 -
                              (2.0 / 24.0 + 1.0 / 86400.0 + 2.0 + 2.0), 1.0e-9);
        BOOST_CHECK_THROW(ts->tradingTime(Date(10, January, 2024, 13, 0, 0)), Error);
    }
    // from the start of the day, everything counts
    BOOST_CHECK_SMALL(fromDay.tradingTime(day + 1) * 365.0 - (1.0 + 1.0 + 2.0 + 4.0), 1.0e-9);
    // and the times agree with the day-based curve
    BOOST_CHECK_SMALL(fromDay.tradingTime(now, day + 1) - fixed.tradingTime(day + 1), 1.0e-12);

    // the floating curve follows the time of day too
    Settings::instance().evaluationDate() = Date(10, January, 2024, 9, 0, 0);
    BOOST_CHECK_EQUAL(floating.eventIntervals().size(), Size(3));
    BOOST_CHECK_SMALL(floating.tradingTime(day + 1) * 365.0 - (15.0 / 24.0 + 1.0 + 2.0 + 4.0),
                      1.0e-9);
}
#endif

BOOST_AUTO_TEST_CASE(testTradingTimeFloatingReference) {
    BOOST_TEST_MESSAGE("Testing trading time with a reference date following the evaluation date...");

    const Date today(5, January, 2024);
    Settings::instance().evaluationDate() = today;
    TradingTimeTermStructure floating(TARGET(), 0.2);
    floating.tradingTime(today + 30);  // build the state for the first reference date
    for (Integer days : {1, 7, 400}) {
        // the reference date is the evaluation date, even on a weekend:
        // trading time runs from now
        Settings::instance().evaluationDate() = today + days;
        const Date reference = floating.referenceDate();
        BOOST_CHECK_EQUAL(reference, today + days);
        TradingTimeTermStructure fixed(reference, TARGET(), 0.2);
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
    TradingTimeTermStructure ts(saturday, WeekendsOnly(), 0.25);
    BOOST_CHECK_EQUAL(ts.tradingTime(saturday, saturday), 0.0);
    // starting on a weekend, time runs at the holiday weight
    BOOST_CHECK_SMALL(ts.tradingTime(0.5 / 365.0) * 365.0 - 0.125, 1.0e-12);
    BOOST_CHECK_SMALL(ts.tradingTime(saturday + 3, saturday + 1) + ts.tradingTime(saturday + 1, saturday + 3),
                      1.0e-15);
    BOOST_CHECK_THROW(ts.tradingTime(saturday - 1), Error);
    BOOST_CHECK_THROW(ts.tradingTime(-1.0 / 365.0), Error);

    BOOST_CHECK_THROW(TradingTimeTermStructure(saturday, WeekendsOnly(), -0.1), Error);
    BOOST_CHECK_THROW(TradingTimeTermStructure(saturday, WeekendsOnly(), 1.1), Error);
    // default construction: reference date at the evaluation date, even on a weekend
    TradingTimeTermStructure byDefault;
    BOOST_CHECK_EQUAL(byDefault.referenceDate(), saturday);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
