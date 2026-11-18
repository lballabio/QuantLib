/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yang Ye

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/instruments/varianceswap.hpp>
#include <ql/instruments/volatilityswap.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/pricingengines/forward/lognormalvolatilityswapengine.hpp>
#include <ql/pricingengines/forward/replicatingvarianceswapengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <cmath>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(VolatilitySwapTests)

namespace {

    struct CommonVars {
        DayCounter dc = Actual365Fixed();
        Date today = Date(15, November, 2026);
        Date start = today + 1 * Months;
        Date maturity = today + 13 * Months;
        Rate rate = 0.03;
        ext::shared_ptr<SimpleQuote> variance = ext::make_shared<SimpleQuote>(0.04);
        ext::shared_ptr<SimpleQuote> logVol = ext::make_shared<SimpleQuote>(0.5);
        RelinkableHandle<YieldTermStructure> curve;

        CommonVars() {
            Settings::instance().evaluationDate() = today;
            curve.linkTo(flatRate(today, rate, dc));
        }

        ext::shared_ptr<PricingEngine> engine() const {
            return ext::make_shared<LognormalVolatilitySwapEngine>(
                curve, Handle<Quote>(variance), Handle<Quote>(logVol));
        }

        Real expectedFair() const {
            Real s = logVol->value();
            return std::sqrt(variance->value()) * std::exp(-s * s / 8.0);
        }
    };

}

BOOST_AUTO_TEST_CASE(testClosedForm) {
    BOOST_TEST_MESSAGE("Testing volatility swap closed-form fair strike and value...");

    CommonVars vars;
    const Real variances[] = {0.01, 0.04, 0.09, 0.25};
    const Real logVols[] = {0.0, 0.1, 0.5, 1.0, 1.5};
    const Real strikes[] = {0.0, 0.15, 0.2, 0.3};
    const Real notional = 250000.0;
    const Real tolerance = 1.0e-13;

    for (Real v : variances) {
        for (Real s : logVols) {
            vars.variance->setValue(v);
            vars.logVol->setValue(s);
            Real fair = std::sqrt(v) * std::exp(-s * s / 8.0);
            Real df = vars.curve->discount(vars.maturity);

            for (Real k : strikes) {
                VolatilitySwap lng(Position::Long, k, notional, vars.start, vars.maturity);
                VolatilitySwap sht(Position::Short, k, notional, vars.start, vars.maturity);
                lng.setPricingEngine(vars.engine());
                sht.setPricingEngine(vars.engine());

                Real expected = notional * df * (fair - k);
                if (std::fabs(lng.NPV() - expected) > tolerance * notional ||
                    std::fabs(sht.NPV() + expected) > tolerance * notional ||
                    std::fabs(lng.fairVolatility() - fair) > tolerance ||
                    std::fabs(sht.fairVolatility() - fair) > tolerance)
                    BOOST_ERROR("volatility swap mismatch:"
                                << "\n    variance strike:     " << v
                                << "\n    log-volatility:      " << s
                                << "\n    volatility strike:   " << k
                                << "\n    expected fair vol:   " << fair
                                << "\n    calculated fair vol: " << lng.fairVolatility()
                                << "\n    expected NPV:        " << expected
                                << "\n    calculated long NPV: " << lng.NPV()
                                << "\n    calculated short NPV:" << sht.NPV());
            }

            // the swap struck at the fair volatility is worth nothing
            VolatilitySwap atFair(Position::Long, fair, notional, vars.start, vars.maturity);
            atFair.setPricingEngine(vars.engine());
            if (std::fabs(atFair.NPV()) > tolerance * notional)
                BOOST_ERROR("swap struck at the fair volatility has non-zero value "
                            << atFair.NPV() << " for variance " << v << " and log-volatility "
                            << s);
        }
    }
}

BOOST_AUTO_TEST_CASE(testConvexityAdjustment) {
    BOOST_TEST_MESSAGE("Testing volatility swap convexity adjustment...");

    CommonVars vars;
    VolatilitySwap swap(Position::Long, 0.2, 1.0, vars.start, vars.maturity);
    swap.setPricingEngine(vars.engine());

    Real sqrtK = std::sqrt(vars.variance->value());

    // without variance of variance, the strike is the square root of the variance strike
    vars.logVol->setValue(0.0);
    if (std::fabs(swap.fairVolatility() - sqrtK) > 1.0e-15)
        BOOST_ERROR("fair volatility " << swap.fairVolatility()
                    << " differs from the square root of the variance strike " << sqrtK
                    << " at zero variance of variance");

    // the fair strike is strictly below it and decreases with the log-volatility
    Real previous = sqrtK;
    for (Real s : {0.05, 0.1, 0.25, 0.5, 1.0, 2.0}) {
        vars.logVol->setValue(s);
        Real fair = swap.fairVolatility();
        if (!(fair < previous) || !(fair > 0.0))
            BOOST_ERROR("fair volatility " << fair << " at log-volatility " << s
                        << " is not strictly between 0 and the previous value " << previous);
        previous = fair;
    }

    // for small log-volatility the strike agrees with the first-order approximation
    // sqrt(K) - Var(V)/(8 K^(3/2)), Var(V) = K^2 (exp(s^2) - 1), up to O(s^4)
    for (Real s : {0.05, 0.1, 0.2, 0.4}) {
        vars.logVol->setValue(s);
        Real approximation = sqrtK * (1.0 - (std::exp(s * s) - 1.0) / 8.0);
        Real difference = swap.fairVolatility() - approximation;
        Real bound = 0.1 * sqrtK * std::pow(s, 4);
        if (difference < 0.0 || difference > bound)
            BOOST_ERROR("fair volatility differs from the first-order approximation by "
                        << difference << " at log-volatility " << s
                        << ", expected a value in [0, " << bound << "]");
    }
}

BOOST_AUTO_TEST_CASE(testConsistencyWithVarianceSwap) {
    BOOST_TEST_MESSAGE("Testing volatility swap against a replicated variance swap...");

    CommonVars vars;
    Volatility sigma = 0.2;
    Date maturity = vars.today + 1 * Years;

    auto spot = ext::make_shared<SimpleQuote>(100.0);
    auto qTS = flatRate(vars.today, 0.0, vars.dc);
    auto rTS = flatRate(vars.today, vars.rate, vars.dc);
    auto volTS = ext::make_shared<BlackConstantVol>(vars.today, NullCalendar(), sigma, vars.dc);
    auto process = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot), Handle<YieldTermStructure>(qTS), Handle<YieldTermStructure>(rTS),
        Handle<BlackVolTermStructure>(volTS));

    VarianceSwap varianceSwap(Position::Long, sigma * sigma, 1.0, vars.today, maturity);
    std::vector<Real> putStrikes, callStrikes;
    for (Real k = 20.0; k <= 100.0; k += 5.0)
        putStrikes.push_back(k);
    for (Real k = 100.0; k <= 300.0; k += 5.0)
        callStrikes.push_back(k);
    varianceSwap.setPricingEngine(
        ext::make_shared<ReplicatingVarianceSwapEngine>(process, 5.0, callStrikes, putStrikes));
    Real fairVariance = varianceSwap.variance();

    // flat volatility: the replicated fair variance is the squared volatility, up to the
    // discretization of the replicating strikes
    if (std::fabs(fairVariance - sigma * sigma) > 1.0e-3)
        BOOST_ERROR("replicated fair variance " << fairVariance << " differs from "
                    << sigma * sigma);

    // feed the replicated variance strike to the volatility swap
    vars.variance->setValue(fairVariance);
    vars.logVol->setValue(0.0);
    VolatilitySwap flat(Position::Long, sigma, 1.0, vars.today, maturity);
    flat.setPricingEngine(vars.engine());
    if (std::fabs(flat.fairVolatility() - std::sqrt(fairVariance)) > 1.0e-15)
        BOOST_ERROR("fair volatility " << flat.fairVolatility()
                    << " differs from the square root of the replicated variance "
                    << std::sqrt(fairVariance));

    vars.logVol->setValue(0.3);
    Real adjusted = flat.fairVolatility();
    if (adjusted >= std::sqrt(fairVariance) ||
        std::fabs(adjusted - std::sqrt(fairVariance) * std::exp(-0.3 * 0.3 / 8.0)) > 1.0e-15)
        BOOST_ERROR("convexity-adjusted fair volatility " << adjusted << " is inconsistent");
}

BOOST_AUTO_TEST_CASE(testMonteCarlo) {
    BOOST_TEST_MESSAGE("Testing volatility swap fair strike against Monte Carlo...");

    CommonVars vars;
    Real K = vars.variance->value();
    Size samples = 400000;
    InverseCumulativeNormal invCum;

    for (Real s : {0.3, 0.6, 1.0}) {
        vars.logVol->setValue(s);
        MersenneTwisterUniformRng rng(42);
        IncrementalStatistics sqrtV, meanV;
        for (Size i = 0; i < samples; ++i) {
            Real z = invCum(rng.next().value);
            Real v = K * std::exp(-0.5 * s * s + s * z);
            sqrtV.add(std::sqrt(v));
            meanV.add(v);
        }

        VolatilitySwap swap(Position::Long, 0.2, 1.0, vars.start, vars.maturity);
        swap.setPricingEngine(vars.engine());
        Real fair = swap.fairVolatility();

        // the simulated variance has the mean the engine takes as input
        if (std::fabs(meanV.mean() - K) > 4.0 * meanV.errorEstimate())
            BOOST_ERROR("simulated mean variance " << meanV.mean() << " differs from " << K
                        << " by more than four standard errors (" << meanV.errorEstimate() << ")");

        Real error = std::fabs(sqrtV.mean() - fair);
        if (error > 4.0 * sqrtV.errorEstimate())
            BOOST_ERROR("log-volatility " << s << ": simulated E[sqrt(V)] " << sqrtV.mean()
                        << " differs from the engine's fair volatility " << fair << " by "
                        << error << ", more than four standard errors ("
                        << sqrtV.errorEstimate() << ")");

        // the convexity adjustment is resolved by the simulation
        Real adjustment = std::sqrt(K) - fair;
        if (adjustment < 10.0 * sqrtV.errorEstimate())
            BOOST_ERROR("log-volatility " << s << ": the convexity adjustment " << adjustment
                        << " is not resolved by the simulation (standard error "
                        << sqrtV.errorEstimate() << ")");
    }
}

BOOST_AUTO_TEST_CASE(testObserversAndRelinking) {
    BOOST_TEST_MESSAGE("Testing volatility swap engine observability...");

    CommonVars vars;
    VolatilitySwap swap(Position::Long, 0.15, 1000.0, vars.start, vars.maturity);
    swap.setPricingEngine(vars.engine());

    Real before = swap.NPV();
    vars.variance->setValue(0.0625);
    Real afterVariance = swap.NPV();
    if (std::fabs(afterVariance - before) < 1.0e-6)
        BOOST_ERROR("NPV did not react to a change in the variance strike quote");

    vars.logVol->setValue(0.0);
    Real afterLogVol = swap.NPV();
    if (std::fabs(afterLogVol - afterVariance) < 1.0e-6)
        BOOST_ERROR("NPV did not react to a change in the log-volatility quote");

    vars.curve.linkTo(flatRate(vars.today, 0.08, vars.dc));
    Real expected = 1000.0 * std::exp(-0.08 * vars.dc.yearFraction(vars.today, vars.maturity)) *
                    (0.25 - 0.15);
    if (std::fabs(swap.NPV() - expected) > 1.0e-8 * 1000.0)
        BOOST_ERROR("NPV " << swap.NPV() << " did not follow the relinked curve, expected "
                    << expected);
}

BOOST_AUTO_TEST_CASE(testInvalidInputs) {
    BOOST_TEST_MESSAGE("Testing volatility swap input validation...");

    CommonVars vars;

    auto priced = [&](Real strike, Real notional, const Date& start, const Date& maturity) {
        VolatilitySwap swap(Position::Long, strike, notional, start, maturity);
        swap.setPricingEngine(vars.engine());
        return swap.NPV();
    };

    BOOST_CHECK_THROW(priced(-0.1, 1.0, vars.start, vars.maturity), Error);
    BOOST_CHECK_THROW(priced(0.2, 0.0, vars.start, vars.maturity), Error);
    BOOST_CHECK_THROW(priced(0.2, -1.0, vars.start, vars.maturity), Error);
    BOOST_CHECK_THROW(priced(0.2, 1.0, Date(), vars.maturity), Error);
    BOOST_CHECK_THROW(priced(0.2, 1.0, vars.maturity, vars.start), Error);
    // a null maturity date is rejected when the arguments are validated
    VolatilitySwap::arguments nullMaturity;
    nullMaturity.position = Position::Long;
    nullMaturity.strike = 0.2;
    nullMaturity.notional = 1.0;
    nullMaturity.startDate = vars.start;
    BOOST_CHECK_THROW(nullMaturity.validate(), Error);
    // seasoned swap
    BOOST_CHECK_THROW(priced(0.2, 1.0, vars.today - 1 * Days, vars.maturity), Error);
    // a swap that starts today is not seasoned
    BOOST_CHECK_NO_THROW(priced(0.2, 1.0, vars.today, vars.maturity));

    vars.variance->setValue(0.0);
    BOOST_CHECK_THROW(priced(0.2, 1.0, vars.start, vars.maturity), Error);
    vars.variance->setValue(0.04);
    vars.logVol->setValue(-0.1);
    BOOST_CHECK_THROW(priced(0.2, 1.0, vars.start, vars.maturity), Error);
    vars.logVol->setValue(0.5);

    VolatilitySwap noCurve(Position::Long, 0.2, 1.0, vars.start, vars.maturity);
    noCurve.setPricingEngine(ext::make_shared<LognormalVolatilitySwapEngine>(
        RelinkableHandle<YieldTermStructure>(), Handle<Quote>(vars.variance),
        Handle<Quote>(vars.logVol)));
    BOOST_CHECK_THROW(noCurve.NPV(), Error);

    // an expired swap has no value
    VolatilitySwap expired(Position::Long, 0.2, 1.0, vars.today - 2 * Years,
                           vars.today - 1 * Years);
    expired.setPricingEngine(vars.engine());
    if (!expired.isExpired() || expired.NPV() != 0.0)
        BOOST_ERROR("expired volatility swap should be worth zero");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
