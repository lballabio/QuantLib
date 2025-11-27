/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Paolo D'Elia

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
#include <ql/math/interpolations/linearinterpolation.hpp>
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
    BOOST_TEST_MESSAGE("Testing basic behaviour of Linear Interpolated Smile Section");
    // basic scenario: sorted strikes, constructor taking stdDevs (total std devs)
    Time expiry = 0.25; // 3 months
    std::vector<Rate> strikes{90.0, 100.0, 110};
    // total std deviations (i.e., sigma * sqrt(T))
    std::vector<Real> stdDevs{0.20, 0.15, 0.18};
    Real atmLevel = 95.0;

    auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
        expiry, strikes, stdDevs, atmLevel
    );

    // pick an interior strike 95 between 90 and 100
    Real strike = 95.0;

    // vols stored internally are stdDev / sqrt(T)
    Real sqrtT = std::sqrt(expiry);
    Real v90 = stdDevs[0] / sqrtT;
    Real v100 = stdDevs[1] / sqrtT;
    Real expectedVol = linearInterp(strike, 90.0, v90, 100.0, v100);

    Real tol = 1e-12;
    BOOST_CHECK_CLOSE(section->volatilityImpl(strike), expectedVol, tol);

    // variance should be vol^2 * T
    Real expectedVar = expectedVol * expectedVol * expiry;
    BOOST_CHECK_CLOSE(section->varianceImpl(strike), expectedVar, tol);
}

BOOST_AUTO_TEST_CASE(testExtrapolationWhenAllowed) {
    BOOST_TEST_MESSAGE("Testing extrapolation behavior of Linear Interpolated Smile Section");
    // test extrapolation behavior when flatStrikeExtrapolation=false
    Time expiry = 0.25;
    std::vector<Rate> strikes{90.0, 100.0, 110};
    std::vector<Real> stdDevs{0.20, 0.15, 0.18};
    Real atmLevel = 95.0;

    auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
        expiry, strikes, stdDevs, atmLevel
    );

    Real sqrtT = std::sqrt(expiry);
    // compute expected extrapolated vol at strike 80 (below min strike 90)
    Real v90 = stdDevs[0] / sqrtT;
    Real v100 = stdDevs[1] / sqrtT;
    Real strikeLow = 80.0;
    Real expectedLow = linearInterp(strikeLow, 90.0, v90, 100.0, v100);

    Real tol = 1e-12;
    BOOST_CHECK_CLOSE(section->volatilityImpl(strikeLow), expectedLow, tol);

    // extrapolate above max strike, e.g., 120 using last segment 110-100
    Real v110 = stdDevs[2] / sqrtT;
    Real strikeHigh = 120.0;
    // use last two points (110, v110) and (100, v100) for extrapolation;
    // linearInterp works for extrapolation as well
    Real expectedHigh = linearInterp(strikeHigh, 110.0, v110, 100.0, v100);
    BOOST_CHECK_CLOSE(section->volatilityImpl(strikeHigh), expectedHigh, tol);
}

BOOST_AUTO_TEST_CASE(testHandlesUpdatePropagates) {
    BOOST_TEST_MESSAGE("Testing construction of Linear Interpolated Smile Section using Quote Handles for vols and updating them");
    // construct via Quote handles and verify changing the underlying quote updates the section
    Time expiry = 0.25;
    std::vector<Rate> strikes{80.0, 90.0, 100.0};

    // create SimpleQuote instances
    ext::shared_ptr<SimpleQuote> q0(new SimpleQuote(0.20));
    ext::shared_ptr<SimpleQuote> q1(new SimpleQuote(0.15));
    ext::shared_ptr<SimpleQuote> q2(new SimpleQuote(0.18));
    std::vector<Handle<Quote>> stdDevHandles;
    stdDevHandles.emplace_back(Handle<Quote>(q0));
    stdDevHandles.emplace_back(Handle<Quote>(q1));
    stdDevHandles.emplace_back(Handle<Quote>(q2));

    ext::shared_ptr<SimpleQuote> atm(new SimpleQuote(95.0));
    Handle<Quote> atmHandle(atm);

    auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
        expiry, strikes, stdDevHandles, atmHandle
    );

    Real sqrtT = std::sqrt(expiry);
    // current vol at 95
    Real v90 = q1->value() / sqrtT;
    Real v100 = q2->value() / sqrtT;
    Real expectedBefore = linearInterp(95.0, 90.0, v90, 100.0, v100);

    Real tol = 1e-12;
    BOOST_CHECK_CLOSE(section->volatilityImpl(95.0), expectedBefore, tol);

    // now change the middle quote q1 from 0.15 to 0.20
    q1->setValue(0.20);

    // after changing the quote, the section should reflect the new vol
    Real v90_after = q1->value() / sqrtT;
    Real expectedAfter = linearInterp(95.0, 90.0, v90_after, 100.0, v100);
    BOOST_CHECK_CLOSE(section->volatilityImpl(95.0), expectedAfter, tol);
}

BOOST_AUTO_TEST_CASE(testFlatStrikeExtrapolation) {
    BOOST_TEST_MESSAGE("Testing flat strike extrapolation in Interpolated Smile Section");
    // construct via Quote handles and verify changing the underlying quote updates the section
    Time expiry = 0.25;
    std::vector<Rate> strikes{90.0, 100.0, 110.0};

    // create SimpleQuote instances
    ext::shared_ptr<SimpleQuote> q0(new SimpleQuote(0.20));
    ext::shared_ptr<SimpleQuote> q1(new SimpleQuote(0.15));
    ext::shared_ptr<SimpleQuote> q2(new SimpleQuote(0.18));
    std::vector<Handle<Quote>> stdDevHandles;
    stdDevHandles.emplace_back(Handle<Quote>(q0));
    stdDevHandles.emplace_back(Handle<Quote>(q1));
    stdDevHandles.emplace_back(Handle<Quote>(q2));

    ext::shared_ptr<SimpleQuote> atm(new SimpleQuote(95.0));
    Handle<Quote> atmHandle(atm);

    auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
        expiry, strikes, stdDevHandles, atmHandle,
        Linear(), Actual365Fixed(), ShiftedLognormal, 0.0, true
    );

    Real sqrtT = std::sqrt(expiry);
    // Check with strike lower than minStrike()
    Real v90 = q0->value() / sqrtT;
    Real strikeLow = 85.0;

    Real tol = 1e-12;
    BOOST_CHECK_CLOSE(section->volatilityImpl(strikeLow), v90, tol);

    // Check with strike higher than maxStrike()
    Real v110 = q2->value() / sqrtT;
    Real strikeHigh = 120.0;

    BOOST_CHECK_CLOSE(section->volatilityImpl(strikeHigh), v110, tol);

    //Change minStrike vol quote and check if out-of-bounds vol is the same
    q0->setValue(0.21);
    Real v90_after = q0->value() / sqrtT;
    BOOST_CHECK_CLOSE(section->volatilityImpl(strikeLow), v90_after, tol);
}

BOOST_AUTO_TEST_CASE(testErrorThrowingWhenNonSortedStrikes) {
    BOOST_TEST_MESSAGE("Testing throwin QuantLib::Error when creating Interpolated Smile section with non-sorted strikes");
    // basic scenario: sorted strikes, constructor taking stdDevs (total std devs)
    Time expiry = 0.25; // 3 months
    std::vector<Rate> strikes{90.0, 110.0, 100};
    // total std deviations (i.e., sigma * sqrt(T))
    std::vector<Real> stdDevs{0.20, 0.15, 0.18};
    Real atmLevel = 95.0;

    BOOST_CHECK_THROW(
        auto section = ext::make_shared<InterpolatedSmileSection<Linear>>(
            expiry, strikes, stdDevs, atmLevel
        ),
        QuantLib::Error
    );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
