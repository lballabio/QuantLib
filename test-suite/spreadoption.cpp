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

#include "spreadoption.hpp"
#include "utilities.hpp"
#include <ql/experimental/exoticoptions/kirkspreadoptionengine.hpp>
#include <ql/exercise.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using namespace std;

//namespace QuantLib {

#define REPORT_FAILURE( \
    greekName,  \
    payoff, exercise, \
    expected, calculated, tolerance) \
    BOOST_ERROR( \
    exerciseTypeToString(exercise) \
    << " Spread option with " \
    << payoffTypeToString(payoff) << " payoff:\n" \
    << "    strike:           " << payoff->strike() << "\n" \
    << "    reference date:   " << today << "\n" \
    << "    maturity:         " << exercise->lastDate() << "\n" \
    << "    expected   " << greekName << ": " << expected << "\n" \
    << "    calculated " << greekName << ": " << calculated << "\n" \
    << "    error:            " << std::fabs(expected-calculated) \
    << "\n" \
    << "    tolerance:        " << tolerance);


struct Case {
    Real F1;
    Real F2;
    Real X;
    Rate r;
    Volatility sigma1;
    Volatility sigma2;
    Real rho;
    Natural length;
    Real result;
};

void SpreadOptionTest::testKirkEngine() {
    BOOST_TEST_MESSAGE("Testing Kirk approximation for spread options...");

    /* The example data below are from "complete guide to option
       pricing formulas", Espen Gaarder Haug, p 60 */

    Case cases[] = {
        {  28.0,  20.0, 7.0, 0.05, 0.29, 0.36,  0.42, 90,  2.1670 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20, -0.5,  36,  4.7530 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20,  0.0,  36,  3.7970 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20,  0.5,  36,  2.5537 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20, -0.5, 180, 10.7517 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20,  0.0, 180,  8.7020 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20,  0.5, 180,  6.0257 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20, -0.5,  36,  5.4275 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20,  0.0,  36,  4.3712 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20,  0.5,  36,  3.0086 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20, -0.5, 180, 12.1941 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20,  0.0, 180,  9.9340 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20,  0.5, 180,  7.0067 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25, -0.5,  36,  5.4061 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25,  0.0,  36,  4.3451 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25,  0.5,  36,  2.9723 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25, -0.5, 180, 12.1483 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25,  0.0, 180,  9.8780 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25,  0.5, 180,  6.9284 }
    };

    for (Size i=0; i<LENGTH(cases); ++i) {

        // First step: preparing the test values
        // Useful dates
        DayCounter dc = Actual360();
        Date today = Date::todaysDate();
        Date exerciseDate = today  + cases[i].length;

        // Futures values
        boost::shared_ptr<SimpleQuote> F1(new SimpleQuote(cases[i].F1));
        boost::shared_ptr<SimpleQuote> F2(new SimpleQuote(cases[i].F2));

        // Risk-free interest rate
        Rate riskFreeRate = cases[i].r;
        boost::shared_ptr<YieldTermStructure> forwardRate =
            flatRate(today,riskFreeRate,dc);

        // Correlation
        boost::shared_ptr<Quote> rho(new SimpleQuote(cases[i].rho));

        // Volatilities
        Volatility vol1 = cases[i].sigma1;
        Volatility vol2 = cases[i].sigma2;
        boost::shared_ptr<BlackVolTermStructure> volTS1 =
            flatVol(today,vol1,dc);
        boost::shared_ptr<BlackVolTermStructure> volTS2 =
            flatVol(today,vol2,dc);

        // Black-Scholes Processes
        // The BlackProcess is the relevant class for futures contracts
        boost::shared_ptr<BlackProcess> stochProcess1(
                     new BlackProcess(Handle<Quote>(F1),
                                      Handle<YieldTermStructure>(forwardRate),
                                      Handle<BlackVolTermStructure>(volTS1)));

        boost::shared_ptr<BlackProcess> stochProcess2(
                     new BlackProcess(Handle<Quote>(F2),
                                      Handle<YieldTermStructure>(forwardRate),
                                      Handle<BlackVolTermStructure>(volTS2)));

        // Creating the pricing engine
        boost::shared_ptr<PricingEngine> engine(
                      new KirkSpreadOptionEngine(stochProcess1, stochProcess2,
                                                 Handle<Quote>(rho)));

        // Finally, create the option:
        Option::Type type = Option::Call;
        Real strike = cases[i].X;
        boost::shared_ptr<PlainVanillaPayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));
        boost::shared_ptr<Exercise> exercise(
                                          new EuropeanExercise(exerciseDate));

        SpreadOption option(payoff, exercise);
        option.setPricingEngine(engine);

        // And test the data
        Real calculated = option.NPV();
        Real expected = cases[i].result;
        Real tolerance = 1e-4;

        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value",
                           payoff, exercise,
                           expected, calculated, tolerance);
        }
    }
}

test_suite* SpreadOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Spread option tests");

    suite->add(QUANTLIB_TEST_CASE(&SpreadOptionTest::testKirkEngine));

    return suite;
}

