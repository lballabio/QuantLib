/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Peter Caspers
 Copyright (C) 2013 Klaus Spanderen

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

#include "ode.hpp"
#include "utilities.hpp"
#include <ql/experimental/math/expm.hpp>
#include <ql/experimental/math/adaptiverungekutta.hpp>
#include <complex>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct ode1 {
        Real operator()(Real x, Real y) const { return y; }
    };

    struct ode2 {
        std::complex<Real> operator()(Real x,
                                      std::complex<Real> y) {
            return std::complex<Real>(0.0,1.0)*y;
        }
    };

    struct ode3 {
        Disposable<std::vector<Real> > operator()(Real x,
                                                  const std::vector<Real>& y) {
            std::vector<Real> r(2);
            r[0] = y[1]; r[1] = -y[0];
            return r;
        }
    };

    struct ode4 {
        Disposable<std::vector<std::complex<Real> > > operator()(
                                  std::complex<Real> x,
                                  const std::vector<std::complex<Real> >& y) {
            std::vector<std::complex<Real> > r(2);
            r[0] = y[1]; r[1] = -y[0];
            return r;
        }
    };

}

void OdeTest::testAdaptiveRungeKutta() {

    BOOST_TEST_MESSAGE("Testing adaptive Runge Kutta...");

    AdaptiveRungeKutta<Real> rk_real(1E-12,1E-4,0.0);
    AdaptiveRungeKutta<std::complex<Real> > rk_complex(1E-12,1E-4,0.0);
    Real tol1 = 5E-10, tol2 = 2E-12, tol3 = 2E-12, tol4 = 2E-12;

    // f'=f, f(0)=1
    AdaptiveRungeKutta<Real>::OdeFct1d ode1_ = ode1();
    Real y10=1;

    // f'=f, f(0)=i
    AdaptiveRungeKutta<std::complex<Real> >::OdeFct1d ode2_ = ode2();
    std::complex<Real> y20(0.0,1.0);

    // f''=-f, f(0)=0, f'(0)=1
    AdaptiveRungeKutta<Real>::OdeFct ode3_ = ode3();
    std::vector<Real> y30(2); y30[0] = 0.0; y30[1] = 1.0;

    // f''=-f, f(0)=1, f'(0)=i
    AdaptiveRungeKutta<std::complex<Real> >::OdeFct ode4_ = ode4();
    std::vector<std::complex<Real> > y40(2);
    y40[0] = 1.0;
    y40[1] = std::complex<Real>(0.0,1.0);

    Real x=0.0;
    Real y1 = y10;
    std::complex<Real> y2 = y20;
    std::vector<Real> y3 = y30;
    std::vector<std::complex<Real> > y4 = y40;

    while (x<5.0) {
        Real exact1 = exp(x);
        std::complex<Real> exact2 =
            std::exp(std::complex<Real>(0.0,x)) * std::complex<Real>(0.0,1.0);
        Real exact3 = sin(x);
        std::complex<Real> exact4 = std::exp(std::complex<Real>(0.0,x));

        if ( std::fabs( exact1 - y1 ) > tol1 )
            BOOST_FAIL("Error in ode #1: exact solution at x=" << x
                       << " is " << exact1
                       << ", numerical solution is " << y1
                       << " difference " << std::fabs(exact1-y1)
                       << " outside tolerance " << tol1);
        if ( abs( exact2 - y2 ) > tol2 )
            BOOST_FAIL("Error in ode #2: exact solution at x=" << x
                       << " is " << exact2
                       << ", numerical solution is " << y2
                       << " difference " << abs(exact2-y2)
                       << " outside tolerance " << tol2);
        if ( std::fabs( exact3 - y3[0] ) > tol3 )
            BOOST_FAIL("Error in ode #3: exact solution at x=" << x
                       << " is " << exact3
                       << ", numerical solution is " << y3[0]
                       << " difference " << std::fabs(exact3-y3[0])
                       << " outside tolerance " << tol3);
        if ( abs( exact4 - y4[0] ) > tol4 )
            BOOST_FAIL("Error in ode #4: exact solution at x=" << x
                       << " is " << exact4
                       << ", numerical solution is " << y4[0]
                       << " difference " << abs(exact4-y4[0])
                       << " outside tolerance " << tol4);
        x+=0.01;
        y1=rk_real(ode1_,y10,0.0,x);
        y2=rk_complex(ode2_,y20,0.0,x);
        y3=rk_real(ode3_,y30,0.0,x);
        y4=rk_complex(ode4_,y40,0.0,x);
    }
}

namespace {
    Real frobenuiusNorm(const Matrix& m) {
        return std::sqrt(DotProduct((m*transpose(m)).diagonal(),
                                    Array(m.rows(), 1.0)));
    }
}

void OdeTest::testMatrixExponential() {
    BOOST_TEST_MESSAGE("Testing matrix exponential based on ode...");

    // Reference results are taken from
    // http://www.millersville.edu/~bikenaga/linear-algebra/matrix-exponential/matrix-exponential.html

    Matrix m(3, 3);
    m[0][0] = 5; m[0][1] =-6; m[0][2] =-6;
    m[1][0] =-1; m[1][1] = 4; m[1][2] = 2;
    m[2][0] = 3; m[2][1] =-6; m[2][2] =-4;

    const Real tol = 1e-12;

    for (Real t=0.01; t < 11; t+=t) {
        const Matrix calculated = Expm(m, t, tol);

        Matrix expected(3, 3);
        expected[0][0] = -3*std::exp(t)+4*std::exp(2*t);
        expected[0][1] =  6*std::exp(t)-6*std::exp(2*t);
        expected[0][2] =  6*std::exp(t)-6*std::exp(2*t);
        expected[1][0] =    std::exp(t)-  std::exp(2*t);
        expected[1][1] = -2*std::exp(t)+3*std::exp(2*t);
        expected[1][2] = -2*std::exp(t)+2*std::exp(2*t);
        expected[2][0] = -3*std::exp(t)+3*std::exp(2*t);
        expected[2][1] =  6*std::exp(t)-6*std::exp(2*t);
        expected[2][2] =  6*std::exp(t)-5*std::exp(2*t);

        Matrix diff = calculated - expected;
        Real relDiffNorm = frobenuiusNorm(diff)/frobenuiusNorm(expected);

        if ( std::fabs(relDiffNorm) > 100*tol) {
            BOOST_FAIL("Failed to reproduce expected matrix exponential."
                    << "\n rel. difference norm: " << relDiffNorm
                    << "\n tolerance           : " << 100*tol);
        }

        const Matrix negativeTime = Expm((-1)*m, -t, tol);
        diff = negativeTime - expected;
        relDiffNorm = frobenuiusNorm(diff)/frobenuiusNorm(expected);

        if ( std::fabs(relDiffNorm) > 100*tol) {
            BOOST_FAIL("Failed to reproduce expected matrix exponential."
                    << "\n rel. difference norm: " << relDiffNorm
                    << "\n tolerance           : " << 100*tol);
        }

    }
}

void OdeTest::testMatrixExponentialOfZero() {
    BOOST_TEST_MESSAGE("Testing matrix exponential of a zero matrix "
                       "based on ode...");

    Matrix m(3, 3, 0.0);

    const Real tol = 100*QL_EPSILON;
    const Time t=1.0;
    const Matrix calculated = Expm(m, t);

    for (Size i=0; i < calculated.rows(); ++i) {
        for (Size j=0; j < calculated.columns(); ++j) {
            const Real kroneckerDelta = (i==j)? 1.0 : 0.0;
            if (std::fabs(calculated[i][j] -kroneckerDelta) > tol) {
                BOOST_FAIL("Failed to reproduce expected matrix exponential."
                        << "\n tolerance           : " << tol);
            }
        }
    }
}

test_suite* OdeTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("ode tests");
    suite->add(QUANTLIB_TEST_CASE(&OdeTest::testAdaptiveRungeKutta));
    suite->add(QUANTLIB_TEST_CASE(&OdeTest::testMatrixExponential));
    suite->add(QUANTLIB_TEST_CASE(&OdeTest::testMatrixExponentialOfZero));
    return suite;
}
