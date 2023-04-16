/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include "extensibleoptions.hpp"
#include "utilities.hpp"
#include <ql/experimental/exoticoptions/holderextensibleoption.hpp>
#include <ql/experimental/exoticoptions/writerextensibleoption.hpp>
#include <ql/experimental/exoticoptions/analyticholderextensibleoptionengine.hpp>
#include <ql/experimental/exoticoptions/analyticwriterextensibleoptionengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void ExtensibleOptionsTest::testAnalyticHolderExtensibleOptionEngine() {
    BOOST_TEST_MESSAGE(
        "Testing analytic engine for holder-extensible option...");

    Option::Type type = Option::Call;
    Real strike1 = 100.0;
    Real strike2 = 105.0;
    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();
    Date exDate1 = today + 180;
    Date exDate2 = today + 270;
    Real premium = 1.0;

    std::shared_ptr<SimpleQuote> spot = std::make_shared<SimpleQuote>(100.0);
    std::shared_ptr<SimpleQuote> qRate = std::make_shared<SimpleQuote>(0.0);
    std::shared_ptr<SimpleQuote> rRate = std::make_shared<SimpleQuote>(0.08);
    std::shared_ptr<SimpleQuote> vol = std::make_shared<SimpleQuote>(0.25);

    std::shared_ptr<StrikedTypePayoff> payoff =
        std::make_shared<PlainVanillaPayoff>(type, strike1);
    std::shared_ptr<Exercise> exercise =
        std::make_shared<EuropeanExercise>(exDate1);

    HolderExtensibleOption option(type, premium,
                                  exDate2, strike2,
                                  payoff, exercise);

    Handle<Quote> underlying(spot);
    Handle<YieldTermStructure> dividendTS(flatRate(today, qRate, dc));
    Handle<YieldTermStructure> riskFreeTS(flatRate(today, rRate, dc));
    Handle<BlackVolTermStructure> blackVolTS(flatVol(today, vol, dc));

    const std::shared_ptr<BlackScholesMertonProcess> process =
        std::make_shared<BlackScholesMertonProcess>(underlying,
                                                      dividendTS,
                                                      riskFreeTS,
                                                      blackVolTS);

    option.setPricingEngine(
           std::make_shared<AnalyticHolderExtensibleOptionEngine>(process));

    Real calculated = option.NPV();
    Real expected = 9.4233;
    Real error = std::fabs(calculated-expected);
    Real tolerance = 1e-4;
    if (error > tolerance)
        BOOST_ERROR("Failed to reproduce holder-extensible option value"
                    << "\n    expected:   " << expected
                    << "\n    calculated: " << calculated
                    << "\n    error:      " << error);
}


void ExtensibleOptionsTest::testAnalyticWriterExtensibleOptionEngine() {
    BOOST_TEST_MESSAGE("Testing analytic engine for writer-extensible option...");

    // What we need for the option (tests):
    Option::Type type = Option::Call;
    Real strike1 = 90.0;
    Real strike2 = 82.0;
    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();
    Date exDate1 = today + 180;
    Date exDate2 = today + 270;

    std::shared_ptr<SimpleQuote> spot = std::make_shared<SimpleQuote>(80.0);
    std::shared_ptr<SimpleQuote> qRate = std::make_shared<SimpleQuote>(0.0);
    std::shared_ptr<YieldTermStructure> dividendTS =
        flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate = std::make_shared<SimpleQuote>(0.10);
    std::shared_ptr<YieldTermStructure> riskFreeTS =
        flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol = std::make_shared<SimpleQuote>(0.30);
    std::shared_ptr<BlackVolTermStructure> blackVolTS =
        flatVol(today, vol, dc);

    // B&S process (needed for the engine):
    const std::shared_ptr<GeneralizedBlackScholesProcess> process =
        std::make_shared<GeneralizedBlackScholesProcess>(
                    Handle<Quote>(spot),
                    Handle<YieldTermStructure>(dividendTS),
                    Handle<YieldTermStructure>(riskFreeTS),
                    Handle<BlackVolTermStructure>(blackVolTS));

    // The engine:
    std::shared_ptr<PricingEngine> engine =
        std::make_shared<AnalyticWriterExtensibleOptionEngine>(process);

    // Create the arguments:
    std::shared_ptr<PlainVanillaPayoff> payoff1 =
        std::make_shared<PlainVanillaPayoff>(type, strike1);
    std::shared_ptr<Exercise> exercise1 =
        std::make_shared<EuropeanExercise>(exDate1);
    std::shared_ptr<PlainVanillaPayoff> payoff2 =
        std::make_shared<PlainVanillaPayoff>(type, strike2);
    std::shared_ptr<Exercise> exercise2 =
        std::make_shared<EuropeanExercise>(exDate2);

    // Create the option by calling the constructor:
    WriterExtensibleOption option(payoff1, exercise1,
                                  payoff2, exercise2);

    //Set the engine of our option:
    option.setPricingEngine(engine);

    //Compare the calculated NPV value to the theoretical value:
    Real calculated = option.NPV();
    Real expected = 6.8238;
    Real error = std::fabs(calculated-expected);
    Real tolerance = 1e-4;
    if (error > tolerance)
        BOOST_ERROR("Failed to reproduce writer-extensible option value"
                    << "\n    expected:   " << expected
                    << "\n    calculated: " << calculated
                    << "\n    error:      " << error);
}

test_suite* ExtensibleOptionsTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Extensible option tests");

    suite->add(QUANTLIB_TEST_CASE(
       &ExtensibleOptionsTest::testAnalyticHolderExtensibleOptionEngine));
    suite->add(QUANTLIB_TEST_CASE(
       &ExtensibleOptionsTest::testAnalyticWriterExtensibleOptionEngine));

    return suite;
}
