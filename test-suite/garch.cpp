/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Liquidnet Holdings, Inc.

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

#include "garch.hpp"
#include "utilities.hpp"
#include <ql/models/volatility/garch.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <boost/random.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    class DummyOptimizationMethod : public OptimizationMethod {
      public:
        virtual EndCriteria::Type minimize(Problem& P,
                                           const EndCriteria& endCriteria) {
            P.setFunctionValue(P.value(P.currentValue()));
            return EndCriteria::None;
        }
    };

    struct Results {
        Real alpha;
        Real beta;
        Real omega;
        Real logLikelihood;
    };

}

#define CHECK(results, garch, member, tolerance) \
    if (std::fabs(results.member - garch.member()) > tolerance) { \
        BOOST_ERROR("Failed to reproduce expected " #member \
                    << "\n    calculated: " << garch.member() \
                    << "\n    expected:   " << results.member); \
    }

void GARCHTest::testCalibration() {

    BOOST_MESSAGE("Testing GARCH model calibration ...");

    typedef boost::variate_generator<boost::minstd_rand,
                                     boost::normal_distribution<Real> >
        GaussianGenerator;

    Date start(7, July, 1962), d = start;
    Calendar calendar = TARGET();
    TimeSeries<Volatility> ts;
    Garch11 garch(0.2, 0.3, 0.4);
    GaussianGenerator g(boost::minstd_rand(48),
                        boost::normal_distribution<Real>(0.0, 1.0));

    Volatility r = 0.0, v = 0.0;
    for (std::size_t i = 0; i < 50000; ++i, d = calendar.advance(d, 1, Days)) {
        v = garch.forecast(r, v);
        r = g() * std::sqrt(v);
        ts[d] = r;
    }

    // Default calibration; works fine in most cases
    Garch11 cgarch1(ts);
    Real f1 = cgarch1.logLikelihood();
    Real f2 = -cgarch1.costFunction(ts.cbegin_values(), ts.cend_values(),
                                    garch.alpha(), garch.beta(), garch.omega());

    Results calibrated = { 0.193890, 0.290606, 0.204217, -0.018323 };
    Real tolerance = 1e-6;

    CHECK(calibrated, cgarch1, alpha, tolerance);
    CHECK(calibrated, cgarch1, beta, tolerance);
    CHECK(calibrated, cgarch1, omega, tolerance);
    CHECK(calibrated, cgarch1, logLikelihood, tolerance);

    // Type 1 initial guess - no further optimization
    Garch11 cgarch2(ts, Garch11::MomentMatchingGuess);
    DummyOptimizationMethod m;
    cgarch2.calibrate(ts, m, EndCriteria (3, 2, 0.0, 0.0, 0.0));
    Results expected1 = { 0.325500, 0.293661, 0.150953, -0.025423 };

    CHECK(expected1, cgarch2, alpha, tolerance);
    CHECK(expected1, cgarch2, beta, tolerance);
    CHECK(expected1, cgarch2, omega, tolerance);
    CHECK(expected1, cgarch2, logLikelihood, tolerance);

    // Optimization from this initial guess
    cgarch2.calibrate(ts);

    CHECK(calibrated, cgarch2, alpha, tolerance);
    CHECK(calibrated, cgarch2, beta, tolerance);
    CHECK(calibrated, cgarch2, omega, tolerance);
    CHECK(calibrated, cgarch2, logLikelihood, tolerance);

    // Type 2 initial guess - no further optimization
    Garch11 cgarch3(ts, Garch11::GammaGuess);
    cgarch3.calibrate(ts, m, EndCriteria (3, 2, 0.0, 0.0, 0.0));
    Results expected2 = { 0.341533, 0.160356, 0.197436, -0.024357 };

    CHECK(expected2, cgarch3, alpha, tolerance);
    CHECK(expected2, cgarch3, beta, tolerance);
    CHECK(expected2, cgarch3, omega, tolerance);
    CHECK(expected2, cgarch3, logLikelihood, tolerance);

    // Optimization from this initial guess
    cgarch3.calibrate(ts);

    CHECK(calibrated, cgarch3, alpha, tolerance);
    CHECK(calibrated, cgarch3, beta, tolerance);
    CHECK(calibrated, cgarch3, omega, tolerance);
    CHECK(calibrated, cgarch3, logLikelihood, tolerance);

    // Double optimization using type 1 and 2 initial guesses
    Garch11 cgarch4(ts,  Garch11::DoubleOptimization);
    cgarch4.calibrate(ts);

    CHECK(calibrated, cgarch4, alpha, tolerance);
    CHECK(calibrated, cgarch4, beta, tolerance);
    CHECK(calibrated, cgarch4, omega, tolerance);
    CHECK(calibrated, cgarch4, logLikelihood, tolerance);

    // Alternative, gradient based optimization - usually gives worse
    // results than simplex
    LevenbergMarquardt lm;
    cgarch4.calibrate(ts, lm, EndCriteria (100000, 500, 1e-8, 1e-8, 1e-8));
    Results expected3 = { 0.253443, 0.267758, 0.687105, -0.21211 };

    CHECK(expected3, cgarch4, alpha, tolerance);
    CHECK(expected3, cgarch4, beta, tolerance);
    CHECK(expected3, cgarch4, omega, tolerance);
    CHECK(expected3, cgarch4, logLikelihood, tolerance);
}

test_suite* GARCHTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("GARCH model tests");
    suite->add(QUANTLIB_TEST_CASE(&GARCHTest::testCalibration));
    return suite;
}
