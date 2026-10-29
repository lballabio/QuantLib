/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Paolo D'Elia
 Copyright (C) 2026 Yassine Idyiahia

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
#include <ql/termstructures/volatility/interpolatedsmilesection.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

#include <vector>
#include <cmath>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(InterpolatedSmileSectionTests)

// Helper for linear interpolation/extrapolation between two points
static Real linearInterp(Real x, Real x1, Real y1, Real x2, Real y2) {
    if (x2 == x1)
        return y1;
    return y1 + (y2 - y1) * ( (x - x1) / (x2 - x1) );
}

BOOST_AUTO_TEST_CASE(testInterpolationAndVariance) {
    BOOST_TEST_MESSAGE("Testing basic behavior of linearly interpolated smile section...");
    // basic scenario: sorted strikes, constructor taking stdDevs (total std devs)
    Time expiry = 0.25; // 3 months
    Real sqrtT = std::sqrt(expiry);
    std::vector<Rate> strikes{90.0, 100.0, 110};
    // total std deviations (i.e., sigma * sqrt(T))
    std::vector<Real> stdDevs{
     0.20 * sqrtT,
     0.15 * sqrtT,
     0.18 * sqrtT};
    Real atmLevel = 95.0;

    auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
        expiry, strikes, stdDevs, atmLevel
    );

    // pick an interior strike 95 between 90 and 100
    Real strike = 95.0;

    Real v90 = stdDevs[0] / sqrtT;
    Real v100 = stdDevs[1] / sqrtT;
    Real expectedVol = linearInterp(strike, 90.0, v90, 100.0, v100);

    Real tol = 1e-12;
    QL_CHECK_CLOSE(section->volatilityImpl(strike), expectedVol, tol);

    // variance should be vol^2 * T
    Real expectedVar = expectedVol * expectedVol * expiry;
    QL_CHECK_CLOSE(section->varianceImpl(strike), expectedVar, tol);
}

BOOST_AUTO_TEST_CASE(testExtrapolationWhenAllowed) {
    BOOST_TEST_MESSAGE("Testing extrapolation behavior of linearly interpolated smile section...");
    // test extrapolation behavior when flatStrikeExtrapolation=false
    Time expiry = 0.25;
    Real sqrtT = std::sqrt(expiry);
    std::vector<Rate> strikes{90.0, 100.0, 110};
    std::vector<Real> stdDevs{
     0.20 * sqrtT,
     0.15 * sqrtT,
     0.18 * sqrtT};
    Real atmLevel = 95.0;

    auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
        expiry, strikes, stdDevs, atmLevel
    );

    // compute expected extrapolated vol at strike 80 (below min strike 90)
    Real v90 = stdDevs[0] / sqrtT;
    Real v100 = stdDevs[1] / sqrtT;
    Real strikeLow = 80.0;
    Real expectedLow = linearInterp(strikeLow, 90.0, v90, 100.0, v100);

    Real tol = 1e-12;
    QL_CHECK_CLOSE(section->volatilityImpl(strikeLow), expectedLow, tol);

    // extrapolate above max strike, e.g., 120 using last segment 110-100
    Real v110 = stdDevs[2] / sqrtT;
    Real strikeHigh = 120.0;
    // use last two points (110, v110) and (100, v100) for extrapolation;
    // linearInterp works for extrapolation as well
    Real expectedHigh = linearInterp(strikeHigh, 110.0, v110, 100.0, v100);
    QL_CHECK_CLOSE(section->volatilityImpl(strikeHigh), expectedHigh, tol);
}

BOOST_AUTO_TEST_CASE(testHandlesUpdatePropagates) {
    BOOST_TEST_MESSAGE("Testing that linearly interpolated smile section observes its quotes...");
    // construct via Quote handles and verify changing the underlying quote updates the section
    Time expiry = 0.25;
    Real sqrtT = std::sqrt(expiry);
    std::vector<Rate> strikes{80.0, 90.0, 100.0};

    // create SimpleQuote instances
    ext::shared_ptr<SimpleQuote> q0(new SimpleQuote(0.20 * sqrtT));
    ext::shared_ptr<SimpleQuote> q1(new SimpleQuote(0.15 * sqrtT));
    ext::shared_ptr<SimpleQuote> q2(new SimpleQuote(0.18 * sqrtT));
    std::vector<Handle<Quote>> stdDevHandles;
    stdDevHandles.emplace_back(q0);
    stdDevHandles.emplace_back(q1);
    stdDevHandles.emplace_back(q2);

    ext::shared_ptr<SimpleQuote> atm(new SimpleQuote(95.0));
    Handle<Quote> atmHandle(atm);

    auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
        expiry, strikes, stdDevHandles, atmHandle
    );

    // current vol at 95
    Real v90 = q1->value() / sqrtT;
    Real v100 = q2->value() / sqrtT;
    Real expectedBefore = linearInterp(95.0, 90.0, v90, 100.0, v100);

    Real tol = 1e-12;
    QL_CHECK_CLOSE(section->volatilityImpl(95.0), expectedBefore, tol);

    // now change the middle quote q1 from 0.15 to 0.20
    q1->setValue(0.20 * sqrtT);

    // after changing the quote, the section should reflect the new vol
    Real v90_after = q1->value() / sqrtT;
    Real expectedAfter = linearInterp(95.0, 90.0, v90_after, 100.0, v100);
    QL_CHECK_CLOSE(section->volatilityImpl(95.0), expectedAfter, tol);
}

BOOST_AUTO_TEST_CASE(testFlatStrikeExtrapolation) {
    BOOST_TEST_MESSAGE("Testing flat strike extrapolation in interpolated smile section...");
    // construct via Quote handles and verify changing the underlying quote updates the section
    Time expiry = 0.25;
    Real sqrtT = std::sqrt(expiry);
    std::vector<Rate> strikes{90.0, 100.0, 110.0};

    // create SimpleQuote instances
    ext::shared_ptr<SimpleQuote> q0(new SimpleQuote(0.20 * sqrtT));
    ext::shared_ptr<SimpleQuote> q1(new SimpleQuote(0.15 * sqrtT));
    ext::shared_ptr<SimpleQuote> q2(new SimpleQuote(0.18 * sqrtT));
    std::vector<Handle<Quote>> stdDevHandles;
    stdDevHandles.emplace_back(q0);
    stdDevHandles.emplace_back(q1);
    stdDevHandles.emplace_back(q2);

    ext::shared_ptr<SimpleQuote> atm(new SimpleQuote(95.0));
    Handle<Quote> atmHandle(atm);

    auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
        expiry, strikes, stdDevHandles, atmHandle,
        Linear(), Actual365Fixed(), ShiftedLognormal, 0.0, true
    );

    // Check with strike lower than minStrike()
    Real v90 = q0->value() / sqrtT;
    Real strikeLow = 85.0;

    Real tol = 1e-12;
    QL_CHECK_CLOSE(section->volatilityImpl(strikeLow), v90, tol);

    // Check with strike higher than maxStrike()
    Real v110 = q2->value() / sqrtT;
    Real strikeHigh = 120.0;

    QL_CHECK_CLOSE(section->volatilityImpl(strikeHigh), v110, tol);

    //Change minStrike vol quote and check if out-of-bounds vol is the same
    q0->setValue(0.21 * sqrtT);
    Real v90_after = q0->value() / sqrtT;
    QL_CHECK_CLOSE(section->volatilityImpl(strikeLow), v90_after, tol);
}

BOOST_AUTO_TEST_CASE(testErrorThrowingWhenNonSortedStrikes) {
    BOOST_TEST_MESSAGE("Testing that creation of interpolated smile section with non-sorted strikes throws...");
    // basic scenario: sorted strikes, constructor taking stdDevs (total std devs)
    Time expiry = 0.25; // 3 months
    Real sqrtT = std::sqrt(expiry);
    std::vector<Rate> strikes{90.0, 110.0, 100};
    // total std deviations (i.e., sigma * sqrt(T))
    std::vector<Real> stdDevs{
     0.20 * sqrtT,
     0.15 * sqrtT,
     0.18 * sqrtT};
    Real atmLevel = 95.0;

    BOOST_CHECK_THROW(
        auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
            expiry, strikes, stdDevs, atmLevel
        ),
        QuantLib::Error
    );
}

BOOST_AUTO_TEST_CASE(testDigitalOptionPriceAtLargeStrikes) {
    BOOST_TEST_MESSAGE("Testing digital option price at strikes large enough "
                       "to absorb the differencing gap...");

    // The digital differences the option price across a gap. Once the gap falls
    // below the strike's own precision the interval quantizes to whole ulps, and
    // the probability that comes back is wrong without anything reporting it.
    // The low levels also pin the unaffected range, where the floor never binds.
    Time expiry = 1.0;
    Real sqrtT = std::sqrt(expiry);
    Volatility vol = 0.20;
    Real tol = 1e-4;

    // At the money with a flat smile, P(S > F) = N(-sigma sqrt(T) / 2),
    // independent of the level.
    Real expected = CumulativeNormalDistribution()(-0.5 * vol * sqrtT);

    for (Real atmLevel : { 1.0e2, 1.0e3, 1.0e6, 1.0e11 }) {
        std::vector<Rate> strikes{0.9 * atmLevel, atmLevel, 1.1 * atmLevel};
        std::vector<Real> stdDevs{vol * sqrtT, vol * sqrtT, vol * sqrtT};

        auto section = ext::make_shared<InterpolatedSmileSection<Linear> >(
            expiry, strikes, stdDevs, atmLevel);

        Real calculated = section->digitalOptionPrice(atmLevel, Option::Call, 1.0);

        if (std::fabs(calculated - expected) > tol) {
            BOOST_FAIL("failed to reproduce the digital option price at a large strike"
                       << "\n   strike:     " << atmLevel
                       << "\n   calculated: " << calculated
                       << "\n   expected:   " << expected
                       << "\n   diff:       " << calculated - expected
                       << "\n   tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testDensityAtLargeStrikes) {
    BOOST_TEST_MESSAGE("Testing smile-section density at equity index level "
                       "strikes, which absorb the differencing gap...");

    // density differences the digital across a gap, and the digital itself
    // differences the option price, so the roundoff is |strike|*eps/gap^2
    // against a density falling like 1/strike. The relative error therefore
    // grows like strike^2 and reaches 100% around 3e4 -- Nikkei or Hang Seng
    // levels -- so a fixed gap fails far earlier here than for the digital.
    Time expiry = 1.0;
    Real sqrtT = std::sqrt(expiry);
    Volatility vol = 0.20;
    Real tol = 1e-3;

    // At 1e2 the floor coincides with the default gap and nothing changes. The
    // middle three are KOSPI, S&P 500 and Nikkei 225 levels, where the unfixed
    // density is respectively 0.2%, 8% and 600% out. Past 1e6 the second
    // difference is pure roundoff and can come back negative.
    for (Real atmLevel : { 1.0e2, 2.5e3, 5.0e3, 3.8e4, 1.0e6, 1.0e11 }) {
        std::vector<Rate> strikes{0.9 * atmLevel, atmLevel, 1.1 * atmLevel};
        std::vector<Real> stdDevs{vol * sqrtT, vol * sqrtT, vol * sqrtT};

        auto section = ext::make_shared<InterpolatedSmileSection<Linear> >(
            expiry, strikes, stdDevs, atmLevel);

        Real calculated = section->density(atmLevel, 1.0);

        // At the money with a flat smile the Black density is
        // phi(d2) / (K sigma sqrt(T)) with d2 = -sigma sqrt(T) / 2.
        Real d2 = -0.5 * vol * sqrtT;
        Real expected = NormalDistribution()(d2) / (atmLevel * vol * sqrtT);

        // NaN would make the relative error below compare
        // false against the tolerance and slip through.
        BOOST_REQUIRE(std::isfinite(value(calculated)));
        Real relError = std::fabs(calculated - expected) / expected;
        if (relError > tol) {
            BOOST_FAIL("failed to reproduce the density at a large strike"
                       << "\n   strike:     " << atmLevel
                       << "\n   calculated: " << calculated
                       << "\n   expected:   " << expected
                       << "\n   rel error:  " << relError
                       << "\n   tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testDensityHonoursExplicitGap) {
    BOOST_TEST_MESSAGE("Testing that smile-section density honours an explicit gap...");

    // A gap above the floor must reach both digitals and the divisor, leaving
    // density the central second difference of the call price.
    Time expiry = 1.0;
    Real sqrtT = std::sqrt(expiry);
    Volatility vol = 0.20;
    Real atmLevel = 1.0e11;
    Real gap = 1.0e10;

    std::vector<Rate> strikes{0.9 * atmLevel, atmLevel, 1.1 * atmLevel};
    std::vector<Real> stdDevs{vol * sqrtT, vol * sqrtT, vol * sqrtT};

    auto section = ext::make_shared<InterpolatedSmileSection<Linear> >(
        expiry, strikes, stdDevs, atmLevel);

    Real calculated = section->density(atmLevel, 1.0, gap);

    Real stdDev = vol * sqrtT;
    Real expected = (blackFormula(Option::Call, atmLevel - gap, atmLevel, stdDev) -
                     2.0 * blackFormula(Option::Call, atmLevel, atmLevel, stdDev) +
                     blackFormula(Option::Call, atmLevel + gap, atmLevel, stdDev)) / (gap * gap);

    Real tol = 1e-8;
    Real relError = std::fabs(calculated - expected) / expected;
    if (relError > tol) {
        BOOST_FAIL("failed to honour the explicit gap"
                   << "\n   gap:        " << gap
                   << "\n   calculated: " << calculated
                   << "\n   expected:   " << expected
                   << "\n   rel error:  " << relError
                   << "\n   tolerance:  " << tol);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
