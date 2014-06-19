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
#include <boost/make_shared.hpp>

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

    boost::shared_ptr<SimpleQuote> spot = boost::make_shared<SimpleQuote>(100.0);
    boost::shared_ptr<SimpleQuote> qRate = boost::make_shared<SimpleQuote>(0.0);
    boost::shared_ptr<SimpleQuote> rRate = boost::make_shared<SimpleQuote>(0.08);
    boost::shared_ptr<SimpleQuote> vol = boost::make_shared<SimpleQuote>(0.25);

    boost::shared_ptr<StrikedTypePayoff> payoff =
        boost::make_shared<PlainVanillaPayoff>(type, strike1);
    boost::shared_ptr<Exercise> exercise =
        boost::make_shared<EuropeanExercise>(exDate1);

    HolderExtensibleOption option(type, premium,
                                  exDate2, strike2,
                                  payoff, exercise);

    Handle<Quote> underlying(spot);
    Handle<YieldTermStructure> dividendTS(flatRate(today, qRate, dc));
    Handle<YieldTermStructure> riskFreeTS(flatRate(today, rRate, dc));
    Handle<BlackVolTermStructure> blackVolTS(flatVol(today, vol, dc));

    const boost::shared_ptr<BlackScholesMertonProcess> process =
        boost::make_shared<BlackScholesMertonProcess>(underlying,
                                                      dividendTS,
                                                      riskFreeTS,
                                                      blackVolTS);

    option.setPricingEngine(
           boost::make_shared<AnalyticHolderExtensibleOptionEngine>(process));

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

    boost::shared_ptr<SimpleQuote> spot = boost::make_shared<SimpleQuote>(80.0);
    boost::shared_ptr<SimpleQuote> qRate = boost::make_shared<SimpleQuote>(0.0);
    boost::shared_ptr<YieldTermStructure> dividendTS =
        flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate = boost::make_shared<SimpleQuote>(0.10);
    boost::shared_ptr<YieldTermStructure> riskFreeTS =
        flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol = boost::make_shared<SimpleQuote>(0.30);
    boost::shared_ptr<BlackVolTermStructure> blackVolTS =
        flatVol(today, vol, dc);

    // B&S process (needed for the engine):
    const boost::shared_ptr<GeneralizedBlackScholesProcess> process =
        boost::make_shared<GeneralizedBlackScholesProcess>(
                    Handle<Quote>(spot),
                    Handle<YieldTermStructure>(dividendTS),
                    Handle<YieldTermStructure>(riskFreeTS),
                    Handle<BlackVolTermStructure>(blackVolTS));

    // The engine:
    boost::shared_ptr<PricingEngine> engine =
        boost::make_shared<AnalyticWriterExtensibleOptionEngine>(process);

    // Create the arguments:
    boost::shared_ptr<PlainVanillaPayoff> payoff1 =
        boost::make_shared<PlainVanillaPayoff>(type, strike1);
    boost::shared_ptr<Exercise> exercise1 =
        boost::make_shared<EuropeanExercise>(exDate1);
    boost::shared_ptr<PlainVanillaPayoff> payoff2 =
        boost::make_shared<PlainVanillaPayoff>(type, strike2);
    boost::shared_ptr<Exercise> exercise2 =
        boost::make_shared<EuropeanExercise>(exDate2);

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
    test_suite* suite = BOOST_TEST_SUITE("Extensible option tests");

    suite->add(QUANTLIB_TEST_CASE(
       &ExtensibleOptionsTest::testAnalyticHolderExtensibleOptionEngine));
    suite->add(QUANTLIB_TEST_CASE(
       &ExtensibleOptionsTest::testAnalyticWriterExtensibleOptionEngine));

    return suite;
}
