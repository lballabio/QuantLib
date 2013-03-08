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

#include "writerextensibleoption.hpp"
#include "utilities.hpp"
#include <ql/experimental/exoticoptions/writerextensibleoption.hpp>
#include <ql/experimental/exoticoptions/analyticwriterextensibleoptionengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName,payoff1,payoff2,exercise1,exercise2, \
                       spot, t1,t2,qRate,rRate,                       \
                       today,vol,expected,calculated,tolerance)       \
    BOOST_ERROR(payoff1->optionType() << " option with "                \
        << "    exercise1:         "<<exerciseTypeToString(exercise1)<< "\n" \
        << "    exercise2:         "<<exerciseTypeToString(exercise2)<< "\n" \
        << "    payoff1:           "<<payoffTypeToString(payoff1) << "\n" \
        << "    payoff2:           "<< payoffTypeToString(payoff2) << "\n" \
        << "    spot value:        " << spot << "\n" \
        << "    strike1:           " << payoff1->strike() << "\n" \
        << "    strike2:           " << payoff2->strike() << "\n" \
        << "    dividend yield:    " << io::rate(qRate) << "\n" \
        << "    risk-free rate:    " << io::rate(rRate) << "\n" \
        << "    reference date:    " << today << "\n" \
        << "    maturity1:         " << exercise1->lastDate() << "\n" \
        << "    maturity2:         " << exercise2->lastDate() << "\n" \
        << "    volatility:        " << io::volatility(vol) << "\n\n" \
        << "    expected   " << greekName << ": " << expected << "\n" \
        << "    calculated " << greekName << ": " << calculated << "\n" \
        << "    error:             " << std::fabs(expected-calculated)<< "\n" \
        << "    tolerance:         " << tolerance);

void WriterExtensibleOptionTest::testAnalyticWriterExtensibleOptionEngine() {
    BOOST_TEST_MESSAGE("Testing analytic engine for writer-extensible option...");

    // What we need for the option (tests):
    Option::Type type = Option::Call;
    Real strike1 = 90.0;
    Real strike2 = 82.0;
    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();
    Date exDate1 = today + 180;
    Date exDate2 = today + 270;

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(80.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> dividendTS =
        flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.10));
    boost::shared_ptr<YieldTermStructure> riskFreeTS =
        flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.30));
    boost::shared_ptr<BlackVolTermStructure> blackVolTS =
        flatVol(today, vol, dc);

    // B&S process (needed for the engine):
    const boost::shared_ptr<GeneralizedBlackScholesProcess> process(
        new GeneralizedBlackScholesProcess(
                    Handle<Quote>(spot),
                    Handle<YieldTermStructure>(dividendTS),
                    Handle<YieldTermStructure>(riskFreeTS),
                    Handle<BlackVolTermStructure>(blackVolTS)));

    // The engine:
    boost::shared_ptr<PricingEngine> engine(
                           new AnalyticWriterExtensibleOptionEngine(process));

    // Create the arguments:
    boost::shared_ptr<PlainVanillaPayoff> payoff1(
                                       new PlainVanillaPayoff(type, strike1));
    boost::shared_ptr<Exercise> exercise1(new EuropeanExercise(exDate1));
    boost::shared_ptr<PlainVanillaPayoff> payoff2(
                                       new PlainVanillaPayoff(type, strike2));
    boost::shared_ptr<Exercise> exercise2(new EuropeanExercise(exDate2));

    // Create the option by calling the constructor:
    WriterExtensibleOption option(payoff1, exercise1,
                                  payoff2, exercise2);

    //Set the engine of our option:
    option.setPricingEngine(engine);

    //Compare the calculated NPV value to the theoretical value:
    Real calculated = option.NPV();
    Real expected = 6.8238;

    Real tolerance = 1e-4;

    if (std::fabs(calculated-expected) > tolerance) {
        REPORT_FAILURE("value", payoff1, payoff2, exercise1, exercise2,
                       spot->value(), t1, t2, qRate->value(), rRate->value(),
                       today, vol->value(), expected, calculated, tolerance);
    }
}

test_suite* WriterExtensibleOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Writer-extensible option tests");

    suite->add(QUANTLIB_TEST_CASE(
       &WriterExtensibleOptionTest::testAnalyticWriterExtensibleOptionEngine));

    return suite;
}
