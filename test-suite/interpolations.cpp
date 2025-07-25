/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2005, 2006 StatPro Italia srl
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2009 Dimitri Reiswich
 Copyright (C) 2014 Peter Caspers
 Copyright (C) 2018 Klaus Spanderen

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

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/volatility/noarbsabrinterpolation.hpp>
#include <ql/experimental/math/laplaceinterpolation.hpp>
#include <ql/math/bspline.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/math/interpolations/chebyshevinterpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/forwardflatinterpolation.hpp>
#include <ql/math/interpolations/kernelinterpolation.hpp>
#include <ql/math/interpolations/kernelinterpolation2d.hpp>
#include <ql/math/interpolations/lagrangeinterpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/multicubicspline.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <ql/math/kernelfunctions.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <ql/math/richardsonextrapolation.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/utilities/null.hpp>
#include <cmath>
#include <utility>
#include <tuple>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(InterpolationTests)

std::vector<Real> xRange(Real start, Real finish, Size points) {
    std::vector<Real> x(points);
    Real dx = (finish-start)/(points-1);
    for (Size i=0; i<points-1; i++)
        x[i] = start+i*dx;
    x[points-1] = finish;
    return x;
}

std::vector<Real> gaussian(const std::vector<Real>& x) {
    std::vector<Real> y(x.size());
    for (Size i=0; i<x.size(); i++)
        y[i] = std::exp(-x[i]*x[i]);
    return y;
}

std::vector<Real> parabolic(const std::vector<Real>& x) {
    std::vector<Real> y(x.size());
    for (Size i=0; i<x.size(); i++)
        y[i] = -x[i]*x[i];
    return y;
}

template <class I, class J>
void checkValues(const char* type,
                 const CubicInterpolation& cubic,
                 I xBegin, I xEnd, J yBegin) {
    Real tolerance = 2.0e-15;
    while (xBegin != xEnd) {
        Real interpolated = cubic(*xBegin);
        if (std::fabs(interpolated-*yBegin) > tolerance) {
            BOOST_ERROR(type << " interpolation failed at x = " << *xBegin
                        << std::scientific
                        << "\n    interpolated value: " << interpolated
                        << "\n    expected value:     " << *yBegin
                        << "\n    error:              "
                        << std::fabs(interpolated-*yBegin));
        }
        ++xBegin; ++yBegin;
    }
}

void check1stDerivativeValue(const char* type,
                             const CubicInterpolation& cubic,
                             Real x,
                             Real value) {
    Real tolerance = 1.0e-14;
    Real interpolated = cubic.derivative(x);
    Real error = std::fabs(interpolated-value);
    if (error > tolerance) {
        BOOST_ERROR(type << " interpolation first derivative failure\n"
                    << "at x = " << x
                    << "\n    interpolated value: " << interpolated
                    << "\n    expected value:     " << value
                    << std::scientific
                    << "\n    error:              " << error);
    }
}

void check2ndDerivativeValue(const char* type,
                             const CubicInterpolation& cubic,
                             Real x,
                             Real value) {
    Real tolerance = 1.0e-13;
    Real interpolated = cubic.secondDerivative(x);
    Real error = std::fabs(interpolated-value);
    if (error > tolerance) {
        BOOST_ERROR(type << " interpolation second derivative failure\n"
                    << "at x = " << x
                    << "\n    interpolated value: " << interpolated
                    << "\n    expected value:     " << value
                    << std::scientific
                    << "\n    error:              " << error);
    }
}

void checkNotAKnotCondition(const char* type,
                            const CubicInterpolation& cubic) {
    Real tolerance = 1.0e-14;
    const std::vector<Real>& c = cubic.cCoefficients();
    if (std::fabs(c[0]-c[1]) > tolerance) {
        BOOST_ERROR(type << " interpolation failure"
                    << "\n    cubic coefficient of the first"
                    << " polinomial is " << c[0]
                    << "\n    cubic coefficient of the second"
                    << " polinomial is " << c[1]);
    }
    Size n = c.size();
    if (std::fabs(c[n-2]-c[n-1]) > tolerance) {
        BOOST_ERROR(type << " interpolation failure"
                    << "\n    cubic coefficient of the 2nd to last"
                    << " polinomial is " << c[n-2]
                    << "\n    cubic coefficient of the last"
                    << " polinomial is " << c[n-1]);
    }
}

void checkSymmetry(const char* type,
                   const CubicInterpolation& cubic,
                   Real xMin) {
    Real tolerance = 1.0e-15;
    for (Real x = xMin; x < 0.0; x += 0.1) {
        Real y1 = cubic(x), y2 = cubic(-x);
        if (std::fabs(y1-y2) > tolerance) {
            BOOST_ERROR(type << " interpolation not symmetric"
                        << "\n    x = " << x
                        << "\n    g(x)  = " << y1
                        << "\n    g(-x) = " << y2
                        << "\n    error:  " << std::fabs(y1-y2));
        }
    }
}

template <class F>
class errorFunction {
  public:
    errorFunction(F f) : f_(std::move(f)) {}
    Real operator()(Real x) const {
        Real temp = f_(x)-std::exp(-x*x);
        return temp*temp;
    }
  private:
    F f_;
};

template <class F>
errorFunction<F> make_error_function(const F& f) {
    return errorFunction<F>(f);
}

Real multif(Real s, Real t, Real u, Real v, Real w) {
    return std::sqrt(s * std::sinh(std::log(t)) +
                     std::exp(std::sin(u) * std::sin(3 * v)) +
                     std::sinh(std::log(v * w)));
}

Real epanechnikovKernel(Real u){

    if(std::fabs(u)<=1){
        return (3.0/4.0)*(1-u*u);
    }else{
        return 0.0;
    }
}

struct NotThrown {};

Integer sign(Real y1, Real y2) {
    return y1 == y2 ? 0 :
           y1 < y2 ?  1 :
                     -1 ;
}

class GF {
  public:
    GF(Real exponent, Real factor)
    : exponent_(exponent), factor_(factor) {}

    Real operator()(Real h) const {
        return M_PI + factor_*std::pow(h, exponent_)
            + std::pow(factor_*h, exponent_ + 1);
    }
  private:
    const Real exponent_, factor_;
};

Real limCos(Real h) {
    return -std::cos(h);
}

Real f(Real h) {
    return std::pow( 1.0 + h, 1/h);
}

Real lagrangeTestFct(Real x) {
    return std::fabs(x) + 0.5*x - x*x;
}


/* See J. M. Hyman, "Accurate monotonicity preserving cubic interpolation"
   SIAM J. of Scientific and Statistical Computing, v. 4, 1983, pp. 645-654.
   http://math.lanl.gov/~mac/papers/numerics/H83.pdf
*/

BOOST_AUTO_TEST_CASE(testSplineErrorOnGaussianValues) {

    BOOST_TEST_MESSAGE("Testing spline approximation on Gaussian data sets...");

    Size points[]                = {      5,      9,     17,     33 };

    // complete spline data from the original 1983 Hyman paper
    Real tabulatedErrors[]     = { 3.5e-2, 2.0e-3, 4.0e-5, 1.8e-6 };
    Real toleranceOnTabErr[]   = { 0.1e-2, 0.1e-3, 0.1e-5, 0.1e-6 };

    // (complete) MC spline data from the original 1983 Hyman paper
    // NB: with the improved Hyman filter from the Dougherty, Edelman, and
    //     Hyman 1989 paper the n=17 nonmonotonicity is not filtered anymore
    //     so the error agrees with the non MC method.
    Real tabulatedMCErrors[]   = { 1.7e-2, 2.0e-3, 4.0e-5, 1.8e-6 };
    Real toleranceOnTabMCErr[] = { 0.1e-2, 0.1e-3, 0.1e-5, 0.1e-6 };

    SimpsonIntegral integral(1e-12, 10000);
    std::vector<Real> x, y;

    // still unexplained scale factor needed to obtain the numerical
    // results from the paper
    Real scaleFactor = 1.9;

    for (Size i=0; i<std::size(points); i++) {
        Size n = points[i];
        std::vector<Real> x = xRange(-1.7, 1.9, n);
        std::vector<Real> y = gaussian(x);

        // Not-a-knot
        CubicInterpolation f(x.begin(), x.end(), y.begin(),
                             CubicInterpolation::Spline, false,
                             CubicInterpolation::NotAKnot, Null<Real>(),
                             CubicInterpolation::NotAKnot, Null<Real>());
        f.update();
        Real result = std::sqrt(integral(make_error_function(f), -1.7, 1.9));
        result /= scaleFactor;
        if (std::fabs(result-tabulatedErrors[i]) > toleranceOnTabErr[i])
            BOOST_ERROR("Not-a-knot spline interpolation "
                        << "\n    sample points:      " << n
                        << "\n    norm of difference: " << result
                        << "\n    it should be:       " << tabulatedErrors[i]);

        // MC not-a-knot
        f = CubicInterpolation(x.begin(), x.end(), y.begin(),
                               CubicInterpolation::Spline, true,
                               CubicInterpolation::NotAKnot, Null<Real>(),
                               CubicInterpolation::NotAKnot, Null<Real>());
        f.update();
        result = std::sqrt(integral(make_error_function(f), -1.7, 1.9));
        result /= scaleFactor;
        if (std::fabs(result-tabulatedMCErrors[i]) > toleranceOnTabMCErr[i])
            BOOST_ERROR("MC Not-a-knot spline interpolation "
                        << "\n    sample points:      " << n
                        << "\n    norm of difference: " << result
                        << "\n    it should be:       "
                        << tabulatedMCErrors[i]);
    }

}

/* See J. M. Hyman, "Accurate monotonicity preserving cubic interpolation"
   SIAM J. of Scientific and Statistical Computing, v. 4, 1983, pp. 645-654.
   http://math.lanl.gov/~mac/papers/numerics/H83.pdf
*/

BOOST_AUTO_TEST_CASE(testSplineOnGaussianValues) {

    BOOST_TEST_MESSAGE("Testing spline interpolation on a Gaussian data set...");

    Real interpolated, interpolated2;
    Size n = 5;

    std::vector<Real> x(n), y(n);
    Real x1_bad=-1.7, x2_bad=1.7;

    for (Real start = -1.9, j=0; j<2; start+=0.2, j++) {
        x = xRange(start, start+3.6, n);
        y = gaussian(x);

        // Not-a-knot spline
        CubicInterpolation f(x.begin(), x.end(), y.begin(),
                             CubicInterpolation::Spline, false,
                             CubicInterpolation::NotAKnot, Null<Real>(),
                             CubicInterpolation::NotAKnot, Null<Real>());
        f.update();
        checkValues("Not-a-knot spline", f,
                    x.begin(), x.end(), y.begin());
        checkNotAKnotCondition("Not-a-knot spline", f);
        // bad performance
        interpolated = f(x1_bad);
        interpolated2= f(x2_bad);
        if (interpolated>0.0 && interpolated2>0.0 ) {
            BOOST_ERROR("Not-a-knot spline interpolation "
                        << "bad performance unverified"
                        << "\nat x = " << x1_bad
                        << " interpolated value: " << interpolated
                        << "\nat x = " << x2_bad
                        << " interpolated value: " << interpolated
                        << "\n at least one of them was expected to be < 0.0");
        }

        // MC not-a-knot spline
        f = CubicInterpolation(x.begin(), x.end(), y.begin(),
                               CubicInterpolation::Spline, true,
                               CubicInterpolation::NotAKnot, Null<Real>(),
                               CubicInterpolation::NotAKnot, Null<Real>());
        f.update();
        checkValues("MC not-a-knot spline", f,
                    x.begin(), x.end(), y.begin());
        // good performance
        interpolated = f(x1_bad);
        if (interpolated<0.0) {
            BOOST_ERROR("MC not-a-knot spline interpolation "
                        << "good performance unverified\n"
                        << "at x = " << x1_bad
                        << "\ninterpolated value: " << interpolated
                        << "\nexpected value > 0.0");
        }
        interpolated = f(x2_bad);
        if (interpolated<0.0) {
            BOOST_ERROR("MC not-a-knot spline interpolation "
                        << "good performance unverified\n"
                        << "at x = " << x2_bad
                        << "\ninterpolated value: " << interpolated
                        << "\nexpected value > 0.0");
        }
    }
}


/* See J. M. Hyman, "Accurate monotonicity preserving cubic interpolation"
   SIAM J. of Scientific and Statistical Computing, v. 4, 1983, pp. 645-654.
   http://math.lanl.gov/~mac/papers/numerics/H83.pdf
*/

BOOST_AUTO_TEST_CASE(testSplineOnRPN15AValues) {

    BOOST_TEST_MESSAGE("Testing spline interpolation on RPN15A data set...");

    const Real RPN15A_x[] = {
        7.99,       8.09,       8.19,      8.7,
        9.2,     10.0,     12.0,     15.0,     20.0
    };
    const Real RPN15A_y[] = {
        0.0, 2.76429e-5, 4.37498e-5, 0.169183,
        0.469428, 0.943740, 0.998636, 0.999919, 0.999994
    };

    Real interpolated;

    // Natural spline
    CubicInterpolation f = CubicInterpolation(
                                    std::begin(RPN15A_x), std::end(RPN15A_x),
                                    std::begin(RPN15A_y),
                                    CubicInterpolation::Spline, false,
                                    CubicInterpolation::SecondDerivative, 0.0,
                                    CubicInterpolation::SecondDerivative, 0.0);
    f.update();
    checkValues("Natural spline", f,
                std::begin(RPN15A_x), std::end(RPN15A_x), std::begin(RPN15A_y));
    check2ndDerivativeValue("Natural spline", f,
                            *std::begin(RPN15A_x), 0.0);
    check2ndDerivativeValue("Natural spline", f,
                            *(std::end(RPN15A_x)-1), 0.0);
    // poor performance
    Real x_bad = 11.0;
    interpolated = f(x_bad);
    if (interpolated<1.0) {
        BOOST_ERROR("Natural spline interpolation "
                    << "poor performance unverified\n"
                    << "at x = " << x_bad
                    << "\ninterpolated value: " << interpolated
                    << "\nexpected value > 1.0");
    }


    // Clamped spline
    f = CubicInterpolation(std::begin(RPN15A_x), std::end(RPN15A_x), std::begin(RPN15A_y),
                           CubicInterpolation::Spline, false,
                           CubicInterpolation::FirstDerivative, 0.0,
                           CubicInterpolation::FirstDerivative, 0.0);
    f.update();
    checkValues("Clamped spline", f,
                std::begin(RPN15A_x), std::end(RPN15A_x), std::begin(RPN15A_y));
    check1stDerivativeValue("Clamped spline", f,
                            *std::begin(RPN15A_x), 0.0);
    check1stDerivativeValue("Clamped spline", f,
                            *(std::end(RPN15A_x)-1), 0.0);
    // poor performance
    interpolated = f(x_bad);
    if (interpolated<1.0) {
        BOOST_ERROR("Clamped spline interpolation "
                    << "poor performance unverified\n"
                    << "at x = " << x_bad
                    << "\ninterpolated value: " << interpolated
                    << "\nexpected value > 1.0");
    }


    // Not-a-knot spline
    f = CubicInterpolation(std::begin(RPN15A_x), std::end(RPN15A_x), std::begin(RPN15A_y),
                           CubicInterpolation::Spline, false,
                           CubicInterpolation::NotAKnot, Null<Real>(),
                           CubicInterpolation::NotAKnot, Null<Real>());
    f.update();
    checkValues("Not-a-knot spline", f,
                std::begin(RPN15A_x), std::end(RPN15A_x), std::begin(RPN15A_y));
    checkNotAKnotCondition("Not-a-knot spline", f);
    // poor performance
    interpolated = f(x_bad);
    if (interpolated<1.0) {
        BOOST_ERROR("Not-a-knot spline interpolation "
                    << "poor performance unverified\n"
                    << "at x = " << x_bad
                    << "\ninterpolated value: " << interpolated
                    << "\nexpected value > 1.0");
    }


    // MC natural spline values
    f = CubicInterpolation(std::begin(RPN15A_x), std::end(RPN15A_x),
                           std::begin(RPN15A_y),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::SecondDerivative, 0.0,
                           CubicInterpolation::SecondDerivative, 0.0);
    f.update();
    checkValues("MC natural spline", f,
                std::begin(RPN15A_x), std::end(RPN15A_x), std::begin(RPN15A_y));
    // good performance
    interpolated = f(x_bad);
    if (interpolated>1.0) {
        BOOST_ERROR("MC natural spline interpolation "
                    << "good performance unverified\n"
                    << "at x = " << x_bad
                    << "\ninterpolated value: " << interpolated
                    << "\nexpected value < 1.0");
    }


    // MC clamped spline values
    f = CubicInterpolation(std::begin(RPN15A_x), std::end(RPN15A_x), std::begin(RPN15A_y),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::FirstDerivative, 0.0,
                           CubicInterpolation::FirstDerivative, 0.0);
    f.update();
    checkValues("MC clamped spline", f,
                std::begin(RPN15A_x), std::end(RPN15A_x), std::begin(RPN15A_y));
    check1stDerivativeValue("MC clamped spline", f,
                            *std::begin(RPN15A_x), 0.0);
    check1stDerivativeValue("MC clamped spline", f,
                            *(std::end(RPN15A_x)-1), 0.0);
    // good performance
    interpolated = f(x_bad);
    if (interpolated>1.0) {
        BOOST_ERROR("MC clamped spline interpolation "
                    << "good performance unverified\n"
                    << "at x = " << x_bad
                    << "\ninterpolated value: " << interpolated
                    << "\nexpected value < 1.0");
    }


    // MC not-a-knot spline values
    f = CubicInterpolation(std::begin(RPN15A_x), std::end(RPN15A_x), std::begin(RPN15A_y),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::NotAKnot, Null<Real>(),
                           CubicInterpolation::NotAKnot, Null<Real>());
    f.update();
    checkValues("MC not-a-knot spline", f,
                std::begin(RPN15A_x), std::end(RPN15A_x), std::begin(RPN15A_y));
    // good performance
    interpolated = f(x_bad);
    if (interpolated>1.0) {
        BOOST_ERROR("MC clamped spline interpolation "
                    << "good performance unverified\n"
                    << "at x = " << x_bad
                    << "\ninterpolated value: " << interpolated
                    << "\nexpected value < 1.0");
    }
}

/* Blossey, Frigyik, Farnum "A Note On CubicSpline Splines"
   Applied Linear Algebra and Numerical Analysis AMATH 352 Lecture Notes
   http://www.amath.washington.edu/courses/352-winter-2002/spline_note.pdf
*/

BOOST_AUTO_TEST_CASE(testSplineOnGenericValues) {

    BOOST_TEST_MESSAGE("Testing spline interpolation on generic values...");

    const Real generic_x[] = { 0.0, 1.0, 3.0, 4.0 };
    const Real generic_y[] = { 0.0, 0.0, 2.0, 2.0 };
    const Real generic_natural_y2[] = { 0.0, 1.5, -1.5, 0.0 };

    Real interpolated, error;
    Size i, n = std::size(generic_x);
    std::vector<Real> x35(3);

    // Natural spline
    CubicInterpolation f(std::begin(generic_x), std::end(generic_x),
                         std::begin(generic_y),
                         CubicInterpolation::Spline, false,
                         CubicInterpolation::SecondDerivative,
                         generic_natural_y2[0],
                         CubicInterpolation::SecondDerivative,
                         generic_natural_y2[n-1]);
    f.update();
    checkValues("Natural spline", f,
                std::begin(generic_x), std::end(generic_x), std::begin(generic_y));
    // cached second derivative
    for (i=0; i<n; i++) {
        interpolated = f.secondDerivative(generic_x[i]);
        error = interpolated - generic_natural_y2[i];
        if (std::fabs(error)>3e-16) {
            BOOST_ERROR("Natural spline interpolation "
                        << "second derivative failed at x=" << generic_x[i]
                        << "\ninterpolated value: " << interpolated
                        << "\nexpected value:     " << generic_natural_y2[i]
                        << "\nerror:              " << error);
        }
    }
    x35[1] = f(3.5);


    // Clamped spline
    Real y1a = 0.0, y1b = 0.0;
    f = CubicInterpolation(std::begin(generic_x), std::end(generic_x), std::begin(generic_y),
                    CubicInterpolation::Spline, false,
                    CubicInterpolation::FirstDerivative, y1a,
                    CubicInterpolation::FirstDerivative, y1b);
    f.update();
    checkValues("Clamped spline", f,
                std::begin(generic_x), std::end(generic_x), std::begin(generic_y));
    check1stDerivativeValue("Clamped spline", f,
                            *std::begin(generic_x), 0.0);
    check1stDerivativeValue("Clamped spline", f,
                            *(std::end(generic_x)-1), 0.0);
    x35[0] = f(3.5);


    // Not-a-knot spline
    f = CubicInterpolation(std::begin(generic_x), std::end(generic_x), std::begin(generic_y),
                           CubicInterpolation::Spline, false,
                           CubicInterpolation::NotAKnot, Null<Real>(),
                           CubicInterpolation::NotAKnot, Null<Real>());
    f.update();
    checkValues("Not-a-knot spline", f,
                std::begin(generic_x), std::end(generic_x), std::begin(generic_y));
    checkNotAKnotCondition("Not-a-knot spline", f);

    x35[2] = f(3.5);

    if (x35[0]>x35[1] || x35[1]>x35[2]) {
        BOOST_ERROR("Spline interpolation failure"
                    << "\nat x = " << 3.5
                    << "\nclamped spline    " << x35[0]
                    << "\nnatural spline    " << x35[1]
                    << "\nnot-a-knot spline " << x35[2]
                    << "\nvalues should be in increasing order");
    }
}

BOOST_AUTO_TEST_CASE(testSimmetricEndConditions) {

    BOOST_TEST_MESSAGE("Testing symmetry of spline interpolation "
                       "end-conditions...");

    Size n = 9;

    std::vector<Real> x, y;
    x = xRange(-1.8, 1.8, n);
    y = gaussian(x);

    // Not-a-knot spline
    CubicInterpolation f(x.begin(), x.end(), y.begin(),
                         CubicInterpolation::Spline, false,
                         CubicInterpolation::NotAKnot, Null<Real>(),
                         CubicInterpolation::NotAKnot, Null<Real>());
    f.update();
    checkValues("Not-a-knot spline", f,
                x.begin(), x.end(), y.begin());
    checkNotAKnotCondition("Not-a-knot spline", f);
    checkSymmetry("Not-a-knot spline", f, x[0]);


    // MC not-a-knot spline
    f = CubicInterpolation(x.begin(), x.end(), y.begin(),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::NotAKnot, Null<Real>(),
                           CubicInterpolation::NotAKnot, Null<Real>());
    f.update();
    checkValues("MC not-a-knot spline", f,
                x.begin(), x.end(), y.begin());
    checkSymmetry("MC not-a-knot spline", f, x[0]);
}

BOOST_AUTO_TEST_CASE(testDerivativeEndConditions) {

    BOOST_TEST_MESSAGE("Testing derivative end-conditions "
                       "for spline interpolation...");

    Size n = 4;

    std::vector<Real> x, y;
    x = xRange(-2.0, 2.0, n);
    y = parabolic(x);

    // Not-a-knot spline
    CubicInterpolation f(x.begin(), x.end(), y.begin(),
                         CubicInterpolation::Spline, false,
                         CubicInterpolation::NotAKnot, Null<Real>(),
                         CubicInterpolation::NotAKnot, Null<Real>());
    f.update();
    checkValues("Not-a-knot spline", f,
                x.begin(), x.end(), y.begin());
    check1stDerivativeValue("Not-a-knot spline", f,
                            x[0], 4.0);
    check1stDerivativeValue("Not-a-knot spline", f,
                            x[n-1], -4.0);
    check2ndDerivativeValue("Not-a-knot spline", f,
                            x[0], -2.0);
    check2ndDerivativeValue("Not-a-knot spline", f,
                            x[n-1], -2.0);


    // Clamped spline
    f = CubicInterpolation(x.begin(), x.end(), y.begin(),
                           CubicInterpolation::Spline, false,
                           CubicInterpolation::FirstDerivative,  4.0,
                           CubicInterpolation::FirstDerivative, -4.0);
    f.update();
    checkValues("Clamped spline", f,
                x.begin(), x.end(), y.begin());
    check1stDerivativeValue("Clamped spline", f,
                            x[0], 4.0);
    check1stDerivativeValue("Clamped spline", f,
                            x[n-1], -4.0);
    check2ndDerivativeValue("Clamped spline", f,
                            x[0], -2.0);
    check2ndDerivativeValue("Clamped spline", f,
                            x[n-1], -2.0);


    // SecondDerivative spline
    f = CubicInterpolation(x.begin(), x.end(), y.begin(),
                           CubicInterpolation::Spline, false,
                           CubicInterpolation::SecondDerivative, -2.0,
                           CubicInterpolation::SecondDerivative, -2.0);
    f.update();
    checkValues("SecondDerivative spline", f,
                x.begin(), x.end(), y.begin());
    check1stDerivativeValue("SecondDerivative spline", f,
                            x[0], 4.0);
    check1stDerivativeValue("SecondDerivative spline", f,
                            x[n-1], -4.0);
    check2ndDerivativeValue("SecondDerivative spline", f,
                            x[0], -2.0);
    check2ndDerivativeValue("SecondDerivative spline", f,
                            x[n-1], -2.0);

    // MC Not-a-knot spline
    f = CubicInterpolation(x.begin(), x.end(), y.begin(),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::NotAKnot, Null<Real>(),
                           CubicInterpolation::NotAKnot, Null<Real>());
    f.update();
    checkValues("MC Not-a-knot spline", f,
                x.begin(), x.end(), y.begin());
    check1stDerivativeValue("MC Not-a-knot spline", f,
                            x[0], 4.0);
    check1stDerivativeValue("MC Not-a-knot spline", f,
                            x[n-1], -4.0);
    check2ndDerivativeValue("MC Not-a-knot spline", f,
                            x[0], -2.0);
    check2ndDerivativeValue("MC Not-a-knot spline", f,
                            x[n-1], -2.0);


    // MC Clamped spline
    f = CubicInterpolation(x.begin(), x.end(), y.begin(),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::FirstDerivative,  4.0,
                           CubicInterpolation::FirstDerivative, -4.0);
    f.update();
    checkValues("MC Clamped spline", f,
                x.begin(), x.end(), y.begin());
    check1stDerivativeValue("MC Clamped spline", f,
                            x[0], 4.0);
    check1stDerivativeValue("MC Clamped spline", f,
                            x[n-1], -4.0);
    check2ndDerivativeValue("MC Clamped spline", f,
                            x[0], -2.0);
    check2ndDerivativeValue("MC Clamped spline", f,
                            x[n-1], -2.0);


    // MC SecondDerivative spline
    f = CubicInterpolation(x.begin(), x.end(), y.begin(),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::SecondDerivative, -2.0,
                           CubicInterpolation::SecondDerivative, -2.0);
    f.update();
    checkValues("MC SecondDerivative spline", f,
                x.begin(), x.end(), y.begin());
    check1stDerivativeValue("MC SecondDerivative spline", f,
                            x[0], 4.0);
    check1stDerivativeValue("MC SecondDerivative spline", f,
                            x[n-1], -4.0);
    check2ndDerivativeValue("SecondDerivative spline", f,
                            x[0], -2.0);
    check2ndDerivativeValue("MC SecondDerivative spline", f,
                            x[n-1], -2.0);

}


/* See R. L. Dougherty, A. Edelman, J. M. Hyman,
   "Nonnegativity-, Monotonicity-, or Convexity-Preserving CubicSpline and Quintic
   Hermite Interpolation"
   Mathematics Of Computation, v. 52, n. 186, April 1989, pp. 471-494.
*/

BOOST_AUTO_TEST_CASE(testNonRestrictiveHymanFilter) {

    BOOST_TEST_MESSAGE("Testing non-restrictive Hyman filter...");

    Size n = 4;

    std::vector<Real> x, y;
    x = xRange(-2.0, 2.0, n);
    y = parabolic(x);
    Real zero=0.0, interpolated, expected=0.0;

    // MC Not-a-knot spline
    CubicInterpolation f(x.begin(), x.end(), y.begin(),
                         CubicInterpolation::Spline, true,
                         CubicInterpolation::NotAKnot, Null<Real>(),
                         CubicInterpolation::NotAKnot, Null<Real>());
    f.update();
    interpolated = f(zero);
    if (std::fabs(interpolated-expected)>1e-15) {
        BOOST_ERROR("MC not-a-knot spline"
                    << " interpolation failed at x = " << zero
                    << "\n    interpolated value: " << interpolated
                    << "\n    expected value:     " << expected
                    << "\n    error:              "
                    << std::fabs(interpolated-expected));
    }


    // MC Clamped spline
    f = CubicInterpolation(x.begin(), x.end(), y.begin(),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::FirstDerivative,  4.0,
                           CubicInterpolation::FirstDerivative, -4.0);
    f.update();
    interpolated = f(zero);
    if (std::fabs(interpolated-expected)>1e-15) {
        BOOST_ERROR("MC clamped spline"
                    << " interpolation failed at x = " << zero
                    << "\n    interpolated value: " << interpolated
                    << "\n    expected value:     " << expected
                    << "\n    error:              "
                    << std::fabs(interpolated-expected));
    }


    // MC SecondDerivative spline
    f = CubicInterpolation(x.begin(), x.end(), y.begin(),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::SecondDerivative, -2.0,
                           CubicInterpolation::SecondDerivative, -2.0);
    f.update();
    interpolated = f(zero);
    if (std::fabs(interpolated-expected)>1e-15) {
        BOOST_ERROR("MC SecondDerivative spline"
                    << " interpolation failed at x = " << zero
                    << "\n    interpolated value: " << interpolated
                    << "\n    expected value:     " << expected
                    << "\n    error:              "
                    << std::fabs(interpolated-expected));
    }

}

BOOST_AUTO_TEST_CASE(testMultiSpline) {
    BOOST_TEST_MESSAGE("Testing N-dimensional cubic spline...");

    std::vector<Size> dim(5);
    dim[0] = 6; dim[1] = 5; dim[2] = 5; dim[3] = 6; dim[4] = 4;

    std::vector<Real> args(5), offsets(5);
    offsets[0] = 1.005; offsets[1] = 14.0; offsets[2] = 33.005;
    offsets[3] = 35.025; offsets[4] = 19.025;

    Real &s = args[0] = offsets[0],
         &t = args[1] = offsets[1],
         &u = args[2] = offsets[2],
         &v = args[3] = offsets[3],
         &w = args[4] = offsets[4];

    Size i, j, k, l, m;

    SplineGrid grid(5);

    Real r = 0.15;

    for (i = 0; i < 5; ++i) {
        Real temp = offsets[i];
        for (j = 0; j < dim[i]; temp += r, ++j)
            grid[i].push_back(temp);
    }

    MultiCubicSpline<5>::data_table y5(dim);

    for (i = 0; i < dim[0]; ++i)
        for (j = 0; j < dim[1]; ++j)
            for (k = 0; k < dim[2]; ++k)
                for (l = 0; l < dim[3]; ++l)
                    for (m = 0; m < dim[4]; ++m)
                        y5[i][j][k][l][m] =
                            multif(grid[0][i], grid[1][j], grid[2][k],
                                   grid[3][l], grid[4][m]);

    MultiCubicSpline<5> cs(grid, y5);
    /* it would fail with
    for (i = 0; i < dim[0]; ++i)
        for (j = 0; j < dim[1]; ++j)
            for (k = 0; k < dim[2]; ++k)
                for (l = 0; l < dim[3]; ++l)
                    for (m = 0; m < dim[4]; ++m) {
    */
    for (i = 1; i < dim[0]-1; ++i)
        for (j = 1; j < dim[1]-1; ++j)
            for (k = 1; k < dim[2]-1; ++k)
                for (l = 1; l < dim[3]-1; ++l)
                    for (m = 1; m < dim[4]-1; ++m) {
                        s = grid[0][i];
                        t = grid[1][j];
                        u = grid[2][k];
                        v = grid[3][l];
                        w = grid[4][m];
                        Real interpolated = cs(args);
                        Real expected = y5[i][j][k][l][m];
                        Real error = std::fabs(interpolated-expected);
                        Real tolerance = 1e-16;
                        if (error > tolerance) {
                            BOOST_ERROR(
                                "\n  At ("
                                << s << "," << t << "," << u << ","
                                            << v << "," << w << "):"
                                << "\n    interpolated: " << interpolated
                                << "\n    actual value: " << expected
                                << "\n       error: " << error
                                << "\n    tolerance: " << tolerance);
                        }
                    }


    unsigned long seed = 42;
    SobolRsg rsg(5, seed);

    Real tolerance = 1.7e-4;
    // actually tested up to 2^21-1=2097151 Sobol draws
    for (i = 0; i < 1023; ++i) {
        const std::vector<Real>& next = rsg.nextSequence().value;
        s = grid[0].front() + next[0]*(grid[0].back()-grid[0].front());
        t = grid[1].front() + next[1]*(grid[1].back()-grid[1].front());
        u = grid[2].front() + next[2]*(grid[2].back()-grid[2].front());
        v = grid[3].front() + next[3]*(grid[3].back()-grid[3].front());
        w = grid[4].front() + next[4]*(grid[4].back()-grid[4].front());
        Real interpolated = cs(args), expected = multif(s, t, u, v, w);
        Real error = std::fabs(interpolated-expected);
        if (error > tolerance) {
            BOOST_ERROR(
                "\n  At ("
                << s << "," << t << "," << u << "," << v << "," << w << "):"
                << "\n    interpolated: " << interpolated
                << "\n    actual value: " << expected
                << "\n    error:        " << error
                << "\n    tolerance:    " << tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testAsFunctor) {

    BOOST_TEST_MESSAGE("Testing use of interpolations as functors...");

    const Real x[] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
    const Real y[] = { 5.0, 4.0, 3.0, 2.0, 1.0 };

   Interpolation f = LinearInterpolation(std::begin(x), std::end(x), std::begin(y));
    f.update();

    const Real x2[] = { -2.0, -1.0, 0.0, 1.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    Size N = std::size(x2);
    std::vector<Real> y2(N);
    Real tolerance = 1.0e-12;

    // case 1: extrapolation not allowed
    try {
        std::transform(std::begin(x2), std::end(x2), y2.begin(), f);
        throw NotThrown();
    }
    catch (Error&) {
        // as expected; do nothing
    }
    catch (NotThrown&) {
        QL_FAIL("failed to throw exception when trying to extrapolate");
    }

    // case 2: enable extrapolation
    f.enableExtrapolation();
    y2 = std::vector<Real>(N);
    std::transform(std::begin(x2), std::end(x2), y2.begin(), f);
    for (Size i=0; i<N; i++) {
        Real expected = 5.0-x2[i];
        if (std::fabs(y2[i]-expected) > tolerance)
            BOOST_ERROR(
                "failed to reproduce " << io::ordinal(i+1) << " expected datum"
                << std::fixed
                << "\n    expected:   " << expected
                << "\n    calculated: " << y2[i]
                << std::scientific
                << "\n    error:      " << std::fabs(y2[i]-expected));
    }
}

BOOST_AUTO_TEST_CASE(testFritschButland) {

    BOOST_TEST_MESSAGE("Testing Fritsch-Butland interpolation...");

    const Real x[5] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
    const Real y[][5] = {{ 1.0, 2.0, 1.0, 1.0, 2.0 },
                         { 1.0, 2.0, 1.0, 1.0, 1.0 },
                         { 2.0, 1.0, 0.0, 2.0, 3.0 }};

    for (Size i=0; i<3; ++i) {

        Interpolation f = FritschButlandCubic(std::begin(x), std::end(x), std::begin(y[i]));
        f.update();

        for (Size j=0; j<4; ++j) {
            Real left_knot = x[j];
            Integer expected_sign = sign(y[i][j], y[i][j+1]);
            for (Size k=0; k<10; ++k) {
                Real x1 = left_knot + k*0.1, x2 = left_knot + (k+1)*0.1;
                Real y1 = f(x1), y2 = f(x2);
                if (std::isnan(y1))
                    BOOST_ERROR("NaN detected in case " << i << ":"
                                << std::fixed
                                << "\n    f(" << x1 << ") = " << y1);
                else if (sign(y1, y2) != expected_sign)
                    BOOST_ERROR("interpolation is not monotonic "
                                "in case " << i << ":"
                                << std::fixed
                                << "\n    f(" << x1 << ") = " << y1
                                << "\n    f(" << x2 << ") = " << y2);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testBackwardFlat) {

    BOOST_TEST_MESSAGE("Testing backward-flat interpolation...");

    const Real x[] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
    const Real y[] = { 5.0, 4.0, 3.0, 2.0, 1.0 };

    Interpolation f = BackwardFlatInterpolation(std::begin(x), std::end(x), std::begin(y));
    f.update();

    Size N = std::size(x);
    Size i;
    Real tolerance = 1.0e-12;

    // at original points
    for (i=0; i<N; i++) {
        Real p = x[i];
        Real calculated = f(p);
        Real expected = y[i];
        if (std::fabs(expected-calculated) > tolerance)
            BOOST_ERROR(
                "failed to reproduce " << io::ordinal(i+1) << " datum"
                << std::fixed
                << "\n    expected:   " << expected
                << "\n    calculated: " << calculated
                << std::scientific
                << "\n    error:      " << std::fabs(calculated-expected));
    }

    // at middle points
    for (i=0; i<N-1; i++) {
        Real p = (x[i]+x[i+1])/2;
        Real calculated = f(p);
        Real expected = y[i+1];
        if (std::fabs(expected-calculated) > tolerance)
            BOOST_ERROR(
                "failed to interpolate correctly at " << p
                << std::fixed
                << "\n    expected:   " << expected
                << "\n    calculated: " << calculated
                << std::scientific
                << "\n    error:      " << std::fabs(calculated-expected));
    }

    // outside the original range
    f.enableExtrapolation();

    Real p = x[0] - 0.5;
    Real calculated = f(p);
    Real expected = y[0];
    if (std::fabs(expected-calculated) > tolerance)
        BOOST_ERROR(
            "failed to extrapolate correctly at " << p
            << std::fixed
            << "\n    expected:   " << expected
            << "\n    calculated: " << calculated
            << std::scientific
            << "\n    error:      " << std::fabs(calculated-expected));

    p = x[N-1] + 0.5;
    calculated = f(p);
    expected = y[N-1];
    if (std::fabs(expected-calculated) > tolerance)
        BOOST_ERROR(
            "failed to extrapolate correctly at " << p
            << std::fixed
            << "\n    expected:   " << expected
            << "\n    calculated: " << calculated
            << std::scientific
            << "\n    error:      " << std::fabs(calculated-expected));

    // primitive at original points
    calculated = f.primitive(x[0]);
    expected = 0.0;
    if (std::fabs(expected-calculated) > tolerance)
        BOOST_ERROR(
            "failed to calculate primitive at " << x[0]
            << std::fixed
            << "\n    expected:   " << expected
            << "\n    calculated: " << calculated
            << std::scientific
            << "\n    error:      " << std::fabs(calculated-expected));

    Real sum = 0.0;
    for (i=1; i<N; i++) {
        sum += (x[i]-x[i-1])*y[i];
        Real calculated = f.primitive(x[i]);
        Real expected = sum;
        if (std::fabs(expected-calculated) > tolerance)
            BOOST_ERROR(
                "failed to calculate primitive at " << x[i]
                << std::fixed
                << "\n    expected:   " << expected
                << "\n    calculated: " << calculated
                << std::scientific
                << "\n    error:      " << std::fabs(calculated-expected));
    }

    // primitive at middle points
    sum = 0.0;
    for (i=0; i<N-1; i++) {
        Real p = (x[i]+x[i+1])/2;
        sum += (x[i+1]-x[i])*y[i+1]/2;
        Real calculated = f.primitive(p);
        Real expected = sum;
        sum += (x[i+1]-x[i])*y[i+1]/2;
        if (std::fabs(expected-calculated) > tolerance)
            BOOST_ERROR(
                "failed to calculate primitive at " << x[i]
                << std::fixed
                << "\n    expected:   " << expected
                << "\n    calculated: " << calculated
                << std::scientific
                << "\n    error:      " << std::fabs(calculated-expected));
    }

}

BOOST_AUTO_TEST_CASE(testForwardFlat) {

    BOOST_TEST_MESSAGE("Testing forward-flat interpolation...");

    const Real x[] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
    const Real y[] = { 5.0, 4.0, 3.0, 2.0, 1.0 };

    Interpolation f = ForwardFlatInterpolation(std::begin(x), std::end(x), std::begin(y));
    f.update();

    Size N = std::size(x);
    Size i;
    Real tolerance = 1.0e-12;

    // at original points
    for (i=0; i<N; i++) {
        Real p = x[i];
        Real calculated = f(p);
        Real expected = y[i];
        if (std::fabs(expected-calculated) > tolerance)
            BOOST_ERROR(
                "failed to reproduce " << io::ordinal(i+1) << " datum"
                << std::fixed
                << "\n    expected:   " << expected
                << "\n    calculated: " << calculated
                << std::scientific
                << "\n    error:      " << std::fabs(calculated-expected));
    }

    // at middle points
    for (i=0; i<N-1; i++) {
        Real p = (x[i]+x[i+1])/2;
        Real calculated = f(p);
        Real expected = y[i];
        if (std::fabs(expected-calculated) > tolerance)
            BOOST_ERROR(
                "failed to interpolate correctly at " << p
                << std::fixed
                << "\n    expected:   " << expected
                << "\n    calculated: " << calculated
                << std::scientific
                << "\n    error:      " << std::fabs(calculated-expected));
    }

    // outside the original range
    f.enableExtrapolation();

    Real p = x[0] - 0.5;
    Real calculated = f(p);
    Real expected = y[0];
    if (std::fabs(expected-calculated) > tolerance)
        BOOST_ERROR(
            "failed to extrapolate correctly at " << p
            << std::fixed
            << "\n    expected:   " << expected
            << "\n    calculated: " << calculated
            << std::scientific
            << "\n    error:      " << std::fabs(calculated-expected));

    p = x[N-1] + 0.5;
    calculated = f(p);
    expected = y[N-1];
    if (std::fabs(expected-calculated) > tolerance)
        BOOST_ERROR(
            "failed to extrapolate correctly at " << p
            << std::fixed
            << "\n    expected:   " << expected
            << "\n    calculated: " << calculated
            << std::scientific
            << "\n    error:      " << std::fabs(calculated-expected));

    // primitive at original points
    calculated = f.primitive(x[0]);
    expected = 0.0;
    if (std::fabs(expected-calculated) > tolerance)
        BOOST_ERROR(
            "failed to calculate primitive at " << x[0]
            << std::fixed
            << "\n    expected:   " << expected
            << "\n    calculated: " << calculated
            << std::scientific
            << "\n    error:      " << std::fabs(calculated-expected));

    Real sum = 0.0;
    for (i=1; i<N; i++) {
        sum += (x[i]-x[i-1])*y[i-1];
        Real calculated = f.primitive(x[i]);
        Real expected = sum;
        if (std::fabs(expected-calculated) > tolerance)
            BOOST_ERROR(
                "failed to calculate primitive at " << x[i]
                << std::fixed
                << "\n    expected:   " << expected
                << "\n    calculated: " << calculated
                << std::scientific
                << "\n    error:      " << std::fabs(calculated-expected));
    }

    // primitive at middle points
    sum = 0.0;
    for (i=0; i<N-1; i++) {
        Real p = (x[i]+x[i+1])/2;
        sum += (x[i+1]-x[i])*y[i]/2;
        Real calculated = f.primitive(p);
        Real expected = sum;
        sum += (x[i+1]-x[i])*y[i]/2;
        if (std::fabs(expected-calculated) > tolerance)
            BOOST_ERROR(
                "failed to calculate primitive at " << p
                << std::fixed
                << "\n    expected:   " << expected
                << "\n    calculated: " << calculated
                << std::scientific
                << "\n    error:      " << std::fabs(calculated-expected));
    }
}

BOOST_AUTO_TEST_CASE(testSabrInterpolation){

    BOOST_TEST_MESSAGE("Testing Sabr interpolation...");

    // Test SABR function against input volatilities
    Real tolerance = 1.0e-12;
    std::vector<Real> strikes(31);
    std::vector<Real> volatilities(31);
    // input strikes
    strikes[0] = 0.03 ; strikes[1] = 0.032 ; strikes[2] = 0.034 ;
    strikes[3] = 0.036 ; strikes[4] = 0.038 ; strikes[5] = 0.04 ;
    strikes[6] = 0.042 ; strikes[7] = 0.044 ; strikes[8] = 0.046 ;
    strikes[9] = 0.048 ; strikes[10] = 0.05 ; strikes[11] = 0.052 ;
    strikes[12] = 0.054 ; strikes[13] = 0.056 ; strikes[14] = 0.058 ;
    strikes[15] = 0.06 ; strikes[16] = 0.062 ; strikes[17] = 0.064 ;
    strikes[18] = 0.066 ; strikes[19] = 0.068 ; strikes[20] = 0.07 ;
    strikes[21] = 0.072 ; strikes[22] = 0.074 ; strikes[23] = 0.076 ;
    strikes[24] = 0.078 ; strikes[25] = 0.08 ; strikes[26] = 0.082 ;
    strikes[27] = 0.084 ; strikes[28] = 0.086 ; strikes[29] = 0.088;
    strikes[30] = 0.09;
    // input volatilities
    volatilities[0] = 1.16725837321531 ; volatilities[1] = 1.15226075991385 ; volatilities[2] = 1.13829711098834 ;
    volatilities[3] = 1.12524190877505 ; volatilities[4] = 1.11299079244474 ; volatilities[5] = 1.10145609357162 ;
    volatilities[6] = 1.09056348513411 ; volatilities[7] = 1.08024942745106 ; volatilities[8] = 1.07045919457758 ;
    volatilities[9] = 1.06114533019077 ; volatilities[10] = 1.05226642581503 ; volatilities[11] = 1.04378614411707 ;
    volatilities[12] = 1.03567243073732 ; volatilities[13] = 1.0278968727451 ; volatilities[14] = 1.02043417226345 ;
    volatilities[15] = 1.01326171139321 ; volatilities[16] = 1.00635919013311 ; volatilities[17] = 0.999708323124949 ;
    volatilities[18] = 0.993292584155381 ; volatilities[19] = 0.987096989695393 ; volatilities[20] = 0.98110791455717 ;
    volatilities[21] = 0.975312934134512 ; volatilities[22] = 0.969700688771689 ; volatilities[23] = 0.964260766651027;
    volatilities[24] = 0.958983602256592 ; volatilities[25] = 0.953860388001395 ; volatilities[26] = 0.948882997029509 ;
    volatilities[27] = 0.944043915545469 ; volatilities[28] = 0.939336183299237 ; volatilities[29] = 0.934753341079515 ;
    volatilities[30] = 0.930289384251337;

    Time expiry = 1.0;
    Real forward = 0.039;
    // input SABR coefficients (corresponding to the vols above)
    Real initialAlpha = 0.3;
    Real initialBeta = 0.6;
    Real initialNu = 0.02;
    Real initialRho = 0.01;
    // calculate SABR vols and compare with input vols
    for(Size i=0; i< strikes.size(); i++){
        Real calculatedVol = sabrVolatility(strikes[i], forward, expiry,
                                            initialAlpha, initialBeta,
                                            initialNu, initialRho);
        if (std::fabs(volatilities[i]-calculatedVol) > tolerance)
        BOOST_ERROR(
            "failed to calculate Sabr function at strike " << strikes[i]
            << "\n    expected:   " << volatilities[i]
            << "\n    calculated: " << calculatedVol
            << "\n    error:      " << std::fabs(calculatedVol-volatilities[i]));
    }

    // Test SABR calibration against input parameters
    Real alphaGuess = std::sqrt(0.2);
    Real betaGuess = 0.5;
    Real nuGuess = std::sqrt(0.4);
    Real rhoGuess = 0.0;

    const bool vegaWeighted[]= {true, false};
    const bool isAlphaFixed[]= {true, false};
    const bool isBetaFixed[]= {true, false};
    const bool isNuFixed[]= {true, false};
    const bool isRhoFixed[]= {true, false};

    Real calibrationTolerance = 5.0e-8;
    // initialize optimization methods
    std::vector<ext::shared_ptr<OptimizationMethod>> methods_ = {
        ext::shared_ptr<OptimizationMethod>(new Simplex(0.01)),
        ext::shared_ptr<OptimizationMethod>(new LevenbergMarquardt(1e-8, 1e-8, 1e-8))
    };
    // Initialize end criteria
    ext::shared_ptr<EndCriteria> endCriteria(new
                  EndCriteria(100000, 100, 1e-8, 1e-8, 1e-8));
    // Test looping over all possibilities
    for (auto& method : methods_) {
        for (bool i : vegaWeighted) {
            for (bool k_a : isAlphaFixed) {
                for (bool k_b : isBetaFixed) {
                    for (bool k_n : isNuFixed) {
                        for (bool k_r : isRhoFixed) {
                            // to meet the tough calibration tolerance we need to lower the default
                            // error threshold for accepting a calibration (to be more specific,
                            // some of the new test cases arising from fixing a subset of the
                            // model's parameters do not calibrate with the desired error using the
                            // initial guess (i.e. optimization runs into a local minimum) - then a
                            // series of random start values for optimization is chosen until our
                            // tight custom error threshold is satisfied.
                            SABRInterpolation sabrInterpolation(
                                strikes.begin(), strikes.end(), volatilities.begin(), expiry,
                                forward, k_a ? initialAlpha : alphaGuess,
                                k_b ? initialBeta : betaGuess, k_n ? initialNu : nuGuess,
                                k_r ? initialRho : rhoGuess, k_a, k_b, k_n, k_r, i, endCriteria,
                                method, 1E-10);
                            sabrInterpolation.update();

                            // Recover SABR calibration parameters
                            bool failed = false;
                            Real calibratedAlpha = sabrInterpolation.alpha();
                            Real calibratedBeta = sabrInterpolation.beta();
                            Real calibratedNu = sabrInterpolation.nu();
                            Real calibratedRho = sabrInterpolation.rho();
                            Real error;

                            // compare results: alpha
                            error = std::fabs(initialAlpha - calibratedAlpha);
                            if (error > calibrationTolerance) {
                                BOOST_ERROR("\nfailed to calibrate alpha Sabr parameter:"
                                            << "\n    expected:        " << initialAlpha
                                            << "\n    calibrated:      " << calibratedAlpha
                                            << "\n    error:           " << error);
                                failed = true;
                            }
                            // Beta
                            error = std::fabs(initialBeta - calibratedBeta);
                            if (error > calibrationTolerance) {
                                BOOST_ERROR("\nfailed to calibrate beta Sabr parameter:"
                                            << "\n    expected:        " << initialBeta
                                            << "\n    calibrated:      " << calibratedBeta
                                            << "\n    error:           " << error);
                                failed = true;
                            }
                            // Nu
                            error = std::fabs(initialNu - calibratedNu);
                            if (error > calibrationTolerance) {
                                BOOST_ERROR("\nfailed to calibrate nu Sabr parameter:"
                                            << "\n    expected:        " << initialNu
                                            << "\n    calibrated:      " << calibratedNu
                                            << "\n    error:           " << error);
                                failed = true;
                            }
                            // Rho
                            error = std::fabs(initialRho - calibratedRho);
                            if (error > calibrationTolerance) {
                                BOOST_ERROR("\nfailed to calibrate rho Sabr parameter:"
                                            << "\n    expected:        " << initialRho
                                            << "\n    calibrated:      " << calibratedRho
                                            << "\n    error:           " << error);
                                failed = true;
                            }

                            if (failed)
                                BOOST_FAIL("\nSabr calibration failure:"
                                           << "\n    isAlphaFixed:    " << k_a
                                           << "\n    isBetaFixed:     " << k_b
                                           << "\n    isNuFixed:       " << k_n
                                           << "\n    isRhoFixed:      " << k_r
                                           << "\n    vegaWeighted[i]: " << i);
                        }
                    }
                }
            }
        }
    }
}


struct SabrTestCase {
    Time expiry;
    Real forward;
    Real alpha;
    Real beta;
    Real nu;
    Real rho;
};

BOOST_AUTO_TEST_CASE(testSabrGuess){

    BOOST_TEST_MESSAGE("Testing Sabr guess...");

    #if BOOST_VERSION >= 107800

    // table 3 in Le Floc'h and Kennedy.
    // They only seems to use it with lognormal volatility
    // and no shift; we extend the test here.
    SabrTestCase cases[] = {
        {0.058, 2016, 0.271, 1.0, 1.010, -0.345},
        {0.153, 2016, 0.256, 1.0, 0.933, -0.321},
        {0.230, 2016, 0.256, 1.0, 0.820, -0.346},
        {0.479, 2016, 0.255, 1.0, 0.629, -0.370},
        {0.729, 2016, 0.257, 1.0, 0.528, -0.403},
        {1.227, 2016, 0.260, 1.0, 0.448, -0.429},
        {1.726, 2016, 0.261, 1.0, 0.392, -0.440},
        {2.244, 2016, 0.262, 1.0, 0.355, -0.445},
        {2.742, 2016, 0.262, 1.0, 0.329, -0.445},
        {3.241, 2016, 0.262, 1.0, 0.310, -0.447},
        {4.239, 2016, 0.263, 1.0, 0.284, -0.452}
    };

    for (const auto& c : cases) {
        for (auto type: {VolatilityType::ShiftedLognormal, VolatilityType::Normal}) {
            for (auto shift: {0.0, 0.01, 0.1}) {
                // strikes at forward and plus or minus 5%
                Real k_m = c.forward * 0.95, k_0 = c.forward, k_p = c.forward * 1.05;
                Real vol_m = shiftedSabrVolatility(k_m, c.forward, c.expiry, c.alpha, c.beta, c.nu, c.rho, shift, type);
                Real vol_0 = shiftedSabrVolatility(k_0, c.forward, c.expiry, c.alpha, c.beta, c.nu, c.rho, shift, type);
                Real vol_p = shiftedSabrVolatility(k_p, c.forward, c.expiry, c.alpha, c.beta, c.nu, c.rho, shift, type);

                // try to invert smile and retrieve parameters
                auto [alpha, beta, nu, rho] = sabrGuess(k_m, vol_m, k_0, vol_0, k_p, vol_p,
                                                        c.forward, c.expiry, c.beta,
                                                        shift, type);

                if (std::fabs(alpha - c.alpha) > 0.0001)
                    BOOST_ERROR("alpha = " << alpha << ", expected = " << c.alpha << ", error = " << alpha - c.alpha);
                BOOST_CHECK_EQUAL(beta, c.beta);
                if (std::fabs(nu - c.nu) > 0.01)
                    BOOST_ERROR("nu = " << nu << ", expected = " << c.nu << ", error = " << nu - c.nu);
                if (std::fabs(rho - c.rho) > 0.005)
                    BOOST_ERROR("rho = " << rho << ", expected = " << c.rho << ", error = " << rho - c.rho);
            }
        }
    }

    #else

    BOOST_CHECK_EXCEPTION(sabrGuess(99, 0.32, 100, 0.30, 101, 0.31,
                                    100, 1.0, 0.5, 0.0, VolatilityType::Normal),
                          Error,
                          ExpectedErrorMessage("Boost 1.78 or later is required"));

    #endif
}


BOOST_AUTO_TEST_CASE(testKernelInterpolation) {

    BOOST_TEST_MESSAGE("Testing kernel 1D interpolation...");

    std::vector<Real> deltaGrid = {0.10, 0.25, 0.50, 0.75, 0.90};

    std::vector<Real> yd2(deltaGrid.size()); // test y-values 2

    std::vector<Real> yd3(deltaGrid.size()); // test y-values 3

    std::vector<std::vector<Real>> yd = {
        {11.275, 11.125, 11.250, 11.825, 12.625},
        {16.025, 13.450, 11.350, 10.150, 10.075},
        {10.300, 9.6375, 9.2000, 9.1125, 9.4000}
    };
    std::vector<Real> lambdaVec = {0.05, 0.50, 0.75, 1.65, 2.55};

    Real tolerance = 2.0e-5;

    Real expectedVal;
    Real calcVal;

    // Check that y-values at knots are exactly the feeded y-values,
    // irrespective of kernel parameters
    for (Real i : lambdaVec) {
        GaussianKernel myKernel(0, i);

        for (auto currY : yd) {

            KernelInterpolation f(deltaGrid.begin(), deltaGrid.end(), currY.begin(), myKernel
#ifdef __FAST_MATH__
                                  ,
                                  1e-6
#endif
            );
            f.update();

            for (Size dIt=0; dIt< deltaGrid.size(); ++dIt) {
                expectedVal=currY[dIt];
                calcVal=f(deltaGrid[dIt]);

                if (std::fabs(expectedVal-calcVal)>tolerance) {

                    BOOST_ERROR("Kernel interpolation failed at x = "
                                << deltaGrid[dIt]
                                << std::scientific
                                << "\n    interpolated value: " << calcVal
                                << "\n    expected value:     " << expectedVal
                                << "\n    error:              "
                                << std::fabs(expectedVal-calcVal));
                }
            }
        }
    }

    std::vector<Real> testDeltaGrid = {0.121, 0.279, 0.678, 0.790, 0.980};

    // Gaussian Kernel values for testDeltaGrid with a standard
    // deviation of 2.05 (the value is arbitrary.)  Source: parrallel
    // implementation in R, no literature sources found

    std::vector<std::vector<Real>> ytd = {
        {11.23847, 11.12003, 11.58932, 11.99168, 13.29650},
        {15.55922, 13.11088, 10.41615, 10.05153, 10.50741},
        {10.17473, 9.557842, 9.09339,  9.149687, 9.779971}
    };

    GaussianKernel myKernel(0,2.05);

    for (Size j=0; j< ytd.size(); ++j) {
        std::vector<Real> currY=yd[j];
        const std::vector<Real>& currTY=ytd[j];

        // Build interpolation according to original grid + y-values
        KernelInterpolation f(deltaGrid.begin(), deltaGrid.end(),
                              currY.begin(), myKernel);
        f.update();

        // test values at test Grid
        for (Size dIt=0; dIt< testDeltaGrid.size(); ++dIt) {

            expectedVal=currTY[dIt];
            f.enableExtrapolation();// allow extrapolation

            calcVal=f(testDeltaGrid[dIt]);
            if (std::fabs(expectedVal-calcVal)>tolerance) {

                BOOST_ERROR("Kernel interpolation failed at x = "
                            << deltaGrid[dIt]
                            << std::scientific
                            << "\n    interpolated value: " << calcVal
                            << "\n    expected value:     " << expectedVal
                            << "\n    error:              "
                            << std::fabs(expectedVal-calcVal));
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testKernelInterpolation2D){

    // No test values known from the literature.
    // Testing for consistency of input output data
    // at the nodes

    BOOST_TEST_MESSAGE("Testing kernel 2D interpolation...");

    Real mean=0.0, var=0.18;
    GaussianKernel myKernel(mean,var);

    std::vector<Real> xVec(10);
    xVec[0] = 0.10; xVec[1] = 0.20; xVec[2] = 0.30; xVec[3] = 0.40;
    xVec[4] = 0.50; xVec[5] = 0.60; xVec[6] = 0.70; xVec[7] = 0.80;
    xVec[8] = 0.90; xVec[9] = 1.00;

    std::vector<Real> yVec(3);
    yVec[0] = 1.0; yVec[1] = 2.0; yVec[2] = 3.5;

    Matrix M(xVec.size(),yVec.size());

    M[0][0]=0.25; M[1][0]=0.24; M[2][0]=0.23; M[3][0]=0.20; M[4][0]=0.19;
    M[5][0]=0.20; M[6][0]=0.21; M[7][0]=0.22; M[8][0]=0.26; M[9][0]=0.29;

    M[0][1]=0.27; M[1][1]=0.26; M[2][1]=0.25; M[3][1]=0.22; M[4][1]=0.21;
    M[5][1]=0.22; M[6][1]=0.23; M[7][1]=0.24; M[8][1]=0.28; M[9][1]=0.31;

    M[0][2]=0.21; M[1][2]=0.22; M[2][2]=0.27; M[3][2]=0.29; M[4][2]=0.24;
    M[5][2]=0.28; M[6][2]=0.25; M[7][2]=0.22; M[8][2]=0.29; M[9][2]=0.30;

    KernelInterpolation2D kernel2D(xVec.begin(),xVec.end(),
                                   yVec.begin(),yVec.end(),M,myKernel);

    Real calcVal,expectedVal;
    Real tolerance = 1.0e-10;

    for(Size i=0;i<M.rows();++i){
        for(Size j=0;j<M.columns();++j){

            calcVal=kernel2D(xVec[i],yVec[j]);
            expectedVal=M[i][j];

            if(std::fabs(expectedVal-calcVal)>tolerance){

                BOOST_ERROR("2D Kernel interpolation failed at x = " << xVec[i]
                            << ", y = " << yVec[j]
                            << "\n    interpolated value: " << calcVal
                            << "\n    expected value:     " << expectedVal
                            << "\n    error:              "
                            << std::fabs(expectedVal-calcVal));
            }
        }
    }

    // alternative data set
    std::vector<Real> xVec1(4);
    xVec1[0] = 80.0; xVec1[1] = 90.0; xVec1[2] = 100.0; xVec1[3] = 110.0;

    std::vector<Real> yVec1(8);
    yVec1[0] = 0.5; yVec1[1] = 0.7; yVec1[2] = 1.0; yVec1[3] = 2.0;
    yVec1[4] = 3.5; yVec1[5] = 4.5; yVec1[6] = 5.5; yVec1[7] = 6.5;

    Matrix M1(xVec1.size(),yVec1.size());
    M1[0][0]=10.25; M1[1][0]=12.24;M1[2][0]=14.23;M1[3][0]=17.20;
    M1[0][1]=12.25; M1[1][1]=15.24;M1[2][1]=16.23;M1[3][1]=16.20;
    M1[0][2]=12.25; M1[1][2]=13.24;M1[2][2]=13.23;M1[3][2]=17.20;
    M1[0][3]=13.25; M1[1][3]=15.24;M1[2][3]=12.23;M1[3][3]=19.20;
    M1[0][4]=14.25; M1[1][4]=16.24;M1[2][4]=13.23;M1[3][4]=12.20;
    M1[0][5]=15.25; M1[1][5]=17.24;M1[2][5]=14.23;M1[3][5]=12.20;
    M1[0][6]=16.25; M1[1][6]=13.24;M1[2][6]=15.23;M1[3][6]=10.20;
    M1[0][7]=14.25; M1[1][7]=14.24;M1[2][7]=16.23;M1[3][7]=19.20;

    // test with function pointer
    KernelInterpolation2D kernel2DEp(xVec1.begin(),xVec1.end(),
                                     yVec1.begin(),yVec1.end(),M1,
                                     &epanechnikovKernel);

    for(Size i=0;i<M1.rows();++i){
        for(Size j=0;j<M1.columns();++j){

            calcVal=kernel2DEp(xVec1[i],yVec1[j]);
            expectedVal=M1[i][j];

            if(std::fabs(expectedVal-calcVal)>tolerance){

                BOOST_ERROR("2D Epanechnkikov Kernel interpolation failed at x = " << xVec1[i]
                            << ", y = " << yVec1[j]
                            << "\n    interpolated value: " << calcVal
                            << "\n    expected value:     " << expectedVal
                            << "\n    error:              "
                            << std::fabs(expectedVal-calcVal));
            }
        }
    }

    // test updating mechanism by changing initial variables
    xVec1[0] = 60.0; xVec1[1] = 95.0; xVec1[2] = 105.0; xVec1[3] = 135.0;

    yVec1[0] = 12.5; yVec1[1] = 13.7; yVec1[2] = 15.0; yVec1[3] = 19.0;
    yVec1[4] = 26.5; yVec1[5] = 27.5; yVec1[6] = 29.2; yVec1[7] = 36.5;

    kernel2DEp.update();

    for(Size i=0;i<M1.rows();++i){
        for(Size j=0;j<M1.columns();++j){

            calcVal=kernel2DEp(xVec1[i],yVec1[j]);
            expectedVal=M1[i][j];

            if(std::fabs(expectedVal-calcVal)>tolerance){

                BOOST_ERROR("2D Epanechnkikov Kernel updated interpolation failed at x = " << xVec1[i]
                            << ", y = " << yVec1[j]
                            << "\n    interpolated value: " << calcVal
                            << "\n    expected value:     " << expectedVal
                            << "\n    error:              "
                            << std::fabs(expectedVal-calcVal));
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testBicubicDerivatives) {
    BOOST_TEST_MESSAGE("Testing bicubic spline derivatives...");

    std::vector<Real> x(100), y(100);
    for (Size i=0; i < 100; ++i) {
        x[i] = y[i] = i/20.0;
    }

    Matrix f(100, 100);
    for (Size i=0; i < 100; ++i)
        for (Size j=0; j < 100; ++j)
            f[i][j] = y[i]/10*std::sin(x[j])+std::cos(y[i]);

    const Real tol=0.005;
    BicubicSpline spline(x.begin(), x.end(), y.begin(), y.end(), f);

    for (Size i=5; i < 95; i+=10) {
        for (Size j=5; j < 95; j+=10) {
            Real f_x  = spline.derivativeX(x[j],y[i]);
            Real f_xx = spline.secondDerivativeX(x[j],y[i]);
            Real f_y  = spline.derivativeY(x[j],y[i]);
            Real f_yy = spline.secondDerivativeY(x[j],y[i]);
            Real f_xy = spline.derivativeXY(x[j],y[i]);

            if (std::fabs(f_x - y[i]/10*std::cos(x[j])) > tol) {
                BOOST_ERROR("Failed to reproduce f_x");
            }
            if (std::fabs(f_xx + y[i]/10*std::sin(x[j])) > tol) {
                BOOST_ERROR("Failed to reproduce f_xx");
            }
            if (std::fabs(f_y - (std::sin(x[j])/10-std::sin(y[i]))) > tol) {
                BOOST_ERROR("Failed to reproduce f_y");
            }
            if (std::fabs(f_yy + std::cos(y[i])) > tol) {
                BOOST_ERROR("Failed to reproduce f_yy");
            }
            if (std::fabs(f_xy - std::cos(x[j])/10) > tol) {
                BOOST_ERROR("Failed to reproduce f_xy");
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testBicubicUpdate) {
    BOOST_TEST_MESSAGE("Testing that bicubic splines actually update...");

    Size N=6;
    std::vector<Real> x(N), y(N);
    for (Size i=0; i < N; ++i) {
        x[i] = y[i] = i*0.2;
    }

    Matrix f(N, N);
    for (Size i=0; i < N; ++i)
        for (Size j=0; j < N; ++j)
            f[i][j] = x[j]*(x[j] + y[i]);

    BicubicSpline spline(x.begin(), x.end(), y.begin(), y.end(), f);

    Real old_result = spline(x[2]+0.1, y[4]);

    // modify input matrix and update.
    f[4][3] += 1.0;
    spline.update();

    Real new_result = spline(x[2]+0.1, y[4]);
    if (std::fabs(old_result-new_result) < 0.5) {
        BOOST_ERROR("Failed to update bicubic spline");
    }
}

BOOST_AUTO_TEST_CASE(testUnknownRichardsonExtrapolation) {
    BOOST_TEST_MESSAGE("Testing Richardson extrapolation with "
            "unknown order of convergence...");

    const Real stepSize = 0.01;

    const std::pair<Real, Real> testCases[] = {
            std::make_pair(1.0, 1.0), std::make_pair(1.0, -1.0),
            std::make_pair(2.0, 0.25), std::make_pair(2.0, -1.0),
            std::make_pair(3.0, 2.0), std::make_pair(3.0, -0.5),
            std::make_pair(4.0, 1.0), std::make_pair(4.0, 0.5)
    };

    for (auto testCase : testCases) {
        const RichardsonExtrapolation extrap(GF(testCase.first, testCase.second), stepSize);

        const Real calculated = extrap(4.0, 2.0);
        const Real diff = std::fabs(M_PI - calculated);

        const Real tol = std::pow(stepSize, testCase.first+1);

        if (diff > tol) {
            BOOST_ERROR("failed to reproduce Richardson extrapolation "
                    " with unknown order of convergence");
        }
    }

    const Real highOrder = RichardsonExtrapolation(GF(14.0, 1.0), 0.5)(4.0,2.0);
    if (std::fabs(highOrder - M_PI) > 1e-12) {
        BOOST_ERROR("failed to reproduce Richardson extrapolation "
                " with unknown order of convergence");
    }

    try {
        RichardsonExtrapolation(GF(16.0, 1.0), 0.5)(4.0,2.0);
        BOOST_ERROR("Richardson extrapolation with order of"
            " convergence above 15 should throw exception");
    }
    catch (...) {}

    const Real limCosValue = RichardsonExtrapolation(limCos, 0.01)(4.0,2.0);
    if (std::fabs(limCosValue + 1.0) > 1e-6)
        BOOST_ERROR("failed to reproduce Richardson extrapolation "
                " with unknown order of convergence");
}

BOOST_AUTO_TEST_CASE(testRichardsonExtrapolation) {
    BOOST_TEST_MESSAGE("Testing Richardson extrapolation...");

    /* example taken from
     * http://www.ipvs.uni-stuttgart.de/abteilungen/bv/lehre/
     *      lehrveranstaltungen/vorlesungen/WS0910/
     *      NSG_termine/dateien/Richardson.pdf
     */

    const Real stepSize = 0.1;
    const Real orderOfConvergence = 1.0;
    const RichardsonExtrapolation extrap(f, stepSize, orderOfConvergence);


    Real tol = 0.00002;
    Real expected = 2.71285;

    const Real scalingFactor = 2.0;
    Real calculated = extrap(scalingFactor);

    if (std::fabs(expected-calculated) > tol) {
        BOOST_ERROR("failed to reproduce Richardson extrapolation");
    }

    calculated = extrap();
    if (std::fabs(expected-calculated) > tol) {
        BOOST_ERROR("failed to reproduce Richardson extrapolation");
    }

    expected = 2.721376;
    const Real scalingFactor2 = 4.0;
    calculated = extrap(scalingFactor2, scalingFactor);

    if (std::fabs(expected-calculated) > tol) {
        BOOST_ERROR("failed to reproduce Richardson extrapolation");
    }
}

BOOST_AUTO_TEST_CASE(testNoArbSabrInterpolation, *precondition(if_speed(Fast))){

    BOOST_TEST_MESSAGE("Testing no-arbitrage Sabr interpolation...");

    // Test SABR function against input volatilities
#ifndef __FAST_MATH__
    Real tolerance = 1.0e-12;
#else
    Real tolerance = 1.0e-8;
#endif
    std::vector<Real> strikes(31);
    std::vector<Real> volatilities(31), volatilities2(31);
    // input strikes
    strikes[0] = 0.03 ; strikes[1] = 0.032 ; strikes[2] = 0.034 ;
    strikes[3] = 0.036 ; strikes[4] = 0.038 ; strikes[5] = 0.04 ;
    strikes[6] = 0.042 ; strikes[7] = 0.044 ; strikes[8] = 0.046 ;
    strikes[9] = 0.048 ; strikes[10] = 0.05 ; strikes[11] = 0.052 ;
    strikes[12] = 0.054 ; strikes[13] = 0.056 ; strikes[14] = 0.058 ;
    strikes[15] = 0.06 ; strikes[16] = 0.062 ; strikes[17] = 0.064 ;
    strikes[18] = 0.066 ; strikes[19] = 0.068 ; strikes[20] = 0.07 ;
    strikes[21] = 0.072 ; strikes[22] = 0.074 ; strikes[23] = 0.076 ;
    strikes[24] = 0.078 ; strikes[25] = 0.08 ; strikes[26] = 0.082 ;
    strikes[27] = 0.084 ; strikes[28] = 0.086 ; strikes[29] = 0.088;
    strikes[30] = 0.09;
    // input volatilities for noarb sabr (other than above
    // alpha is 0.2 here due to the restriction sigmaI <= 1.0 !)
    volatilities[0] = 0.773729077752926;
    volatilities[1] = 0.763916242454194;
    volatilities[2] = 0.754773878663612;
    volatilities[3] = 0.746222305031368;
    volatilities[4] = 0.738193023523582;
    volatilities[5] = 0.730629785825930;
    volatilities[6] = 0.723484825471685;
    volatilities[7] = 0.716716812668892;
    volatilities[8] = 0.710290301049393;
    volatilities[9] = 0.704174528906769;
    volatilities[10] = 0.698342635400901;
    volatilities[11] = 0.692771033345972;
    volatilities[12] = 0.687438902593476;
    volatilities[13] = 0.682327777297265;
    volatilities[14] = 0.677421206991904;
    volatilities[15] = 0.672704476238547;
    volatilities[16] = 0.668164371832768;
    volatilities[17] = 0.663788984329375;
    volatilities[18] = 0.659567547226380;
    volatilities[19] = 0.655490294349232;
    volatilities[20] = 0.651548341349061;
    volatilities[21] = 0.647733583657137;
    volatilities[22] = 0.644038608699086;
    volatilities[23] = 0.640456620061898;
    volatilities[24] = 0.636981371712714;
    volatilities[25] = 0.633607110719560;
    volatilities[26] = 0.630328527192861;
    volatilities[27] = 0.627140710386248;
    volatilities[28] = 0.624039110072250;
    volatilities[29] = 0.621019502453590;
    volatilities[30] = 0.618077959983455;

    Time expiry = 1.0;
    Real forward = 0.039;
    // input SABR coefficients (corresponding to the vols above)
    Real initialAlpha = 0.2;
    Real initialBeta = 0.6;
    Real initialNu = 0.02;
    Real initialRho = 0.01;
    // calculate SABR vols and compare with input vols
    NoArbSabrSmileSection noarbSabr(expiry, forward,
                                    {initialAlpha, initialBeta, initialNu, initialRho});
    for (Size i = 0; i < strikes.size(); i++) {
        Real calculatedVol = noarbSabr.volatility(strikes[i]);
        if (std::fabs(volatilities[i]-calculatedVol) > tolerance)
        BOOST_ERROR(
            "failed to calculate noarb-Sabr function at strike " << strikes[i]
            << "\n    expected:   " << volatilities[i]
            << "\n    calculated: " << calculatedVol
            << "\n    error:      " << std::fabs(calculatedVol-volatilities[i]));
    }

    // Test SABR calibration against input parameters
    Real betaGuess = 0.5;
    Real alphaGuess = 0.2 / std::pow(forward,betaGuess-1.0); // new default value for alpha
    Real nuGuess = std::sqrt(0.4);
    Real rhoGuess = 0.0;

    const bool vegaWeighted[]= {true, false};
    const bool isAlphaFixed[]= {true, false};
    const bool isBetaFixed[]= {true, false};
    const bool isNuFixed[]= {true, false};
    const bool isRhoFixed[]= {true, false};

    Real calibrationTolerance = 5.0e-6;
    // initialize optimization methods
    std::vector<ext::shared_ptr<OptimizationMethod>> methods_ = {
        ext::shared_ptr<OptimizationMethod>(new Simplex(0.01)),
        ext::shared_ptr<OptimizationMethod>(new LevenbergMarquardt(1e-8, 1e-8, 1e-8))
    };
    // Initialize end criteria
    ext::shared_ptr<EndCriteria> endCriteria(new
                  EndCriteria(100000, 100, 1e-8, 1e-8, 1e-8));
    // Test looping over all possibilities
    for (Size j=1; j<methods_.size(); ++j) { // skip simplex (gets caught in some cases)
        for (bool i : vegaWeighted) {
            for (bool k_a : isAlphaFixed) {
                for (Size k_b=0; k_b<1/*std::size(isBetaFixed)*/; ++k_b) { // keep beta fixed (all 4 params free is a problem for this kind of test)
                    for (bool k_n : isNuFixed) {
                        for (bool k_r : isRhoFixed) {
                            NoArbSabrInterpolation noarbSabrInterpolation(
                                strikes.begin(), strikes.end(), volatilities.begin(), expiry,
                                forward, k_a ? initialAlpha : alphaGuess,
                                isBetaFixed[k_b] ? initialBeta : betaGuess,
                                k_n ? initialNu : nuGuess, k_r ? initialRho : rhoGuess, k_a,
                                isBetaFixed[k_b], k_n, k_r, i, endCriteria, methods_[j], 1E-10);
                            noarbSabrInterpolation.update();

                            // Recover SABR calibration parameters
                            bool failed = false;
                            Real calibratedAlpha = noarbSabrInterpolation.alpha();
                            Real calibratedBeta = noarbSabrInterpolation.beta();
                            Real calibratedNu = noarbSabrInterpolation.nu();
                            Real calibratedRho = noarbSabrInterpolation.rho();
                            Real error;

                            // compare results: alpha
                            error = std::fabs(initialAlpha-calibratedAlpha);
                            if (error > calibrationTolerance) {
                                BOOST_ERROR("\nfailed to calibrate alpha Sabr parameter:" <<
                                            "\n    expected:        " << initialAlpha <<
                                            "\n    calibrated:      " << calibratedAlpha <<
                                            "\n    error:           " << error);
                                failed = true;
                            }
                            // Beta
                            error = std::fabs(initialBeta-calibratedBeta);
                            if (error > calibrationTolerance) {
                                BOOST_ERROR("\nfailed to calibrate beta Sabr parameter:" <<
                                            "\n    expected:        " << initialBeta <<
                                            "\n    calibrated:      " << calibratedBeta <<
                                            "\n    error:           " << error);
                                failed = true;
                            }
                            // Nu
                            error = std::fabs(initialNu-calibratedNu);
                            if (error > calibrationTolerance) {
                                BOOST_ERROR("\nfailed to calibrate nu Sabr parameter:" <<
                                            "\n    expected:        " << initialNu <<
                                            "\n    calibrated:      " << calibratedNu <<
                                            "\n    error:           " << error);
                                failed = true;
                            }
                            // Rho
                            error = std::fabs(initialRho-calibratedRho);
                            if (error > calibrationTolerance) {
                                BOOST_ERROR("\nfailed to calibrate rho Sabr parameter:" <<
                                            "\n    expected:        " << initialRho <<
                                            "\n    calibrated:      " << calibratedRho <<
                                            "\n    error:           " << error);
                                failed = true;
                            }

                            if (failed)
                                BOOST_TEST_MESSAGE("\nnoarb-Sabr calibration failure:"
                                                   << "\n    isAlphaFixed:    " << k_a
                                                   << "\n    isBetaFixed:     " << isBetaFixed[k_b]
                                                   << "\n    isNuFixed:       " << k_n
                                                   << "\n    isRhoFixed:      " << k_r
                                                   << "\n    vegaWeighted[i]: " << i);
                        }
                    }
                }
            }
        }
    }

}

BOOST_AUTO_TEST_CASE(testSabrSingleCases) {

    BOOST_TEST_MESSAGE("Testing Sabr calibration single cases...");

    // case #1
    // this fails with an exception thrown in 1.4, fixed in 1.5

    std::vector<Real> strikes = { 0.01, 0.01125, 0.0125, 0.01375, 0.0150 };
    std::vector<Real> vols = { 0.1667, 0.2020, 0.2785, 0.3279, 0.3727 };

    Real tte = 0.3833;
    Real forward = 0.011025;

    SABRInterpolation s0(strikes.begin(), strikes.end(), vols.begin(), tte, forward,
                         Null<Real>(), 0.25, Null<Real>(), Null<Real>(),
                         false, true, false, false);
    s0.update();

    if (s0.maxError() > 0.01 || s0.rmsError() > 0.01) {
        BOOST_ERROR("Sabr case #1 failed with max error ("
                      << s0.maxError() << ") and rms error (" << s0.rmsError()
                      << "), both should be < 0.01");
    }

}

BOOST_AUTO_TEST_CASE(testTransformations) {

    BOOST_TEST_MESSAGE("Testing Sabr and no-arbitrage Sabr transformation functions...");

    Real size = 25.0; // test inputs from [-size,size]^4

    Size N = 100000;

    Array x(4), y(4), z(4);
    std::vector<Real> s;
    std::vector<bool> fixed(4, false);
    std::vector<Real> params(4, 0.0);
    Real forward = 0.03;

    HaltonRsg h(4, 42, false, false);

    for (Size i = 0; i < 1E6; ++i) {

        s = h.nextSequence().value;
        for (Size j = 0; j < 4; ++j)
            x[j] = 2.0 * size * s[j] - size;

        // sabr
        y = QuantLib::detail::SABRSpecs().direct(x, fixed, params, forward);
        validateSabrParameters(y[0], y[1], y[2], y[3]);
        z = QuantLib::detail::SABRSpecs().inverse(y, fixed, params, forward);
        z = QuantLib::detail::SABRSpecs().direct(z, fixed, params, forward);
        if (!close(z[0], y[0], N) || !close(z[1], y[1], N) || !close(z[2], y[2], N) ||
            !close(z[3], y[3], N))
            BOOST_ERROR("SabrInterpolation: direct(inverse("
                        << y[0] << "," << y[1] << "," << y[2] << "," << y[3]
                        << ")) = (" << z[0] << "," << z[1] << "," << z[2] << ","
                        << z[3] << "), difference is (" << z[0] - y[0] << ","
                        << z[1] - y[1] << "," << z[2] - y[2] << ","
                        << z[3] - y[3] << ")");

        // noarb sabr
        y = QuantLib::detail::NoArbSabrSpecs().direct(x, fixed, params, forward);

        // we can not invoke the constructor, this would be too slow, so
        // we copy the parameter check here ...
        Real alpha = y[0];
        Real beta = y[1];
        Real nu = y[2];
        Real rho = y[3];
        QL_REQUIRE(beta >= QuantLib::detail::NoArbSabrModel::beta_min &&
                       beta <= QuantLib::detail::NoArbSabrModel::beta_max,
                   "beta (" << beta << ") out of bounds");
        Real sigmaI = alpha * std::pow(forward, beta - 1.0);
        QL_REQUIRE(sigmaI >= QuantLib::detail::NoArbSabrModel::sigmaI_min &&
                       sigmaI <= QuantLib::detail::NoArbSabrModel::sigmaI_max,
                   "sigmaI = alpha*forward^(beta-1.0) ("
                       << sigmaI << ") out of bounds, alpha=" << alpha
                       << " beta=" << beta << " forward=" << forward);
        QL_REQUIRE(nu >= QuantLib::detail::NoArbSabrModel::nu_min &&
                       nu <= QuantLib::detail::NoArbSabrModel::nu_max,
                   "nu (" << nu << ") out of bounds");
        QL_REQUIRE(rho >= QuantLib::detail::NoArbSabrModel::rho_min &&
                       rho <= QuantLib::detail::NoArbSabrModel::rho_max,
                   "rho (" << rho << ") out of bounds");

        z = QuantLib::detail::NoArbSabrSpecs().inverse(y, fixed, params, forward);
        z = QuantLib::detail::NoArbSabrSpecs().direct(z, fixed, params, forward);
        if (!close(z[0], y[0], N) || !close(z[1], y[1], N) || !close(z[2], y[2], N) ||
            !close(z[3], y[3], N))
            BOOST_ERROR("NoArbSabrInterpolation: direct(inverse("
                        << y[0] << "," << y[1] << "," << y[2] << "," << y[3]
                        << ")) = (" << z[0] << "," << z[1] << "," << z[2] << ","
                        << z[3] << "), difference is (" << z[0] - y[0] << ","
                        << z[1] - y[1] << "," << z[2] - y[2] << ","
                        << z[3] - y[3] << ")");
    }
}

BOOST_AUTO_TEST_CASE(testFlochKennedySabrIsSmoothAroundATM) {
    BOOST_TEST_MESSAGE("Testing that Andersen SABR formula is smooth "
                       "close to the ATM level...");

    const Real f0    = 1.1;
    const Real alpha = 0.35;
    const Real nu    = 1.1;
    const Real rho   = 0.25;
    const Real beta  = 0.3;
    const Real strike= f0;
    const Time t = 2.1;

    const Real vol = sabrFlochKennedyVolatility(strike, f0, t, alpha, beta, nu, rho);

    const Real expected = 0.3963883944;
    const Real tol = 1e-8;
    const Real diff = std::fabs(expected - vol);
    if (diff > tol) {
        BOOST_ERROR("\nfailed to get ATM value :" <<
                    "\n    expected:   " << expected <<
                    "\n    calculated: " << vol <<
                    "\n    diff:      " << diff);
    }

    Real k = 0.996*strike;
    Real v = sabrFlochKennedyVolatility(k, f0, t, alpha, beta, nu, rho);

    for (; k < 1.004*strike; k += 0.0001*strike) {
        const Real vt = sabrFlochKennedyVolatility(k, f0, t, alpha, beta, nu, rho);

        const Real diff = std::fabs(v - vt);

        if (diff > 1e-5) {
            BOOST_ERROR("\nSabr vol spike around ATM :" <<
                        "\n    volatility at " << k-0.0001*strike <<
                        " is " << v <<
                        "\n    volatility at " << k << " is " << vt <<
                        "\n    difference: " << diff <<
                        "\n    tolerance : " << 1e-5);
        }
        v = vt;
    }
}

BOOST_AUTO_TEST_CASE(testLeFlochKennedySabrExample) {
    BOOST_TEST_MESSAGE("Testing Le Floc'h Kennedy SABR Example...");

    /*
    Example is taken from F. Le Floc'h, G. Kennedy:
     Explicit SABR Calibration through Simple Expansions.
     https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2467231
    */

    const Real f0    = 1.0;
    const Real alpha = 0.35;
    const Real nu    = 1.0;
    const Real rho   = 0.25;
    const Real beta  = 0.25;
    const Real strikes[]= {1.0, 1.5, 0.5};
    const Time t = 2.0;

    const Real expected[] = {0.408702473958, 0.428489933046, 0.585701651161};

    for (Size i=0; i < std::size(strikes); ++i) {
        const Real strike = strikes[i];
        const Real vol =
            sabrFlochKennedyVolatility(strike, f0, t, alpha, beta, nu, rho);

        const Real tol = 1e-8;
        const Real diff = std::fabs(expected[i] - vol);

        if (diff > tol) {
            BOOST_ERROR("\nfailed to reproduce reference examples :" <<
                        "\n    expected:   " << expected[i] <<
                        "\n    calculated: " << vol <<
                        "\n    diff:       " << diff);
        }
    }
}

BOOST_AUTO_TEST_CASE(testLagrangeInterpolation) {

    BOOST_TEST_MESSAGE("Testing Lagrange interpolation...");

    const std::vector<Real> x = {-1.0 , -0.5, -0.25, 0.1, 0.4, 0.75, 0.96};
    Array y(x.size());
    std::transform(x.begin(), x.end(), y.begin(), &lagrangeTestFct);

    LagrangeInterpolation interpl(x.begin(), x.end(), y.begin());

    // reference results are taken from R package pracma
    const Real references[] = {
        -0.5000000000000000,-0.5392414024347419,-0.5591485962711904,
        -0.5629199661387594,-0.5534414777017116,-0.5333043347921566,
        -0.5048221831582063,-0.4700478608272949,-0.4307896950846587,
        -0.3886273460669714,-0.3449271969711449,-0.3008572908782903,
        -0.2574018141928359,-0.2153751266968088,-0.1754353382192734,
        -0.1380974319209344,-0.1037459341938971,-0.0726471311765894,
        -0.0449608318838433,-0.0207516779521373,0.0000000000000000,
        0.0173877793964286,0.0315691961126723,0.0427562482700356,
        0.0512063534145595,0.0572137590808174,0.0611014067405497,
        0.0632132491361394,0.0639070209989264,0.0635474631523613,
        0.0625000000000000,0.0611248703983366,0.0597717119144768,
        0.0587745984686508,0.0584475313615655,0.0590803836865967,
        0.0609352981268212,0.0642435381368876,0.0692027925097279,
        0.0759749333281079,0.0846842273010179,0.0954160004849021,
        0.1082157563897290,0.1230887474699003,0.1400000000000001,
        0.1588747923353829,0.1795995865576031,0.2020234135046815,
        0.2259597111862140,0.2511886165833182,0.2774597108334206,
        0.3044952177998833,0.3319936560264689,0.3596339440766487,
        0.3870799592577457,0.4139855497299214,0.4400000000000001,
        0.4647739498001331,0.4879657663513030,0.5092483700116673,
        0.5283165133097421,0.5448945133624253,0.5587444376778583,
        0.5696747433431296,0.5775493695968156,0.5822972837863635,
        0.5839224807103117,0.5825144353453510,0.5782590089582251,
        0.5714498086024714,0.5625000000000000,0.5519545738075141,
        0.5405030652677689,0.5289927272456703,0.5184421566492137,
        0.5100553742352614,0.5052363578001620,0.5056040287552059,
        0.5130076920869246
    };

    constexpr double tol = 50*QL_EPSILON;
    for (Size i=0; i < 79; ++i) {
        const Real xx = -1.0 + i*0.025;
        const Real calculated = interpl(xx);
        if (   std::isnan(calculated)
            || std::fabs(references[i] - calculated) > tol) {
            BOOST_FAIL("failed to reproduce the Lagrange interpolation"
                    << "\n    x         : " << xx
                    << "\n    calculated: " << calculated
                    << "\n    expected  : " << references[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(testLagrangeInterpolationAtSupportPoint) {
    BOOST_TEST_MESSAGE(
        "Testing Lagrange interpolation at supporting points...");

    const Size n=5;
    Array x(n), y(n);
    for (Size i=0; i < n; ++i) {
        x[i] = i/Real(n);
        y[i] = 1.0/(1.0 - x[i]);
    }
    LagrangeInterpolation interpl(x.begin(), x.end(), y.begin());

    const Real relTol = 5e-12;

    for (Size i=1; i < n-1; ++i) {
        for (Real z = x[i] - 100*QL_EPSILON;
            z < x[i] + 100*QL_EPSILON; z+=2*QL_EPSILON) {
            const Real expected = 1.0/(1.0 - x[i]);
            const Real calculated = interpl(z);

            if (   std::isnan(calculated)
                || std::fabs(expected - calculated) > relTol) {
                BOOST_FAIL("failed to reproduce the Lagrange interplation"
                        << "\n    x         : " << z
                        << "\n    calculated: " << calculated
                        << "\n    expected  : " << expected);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testLagrangeInterpolationDerivative) {
    BOOST_TEST_MESSAGE(
        "Testing Lagrange interpolation derivatives...");

    Array x(5), y(5);
    x[0] = -1.0; y[0] = 2.0;
    x[1] = -0.3; y[1] = 3.0;
    x[2] =  0.1; y[2] = 6.0;
    x[3] =  0.3; y[3] = 3.0;
    x[4] =  0.9; y[4] =-1.0;

    LagrangeInterpolation interpl(x.begin(), x.end(), y.begin());

    const Real eps = std::sqrt(QL_EPSILON);
    for (Real x=-1.0; x <= 0.9; x+=0.01) {
        const Real calculated = interpl.derivative(x, true);
        const Real expected = (interpl(x+eps, true)
            - interpl(x-eps, true))/(2*eps);

        if (   std::isnan(calculated)
            || std::fabs(expected - calculated) > 25*eps) {
            BOOST_FAIL("failed to reproduce the Lagrange"
                    " interplation derivative"
                    << "\n    x         : " << x
                    << "\n    calculated: " << calculated
                    << "\n    expected  : " << expected);
        }
    }
}

BOOST_AUTO_TEST_CASE(testLagrangeInterpolationOnChebyshevPoints) {
    BOOST_TEST_MESSAGE(
        "Testing Lagrange interpolation on Chebyshev nodes...");

    // Test example taken from
    // J.P. Berrut, L.N. Trefethen, Barycentric Lagrange Interpolation
    // https://people.maths.ox.ac.uk/trefethen/barycentric.pdf

    const Size n=50;
    Array x(n+1), y(n+1);
    for (Size i=0; i <= n; ++i) {
        // Chebyshev nodes
        x[i] = std::cos( (2*i+1)*M_PI/(2*n+2) );
        y[i] = std::exp(x[i])/std::cos(x[i]);
    }

    LagrangeInterpolation interpl(x.begin(), x.end(), y.begin());

    const Real tol = 1e-13;
    const Real tolDeriv = 1e-11;

    for (Real x=-1.0; x <= 1.0; x+=0.03) {
        const Real calculated = interpl(x, true);
        const Real expected = std::exp(x)/std::cos(x);

        const Real diff = std::fabs(expected - calculated);
        if (std::isnan(calculated) || diff > tol) {
            BOOST_FAIL("failed to reproduce the Lagrange"
                    " interpolation on Chebyshev nodes"
                    << "\n    x         : " << x
                    << "\n    calculated: " << calculated
                    << "\n    expected  : " << expected
                    << std::scientific
                    << "\n    difference: " << diff);
        }

        const Real calculatedDeriv = interpl.derivative(x, true);
        const Real expectedDeriv = std::exp(x)*(std::cos(x) + std::sin(x)) / squared(std::cos(x));

        const Real diffDeriv = std::fabs(expectedDeriv - calculatedDeriv);
        if (std::isnan(calculated) || diffDeriv > tolDeriv) {
            BOOST_FAIL("failed to reproduce the Lagrange"
                    " interpolation derivative on Chebyshev nodes"
                    << "\n    x         : " << x
                    << "\n    calculated: " << calculatedDeriv
                    << "\n    expected  : " << expectedDeriv
                    << std::scientific
                    << "\n    difference: " << diffDeriv);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBSplines) {
    BOOST_TEST_MESSAGE("Testing B-Splines...");

    // reference values have been generate with the R package splines2
    // https://cran.r-project.org/web/packages/splines2/splines2.pdf

    std::vector<Real> knots = { -1.0, 0.5, 0.75, 1.2, 3.0, 4.0, 5.0 };

    const Natural p = 2;
    const BSpline bspline(p, knots.size()-p-2, knots);

    std::vector<std::tuple<Natural, Real, Real>> referenceValues = {
        {0, -0.95, 9.5238095238e-04},
        {0, -0.01, 0.37337142857},
        {0, 0.49, 0.84575238095},
        {0, 1.21, 0.0},
        {1, 1.49, 0.562987654321},
        {1, 1.59, 0.490888888889},
        {2, 1.99, 0.62429409171},
        {3, 1.19, 0.0},
        {3, 1.99, 0.12382936508},
        {3, 3.59, 0.765914285714}
    };

    const Real tol = 1e-10;
    for (const auto& [idx, x, expected] : referenceValues) {

        const Real calculated = bspline(idx, x);

        if (   std::isnan(calculated)
            || std::fabs(calculated - expected) > tol) {
            BOOST_FAIL("failed to reproduce the B-Spline value"
                    << "\n    i         : " << idx
                    << "\n    x         : " << x
                    << "\n    calculated: " << calculated
                    << "\n    expected  : " << expected
                    << "\n    difference: " << std::fabs(calculated-expected)
                    << "\n    tolerance : " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBackwardFlatOnSinglePoint) {
    BOOST_TEST_MESSAGE("Testing piecewise constant interpolation on a "
                       "single point...");
    const std::vector<Real> knots(1, 1.0), values(1, 2.5);

    const Interpolation impl(BackwardFlat().interpolate(
        knots.begin(), knots.end(), values.begin()));

    const Real x[] = { -1.0, 1.0, 2.0, 3.0 };

    for (Real i : x) {
        const Real calculated = impl(i, true);
        const Real expected = values[0];

        if (!close_enough(calculated, expected)) {
            BOOST_FAIL("failed to reproduce a piecewise constant "
                       "interpolation on a single point "
                       << "\n   x         : " << i << "\n   expected  : " << expected
                       << "\n   calculated: " << calculated);
        }

        const Real expectedPrimitive = values[0] * (i - knots[0]);
        const Real calculatedPrimitive = impl.primitive(i, true);

        if (!close_enough(calculatedPrimitive, expectedPrimitive)) {
            BOOST_FAIL("failed to reproduce primitive on a piecewise constant "
                       "interpolation for a single point "
                       << "\n   x         : " << i << "\n   expected  : " << expectedPrimitive
                       << "\n   calculated: " << calculatedPrimitive);
        }
    }
}

BOOST_AUTO_TEST_CASE(testChebyshevInterpolation) {
    BOOST_TEST_MESSAGE("Testing Chebyshev interpolation...");

    const auto fcts =
        std::vector<std::pair<std::function<Real(Real)>, std::string> >{
        {[](Real x) { return std::sin(x); }, "sin"},
        {[](Real x) { return std::cos(x); }, "cos"},
        {[](Real x) { return std::exp(-x*x); }, "e^(-x*x)"}
    };

    const auto tests = std::vector<std::pair<Size, Real> >{
        {11, 1e-5},
        {20, 1e-11}
    };

    for (const auto& t: tests) {
        for (const auto& fct: fcts) {
            ChebyshevInterpolation interp(t.first, fct.first);

            for (Real x=-0.99; x < 1.0; x+=0.01) {
                const Real expected = fct.first(x);
                const Real calculated = interp(x);
                const Real diff = std::fabs(expected-calculated);
                const Real tol = t.second;

                if (   std::isnan(calculated)
                    || std::fabs(calculated - expected) > tol) {
                    BOOST_FAIL("failed to reproduce the Chebyshev interpolation values"
                            << "\n    x         : " << x
                            << "\n    fct       : " << fct.second
                            << "\n    calculated: " << calculated
                            << "\n    expected  : " << expected
                            << "\n    difference: " << diff
                            << "\n    tolerance : " << tol);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testChebyshevInterpolationOnNodes) {
    BOOST_TEST_MESSAGE("Testing Chebyshev interpolation on and around nodes...");

    constexpr double tol = 10*QL_EPSILON;
    const auto testFct = [](Real x) { return std::sin(x);};

    const Size nrNodes = 7;
    Array y(nrNodes);

    for (auto pointType: {ChebyshevInterpolation::FirstKind,
                          ChebyshevInterpolation::SecondKind}) {

        const Array nodes = ChebyshevInterpolation::nodes(nrNodes, pointType);
        std::transform(std::begin(nodes), std::end(nodes), std::begin(y), testFct);

        const ChebyshevInterpolation interp(y, pointType);
        for (auto node: nodes) {
            // test on Chebyshev node
            const Real expected = testFct(node);
            const Real calculated = interp(node);
            const Real diff = std::abs(expected - calculated);

            if (diff > tol) {
                BOOST_ERROR("failed to reproduce the node values"
                        << std::setprecision(16)
                        << "\n    node      : " << node
                        << "\n    calculated: " << calculated
                        << "\n    expected  : " << expected
                        << "\n    difference: " << diff
                        << "\n    tolerance : " << tol);
            }


            // check around Chebyshev node
            for (Integer i=-50; i < 50; ++i) {
                const Real x = node + i*QL_EPSILON;
                const Real expected = testFct(x);
                const Real calculated = interp(x, true);
                const Real diff = std::abs(expected - calculated);

                if (diff > tol) {
                    BOOST_ERROR("failed to reproduce values around nodes"
                            << std::setprecision(16)
                            << "\n    node      : " << node
                            << "\n    epsilon   : " << x - node
                            << "\n    calculated: " << calculated
                            << "\n    expected  : " << expected
                            << "\n    difference: " << diff
                            << "\n    tolerance : " << tol);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testChebyshevInterpolationUpdateY) {
    BOOST_TEST_MESSAGE("Testing Y update for Chebyshev interpolation...");

    Array y({1, 4, 7, 4});
    ChebyshevInterpolation interp(y);

    Array yd({6, 4, 5, 6});
    interp.updateY(yd);

    constexpr double tol = 10*QL_EPSILON;

    for (Size i=0; i < y.size(); ++i) {
        const Real expected = yd[i];
        const Real calculated = interp(interp.nodes()[i], true);
        const Real diff = std::abs(calculated - expected);

        if (diff > tol) {
            BOOST_ERROR("failed to reproduce updated node values"
                    << std::setprecision(16)
                    << "\n    node      : " << i
                    << "\n    expected  : " << expected
                    << "\n    calculated: " << calculated
                    << "\n    difference: " << diff
                    << "\n    tolerance : " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testLaplaceInterpolation) {
    BOOST_TEST_MESSAGE("Testing Laplace interpolation...");

    Real tol = 1E-12;
    Real na = Null<Real>();

    // full matrix

    Matrix m1 = {
        {1.0, 2.0, 4.0},
        {6.0, 6.5, 7.0},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m1);

    BOOST_CHECK_CLOSE(m1(0, 0), 1.0, tol);
    BOOST_CHECK_CLOSE(m1(0, 2), 4.0, tol);
    BOOST_CHECK_CLOSE(m1(2, 0), 5.0, tol);
    BOOST_CHECK_CLOSE(m1(2, 1), 3.0, tol);

    // inner point

    Matrix m2 = {
        {1.0, 2.0, 4.0},
        {6.0, na, 7.0},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m2);

    BOOST_CHECK_CLOSE(m2(1, 1), 4.5, tol);
    BOOST_CHECK_CLOSE(m2(2, 1), 3.0, tol);

    // boundaries

    Matrix m3 = {
        {1.0, na, 4.0},
        {6.0, 6.5, 7.0},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m3);

    BOOST_CHECK_CLOSE(m3(0, 1), 2.5, tol);

    Matrix m4 = {
        {1.0, 2.0, 4.0},
        {na, 6.5, 7.0},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m4);

    BOOST_CHECK_CLOSE(m4(1, 0), 3.0, tol);

    Matrix m5 = {
        {1.0, 2.0, 4.0},
        {6.0, 6.5, 7.0},
        {5.0, na, 2.0},
    };

    laplaceInterpolation(m5);

    BOOST_CHECK_CLOSE(m5(2, 1), 3.5, tol);

    Matrix m6 = {
        {1.0, 2.0, 4.0},
        {6.0, 6.5, na},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m6);

    BOOST_CHECK_CLOSE(m6(1, 2), 3.0, tol);

    // corners

    Matrix m7 = {
        {na, 2.0, 4.0},
        {6.0, 6.5, 7.0},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m7);

    BOOST_CHECK_CLOSE(m7(0, 0), 4.0, tol);

    Matrix m8 = {
        {1.0, 2.0, 4.0},
        {6.0, 6.5, 7.0},
        {na, 3.0, 2.0},
    };

    laplaceInterpolation(m8);

    BOOST_CHECK_CLOSE(m8(2, 0), 4.5, tol);

    Matrix m9 = {
        {1.0, 2.0, 4.0},
        {6.0, 6.5, 7.0},
        {5.0, 3.0, na},
    };

    laplaceInterpolation(m9);

    BOOST_CHECK_CLOSE(m9(2, 2), 5.0, tol);

    Matrix m10 = {
        {1.0, 2.0, na},
        {6.0, 6.5, 7.0},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m10);

    BOOST_CHECK_CLOSE(m10(0, 2), 4.5, tol);

    // one dim (col vector)

    Matrix m20 = { {na}, {na}, {3.0}, {5.0}, {7.0}, {na} };

    laplaceInterpolation(m20);

    BOOST_CHECK_CLOSE(m20(0, 0), 3.0, tol);
    BOOST_CHECK_CLOSE(m20(1, 0), 3.0, tol);
    BOOST_CHECK_CLOSE(m20(5, 0), 7.0, tol);

    // one dim (row vector)

    Matrix m21 = {{na, na, 3.0, 5.0, 7.0, na}};

    laplaceInterpolation(m21);

    BOOST_CHECK_CLOSE(m21(0, 0), 3.0, tol);
    BOOST_CHECK_CLOSE(m21(0, 1), 3.0, tol);
    BOOST_CHECK_CLOSE(m21(0, 5), 7.0, tol);

    // non equidistant grid, inner point

    Matrix m30 = {
        {1.0, 2.0, 4.0},
        {6.0, na, 7.0},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m30, {1.0, 2.0, 4.0}, {1.0, 2.0, 4.0});

    BOOST_CHECK_CLOSE(m30(1, 1), 26.0 / 6.0, tol);

    // non equidistant grid, boundaries

    Matrix m31 = {
        {1.0, na, 4.0},
        {6.0, 6.5, 7.0},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m31, {1.0, 2.0, 4.0}, {1.0, 2.0, 4.0});

    BOOST_CHECK_CLOSE(m31(0, 1), 6.0 / 3.0, tol);

    Matrix m32 = {
        {1.0, 2.0, 4.0},
        {na, 6.5, 7.0},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m32, {1.0, 2.0, 4.0}, {1.0, 2.0, 4.0});

    BOOST_CHECK_CLOSE(m32(1, 0), 7.0 / 3.0, tol);

    Matrix m33 = {
        {1.0, 2.0, 4.0},
        {6.0, 6.5, 7.0},
        {5.0, na, 2.0},
    };

    laplaceInterpolation(m33, {1.0, 2.0, 4.0}, {1.0, 2.0, 4.0});

    BOOST_CHECK_CLOSE(m33(2, 1), 12.0 / 3.0, tol);

    Matrix m34 = {
        {1.0, 2.0, 4.0},
        {6.0, 6.5, na},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m34, {1.0, 2.0, 4.0}, {1.0, 2.0, 4.0});

    BOOST_CHECK_CLOSE(m34(1, 2), 10.0 / 3.0, tol);

    // non equidistant grid, corners

    Matrix m35 = {
        {na, 2.0, 4.0},
        {6.0, 6.5, 7.0},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m35, {1.0, 2.0, 4.0}, {1.0, 3.0, 7.0});

    BOOST_CHECK_CLOSE(m35(0, 0), 10.0 / 3.0, tol);

    Matrix m36 = {
        {1.0, 2.0, 4.0},
        {6.0, 6.5, 7.0},
        {na, 3.0, 2.0},
    };

    laplaceInterpolation(m36, {1.0, 2.0, 4.0}, {1.0, 3.0, 7.0});

    BOOST_CHECK_CLOSE(m36(2, 0), 18.0 / 5.0, tol);

    Matrix m37 = {
        {1.0, 2.0, 4.0},
        {6.0, 6.5, 7.0},
        {5.0, 3.0, na},
    };

    laplaceInterpolation(m37, {1.0, 2.0, 4.0}, {1.0, 3.0, 7.0});

    BOOST_CHECK_CLOSE(m37(2, 2), 13.0 / 3.0, tol);

    Matrix m38 = {
        {1.0, 2.0, na},
        {6.0, 6.5, 7.0},
        {5.0, 3.0, 2.0},
    };

    laplaceInterpolation(m38, {1.0, 2.0, 4.0}, {1.0, 2.0, 3.0});

    BOOST_CHECK_CLOSE(m38(0, 2), 16.0 / 3.0, tol);

    // single point with given value

    Matrix m50 = {
        {1.0},
    };

    laplaceInterpolation(m50);

    BOOST_CHECK_CLOSE(m50(0, 0), 1.0, tol);

    // single point with missing value

    Matrix m51 = {
        {Null<Real>()},
    };

    laplaceInterpolation(m51);

    BOOST_CHECK_CLOSE(m51(0, 0), 0.0, tol);

    // no point

    LaplaceInterpolation l0([](const std::vector<Size>&) { return Null<Real>(); }, {});
    BOOST_CHECK_CLOSE(l0({}), 0.0, tol);

    // single test cases from actual issues observed in the field

    std::vector<Real> tx = {0.0849315, 0.257534, 0.509589, 1.00548, 2.00274, 3.00274, 4.00274,
                            5.00548,   7.00822,  10.0082,  15.011,  20.0137, 30.0219, 70.0493};
    std::vector<Real> ty = {0.25, 1, 2, 3, 4, 5, 7, 10, 15, 20, 30, 100};
    Matrix m52 = {{na, na, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, na},
                  {na, na, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, na},
                  {na, na, na, na, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, na},
                  {na, na, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, na},
                  {na, na, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, na},
                  {na, na, na, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, na},
                  {na, na, na, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, na},
                  {na, na, na, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, na},
                  {na, na, na, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, na, na},
                  {na, na, na, na, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, na, na},
                  {na, na, na, na, na, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, na, na},
                  {na, na, na, na, na, na, na, na, na, na, na, na, na, na}};

    // we need to allow for more iterations to achieve the desired accuracy
    laplaceInterpolation(m52, tx, ty, 1E-6, 100);

    for (auto const& v : m52) {
        BOOST_CHECK_CLOSE(v, 1.0, 0.1);
    }

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
