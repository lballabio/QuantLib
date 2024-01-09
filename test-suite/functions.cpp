/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2014 Klaus Spanderen
 Copyright (C) 2015 Johannes Göttker-Schnetmann

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
#include <ql/math/comparison.hpp>
#include <ql/math/factorial.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/math/modifiedbessel.hpp>
#include <ql/math/expm1.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::exp;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(FunctionsTests)

BOOST_AUTO_TEST_CASE(testFactorial) {

    BOOST_TEST_MESSAGE("Testing factorial numbers...");

    Real expected = 1.0;
    Real calculated = Factorial::get(0);
    if (calculated!=expected)
        BOOST_FAIL("Factorial(0) = " << calculated);

    for (Natural i=1; i<171; ++i) {
        expected *= i;
        calculated = Factorial::get(i);
        if (std::fabs(calculated-expected)/expected > 1.0e-9)
            BOOST_FAIL("Factorial(" << i << ")" <<
                       std::setprecision(16) << std::scientific <<
                       "\n calculated: " << calculated <<
                       "\n   expected: " << expected <<
                       "\n rel. error: " <<
                       std::fabs(calculated-expected)/expected);
    }
}

BOOST_AUTO_TEST_CASE(testGammaFunction) {

    BOOST_TEST_MESSAGE("Testing Gamma function...");

    Real expected = 0.0;
    Real calculated = GammaFunction().logValue(1);
    if (std::fabs(calculated) > 1.0e-15)
        BOOST_ERROR("GammaFunction(1)\n"
                    << std::setprecision(16) << std::scientific
                    << "    calculated: " << calculated << "\n"
                    << "    expected:   " << expected);

    for (Size i=2; i<9000; i++) {
        expected  += std::log(Real(i));
        calculated = GammaFunction().logValue(static_cast<Real>(i+1));
        if (std::fabs(calculated-expected)/expected > 1.0e-9)
            BOOST_ERROR("GammaFunction(" << i << ")\n"
                        << std::setprecision(16) << std::scientific
                        << "    calculated: " << calculated << "\n"
                        << "    expected:   " << expected << "\n"
                        << "    rel. error: "
                        << std::fabs(calculated-expected)/expected);
    }
}

BOOST_AUTO_TEST_CASE(testGammaValues) {

    BOOST_TEST_MESSAGE("Testing Gamma values...");

    // reference results are calculated with R
    Real tasks[][3] = {
            { 0.0001, 9999.422883231624, 1e3},
            { 1.2, 0.9181687423997607, 1e3},
            { 7.3, 1271.4236336639089586, 1e3},
            {-1.1, 9.7148063829028946, 1e3},
            {-4.001,-41.6040228304425312, 1e3},
            {-4.999, -8.347576090315059, 1e3},
            {-19.000001, 8.220610833201313e-12, 1e8},
            {-19.5, 5.811045977502255e-18, 1e3},
            {-21.000001, 1.957288098276488e-14, 1e8},
            {-21.5, 1.318444918321553e-20, 1e6}
    };

    for (auto& task : tasks) {
        const Real x = task[0];
        const Real expected = task[1];
        const Real calculated = GammaFunction().value(x);
        const Real tol = task[2] * QL_EPSILON * std::fabs(expected);

        if (std::fabs(calculated - expected) > tol) {
            BOOST_ERROR("GammaFunction(" << x << ")\n"
                        << std::setprecision(16) << std::scientific
                        << "    calculated: " << calculated << "\n"
                        << "    expected:   " << expected << "\n"
                        << "    rel. error: "
                        << std::fabs(calculated-expected)/expected);
        }
    }
}

BOOST_AUTO_TEST_CASE(testModifiedBesselFunctions) {
    BOOST_TEST_MESSAGE("Testing modified Bessel function of first and second kind...");

    /* reference values are computed with R and the additional package Bessel
     * http://cran.r-project.org/web/packages/Bessel
     */

    Real r[][4] = {
        {-1.3, 2.0, 1.2079888436539505, 0.1608243636110430},
        { 1.3, 2.0, 1.2908192151358788, 0.1608243636110430},
        { 0.001, 2.0, 2.2794705965773794, 0.1138938963603362},
        { 1.2, 0.5,   0.1768918783499572, 2.1086579232338192},
        { 2.3, 0.1, 0.00037954958988425198, 572.096866928290183},
        {-2.3, 1.1, 1.07222017902746969, 1.88152553684107371},
        {-10.0001, 1.1, 13857.7715614282552, 69288858.9474423379}
    };

    for (auto& i : r) {
        const Real nu = i[0];
        const Real x = i[1];
        const Real expected_i = i[2];
        const Real expected_k = i[3];
        const Real tol_i = 5e4 * QL_EPSILON*std::fabs(expected_i);
        const Real tol_k = 5e4 * QL_EPSILON*std::fabs(expected_k);

        const Real calculated_i = modifiedBesselFunction_i(nu, x);
        const Real calculated_k = modifiedBesselFunction_k(nu, x);

        if (std::fabs(expected_i - calculated_i) > tol_i) {
            BOOST_ERROR("failed to reproduce modified Bessel "
                       << "function of first kind"
                       << "\n order     : " << nu
                       << "\n argument  : " << x
                       << "\n calculated: " << calculated_i
                       << "\n expected  : " << expected_i);
        }
        if (std::fabs(expected_k - calculated_k) > tol_k) {
            BOOST_ERROR("failed to reproduce modified Bessel "
                       << "function of second kind"
                       << "\n order     : " << nu
                       << "\n argument  : " << x
                       << "\n calculated: " << calculated_k
                       << "\n expected  : " << expected_k);
        }
    }

    Real c[][7] = {
        {-1.3, 2.0, 0.0, 1.2079888436539505, 0.0,
                         0.1608243636110430, 0.0},
        { 1.2, 1.5, 0.3, 0.7891550871263575, 0.2721408731632123,
                         0.275126507673411, -0.1316314405663727},
        { 1.2, -1.5,0.0,-0.6650597524355781, -0.4831941938091643,
                        -0.251112360556051, -2.400130904230102},
        {-11.2, 1.5, 0.3,12780719.20252659, 16401053.26770633,
                        -34155172.65672453, -43830147.36759921},
        { 1.2, -1.5,2.0,-0.3869803778520574, 0.9756701796853728,
                        -3.111629716783005, 0.6307859871879062},
        { 1.2, 0.0, 9.9999,-0.03507838078252647, 0.1079601550451466,
                        -0.05979939995451453, 0.3929814473878203},
        { 1.2, 0.0, 10.1, -0.02782046891519293, 0.08562259917678558,
                        -0.02035685034691133, 0.3949834389686676},
        { 1.2, 0.0, 12.1, 0.07092110620741207, -0.2182727210128104,
                        0.3368505862966958, -0.1299038064313366},
        { 1.2, 0.0, 14.1,-0.03014378676768797, 0.09277303628303372,
                        -0.237531022649052, -0.2351923034581644},
        { 1.2, 0.0, 16.1,-0.03823210284792657, 0.1176663135266562,
                        -0.1091239402448228, 0.2930535651966139},
        { 1.2, 0.0, 18.1,0.05626742394733754, -0.173173324361983,
                        0.2941636588154642, -0.02023355577954348},
        { 1.2, 0.0, 180.1,-0.001230682086826484, 0.003787649998122361,
                        0.02284509628723454, 0.09055419580980778},
        { 1.2, 0.0, 21.0,-0.04746415965014021, 0.1460796627610969,
                        -0.2693825171336859, -0.04830804448126782},
        { 1.2, 10.0, 0.0, 2609.784936867044, 0, 1.904394919838336e-05, 0},
        { 1.2, 14.0, 0.0, 122690.4873454286, 0, 2.902060692576643e-07, 0},
        { 1.2, 20.0, 10.0, -37452017.91168936, -13917587.22151363,
                        -3.821534367487143e-10, 4.083211255351664e-10},
        { 1.2, 9.0, 9.0, -621.7335051293694,  618.1455736670332,
                        -4.480795479964915e-05, -3.489034389148745e-08}
    };

    for (auto& i : c) {
        const Real nu = i[0];
        const std::complex<Real> z = std::complex<Real>(i[1], i[2]);
        const std::complex<Real> expected_i = std::complex<Real>(i[3], i[4]);
        const std::complex<Real> expected_k = std::complex<Real>(i[5], i[6]);

        const Real tol_i = 5e4*QL_EPSILON*std::abs(expected_i);
        const Real tol_k = 1e6*QL_EPSILON*std::abs(expected_k);

        const std::complex<Real> calculated_i=modifiedBesselFunction_i(nu, z);
        const std::complex<Real> calculated_k=modifiedBesselFunction_k(nu, z);

        if (std::abs(expected_i - calculated_i) > tol_i) {
            BOOST_ERROR("failed to reproduce modified Bessel "
                       << "function of first kind"
                       << "\n order     : " << nu
                       << "\n argument  : " << z
                       << "\n calculated: " << calculated_i
                       << "\n expected  : " << expected_i);
        }
        if (   std::abs(expected_k) > 1e-4 // do not check small values
            && std::abs(expected_k - calculated_k) > tol_k) {
            BOOST_ERROR("failed to reproduce modified Bessel "
                       << "function of second kind"
                       << "\n order     : " << nu
                       << "\n argument  : " << z
                       << "\n diff      : " << calculated_k-expected_k
                       << "\n calculated: " << calculated_k
                       << "\n expected  : " << expected_k);
        }
    }
}

BOOST_AUTO_TEST_CASE(testWeightedModifiedBesselFunctions) {
    BOOST_TEST_MESSAGE("Testing weighted modified Bessel functions...");
    for (Real nu = -5.0; nu <= 5.0; nu += 0.5) {
        for (Real x = 0.1; x <= 15.0; x += 0.5) {
            Real calculated_i = modifiedBesselFunction_i_exponentiallyWeighted(nu, x);
            Real expected_i = modifiedBesselFunction_i(nu, x) * exp(-x);
            Real calculated_k = modifiedBesselFunction_k_exponentiallyWeighted(nu, x);
            Real expected_k =
                M_PI_2 * (modifiedBesselFunction_i(-nu, x) - modifiedBesselFunction_i(nu, x)) *
                exp(-x) / std::sin(M_PI * nu);
            Real tol_i = 1e3 * QL_EPSILON * std::fabs(expected_i) * std::max(exp(x), 1.0);
            Real tol_k = std::max(QL_EPSILON, 1e3 * QL_EPSILON * std::fabs(expected_k) *
                                                        std::max(exp(x), 1.0));
            if (std::abs(expected_i - calculated_i) > tol_i) {
                BOOST_ERROR("failed to verify exponentially weighted"
                            << "modified Bessel function of first kind"
                            << "\n order      : " << nu << "\n argument   : " << x
                            << "\n calculated  : " << calculated_i << "\n expected   : "
                            << expected_i << "\n difference : " << (expected_i - calculated_i));
            }
            if (std::abs(expected_k - calculated_k) > tol_k) {
                BOOST_ERROR("failed to verify exponentially weighted"
                            << "modified Bessel function of second kind"
                            << "\n order      : " << nu << "\n argument   : " << x
                            << "\n calculated  : " << calculated_k << "\n expected   : "
                            << expected_k << "\n difference : " << (expected_k - calculated_k));
            }
        }
    }
    for (Real nu = -5.0; nu <= 5.0; nu += 0.5) {
        for (Real x = -5.0; x <= 5.0; x += 0.5) {
            for (Real y = -5.0; y <= 5.0; y += 0.5) {
                std::complex<Real> z(x, y);
                std::complex<Real> calculated_i =
                    modifiedBesselFunction_i_exponentiallyWeighted(nu, z);
                std::complex<Real> expected_i = modifiedBesselFunction_i(nu, z) * exp(-z);
                std::complex<Real> calculated_k =
                    modifiedBesselFunction_k_exponentiallyWeighted(nu, z);
                std::complex<Real> expected_k = M_PI_2 *
                                                      (modifiedBesselFunction_i(-nu, z) * exp(-z) -
                                                       modifiedBesselFunction_i(nu, z) * exp(-z)) /
                                                      std::sin(M_PI * nu);
                Real tol_i = 1e3 * QL_EPSILON * std::abs(calculated_i);
                Real tol_k = 1e3 * QL_EPSILON * std::abs(calculated_k);
                if (std::abs(calculated_i - expected_i) > tol_i) {
                    BOOST_ERROR("failed to verify exponentially weighted"
                                << "modified Bessel function of first kind"
                                << "\n order      : " << nu << "\n argument   : " << x
                                << "\n calculated  : " << calculated_i << "\n expected   : "
                                << expected_i << "\n difference : " << (expected_i - calculated_i));
                }
                if (std::abs(expected_k - calculated_k) > tol_k) {
                    BOOST_ERROR("failed to verify exponentially weighted"
                                << "modified Bessel function of second kind"
                                << "\n order      : " << nu << "\n argument   : " << x
                                << "\n calculated  : " << calculated_k << "\n expected   : "
                                << expected_k << "\n difference : " << (expected_k - calculated_k));
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testExpm1) {
    BOOST_TEST_MESSAGE("Testing complex valued expm1...");

    const std::complex<Real> z = std::complex<Real>(1.2, 0.5);
    QL_CHECK_SMALL(std::abs(std::exp(z) - 1.0 - expm1(z)), 10*QL_EPSILON);

    const std::complex<Real> calculated = expm1(std::complex<Real>(5e-6, 5e-5));
    //scipy reference value
    const std::complex<Real> expected(4.998762493771078e-06,5.000024997979157e-05);
    const Real tol = std::max(2.2e-14, 100*QL_EPSILON);
    QL_CHECK_CLOSE_FRACTION(calculated.real(), expected.real(), tol);
    QL_CHECK_CLOSE_FRACTION(calculated.imag(), expected.imag(), tol);
}

BOOST_AUTO_TEST_CASE(testLog1p) {
    BOOST_TEST_MESSAGE("Testing complex valued log1p...");

    const std::complex<Real> z = std::complex<Real>(1.2, 0.57);
    QL_CHECK_SMALL(std::abs(std::log(1.0+z) - log1p(z)), 10*QL_EPSILON);

    const std::complex<Real> calculated = log1p(std::complex<Real>(5e-6, 5e-5));
    //scipy reference value
    const std::complex<Real> expected(5.0012374875401984e-06, 4.999974995958395e-05);
    const Real tol = std::max(2.2e-14, 100*QL_EPSILON);
    QL_CHECK_CLOSE_FRACTION(calculated.real(), expected.real(), tol);
    QL_CHECK_CLOSE_FRACTION(calculated.imag(), expected.imag(), tol);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
