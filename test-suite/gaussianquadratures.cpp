/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "gaussianquadratures.hpp"
#include "utilities.hpp"
#include <ql/types.hpp>
#include <ql/Math/functional.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/Math/gaussianquadratures.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(GaussianQuadraturesTest)

Real tolerance = 1.0e-4;

template <class T, class F>
void testSingle(const T& I, const std::string& tag,
                const F& f, Real expected) {
    Real calculated = I(f);
    if (std::fabs(calculated-expected) > tolerance) {
        BOOST_FAIL("integrating\n"
                   << tag
                   << "    calculated: "
                   <<calculated
                   <<"    expected:   "
                   <<expected);
    }
}

// test functions

Real inv_exp(Real x) {
    return std::exp(-x);
}

Real x_inv_exp(Real x) {
    return x*std::exp(-x);
}

Real x_normaldistribution(Real x) {
    return x*NormalDistribution()(x);
}

Real x_x_normaldistribution(Real x) {
    return x*x*NormalDistribution()(x);
}

Real inv_cosh(Real x) {
    return 1/cosh(x);
}

Real x_inv_cosh(Real x) {
    return x/cosh(x);
}

template <class T>
void testJacobi(const T& I) {
    testSingle(I, "f(x) = 1",
               constant<Real,Real>(1.0), 2.0);
    testSingle(I, "f(x) = x",
               identity<Real>(),         0.0);
    testSingle(I, "f(x) = x^2",
               square<Real>(),           2/3.);
    testSingle(I, "f(x) = sin(x)",
               std::ptr_fun<Real,Real>(std::sin), 0.0);
    testSingle(I, "f(x) = cos(x)",
               std::ptr_fun<Real,Real>(std::cos),
               std::sin(1.0)-std::sin(-1.0));
    testSingle(I, "f(x) = Gaussian(x)",
               NormalDistribution(),
               CumulativeNormalDistribution()(1.0)
               -CumulativeNormalDistribution()(-1.0));
}

template <class T>
void testLaguerre(const T& I) {
    testSingle(I, "f(x) = exp(-x)",
               std::ptr_fun<Real,Real>(inv_exp), 1.0);
    testSingle(I, "f(x) = x*exp(-x)",
               std::ptr_fun<Real,Real>(x_inv_exp), 1.0);
    testSingle(I, "f(x) = Gaussian(x)",
               NormalDistribution(), 0.5);
}

QL_END_TEST_LOCALS(GaussianQuadraturesTest)


void GaussianQuadraturesTest::testJacobi() {
    BOOST_MESSAGE("Testing Gauss-Jacobi integration...");

    ::testJacobi(GaussLegendreIntegration(16));
    ::testJacobi(GaussChebyshevIntegration(130));
    ::testJacobi(GaussChebyshev2thIntegration(130));
    ::testJacobi(GaussGegenbauerIntegration(50,0.55));
}

void GaussianQuadraturesTest::testLaguerre() {
     BOOST_MESSAGE("Testing Gauss-Laguerre integration...");

     ::testLaguerre(GaussLaguerreIntegration(16));
     ::testLaguerre(GaussLaguerreIntegration(150,0.01));

     testSingle(GaussLaguerreIntegration(16, 1.0), "f(x) = x*exp(-x)",
                std::ptr_fun<Real,Real>(x_inv_exp), 1.0);
     testSingle(GaussLaguerreIntegration(32, 0.9), "f(x) = x*exp(-x)",
                std::ptr_fun<Real,Real>(x_inv_exp), 1.0);
}

void GaussianQuadraturesTest::testHermite() {
     BOOST_MESSAGE("Testing Gauss-Hermite integration...");

     testSingle(GaussHermiteIntegration(16), "f(x) = Gaussian(x)",
                NormalDistribution(), 1.0);
     testSingle(GaussHermiteIntegration(16,0.5), "f(x) = x*Gaussian(x)",
                std::ptr_fun<Real,Real>(x_normaldistribution), 0.0);
     testSingle(GaussHermiteIntegration(64,0.9), "f(x) = x*x*Gaussian(x)",
                std::ptr_fun<Real,Real>(x_x_normaldistribution), 1.0);
}

void GaussianQuadraturesTest::testHyperbolic() {
     BOOST_MESSAGE("Testing Gauss hyperbolic integration...");

     testSingle(GaussHyperbolicIntegration(16), "f(x) = 1/cosh(x)",
                std::ptr_fun<Real,Real>(inv_cosh), M_PI);
     testSingle(GaussHyperbolicIntegration(16), "f(x) = x/cosh(x)",
                std::ptr_fun<Real,Real>(x_inv_cosh), 0.0);
}

test_suite* GaussianQuadraturesTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Gaussian quadratures tests");
    suite->add(BOOST_TEST_CASE(&GaussianQuadraturesTest::testJacobi));
    suite->add(BOOST_TEST_CASE(&GaussianQuadraturesTest::testLaguerre));
    suite->add(BOOST_TEST_CASE(&GaussianQuadraturesTest::testHermite));
    suite->add(BOOST_TEST_CASE(&GaussianQuadraturesTest::testHyperbolic));
    return suite;
}

