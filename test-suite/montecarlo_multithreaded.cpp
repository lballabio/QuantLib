/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include "montecarlo_multithreaded.hpp"
#include "utilities.hpp"
#include <ql/processes/hestonprocess.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/vanilla/mceuropeanhestonengine.hpp>
#include <ql/experimental/exoticoptions/analyticpdfhestonengine.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/period.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/experimental/math/rngtraits_multithreaded.hpp>

#include <boost/make_shared.hpp>

#if defined(_OPENMP)
#include <omp.h>
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

void MonteCarloMultiThreadedTest::testHestonEngine() {

#if !defined(_OPENMP)

    BOOST_TEST_MESSAGE("Skipping multithreaded Monte Carlo Heston engine test, because OpenMP is not enabled");

#else

    // this is taken from the Heston model tests 
    BOOST_TEST_MESSAGE(
                "Testing multithreaded Monte Carlo Heston engine against cached values...");

    SavedSettings backup;

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual();
    Date exerciseDate(28, March, 2005);

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                   new PlainVanillaPayoff(Option::Put, 1.05));
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.7, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.4, dayCounter));

    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(1.05)));

    boost::shared_ptr<HestonProcess> process(new HestonProcess(
                   riskFreeTS, dividendTS, s0, 0.3, 1.16, 0.2, 0.8, 0.8,
                   HestonProcess::QuadraticExponentialMartingale));

    VanillaOption option(payoff, exercise);

    boost::shared_ptr<PricingEngine> engine;

    // use a multithreaded RNG and more samples than in the original test
    // to produce some traffic
    
    // PseudoRandomMultiThreaded can only handle 8 threads
    if(omp_get_num_threads() > 8)
        omp_set_num_threads(8);

    engine = MakeMCEuropeanHestonEngine<PseudoRandomMultiThreaded>(process)
        .withStepsPerYear(11)
        .withAntitheticVariate()
        .withSamples(500000)
        .withSeed(1234);

    option.setPricingEngine(engine);

    Real expected = 0.0632851308977151;
    Real calculated = option.NPV();
    Real errorEstimate = option.errorEstimate();
    Real tolerance = 7.5e-4;

    if (std::fabs(calculated - expected) > 2.34*errorEstimate) {
        BOOST_ERROR("Failed to reproduce cached price"
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << " +/- " << errorEstimate);
    }

    if (errorEstimate > tolerance) {
        BOOST_ERROR("failed to reproduce error estimate"
                    << "\n    calculated: " << errorEstimate
                    << "\n    expected:   " << tolerance);
    }

#endif

}

test_suite* MonteCarloMultithreadedTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Monte carlo multithreaded tests");

    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testHestonEngine));

}
