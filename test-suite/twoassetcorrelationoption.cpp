/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include "twoassetcorrelationoption.hpp"
#include "utilities.hpp"
#include <ql/experimental/exoticoptions/twoassetcorrelationoption.hpp>
#include <ql/experimental/exoticoptions/analytictwoassetcorrelationengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/time/daycounters/actual360.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void TwoAssetCorrelationOptionTest::testAnalyticEngine() {
    BOOST_TEST_MESSAGE(
        "Testing analytic engine for two-asset correlation option...");

    Date today = Settings::instance().evaluationDate();
    DayCounter dc = Actual360();

    Option::Type type = Option::Call;
    Real strike1 = 50.0;
    Real strike2 = 70.0;
    Date exDate = today + 180;

    std::shared_ptr<Exercise> exercise =
        std::make_shared<EuropeanExercise>(exDate);

    TwoAssetCorrelationOption option(type, strike1, strike2, exercise);

    Handle<Quote> underlying1(std::make_shared<SimpleQuote>(52.0));
    Handle<Quote> underlying2(std::make_shared<SimpleQuote>(65.0));
    Handle<YieldTermStructure> dividendTS1(flatRate(today, 0.0, dc));
    Handle<YieldTermStructure> dividendTS2(flatRate(today, 0.0, dc));
    Handle<YieldTermStructure> riskFreeTS(flatRate(today, 0.1, dc));
    Handle<BlackVolTermStructure> blackVolTS1(flatVol(today, 0.2, dc));
    Handle<BlackVolTermStructure> blackVolTS2(flatVol(today, 0.3, dc));
    Handle<Quote> correlation(std::make_shared<SimpleQuote>(0.75));

    std::shared_ptr<BlackScholesMertonProcess> process1 =
        std::make_shared<BlackScholesMertonProcess>(underlying1,
                                                      dividendTS1,
                                                      riskFreeTS,
                                                      blackVolTS1);

    std::shared_ptr<BlackScholesMertonProcess> process2 =
        std::make_shared<BlackScholesMertonProcess>(underlying2,
                                                      dividendTS2,
                                                      riskFreeTS,
                                                      blackVolTS2);

    option.setPricingEngine(
          std::make_shared<AnalyticTwoAssetCorrelationEngine>(process1,
                                                                process2,
                                                                correlation));

    Real calculated = option.NPV();
    Real expected = 4.7073;
    Real error = std::fabs(calculated-expected);
    Real tolerance = 1e-4;
    if (error > tolerance)
        BOOST_ERROR("Failed to reproduce holder-extensible option value"
                    << "\n    expected:   " << expected
                    << "\n    calculated: " << calculated
                    << "\n    error:      " << error);
}

test_suite* TwoAssetCorrelationOptionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Two-asset correlation option tests");

    suite->add(QUANTLIB_TEST_CASE(
       &TwoAssetCorrelationOptionTest::testAnalyticEngine));

    return suite;
}
