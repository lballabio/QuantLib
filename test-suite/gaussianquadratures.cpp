/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2016 Klaus Spanderen

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

#include "gaussianquadratures.hpp"
#include "utilities.hpp"

#include <ql/types.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/experimental/math/gaussiannoncentralchisquaredpolynomial.hpp>

#include <boost/math/distributions/non_central_chi_squared.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    Real tolerance = 1.0e-4;

    template <class T, class F>
    void testSingle(const T& I, const std::string& tag,
                    const F& f, Real expected) {
        Real calculated = I(f);
        if (std::fabs(calculated-expected) > tolerance) {
            BOOST_ERROR("integrating" << tag << "\n"
                        << "    calculated: " << calculated << "\n"
                        << "    expected:   " << expected);
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
        return 1/std::cosh(x);
    }

    Real x_inv_cosh(Real x) {
        return x/std::cosh(x);
    }

    Real x_x_nonCentralChiSquared(Real x) {
        return x * x * boost::math::pdf(
            boost::math::non_central_chi_squared_distribution<Real>(4.0,1.0),x);
    }

    Real x_sin_exp_nonCentralChiSquared(Real x) {
        return x * std::sin(0.1*x) * std::exp(0.3*x) * boost::math::pdf(
            boost::math::non_central_chi_squared_distribution<Real>(1.0,1.0),x);
    }

    template <class T>
    void testSingleJacobi(const T& I) {
        testSingle(I, "f(x) = 1",
                   constant<Real,Real>(1.0), 2.0);
        testSingle(I, "f(x) = x",
                   identity<Real>(),         0.0);
        testSingle(I, "f(x) = x^2",
                   square<Real>(),           2/3.);
        testSingle(I, "f(x) = sin(x)",
                   static_cast<Real(*)(Real)>(std::sin), 0.0);
        testSingle(I, "f(x) = cos(x)",
                   static_cast<Real(*)(Real)>(std::cos),
                   std::sin(1.0)-std::sin(-1.0));
        testSingle(I, "f(x) = Gaussian(x)",
                   NormalDistribution(),
                   CumulativeNormalDistribution()(1.0)
                   -CumulativeNormalDistribution()(-1.0));
    }

    template <class T>
    void testSingleLaguerre(const T& I) {
        testSingle(I, "f(x) = exp(-x)",
                   inv_exp, 1.0);
        testSingle(I, "f(x) = x*exp(-x)",
                   x_inv_exp, 1.0);
        testSingle(I, "f(x) = Gaussian(x)",
                   NormalDistribution(), 0.5);
    }

    template <class F>
    void testSingleTabulated(const F& f, const std::string& tag,
                             Real expected, Real tolerance) {
        const Size order[] = { 6, 7, 12, 20 };
        TabulatedGaussLegendre quad;
        for (Size i=0; i<LENGTH(order); i++) {
            quad.order(order[i]);
            Real realised = quad(f);
            if (std::fabs(realised-expected) > tolerance) {
                BOOST_ERROR(" integrating " << tag << "\n"
                            << "    order " << order[i] << "\n"
                            << "    realised: " << realised << "\n"
                            << "    expected: " << expected);
            }
        }
    }

}


void GaussianQuadraturesTest::testJacobi() {
    BOOST_TEST_MESSAGE("Testing Gauss-Jacobi integration...");

    testSingleJacobi(GaussLegendreIntegration(16));
    testSingleJacobi(GaussChebyshevIntegration(130));
    testSingleJacobi(GaussChebyshev2ndIntegration(130));
    testSingleJacobi(GaussGegenbauerIntegration(50,0.55));
}

void GaussianQuadraturesTest::testLaguerre() {
     BOOST_TEST_MESSAGE("Testing Gauss-Laguerre integration...");

     testSingleLaguerre(GaussLaguerreIntegration(16));
     testSingleLaguerre(GaussLaguerreIntegration(150,0.01));

     testSingle(GaussLaguerreIntegration(16, 1.0), "f(x) = x*exp(-x)",
                x_inv_exp, 1.0);
     testSingle(GaussLaguerreIntegration(32, 0.9), "f(x) = x*exp(-x)",
                x_inv_exp, 1.0);
}

void GaussianQuadraturesTest::testHermite() {
     BOOST_TEST_MESSAGE("Testing Gauss-Hermite integration...");

     testSingle(GaussHermiteIntegration(16), "f(x) = Gaussian(x)",
                NormalDistribution(), 1.0);
     testSingle(GaussHermiteIntegration(16,0.5), "f(x) = x*Gaussian(x)",
                x_normaldistribution, 0.0);
     testSingle(GaussHermiteIntegration(64,0.9), "f(x) = x*x*Gaussian(x)",
                x_x_normaldistribution, 1.0);
}

void GaussianQuadraturesTest::testHyperbolic() {
     BOOST_TEST_MESSAGE("Testing Gauss hyperbolic integration...");

     testSingle(GaussHyperbolicIntegration(16), "f(x) = 1/cosh(x)",
                inv_cosh, M_PI);
     testSingle(GaussHyperbolicIntegration(16), "f(x) = x/cosh(x)",
                x_inv_cosh, 0.0);
}

void GaussianQuadraturesTest::testTabulated() {
     BOOST_TEST_MESSAGE("Testing tabulated Gauss-Laguerre integration...");

     testSingleTabulated(constant<Real,Real>(1.0), "f(x) = 1",
                         2.0,       1.0e-13);
     testSingleTabulated(identity<Real>(), "f(x) = x",
                         0.0,       1.0e-13);
     testSingleTabulated(square<Real>(), "f(x) = x^2",
                         (2.0/3.0), 1.0e-13);
     testSingleTabulated(cube<Real>(), "f(x) = x^3",
                         0.0,       1.0e-13);
     testSingleTabulated(fourth_power<Real>(), "f(x) = x^4",
                         (2.0/5.0), 1.0e-13);
}

void GaussianQuadraturesTest::testNonCentralChiSquared() {
     BOOST_TEST_MESSAGE(
         "Testing Gauss non-central chi-squared integration...");

     testSingle(
        GaussianQuadrature(2, GaussNonCentralChiSquaredPolynomial(4.0, 1.0)),
        "f(x) = x^2 * nonCentralChiSquared(4, 1)(x)",
        x_x_nonCentralChiSquared, 37.0);

     testSingle(
        GaussianQuadrature(14, GaussNonCentralChiSquaredPolynomial(1.0, 1.0)),
        "f(x) = x * sin(0.1*x)*exp(0.3*x)*nonCentralChiSquared(1, 1)(x)",
        x_sin_exp_nonCentralChiSquared, 17.408092);
}


void GaussianQuadraturesTest::testNonCentralChiSquaredSumOfNotes() {
     BOOST_TEST_MESSAGE(
         "Testing Gauss non-central chi-squared sum of notes...");

     // Walter Gautschi, How and How not to check Gaussian Quadrature Formulae
     // https://www.cs.purdue.edu/homes/wxg/selected_works/section_08/084.pdf

     // Expected results have been calculated with a multi precision library
     // following the description of test #4 in the paper above.
     // Using QuantLib's own determinant function will not work here
     // as it supports only double precision.

     const Real expected[] = {
         47.53491786730293,
         70.6103295419633383,
         98.0593406849441607,
         129.853401537905341,
         165.96963582663912,
         206.389183233992043
     };

     const Real nu=4.0;
     const Real lambda=1.0;
     const GaussNonCentralChiSquaredPolynomial orthPoly(nu, lambda);

#ifdef MULTIPRECISION_NON_CENTRAL_CHI_SQUARED_QUADRATURE
     const Real tol = 1e-12;
#else
     const Real tol = 1e-5;
#endif
	 for (Size n = 4; n < 10; ++n) {
		 const Array x = GaussianQuadrature(n, orthPoly).x();
         const Real calculated = std::accumulate(x.begin(), x.end(), 0.0);


         if (std::fabs(calculated - expected[n-4]) > tol) {
             BOOST_ERROR("failed to reproduce rule of sum"
                         << "\n    calculated: " << calculated
                         << "\n    expected:   " << expected[n-4]
                         << "\n    diff    :   " << calculated - expected[n-4]);
         }
     }
}


test_suite* GaussianQuadraturesTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Gaussian quadratures tests");
    suite->add(QUANTLIB_TEST_CASE(&GaussianQuadraturesTest::testJacobi));
    suite->add(QUANTLIB_TEST_CASE(&GaussianQuadraturesTest::testLaguerre));
    suite->add(QUANTLIB_TEST_CASE(&GaussianQuadraturesTest::testHermite));
    suite->add(QUANTLIB_TEST_CASE(&GaussianQuadraturesTest::testHyperbolic));
    suite->add(QUANTLIB_TEST_CASE(&GaussianQuadraturesTest::testTabulated));
    return suite;
}

test_suite* GaussianQuadraturesTest::experimental() {
    test_suite* suite = BOOST_TEST_SUITE(
        "Gaussian quadratures experimental tests");

    suite->add(QUANTLIB_TEST_CASE(
        &GaussianQuadraturesTest::testNonCentralChiSquared));
    suite->add(QUANTLIB_TEST_CASE(
        &GaussianQuadraturesTest::testNonCentralChiSquaredSumOfNotes));

    return suite;
}
