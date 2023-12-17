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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/types.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/integrals/momentbasedgaussianpolynomial.hpp>
#include <ql/math/integrals/gausslaguerrecosinepolynomial.hpp>
#include <ql/experimental/math/gaussiannoncentralchisquaredpolynomial.hpp>

#include <boost/math/distributions/non_central_chi_squared.hpp>

#ifndef TEST_BOOST_MULTIPRECISION_GAUSSIAN_QUADRATURE
//#define TEST_BOOST_MULTIPRECISION_GAUSSIAN_QUADRATURE
#endif

#ifdef TEST_BOOST_MULTIPRECISION_GAUSSIAN_QUADRATURE
    #if BOOST_VERSION < 105300
        #error This boost version is too old to support boost multi precision
    #endif

    #include <boost/multiprecision/cpp_dec_float.hpp>
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(GaussianQuadraturesTests)

template <class T>
void testSingle(const T& I, const std::string& tag,
                const boost::function<Real(Real)>& f, Real expected) {
    Real calculated = I(f);
    if (std::fabs(calculated-expected) > 1.0e-4) {
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
               [](Real x) -> Real { return 1.0; }, 2.0);
    testSingle(I, "f(x) = x",
               [](Real x) -> Real { return x; }, 0.0);
    testSingle(I, "f(x) = x^2",
               [](Real x) -> Real{ return x * x; }, 2/3.);
    testSingle(I, "f(x) = sin(x)",
               [](Real x) -> Real { return std::sin(x); }, 0.0);
    testSingle(I, "f(x) = cos(x)",
               [](Real x) -> Real { return std::cos(x); },
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

void testSingleTabulated(const boost::function<Real(Real)>& f,
                         const std::string& tag,
                         Real expected, Real tolerance) {
    const Size order[] = { 6, 7, 12, 20 };
    TabulatedGaussLegendre quad;
    for (unsigned long i : order) {
        quad.order(i);
        Real realised = quad(f);
        if (std::fabs(realised-expected) > tolerance) {
            BOOST_ERROR(" integrating " << tag << "\n"
                        << "    order " << i << "\n"
                        << "    realised: " << realised << "\n"
                        << "    expected: " << expected);
        }
    }
}

template <class mp_float>
class MomentBasedGaussLaguerrePolynomial
    : public MomentBasedGaussianPolynomial<mp_float> {
  public:
    mp_float moment(Size i) const override {
        if (i == 0)
            return mp_float(1.0);
        else
            return mp_float(i)*moment(i-1);
    }

    Real w(Real x) const override { return std::exp(-x); }
};


BOOST_AUTO_TEST_CASE(testJacobi) {
    BOOST_TEST_MESSAGE("Testing Gauss-Jacobi integration...");

    testSingleJacobi(GaussLegendreIntegration(16));
    testSingleJacobi(GaussChebyshevIntegration(130));
    testSingleJacobi(GaussChebyshev2ndIntegration(130));
    testSingleJacobi(GaussGegenbauerIntegration(50,0.55));
}

BOOST_AUTO_TEST_CASE(testLaguerre) {
     BOOST_TEST_MESSAGE("Testing Gauss-Laguerre integration...");

     testSingleLaguerre(GaussLaguerreIntegration(16));
     testSingleLaguerre(GaussLaguerreIntegration(150,0.01));

     testSingle(GaussLaguerreIntegration(16, 1.0), "f(x) = x*exp(-x)",
                x_inv_exp, 1.0);
     testSingle(GaussLaguerreIntegration(32, 0.9), "f(x) = x*exp(-x)",
                x_inv_exp, 1.0);
}

BOOST_AUTO_TEST_CASE(testHermite) {
     BOOST_TEST_MESSAGE("Testing Gauss-Hermite integration...");

     testSingle(GaussHermiteIntegration(16), "f(x) = Gaussian(x)",
                NormalDistribution(), 1.0);
     testSingle(GaussHermiteIntegration(16,0.5), "f(x) = x*Gaussian(x)",
                x_normaldistribution, 0.0);
     testSingle(GaussHermiteIntegration(64,0.9), "f(x) = x*x*Gaussian(x)",
                x_x_normaldistribution, 1.0);
}

BOOST_AUTO_TEST_CASE(testHyperbolic) {
     BOOST_TEST_MESSAGE("Testing Gauss hyperbolic integration...");

     testSingle(GaussHyperbolicIntegration(16), "f(x) = 1/cosh(x)",
                inv_cosh, M_PI);
     testSingle(GaussHyperbolicIntegration(16), "f(x) = x/cosh(x)",
                x_inv_cosh, 0.0);
}

BOOST_AUTO_TEST_CASE(testTabulated) {
     BOOST_TEST_MESSAGE("Testing tabulated Gauss-Laguerre integration...");

     testSingleTabulated([](Real x) -> Real { return x; }, "f(x) = x",
                         0.0,       1.0e-13);
     testSingleTabulated([](Real x) -> Real { return x * x; }, "f(x) = x^2",
                         (2.0/3.0), 1.0e-13);
     testSingleTabulated([](Real x) -> Real { return x * x * x; }, "f(x) = x^3",
                         0.0,       1.0e-13);
     testSingleTabulated([](Real x) -> Real { return x * x * x * x; }, "f(x) = x^4",
                         (2.0/5.0), 1.0e-13);
}

BOOST_AUTO_TEST_CASE(testMomentBasedGaussianPolynomial) {
     BOOST_TEST_MESSAGE("Testing moment-based Gaussian polynomials...");

     GaussLaguerrePolynomial g;

     std::vector<ext::shared_ptr<GaussianOrthogonalPolynomial> > ml;
     ml.push_back(
         ext::make_shared<MomentBasedGaussLaguerrePolynomial<Real> >());

#ifdef TEST_BOOST_MULTIPRECISION_GAUSSIAN_QUADRATURE
     ml.push_back(
         ext::make_shared<MomentBasedGaussLaguerrePolynomial<
             boost::multiprecision::number<
                 boost::multiprecision::cpp_dec_float<20> > > >());
#endif

     const Real tol = 1e-12;
     for (auto& k : ml) {

         for (Size i=0; i < 10; ++i) {
             const Real diffAlpha = std::fabs(k->alpha(i) - g.alpha(i));
             const Real diffBeta = std::fabs(k->beta(i) - g.beta(i));

             if (diffAlpha > tol) {
                 BOOST_ERROR("failed to reproduce alpha for Laguerre quadrature"
                             << "\n    calculated: " << k->alpha(i) << "\n    expected  : "
                             << g.alpha(i) << "\n    diff      : " << diffAlpha);
             }
             if (i > 0 && diffBeta > tol) {
                 BOOST_ERROR("failed to reproduce beta for Laguerre quadrature"
                             << "\n    calculated: " << k->beta(i) << "\n    expected  : "
                             << g.beta(i) << "\n    diff      : " << diffBeta);
             }
         }
     }
}

BOOST_AUTO_TEST_CASE(testGaussLaguerreCosinePolynomial) {
    BOOST_TEST_MESSAGE("Testing Gauss-Laguerre-Cosine quadrature...");

    const GaussianQuadrature quadCosine(
            16, GaussLaguerreCosinePolynomial<Real>(0.2));

    testSingle(quadCosine, "f(x) = exp(-x)",
               inv_exp, 1.0);
    testSingle(quadCosine, "f(x) = x*exp(-x)",
               x_inv_exp, 1.0);

    const GaussianQuadrature quadSine(
            16, GaussLaguerreSinePolynomial<Real>(0.2));

    testSingle(quadSine, "f(x) = exp(-x)",
               inv_exp, 1.0);
    testSingle(quadSine, "f(x) = x*exp(-x)",
               x_inv_exp, 1.0);
}

BOOST_AUTO_TEST_CASE(testNonCentralChiSquared) {
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

BOOST_AUTO_TEST_CASE(testNonCentralChiSquaredSumOfNodes) {
    BOOST_TEST_MESSAGE(
        "Testing Gauss non-central chi-squared sum of nodes...");

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

    const Real tol = 1e-5;

    for (Size n = 4; n < 10; ++n) {
         const Array x = GaussianQuadrature(n, orthPoly).x();
         const Real calculated = std::accumulate(x.begin(), x.end(), Real(0.0));


         if (std::fabs(calculated - expected[n-4]) > tol) {
             BOOST_ERROR("failed to reproduce rule of sum"
                         << "\n    calculated: " << calculated
                         << "\n    expected:   " << expected[n-4]
                         << "\n    diff    :   " << calculated - expected[n-4]);
         }
    }
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
