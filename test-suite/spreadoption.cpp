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

#undef REPORT_FAILURE
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
    Real value;
    Real theta;
};

void SpreadOptionTest::testKirkEngine() {
    BOOST_TEST_MESSAGE("Testing Kirk approximation for spread options...");

    /* The example data below are from "complete guide to option
       pricing formulas", Espen Gaarder Haug, p 60

       Expected values of option theta were calculated using automatic
       differentiation of the pricing function. The engine uses closed-form
       formula */

    Case cases[] = {
        {  28.0,  20.0, 7.0, 0.05, 0.29, 0.36,  0.42, 90,  2.1670,  -3.0431 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20, -0.5,  36,  4.7530, -25.5905 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20,  0.0,  36,  3.7970, -20.8841 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20,  0.5,  36,  2.5537, -14.7260 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20, -0.5, 180, 10.7517, -10.0847 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20,  0.0, 180,  8.7020,  -8.2619 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.20,  0.5, 180,  6.0257,  -5.8661 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20, -0.5,  36,  5.4275, -28.9013 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20,  0.0,  36,  4.3712, -23.7133 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20,  0.5,  36,  3.0086, -16.9864 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20, -0.5, 180, 12.1941, -11.3603 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20,  0.0, 180,  9.9340,  -9.3589 },
        { 122.0, 120.0, 3.0, 0.10, 0.25, 0.20,  0.5, 180,  7.0067,  -6.7463 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25, -0.5,  36,  5.4061, -28.7963 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25,  0.0,  36,  4.3451, -23.5848 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25,  0.5,  36,  2.9723, -16.8060 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25, -0.5, 180, 12.1483, -11.3200 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25,  0.0, 180,  9.8780,  -9.3091 },
        { 122.0, 120.0, 3.0, 0.10, 0.20, 0.25,  0.5, 180,  6.9284,  -6.6761 }
    };

    for (auto& i : cases) {

        // First step: preparing the test values
        // Useful dates
        DayCounter dc = Actual360();
        Date today = Date::todaysDate();
        Date exerciseDate = today + i.length;

        // Futures values
        std::shared_ptr<SimpleQuote> F1(new SimpleQuote(i.F1));
        std::shared_ptr<SimpleQuote> F2(new SimpleQuote(i.F2));

        // Risk-free interest rate
        Rate riskFreeRate = i.r;
        std::shared_ptr<YieldTermStructure> forwardRate =
            flatRate(today,riskFreeRate,dc);

        // Correlation
        std::shared_ptr<Quote> rho(new SimpleQuote(i.rho));

        // Volatilities
        Volatility vol1 = i.sigma1;
        Volatility vol2 = i.sigma2;
        std::shared_ptr<BlackVolTermStructure> volTS1 =
            flatVol(today,vol1,dc);
        std::shared_ptr<BlackVolTermStructure> volTS2 =
            flatVol(today,vol2,dc);

        // Black-Scholes Processes
        // The BlackProcess is the relevant class for futures contracts
        std::shared_ptr<BlackProcess> stochProcess1(
                     new BlackProcess(Handle<Quote>(F1),
                                      Handle<YieldTermStructure>(forwardRate),
                                      Handle<BlackVolTermStructure>(volTS1)));

        std::shared_ptr<BlackProcess> stochProcess2(
                     new BlackProcess(Handle<Quote>(F2),
                                      Handle<YieldTermStructure>(forwardRate),
                                      Handle<BlackVolTermStructure>(volTS2)));

        // Creating the pricing engine
        std::shared_ptr<PricingEngine> engine(
                      new KirkSpreadOptionEngine(stochProcess1, stochProcess2,
                                                 Handle<Quote>(rho)));

        // Finally, create the option:
        Option::Type type = Option::Call;
        Real strike = i.X;
        std::shared_ptr<PlainVanillaPayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));
        std::shared_ptr<Exercise> exercise(
                                          new EuropeanExercise(exerciseDate));

        SpreadOption option(payoff, exercise);
        option.setPricingEngine(engine);

        // And test the data
        Real value = option.NPV();
        Real theta = option.theta();
        Real tolerance = 1e-4;

        if (std::fabs(value - i.value) > tolerance) {
            REPORT_FAILURE("value", payoff, exercise, i.value, value, tolerance);
        }

        if (std::fabs(theta - i.theta) > tolerance) {
            REPORT_FAILURE("theta", payoff, exercise, i.theta, theta, tolerance);
        }
    }
}

test_suite* SpreadOptionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Spread option tests");

    suite->add(QUANTLIB_TEST_CASE(&SpreadOptionTest::testKirkEngine));

    return suite;
}

