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

#include <ql/experimental/fxslv/conventions/fxsettlementconvention.hpp>
#include <ql/experimental/fxslv/conventions/fxtenorexpiry.hpp>
#include <ql/experimental/fxslv/fxlocalvolsurface.hpp>
#include <ql/experimental/fxslv/fxsettlementforeigncurve.hpp>
#include <ql/experimental/fxslv/fxvariancesurface.hpp>
#include <ql/experimental/fxslv/fxpnlexplain.hpp>
#include <ql/experimental/fxslv/smilesections/fxsmilesectionbystrike.hpp>
#include <ql/termstructures/volatility/equityfx/localvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/noexceptlocalvolsurface.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimetermstructure.hpp>
#include <ql/experimental/fxslv/fxforwardcurve.hpp>
#include <ql/experimental/fxslv/fxcalendartimevolsurface.hpp>
#include <ql/experimental/fxslv/fxpairmarket.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimedaycounter.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimeyieldtermstructure.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/experimental/fxslv/fxvanillagreeks.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/exercise.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/quotes/deltavolquote.hpp>
#include <ql/experimental/fxslv/fxslvpricingcontext.hpp>
#include <ql/experimental/fxslv/stochvolcalibrator.hpp>
#include <ql/experimental/fxslv/slvleveragecalibrator.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancesurface.hpp>
#include <map>
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
    const FxSmileModel testSmileModel = FxSmileModel::Sabr;  // the same, for surfaces

    // The quotes of a test market: 3M and 6M pillars, 25 and 10 delta
    struct FxMarketQuotes {
        struct Vol { Real atm, rr25, bf25, rr10, bf10; };
        std::string foreign, domestic;
        Real spot, foreignRate, domesticRate;
        DeltaVolQuote::DeltaType deltaType;
        DeltaVolQuote::AtmType atmType;
        std::vector<Vol> vols;

        // EUR/USD: forward deltas, no premium adjustment
        static FxMarketQuotes eurusd() {
            return {"EUR", "USD", 1.0850, 0.04, 0.0525, DeltaVolQuote::Fwd, DeltaVolQuote::AtmFwd,
                    {{0.0850, -0.014, 0.0045, -0.0280, 0.0095},     // 3M
                     {0.0900, -0.016, 0.0050, -0.0310, 0.0105}}};   // 6M
        }
        // USD/JPY: premium in USD, the foreign currency, so premium-adjusted
        // spot deltas and a delta-neutral ATM, as quoted up to 1Y
        static FxMarketQuotes usdjpy() {
            return {"USD", "JPY", 150.0, 0.0525, 0.001, DeltaVolQuote::PaSpot,
                    DeltaVolQuote::AtmDeltaNeutral,
                    {{0.1000, -0.015, 0.0035, -0.0280, 0.0120},     // 3M
                     {0.1050, -0.018, 0.0040, -0.0330, 0.0135}}};   // 6M
        }
    };

    struct FxMarket {
        Date today;
        Date expiry3M;
        Date expiry6M;

        DayCounter dc;
        Handle<YieldTermStructure> foreignTs; // foreign
        Handle<YieldTermStructure> domesticTs; // domestic
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

        FxSettleConvention settlement{WeekendsOnly(), 2};
        Real holidayWeight;
        std::string foreignCcy, domesticCcy;

        ext::shared_ptr<FxVarianceSurfaceNCP> surface;
        // the local-vol process in calendar time, from makeFxLocalVolProcess
        ext::shared_ptr<GeneralizedBlackScholesProcess> process;
        // the local-vol process on the trading-time clock, for pricing
        ext::shared_ptr<GeneralizedBlackScholesProcess> tradingTimeProcess;

        explicit FxMarket(Real holidayWeight = 0.2,
                          std::vector<ext::shared_ptr<TradingTimeEvent>> events = {})
        : FxMarket(FxMarketQuotes::eurusd(), holidayWeight, std::move(events)) {}

        explicit FxMarket(const FxMarketQuotes& quotes,
                          Real holidayWeight = 0.2,
                          std::vector<ext::shared_ptr<TradingTimeEvent>> events = {})
        : holidayWeight(holidayWeight), foreignCcy(quotes.foreign), domesticCcy(quotes.domestic) {
            today    = Date(2, January, 2024);
            expiry3M = Date(2, April,   2024);
            expiry6M = Date(2, July,    2024);
            Settings::instance().evaluationDate() = today;
            dc = Actual365Fixed();

            foreignTs = Handle<YieldTermStructure>(
                ext::make_shared<FlatForward>(0, NullCalendar(), quotes.foreignRate, dc));
            domesticTs = Handle<YieldTermStructure>(
                ext::make_shared<FlatForward>(0, NullCalendar(), quotes.domesticRate, dc));
            timeTs = Handle<TradingTimeTermStructure>(ext::make_shared<TradingTimeTermStructure>(
                today, WeekendsOnly(), holidayWeight, std::move(events)));

            spotSQ = ext::make_shared<SimpleQuote>(quotes.spot);
            spot   = Handle<Quote>(spotSQ);

            deltaType = quotes.deltaType;
            atmType   = quotes.atmType;
            flyType   = FxRrBfQuotes::SmileStrangle;
            deltas    = { 0.25, 0.10 };

            // Two pillars: 3M and 6M
            pillars = { expiry3M, expiry6M };

            const std::vector<FxMarketQuotes::Vol>& mkt = quotes.vols;
            QL_REQUIRE(mkt.size() == pillars.size(), "one set of vols per pillar needed");

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

            surface = ext::make_shared<FxVarianceSurfaceNCP>(testSmileModel,
                today, spot, pillars, atms, rrs, bfs, deltas,
                foreignTs, domesticTs, timeTs,
                deltaType, atmType, flyType,
                FxSettleConvention(WeekendsOnly(), 2), FxExpiryCut(10),
                WeekendsOnly(), Following, true);
            surface->enableExtrapolation();

            // in calendar time, on the surface seen in calendar time
            process = makeFxLocalVolProcess(
                spot, foreignTs, domesticTs, settlement,
                Handle<BlackVolTermStructure>(ext::make_shared<FxCalendarTimeVolSurface>(surface)));
            tradingTimeProcess = pairMarket().localVolProcess();
        }

        // The pair's conventions
        FxPairConventions pairConventions() const {
            return FxPairConventions(foreignCcy, domesticCcy, settlement, FxExpiryCut(10),
                                     deltaType, atmType, flyType);
        }

        // The pair's market, from the quotes' current values (without the
        // trading-time events the surface may have been given)
        FxPairMarket pairMarket() const {
            // the 3M and 6M tenors, whose expiries are the pillars
            FxVolQuotes quotes;
            quotes.tenors = {3 * Months, 6 * Months};
            quotes.deltas = deltas;
            for (Size i = 0; i < pillars.size(); ++i) {
                quotes.atms.push_back(atms[i]->value());
                quotes.riskReversals.push_back({rrs[i][0]->value(), rrs[i][1]->value()});
                quotes.butterflies.push_back({bfs[i][0]->value(), bfs[i][1]->value()});
            }
            return FxPairMarket(today, pairConventions(), spot->value(), foreignTs, domesticTs,
                                quotes, {},
                                FxVolSurfaceConfig(FxSmileModel::Sabr, FxVolInterpolation::Ncp,
                                                   holidayWeight));
        }

        // Build a 3M EUR call at ATM forward
        ext::shared_ptr<VanillaOption> makeAtmCall() const {
            Time T   = dc.yearFraction(today, expiry3M);
            Real fwd = spot->value() * foreignTs->discount(T) / domesticTs->discount(T);
            auto payoff   = ext::make_shared<PlainVanillaPayoff>(Option::Call, fwd);
            auto exercise = ext::make_shared<EuropeanExercise>(expiry3M);
            return ext::make_shared<VanillaOption>(payoff, exercise);
        }

        // Build a 3M EUR put at ATM forward
        ext::shared_ptr<VanillaOption> makeAtmPut() const {
            Time T   = dc.yearFraction(today, expiry3M);
            Real fwd = spot->value() * foreignTs->discount(T) / domesticTs->discount(T);
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

    FxLocalVolSurface lv(mkt.tradingTimeProcess->blackVolatility(),
                         mkt.tradingTimeProcess->riskFreeRate(),
                         mkt.tradingTimeProcess->dividendYield(),
                         mkt.tradingTimeProcess->stateVariable());
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

    FxVanillaBumpRisk riskCalc(option, mkt.pairMarket());

    std::vector<Time> times   = { 1.0/12, 3.0/12, 6.0/12 };
    std::vector<Real> strikes = { 0.95, 1.00, 1.05, 1.10 };

    auto fixedLV = riskCalc.buildFixedLocalVolSurface(times, strikes);
    fixedLV->enableExtrapolation();

    FxLocalVolSurface lvDupire(mkt.tradingTimeProcess->blackVolatility(),
                               mkt.tradingTimeProcess->riskFreeRate(),
                               mkt.tradingTimeProcess->dividendYield(),
                               mkt.tradingTimeProcess->stateVariable());
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
//    navva = 0.01 S vanna (by construction)
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testGreekSigns) {
    BOOST_TEST_MESSAGE("Testing FX local vol Greeks: sign conventions...");

    FxMarket mkt;

    // ── Call Greeks ──────────────────────────────────────────────────────────
    {
        auto option = mkt.makeAtmCall();
        FxVanillaBumpRisk riskCalc(option, mkt.pairMarket(),
                                   /*notional=*/1.0e6,
                                   /*spotBump=*/0.001,
                                   /*volBump=*/0.01,
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
        // at the money forward d1 d2 = -sigma^2 T / 4: volga is about 0, slightly negative
        BOOST_CHECK_MESSAGE(std::fabs(g.volga) < 1.0e-3 * g.vega,
                            "call volga at the money should be about 0: " << g.volga);

        // navva is vanna in other units: the change of vega (USD) per 1% spot
        // move against that of delta (EUR) per 1% vol move
        BOOST_CHECK_CLOSE(g.navva, 0.01 * mkt.spot->value() * g.vanna, 1.0e-10);
    }

    // ── Put Greeks ───────────────────────────────────────────────────────────
    {
        auto option = mkt.makeAtmPut();
        FxVanillaBumpRisk riskCalc(option, mkt.pairMarket(),
                                   /*notional=*/1.0e6,
                                   /*spotBump=*/0.001,
                                   /*volBump=*/0.01,
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
        // at the money forward d1 d2 = -sigma^2 T / 4: volga is about 0, slightly negative
        BOOST_CHECK_MESSAGE(std::fabs(g.volga) < 1.0e-3 * g.vega,
                            "put volga at the money should be about 0: " << g.volga);
    }

    // ── Out of the money, d1 and d2 have the same sign and volga is positive ─
    for (Real moneyness : {0.95, 1.05}) {
        const Option::Type type = moneyness > 1.0 ? Option::Call : Option::Put;
        const Real strike = moneyness * mkt.surface->fwd(mkt.expiry3M);
        auto option = ext::make_shared<VanillaOption>(
            ext::make_shared<PlainVanillaPayoff>(type, strike),
            ext::make_shared<EuropeanExercise>(mkt.expiry3M));
        FxVanillaBumpRisk riskCalc(option, mkt.pairMarket(), 1.0e6, 0.001, 0.01, 0.001, 0.001,
                                   50, 50);
        for (bool localVol : {false, true}) {
            const FxVanillaGreeks g = riskCalc.calculate(localVol);
            BOOST_CHECK_MESSAGE(g.volga > 0.0, (localVol ? "local-vol " : "Black ")
                                                   << (type == Option::Call ? "call" : "put")
                                                   << " volga at " << moneyness
                                                   << " F must be positive: " << g.volga);
        }
    }
}

BOOST_AUTO_TEST_CASE(testThetaIsTheRollOfTheMarket) {
    BOOST_TEST_MESSAGE("Testing that FX vanilla theta is the P&L of the roll of the market...");

    FxMarket mkt;
    auto option = mkt.makeAtmCall();
    const Real notional = 1.0e6;
    FxVanillaBumpRisk riskCalc(option, mkt.pairMarket(), notional);
    // the Black theta is the P&L the explain reports from today's market to
    // the rolled one, i.e. with no market move
    const Date next = mkt.settlement.calendar().advance(mkt.today, 1, Days);
    const Real theta = riskCalc.calculate(/*localVol=*/false).theta;
    const auto strike = ext::dynamic_pointer_cast<StrikedTypePayoff>(option->payoff())->strike();
    const FxPairMarket today = riskCalc.market();
    const FxPnlExplainResult r = fxVanillaPnlExplain(Option::Call, strike, mkt.expiry3M, notional,
                                                     today, today.rolled(next));
    BOOST_CHECK_LT(theta, 0.0);
    BOOST_CHECK_CLOSE(theta, r.total, 1.0e-4);
    // ...which the explain splits into the theta at today's vol and the vol roll
    BOOST_CHECK_CLOSE(theta, r.theta + r.volRollAtm + r.volRollRiskReversal + r.volRollButterfly +
                                 r.volRollOther, 1.0);
    // the evaluation date is today's again
    BOOST_CHECK_EQUAL(Settings::instance().evaluationDate(), mkt.today);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Test 4: Put-call parity
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testPutCallParity) {
    BOOST_TEST_MESSAGE("Testing FX local vol: put-call parity...");

    FxMarket mkt;
    const Real notional = 1.0e6;

    auto callOption = mkt.makeAtmCall();
    auto putOption  = mkt.makeAtmPut();

    FxVanillaBumpRisk callCalc(callOption, mkt.pairMarket(),
                                notional, 0.001, 0.001, 0.001, 0.001, 50, 50);
    FxVanillaBumpRisk putCalc(putOption, mkt.pairMarket(),
                               notional, 0.001, 0.001, 0.001, 0.001, 50, 50);

    FxVanillaGreeks callG = callCalc.calculate(true);
    FxVanillaGreeks putG  = putCalc.calculate(true);

    // For ATM forward: call NPV - put NPV ≈ 0
    const Real priceTol = 10.0; // $10 on a 1M notional position
    BOOST_CHECK_MESSAGE(
        std::fabs(callG.npv - putG.npv) < priceTol,
        "put-call parity violation: call-put=" << (callG.npv - putG.npv)
        << " (tolerance=" << priceTol << ")");

    // Gamma and vega parity: C - P is linear in the forward, so the call's
    // and the put's are equal in the model.  The finite-difference local vol
    // pricing meets parity only to about 9 USD here, converging to it no
    // closer as the grid is refined, nor in calendar time: the local vol in
    // the wings of its spot grid is large (0.3 to 0.9 against 0.09 at the
    // money), and capping it at 0.15 restores parity to 0.02 USD.  Being
    // vol and spot dependent, that error leaves call and put gamma 0.06% and
    // vega 0.12% apart on this grid (0.01% and 0.08% on 400 points).
    BOOST_CHECK_MESSAGE(
        std::fabs(callG.spotGamma - putG.spotGamma) < 1.0e-3 * callG.spotGamma,
        "gamma parity violation: call gamma=" << callG.spotGamma
        << " put gamma=" << putG.spotGamma);
    BOOST_CHECK_MESSAGE(
        std::fabs(callG.vega - putG.vega) < 2.0e-3 * callG.vega,
        "vega parity violation: call vega=" << callG.vega
        << " put vega=" << putG.vega);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Test 5: Delta and gamma consistency
//
//  Taylor approximation: ΔV ≈ spotDelta × ds + 0.5 × spotGamma / (0.01 S) × ds²
//  spotDelta being dV/dS and spotGamma its change per 1% spot move
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testDeltaGammaConsistency) {
    BOOST_TEST_MESSAGE("Testing FX local vol: delta-gamma consistency...");

    FxMarket mkt;
    auto option = mkt.makeAtmCall();

    const Real notional = 1.0;  // unit notional for this test

    FxVanillaBumpRisk riskCalc(option, mkt.pairMarket(),
                                notional, 0.001, 0.001, 0.001, 0.001, 50, 50);
    FxVanillaGreeks g = riskCalc.calculate(true);

    Real S0  = mkt.spotSQ->value();
    Real ds  = 0.0005; // 5 pips absolute
    Real V0  = g.npv;

    // the market at the shifted spot, its smile re-anchored there
    FxVanillaBumpRisk shiftedCalc(option, mkt.pairMarket().withSpot(S0 + ds), notional, 0.001,
                                  0.001, 0.001, 0.001, 50, 50);
    Real Vshifted = shiftedCalc.calculate(true).npv;

    Real actualPnL = Vshifted - V0;

    // spotDelta is dV/dS, an amount of EUR; spotGamma its change per 1% spot
    Real predictedPnL = g.spotDelta * ds + 0.5 * g.spotGamma / (0.01 * S0) * ds * ds;

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

    FxVanillaBumpRisk riskCalc(option, mkt.pairMarket(),
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
//  Test 7: Sticky-delta and local-vol (frozen local vol) deltas, with skew
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testStickyDeltaVsLocalVolDelta) {
    BOOST_TEST_MESSAGE("Testing FX local vol: sticky-delta and local-vol deltas...");

    FxMarket mkt;
    auto option = mkt.makeAtmCall();
    const Real notional = 1.0e6;

    FxVanillaBumpRisk riskCalc(option, mkt.pairMarket(),
                                notional,
                                /*spotBump=*/0.001,
                                /*volBump=*/0.001,
                                /*rrBump=*/0.001,
                                /*bfBump=*/0.001,
                                /*tGrid=*/50, /*xGrid=*/50,
                                /*lvTimePts=*/15, /*lvStrikePts=*/30);

    FxVanillaGreeks sdG = riskCalc.calculate(true, FxVanillaBumpRisk::StickyType::Delta);
    FxVanillaGreeks lvG = riskCalc.calculate(true, FxVanillaBumpRisk::StickyType::LocalVol);

    BOOST_CHECK_MESSAGE(sdG.spotDelta > 0.0,
        "sticky-delta call spot delta must be >0: " << sdG.spotDelta);
    BOOST_CHECK_MESSAGE(lvG.spotDelta > 0.0,
        "local-vol call spot delta must be >0: " << lvG.spotDelta);

    BOOST_CHECK_MESSAGE(sdG.npv > 0.0, "sticky-delta NPV must be >0: " << sdG.npv);
    BOOST_CHECK_MESSAGE(lvG.npv > 0.0, "local-vol NPV must be >0: " << lvG.npv);

    const Real npvTol = 10.0;
    BOOST_CHECK_MESSAGE(
        std::fabs(sdG.npv - lvG.npv) < npvTol,
        "sticky-delta and frozen local-vol NPVs should agree: "
        << "sd=" << sdG.npv << " ss=" << lvG.npv
        << " diff=" << std::fabs(sdG.npv - lvG.npv));

    // Deltas must differ due to skew (at least 0.01% relative difference).
    const Real minRelDiff = 1.0e-4;
    const Real relDiff = std::fabs(sdG.spotDelta - lvG.spotDelta)
                         / std::fabs(sdG.spotDelta);
    BOOST_CHECK_MESSAGE(
        relDiff > minRelDiff,
        "sticky-delta and local-vol deltas should differ with non-zero skew: "
        << "sd=" << sdG.spotDelta << " ss=" << lvG.spotDelta
        << " relDiff=" << relDiff * 100.0 << "%"
        << " (min expected=" << minRelDiff * 100.0 << "%)");

    // A call's delta is its Black delta plus vega times the move of its
    // implied vol with spot at a fixed strike, which is about -skew with sticky
    // delta and +skew under local vol (Hagan et al., 2002).  With EUR put skew
    // (implied vol falling in the strike), the local-vol delta is the lower,
    // by about twice vega times the skew, in the units of the deltas
    BOOST_CHECK_MESSAGE(
        lvG.spotDelta < sdG.spotDelta,
        "with EUR put skew, the local-vol delta should be < the sticky-delta one: "
        << "sd=" << sdG.spotDelta << " lv=" << lvG.spotDelta);
    const Real strike = ext::dynamic_pointer_cast<StrikedTypePayoff>(option->payoff())->strike();
    const Real h = 0.001;
    const Real skew = (mkt.surface->marketVol(mkt.expiry3M, strike + h) -
                       mkt.surface->marketVol(mkt.expiry3M, strike - h)) / (2.0 * h);
    const Real expectedGap = 2.0 * (sdG.vega / 0.01) * std::fabs(skew);
    // the rule is first order: here the gap is EUR 120,607 against its 96,799
    BOOST_CHECK_CLOSE(sdG.spotDelta - lvG.spotDelta, expectedGap, 30.0);

    // sticky strike, implied vols at fixed strikes not moving, is the Black
    // delta at the option's vol, whether priced with local vol or not, and
    // lies between: sticky delta is above it by vega times the skew (EUR
    // 49,694 against 48,400 here), local vol below it by about as much, its
    // rule being the cruder (70,913 against 48,400)
    const FxVanillaGreeks ssG = riskCalc.calculate(true, FxVanillaBumpRisk::StickyType::Strike);
    const FxVanillaGreeks ssBlack =
        riskCalc.calculate(false, FxVanillaBumpRisk::StickyType::Strike);
    BOOST_CHECK_EQUAL(ssG.spotDelta, ssBlack.spotDelta);
    BOOST_CHECK_LT(lvG.spotDelta, ssG.spotDelta);
    BOOST_CHECK_CLOSE(sdG.spotDelta - ssG.spotDelta, expectedGap / 2.0, 10.0);

    // navva = 0.01 S vanna in both modes
    BOOST_CHECK_CLOSE(sdG.navva, 0.01 * mkt.spot->value() * sdG.vanna, 1.0e-10);
    BOOST_CHECK_CLOSE(lvG.navva, 0.01 * mkt.spot->value() * lvG.vanna, 1.0e-10);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Premium-adjusted greeks
// ─────────────────────────────────────────────────────────────────────────────

BOOST_AUTO_TEST_CASE(testPremiumAdjustedBlackGreeks) {
    BOOST_TEST_MESSAGE("Testing FX premium-adjusted greeks against Black...");

    // USD/JPY, the premium in USD: with sticky strike, the greeks are
    // Black's at the option's vol.  Per unit of notional, a call's (put's)
    // spot delta is B_f N(d1) (-B_f N(-d1)) and its forward delta N(d1)
    // (-N(-d1)); its premium-adjusted spot delta is B_d (K/S) N(d2)
    // (-B_d (K/S) N(-d2)) and forward delta (K/F) N(d2) (-(K/F) N(-d2)),
    // Reiswich and Wystup (2010), whose change per 1% spot move is
    // 0.01 B_d (K/S) [n(d2)/(sigma sqrt(T)) - w N(w d2)]
    FxMarket mkt(FxMarketQuotes::usdjpy());
    const Real notional = 1.0e6;   // USD
    const Real S = mkt.spot->value();
    const auto& process = mkt.tradingTimeProcess;
    const Real Bd = process->riskFreeRate()->discount(mkt.expiry3M);
    const Real Bf = process->dividendYield()->discount(mkt.expiry3M);
    const Real F = S * Bf / Bd;
    const CumulativeNormalDistribution N;
    const NormalDistribution n;

    struct Case { Option::Type type; Real strike; };
    // ATM forward, out of the money and deep in the money
    for (const Case& c : {Case{Option::Call, F}, Case{Option::Put, F},
                          Case{Option::Call, 1.04 * F}, Case{Option::Put, 0.96 * F},
                          Case{Option::Call, 0.85 * F}}) {
        auto option = ext::make_shared<VanillaOption>(
            ext::make_shared<PlainVanillaPayoff>(c.type, c.strike),
            ext::make_shared<EuropeanExercise>(mkt.expiry3M));
        FxVanillaBumpRisk riskCalc(option, mkt.pairMarket(),
                                   notional);
        const FxVanillaGreeks g =
            riskCalc.calculate(false, FxVanillaBumpRisk::StickyType::Strike);
        const FxPremiumAdjustedGreeks& pa = g.premiumAdjusted;

        const Real K = c.strike;
        const Real w = (c.type == Option::Call) ? 1.0 : -1.0;
        const Real stdDev = std::sqrt(process->blackVolatility()->blackVariance(mkt.expiry3M, K));
        const Real d1 = std::log(F / K) / stdDev + 0.5 * stdDev;
        const Real d2 = d1 - stdDev;
        const Real delta = notional * w * Bd * (K / S) * N(w * d2);
        const Real gamma =
            0.01 * notional * Bd * (K / S) * (n(d2) / stdDev - w * N(w * d2));

        BOOST_TEST_INFO("type " << c.type << ", strike " << K);
        // plain deltas: amounts of USD, spot and forward
        BOOST_CHECK_CLOSE(g.spotDelta, notional * w * Bf * N(w * d1), 0.01);
        BOOST_CHECK_CLOSE(g.fwdDelta, notional * w * N(w * d1), 0.01);
        // premium-adjusted
        BOOST_CHECK_CLOSE(pa.spotDelta, delta, 0.01);
        BOOST_CHECK_CLOSE(pa.fwdDelta, notional * w * (K / F) * N(w * d2), 0.01);
        BOOST_CHECK_CLOSE(pa.spotGamma, gamma, 0.1);
        BOOST_CHECK_CLOSE(pa.fwdGamma, gamma / Bf, 0.1);
        // the adjustment: the value in USD off the delta, vega / S off vanna
        BOOST_CHECK_CLOSE(pa.spotDelta, g.spotDelta - g.npv / S, 1.0e-10);
        BOOST_CHECK_CLOSE(pa.vanna, g.vanna - g.vega / S, 1.0e-10);
        // the premium makes the delta smaller in the size of a call, larger
        // in that of a put
        BOOST_CHECK_LT(pa.spotDelta, g.spotDelta);
        // deep in the money, a call's premium-adjusted gamma is negative:
        // as spot rises, the value in USD, V/S, grows faster than delta
        if (c.type == Option::Call && K < 0.9 * F) {
            BOOST_CHECK_GT(g.spotGamma, 0.0);
            BOOST_CHECK_LT(pa.spotGamma, 0.0);
        }
    }

}

BOOST_AUTO_TEST_CASE(testPremiumAdjustedParityWithLocalVol) {
    BOOST_TEST_MESSAGE("Testing FX premium-adjusted greeks: parity under local vol "
                       "on a premium-adjusted market...");

    // USD/JPY, its smile quoted in premium-adjusted deltas
    FxMarket mkt(FxMarketQuotes::usdjpy());
    const Real notional = 1.0e6;
    const auto& process = mkt.tradingTimeProcess;
    const Real Bd = process->riskFreeRate()->discount(mkt.expiry3M);

    auto callOption = mkt.makeAtmCall();
    auto putOption = mkt.makeAtmPut();
    const Real K = ext::dynamic_pointer_cast<StrikedTypePayoff>(callOption->payoff())->strike();

    FxVanillaBumpRisk callCalc(callOption, mkt.pairMarket(), notional,
                               0.001, 0.01, 0.001, 0.001, 50, 50);
    FxVanillaBumpRisk putCalc(putOption, mkt.pairMarket(), notional,
                              0.001, 0.01, 0.001, 0.001, 50, 50);
    const FxVanillaGreeks call = callCalc.calculate(true);
    const FxVanillaGreeks put = putCalc.calculate(true);

    // the local-vol prices are those of the smile
    const FxVanillaGreeks callBlack = callCalc.calculate(false);
    BOOST_CHECK_CLOSE(call.npv, callBlack.npv, 1.0);

    // plain greeks: the call's delta exceeds the put's by B_f per unit of
    // notional, and their gammas are equal.  On the
    // finite-difference grid the delta gap is off by 0.23% at 50 points,
    // converging with the grid (0.067%, 0.026%, 0.016% at 100, 200, 400;
    // 1e-14 with Black), the same on EUR/USD; the gammas differ by 0.08%
    // (see testPutCallParity)
    const Real S = mkt.spot->value();
    const Real Bf = process->dividendYield()->discount(mkt.expiry3M);
    BOOST_CHECK_CLOSE(call.spotDelta - put.spotDelta, notional * Bf, 0.3);
    BOOST_CHECK_CLOSE(call.fwdDelta - put.fwdDelta, notional, 0.3);
    BOOST_CHECK_SMALL(call.spotGamma - put.spotGamma, 1.0e-3 * call.spotGamma);

    // premium-adjusted: the call's delta exceeds the put's by K B_d / S (in
    // forwards, K / F), and its gamma, per 1% spot move, falls short by
    // 0.01 K B_d / S; the same grid errors (here 0.23% on the deltas, 0.5%
    // on the gammas)
    const FxPremiumAdjustedGreeks& cpa = call.premiumAdjusted;
    const FxPremiumAdjustedGreeks& ppa = put.premiumAdjusted;
    const Real F = S * Bf / Bd;
    BOOST_CHECK_CLOSE(cpa.spotDelta - ppa.spotDelta, notional * K * Bd / S, 0.3);
    BOOST_CHECK_CLOSE(cpa.fwdDelta - ppa.fwdDelta, notional * K / F, 0.3);
    BOOST_CHECK_CLOSE(cpa.spotGamma - ppa.spotGamma, -0.01 * notional * K * Bd / S, 1.0);
    // vanna: C - P does not depend on vol.  The grid's parity error depends
    // on vol, and leaves call and put vanna 0.8% apart here (0.4% to 0.6% on
    // EUR/USD), on any grid
    BOOST_CHECK_SMALL(cpa.vanna - ppa.vanna, 1.5e-2 * std::fabs(cpa.vanna));
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

    const Handle<YieldTermStructure> domesticTs(
        ext::make_shared<FlatForward>(0, NullCalendar(), 0.0525, dc));  // domestic
    const Handle<YieldTermStructure> foreignTs(
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

    const FxSettleConvention settlement(calendar, 2);
    const std::vector<Date> pillars = fxExpiryDates(today, tenors, settlement);
    std::vector<Handle<Quote>>              atms;
    std::vector<std::vector<Handle<Quote>>> rrs, bfs;
    std::vector<ext::shared_ptr<SimpleQuote>> atmSQs;
    std::vector<std::vector<ext::shared_ptr<SimpleQuote>>> rrSQs, bfSQs;

    for (Size i = 0; i < tenors.size(); ++i) {
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
    // weekends carry a fifth of a business day's variance: with none, local vol
    // vanishes over whole days, which the SLV leverage calibration cannot handle
    const Real weekendWeight = 0.2;
    auto timeTs = Handle<TradingTimeTermStructure>(
        ext::make_shared<TradingTimeTermStructure>(today, calendar, weekendWeight));

    auto volSurface = ext::make_shared<FxVarianceSurfaceNCP>(testSmileModel,
        today, spot, pillars, atms, rrs, bfs, deltas,
        foreignTs, domesticTs, timeTs,
        DeltaVolQuote::Fwd, DeltaVolQuote::AtmFwd, FxRrBfQuotes::SmileStrangle,
        settlement, FxExpiryCut(10),
        calendar, Following, true);
    volSurface->enableExtrapolation();
    // SLV is calibrated in calendar time: the surface seen in calendar time
    Handle<BlackVolTermStructure> volHandle(ext::make_shared<FxCalendarTimeVolSurface>(volSurface));

    // ── Local-vol process: settlement forward and Dupire local vol ──────────
    const auto process = makeFxLocalVolProcess(spot, foreignTs, domesticTs, settlement, volHandle);
    const Handle<YieldTermStructure> eurFwdTs = process->dividendYield();
    const Handle<LocalVolTermStructure> localVolHandle = process->localVolatility();

    // ── SLV calibration ──────────────────────────────────────────────────────
    // Use 5 tenors × 3 strikes for Heston calibration, around the settlement
    // forwards of their FX expiries.
    const std::vector<Period> calibTenors = {
        1*Months, 3*Months, 6*Months, 1*Years, 2*Years
    };
    std::vector<StochVolCalibrator::Pillar> hestonPillars;
    for (const auto& tenor : calibTenors) {
        const Real fwd = volSurface->fwd(fxExpiryDate(today, tenor, settlement));
        for (Real m : { 0.95, 1.00, 1.05 })
            hestonPillars.push_back({ tenor, m * fwd });
    }

    HestonParams hp;
    hp.v0    = 0.078 * 0.078;
    hp.kappa = 1.50;
    hp.theta = 0.100 * 0.100;
    hp.sigma = 0.15;
    hp.rho   = -0.25;

    FxSLVPricingContext ctx(spot, domesticTs, eurFwdTs, volHandle, localVolHandle, settlement);

    // under the Feller condition: the SLV calibration fails with a fit
    // violating it strongly, as the unconstrained one does
    HestonCalibrator hestonCal(hp, EndCriteria(1000, 500, 1.0e-8, 1.0e-8, 1.0e-8),
                               BlackCalibrationHelper::ImpliedVolError, {}, true);

    // Small FDM grid for test speed.
    auto fdmP = HestonSLVLeverageCalibrator::defaultParams();
    fdmP.xGrid            = 51;
    fdmP.vGrid            = 21;
    fdmP.tMaxStepsPerYear = 52;
    fdmP.tMinStepsPerYear = 12;
    fdmP.tStepNumberDecay = 2.0;

    HestonSLVLeverageCalibrator levCal(fdmP);

    const Date endDate = pillars.back();  // the 2Y expiry, where local vol ends
    ctx.calibrate(hestonCal, levCal, hestonPillars, endDate);

    // Heston fit quality check.
    BOOST_CHECK_MESSAGE(ctx.hestonRmse() < 0.015,
        "Heston RMSE too large: " << ctx.hestonRmse() * 100.0 << " vol pts");

    // ── Price a 4M ATM-forward call ──────────────────────────────────────────
    // 4M is strictly between the 3M (surface pillar) and 6M (surface pillar).
    const Date expiry4M = fxExpiryDate(today, 4*Months, settlement);
    const Real fwd4M = volSurface->fwd(expiry4M);

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
    const Real strike = m.spot->value() * m.foreignTs->discount(d) / m.domesticTs->discount(d);
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
            m.foreignTs, m.domesticTs, m.deltaType, m.atmType, Actual365Fixed(),
            FxSettleConvention(WeekendsOnly(), 2), m.today);
        const Time T = m.dc.yearFraction(m.today, m.pillars[i]);
        BOOST_CHECK_SMALL(std::sqrt(m.surface->atmVar(m.pillars[i]) / T) - pillar.atmVol(), 1.0e-12);
    }

    // between them, linear in trading time
    const Date mid(15, May, 2024);
    const Real w = (tau(mid) - tau(m.expiry3M)) / (tau(m.expiry6M) - tau(m.expiry3M));
    BOOST_CHECK_SMALL(m.surface->atmVar(mid) - ((1.0 - w) * var3M + w * var6M), 1.0e-12);

    // before the first pillar, proportional to trading time, so a weekend's
    // two days at 0.2 accrue 0.4 of a business day's variance
    const Date saturday(3, February, 2024), monday(5, February, 2024), tuesday(6, February, 2024);
    BOOST_CHECK_SMALL(m.surface->atmVar(saturday) - var3M * tau(saturday) / tau(m.expiry3M),
                      1.0e-12);
    BOOST_CHECK_CLOSE((m.surface->atmVar(monday) - m.surface->atmVar(saturday)) /
                          (m.surface->atmVar(tuesday) - m.surface->atmVar(monday)),
                      0.4, 1.0e-8);

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
            m.foreignTs, m.domesticTs, m.deltaType, m.atmType, Actual365Fixed(),
            FxSettleConvention(WeekendsOnly(), 2), m.today);
        for (Real strike : {1.00, 1.05, 1.085, 1.12, 1.17})
            BOOST_CHECK_SMALL(m.surface->blackVariance(m.pillars[i], strike) - pillar.variance(strike),
                              1.0e-12);
    }
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceInTradingTime) {
    BOOST_TEST_MESSAGE("Testing that the variance surface accrues variance in trading time...");

    FxMarket m;
    // at a given forward moneyness, a weekend's two days at 0.2 accrue 0.4 of
    // a business day's variance, although the forward moves with the carry.
    // The nodes are separate smile fits to the scaled quotes: to 2e-5 of a
    // day's variance at the money, and about 0.5% of it in the wings
    const Date saturday(3, February, 2024), monday(5, February, 2024), tuesday(6, February, 2024);
    for (Real k : {0.93, 1.0, 1.07}) {
        auto w = [&](const Date& d) { return m.surface->blackVariance(d, k * m.surface->fwd(d)); };
        BOOST_CHECK_CLOSE((w(monday) - w(saturday)) / (w(tuesday) - w(monday)), 0.4,
                          k == 1.0 ? 1.0e-2 : 1.0);
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

    BOOST_CHECK_THROW(FxVarianceSurfaceNCP(testSmileModel,
                          m.today, m.spot, m.pillars, m.atms, m.rrs, m.bfs, m.deltas, m.foreignTs,
                          m.domesticTs, m.timeTs, m.deltaType, m.atmType, m.flyType,
                          FxSettleConvention(WeekendsOnly(), 2), FxExpiryCut(10), WeekendsOnly(), Following,
                          true, 0.0),
                      Error);
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceEventBetweenPillars) {
    BOOST_TEST_MESSAGE("Testing that the variance surface jumps at an event between pillars...");

    FxMarket m;
    const Date eventDate(10, April, 2024);  // a Wednesday between the pillars
    const Real timeOfDay = 13.0 / 24.0, size = 2.0;
    auto timeTs = Handle<TradingTimeTermStructure>(ext::make_shared<TradingTimeTermStructure>(
        m.today, WeekendsOnly(), 0.2,
        std::vector<ext::shared_ptr<TradingTimeEvent>>{ext::make_shared<JumpTradingTimeEvent>(
            eventDate, timeOfDay, makeQuoteHandle(size))}));
    auto surface = ext::make_shared<FxVarianceSurfaceNCP>(testSmileModel,
        m.today, m.spot, m.pillars, m.atms, m.rrs, m.bfs, m.deltas, m.foreignTs, m.domesticTs, timeTs,
        m.deltaType, m.atmType, m.flyType, FxSettleConvention(WeekendsOnly(), 2),
        FxExpiryCut(10));
    // in calendar time, where the event takes no time
    const FxCalendarTimeVolSurface calendar(surface);

    // the jump adds trading time at the rate variance accrues around it; all
    // the times below are on the event's day, with the same forward
    const Time te = (eventDate - m.today + timeOfDay) / 365.0, eps = 1.0e-7;
    const Real fwd = surface->fwd(eventDate);
    auto w = [&](Time t) { return calendar.blackVariance(t, fwd); };
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
        return FxVarianceSurfaceNCP(testSmileModel,
            m.today, m.spot, pillars, atms, rrs, bfs, m.deltas, m.foreignTs, m.domesticTs, m.timeTs,
            m.deltaType, m.atmType, m.flyType, FxSettleConvention(WeekendsOnly(), 2),
        FxExpiryCut(10));
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
        return FxVarianceSurfaceNCP(testSmileModel,
            m.today, m.spot, m.pillars, m.atms, m.rrs, m.bfs, m.deltas, m.foreignTs, m.domesticTs, timeTs,
            m.deltaType, m.atmType, m.flyType, FxSettleConvention(WeekendsOnly(), 2),
        FxExpiryCut(10));
    };
    const Date d(15, May, 2024);

    // a fixed trading-time curve from another date
    const auto shifted = surfaceWith(Handle<TradingTimeTermStructure>(
        ext::make_shared<TradingTimeTermStructure>(m.today - 3, WeekendsOnly(), 0.2)));
    BOOST_CHECK_THROW(shifted.blackVariance(d, m.spot->value()), Error);

    // a floating one agrees until the evaluation date moves
    const auto floating = surfaceWith(Handle<TradingTimeTermStructure>(
        ext::make_shared<TradingTimeTermStructure>(WeekendsOnly(), 0.2)));
    const Real before = floating.blackVariance(d, m.spot->value());
    BOOST_CHECK_SMALL(before - m.surface->blackVariance(d, m.spot->value()), 1.0e-15);
    // moving the evaluation date must not throw; using the surface then does
    BOOST_CHECK_NO_THROW(Settings::instance().evaluationDate() = m.today + 1);
    BOOST_CHECK_THROW(floating.blackVariance(d, m.spot->value()), Error);
}

BOOST_AUTO_TEST_CASE(testVarianceSurfaceSettlementForward) {
    BOOST_TEST_MESSAGE("Testing that the variance surface uses settlement forwards throughout...");

    FxMarket m;
    const FxSettleConvention settlement(WeekendsOnly(), 2);

    // at the pillars, the pillar sections' own forwards
    for (Size i = 0; i < m.pillars.size(); ++i) {
        TestSmileSection pillar(
            m.pillars[i], m.spot,
            ext::make_shared<FxRrBfQuotes>(m.atms[i], m.rrs[i], m.bfs[i], m.deltas, m.flyType),
            m.foreignTs, m.domesticTs, m.deltaType, m.atmType, Actual365Fixed(), settlement, m.today);
        BOOST_CHECK_CLOSE(m.surface->fwd(m.pillars[i]), pillar.forward(), 1.0e-12);
    }

    // a Friday expiry delivers on Tuesday, two days later than the time
    // to expiry alone would give
    const Date friday(12, April, 2024);
    const Date spotDate = settlement.valueDate(m.today), delivery = settlement.valueDate(friday);
    BOOST_CHECK_EQUAL(delivery, Date(16, April, 2024));
    const Real expected = m.spot->value() *
                          (m.foreignTs->discount(delivery) / m.foreignTs->discount(spotDate)) /
                          (m.domesticTs->discount(delivery) / m.domesticTs->discount(spotDate));
    BOOST_CHECK_CLOSE(m.surface->fwd(friday), expected, 1.0e-12);
    const Time tFriday = m.surface->timeFromReference(friday);
    BOOST_CHECK_CLOSE(m.surface->fwd(tFriday), expected, 1.0e-12);
    // every option expiring on Friday delivers on Tuesday: the forward
    // holds through the day and steps at midnight
    BOOST_CHECK_CLOSE(m.surface->fwd(tFriday + 0.5 / 365.0), expected, 1.0e-12);
    BOOST_CHECK_CLOSE(m.surface->fwd(tFriday + 1.0 / 365.0), m.surface->fwd(friday + 1), 1.0e-12);

    // curves referenced to the spot date give the same surface
    const Handle<YieldTermStructure> eurFromSpot(
        ext::make_shared<FlatForward>(2, WeekendsOnly(), 0.24, m.dc));
    const Handle<YieldTermStructure> usdFromSpot(
        ext::make_shared<FlatForward>(2, WeekendsOnly(), 0.2525, m.dc));
    BOOST_REQUIRE_EQUAL(eurFromSpot->referenceDate(), spotDate);
    FxVarianceSurfaceNCP fromSpot(testSmileModel,
        m.today, m.spot, m.pillars, m.atms, m.rrs, m.bfs, m.deltas, eurFromSpot, usdFromSpot,
        m.timeTs, m.deltaType, m.atmType, m.flyType, settlement, FxExpiryCut(10));
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
        return FxVarianceSurfaceNCP(testSmileModel,
            m.today, m.spot, m.pillars, m.atms, m.rrs, m.bfs, m.deltas, m.foreignTs, m.domesticTs,
            m.timeTs, m.deltaType, m.atmType, m.flyType, FxSettleConvention(WeekendsOnly(), 2),
            FxExpiryCut(10),
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

BOOST_AUTO_TEST_CASE(testMakeFxLocalVolProcess) {
    BOOST_TEST_MESSAGE("Testing the FX local-vol process factory...");

    FxMarket m;  // its process comes from makeFxLocalVolProcess
    // the pieces it wires
    BOOST_REQUIRE(ext::dynamic_pointer_cast<FxSettlementForeignCurve>(
        m.process->dividendYield().currentLink()));
    const auto localVol =
        ext::dynamic_pointer_cast<FxLocalVolSurface>(m.process->localVolatility().currentLink());
    BOOST_REQUIRE(localVol);
    BOOST_CHECK(m.process->riskFreeRate().currentLink() == m.domesticTs.currentLink());
    // the surface, seen in calendar time as the curves are
    const auto calendar = ext::dynamic_pointer_cast<FxCalendarTimeVolSurface>(
        m.process->blackVolatility().currentLink());
    BOOST_REQUIRE(calendar);
    BOOST_CHECK(calendar->tradingTimeSurface() == m.surface);

    // one forward for the process and the surface, and local vol lower over a weekend
    for (const Date& d : {Date(12, April, 2024), Date(13, April, 2024), m.expiry6M}) {
        const Time t = m.process->time(d) + 0.4 / 365.0;  // within the day
        BOOST_CHECK_CLOSE(m.spot->value() * m.process->dividendYield()->discount(t) /
                              m.process->riskFreeRate()->discount(t),
                          m.surface->fwd(d), 1.0e-12);
    }
    // where variance accrues at a fifth of the rate, local vol is about sqrt(0.2) of
    // that of the business days around it
    auto localVolOn = [&](const Date& d) {
        return m.process->localVolatility()->localVol(m.process->time(d) + 0.5 / 365.0, 1.085,
                                                      true);
    };
    const Real weekend = localVolOn(Date(3, February, 2024));
    const Real around = std::sqrt(localVolOn(Date(2, February, 2024)) * localVolOn(Date(5, February, 2024)));
    BOOST_CHECK_CLOSE(weekend / around, std::sqrt(0.2), 0.1);

    // without a fallback, undefined local vol fails; with one, it is counted
    const Date today(2, January, 2024);
    const DayCounter dc = Actual365Fixed();
    const Handle<YieldTermStructure> rates(ext::make_shared<FlatForward>(today, 0.0, dc));
    const Handle<BlackVolTermStructure> falling(ext::make_shared<BlackVarianceCurve>(
        today, std::vector<Date>{today + 90, today + 180}, std::vector<Volatility>{0.20, 0.10}, dc,
        false));
    const Handle<Quote> one = makeQuoteHandle(1.0);
    const FxSettleConvention settlement(WeekendsOnly(), 2);
    const auto strict = makeFxLocalVolProcess(one, rates, rates, settlement, falling);
    BOOST_CHECK_THROW(strict->localVolatility()->localVol(120.0 / 365.0, 1.0, true), Error);
    const auto lenient = makeFxLocalVolProcess(one, rates, rates, settlement, falling, 0.3);
    BOOST_CHECK_EQUAL(lenient->localVolatility()->localVol(120.0 / 365.0, 1.0, true), 0.3);
    BOOST_CHECK_EQUAL(ext::dynamic_pointer_cast<FxLocalVolSurface>(
                          lenient->localVolatility().currentLink())->fallbacks(),
                      Size(1));
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
        const Time t = m.process->time(d);  // in calendar time, as the process
        for (Real hours : {0.0, 9.0, 23.0}) {
            const Time th = t + hours / 24.0 / 365.0;  // within the day
            BOOST_CHECK_CLOSE(processForward(th), m.surface->fwd(d), 1.0e-12);
        }
        BOOST_CHECK_CLOSE(processForward(t), m.surface->fwd(d), 1.0e-12);
    }

    // it follows its curves
    RelinkableHandle<YieldTermStructure> foreign(m.foreignTs.currentLink());
    auto curve = ext::make_shared<FxSettlementForeignCurve>(foreign, m.domesticTs,
                                                           FxSettleConvention(WeekendsOnly(), 2));
    const Time t1Y = curve->timeFromReference(Date(2, January, 2025));
    const Real before = curve->discount(t1Y);
    Flag flag;
    flag.registerWith(curve);
    foreign.linkTo(ext::make_shared<FlatForward>(0, NullCalendar(), 0.05, m.dc));
    BOOST_CHECK(flag.isUp());
    BOOST_CHECK(curve->discount(t1Y) < before);
}

BOOST_AUTO_TEST_CASE(testFxLocalVolSurfaceOverWeekends) {
    BOOST_TEST_MESSAGE("Testing Dupire local vol where trading time slows over weekends...");

    FxMarket m;
    FxLocalVolSurface lv(m.process->blackVolatility(), m.process->riskFreeRate(),
                         m.process->dividendYield(), m.process->stateVariable());
    lv.enableExtrapolation();

    // defined and positive everywhere, and lower over weekends, where
    // variance accrues at a fifth of the rate
    Size weekend = 0, weekday = 0;
    Real weekendMax = 0.0, weekdayMin = QL_MAX_REAL;
    for (Real strike : {1.03, 1.06, 1.085, 1.11, 1.14}) {
        for (Size i = 1; i < 365; ++i) {
            const Time t = (i + 0.5) / 4.0 / 365.0;
            const Date d = m.today + Integer(std::floor(t * 365.0));
            Volatility vol = Null<Volatility>();
            BOOST_REQUIRE_NO_THROW(vol = lv.localVol(t, strike, true));
            BOOST_CHECK(vol > 0.0);
            if (d.weekday() == Saturday || d.weekday() == Sunday) {
                if (strike == 1.085)
                    weekendMax = std::max(weekendMax, vol);
                ++weekend;
            } else {
                if (strike == 1.085)
                    weekdayMin = std::min(weekdayMin, vol);
                ++weekday;
            }
        }
    }
    BOOST_CHECK(weekend > 0 && weekday > 0);
    BOOST_CHECK_LT(weekendMax, weekdayMin);

    // QuantLib's Dupire on the same inputs agrees
    LocalVolSurface upstream(m.process->blackVolatility(), m.process->riskFreeRate(),
                             m.process->dividendYield(), m.process->stateVariable());
    upstream.enableExtrapolation();
    for (Real strike : {1.06, 1.085, 1.11}) {
        const Time t = m.process->time(Date(14, February, 2024)) + 0.3 / 365.0;
        BOOST_CHECK_CLOSE(lv.localVol(t, strike, true), upstream.localVol(t, strike, true), 1.0e-10);
    }

    // a floor, where given, applies over weekends too, where local vol is
    // about 0.042 at the money
    FxLocalVolSurface floored(m.process->blackVolatility(), m.process->riskFreeRate(),
                              m.process->dividendYield(), m.process->stateVariable(), 1.0e-7, 0.05);
    floored.enableExtrapolation();
    const Time saturday = m.process->time(Date(3, February, 2024)) + 0.5 / 365.0;
    BOOST_CHECK_EQUAL(floored.localVol(saturday, 1.085, true), 0.05);
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
    FxVanillaBumpRisk riskCalc(m.makeAtmCall(), m.pairMarket());
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
        for (Real w : {0.2, 0.3, 1.0}) {
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
    for (Real w : {0.2, 0.3, 1.0}) {
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

    // with weekend days weighted 0.2, a week has 5.4 trading days whatever its first day
    TradingTimeTermStructure ts(today, WeekendsOnly(), 0.2);
    for (Integer start = 0; start < 7; ++start)
        BOOST_CHECK_SMALL(ts.tradingTime(today + start, today + start + 7) * 365.0 - 5.4, 1.0e-12);
}

BOOST_AUTO_TEST_CASE(testTradingTimeEventJumps) {
    BOOST_TEST_MESSAGE("Testing trading time jumps at events...");

    const Date today(5, January, 2024);  // a Friday
    Settings::instance().evaluationDate() = today;
    const Date eventDate(10, January, 2024);  // the next Wednesday
    const Real timeOfDay = 13.5 / 24.0;
    auto size = ext::make_shared<SimpleQuote>(2.0);
    TradingTimeTermStructure plain(today, WeekendsOnly(), 0.2);
    TradingTimeTermStructure ts(
        today, WeekendsOnly(), 0.2,
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
        today, WeekendsOnly(), 0.2,
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

    BOOST_CHECK_THROW(TradingTimeTermStructure(eventDate, WeekendsOnly(), 0.2,
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
    TradingTimeTermStructure plain(today, WeekendsOnly(), 0.2);
    auto ts = ext::make_shared<TradingTimeTermStructure>(
        today, WeekendsOnly(), 0.2,
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
    // more than the day's own rate cannot be taken away, nor all of it:
    // trading time must always run
    halfDay->setValue(-1.5);
    BOOST_CHECK_THROW(observed->tradingTime(christmasEve + 1), Error);
    halfDay->setValue(-1.0);
    BOOST_CHECK_THROW(observed->tradingTime(christmasEve + 1), Error);
    halfDay->setValue(-0.9);
    BOOST_CHECK_SMALL(days(*observed, christmasEve, christmasEve + 1) - 0.1, 1.0e-9);
}

BOOST_AUTO_TEST_CASE(testTradingTimeNegativeWeightsNeedCover) {
    BOOST_TEST_MESSAGE("Testing that negative event weights need trading time to take from...");

    const Date today(5, January, 2024);  // a Friday
    Settings::instance().evaluationDate() = today;
    const Date saturday(6, January, 2024);
    const auto negative = ext::make_shared<DiffuseTradingTimeEvent>(
        saturday, 10.0 / 24.0, saturday, 12.0 / 24.0, makeQuoteHandle(-0.05));
    // alone, it would take 0.6 days per day from a weekend running at 0.2,
    // and run trading time backwards
    TradingTimeTermStructure alone(today, WeekendsOnly(), 0.2, {negative});
    BOOST_CHECK_THROW(alone.tradingTime(today + 7), Error);
    // overlapped by an event giving enough for trading time to keep running, it is accepted
    const auto cover = ext::make_shared<DiffuseTradingTimeEvent>(
        saturday, 9.0 / 24.0, saturday, 13.0 / 24.0, makeQuoteHandle(0.1));
    TradingTimeTermStructure covered(today, WeekendsOnly(), 0.2, {negative, cover});
    BOOST_CHECK_SMALL(covered.tradingTime(today + 7) * 365.0 - 5.45, 1.0e-9);
    // a cover that is too small, or misses part of it, is not enough
    TradingTimeTermStructure tooSmall(
        today, WeekendsOnly(), 0.2,
        {negative, ext::make_shared<DiffuseTradingTimeEvent>(saturday, 9.0 / 24.0, saturday,
                                                             13.0 / 24.0, makeQuoteHandle(0.06))});
    BOOST_CHECK_THROW(tooSmall.tradingTime(today + 7), Error);
    TradingTimeTermStructure partial(
        today, WeekendsOnly(), 0.2,
        {negative, ext::make_shared<DiffuseTradingTimeEvent>(saturday, 11.0 / 24.0, saturday,
                                                             13.0 / 24.0, makeQuoteHandle(1.0))});
    BOOST_CHECK_THROW(partial.tradingTime(today + 7), Error);
    // a negative event already over by the reference date is not checked
    TradingTimeTermStructure past(saturday + 1, WeekendsOnly(), 0.2, {negative});
    BOOST_CHECK_NO_THROW(past.tradingTime(saturday + 8));
}

BOOST_AUTO_TEST_CASE(testTradingTimeEventInProgress) {
    BOOST_TEST_MESSAGE("Testing trading time with an event in progress at the reference date...");

    const Date today(10, January, 2024);  // a Wednesday
    Settings::instance().evaluationDate() = today;
    // released evenly from yesterday to tomorrow, so half of it by today
    TradingTimeTermStructure plain(today, WeekendsOnly(), 0.2);
    TradingTimeTermStructure ts(today, WeekendsOnly(), 0.2,
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
    TradingTimeTermStructure later(today + 1, WeekendsOnly(), 0.2, ts.events());
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
    TradingTimeTermStructure fixed(now, WeekendsOnly(), 0.2, events);
    TradingTimeTermStructure floating(WeekendsOnly(), 0.2, events);
    TradingTimeTermStructure fromDay(day, WeekendsOnly(), 0.2, events);
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

// ─────────────────────────────────────────────────────────────────────────────
//  P&L explain: tenor expiries, event variance, roll and explain
// ─────────────────────────────────────────────────────────────────────────────

namespace {

    struct ExplainMarket {
        std::vector<Period> tenors = {1 * Weeks, 1 * Months, 3 * Months, 6 * Months, 1 * Years};
        FxPairConventions conventions{"EUR",
                                      "USD",
                                      FxSettleConvention(WeekendsOnly(), 2),
                                      FxExpiryCut(10),
                                      DeltaVolQuote::Fwd,
                                      DeltaVolQuote::AtmDeltaNeutral,
                                      FxRrBfQuotes::SmileStrangle};
        FxVolSurfaceConfig config{FxSmileModel::Sabr, FxVolInterpolation::Ncp, 0.2};
        Date today = Date(2, January, 2024);  // a Tuesday

        static Handle<YieldTermStructure> curve(const Date& d, Rate r) {
            return Handle<YieldTermStructure>(
                ext::make_shared<FlatForward>(d, r, Actual365Fixed()));
        }

        static FxVolQuotes quotes() {
            return FxVolQuotes({1 * Weeks, 1 * Months, 3 * Months, 6 * Months, 1 * Years},
                               {0.25, 0.10}, {0.0800, 0.0820, 0.0850, 0.0900, 0.0930},
                               {{-0.010, -0.020}, {-0.012, -0.024}, {-0.014, -0.028},
                                {-0.016, -0.031}, {-0.017, -0.033}},
                               {{0.0035, 0.0075}, {0.0040, 0.0085}, {0.0045, 0.0095},
                                {0.0050, 0.0105}, {0.0055, 0.0115}});
        }

        FxPairMarket market(const Date& date, Real spot, Rate rFor, Rate rDom) const {
            return FxPairMarket(date, conventions, spot, curve(date, rFor), curve(date, rDom),
                                quotes(), {}, config);
        }
        FxPairMarket market() const { return market(today, 1.0850, 0.04, 0.0525); }
    };

    Real explainedSum(const FxPnlExplainResult& r) {
        return r.theta + r.volRollAtm + r.volRollRiskReversal + r.volRollButterfly +
               r.volRollOther + r.delta + r.gamma + r.spotSmileVega + r.spotSmileVanna +
               r.vegaAtm + r.vegaRiskReversal + r.vegaButterfly + r.vannaAtm +
               r.vannaRiskReversal + r.vannaButterfly + r.volga;
    }

    void printExplain(const std::string& name, const FxPnlExplainResult& r) {
        BOOST_TEST_MESSAGE(name << ": total " << r.total << " theta " << r.theta << " roll ["
                                << r.volRollAtm << ", " << r.volRollRiskReversal << ", "
                                << r.volRollButterfly << ", " << r.volRollOther << "] delta "
                                << r.delta << " gamma " << r.gamma << " spotSmile ["
                                << r.spotSmileVega << ", " << r.spotSmileVanna << "] vega ["
                                << r.vegaAtm << ", " << r.vegaRiskReversal << ", "
                                << r.vegaButterfly << "] vanna [" << r.vannaAtm << ", "
                                << r.vannaRiskReversal << ", " << r.vannaButterfly << "] volga "
                                << r.volga << " residual " << r.residual << " vols ["
                                << r.volToday << ", " << r.volRolled << ", " << r.volNext << "]");
    }

}

BOOST_AUTO_TEST_CASE(testTradingTimeCalendarTimeIsItsInverse) {
    BOOST_TEST_MESSAGE("Testing that calendar time inverts trading time...");

    const Date today(2, January, 2024);
    Settings::instance().evaluationDate() = today;
    const Date eventDate(10, January, 2024);
    const Real eventTime = 13.5 / 24.0, eventWeight = 2.0;
    TradingTimeTermStructure ts(today, TARGET(), 0.2,
                                {ext::make_shared<JumpTradingTimeEvent>(
                                    eventDate, eventTime, makeQuoteHandle(eventWeight))});

    // trading time at calendar times, in tenths of a day over a month, maps back
    for (Size i = 0; i <= 300; ++i) {
        const Time t = i * 0.1 / 365.0;
        BOOST_CHECK_SMALL(ts.calendarTime(ts.tradingTime(t)) - t, 1.0e-11);
    }
    // calendar time at trading times maps back, except within the jump
    const Time te = (eventDate - today + eventTime) / 365.0;
    const Time tauBefore = ts.tradingTime(te - 1.0e-9), tauAfter = ts.tradingTime(te);
    // the base trading time just before the event takes 1e-9 years too
    BOOST_CHECK_SMALL((tauAfter - tauBefore) * 365.0 - eventWeight, 1.0e-6);
    for (Size i = 0; i <= 300; ++i) {
        const Time tau = i * 0.1 / 365.0;
        const Time t = ts.calendarTime(tau);
        if (tau > tauBefore && tau < tauAfter)
            BOOST_CHECK_SMALL(t - te, 1.0e-11);  // the jump's trading time passes at its instant
        else
            BOOST_CHECK_SMALL(ts.tradingTime(t) - tau, 1.0e-11);
    }

    // trading time standing still would have no unique inverse: holidays
    // need a positive weight
    BOOST_CHECK_THROW(TradingTimeTermStructure(today, TARGET(), 0.0), Error);
}

BOOST_AUTO_TEST_CASE(testTradingTimeDayCounterAndCurve) {
    BOOST_TEST_MESSAGE("Testing curves on the trading-time clock...");

    FxMarket m(0.2);
    const TradingTimeDayCounter dc(m.timeTs);
    const Handle<YieldTermStructure> curve(
        ext::make_shared<TradingTimeYieldTermStructure>(m.domesticTs, m.timeTs));
    for (Integer d : {1, 4, 5, 6, 30, 91, 400}) {
        const Date date = m.today + d;
        // year fractions are trading times...
        BOOST_CHECK_SMALL(dc.yearFraction(m.today, date) - m.timeTs->tradingTime(date), 1.0e-15);
        BOOST_CHECK_SMALL(curve->timeFromReference(date) - m.timeTs->tradingTime(date), 1.0e-15);
        // ...and discount factors at dates are unchanged
        BOOST_CHECK_CLOSE(curve->discount(date), m.domesticTs->discount(date), 1.0e-9);
    }
    // a weekend takes little trading time but its carry: the rate per year of
    // trading time is higher over it than over a business day
    const Date friday(5, January, 2024), monday(8, January, 2024), tuesday(9, January, 2024);
    const Rate weekend = curve->forwardRate(friday + 1, monday, dc, Continuous).rate();
    const Rate businessDay = curve->forwardRate(monday, tuesday, dc, Continuous).rate();
    BOOST_CHECK_CLOSE(weekend / businessDay, 2.0 / 0.4, 1.0e-6);

    // both reference dates must agree
    const Handle<TradingTimeTermStructure> later(
        ext::make_shared<TradingTimeTermStructure>(m.today + 1, WeekendsOnly(), 0.2));
    BOOST_CHECK_THROW(TradingTimeYieldTermStructure(m.domesticTs, later), Error);
}

BOOST_AUTO_TEST_CASE(testFxPairMarketTradingTimePreservesPrices) {
    BOOST_TEST_MESSAGE("Testing that a pair market on the trading-time clock prices as in "
                       "calendar time...");

    // with an event, released in the middle of a trading day
    FxMarket m(0.2);
    const FxPairMarket market =
        m.pairMarket().withEvents({{Date(31, January, 2024), 0.8, 1.5e-5}});
    auto calendarProcess =
        makeFxLocalVolProcess(market.spotQuote(), market.foreignCurve(), market.domesticCurve(),
                              market.conventions().settlement(),
                              Handle<BlackVolTermStructure>(market.calendarTimeVolSurface()));
    const auto tradingProcess = market.blackProcess();
    const auto localVolProcess = market.localVolProcess();

    for (const Date& expiry : {Date(9, January, 2024), Date(1, February, 2024), m.expiry3M,
                               Date(2, May, 2024)}) {
        const Real fwd = market.forward(expiry);
        for (Real moneyness : {0.95, 1.0, 1.05}) {
            VanillaOption option(
                ext::make_shared<PlainVanillaPayoff>(Option::Call, moneyness * fwd),
                ext::make_shared<EuropeanExercise>(expiry));
            // a change of clock leaves Black prices unchanged...
            option.setPricingEngine(ext::make_shared<AnalyticEuropeanEngine>(calendarProcess));
            const Real calendarPrice = option.NPV();
            option.setPricingEngine(ext::make_shared<AnalyticEuropeanEngine>(tradingProcess));
            BOOST_CHECK_CLOSE(option.NPV(), calendarPrice, 1.0e-8);
            // ...and local vol in trading time reprices them
            option.setPricingEngine(ext::make_shared<FdBlackScholesVanillaEngine>(
                localVolProcess, 200, 200, 0, FdmSchemeDesc::Douglas(), true));
            // to 0.5%, or a millionth of the forward far out of the money
            const Real tolerance = std::max(0.005 * calendarPrice, 1.0e-6 * fwd);
            if (std::fabs(option.NPV() - calendarPrice) > tolerance)
                BOOST_ERROR("local vol in trading time misprices " << expiry << " at "
                            << moneyness << " F: " << option.NPV() << " against Black "
                            << calendarPrice);
        }
    }

    // local vol in trading time does not drop over a weekend, as in calendar time
    const Real s = market.spot();
    const Date friday(5, January, 2024), sunday(7, January, 2024);
    const Real calendarRatio =
        calendarProcess->localVolatility()->localVol(sunday, s, true) /
        calendarProcess->localVolatility()->localVol(friday, s, true);
    const auto& tradingLocalVol = localVolProcess->localVolatility();
    const Real tradingRatio =
        tradingLocalVol->localVol(sunday, s, true) / tradingLocalVol->localVol(friday, s, true);
    BOOST_CHECK_LT(calendarRatio, 0.6);
    BOOST_CHECK_SMALL(tradingRatio - 1.0, 0.2);
}

BOOST_AUTO_TEST_CASE(testFxTenorExpiryDates) {
    BOOST_TEST_MESSAGE("Testing FX tenor expiry and delivery dates...");

    const FxSettleConvention settlement(WeekendsOnly(), 2);
    const Date trade(2, January, 2024);  // Tuesday, spot Thursday 4 January

    struct Case { Period tenor; Date expiry, delivery; };
    const std::vector<Case> cases = {
        {1 * Days, Date(3, January, 2024), Date(5, January, 2024)},
        {1 * Weeks, Date(9, January, 2024), Date(11, January, 2024)},
        // 4 February is a Sunday: delivery rolls to Monday 5 February
        {1 * Months, Date(1, February, 2024), Date(5, February, 2024)},
        {3 * Months, Date(2, April, 2024), Date(4, April, 2024)},
        {6 * Months, Date(2, July, 2024), Date(4, July, 2024)},
        {1 * Years, Date(2, January, 2025), Date(6, January, 2025)},
    };
    for (const auto& c : cases) {
        BOOST_CHECK_EQUAL(fxExpiryDate(trade, c.tenor, settlement), c.expiry);
        BOOST_CHECK_EQUAL(fxDeliveryDate(trade, c.tenor, settlement), c.delivery);
        BOOST_CHECK_EQUAL(settlement.valueDate(fxExpiryDate(trade, c.tenor, settlement)),
                          c.delivery);
    }

    // end of month: spot on Thursday 29 February, the last business day of
    // the month, delivers on the last business day of March (Friday 29th)
    const Date eom(27, February, 2024);
    BOOST_CHECK_EQUAL(fxDeliveryDate(eom, 1 * Months, settlement), Date(29, March, 2024));
    BOOST_CHECK_EQUAL(fxExpiryDate(eom, 1 * Months, settlement), Date(27, March, 2024));

    BOOST_CHECK_THROW(fxExpiryDate(trade, 0 * Days, settlement), Error);
}

BOOST_AUTO_TEST_CASE(testFxEventVarianceIsHeldConstant) {
    BOOST_TEST_MESSAGE("Testing that FX vol events release their own variance...");

    ExplainMarket data;
    const Date event(17, January, 2024);  // a Wednesday inside the 1M pillar interval
    const Real v = 2.0e-5;
    const FxPairMarket m = data.market().withEvents({{event, 0.5, v}});

    // ATM variance released over the event's day, less that of an ordinary
    // business day in the same pillar interval, is the event's variance
    auto eventJump = [&](const FxPairMarket& market) {
        const FxVarianceSurface& s = *market.volSurface();
        const Real withEvent = s.atmVar(event + 1) - s.atmVar(event);
        const Real ordinary = s.atmVar(event) - s.atmVar(event - 1);
        return withEvent - ordinary;
    };
    BOOST_CHECK_CLOSE(eventJump(m), v, 1.0e-8);

    // bumping the ATMs moves the base vol but not the event's variance
    FxVolQuotes bumped = m.quotes();
    for (auto& a : bumped.atms)
        a += 0.01;
    BOOST_CHECK_CLOSE(eventJump(m.withQuotes(bumped)), v, 1.0e-8);

    // the event only redistributes variance within the pillar interval: the
    // pillars' (fitted) ATM vols are those of the surface without the event
    const FxPairMarket noEvent = m.withEvents({});
    const FxVarianceSurface& s = *m.volSurface();
    const FxVarianceSurface& s0 = *noEvent.volSurface();
    for (const auto& pillar : m.pillars())
        BOOST_CHECK_CLOSE(s.atmVol(pillar), s0.atmVol(pillar), 1.0e-12);
    // before the event the ATM variance is lower than without it, after it the same
    const Date oneMonth = m.pillars()[1];
    BOOST_CHECK_LT(s.atmVar(event), s0.atmVar(event));
    BOOST_CHECK_CLOSE(s.atmVar(oneMonth), s0.atmVar(oneMonth), 1.0e-12);

    // the event's variance cannot exceed the pillar's forward variance
    BOOST_CHECK_THROW(m.withEvents({{event, 0.5, 1.0e-3}}).volSurface(), Error);
}

BOOST_AUTO_TEST_CASE(testFxEventsOnExpiryDateFollowTheCut) {
    BOOST_TEST_MESSAGE("Testing that an expiry sees the events of its date before its cut...");

    ExplainMarket data;
    const FxExpiryCut& cut = data.conventions.cut();  // New York 10:00
    const Date wednesday(17, January, 2024);
    const Real morning = 8.5 / 24.0, afternoon = 14.0 / 24.0;

    // the release before the cut counts for the day's expiry, the one at the
    // cut and the one after it only for later expiries
    const std::vector<FxVolEvent> events = {
        {wednesday, morning, 1.0e-5},
        {wednesday, 10.0 / 24.0, 2.0e-5},
        {wednesday, afternoon, 4.0e-5}};
    BOOST_CHECK_CLOSE(fxEventVariance(events, data.today, wednesday, cut), 1.0e-5, 1.0e-12);
    BOOST_CHECK_CLOSE(fxEventVariance(events, data.today, wednesday + 1, cut), 7.0e-5, 1.0e-12);
    // a Tokyo 15:00 cut is 01:00 in New York: the morning release comes after it
    BOOST_CHECK_EQUAL(fxEventVariance(events, data.today, wednesday, FxExpiryCut(1)), 0.0);
    // times of day computed differently are the same instant
    BOOST_CHECK(!fxEventPrecedesExpiry({wednesday, 600.0 / 1440.0, 1.0}, wednesday, cut));
    BOOST_CHECK(!fxEventPrecedesExpiry({wednesday, 10.0 / 24.0 - 1.0e-12, 1.0}, wednesday, cut));
    BOOST_CHECK(fxEventPrecedesExpiry({wednesday - 1, afternoon, 1.0}, wednesday, cut));

    // on the surface: ATM variance released by an expiry over the day
    // before, less that of the ordinary business day before it
    auto jump = [&](const std::vector<FxVolEvent>& evs, const Date& day) {
        const FxPairMarket m = data.market().withEvents(evs);
        const FxVarianceSurface& s = *m.volSurface();
        return (s.atmVar(day) - s.atmVar(day - 1)) - (s.atmVar(day - 1) - s.atmVar(day - 2));
    };
    const Real v = 2.0e-5;
    // the Wednesday expiry sees the morning event...
    BOOST_CHECK_CLOSE(jump({{wednesday, morning, v}}, wednesday), v, 1.0e-6);
    // ...and the Thursday one the afternoon and the at-the-cut events
    BOOST_CHECK_CLOSE(jump({{wednesday, afternoon, v}}, wednesday + 1), v, 1.0e-6);
    BOOST_CHECK_CLOSE(jump({{wednesday, 10.0 / 24.0, v}}, wednesday + 1), v, 1.0e-6);
    BOOST_CHECK_SMALL(jump({{wednesday, afternoon, v}}, wednesday), 1.0e-12);

    // on a pillar date, the event belongs to the pillar's interval only if it
    // precedes the cut: the 1W pillar is Tuesday 9 January
    const Date oneWeek = fxExpiryDates(data.today, data.tenors, data.conventions.settlement())[0];
    auto variance = [&](const std::vector<FxVolEvent>& evs, const Date& d) {
        const FxPairMarket m = data.market().withEvents(evs);
        return m.volSurface()->atmVar(d);
    };
    // before the cut: released by the pillar, so the ATM variance the day
    // before is lower; after it: released in the next interval
    BOOST_CHECK_LT(variance({{oneWeek, morning, v}}, oneWeek - 1),
                   variance({}, oneWeek - 1) - 0.5 * v);
    BOOST_CHECK_CLOSE(variance({{oneWeek, afternoon, v}}, oneWeek - 1),
                      variance({}, oneWeek - 1), 1.0e-10);
    BOOST_CHECK_GT(variance({{oneWeek, afternoon, v}}, oneWeek + 1) -
                       variance({{oneWeek, afternoon, v}}, oneWeek),
                   variance({}, oneWeek + 1) - variance({}, oneWeek) + 0.5 * v);
}

BOOST_AUTO_TEST_CASE(testFxSurfaceRollHoldsEventlessVols) {
    BOOST_TEST_MESSAGE("Testing that the FX surface roll holds eventless vols by tenor...");

    ExplainMarket data;
    const Calendar cal = data.conventions.settlement().calendar();
    const Actual365Fixed dc;

    // quotes built from a flat eventless trading-time vol plus events; the
    // first event is released over the weekend roll, the second is not
    const Date friday(5, January, 2024), monday(8, January, 2024);
    const std::vector<FxVolEvent> events = {{Date(5, January, 2024), 0.5, 1.5e-5},
                                            {Date(10, January, 2024), 0.6, 2.0e-5}};
    const Volatility s = 0.08;
    auto marketFor = [&](const Date& date, const std::vector<FxVolEvent>& evs) {
        FxVolQuotes q = ExplainMarket::quotes();
        const TradingTimeTermStructure base(date, cal, data.config.holidayWeight);
        const std::vector<Date> pillars = fxExpiryDates(date, data.tenors, data.conventions.settlement());
        for (Size i = 0; i < pillars.size(); ++i) {
            const Real w = s * s * base.tradingTime(date, pillars[i]) +
                           fxEventVariance(evs, date, pillars[i], FxExpiryCut(10));
            q.atms[i] = std::sqrt(w / dc.yearFraction(date, pillars[i]));
            // a smile flat in vol, so that every point rolls like the ATM
            q.riskReversals[i] = {0.0, 0.0};
            q.butterflies[i] = {0.0, 0.0};
        }
        return data.market(date, 1.0850, 0.04, 0.0525).withQuotes(q).withEvents(evs);
    };

    // Thursday's market already contains Friday's event; roll it to Friday
    // (event released at the end of Friday), then over the weekend
    const FxPairMarket thursday = marketFor(Date(4, January, 2024), events);
    const FxPairMarket rolledToFriday = thursday.rolled(friday);
    const FxPairMarket expectedFriday = marketFor(friday, events);
    const FxPairMarket rolledToMonday = expectedFriday.rolled(monday);
    const FxPairMarket expectedMonday = marketFor(monday, events);
    for (Size i = 0; i < data.tenors.size(); ++i) {
        BOOST_CHECK_CLOSE(rolledToFriday.quotes().atms[i], expectedFriday.quotes().atms[i],
                          1.0e-10);
        BOOST_CHECK_CLOSE(rolledToMonday.quotes().atms[i], expectedMonday.quotes().atms[i],
                          1.0e-10);
        BOOST_CHECK_SMALL(rolledToMonday.quotes().riskReversals[i][0], 1.0e-14);
        BOOST_CHECK_SMALL(rolledToMonday.quotes().butterflies[i][1], 1.0e-14);
    }

    // the spot rolls along the forward, so the forward to each delivery is unchanged
    const Date spot0 = data.conventions.settlement().valueDate(friday);
    const Date spot1 = data.conventions.settlement().valueDate(monday);
    const Real fwdToSpot1 = expectedFriday.spot() *
                            expectedFriday.foreignCurve()->discount(spot1) /
                            expectedFriday.foreignCurve()->discount(spot0) /
                            (expectedFriday.domesticCurve()->discount(spot1) /
                             expectedFriday.domesticCurve()->discount(spot0));
    BOOST_CHECK_CLOSE(rolledToMonday.spot(), fwdToSpot1, 1.0e-12);
}

BOOST_AUTO_TEST_CASE(testFxPnlExplain) {
    BOOST_TEST_MESSAGE("Testing the FX vanilla P&L explain...");

    ExplainMarket data;
    const FxPairMarket today = data.market().withEvents({{Date(17, January, 2024), 0.5, 2.0e-5}});
    const Date next(3, January, 2024);
    const Date expiry = today.pillars()[2] - 5;  // between the 1M and 3M pillars
    const Real notional = 1.0e6;

    const std::vector<Real> moneynesses = {0.97, 1.0, 1.03};
    for (FxPnlExplainType explainType :
         {FxPnlExplainType::Black, FxPnlExplainType::StickyDelta, FxPnlExplainType::Bartlett}) {
    std::vector<FxPnlExplainResult> calls;
    for (Option::Type type : {Option::Call, Option::Put}) {
        for (Size k = 0; k < moneynesses.size(); ++k) {
            const Real strike = moneynesses[k] * 1.0850;

            // no market move: the next market is the rolled one, so only
            // theta and the roll are left, up to the roll's own second order
            const FxPairMarket rolled = today.rolled(next);
            const auto still =
                fxVanillaPnlExplain(type, strike, expiry, notional, today, rolled, explainType);
            printExplain("no move", still);
            // the residual is the roll's second order, excluded from the volga by design
            BOOST_CHECK_SMALL(still.residual, 0.01 * std::fabs(still.total));
            BOOST_CHECK_SMALL(still.delta, 1.0e-12 * notional);
            BOOST_CHECK_SMALL(still.vegaAtm + still.vegaRiskReversal + still.vegaButterfly,
                              1.0e-12 * notional);
            BOOST_CHECK_SMALL(still.spotSmileVega, 1.0e-12 * notional);
            BOOST_CHECK_CLOSE(still.volNext, still.volRolled, 1.0e-10);
            BOOST_CHECK_CLOSE(explainedSum(still) + still.residual, still.total, 1.0e-10);

            // a market move: spot, rates, every quote and the event's variance
            FxVolQuotes q = rolled.quotes();
            for (Size i = 0; i < q.atms.size(); ++i) {
                q.atms[i] += 0.0020;
                for (Size j = 0; j < 2; ++j) {
                    q.riskReversals[i][j] -= 0.0010;
                    q.butterflies[i][j] += 0.0005;
                }
            }
            const FxPairMarket moved = data.market(next, 1.0850 * 1.004, 0.0405, 0.0520)
                                           .withQuotes(q)
                                           .withEvents({{Date(17, January, 2024), 0.5, 2.5e-5}});
            const auto r =
                fxVanillaPnlExplain(type, strike, expiry, notional, today, moved, explainType);
            printExplain("moved", r);
            // the breakdown depends on the type, the total does not
            const auto black = fxVanillaPnlExplain(type, strike, expiry, notional, today, moved,
                                                   FxPnlExplainType::Black);
            BOOST_CHECK_CLOSE(r.total, black.total, 1.0e-10);
            if (explainType != FxPnlExplainType::Black) {
                BOOST_CHECK_EQUAL(r.spotSmileVega, 0.0);
                BOOST_CHECK_EQUAL(r.spotSmileVanna, 0.0);
            }
            BOOST_CHECK_CLOSE(explainedSum(r) + r.residual, r.total, 1.0e-10);
            BOOST_CHECK_CLOSE(r.forwardRolled, r.forwardToday, 1.0e-10);
            // the expansion explains the move up to a small residual
            BOOST_CHECK_SMALL(r.residual, 0.02 * std::fabs(r.total));
            const Real sign = type == Option::Call ? 1.0 : -1.0;
            BOOST_CHECK_GT(sign * r.delta * (r.forwardNext - r.forwardRolled), 0.0);

            if (type == Option::Call) {
                calls.push_back(r);
            } else {
                // vol terms depend on the strike and expiry only, not on the parity
                const auto& c = calls[k];
                BOOST_CHECK_CLOSE(r.vegaAtm, c.vegaAtm, 1.0e-10);
                BOOST_CHECK_CLOSE(r.vegaRiskReversal, c.vegaRiskReversal, 1.0e-10);
                BOOST_CHECK_CLOSE(r.volRollAtm, c.volRollAtm, 1.0e-10);
                BOOST_CHECK_CLOSE(r.volga, c.volga, 1.0e-10);
                BOOST_CHECK_CLOSE(r.gamma, c.gamma, 1.0e-10);
            }
        }
    }
    }
}

BOOST_AUTO_TEST_CASE(testFxPnlExplainBartlettVega) {
    BOOST_TEST_MESSAGE(
        "Testing that the Bartlett P&L explain leaves only unexpected vol moves in vega...");

    ExplainMarket data;
    const FxPairMarket today = data.market();
    const Date next(3, January, 2024);
    const Date expiry = today.pillars()[2] - 5;
    const FxPairMarket rolled = today.rolled(next);

    // the next market's ATMs move exactly as today's smiles expect given
    // each pillar's forward move; nothing else moves but the spot
    const FxPairMarket todayStrangles = today.smileStrangles();
    FxVolQuotes q = rolled.quotes();
    for (Size i = 0; i < data.tenors.size(); ++i) {
        const Real s =
            todayStrangles.volSurface()->pillarSmileSection(i)->atmVolForwardSensitivity();
        BOOST_CHECK(s != 0.0);
        q.atms[i] += s * 0.004;  // same curves, so every forward moves by the spot's 0.4%
    }
    const FxPairMarket moved = rolled.withSpot(rolled.spot() * 1.004).withQuotes(q);

    for (Real moneyness : {0.97, 1.0, 1.03}) {
        const Real strike = moneyness * 1.0850;
        const auto bartlett = fxVanillaPnlExplain(Option::Call, strike, expiry, 1.0e6, today,
                                                  moved, FxPnlExplainType::Bartlett);
        const auto sticky = fxVanillaPnlExplain(Option::Call, strike, expiry, 1.0e6, today,
                                                moved, FxPnlExplainType::StickyDelta);
        // the expected ATM move is in the Bartlett delta, not in its vega
        // (zero up to rounding in the pillar forwards and the smile fits)
        BOOST_CHECK_SMALL(bartlett.vegaAtm, 1.0e-8 * std::fabs(sticky.vegaAtm));
        BOOST_CHECK_SMALL(bartlett.vannaAtm, 1.0e-8 * std::fabs(sticky.vegaAtm));
        BOOST_CHECK_GT(std::fabs(sticky.vegaAtm), 1.0);
        BOOST_CHECK_CLOSE(bartlett.total, sticky.total, 1.0e-10);
        BOOST_CHECK_SMALL(bartlett.residual, 0.02 * std::fabs(bartlett.total));
    }
}

BOOST_AUTO_TEST_CASE(testFxPnlExplainEventReleasedOvernight) {
    BOOST_TEST_MESSAGE("Testing that the FX P&L explain rolls off an event released overnight...");

    ExplainMarket data;
    const Date next(3, January, 2024);
    const Date expiry = fxExpiryDates(data.today, data.tenors, data.conventions.settlement())[1] + 3;  // after the 1M pillar
    const Real v = 2.0e-5;
    const Actual365Fixed dc;

    // the same quotes with and without an event released at the end of the next date
    const FxPairMarket without = data.market(), withEvent = without.withEvents({{next, 0.5, v}});

    auto rolledVariances = [&](const FxPairMarket& today) {
        const FxPairMarket rolled = today.rolled(next);
        const Real strike = 1.0850;
        const auto r = fxVanillaPnlExplain(Option::Call, strike, expiry, 1.0e6, today, rolled);
        return std::make_pair(r.volToday * r.volToday * dc.yearFraction(data.today, expiry),
                              r.volRolled * r.volRolled * dc.yearFraction(next, expiry));
    };
    const auto [w0, w1] = rolledVariances(withEvent);
    const auto [u0, u1] = rolledVariances(without);
    // with the event the same quotes leave less variance for each ordinary
    // day, so the variance lost overnight is larger by the event's less the
    // smaller loss of the ordinary day: v times the remaining eventless trading
    // time over today's (up to the interpolation between the pillars)
    const Calendar& cal = data.conventions.settlement().calendar();
    const Real weight = data.config.holidayWeight;
    const TradingTimeTermStructure base0(data.today, cal, weight), base1(next, cal, weight);
    const Real expected =
        v * base1.tradingTime(next, expiry) / base0.tradingTime(data.today, expiry);
    BOOST_CHECK_CLOSE((w0 - w1) - (u0 - u1), expected, 2.0);
    BOOST_CHECK_LT((w0 - w1) - (u0 - u1), v);
}

// ─────────────────────────────────────────────────────────────────────────────
//  FX pair conventions, curves and markets
// ─────────────────────────────────────────────────────────────────────────────

namespace {

    FxPairConventions pairConventions(const std::string& foreign, const std::string& domestic) {
        return FxPairConventions(foreign, domestic, FxSettleConvention(WeekendsOnly(), 2),
                                 FxExpiryCut(10), DeltaVolQuote::Fwd, DeltaVolQuote::AtmFwd,
                                 FxRrBfQuotes::SmileStrangle);
    }

    // the tenors of the forward points
    const std::vector<Period> forwardTenors = {1 * Months, 3 * Months, 6 * Months, 1 * Years};

    // forward points of a pair with flat continuous rates
    std::vector<Real> forwardPoints(const FxPairConventions& conventions,
                                    const Date& asOf,
                                    Real spot,
                                    Rate rForeign,
                                    Rate rDomestic) {
        const Date spotDate = conventions.settlement().valueDate(asOf);
        std::vector<Real> points;
        for (const Period& tenor : forwardTenors) {
            const Date delivery =
                conventions.settlement().calendar().advance(spotDate, tenor, Following);
            const Time t = Actual365Fixed().yearFraction(spotDate, delivery);
            points.push_back(spot * (std::exp((rDomestic - rForeign) * t) - 1.0));
        }
        return points;
    }

    // forward to a delivery date, from the spot date of the as-of date
    Real forwardTo(const Date& asOf,
                   const FxPairConventions& conventions,
                   Real spot,
                   const YieldTermStructure& foreign,
                   const YieldTermStructure& domestic,
                   const Date& delivery) {
        const Date spotDate = conventions.settlement().valueDate(asOf);
        return spot * (foreign.discount(delivery) / foreign.discount(spotDate)) /
               (domestic.discount(delivery) / domestic.discount(spotDate));
    }

}

BOOST_AUTO_TEST_CASE(testFxPairConventions) {
    BOOST_TEST_MESSAGE("Testing FX pair conventions...");

    const FxPairConventions eurusd = pairConventions("EUR", "USD");
    BOOST_CHECK_EQUAL(eurusd.name(), "EURUSD");
    BOOST_CHECK(eurusd.withFlyType(FxRrBfQuotes::MarketStrangle).flyType() ==
                FxRrBfQuotes::MarketStrangle);
    BOOST_CHECK_THROW(pairConventions("EUR", "EUR"), Error);
    BOOST_CHECK_THROW(pairConventions("", "USD"), Error);

    // the tenors and deltas are the quotes': their expiries are the pillars,
    // and each tenor needs a quote for each delta
    FxMarket mkt;
    const FxPairMarket market = mkt.pairMarket();
    BOOST_REQUIRE_EQUAL(market.pillars().size(), Size(2));
    BOOST_CHECK_EQUAL(market.pillars()[0], Date(2, April, 2024));
    BOOST_CHECK_EQUAL(market.pillars()[1], Date(2, July, 2024));
    auto withQuotes = [&](std::vector<Period> tenors, std::vector<Real> deltas) {
        FxVolQuotes q = market.quotes();
        q.tenors = std::move(tenors);
        q.deltas = std::move(deltas);
        return market.withQuotes(q);
    };
    BOOST_CHECK_THROW(withQuotes({}, {0.25, 0.10}), Error);
    BOOST_CHECK_THROW(withQuotes({3 * Months}, {0.25, 0.10}), Error);
    BOOST_CHECK_THROW(withQuotes({3 * Months, 6 * Months}, {}), Error);
    BOOST_CHECK_THROW(withQuotes({3 * Months, 6 * Months}, {0.25, 0.5}), Error);
    BOOST_CHECK_THROW(withQuotes({3 * Months, 6 * Months}, {0.25}), Error);
}

BOOST_AUTO_TEST_CASE(testFxCurvesFromForwardPoints) {
    BOOST_TEST_MESSAGE("Testing FX curves bootstrapped from forward points...");

    const Date asOf(2, January, 2024);
    Settings::instance().evaluationDate() = asOf;
    const Handle<YieldTermStructure> usd(
        ext::make_shared<FlatForward>(asOf, 0.0525, Actual365Fixed()));

    // EUR/USD, collateral the domestic currency, and USD/JPY, collateral the
    // foreign one: both curves reprice their pair's forward points
    struct Case { std::string foreign, domestic; Real spot; Rate rForeign, rDomestic; };
    std::map<std::string, Handle<YieldTermStructure>> curves = {{"USD", usd}};
    std::map<std::string, Real> spots;
    for (const Case& c : {Case{"EUR", "USD", 1.0850, 0.04, 0.0525},
                          Case{"USD", "JPY", 150.0, 0.0525, 0.001}}) {
        const FxPairConventions conventions = pairConventions(c.foreign, c.domestic);
        const std::vector<Real> points =
            forwardPoints(conventions, asOf, c.spot, c.rForeign, c.rDomestic);
        const std::string other = c.foreign == "USD" ? c.domestic : c.foreign;
        const Handle<YieldTermStructure> curve(
            bootstrapFxCurve(asOf, conventions, c.spot, forwardTenors, points, "USD", usd));
        curves[other] = curve;
        spots[conventions.name()] = c.spot;

        const Date spotDate = conventions.settlement().valueDate(asOf);
        for (Size i = 0; i < points.size(); ++i) {
            const Date delivery = conventions.settlement().calendar().advance(
                spotDate, forwardTenors[i], Following);
            const Real fwd = forwardTo(asOf, conventions, c.spot, **curves[c.foreign],
                                       **curves[c.domestic], delivery);
            BOOST_TEST_INFO(conventions.name() << " " << forwardTenors[i]);
            BOOST_CHECK_CLOSE(fwd, c.spot + points[i], 1.0e-10);
        }
    }

    // on one collateral, forwards are consistent by construction: EUR/JPY
    // is EUR/USD times USD/JPY at every delivery
    const FxPairConventions eurjpy = pairConventions("EUR", "JPY");
    const Real eurjpySpot = spots["EURUSD"] * spots["USDJPY"];
    for (const Date& delivery : {Date(15, March, 2024), Date(3, October, 2024)}) {
        const Real cross =
            forwardTo(asOf, eurjpy, eurjpySpot, **curves["EUR"], **curves["JPY"], delivery);
        const Real legs = forwardTo(asOf, eurjpy, spots["EURUSD"], **curves["EUR"],
                                    **curves["USD"], delivery) *
                          forwardTo(asOf, eurjpy, spots["USDJPY"], **curves["USD"],
                                    **curves["JPY"], delivery);
        BOOST_CHECK_CLOSE(cross, legs, 1.0e-12);
    }

    // the collateral must be one of the pair's currencies, the points one
    // per tenor, and the evaluation date the as-of date
    const FxPairConventions eurusd = pairConventions("EUR", "USD");
    const std::vector<Real> points = forwardPoints(eurusd, asOf, 1.0850, 0.04, 0.0525);
    BOOST_CHECK_THROW(
        bootstrapFxCurve(asOf, eurusd, 1.0850, forwardTenors, points, "GBP", usd), Error);
    BOOST_CHECK_THROW(bootstrapFxCurve(asOf, eurusd, 1.0850, forwardTenors,
                                       {points[0], points[1]}, "USD", usd),
                      Error);
    Settings::instance().evaluationDate() = asOf + 1;
    BOOST_CHECK_THROW(
        bootstrapFxCurve(asOf, eurusd, 1.0850, forwardTenors, points, "USD", usd), Error);
}

BOOST_AUTO_TEST_CASE(testFxPairMarketForwardsAgree) {
    BOOST_TEST_MESSAGE("Testing that an FX pair market's surface and processes share its "
                       "forward...");

    FxMarket mkt;
    const FxPairMarket market = mkt.pairMarket();
    const auto black = market.blackProcess();
    for (const Date& expiry : {Date(9, January, 2024), mkt.expiry3M, Date(5, May, 2024)}) {
        const Real fwd = market.forward(expiry);
        BOOST_CHECK_CLOSE(market.volSurface()->fwd(expiry), fwd, 1.0e-12);
        const Real processForward = market.spot() * black->dividendYield()->discount(expiry) /
                                    black->riskFreeRate()->discount(expiry);
        BOOST_CHECK_CLOSE(processForward, fwd, 1.0e-10);
        // the same surface and forward as the test market's own surface
        BOOST_CHECK_CLOSE(mkt.surface->fwd(expiry), fwd, 1.0e-12);
        BOOST_CHECK_CLOSE(market.marketVol(expiry, 1.05 * fwd),
                          mkt.surface->marketVol(expiry, 1.05 * fwd), 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(testFxPairMarketIsImmutable) {
    BOOST_TEST_MESSAGE("Testing that an FX pair market does not change...");

    FxMarket mkt;
    const FxPairMarket market = mkt.pairMarket();
    const Real K = 1.10;
    const Volatility vol = market.marketVol(mkt.expiry3M, K);

    // other markets leave this one as it was...
    const FxPairMarket moved = market.withSpot(1.10);
    FxVolQuotes bumped = market.quotes();
    bumped.atms[0] += 0.01;
    const FxPairMarket higher = market.withQuotes(bumped);
    BOOST_CHECK(std::fabs(moved.marketVol(mkt.expiry3M, K) - vol) > 1.0e-6);
    BOOST_CHECK_GT(higher.marketVol(mkt.expiry3M, K), vol + 0.005);
    BOOST_CHECK_EQUAL(market.spot(), mkt.spot->value());
    BOOST_CHECK_EQUAL(market.marketVol(mkt.expiry3M, K), vol);

    // ...and so do the greeks, which bump copies
    FxVanillaBumpRisk riskCalc(mkt.makeAtmCall(), market, 1.0e6, 0.001, 0.01, 0.001, 0.001, 50,
                               50);
    riskCalc.calculate(true);
    BOOST_CHECK_EQUAL(market.marketVol(mkt.expiry3M, K), vol);

    // its dates are fixed: a later evaluation date does not move its surface,
    // but the greeks need the evaluation date to be the market's
    Settings::instance().evaluationDate() = mkt.today + 1;
    BOOST_CHECK_EQUAL(market.marketVol(mkt.expiry3M, K), vol);
    BOOST_CHECK_THROW(riskCalc.calculate(false), Error);
}

BOOST_AUTO_TEST_CASE(testFxPairMarketModelChoice) {
    BOOST_TEST_MESSAGE("Testing FX pair markets with smile models chosen at run time...");

    FxMarket mkt;
    const FxPairMarket sabr = mkt.pairMarket();
    // the model is the one chosen, whatever the interpolation; at a pillar,
    // the quotes read off the surface are those of the pillar's smile (which
    // misses the market's: even SVI's five parameters do not fit this put
    // skew exactly, its wings being bounded)
    for (FxVolInterpolation interpolation : {FxVolInterpolation::Ncp, FxVolInterpolation::Clark}) {
        const FxPairMarket svi(sabr.asOf(), sabr.conventions(), sabr.spot(), sabr.foreignCurve(),
                               sabr.domesticCurve(), sabr.quotes(), {},
                               FxVolSurfaceConfig(FxSmileModel::Svi, interpolation, 0.2));
        for (Size i = 0; i < svi.pillars().size(); ++i) {
            const auto section = svi.volSurface()->pillarSmileSection(i);
            BOOST_CHECK(ext::dynamic_pointer_cast<FxSviSmileSection>(section));
            const FxExpiryQuotes q = svi.expiryQuotes(svi.pillars()[i]);
            BOOST_CHECK_SMALL(q.atm - section->atmVol(), 1.0e-10);
            for (Size j = 0; j < 2; ++j) {
                const Real d = svi.quotes().deltas[j];
                const Volatility call = section->volByDelta(d, Option::Call);
                const Volatility put = section->volByDelta(-d, Option::Put);
                BOOST_CHECK_SMALL(q.riskReversals[j] - (call - put), 1.0e-10);
                BOOST_CHECK_SMALL(q.butterflies[j] - ((call + put) / 2.0 - section->atmVol()),
                                  1.0e-10);
            }
        }
    }
    // every model builds a surface, whose market vols at a pillar are its
    // pillar smile's
    for (FxSmileModel model :
         {FxSmileModel::Sabr, FxSmileModel::Svi, FxSmileModel::Polynomial, FxSmileModel::Quadratic,
          FxSmileModel::CostFlatDynamics, FxSmileModel::CostScaledDynamics,
          FxSmileModel::CostStickyDelta, FxSmileModel::CostStickyDeltaConvex}) {
        const FxPairMarket m(sabr.asOf(), sabr.conventions(), sabr.spot(), sabr.foreignCurve(),
                             sabr.domesticCurve(), sabr.quotes(), {}, FxVolSurfaceConfig(model));
        BOOST_TEST_INFO(model);
        BOOST_CHECK(m.volSurface()->smileModel() == model);
        const Date& pillar = m.pillars()[0];
        const Real K = 1.02 * m.forward(pillar);
        BOOST_CHECK_CLOSE(m.marketVol(pillar, K),
                          m.volSurface()->pillarSmileSection(0)->volByStrike(K), 1.0e-8);
    }

    // the models differ off the quotes
    const FxPairMarket polynomial(sabr.asOf(), sabr.conventions(), sabr.spot(),
                                  sabr.foreignCurve(), sabr.domesticCurve(), sabr.quotes(), {},
                                  FxVolSurfaceConfig(FxSmileModel::Polynomial));
    const Real farStrike = 0.9 * sabr.forward(mkt.expiry3M);
    BOOST_CHECK(std::fabs(polynomial.marketVol(mkt.expiry3M, farStrike) -
                          sabr.marketVol(mkt.expiry3M, farStrike)) > 1.0e-5);

    // the quotes must match the conventions' tenors and deltas
    FxVolQuotes missing = sabr.quotes();
    missing.atms.pop_back();
    BOOST_CHECK_THROW(sabr.withQuotes(missing).volSurface(), Error);
}

BOOST_AUTO_TEST_CASE(testFxPairMarketRoll) {
    BOOST_TEST_MESSAGE("Testing the roll of an FX pair market...");

    FxMarket mkt;
    const FxPairMarket market = mkt.pairMarket();
    const Date next(3, January, 2024);
    const FxPairMarket rolled = market.rolled(next);
    BOOST_CHECK_EQUAL(rolled.asOf(), next);
    BOOST_CHECK_EQUAL(rolled.pillars()[0],
                      fxExpiryDates(next, market.quotes().tenors, market.conventions().settlement())[0]);
    // the spot rolls along the forward: every forward to a delivery is unchanged
    for (const Date& expiry : {mkt.expiry3M, Date(5, May, 2024)})
        BOOST_CHECK_CLOSE(rolled.forward(expiry), market.forward(expiry), 1.0e-12);
    BOOST_CHECK_THROW(market.rolled(market.asOf()), Error);
}

BOOST_AUTO_TEST_CASE(testFxVolSurfaceTimeAxes) {
    BOOST_TEST_MESSAGE("Testing the time axes of an FX variance surface...");

    FxMarket mkt;
    const FxPairMarket market = mkt.pairMarket();
    const auto& surface = market.volSurface();
    const FxVarianceSurface& fx = *market.volSurface();
    const auto calendar = market.calendarTimeVolSurface();

    // the surface's axis is trading time, the view's calendar time
    BOOST_CHECK_EQUAL(surface->dayCounter().name(), "Trading time");
    BOOST_CHECK_EQUAL(calendar->dayCounter().name(), Actual365Fixed().name());

    const Actual365Fixed dc;
    for (const Date& d : {Date(9, January, 2024), Date(13, January, 2024), mkt.expiry3M,
                          Date(5, May, 2024), mkt.expiry6M}) {
        const Real fwd = market.forward(d);
        const Time t = dc.yearFraction(mkt.today, d);
        const Time tau = surface->timeFromReference(d);
        for (Real m : {0.95, 1.0, 1.05}) {
            const Real K = m * fwd;
            // total variance at a date is the same on both axes...
            BOOST_CHECK_CLOSE(calendar->blackVariance(d, K), surface->blackVariance(d, K),
                              1.0e-10);
            // ...the vols differ by the clocks: blackVol is per year of
            // trading time, marketVol per year of calendar time, as quoted
            BOOST_CHECK_CLOSE(fx.marketVol(d, K) * std::sqrt(t),
                              surface->blackVol(d, K) * std::sqrt(tau), 1.0e-10);
            BOOST_CHECK_CLOSE(calendar->blackVol(d, K), fx.marketVol(d, K), 1.0e-10);
        }
    }

    // at a pillar, the market vols are the pillar smile's
    for (Size i = 0; i < market.pillars().size(); ++i) {
        const Date& pillar = market.pillars()[i];
        const auto section = fx.pillarSmileSection(i);
        for (Real m : {0.95, 1.0, 1.05}) {
            const Real K = m * market.forward(pillar);
            BOOST_CHECK_CLOSE(fx.marketVol(pillar, K), section->volByStrike(K), 1.0e-8);
        }
        BOOST_CHECK_CLOSE(fx.atmVol(pillar), section->atmVol(), 1.0e-8);
    }

    // a process needs the surface and its curves on the same axis
    const FxSettleConvention& settlement = market.conventions().settlement();
    BOOST_CHECK_THROW(makeFxLocalVolProcess(market.spotQuote(), market.foreignCurve(),
                                            market.domesticCurve(), settlement,
                                            Handle<BlackVolTermStructure>(surface)),
                      Error);
    BOOST_CHECK_NO_THROW(makeFxLocalVolProcess(
        market.spotQuote(), market.tradingTimeForeignCurve(), market.tradingTimeDomesticCurve(),
        settlement, Handle<BlackVolTermStructure>(surface)));
    BOOST_CHECK_NO_THROW(makeFxLocalVolProcess(market.spotQuote(), market.foreignCurve(),
                                               market.domesticCurve(), settlement,
                                               Handle<BlackVolTermStructure>(calendar)));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
