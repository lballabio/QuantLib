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
#include <ql/termstructures/volatility/flatsmilesection.hpp>
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
    BOOST_TEST_MESSAGE("Testing basic behaviour of linearly interpolated smile section...");
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
    BOOST_CHECK_CLOSE(section->volatilityImpl(strike), expectedVol, tol);

    // variance should be vol^2 * T
    Real expectedVar = expectedVol * expectedVol * expiry;
    BOOST_CHECK_CLOSE(section->varianceImpl(strike), expectedVar, tol);
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
    stdDevHandles.emplace_back(Handle<Quote>(q0));
    stdDevHandles.emplace_back(Handle<Quote>(q1));
    stdDevHandles.emplace_back(Handle<Quote>(q2));

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
    BOOST_CHECK_CLOSE(section->volatilityImpl(95.0), expectedBefore, tol);

    // now change the middle quote q1 from 0.15 to 0.20
    q1->setValue(0.20 * sqrtT);

    // after changing the quote, the section should reflect the new vol
    Real v90_after = q1->value() / sqrtT;
    Real expectedAfter = linearInterp(95.0, 90.0, v90_after, 100.0, v100);
    BOOST_CHECK_CLOSE(section->volatilityImpl(95.0), expectedAfter, tol);
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
    stdDevHandles.emplace_back(Handle<Quote>(q0));
    stdDevHandles.emplace_back(Handle<Quote>(q1));
    stdDevHandles.emplace_back(Handle<Quote>(q2));

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
    BOOST_CHECK_CLOSE(section->volatilityImpl(strikeLow), v90, tol);

    // Check with strike higher than maxStrike()
    Real v110 = q2->value() / sqrtT;
    Real strikeHigh = 120.0;

    BOOST_CHECK_CLOSE(section->volatilityImpl(strikeHigh), v110, tol);

    //Change minStrike vol quote and check if out-of-bounds vol is the same
    q0->setValue(0.21 * sqrtT);
    Real v90_after = q0->value() / sqrtT;
    BOOST_CHECK_CLOSE(section->volatilityImpl(strikeLow), v90_after, tol);
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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(SmileSectionOptionalForward)

BOOST_AUTO_TEST_CASE(testForwardAwareOptionPrice) {
    BOOST_TEST_MESSAGE(
        "Testing SmileSection methods with optional forward parameter...");

    DayCounter dc = Actual365Fixed();
    Volatility vol = 0.20;
    Time T = 1.0;
    Real forward = 100.0;
    Real discount = 0.97;

    // FlatSmileSection without atmLevel — simulates a forward-unaware smile
    auto smile = ext::make_shared<FlatSmileSection>(T, vol, dc);

    // The virtual optionPrice() without forward should fail (no atmLevel)
    BOOST_CHECK_THROW(smile->optionPrice(100.0, Option::Call, discount),
                      QuantLib::Error);

    // With forward parameter it should work
    Real tolerance = 1.0e-12;

    // Compare against blackFormula directly
    Real strike = 105.0;
    Real stdDev = vol * std::sqrt(T);
    Real expected = blackFormula(Option::Call, strike, forward,
                                 stdDev, discount);
    Real calculated = smile->optionPrice(strike, Option::Call,
                                          discount, forward);

    if (std::fabs(calculated - expected) > tolerance)
        BOOST_FAIL("forward-aware optionPrice mismatch"
                   << std::fixed << std::setprecision(12)
                   << "\n    expected:   " << expected
                   << "\n    calculated: " << calculated);

    // digitalOptionPrice with forward
    Real gap = 1.0e-5;
    Real digital = smile->digitalOptionPrice(strike, Option::Call,
                                              discount, gap, forward);
    if (digital <= 0.0 || digital > 1.0 / discount)
        BOOST_FAIL("forward-aware digitalOptionPrice out of range"
                   << std::fixed << std::setprecision(12)
                   << "\n    digital: " << digital);

    // density with forward
    Real dens = smile->density(strike, discount, 1.0e-4, forward);
    if (dens <= 0.0)
        BOOST_FAIL("forward-aware density should be positive"
                   << std::fixed << std::setprecision(12)
                   << "\n    density: " << dens);

    // vega with forward
    Real v = smile->vega(strike, discount, forward);
    if (v <= 0.0)
        BOOST_FAIL("forward-aware vega should be positive"
                   << std::fixed << std::setprecision(12)
                   << "\n    vega: " << v);

    // Consistency: forward parameter should match atmLevel-based result
    auto smileWithFwd = ext::make_shared<FlatSmileSection>(T, vol, dc,
                                                            forward);
    Real fromAtmLevel = smileWithFwd->optionPrice(strike, Option::Call,
                                                   discount);
    Real fromForward = smileWithFwd->optionPrice(strike, Option::Call,
                                                  discount, forward);

    if (std::fabs(fromAtmLevel - fromForward) > tolerance)
        BOOST_FAIL("forward parameter inconsistent with atmLevel"
                   << std::fixed << std::setprecision(12)
                   << "\n    via atmLevel: " << fromAtmLevel
                   << "\n    via forward:  " << fromForward);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
