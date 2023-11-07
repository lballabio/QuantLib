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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/models/volatility/garch.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/randomnumbers/inversecumulativerng.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace garch_test {

    class DummyOptimizationMethod : public OptimizationMethod {
      public:
        EndCriteria::Type minimize(Problem& P, const EndCriteria& endCriteria) override {
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

    typedef InverseCumulativeRng<MersenneTwisterUniformRng,
                                 InverseCumulativeNormal>
        GaussianGenerator;

    static Real expected_calc[] = {
        0.452769, 0.513323, 0.530141, 0.5350841, 0.536558,
        0.536999, 0.537132, 0.537171, 0.537183, 0.537187
    };

    void check_ts(const std::pair<Date, Volatility> &x) {
        if (x.first.serialNumber() < 22835 || x.first.serialNumber() > 22844) {
            BOOST_ERROR("Failed to reproduce calculated GARCH time: "
                        << "\n    calculated: " << x.first.serialNumber()
                        << "\n    expected:   [22835, 22844]");
        }
        Real error =
            std::fabs(x.second - expected_calc[x.first.serialNumber()-22835]);
        if (error > 1.0e-6) {
            BOOST_ERROR("Failed to reproduce calculated GARCH value at "
                        << x.first.serialNumber() << ": "
                        << "\n    calculated: " << x.second
                        << "\n    expected:   "
                        << expected_calc[x.first.serialNumber()-22835]);
        }
    }
}

#define CHECK(results, garch, member) \
    if (std::fabs(results.member - garch.member()) > 1.0e-6) { \
        BOOST_ERROR("Failed to reproduce expected " #member \
                    << "\n    calculated: " << garch.member() \
                    << "\n    expected:   " << results.member); \
    }

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(GARCHTest)

BOOST_AUTO_TEST_CASE(testCalibration) {

    BOOST_TEST_MESSAGE("Testing GARCH model calibration...");

    using namespace garch_test;

    Date start(7, July, 1962), d = start;
    TimeSeries<Volatility> ts;
    Garch11 garch(0.2, 0.3, 0.4);
    GaussianGenerator rng(MersenneTwisterUniformRng(48));

    Volatility r = 0.0, v = 0.0;
    for (std::size_t i = 0; i < 50000; ++i, d += 1) {
        v = garch.forecast(r, v);
        r = rng.next().value * std::sqrt(v);
        ts[d] = r;
    }

    // Default calibration; works fine in most cases
    Garch11 cgarch1(ts);

    Results calibrated = { 0.207592, 0.281979, 0.204647, -0.0217413 };

    CHECK(calibrated, cgarch1, alpha);
    CHECK(calibrated, cgarch1, beta);
    CHECK(calibrated, cgarch1, omega);
    CHECK(calibrated, cgarch1, logLikelihood);

    // Type 1 initial guess - no further optimization
    Garch11 cgarch2(ts, Garch11::MomentMatchingGuess);
    DummyOptimizationMethod m;
    cgarch2.calibrate(ts, m, EndCriteria (3, 2, 0.0, 0.0, 0.0));
    Results expected1 = { 0.265749, 0.156956, 0.230964, -0.0227179 };

    CHECK(expected1, cgarch2, alpha);
    CHECK(expected1, cgarch2, beta);
    CHECK(expected1, cgarch2, omega);
    CHECK(expected1, cgarch2, logLikelihood);

    // Optimization from this initial guess
    cgarch2.calibrate(ts);

    CHECK(calibrated, cgarch2, alpha);
    CHECK(calibrated, cgarch2, beta);
    CHECK(calibrated, cgarch2, omega);
    CHECK(calibrated, cgarch2, logLikelihood);

    // Type 2 initial guess - no further optimization
    Garch11 cgarch3(ts, Garch11::GammaGuess);
    cgarch3.calibrate(ts, m, EndCriteria (3, 2, 0.0, 0.0, 0.0));
    Results expected2 = { 0.269896, 0.211373, 0.207534, -0.022798 };

    CHECK(expected2, cgarch3, alpha);
    CHECK(expected2, cgarch3, beta);
    CHECK(expected2, cgarch3, omega);
    CHECK(expected2, cgarch3, logLikelihood);

    // Optimization from this initial guess
    cgarch3.calibrate(ts);

    CHECK(calibrated, cgarch3, alpha);
    CHECK(calibrated, cgarch3, beta);
    CHECK(calibrated, cgarch3, omega);
    CHECK(calibrated, cgarch3, logLikelihood);

    // Double optimization using type 1 and 2 initial guesses
    Garch11 cgarch4(ts,  Garch11::DoubleOptimization);
    cgarch4.calibrate(ts);

    CHECK(calibrated, cgarch4, alpha);
    CHECK(calibrated, cgarch4, beta);
    CHECK(calibrated, cgarch4, omega);
    CHECK(calibrated, cgarch4, logLikelihood);

    // Alternative, gradient based optimization - usually gives worse
    // results than simplex
    LevenbergMarquardt lm;
    cgarch4.calibrate(ts, lm, EndCriteria (100000, 500, 1e-8, 1e-8, 1e-8));
    Results expected3 = { 0.265196, 0.277364, 0.678812, -0.216313 };

    CHECK(expected3, cgarch4, alpha);
    CHECK(expected3, cgarch4, beta);
    CHECK(expected3, cgarch4, omega);
    CHECK(expected3, cgarch4, logLikelihood);
}

BOOST_AUTO_TEST_CASE(testCalculation) {
    BOOST_TEST_MESSAGE("Testing GARCH model calculation...");

    using namespace garch_test;

    Date d(7, July, 1962);
    TimeSeries<Volatility> ts;
    Garch11 garch(0.2, 0.3, 0.4);

    Volatility r = 0.1;
    for (std::size_t i = 0; i < 10; ++i, d += 1) {
        ts[d] = r;
    }

    TimeSeries<Volatility> tsout = garch.calculate(ts);
    std::for_each(tsout.cbegin(), tsout.cend(), check_ts);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()