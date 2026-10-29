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

/*! \file fxsmilesections.cpp
    \brief Tests for FX smile section calibration (strike- and delta-parameterized).
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/fxslv/fxdeltaconverter.hpp>
#include <ql/experimental/fxslv/fxsettlementconvention.hpp>
#include <ql/experimental/fxslv/fxsmilequotes.hpp>
#include <ql/experimental/fxslv/fxsmilesection.hpp>
#include <ql/experimental/fxslv/fxsmilesectionbystrike.hpp>
#include <ql/experimental/fxslv/fxsmilesectionbydelta.hpp>
#include <ql/experimental/fxslv/fxcostsmilesection.hpp>
#include <ql/experimental/fxslv/fxstickydeltasolver.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/blackdeltacalculator.hpp>
#include <ql/quotes/deltavolquote.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/date.hpp>
#include <ql/settings.hpp>
#include <algorithm>
#include <functional>
#include <limits>
#include <sstream>
#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(FxSmileSectionTests)

// ---------------------------------------------------------------------------
//  Shared market data used by all tests.
// ---------------------------------------------------------------------------

namespace {

    struct MarketData {
        Date todaysDate;
        Date expiryDate;
        Handle<YieldTermStructure> forDiscount;
        Handle<YieldTermStructure> domDiscount;
        Handle<Quote> spot;
        Handle<Quote> v_atm;
        Handle<Quote> v_25rr;
        Handle<Quote> v_10rr;
        Handle<Quote> v_25bf;
        Handle<Quote> v_10bf;
        std::vector<Real> deltas;
        DeltaVolQuote::DeltaType deltaType;
        DeltaVolQuote::AtmType atmType;
        FxRrBfQuotes::FlyType flyType;
        FxSettleConvention settlement{WeekendsOnly(), 2};

        ext::shared_ptr<FxSmileQuotes> rrBfQuotes() const {
            return ext::make_shared<FxRrBfQuotes>(
                v_atm, std::vector<Handle<Quote>>{v_25rr, v_10rr},
                std::vector<Handle<Quote>>{v_25bf, v_10bf}, deltas, flyType);
        }

        // Derived market vols (smile-strangle convention)
        // 25D: call = atm + rr/2 + bf,  put = atm - rr/2 + bf
        // 10D: call = atm + rr/2 + bf,  put = atm - rr/2 + bf
        Real v_25c, v_25p, v_10c, v_10p;

        MarketData() {
            todaysDate = Date(2, January, 2024);
            expiryDate = Date(2, January, 2025);
            Settings::instance().evaluationDate() = todaysDate;

            forDiscount = Handle<YieldTermStructure>(
                ext::make_shared<FlatForward>(todaysDate, 0.05, Actual365Fixed()));
            domDiscount = Handle<YieldTermStructure>(
                ext::make_shared<FlatForward>(todaysDate, 0.03, Actual365Fixed()));

            spot  = makeQuoteHandle(1.7554);
            v_atm = makeQuoteHandle(0.14483);
            v_25rr = makeQuoteHandle(0.02770);
            v_10rr = makeQuoteHandle(0.048752);
            v_25bf = makeQuoteHandle(0.007425);
            v_10bf = makeQuoteHandle(0.02376);

            deltas    = {0.25, 0.1};
            deltaType = DeltaVolQuote::PaSpot;
            atmType   = DeltaVolQuote::AtmFwd;
            flyType   = FxRrBfQuotes::SmileStrangle;

            // In smile-strangle convention the market quotes are:
            //   bf  = (call_vol + put_vol)/2 - atm
            //   rr  = call_vol - put_vol
            // => call_vol = atm + rr/2 + bf
            //    put_vol  = atm - rr/2 + bf
            Real atm = v_atm->value();
            v_25c = atm + 0.5 * v_25rr->value() + v_25bf->value();
            v_25p = atm - 0.5 * v_25rr->value() + v_25bf->value();
            v_10c = atm + 0.5 * v_10rr->value() + v_10bf->value();
            v_10p = atm - 0.5 * v_10rr->value() + v_10bf->value();
        }
    };

    // Check the fit to the market quotes.  Most models have fewer
    // parameters than the five quotes, so they cannot reproduce them; the
    // bound on the RMS error, in vol, is then a regression limit set a
    // little above the current fit.
    void checkSmileSection(const FxSmileSection& ss, Real maxError) {
        BOOST_CHECK_EQUAL(ss.calibrationResiduals().size(), Size(5));
        BOOST_CHECK_MESSAGE(ss.calibrationError() < maxError,
                            "calibration error too large: " << ss.calibrationError()
                                << " (limit " << maxError << ")");
    }

    typedef std::function<ext::shared_ptr<FxSmileSection>(const ext::shared_ptr<FxSmileQuotes>&)>
        SectionFactory;

    template <class Section>
    SectionFactory sectionFactory(const MarketData& md) {
        return [&md](const ext::shared_ptr<FxSmileQuotes>& quotes) {
            return ext::make_shared<Section>(md.expiryDate, md.spot, quotes, md.forDiscount,
                                             md.domDiscount, md.deltaType, md.atmType,
                                             Actual365Fixed(), md.settlement);
        };
    }

    // Quotes read off a calibrated smile at the market's deltas: the model
    // fits these exactly, with the parameters it already has.
    ext::shared_ptr<FxSmileQuotes> quotesFromSmile(const FxSmileSection& ss) {
        const Time t = ss.exerciseTime();
        const DeltaVolQuote::DeltaType dt = ss.deltaType();
        std::vector<Handle<DeltaVolQuote>> q;
        q.emplace_back(
            ext::make_shared<DeltaVolQuote>(makeQuoteHandle(ss.atmVol()), dt, t, ss.atmType()));
        for (Real d : {0.25, 0.10}) {
            q.emplace_back(ext::make_shared<DeltaVolQuote>(
                d, makeQuoteHandle(ss.volByDelta(d, Option::Call)), t, dt));
            q.emplace_back(ext::make_shared<DeltaVolQuote>(
                -d, makeQuoteHandle(ss.volByDelta(-d, Option::Put)), t, dt));
        }
        return ext::make_shared<FxDeltaVolQuotes>(q);
    }

    // Two sections give the same smile.
    void checkSameSmile(const std::string& name,
                        const FxSmileSection& a,
                        const FxSmileSection& b,
                        Real tolerance) {
        for (Real m : {0.8, 0.9, 1.0, 1.1, 1.2}) {
            const Rate K = m * a.forward();
            BOOST_CHECK_MESSAGE(std::fabs(a.volByStrike(K) - b.volByStrike(K)) < tolerance,
                                name << ": smiles differ at K/F = " << m << ": "
                                     << a.volByStrike(K) << " vs " << b.volByStrike(K));
        }
    }

    // Check that volByStrike and volByDelta are consistent via the
    // strikeByDelta round-trip.
    void checkStrikeDeltaConsistency(FxSmileSection& ss,
                                     const MarketData& /*md*/,
                                     Real tol = 1.0e-6) {
        std::vector<Real> testDeltas = {-0.10, -0.25, -0.40};
        for (Real delta : testDeltas) {
            Rate k = ss.strikeByDelta(delta, Option::Put);
            Volatility v_delta = ss.volByDelta(delta, Option::Put);
            Volatility v_strike = ss.volByStrike(k);
            Real error = std::fabs(v_delta - v_strike);
            BOOST_CHECK_MESSAGE(error < tol,
                "volByDelta / volByStrike inconsistency at delta=" << delta
                << ": v_delta=" << v_delta
                << " v_strike=" << v_strike
                << " error=" << error);
        }
    }

} // anonymous namespace


// ---------------------------------------------------------------------------
//  1. polynomialSmileSection (strike-parameterized, 3 params)
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(testPolynomialSmileSection) {
    BOOST_TEST_MESSAGE("Testing polynomial smile section calibration...");

    MarketData md;

    FxPolynomialSmileSection ss(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount, md.domDiscount,
                                md.deltaType, md.atmType,
                                Actual365Fixed(), md.settlement);

    // 3 parameters for 5 quotes
    checkSmileSection(ss, 15.0e-4);
    checkStrikeDeltaConsistency(ss, md);
}


// ---------------------------------------------------------------------------
//  2. fxSabrSmileSection (strike-parameterized, 3 params: alpha, nu, rho)
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(testSabrSmileSection) {
    BOOST_TEST_MESSAGE("Testing FX SABR smile section calibration...");

    MarketData md;

    FxSabrSmileSection ss(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount, md.domDiscount,
                          md.deltaType, md.atmType,
                          Actual365Fixed(), md.settlement);

    // 3 free parameters (alpha, nu, rho) for 5 quotes
    checkSmileSection(ss, 25.0e-4);
    checkStrikeDeltaConsistency(ss, md);

    // parameter accessors calibrate first, so they work on a fresh section too
    FxSabrSmileSection fresh(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount,
                             md.domDiscount, md.deltaType, md.atmType, Actual365Fixed(),
                             md.settlement);
    BOOST_CHECK_EQUAL(fresh.alpha(), ss.alpha());
    BOOST_CHECK_EQUAL(fresh.rho(), ss.rho());

    // Sanity-check parameter bounds
    BOOST_CHECK_MESSAGE(ss.alpha() > 0.0, "SABR alpha should be positive");
    BOOST_CHECK_MESSAGE(ss.nu() > 0.0, "SABR nu should be positive");
    BOOST_CHECK_MESSAGE(ss.rho() > -1.0 && ss.rho() < 1.0,
                        "SABR rho should be in (-1,1)");
}


// ---------------------------------------------------------------------------
//  3. fxSviSmileSection (strike-parameterized, 5 params: a,b,rho,m,sigma)
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(testSviSmileSection) {
    BOOST_TEST_MESSAGE("Testing FX SVI smile section calibration...");

    MarketData md;

    FxSviSmileSection ss(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount, md.domDiscount,
                         md.deltaType, md.atmType,
                         Actual365Fixed(), md.settlement);

    // 5 parameters for 5 quotes: the fit is exact
    checkSmileSection(ss, 1.0e-6);
    checkStrikeDeltaConsistency(ss, md);
}


// ---------------------------------------------------------------------------
//  4. quadraticSmileSection (delta-parameterized, 3 params: a, b, c)
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(testQuadraticSmileSection) {
    BOOST_TEST_MESSAGE("Testing quadratic (delta-parameterized) smile section calibration...");

    MarketData md;

    FxQuadraticSmileSection ss(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount, md.domDiscount,
                               md.deltaType, md.atmType,
                               Actual365Fixed(), md.settlement);

    // 3 parameters for 5 quotes; residuals in put delta coordinates
    checkSmileSection(ss, 30.0e-4);
    checkStrikeDeltaConsistency(ss, md, 1.0e-5);
}


// ---------------------------------------------------------------------------
//  5. fxCostSmileSectionFlatDynamics
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(testCostSmileSectionFlatDynamics) {
    BOOST_TEST_MESSAGE("Testing FX cost smile section (flat dynamics) calibration...");

    MarketData md;

    FxCostSmileSectionFlatDynamics ss(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount, md.domDiscount,
                                      md.deltaType, md.atmType,
                                      Actual365Fixed(), md.settlement, Date(), true);

    // 3 free coefficients for 5 quotes, fitted in closed form
    checkSmileSection(ss, 17.0e-4);
    checkStrikeDeltaConsistency(ss, md, 1.0e-5);
}


// ---------------------------------------------------------------------------
//  6. fxCostSmileSectionScaledDynamics
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(testCostSmileSectionScaledDynamics) {
    BOOST_TEST_MESSAGE("Testing FX cost smile section (scaled dynamics) calibration...");

    MarketData md;

    FxCostSmileSectionScaledDynamics ss(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount, md.domDiscount,
                                        md.deltaType, md.atmType,
                                        Actual365Fixed(), md.settlement, Date(), true);

    // 3 free coefficients for 5 quotes, fitted in closed form
    checkSmileSection(ss, 35.0e-4);
    checkStrikeDeltaConsistency(ss, md, 1.0e-5);
}


// ---------------------------------------------------------------------------
//  6b. Sticky-delta cost smile: the ODE solver and the smile section
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(testStickyDeltaSmileSolver) {
    BOOST_TEST_MESSAGE("Testing the sticky-delta cost smile ODE solver...");

    const std::vector<Real> costs = {1.0, -15.3166, -0.06278, 0.28742};
    const std::vector<Real> xs = {-0.08, -0.03, 0.0, 0.03, 0.08};

    // reference values from an independent implementation (Python, RK4 node
    // to node with a 1e-13 tolerance, Hermite interpolation)
    const std::vector<Real> refStd = {0.040779300983, 0.032947238825, 0.032620876353,
                                      0.035379624984, 0.045066283516};
    const std::vector<Real> refPa = {0.040868871525, 0.033074416691, 0.032755032154,
                                     0.035508341595, 0.045174432271};

    for (bool pa : {false, true}) {
        const FxStickyDeltaSmileSolver solver(costs, pa, -0.1, 0.1);
        const std::vector<Real>& ref = pa ? refPa : refStd;

        // the singular point d- = 0 is on the smile
        const Real ws = solver.singularTotalVol();
        BOOST_CHECK_CLOSE(ws, pa ? 1.0 / (2.0 * (costs[2] - costs[1])) : -1.0 / (2.0 * costs[1]),
                          1.0e-12);
        BOOST_CHECK_CLOSE(solver.singularLogMoneyness(), ws * ws / 2.0, 1.0e-12);
        BOOST_CHECK_SMALL(solver.totalVol(solver.singularLogMoneyness()) - ws, 1.0e-15);
        BOOST_CHECK(solver.minLogMoneyness() <= -0.1 && solver.maxLogMoneyness() >= 0.1);

        for (Size i = 0; i < xs.size(); ++i)
            BOOST_CHECK_SMALL(solver.totalVol(xs[i]) - ref[i], 1.0e-9);

        for (Real x = -0.095; x < 0.1; x += 0.005) {
            const Real w = solver.totalVol(x), beta = solver.slope(x);
            // the slope is that of the smile...
            const Real h = 1.0e-5;
            const Real fd = -(solver.totalVol(x + h) - solver.totalVol(x - h)) / (2.0 * h);
            BOOST_CHECK_MESSAGE(std::fabs(beta - fd) < 1.0e-6,
                                "slope " << beta << " differs from the smile's " << fd
                                         << " at x = " << x);
            // ...and solves the cost equation there
            const Real root = solver.quadraticRoot(x, w, beta);
            BOOST_CHECK_MESSAGE(std::fabs(root - beta) < 1.0e-5,
                                "slope " << beta << " does not solve the cost equation ("
                                         << root << ") at x = " << x);
        }
    }

    // with premium adjustment the smile is the unadjusted one with
    // Ctheta replaced by Ctheta + Ceta
    const FxStickyDeltaSmileSolver unadjusted(costs, false, -0.1, 0.1);
    const FxStickyDeltaSmileSolver adjusted({costs[0], costs[1] + costs[2], costs[2], costs[3]},
                                            true, -0.1, 0.1);
    for (Real x : xs)
        BOOST_CHECK_SMALL(adjusted.totalVol(x) - unadjusted.totalVol(x), 1.0e-12);

    // only the cost ratios matter
    const FxStickyDeltaSmileSolver scaled({2.0, 2.0 * costs[1], 2.0 * costs[2], 2.0 * costs[3]},
                                          false, -0.1, 0.1);
    for (Real x : xs)
        BOOST_CHECK_SMALL(scaled.totalVol(x) - unadjusted.totalVol(x), 1.0e-12);

    // and the solver recognises its costs at any positive scale, but not others
    BOOST_CHECK(unadjusted.hasCosts(costs));
    BOOST_CHECK(unadjusted.hasCosts(scaled.costs()));
    BOOST_CHECK(unadjusted.hasCosts({0.37, 0.37 * costs[1], 0.37 * costs[2], 0.37 * costs[3]}));
    BOOST_CHECK(!unadjusted.hasCosts({1.0, costs[1] * (1.0 + 1.0e-9), costs[2], costs[3]}));
    BOOST_CHECK(!unadjusted.hasCosts({-1.0, -costs[1], -costs[2], -costs[3]}));
    BOOST_CHECK(!unadjusted.hasCosts({1.0, costs[1], costs[2]}));

    // costs without a positive total vol at d- = 0 give no smile
    BOOST_CHECK_THROW(FxStickyDeltaSmileSolver({1.0, 0.5, -0.06, 0.29}, false, -0.1, 0.1), Error);
    BOOST_CHECK_THROW(FxStickyDeltaSmileSolver({-1.0, -15.0, -0.06, 0.29}, false, -0.1, 0.1),
                      Error);
}

BOOST_AUTO_TEST_CASE(testCostSmileSectionStickyDelta) {
    BOOST_TEST_MESSAGE("Testing FX cost smile section (sticky delta) calibration...");

    MarketData md;

    for (auto deltaType : {DeltaVolQuote::PaSpot, DeltaVolQuote::Spot}) {
        FxCostSmileSectionStickyDelta ss(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount,
                                         md.domDiscount, deltaType, md.atmType,
                                         Actual365Fixed(), md.settlement, Date(), true);

        // 3 free cost ratios for 5 quotes, fitted by least squares
        checkSmileSection(ss, 27.0e-4);
        checkStrikeDeltaConsistency(ss, md, 1.0e-6);

        // the slope used by the dynamics is the slope of the smile
        const Time tau = ss.exerciseTime();
        for (Real m : {0.8, 0.9, 1.0, 1.1, 1.2}) {
            const Rate K = m * ss.forward();
            const Real h = 1.0e-5;
            const Real fd = (ss.volByStrike(K * std::exp(h)) - ss.volByStrike(K * std::exp(-h))) *
                            std::sqrt(tau) / (2.0 * h);
            BOOST_CHECK_MESSAGE(std::fabs(ss.slope(K) - fd) < 1.0e-6,
                                "slope " << ss.slope(K) << " differs from the smile's " << fd
                                         << " at K/F = " << m);
            const Real dK = 1.0e-5 * K;
            const Real fdK = (ss.volByStrike(K + dK) - ss.volByStrike(K - dK)) / (2.0 * dK);
            BOOST_CHECK_MESSAGE(std::fabs(ss.volDerivative(K) - fdK) < 1.0e-6,
                                "volDerivative " << ss.volDerivative(K) << " differs from "
                                                 << fdK << " at K/F = " << m);
        }
    }
}


BOOST_AUTO_TEST_CASE(testStickyDeltaConvexSmileSolver) {
    BOOST_TEST_MESSAGE("Testing the sticky-delta cost smile with convexity...");

    const std::vector<Real> costs = {1.0, -15.3166, -0.06278, 0.28742};
    const std::vector<Real> xs = {-0.08, -0.03, 0.0, 0.03, 0.08};

    // reference values from an independent implementation (Python, scipy's
    // fourth-order collocation solve_bvp with a 1e-11 tolerance), on the
    // singular point of the smile without convexity plus or minus 8 total vols
    const std::vector<Real> refW[2] = {
        {0.043161310158, 0.036456627963, 0.036126201696, 0.038485343052, 0.046943186387},
        {0.043273312194, 0.036599102919, 0.036274554535, 0.038628792457, 0.047067508358}};
    const std::vector<Real> refBeta[2] = {
        {0.198903095, 0.058920443, -0.035843821, -0.118586654, -0.208535935},
        {0.198249008, 0.058525044, -0.035843819, -0.118285792, -0.208165570}};
    const std::vector<Real> refKappa[2] = {
        {1.09523452, 1.64062894, 1.49098144, 1.16595743, 0.45951015},
        {1.09805744, 1.63395387, 1.48480998, 1.16252430, 0.46156852}};

    for (bool pa : {false, true}) {
        const FxStickyDeltaSmileSolver flat(costs, pa, -0.1, 0.1);
        const Real lo = flat.singularLogMoneyness() - 8.0 * flat.singularTotalVol();
        const Real hi = flat.singularLogMoneyness() + 8.0 * flat.singularTotalVol();
        const FxStickyDeltaConvexSmileSolver smile(costs, pa, lo, hi);
        const Size k = pa ? 1 : 0;

        Real worstW = 0.0, worstBeta = 0.0, worstKappa = 0.0;
        for (Size i = 0; i < xs.size(); ++i) {
            worstW = std::max(worstW, std::fabs(smile.totalVol(xs[i]) - refW[k][i]));
            worstBeta = std::max(worstBeta, std::fabs(smile.slope(xs[i]) - refBeta[k][i]));
            worstKappa = std::max(worstKappa, std::fabs(smile.convexity(xs[i]) - refKappa[k][i]));
        }
        // the reference is given to 12, 9 and 8 decimals
        BOOST_CHECK_SMALL(worstW, 1.0e-10);
        BOOST_CHECK_SMALL(worstBeta, 1.0e-7);
        BOOST_CHECK_SMALL(worstKappa, 1.0e-6);

        // the slope is that of the smile, and the convexity is (beta + w'')/2
        Real worstSlope = 0.0, worstConvexity = 0.0;
        for (Real x = -0.2; x < 0.2; x += 0.01) {
            const Real h = 1.0e-4;
            const Real wm = smile.totalVol(x - h), w0 = smile.totalVol(x),
                       wp = smile.totalVol(x + h);
            worstSlope = std::max(worstSlope, std::fabs(smile.slope(x) + (wp - wm) / (2.0 * h)));
            const Real wpp = (wp - 2.0 * w0 + wm) / (h * h);
            worstConvexity = std::max(
                worstConvexity, std::fabs(smile.convexity(x) - 0.5 * (smile.slope(x) + wpp)));
        }
        // limited by the finite differences, notably the second one
        BOOST_CHECK_SMALL(worstSlope, 1.0e-6);
        BOOST_CHECK_SMALL(worstConvexity, 2.0e-4);
    }

    // with premium adjustment the smile is the unadjusted one with Ctheta
    // replaced by Ctheta + Ceta, and only the cost ratios matter
    const FxStickyDeltaConvexSmileSolver unadjusted(costs, false, -0.25, 0.25);
    const FxStickyDeltaConvexSmileSolver adjusted(
        {costs[0], costs[1] + costs[2], costs[2], costs[3]}, true, -0.25, 0.25);
    const FxStickyDeltaConvexSmileSolver scaled(
        {3.0, 3.0 * costs[1], 3.0 * costs[2], 3.0 * costs[3]}, false, -0.25, 0.25);
    for (Real x : xs) {
        BOOST_CHECK_SMALL(adjusted.totalVol(x) - unadjusted.totalVol(x), 1.0e-12);
        BOOST_CHECK_SMALL(scaled.totalVol(x) - unadjusted.totalVol(x), 1.0e-12);
    }
    BOOST_CHECK(unadjusted.hasCosts(scaled.costs()));

    // the end conditions only matter near the ends: their effect decays
    // over a few total vols (about 0.03 here) away from them
    const FxStickyDeltaConvexSmileSolver wider(costs, false, -0.4, 0.4);
    for (Real x : xs)
        BOOST_CHECK_SMALL(wider.totalVol(x) - unadjusted.totalVol(x), 1.0e-7);

    // a tolerance below rounding still converges
    const FxStickyDeltaConvexSmileSolver strict(costs, false, -0.25, 0.25, 401, 1.0e-17);
    for (Real x : xs)
        BOOST_CHECK_SMALL(strict.totalVol(x) - unadjusted.totalVol(x), 1.0e-12);

    // costs without a positive total vol at d- = 0 give no smile
    BOOST_CHECK_THROW(FxStickyDeltaConvexSmileSolver({1.0, 0.5, -0.06, 0.29}, false, -0.1, 0.1),
                      Error);
}

BOOST_AUTO_TEST_CASE(testCostSmileSectionStickyDeltaConvex) {
    BOOST_TEST_MESSAGE(
        "Testing FX cost smile section (sticky delta with convexity) calibration...");

    MarketData md;

    for (auto deltaType : {DeltaVolQuote::PaSpot, DeltaVolQuote::Spot}) {
        FxCostSmileSectionStickyDeltaConvex ss(md.expiryDate, md.spot, md.rrBfQuotes(),
                                               md.forDiscount, md.domDiscount, deltaType,
                                               md.atmType, Actual365Fixed(), md.settlement,
                                               Date(), true);
        FxCostSmileSectionStickyDelta flat(md.expiryDate, md.spot, md.rrBfQuotes(),
                                           md.forDiscount, md.domDiscount, deltaType, md.atmType,
                                           Actual365Fixed(), md.settlement, Date(), true);

        // 3 free cost ratios for 5 quotes, fitted by least squares
        checkSmileSection(ss, 25.0e-4);
        checkStrikeDeltaConsistency(ss, md, 1.0e-6);

        const Time tau = ss.exerciseTime();
        for (Real m : {0.8, 0.9, 1.0, 1.1, 1.2}) {
            const Rate K = m * ss.forward();
            const Real h = 1.0e-5;
            const Real fd = (ss.volByStrike(K * std::exp(h)) - ss.volByStrike(K * std::exp(-h))) *
                            std::sqrt(tau) / (2.0 * h);
            BOOST_CHECK_SMALL(ss.slope(K) - fd, 1.0e-6);
            const Real dK = 1.0e-5 * K;
            const Real fdK = (ss.volByStrike(K + dK) - ss.volByStrike(K - dK)) / (2.0 * dK);
            BOOST_CHECK_SMALL(ss.volDerivative(K) - fdK, 1.0e-6);
            // the convex dynamics have a convexity, the flat ones none
            BOOST_CHECK_GT(std::fabs(ss.convexity(K)), 0.0);
            BOOST_CHECK_EQUAL(flat.convexity(K), 0.0);
        }
    }
}


BOOST_AUTO_TEST_CASE(testAtmVolForwardSensitivity) {
    BOOST_TEST_MESSAGE(
        "Testing the ATM vol move expected from a forward move in FX smile models...");

    MarketData md;

    // cost models: the regression of the vol shock on the forward move
    FxCostSmileSectionFlatDynamics flat(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount,
                                        md.domDiscount, md.deltaType, md.atmType,
                                        Actual365Fixed(), md.settlement, Date(), true);
    FxCostSmileSectionScaledDynamics scaled(md.expiryDate, md.spot, md.rrBfQuotes(),
                                            md.forDiscount, md.domDiscount, md.deltaType,
                                            md.atmType, Actual365Fixed(), md.settlement, Date(),
                                            true);
    const Real sqrtTau = std::sqrt(flat.exerciseTime());
    BOOST_CHECK_CLOSE(flat.atmVolForwardSensitivity(),
                      flat.costs()[2] / flat.costs()[0] / sqrtTau, 1.0e-12);
    BOOST_CHECK_CLOSE(scaled.atmVolForwardSensitivity(),
                      scaled.costs()[2] / scaled.costs()[0] * scaled.atmVol(), 1.0e-12);

    // SABR: alpha moves by rho nu per unit dF/F, and the ATM vol by that
    // times its sensitivity to alpha; at the forward, for beta = 1 (Hagan),
    // sigma = alpha [1 + (rho nu alpha / 4 + (2 - 3 rho^2) nu^2 / 24) T]
    FxSabrSmileSection sabr(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount,
                            md.domDiscount, md.deltaType, md.atmType, Actual365Fixed(),
                            md.settlement);
    const Real alpha = sabr.alpha(), nu = sabr.nu(), rho = sabr.rho(), T = sabr.exerciseTime();
    const Real dVolDAlpha =
        1.0 + (rho * nu * alpha / 2.0 + (2.0 - 3.0 * rho * rho) * nu * nu / 24.0) * T;
    // the ATM strike is near, not at, the forward
    BOOST_CHECK_CLOSE(sabr.atmVolForwardSensitivity(), rho * nu * dVolDAlpha, 0.5);

    // models without dynamics expect no move
    FxPolynomialSmileSection polynomial(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount,
                                        md.domDiscount, md.deltaType, md.atmType,
                                        Actual365Fixed(), md.settlement);
    BOOST_CHECK_EQUAL(polynomial.atmVolForwardSensitivity(), 0.0);
}


// ---------------------------------------------------------------------------
//  7. DeltaVolQuote constructor path (all models)
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(testDeltaVolQuoteConstructorPath) {
    BOOST_TEST_MESSAGE("Testing FX smile sections via DeltaVolQuote constructor...");

    MarketData md;

    // Build DeltaVolQuote handles: ATM + 4 wing quotes (25D and 10D)
    std::vector<Handle<DeltaVolQuote>> quotes;
    quotes.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
        md.v_atm, DeltaVolQuote::Fwd, 1.0, DeltaVolQuote::AtmFwd)));

    // 25D put / 25D call
    quotes.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
        -0.25, makeQuoteHandle(md.v_25p), 1.0, DeltaVolQuote::PaSpot)));
    quotes.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
        0.25, makeQuoteHandle(md.v_25c), 1.0, DeltaVolQuote::PaSpot)));

    // 10D put / 10D call
    quotes.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
        -0.10, makeQuoteHandle(md.v_10p), 1.0, DeltaVolQuote::PaSpot)));
    quotes.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
        0.10, makeQuoteHandle(md.v_10c), 1.0, DeltaVolQuote::PaSpot)));

    // These are the market quotes, in the conventions the section reads
    // them in, so every model must give the smile it gets from the RR/BF
    // quotes.
    auto deltaVolQuotes = ext::make_shared<FxDeltaVolQuotes>(quotes);
    auto compare = [&](const std::string& name, const SectionFactory& make) {
        auto fromRrBf = make(md.rrBfQuotes());
        auto fromDeltaVols = make(deltaVolQuotes);
        // the targets come in a different order, and LM stops on a relative
        // change of the objective, so the fits agree to well below 0.1bp
        // rather than to machine precision
        BOOST_CHECK_SMALL(fromRrBf->calibrationError() - fromDeltaVols->calibrationError(), 1.0e-7);
        checkSameSmile(name, *fromRrBf, *fromDeltaVols, 1.0e-5);
    };
    compare("Polynomial", sectionFactory<FxPolynomialSmileSection>(md));
    compare("SABR", sectionFactory<FxSabrSmileSection>(md));
    compare("SVI", sectionFactory<FxSviSmileSection>(md));
    compare("Quadratic", sectionFactory<FxQuadraticSmileSection>(md));
    compare("CostFlatDynamics", sectionFactory<FxCostSmileSectionFlatDynamics>(md));
    compare("CostScaledDynamics", sectionFactory<FxCostSmileSectionScaledDynamics>(md));
}


BOOST_AUTO_TEST_CASE(testCalibrationRoundTrip) {
    BOOST_TEST_MESSAGE("Testing that FX smile sections refit quotes generated by their own smile...");

    MarketData md;

    // Each model is calibrated to the market, quotes are read off its
    // smile, and a fresh section must fit them exactly and give back the
    // same smile.
    auto roundTrip = [&](const std::string& name, const SectionFactory& make,
                         Real tolerance = 1.0e-8) {
        auto original = make(md.rrBfQuotes());
        auto refitted = make(quotesFromSmile(*original));
        BOOST_CHECK_MESSAGE(refitted->calibrationError() < tolerance,
                            name << ": quotes from its own smile not refitted: error "
                                 << refitted->calibrationError());
        checkSameSmile(name, *original, *refitted, 1.0e-6);
    };
    roundTrip("Polynomial", sectionFactory<FxPolynomialSmileSection>(md));
    roundTrip("SABR", sectionFactory<FxSabrSmileSection>(md));
    roundTrip("SVI", sectionFactory<FxSviSmileSection>(md));
    roundTrip("Quadratic", sectionFactory<FxQuadraticSmileSection>(md));
    roundTrip("CostFlatDynamics", sectionFactory<FxCostSmileSectionFlatDynamics>(md));
    roundTrip("CostScaledDynamics", sectionFactory<FxCostSmileSectionScaledDynamics>(md));
    roundTrip("CostStickyDelta", sectionFactory<FxCostSmileSectionStickyDelta>(md));
    // the least-squares fit of the convex smile stops at about 1e-8 in vol
    roundTrip("CostStickyDeltaConvex", sectionFactory<FxCostSmileSectionStickyDeltaConvex>(md),
              3.0e-8);
}


// ---------------------------------------------------------------------------
//  8. Market data reactivity (observer pattern)
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(testAtmLevelIsTheForward) {
    BOOST_TEST_MESSAGE("Testing that an FX smile section's ATM level is the forward...");

    MarketData md;
    // under delta-neutral ATM the ATM strike is not the forward
    FxQuadraticSmileSection ss(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount,
                               md.domDiscount, md.deltaType, DeltaVolQuote::AtmDeltaNeutral,
                               Actual365Fixed(), md.settlement);
    const Real F = ss.forward();
    BOOST_CHECK_EQUAL(ss.atmLevel(), F);
    BOOST_CHECK(std::fabs(ss.atmStrike() - F) > 1.0e-4);
    BOOST_CHECK_SMALL(ss.volByStrike(ss.atmStrike()) - ss.atmVol(), 1.0e-12);

    // so SmileSection's pricing uses the right forward
    const Real sqrtT = std::sqrt(ss.exerciseTime());
    for (Real m : {0.9, 1.0, 1.1}) {
        const Rate K = m * F;
        const Option::Type type = K >= F ? Option::Call : Option::Put;
        const Real expected = BlackCalculator(type, K, F, ss.volByStrike(K) * sqrtT).value();
        BOOST_CHECK_CLOSE(ss.optionPrice(K, type), expected, 1.0e-10);
    }
}

namespace {

    template <class Section>
    class WithPeakCallDeltaStrike : public Section {
      public:
        using Section::Section;
        Rate peakStrike() const {
            this->calculate();
            return this->peakCallDeltaStrike();
        }
    };

    template <class Section>
    ext::shared_ptr<FxSmileSection> paSection(const MarketData& md,
                                              const ext::shared_ptr<SimpleQuote>& spot,
                                              const ext::shared_ptr<FxSmileQuotes>& quotes,
                                              DeltaVolQuote::DeltaType dt) {
        return ext::make_shared<Section>(md.expiryDate, Handle<Quote>(spot), quotes,
                                         md.forDiscount, md.domDiscount, dt,
                                         DeltaVolQuote::AtmDeltaNeutral, Actual365Fixed(),
                                         md.settlement);
    }

    Real callDelta(const FxSmileSection& ss, Rate strike) {
        return BlackDeltaCalculator(Option::Call, ss.deltaType(), ss.spot()->value(),
                                    ss.domesticDiscountFactor(), ss.foreignDiscountFactor(),
                                    ss.volByStrike(strike) * std::sqrt(ss.exerciseTime()))
            .deltaFromStrike(strike);
    }

}

BOOST_AUTO_TEST_CASE(testPremiumAdjustedMarketStrangles) {
    BOOST_TEST_MESSAGE("Testing FX smiles fitted to broker strangles with premium-adjusted deltas...");

    MarketData md;
    const Real delta = 0.25;
    auto spot = ext::make_shared<SimpleQuote>(md.spot->value());

    // ATM, risk reversal and broker strangle at one delta: three targets
    // for three parameters, so the fit is exact.  The risk reversal needs
    // call deltas of the trial smile, which for premium-adjusted deltas
    // used to depend on the previous calibration and failed on the first.
    for (auto dt : {DeltaVolQuote::PaSpot, DeltaVolQuote::PaFwd}) {
        auto quotes = ext::make_shared<FxRrBfQuotes>(
            md.v_atm, std::vector<Handle<Quote>>{md.v_25rr}, std::vector<Handle<Quote>>{md.v_25bf},
            std::vector<Real>{delta}, FxRrBfQuotes::MarketStrangle);
        const std::vector<std::pair<std::string, ext::shared_ptr<FxSmileSection>>> sections = {
            {"Polynomial", paSection<FxPolynomialSmileSection>(md, spot, quotes, dt)},
            {"Quadratic", paSection<FxQuadraticSmileSection>(md, spot, quotes, dt)},
            {"SABR", paSection<FxSabrSmileSection>(md, spot, quotes, dt)}};
        for (const auto& [name, ss] : sections) {
            BOOST_CHECK_MESSAGE(ss->calibrationError() < 1.0e-8,
                                name << " (" << dt << "): calibration error " << ss->calibrationError());
            const Real rr = ss->volByDelta(delta, Option::Call) - ss->volByDelta(-delta, Option::Put);
            BOOST_CHECK_SMALL(rr - md.v_25rr->value(), 1.0e-8);
            BOOST_CHECK_SMALL(ss->atmVol() - md.v_atm->value(), 1.0e-8);

            // the same quotes give the same smile after recalibrating
            const Rate K = 1.05 * ss->forward();
            const Volatility v = ss->volByStrike(K);
            spot->setValue(1.80);
            ss->volByStrike(K);
            spot->setValue(md.spot->value());
            BOOST_CHECK_CLOSE(ss->volByStrike(K), v, 1.0e-10);
        }
    }
}

BOOST_AUTO_TEST_CASE(testMaxCallDelta) {
    BOOST_TEST_MESSAGE("Testing the largest call delta of FX smile sections...");

    MarketData md;

    for (auto dt : {DeltaVolQuote::PaSpot, DeltaVolQuote::PaFwd}) {
        WithPeakCallDeltaStrike<FxQuadraticSmileSection> ss(
            md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount, md.domDiscount, dt,
            md.atmType, Actual365Fixed(), md.settlement);
        const Rate kPeak = ss.peakStrike();
        const Real maxDelta = ss.maxCallDelta();
        BOOST_CHECK_SMALL(callDelta(ss, kPeak) - maxDelta, 1.0e-14);
        // it is the peak of the smile's call delta...
        for (Real m : {0.8, 0.95, 0.999, 1.001, 1.05, 1.25})
            BOOST_CHECK_MESSAGE(callDelta(ss, m * kPeak) < maxDelta,
                                dt << ": call delta at " << m << " x peak strike exceeds the peak");
        // ...so every lower call delta is attained above it, and none higher
        const Real d = maxDelta - 1.0e-4;
        BOOST_CHECK(ss.strikeByDelta(d, Option::Call) > kPeak);
        BOOST_CHECK_SMALL(callDelta(ss, ss.strikeByDelta(d, Option::Call)) - d, 1.0e-10);
        BOOST_CHECK_THROW(ss.volByDelta(maxDelta + 1.0e-4, Option::Call), Error);
        // and it does not limit the strikes the smile is defined for
        BOOST_CHECK(ss.minStrike() < 1.0e-100);
    }

    // unadjusted deltas: the limit at zero strike
    FxQuadraticSmileSection spotDelta(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount,
                                      md.domDiscount, DeltaVolQuote::Spot, md.atmType,
                                      Actual365Fixed(), md.settlement);
    BOOST_CHECK_EQUAL(spotDelta.maxCallDelta(), spotDelta.foreignDiscountFactor());
    FxQuadraticSmileSection fwdDelta(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount,
                                     md.domDiscount, DeltaVolQuote::Fwd, md.atmType,
                                     Actual365Fixed(), md.settlement);
    BOOST_CHECK_EQUAL(fwdDelta.maxCallDelta(), 1.0);

    // a total vol above about 1.25 puts the peak above the forward (here
    // about 2; a 25D premium-adjusted call no longer exists, so 10D quotes)
    const Date longExpiry = md.todaysDate + 11 * Years;
    WithPeakCallDeltaStrike<FxQuadraticSmileSection> highVol(
        longExpiry, md.spot,
        ext::make_shared<FxRrBfQuotes>(makeQuoteHandle(0.60), std::vector<Handle<Quote>>{makeQuoteHandle(0.02)},
                                       std::vector<Handle<Quote>>{makeQuoteHandle(0.01)},
                                       std::vector<Real>{0.10}, FxRrBfQuotes::SmileStrangle),
        md.forDiscount, md.domDiscount, DeltaVolQuote::PaFwd, md.atmType, Actual365Fixed(),
        md.settlement);
    const Rate kPeak = highVol.peakStrike();
    BOOST_CHECK(kPeak > highVol.forward());
    for (Real m : {0.999, 1.001})
        BOOST_CHECK(callDelta(highVol, m * kPeak) < highVol.maxCallDelta());
}

namespace {

    template <class Section>
    SectionFactory conventionFactory(const MarketData& md,
                                     DeltaVolQuote::DeltaType dt,
                                     DeltaVolQuote::AtmType at) {
        return [&md, dt, at](const ext::shared_ptr<FxSmileQuotes>& quotes) {
            return ext::make_shared<Section>(md.expiryDate, md.spot, quotes, md.forDiscount,
                                             md.domDiscount, dt, at, Actual365Fixed(),
                                             md.settlement);
        };
    }

    // strike <-> delta <-> vol round trips on a calibrated smile
    void checkDeltaRoundTrips(const std::string& name, const FxSmileSection& ss) {
        for (Real d : {0.10, 0.25}) {
            for (auto [type, delta] : {std::make_pair(Option::Call, d), std::make_pair(Option::Put, -d)}) {
                const Rate K = ss.strikeByDelta(delta, type);
                BOOST_CHECK_MESSAGE(std::fabs(ss.deltaByStrike(K, type) - delta) < 1.0e-8,
                                    name << ": delta " << delta << " -> strike " << K
                                         << " -> delta " << ss.deltaByStrike(K, type));
                BOOST_CHECK_MESSAGE(std::fabs(ss.volByDelta(delta, type) - ss.volByStrike(K)) < 1.0e-8,
                                    name << ": vol by delta " << delta << " and by its strike differ");
            }
        }
    }

}

BOOST_AUTO_TEST_CASE(testRoundTripsInAllConventions) {
    BOOST_TEST_MESSAGE("Testing FX smile round trips in every delta and ATM convention...");

    MarketData md;

    for (auto dt : {DeltaVolQuote::Spot, DeltaVolQuote::Fwd, DeltaVolQuote::PaSpot,
                    DeltaVolQuote::PaFwd}) {
        for (auto at : {DeltaVolQuote::AtmFwd, DeltaVolQuote::AtmDeltaNeutral}) {
            const std::vector<std::pair<std::string, SectionFactory>> models = {
                {"Polynomial", conventionFactory<FxPolynomialSmileSection>(md, dt, at)},
                {"SABR", conventionFactory<FxSabrSmileSection>(md, dt, at)},
                {"SVI", conventionFactory<FxSviSmileSection>(md, dt, at)},
                {"Quadratic", conventionFactory<FxQuadraticSmileSection>(md, dt, at)},
                {"CostFlatDynamics", conventionFactory<FxCostSmileSectionFlatDynamics>(md, dt, at)},
                {"CostScaledDynamics", conventionFactory<FxCostSmileSectionScaledDynamics>(md, dt, at)}};
            for (const auto& [model, make] : models) {
                std::ostringstream name;
                name << model << " (" << dt << ", " << at << ")";
                try {
                    auto ss = make(md.rrBfQuotes());
                    // the ATM vol lies on the smile at the ATM strike...
                    BOOST_CHECK_MESSAGE(std::fabs(ss->volByStrike(ss->atmStrike()) - ss->atmVol()) < 1.0e-10,
                                        name.str() << ": ATM vol not on the smile");
                    // ...strikes, deltas and vols are consistent...
                    checkDeltaRoundTrips(name.str(), *ss);
                    // ...and quotes read off the smile are refitted exactly
                    auto refitted = make(quotesFromSmile(*ss));
                    BOOST_CHECK_MESSAGE(refitted->calibrationError() < 1.0e-8,
                                        name.str() << ": quotes from its own smile not refitted: error "
                                                   << refitted->calibrationError());
                    checkSameSmile(name.str(), *ss, *refitted, 1.0e-6);
                } catch (std::exception& e) {
                    BOOST_ERROR(name.str() << ": " << e.what());
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testDeltaConverter) {
    BOOST_TEST_MESSAGE("Testing strike and delta conversions in FX delta conventions...");

    const Real spot = 1.7554, ddom = 0.97, dfor = 0.95;
    for (auto dt : {DeltaVolQuote::Spot, DeltaVolQuote::Fwd, DeltaVolQuote::PaSpot,
                    DeltaVolQuote::PaFwd}) {
        const FxDeltaConverter conv(dt, spot, ddom, dfor);
        const Real F = conv.forward();
        BOOST_CHECK_CLOSE(F, spot * dfor / ddom, 1.0e-12);

        for (Real w : {0.05, 0.15, 0.6, 2.0}) {
            for (Real m : {0.3, 0.7, 0.9, 1.0, 1.1, 1.5, 3.0}) {
                const Rate K = m * F;
                const Real call = conv.delta(Option::Call, K, w);
                const Real put = conv.delta(Option::Put, K, w);
                // parity, and the put delta within its range
                BOOST_CHECK_SMALL(call - put - conv.callMinusPutDelta(K), 1.0e-14);
                const Real minPutDelta = conv.putDeltaLimit(K);
                BOOST_CHECK(put >= minPutDelta && put <= 0.0);
                // the same deltas as BlackDeltaCalculator
                BOOST_CHECK_SMALL(call - BlackDeltaCalculator(Option::Call, dt, spot, ddom, dfor, w)
                                             .deltaFromStrike(K), 1.0e-15);

                // strike from delta gives the strike back; for premium-
                // adjusted calls only above the peak, where they are quoted.
                // Unadjusted strikes use the closed form, whose inverse
                // normal is accurate to about 1e-9, times the stdDev; deltas
                // within 1e-6 of their limit no longer determine the strike.
                std::ostringstream where;
                where << dt << ", stdDev " << w << ", K/F " << m;
                const Real tolerance = conv.premiumAdjusted() ? 1.0e-10 : 1.0e-8;
                const bool saturated =
                    !conv.premiumAdjusted() &&
                    std::min(std::fabs(call), std::fabs(put)) < conv.callMinusPutDelta(K) * 1.0e-6;
                if (put < -1.0e-12 && !saturated) {
                    const Real error = conv.strike(Option::Put, put, w) / K - 1.0;
                    BOOST_CHECK_MESSAGE(std::fabs(error) < tolerance,
                                        "put strike round trip off by " << error << " (" << where.str() << ")");
                }
                const bool quotedCall = !conv.premiumAdjusted() || K > conv.peakCallStrike(w);
                if (quotedCall && call > 1.0e-12 && !saturated) {
                    const Real error = conv.strike(Option::Call, call, w) / K - 1.0;
                    BOOST_CHECK_MESSAGE(std::fabs(error) < tolerance,
                                        "call strike round trip off by " << error << " (" << where.str() << ")");
                }
            }
            // ATM strikes as BlackDeltaCalculator gives them
            for (auto at : {DeltaVolQuote::AtmFwd, DeltaVolQuote::AtmSpot,
                            DeltaVolQuote::AtmDeltaNeutral})
                BOOST_CHECK_CLOSE(conv.atmStrike(at, w),
                                  BlackDeltaCalculator(Option::Call, dt, spot, ddom, dfor, w).atmStrike(at),
                                  1.0e-12);

            if (conv.premiumAdjusted()) {
                // the peak is the largest call delta, and higher ones fail clearly
                const Rate kPeak = conv.peakCallStrike(w);
                const Real maxDelta = conv.delta(Option::Call, kPeak, w);
                for (Real m : {0.99, 0.999, 1.001, 1.01})
                    BOOST_CHECK(conv.delta(Option::Call, m * kPeak, w) < maxDelta);
                BOOST_CHECK_THROW(conv.strike(Option::Call, maxDelta + 1.0e-6, w), Error);
            } else {
                BOOST_CHECK_EQUAL(conv.callDeltaLimit(), dt == DeltaVolQuote::Spot ? dfor : 1.0);
            }
        }

        // incoherent type and delta
        BOOST_CHECK_THROW(conv.strike(Option::Call, -0.25, 0.15), Error);
        BOOST_CHECK_THROW(conv.strike(Option::Put, 0.25, 0.15), Error);
    }
    // premium-adjusted puts deeper than -dfor (in the money), which
    // BlackDeltaCalculator::strikeFromDelta rejects
    const FxDeltaConverter pa(DeltaVolQuote::PaSpot, spot, ddom, dfor);
    const Rate itm = pa.strike(Option::Put, -1.2, 0.15);
    BOOST_CHECK_SMALL(pa.delta(Option::Put, itm, 0.15) + 1.2, 1.0e-10);
}

namespace {

    // A smile quadratic in log-moneyness that is only defined near the
    // money, as some models are in practice.
    class NearTheMoneySmile : public FxSmileSectionByStrike {
      public:
        using FxSmileSectionByStrike::FxSmileSectionByStrike;
      protected:
        Array initialParams() const override {
            Array guess(3, 0.0);
            guess[0] = referenceVol();
            return guess;
        }
      private:
        Volatility volByStrikeImpl(Real strike, Real fwd, Time, const std::vector<Real>& p) const override {
            const Real k = std::log(strike / fwd);
            if (std::fabs(k) > 0.5)
                return std::numeric_limits<Real>::quiet_NaN();
            return p[0] + p[1] * k + p[2] * k * k;
        }
    };

}

BOOST_AUTO_TEST_CASE(testStrikeByDeltaOnlyVisitsStrikesNearTheRoot) {
    BOOST_TEST_MESSAGE("Testing FX strike-by-delta inversion on a smile defined only near the money...");

    MarketData md;
    for (auto dt : {DeltaVolQuote::Spot, DeltaVolQuote::Fwd, DeltaVolQuote::PaSpot,
                    DeltaVolQuote::PaFwd}) {
        NearTheMoneySmile ss(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount,
                             md.domDiscount, dt, md.atmType, Actual365Fixed(), md.settlement);
        BOOST_REQUIRE(std::isnan(ss.volByStrike(0.5 * ss.forward())));
        // the quoted strikes lie well inside the domain, so inverting their
        // deltas must not depend on the smile far away
        for (Real d : {0.10, 0.25}) {
            for (auto [type, delta] :
                 {std::make_pair(Option::Call, d), std::make_pair(Option::Put, -d)}) {
                try {
                    const Rate K = ss.strikeByDelta(delta, type);
                    BOOST_CHECK_SMALL(ss.deltaByStrike(K, type) - delta, 1.0e-8);
                } catch (std::exception& e) {
                    BOOST_ERROR(dt << ", delta " << delta << ": " << e.what());
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testPremiumAdjustedCallsBeyondTheFlatVolPeak) {
    BOOST_TEST_MESSAGE("Testing premium-adjusted call deltas the smile reaches but a flat vol does not...");

    // With a put skew the call wing has a lower vol than the reference
    // (ATM) vol, and a lower vol lets premium-adjusted call deltas reach
    // higher, so the smile attains call deltas a flat reference vol does
    // not.  This only matters at a high total vol, where the largest
    // attainable delta comes down towards the quoted ones.
    MarketData md;
    const Date expiry = md.todaysDate + 5 * Years;
    auto quotes = ext::make_shared<FxRrBfQuotes>(
        makeQuoteHandle(0.60), std::vector<Handle<Quote>>{makeQuoteHandle(-0.10)},
        std::vector<Handle<Quote>>{makeQuoteHandle(0.01)}, std::vector<Real>{0.10},
        FxRrBfQuotes::SmileStrangle);
    FxPolynomialSmileSection ss(expiry, md.spot, quotes, md.forDiscount, md.domDiscount,
                                DeltaVolQuote::PaFwd, DeltaVolQuote::AtmFwd, Actual365Fixed(),
                                md.settlement);

    const FxDeltaConverter conv = ss.deltaConverter();
    const Real flatStdDev = 0.60 * std::sqrt(ss.exerciseTime());
    const Real flatMax = conv.delta(Option::Call, conv.peakCallStrike(flatStdDev), flatStdDev);
    const Real smileMax = ss.maxCallDelta();
    BOOST_REQUIRE_MESSAGE(smileMax > flatMax + 1.0e-3,
                          "test setup: smile max " << smileMax << ", flat max " << flatMax);

    const Real delta = 0.5 * (flatMax + smileMax);
    BOOST_CHECK_THROW(conv.strike(Option::Call, delta, flatStdDev), Error);
    try {
        const Rate K = ss.strikeByDelta(delta, Option::Call);
        BOOST_CHECK_SMALL(ss.deltaByStrike(K, Option::Call) - delta, 1.0e-8);
        BOOST_CHECK_SMALL(ss.volByDelta(delta, Option::Call) - ss.volByStrike(K), 1.0e-8);
    } catch (std::exception& e) {
        BOOST_ERROR("call delta " << delta << " between the flat and smile peaks: " << e.what());
    }
}

BOOST_AUTO_TEST_CASE(testQuarticRoots) {
    BOOST_TEST_MESSAGE("Testing the real roots of the cost models' quartic...");

    auto check = [](const std::vector<Real>& coefficients, std::vector<Real> expected) {
        std::vector<Real> roots = {42.0, 43.0};  // stale content must go
        QuarticPolynomial(coefficients).roots(roots);
        std::sort(roots.begin(), roots.end());
        std::sort(expected.begin(), expected.end());
        BOOST_REQUIRE_EQUAL(roots.size(), expected.size());
        for (Size i = 0; i < roots.size(); ++i)
            BOOST_CHECK_SMALL(roots[i] - expected[i], 1.0e-10);
    };
    // coefficients in increasing powers
    check({24.0, -50.0, 35.0, -10.0, 1.0}, {1.0, 2.0, 3.0, 4.0});  // (w-1)(w-2)(w-3)(w-4)
    check({2.0, -3.0, 3.0, -3.0, 1.0}, {1.0, 2.0});                // (w-1)(w-2)(w^2+1)
    check({2.0, 3.0, 3.0, 3.0, 1.0}, {-1.0, -2.0});                // (w+1)(w+2)(w^2+1)
    check({1.0, 0.0, 2.0, 0.0, 1.0}, {});                          // (w^2+1)^2
}

BOOST_AUTO_TEST_CASE(testSviAndSabrParametersStayValid) {
    BOOST_TEST_MESSAGE("Testing that calibrated FX SVI and SABR parameters are valid...");

    // Quotes that a valid SVI fits exactly, and quotes that would need
    // |rho| > 1 (steep risk reversals against flat wings, and a put skew):
    // the fit must stay valid and miss instead.
    MarketData md;
    struct Quotes { const char* name; Real atm, rr25, rr10, bf25, bf10; };
    for (const auto& data : {Quotes{"market", 0.14483, 0.0277, 0.048752, 0.007425, 0.02376},
                             Quotes{"steep", 0.14483, 0.0577, 0.101575, 0.007425, 0.016125},
                             Quotes{"put skew", 0.10, -0.03, -0.054, 0.004, 0.0128}}) {
        auto quotes = ext::make_shared<FxRrBfQuotes>(
            makeQuoteHandle(data.atm),
            std::vector<Handle<Quote>>{makeQuoteHandle(data.rr25), makeQuoteHandle(data.rr10)},
            std::vector<Handle<Quote>>{makeQuoteHandle(data.bf25), makeQuoteHandle(data.bf10)},
            std::vector<Real>{0.25, 0.10}, FxRrBfQuotes::SmileStrangle);
        for (auto dt : {DeltaVolQuote::Spot, DeltaVolQuote::PaSpot}) {
            for (int years : {1, 5}) {
                std::ostringstream name;
                name << data.name << " quotes, " << dt << ", " << years << "Y";
                const Date expiry = md.todaysDate + years * Years;
                try {
                    FxSviSmileSection svi(expiry, md.spot, quotes, md.forDiscount, md.domDiscount,
                                          dt, md.atmType, Actual365Fixed(), md.settlement);
                    BOOST_CHECK_MESSAGE(svi.b() > 0.0, name.str() << ": SVI b = " << svi.b());
                    BOOST_CHECK_MESSAGE(std::fabs(svi.rho()) <= 1.0, name.str() << ": SVI rho = " << svi.rho());
                    BOOST_CHECK_MESSAGE(svi.sigma() > 0.0, name.str() << ": SVI sigma = " << svi.sigma());
                    BOOST_CHECK_MESSAGE(svi.minimumVariance() > 0.0,
                                        name.str() << ": SVI minimum variance = " << svi.minimumVariance());
                    BOOST_CHECK_MESSAGE(svi.b() * (1.0 + std::fabs(svi.rho())) <= 2.0,
                                        name.str() << ": SVI wing slope too steep");

                    FxSabrSmileSection sabr(expiry, md.spot, quotes, md.forDiscount, md.domDiscount,
                                            dt, md.atmType, Actual365Fixed(), md.settlement);
                    BOOST_CHECK_MESSAGE(sabr.alpha() > 0.0, name.str() << ": SABR alpha = " << sabr.alpha());
                    BOOST_CHECK_MESSAGE(sabr.nu() > 0.0, name.str() << ": SABR nu = " << sabr.nu());
                    BOOST_CHECK_MESSAGE(std::fabs(sabr.rho()) < 1.0, name.str() << ": SABR rho = " << sabr.rho());
                } catch (std::exception& e) {
                    BOOST_ERROR(name.str() << ": " << e.what());
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testMarketDataReactivity) {
    BOOST_TEST_MESSAGE("Testing FX smile section reactivity to market data changes...");

    MarketData md;

    auto spotQuote = ext::make_shared<SimpleQuote>(md.spot->value());
    auto atmQuote  = ext::make_shared<SimpleQuote>(md.v_atm->value());
    auto rr25Quote = ext::make_shared<SimpleQuote>(md.v_25rr->value());
    auto rr10Quote = ext::make_shared<SimpleQuote>(md.v_10rr->value());
    auto bf25Quote = ext::make_shared<SimpleQuote>(md.v_25bf->value());
    auto bf10Quote = ext::make_shared<SimpleQuote>(md.v_10bf->value());

    FxPolynomialSmileSection ss(
        md.expiryDate,
        Handle<Quote>(spotQuote),
        ext::make_shared<FxRrBfQuotes>(
            Handle<Quote>(atmQuote),
            std::vector<Handle<Quote>>{Handle<Quote>(rr25Quote), Handle<Quote>(rr10Quote)},
            std::vector<Handle<Quote>>{Handle<Quote>(bf25Quote), Handle<Quote>(bf10Quote)},
            md.deltas, md.flyType),
        md.forDiscount, md.domDiscount,
        md.deltaType, md.atmType, Actual365Fixed(), md.settlement);

    // Each input must move the smile.  Under AtmFwd the ATM strike is the
    // forward whatever the vol, so vol changes are checked on vols.
    const Real F = ss.forward();
    const Rate kPut = 0.9 * F, kCall = 1.1 * F;

    auto checkMoves = [&](SimpleQuote& quote, Real bump, const std::string& name,
                          const std::function<Real()>& result) {
        const Real before = result(), base = quote.value();
        quote.setValue(base + bump);
        const Real after = result();
        quote.setValue(base);
        BOOST_CHECK_MESSAGE(std::fabs(after - before) > 1.0e-6,
                            "smile section did not react to a change in the " << name);
        BOOST_CHECK_CLOSE(result(), before, 1.0e-8);   // and comes back
    };

    checkMoves(*atmQuote, 0.01, "ATM vol", [&] { return ss.atmVol(); });
    checkMoves(*rr25Quote, 0.01, "25D risk reversal",
               [&] { return ss.volByStrike(kCall) - ss.volByStrike(kPut); });
    checkMoves(*rr10Quote, 0.01, "10D risk reversal",
               [&] { return ss.volByStrike(kCall) - ss.volByStrike(kPut); });
    checkMoves(*bf25Quote, 0.005, "25D butterfly",
               [&] { return ss.volByStrike(kCall) + ss.volByStrike(kPut); });
    checkMoves(*bf10Quote, 0.005, "10D butterfly",
               [&] { return ss.volByStrike(kCall) + ss.volByStrike(kPut); });
    checkMoves(*spotQuote, 0.01, "spot", [&] { return ss.forward(); });
    checkMoves(*spotQuote, 0.01, "spot (ATM strike)", [&] { return ss.atmStrike(); });

    // an ATM vol bump moves the calibrated ATM vol by about as much
    const Real atmBefore = ss.atmVol();
    atmQuote->setValue(atmQuote->value() + 0.01);
    BOOST_CHECK_SMALL(ss.atmVol() - atmBefore - 0.01, 1.0e-3);
}

namespace {

    // Section used by the settlement tests: the quadratic is the cheapest
    // model to calibrate, and the forward does not depend on the model.
    ext::shared_ptr<FxSmileSection>
    dateModeSection(const MarketData& md,
                    const Handle<YieldTermStructure>& forDiscount,
                    const Handle<YieldTermStructure>& domDiscount,
                    const Date& referenceDate = Date()) {
        return ext::make_shared<FxQuadraticSmileSection>(
            md.expiryDate, md.spot, md.rrBfQuotes(),
            forDiscount, domDiscount, md.deltaType, md.atmType,
            Actual365Fixed(), md.settlement, referenceDate);
    }

    Real expectedForward(Real spot,
                         const Handle<YieldTermStructure>& forDiscount,
                         const Handle<YieldTermStructure>& domDiscount,
                         const Date& spotDate,
                         const Date& deliveryDate) {
        return spot * (forDiscount->discount(deliveryDate) / forDiscount->discount(spotDate)) /
               (domDiscount->discount(deliveryDate) / domDiscount->discount(spotDate));
    }

}

BOOST_AUTO_TEST_CASE(testSettlementConventionDates) {
    BOOST_TEST_MESSAGE("Testing FX spot and delivery dates...");

    FxSettleConvention conv(WeekendsOnly(), 2);
    // Tuesday -> Thursday
    BOOST_CHECK_EQUAL(conv.valueDate(Date(2, January, 2024)), Date(4, January, 2024));
    // Thursday -> Monday: the lag skips the weekend
    BOOST_CHECK_EQUAL(conv.valueDate(Date(2, January, 2025)), Date(6, January, 2025));
    // T+1 pairs
    BOOST_CHECK_EQUAL(FxSettleConvention(WeekendsOnly(), 1).valueDate(Date(5, January, 2024)),
                      Date(8, January, 2024));

    BOOST_CHECK_THROW(FxSettleConvention(Calendar(), 2), Error);
}

BOOST_AUTO_TEST_CASE(testDateModeForwardUsesSpotToDelivery) {
    BOOST_TEST_MESSAGE("Testing that date-based FX smile sections discount from spot to delivery...");

    MarketData md;
    auto ss = dateModeSection(md, md.forDiscount, md.domDiscount);

    const Date spotDate(4, January, 2024), deliveryDate(6, January, 2025);
    BOOST_CHECK_EQUAL(ss->spotDate(), spotDate);
    BOOST_CHECK_EQUAL(ss->deliveryDate(), deliveryDate);

    const Real expected =
        expectedForward(md.spot->value(), md.forDiscount, md.domDiscount, spotDate, deliveryDate);
    BOOST_CHECK_CLOSE(ss->forward(), expected, 1.0e-12);

    // vol time still runs from the trade date to the expiry date
    BOOST_CHECK_CLOSE(ss->exerciseTime(),
                      Actual365Fixed().yearFraction(md.todaysDate, md.expiryDate), 1.0e-12);

    // the spot-to-delivery forward differs from the naive one to the expiry time
    const Real naive = md.spot->value() * md.forDiscount->discount(ss->exerciseTime()) /
                       md.domDiscount->discount(ss->exerciseTime());
    BOOST_CHECK(std::fabs(ss->forward() - naive) > 1.0e-8);
}

BOOST_AUTO_TEST_CASE(testDateModeIgnoresCurveReferenceDate) {
    BOOST_TEST_MESSAGE("Testing that date-based FX smile sections do not depend on the curves' "
                       "reference dates...");

    MarketData md;
    const Date spotDate = md.settlement.valueDate(md.todaysDate);
    // same continuously-compounded rates, but curves starting on the spot date
    Handle<YieldTermStructure> forFromSpot(
        ext::make_shared<FlatForward>(spotDate, 0.05, Actual365Fixed()));
    Handle<YieldTermStructure> domFromSpot(
        ext::make_shared<FlatForward>(spotDate, 0.03, Actual365Fixed()));

    auto fromToday = dateModeSection(md, md.forDiscount, md.domDiscount);
    auto fromSpot = dateModeSection(md, forFromSpot, domFromSpot);
    BOOST_CHECK_CLOSE(fromToday->forward(), fromSpot->forward(), 1.0e-12);
    BOOST_CHECK_CLOSE(fromToday->foreignDiscountFactor(), fromSpot->foreignDiscountFactor(), 1.0e-12);
    BOOST_CHECK_CLOSE(fromToday->domesticDiscountFactor(), fromSpot->domesticDiscountFactor(), 1.0e-12);

    // a curve starting after the spot date cannot give P(spot, delivery)
    Handle<YieldTermStructure> late(
        ext::make_shared<FlatForward>(spotDate + 1, 0.05, Actual365Fixed()));
    BOOST_CHECK_THROW(dateModeSection(md, late, md.domDiscount)->forward(), Error);
}

BOOST_AUTO_TEST_CASE(testDateModeFloatsWithEvaluationDate) {
    BOOST_TEST_MESSAGE("Testing that date-based FX smile sections float with the evaluation date...");

    MarketData md;
    auto floating = dateModeSection(md, md.forDiscount, md.domDiscount);
    auto fixed = dateModeSection(md, md.forDiscount, md.domDiscount, md.todaysDate);
    const Real fixedForward = fixed->forward();
    BOOST_CHECK_EQUAL(floating->spotDate(), Date(4, January, 2024));

    // move from Tuesday to Wednesday: the new spot date is Friday
    Settings::instance().evaluationDate() = Date(3, January, 2024);
    BOOST_CHECK_EQUAL(floating->spotDate(), Date(5, January, 2024));
    BOOST_CHECK_EQUAL(floating->deliveryDate(), Date(6, January, 2025));
    BOOST_CHECK_CLOSE(floating->forward(),
                      expectedForward(md.spot->value(), md.forDiscount, md.domDiscount,
                                      Date(5, January, 2024), Date(6, January, 2025)),
                      1.0e-12);

    // a section with an explicit reference date stays where it was
    BOOST_CHECK_EQUAL(fixed->spotDate(), Date(4, January, 2024));
    BOOST_CHECK_CLOSE(fixed->forward(), fixedForward, 1.0e-12);
}

BOOST_AUTO_TEST_CASE(testTimeModeForwardAndConsistencyChecks) {
    BOOST_TEST_MESSAGE("Testing time-based FX smile sections...");

    MarketData md;
    const Time tau = 1.0;
    auto section = [&](const Handle<YieldTermStructure>& forDiscount,
                       const Handle<YieldTermStructure>& domDiscount,
                       const DayCounter& dc) {
        return ext::make_shared<FxQuadraticSmileSection>(
            tau, md.spot, md.rrBfQuotes(), forDiscount,
            domDiscount, md.deltaType, md.atmType, dc);
    };

    auto ss = section(md.forDiscount, md.domDiscount, Actual365Fixed());
    BOOST_CHECK_CLOSE(ss->forward(),
                      md.spot->value() * md.forDiscount->discount(tau) /
                          md.domDiscount->discount(tau),
                      1.0e-12);
    BOOST_CHECK(!ss->settleConvention());
    BOOST_CHECK_THROW(ss->spotDate(), Error);

    // an empty day counter is fine in time mode
    BOOST_CHECK_NO_THROW(section(md.forDiscount, md.domDiscount, DayCounter())->forward());

    // the curves must share a time axis...
    Handle<YieldTermStructure> act360(
        ext::make_shared<FlatForward>(md.todaysDate, 0.03, Actual360()));
    BOOST_CHECK_THROW(section(md.forDiscount, act360, Actual365Fixed())->forward(), Error);
    Handle<YieldTermStructure> shifted(
        ext::make_shared<FlatForward>(md.todaysDate + 1, 0.03, Actual365Fixed()));
    BOOST_CHECK_THROW(section(md.forDiscount, shifted, Actual365Fixed())->forward(), Error);
    // ...but the section's own day counter plays no part in time mode
    BOOST_CHECK_NO_THROW(section(md.forDiscount, md.domDiscount, Actual360())->forward());
}

BOOST_AUTO_TEST_CASE(testSmileQuotesValidationAndNotification) {
    BOOST_TEST_MESSAGE("Testing FX smile quotes validation and change notification...");

    MarketData md;

    // risk reversals, butterflies and deltas must line up
    BOOST_CHECK_THROW(FxRrBfQuotes(md.v_atm, {md.v_25rr}, {md.v_25bf, md.v_10bf}, md.deltas,
                                   FxRrBfQuotes::SmileStrangle),
                      Error);
    BOOST_CHECK_THROW(FxRrBfQuotes(md.v_atm, {md.v_25rr, md.v_10rr}, {md.v_25bf}, md.deltas,
                                   FxRrBfQuotes::SmileStrangle),
                      Error);
    // deltas must be distinct and in (0, 0.5), and every quote given
    for (Real badDelta : {0.0, 0.5, -0.25, 0.6})
        BOOST_CHECK_THROW(FxRrBfQuotes(md.v_atm, {md.v_25rr}, {md.v_25bf}, {badDelta},
                                       FxRrBfQuotes::SmileStrangle),
                          Error);
    BOOST_CHECK_THROW(FxRrBfQuotes(md.v_atm, {md.v_25rr, md.v_10rr}, {md.v_25bf, md.v_10bf},
                                   {0.25, 0.25}, FxRrBfQuotes::SmileStrangle),
                      Error);
    BOOST_CHECK_THROW(FxRrBfQuotes(Handle<Quote>(), {md.v_25rr}, {md.v_25bf}, {0.25},
                                   FxRrBfQuotes::SmileStrangle),
                      Error);
    BOOST_CHECK_THROW(FxRrBfQuotes(md.v_atm, {Handle<Quote>()}, {md.v_25bf}, {0.25},
                                   FxRrBfQuotes::SmileStrangle),
                      Error);
    BOOST_CHECK_THROW(FxRrBfQuotes(md.v_atm, {md.v_25rr}, {Handle<Quote>()}, {0.25},
                                   FxRrBfQuotes::SmileStrangle),
                      Error);

    // delta-vol quotes: at least one, none empty, and each with a delta or
    // an ATM convention
    BOOST_CHECK_THROW(FxDeltaVolQuotes({}), Error);
    BOOST_CHECK_THROW(FxDeltaVolQuotes({Handle<DeltaVolQuote>()}), Error);
    BOOST_CHECK_THROW(FxDeltaVolQuotes({Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
                          0.0, md.v_atm, 1.0, md.deltaType))}),
                      Error);

    // the section is notified through the quotes object, which may be
    // shared by several sections
    auto rr25 = ext::make_shared<SimpleQuote>(md.v_25rr->value());
    auto quotes = ext::make_shared<FxRrBfQuotes>(
        md.v_atm, std::vector<Handle<Quote>>{Handle<Quote>(rr25), md.v_10rr},
        std::vector<Handle<Quote>>{md.v_25bf, md.v_10bf}, md.deltas, md.flyType);
    BOOST_CHECK_CLOSE(quotes->referenceVol(), md.v_atm->value(), 1.0e-12);

    FxQuadraticSmileSection first(md.expiryDate, md.spot, quotes, md.forDiscount, md.domDiscount,
                                  md.deltaType, md.atmType, Actual365Fixed(), md.settlement);
    FxQuadraticSmileSection second(1.0, md.spot, quotes, md.forDiscount, md.domDiscount,
                                   md.deltaType, md.atmType, Actual365Fixed());
    const Real K = 1.05 * first.forward();
    const Real v1 = first.volByStrike(K), v2 = second.volByStrike(K);

    rr25->setValue(rr25->value() + 0.01);
    BOOST_CHECK(std::fabs(first.volByStrike(K) - v1) > 1.0e-6);
    BOOST_CHECK(std::fabs(second.volByStrike(K) - v2) > 1.0e-6);

    BOOST_CHECK_THROW(FxQuadraticSmileSection(md.expiryDate, md.spot,
                                              ext::shared_ptr<FxSmileQuotes>(), md.forDiscount,
                                              md.domDiscount, md.deltaType, md.atmType,
                                              Actual365Fixed(), md.settlement),
                      Error);
}

BOOST_AUTO_TEST_CASE(testDeltaVolQuotesUseTheirOwnConventions) {
    BOOST_TEST_MESSAGE("Testing that FX delta-vol quotes are read in their own conventions...");

    MarketData md;
    // spot-delta quotes on a premium-adjusted section: three quotes for
    // the quadratic's three parameters, so the fit is exact
    const DeltaVolQuote::DeltaType quoteType = DeltaVolQuote::Spot;
    BOOST_REQUIRE(md.deltaType != quoteType);
    std::vector<Handle<DeltaVolQuote>> q = {
        Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(md.v_atm, quoteType, 1.0,
                                                              DeltaVolQuote::AtmDeltaNeutral)),
        Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(0.25, makeQuoteHandle(md.v_25c), 1.0, quoteType)),
        Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(-0.25, makeQuoteHandle(md.v_25p), 1.0, quoteType))};
    FxQuadraticSmileSection ss(md.expiryDate, md.spot, ext::make_shared<FxDeltaVolQuotes>(q),
                               md.forDiscount, md.domDiscount, md.deltaType, md.atmType,
                               Actual365Fixed(), md.settlement);

    const Real S = md.spot->value(), sqrtT = std::sqrt(ss.exerciseTime());
    const Real ddom = ss.domesticDiscountFactor(), dfor = ss.foreignDiscountFactor();
    auto strike = [&](Option::Type type, DeltaVolQuote::DeltaType dt, Real delta, Volatility v) {
        return BlackDeltaCalculator(type, dt, S, ddom, dfor, v * sqrtT).strikeFromDelta(delta);
    };
    const Real kAtm = BlackDeltaCalculator(Option::Call, quoteType, S, ddom, dfor,
                                           md.v_atm->value() * sqrtT)
                          .atmStrike(DeltaVolQuote::AtmDeltaNeutral);
    const Real kCall = strike(Option::Call, quoteType, 0.25, md.v_25c);
    const Real kPut = strike(Option::Put, quoteType, -0.25, md.v_25p);

    // the smile passes through each quote at the strike of the quote's own convention...
    BOOST_CHECK_SMALL(ss.volByStrike(kAtm) - md.v_atm->value(), 1.0e-8);
    BOOST_CHECK_SMALL(ss.volByStrike(kCall) - md.v_25c, 1.0e-8);
    BOOST_CHECK_SMALL(ss.volByStrike(kPut) - md.v_25p, 1.0e-8);
    // ...which is not the strike the section's convention would give
    BOOST_CHECK(std::fabs(strike(Option::Call, md.deltaType, 0.25, md.v_25c) - kCall) > 1.0e-3);
}

BOOST_AUTO_TEST_CASE(testCalibrationDoesNotDependOnHistory) {
    BOOST_TEST_MESSAGE("Testing that the same FX quotes always give the same smile...");

    MarketData md;
    auto spot = ext::make_shared<SimpleQuote>(md.spot->value());
    std::vector<Handle<DeltaVolQuote>> q = {
        Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(md.v_atm, md.deltaType, 1.0, md.atmType)),
        Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(0.25, makeQuoteHandle(md.v_25c), 1.0, md.deltaType)),
        Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(-0.25, makeQuoteHandle(md.v_25p), 1.0, md.deltaType)),
        Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(0.10, makeQuoteHandle(md.v_10c), 1.0, md.deltaType)),
        Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(-0.10, makeQuoteHandle(md.v_10p), 1.0, md.deltaType))};

    // the polynomial seeds and scales with the starting ATM vol, so it
    // used to drift when that seed was the previous calibration's ATM
    FxPolynomialSmileSection ss(md.expiryDate, Handle<Quote>(spot),
                                ext::make_shared<FxDeltaVolQuotes>(q), md.forDiscount,
                                md.domDiscount, md.deltaType, md.atmType, Actual365Fixed(),
                                md.settlement);
    const Real K1 = 1.60, K2 = 1.90;
    const Real first1 = ss.volByStrike(K1), first2 = ss.volByStrike(K2);

    for (int i = 0; i < 3; ++i) {
        spot->setValue(1.80);
        ss.volByStrike(K1); // recalibrate elsewhere...
        spot->setValue(md.spot->value());
        // ...and back to the original quotes
        BOOST_CHECK_CLOSE(ss.volByStrike(K1), first1, 1.0e-10);
        BOOST_CHECK_CLOSE(ss.volByStrike(K2), first2, 1.0e-10);
    }
}

namespace {

    // Quote objects that break the calibration contract.
    class QuotesThatNeverFit : public FxSmileQuotes {
      public:
        Volatility referenceVol() const override { return 0.1; }
      private:
        void calibrate(const FxSmileSection&) const override {}
    };

    class QuotesThatFitLater : public FxSmileQuotes {
      public:
        explicit QuotesThatFitLater(std::vector<Handle<DeltaVolQuote>> quotes)
        : quotes_(std::move(quotes)) {}
        Volatility referenceVol() const override { return 0.1; }
        void fitAgain() const { fit(*section_, targets(*section_)); }
      private:
        void calibrate(const FxSmileSection& section) const override {
            section_ = &section;
            fit(section, targets(section));
        }
        FxSmileTargets targets(const FxSmileSection& section) const {
            FxSmileTargets t;
            for (const auto& q : quotes_)
                t.push_back(ext::make_shared<FxDeltaVolTarget>(q));
            return t;
        }
        std::vector<Handle<DeltaVolQuote>> quotes_;
        mutable const FxSmileSection* section_ = nullptr;
    };

}

namespace {

    // A target no smile can meet.
    class UnreachableTarget : public FxSmileTarget {
      public:
        Real residual(const FxSmileSection&) const override {
            return std::numeric_limits<Real>::quiet_NaN();
        }
    };

    class QuotesWithUnreachableTarget : public FxSmileQuotes {
      public:
        Volatility referenceVol() const override { return 0.1; }
      private:
        void calibrate(const FxSmileSection& section) const override {
            fit(section, {ext::make_shared<UnreachableTarget>()});
        }
    };

}

BOOST_AUTO_TEST_CASE(testFailedCalibrationThrows) {
    BOOST_TEST_MESSAGE("Testing that a failed FX smile calibration raises an error...");

    MarketData md;
    FxQuadraticSmileSection ss(md.expiryDate, md.spot, ext::make_shared<QuotesWithUnreachableTarget>(),
                               md.forDiscount, md.domDiscount, md.deltaType, md.atmType,
                               Actual365Fixed(), md.settlement);
    BOOST_CHECK_THROW(ss.volByStrike(md.spot->value()), Error);
    BOOST_CHECK_THROW(ss.calibrationError(), Error);
}

BOOST_AUTO_TEST_CASE(testSmileQuotesMustFitDuringCalibration) {
    BOOST_TEST_MESSAGE("Testing that FX smile quotes can only fit a section while calibrating it...");

    MarketData md;

    FxQuadraticSmileSection unfitted(md.expiryDate, md.spot,
                                     ext::make_shared<QuotesThatNeverFit>(), md.forDiscount,
                                     md.domDiscount, md.deltaType, md.atmType, Actual365Fixed(),
                                     md.settlement);
    BOOST_CHECK_THROW(unfitted.volByStrike(md.spot->value()), Error);

    std::vector<Handle<DeltaVolQuote>> q = {
        Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(md.v_atm, md.deltaType, 1.0, md.atmType)),
        Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(0.25, makeQuoteHandle(md.v_25c), 1.0, md.deltaType)),
        Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(-0.25, makeQuoteHandle(md.v_25p), 1.0, md.deltaType))};
    auto later = ext::make_shared<QuotesThatFitLater>(q);
    FxQuadraticSmileSection ss(md.expiryDate, md.spot, later, md.forDiscount, md.domDiscount,
                               md.deltaType, md.atmType, Actual365Fixed(), md.settlement);
    const Real K = 1.05 * ss.forward(), v = ss.volByStrike(K);
    BOOST_CHECK_THROW(later->fitAgain(), Error);   // outside calibration: rejected...
    BOOST_CHECK_EQUAL(ss.volByStrike(K), v);       // ...and the smile is untouched
}

BOOST_AUTO_TEST_CASE(testMarketStrangleCalibration) {
    BOOST_TEST_MESSAGE("Testing joint FX smile calibration to broker (market) strangles...");

    MarketData md;
    const DeltaVolQuote::DeltaType dt = DeltaVolQuote::Spot;
    const DeltaVolQuote::AtmType at = DeltaVolQuote::AtmDeltaNeutral;
    const Real delta = 0.25;

    // One delta level: ATM, risk reversal and broker strangle are three
    // targets for the quadratic's three parameters, so the joint fit is
    // exact and each condition can be checked on its own.
    for (Real flyScale : {1.0, 0.0}) {   // quoted fly, and a zero (flat-wing) fly
        const Volatility atm = md.v_atm->value(), rr = md.v_25rr->value();
        const Volatility bf = flyScale * md.v_25bf->value();
        FxQuadraticSmileSection ss(md.expiryDate, md.spot,
                                   ext::make_shared<FxRrBfQuotes>(
                                     md.v_atm, std::vector<Handle<Quote>>{md.v_25rr},
                                     std::vector<Handle<Quote>>{makeQuoteHandle(bf)},
                                     std::vector<Real>{delta}, FxRrBfQuotes::MarketStrangle),
                                 md.forDiscount, md.domDiscount, dt, at, Actual365Fixed(),
                                 md.settlement);

        const Time tau = ss.exerciseTime();
        const Real F = ss.forward(), S = md.spot->value();
        const Real ddom = ss.domesticDiscountFactor(), dfor = ss.foreignDiscountFactor();

        // the broker strangle, struck and priced at atm + bf, is repriced by the smile
        const Real w = (atm + bf) * std::sqrt(tau);
        const Real Kc = BlackDeltaCalculator(Option::Call, dt, S, ddom, dfor, w).strikeFromDelta(delta);
        const Real Kp = BlackDeltaCalculator(Option::Put, dt, S, ddom, dfor, w).strikeFromDelta(-delta);
        const BlackCalculator mc(Option::Call, Kc, F, w), mp(Option::Put, Kp, F, w);
        const Real smile =
            BlackCalculator(Option::Call, Kc, F, ss.volByStrike(Kc) * std::sqrt(tau)).value() +
            BlackCalculator(Option::Put, Kp, F, ss.volByStrike(Kp) * std::sqrt(tau)).value();
        BOOST_CHECK_SMALL((smile - mc.value() - mp.value()) / (mc.vega(tau) + mp.vega(tau)), 1.0e-8);

        // the risk reversal holds at the smile's own deltas
        BOOST_CHECK_SMALL(ss.volByDelta(delta, Option::Call) - ss.volByDelta(-delta, Option::Put) - rr,
                          1.0e-8);

        // the ATM quote holds, and atmVol() lies on the smile
        BOOST_CHECK_SMALL(ss.atmVol() - atm, 1.0e-8);
        BOOST_CHECK_SMALL(ss.atmVol() - ss.volByStrike(ss.atmStrike()), 1.0e-12);
    }

    // Two delta levels: five targets for three parameters, fitted jointly
    // in the least-squares sense; the ATM convention still holds.
    FxQuadraticSmileSection ss(md.expiryDate, md.spot,
                               ext::make_shared<FxRrBfQuotes>(
                                 md.v_atm, std::vector<Handle<Quote>>{md.v_25rr, md.v_10rr},
                                 std::vector<Handle<Quote>>{md.v_25bf, md.v_10bf}, md.deltas,
                                 FxRrBfQuotes::MarketStrangle),
                             md.forDiscount, md.domDiscount, dt, at, Actual365Fixed(),
                             md.settlement);
    BOOST_CHECK_SMALL(ss.atmVol() - ss.volByStrike(ss.atmStrike()), 1.0e-12);

    // cost models have a closed form for points on the smile only
    FxCostSmileSectionFlatDynamics cost(
        md.expiryDate, md.spot,
        ext::make_shared<FxRrBfQuotes>(md.v_atm, std::vector<Handle<Quote>>{md.v_25rr},
                                       std::vector<Handle<Quote>>{md.v_25bf},
                                       std::vector<Real>{delta}, FxRrBfQuotes::MarketStrangle),
        md.forDiscount, md.domDiscount, dt, at, Actual365Fixed(), md.settlement);
    BOOST_CHECK_THROW(cost.volByStrike(md.spot->value()), Error);
}

BOOST_AUTO_TEST_CASE(testProbabilitySpaceFunctions) {
    BOOST_TEST_MESSAGE("Testing FX smile normed call prices and exercise probabilities...");

    MarketData md;
    const DeltaVolQuote::DeltaType dt = DeltaVolQuote::Spot;
    const DeltaVolQuote::AtmType at = DeltaVolQuote::AtmDeltaNeutral;

    // flat smile: closed forms are Black's
    std::vector<Handle<Quote>> zero = {makeQuoteHandle(0.0), makeQuoteHandle(0.0)};
    FxQuadraticSmileSection flat(md.expiryDate, md.spot,
                                 ext::make_shared<FxRrBfQuotes>(md.v_atm, zero, zero, md.deltas,
                                                              FxRrBfQuotes::SmileStrangle),
                               md.forDiscount, md.domDiscount, dt, at, Actual365Fixed(),
                               md.settlement);
    const Real w = md.v_atm->value() * std::sqrt(flat.exerciseTime());
    CumulativeNormalDistribution N;
    for (Real k = 0.7; k <= 1.4001; k += 0.05) {
        const Real d2 = (-std::log(k) - 0.5 * w * w) / w;
        BOOST_CHECK_SMALL(flat.exerciseProbability(k) - N(d2), 1.0e-10);
        BOOST_CHECK_SMALL(flat.normedCallPrice(k) -
                              BlackCalculator(Option::Call, k, 1.0, w).value(),
                          1.0e-12);
    }

    // a skewed smile: the inverse recovers the moneyness
    FxQuadraticSmileSection ss(md.expiryDate, md.spot, md.rrBfQuotes(), md.forDiscount,
                               md.domDiscount, dt, at, Actual365Fixed(), md.settlement);
    for (Real p : {0.02, 0.1, 0.25, 0.5, 0.75, 0.9, 0.98}) {
        const Real k = ss.moneynessFromProbability(p);
        BOOST_CHECK_SMALL(ss.exerciseProbability(k) - p, 1.0e-10);
    }

    BOOST_CHECK_THROW(ss.moneynessFromProbability(0.0), Error);
    BOOST_CHECK_THROW(ss.moneynessFromProbability(1.0), Error);
    BOOST_CHECK_THROW(ss.exerciseProbability(0.0), Error);
    BOOST_CHECK_THROW(ss.normedCallPrice(-1.0), Error);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
