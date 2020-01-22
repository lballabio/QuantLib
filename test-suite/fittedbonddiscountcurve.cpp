/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "fittedbonddiscountcurve.hpp"
#include "utilities.hpp"
#include <ql/termstructures/yield/fittedbonddiscountcurve.hpp>
#include <ql/termstructures/yield/nonlinearfittingmethods.hpp>
#include <ql/instruments/bonds/zerocouponbond.hpp>
#include <ql/time/calendars/target.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void FittedBondDiscountCurveTest::testEvaluation() {

    BOOST_TEST_MESSAGE("Testing that fitted bond curves work as evaluators...");

    Date today = Settings::instance().evaluationDate();
    ext::shared_ptr<Bond> bond = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0,
                                                                  today + Period(10, Years));
    Handle<Quote> q(ext::make_shared<SimpleQuote>(100.0));

    std::vector<ext::shared_ptr<BondHelper> > helpers(1);
    helpers[0] = ext::make_shared<BondHelper>(q, bond);

    ExponentialSplinesFitting fittingMethod;

    Size maxIterations = 0;
    Array guess(9);
    guess[0] = -51293.44;
    guess[1] = -212240.36;
    guess[2] = 168668.51;
    guess[3] = 88792.74;
    guess[4] = 120712.13;
    guess[5] = -34332.83;
    guess[6] = -66479.66;
    guess[7] = 13605.17;
    guess[8] = 0.0;

    FittedBondDiscountCurve curve(0, TARGET(), helpers, Actual365Fixed(),
                                  fittingMethod, 1e-10, maxIterations, guess);

    BOOST_CHECK_NO_THROW(curve.discount(3.0));
}


test_suite* FittedBondDiscountCurveTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Fitted bond discount curve tests");
    suite->add(QUANTLIB_TEST_CASE(&FittedBondDiscountCurveTest::testEvaluation));
    return suite;
}
