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

#include "interpolations.hpp"
#include "utilities.hpp"
#include <ql/utilities/dataformatters.hpp>
#include <ql/utilities/null.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/math/interpolations/forwardflatinterpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/multicubicspline.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <ql/math/interpolations/kernelinterpolation.hpp>
#include <ql/math/interpolations/kernelinterpolation2d.hpp>
#include <ql/math/interpolations/lagrangeinterpolation.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/math/bspline.hpp>
#include <ql/math/kernelfunctions.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/richardsonextrapolation.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/experimental/volatility/noarbsabrinterpolation.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define BEGIN(x) (x+0)
#define END(x) (x+LENGTH(x))

namespace {

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
        errorFunction(const F& f) : f_(f) {}
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

}


/* See J. M. Hyman, "Accurate monotonicity preserving cubic interpolation"
   SIAM J. of Scientific and Statistical Computing, v. 4, 1983, pp. 645-654.
   http://math.lanl.gov/~mac/papers/numerics/H83.pdf
*/
void InterpolationTest::testSplineErrorOnGaussianValues() {

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

    for (Size i=0; i<LENGTH(points); i++) {
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
void InterpolationTest::testSplineOnGaussianValues() {

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
void InterpolationTest::testSplineOnRPN15AValues() {

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
                                    BEGIN(RPN15A_x), END(RPN15A_x),
                                    BEGIN(RPN15A_y),
                                    CubicInterpolation::Spline, false,
                                    CubicInterpolation::SecondDerivative, 0.0,
                                    CubicInterpolation::SecondDerivative, 0.0);
    f.update();
    checkValues("Natural spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
    check2ndDerivativeValue("Natural spline", f,
                            *BEGIN(RPN15A_x), 0.0);
    check2ndDerivativeValue("Natural spline", f,
                            *(END(RPN15A_x)-1), 0.0);
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
    f = CubicInterpolation(BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y),
                           CubicInterpolation::Spline, false,
                           CubicInterpolation::FirstDerivative, 0.0,
                           CubicInterpolation::FirstDerivative, 0.0);
    f.update();
    checkValues("Clamped spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
    check1stDerivativeValue("Clamped spline", f,
                            *BEGIN(RPN15A_x), 0.0);
    check1stDerivativeValue("Clamped spline", f,
                            *(END(RPN15A_x)-1), 0.0);
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
    f = CubicInterpolation(BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y),
                           CubicInterpolation::Spline, false,
                           CubicInterpolation::NotAKnot, Null<Real>(),
                           CubicInterpolation::NotAKnot, Null<Real>());
    f.update();
    checkValues("Not-a-knot spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
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
    f = CubicInterpolation(BEGIN(RPN15A_x), END(RPN15A_x),
                           BEGIN(RPN15A_y),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::SecondDerivative, 0.0,
                           CubicInterpolation::SecondDerivative, 0.0);
    f.update();
    checkValues("MC natural spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
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
    f = CubicInterpolation(BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::FirstDerivative, 0.0,
                           CubicInterpolation::FirstDerivative, 0.0);
    f.update();
    checkValues("MC clamped spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
    check1stDerivativeValue("MC clamped spline", f,
                            *BEGIN(RPN15A_x), 0.0);
    check1stDerivativeValue("MC clamped spline", f,
                            *(END(RPN15A_x)-1), 0.0);
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
    f = CubicInterpolation(BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y),
                           CubicInterpolation::Spline, true,
                           CubicInterpolation::NotAKnot, Null<Real>(),
                           CubicInterpolation::NotAKnot, Null<Real>());
    f.update();
    checkValues("MC not-a-knot spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
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
void InterpolationTest::testSplineOnGenericValues() {

    BOOST_TEST_MESSAGE("Testing spline interpolation on generic values...");

    const Real generic_x[] = { 0.0, 1.0, 3.0, 4.0 };
    const Real generic_y[] = { 0.0, 0.0, 2.0, 2.0 };
    const Real generic_natural_y2[] = { 0.0, 1.5, -1.5, 0.0 };

    Real interpolated, error;
    Size i, n = LENGTH(generic_x);
    std::vector<Real> x35(3);

    // Natural spline
    CubicInterpolation f(BEGIN(generic_x), END(generic_x),
                         BEGIN(generic_y),
                         CubicInterpolation::Spline, false,
                         CubicInterpolation::SecondDerivative,
                         generic_natural_y2[0],
                         CubicInterpolation::SecondDerivative,
                         generic_natural_y2[n-1]);
    f.update();
    checkValues("Natural spline", f,
                BEGIN(generic_x), END(generic_x), BEGIN(generic_y));
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
    f = CubicInterpolation(BEGIN(generic_x), END(generic_x), BEGIN(generic_y),
                    CubicInterpolation::Spline, false,
                    CubicInterpolation::FirstDerivative, y1a,
                    CubicInterpolation::FirstDerivative, y1b);
    f.update();
    checkValues("Clamped spline", f,
                BEGIN(generic_x), END(generic_x), BEGIN(generic_y));
    check1stDerivativeValue("Clamped spline", f,
                            *BEGIN(generic_x), 0.0);
    check1stDerivativeValue("Clamped spline", f,
                            *(END(generic_x)-1), 0.0);
    x35[0] = f(3.5);


    // Not-a-knot spline
    f = CubicInterpolation(BEGIN(generic_x), END(generic_x), BEGIN(generic_y),
                           CubicInterpolation::Spline, false,
                           CubicInterpolation::NotAKnot, Null<Real>(),
                           CubicInterpolation::NotAKnot, Null<Real>());
    f.update();
    checkValues("Not-a-knot spline", f,
                BEGIN(generic_x), END(generic_x), BEGIN(generic_y));
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


void InterpolationTest::testSimmetricEndConditions() {

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


void InterpolationTest::testDerivativeEndConditions() {

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
void InterpolationTest::testNonRestrictiveHymanFilter() {

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

void InterpolationTest::testMultiSpline() {
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

namespace {

    struct NotThrown {};

}

void InterpolationTest::testAsFunctor() {

    BOOST_TEST_MESSAGE("Testing use of interpolations as functors...");

    const Real x[] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
    const Real y[] = { 5.0, 4.0, 3.0, 2.0, 1.0 };

   Interpolation f = LinearInterpolation(BEGIN(x), END(x), BEGIN(y));
    f.update();

    const Real x2[] = { -2.0, -1.0, 0.0, 1.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
    Size N = LENGTH(x2);
    std::vector<Real> y2(N);
    Real tolerance = 1.0e-12;

    // case 1: extrapolation not allowed
    try {
        std::transform(BEGIN(x2), END(x2), y2.begin(), f);
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
    std::transform(BEGIN(x2), END(x2), y2.begin(), f);
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


namespace {

    Integer sign(Real y1, Real y2) {
        return y1 == y2 ? 0 :
               y1 < y2 ?  1 :
                         -1 ;
    }

}

void InterpolationTest::testFritschButland() {

    BOOST_TEST_MESSAGE("Testing Fritsch-Butland interpolation...");

    const Real x[5] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
    const Real y[][5] = {{ 1.0, 2.0, 1.0, 1.0, 2.0 },
                         { 1.0, 2.0, 1.0, 1.0, 1.0 },
                         { 2.0, 1.0, 0.0, 2.0, 3.0 }};

    for (Size i=0; i<3; ++i) {

        Interpolation f = FritschButlandCubic(BEGIN(x), END(x), BEGIN(y[i]));
        f.update();

        for (Size j=0; j<4; ++j) {
            Real left_knot = x[j];
            Integer expected_sign = sign(y[i][j], y[i][j+1]);
            for (Size k=0; k<10; ++k) {
                Real x1 = left_knot + k*0.1, x2 = left_knot + (k+1)*0.1;
                Real y1 = f(x1), y2 = f(x2);
                if (boost::math::isnan(y1))
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


void InterpolationTest::testBackwardFlat() {

    BOOST_TEST_MESSAGE("Testing backward-flat interpolation...");

    const Real x[] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
    const Real y[] = { 5.0, 4.0, 3.0, 2.0, 1.0 };

    Interpolation f = BackwardFlatInterpolation(BEGIN(x), END(x), BEGIN(y));
    f.update();

    Size N = LENGTH(x);
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

void InterpolationTest::testForwardFlat() {

    BOOST_TEST_MESSAGE("Testing forward-flat interpolation...");

    const Real x[] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
    const Real y[] = { 5.0, 4.0, 3.0, 2.0, 1.0 };

    Interpolation f = ForwardFlatInterpolation(BEGIN(x), END(x), BEGIN(y));
    f.update();

    Size N = LENGTH(x);
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

void InterpolationTest::testSabrInterpolation(){

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
    // Use default values (but not null, since then parameters
    // will then not be fixed during optimization, see the
    // interpolation constructor, thus rendering the test cases
    // with fixed parameters non-sensical)
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
    std::vector<ext::shared_ptr<OptimizationMethod> > methods_;
    methods_.push_back( ext::shared_ptr<OptimizationMethod>(new Simplex(0.01)));
    methods_.push_back( ext::shared_ptr<OptimizationMethod>(new LevenbergMarquardt(1e-8, 1e-8, 1e-8)));
    // Initialize end criteria
    ext::shared_ptr<EndCriteria> endCriteria(new
                  EndCriteria(100000, 100, 1e-8, 1e-8, 1e-8));
    // Test looping over all possibilities
    for (Size j=0; j<methods_.size(); ++j) {
      for (Size i=0; i<LENGTH(vegaWeighted); ++i) {
        for (Size k_a=0; k_a<LENGTH(isAlphaFixed); ++k_a) {
          for (Size k_b=0; k_b<LENGTH(isBetaFixed); ++k_b) {
            for (Size k_n=0; k_n<LENGTH(isNuFixed); ++k_n) {
              for (Size k_r=0; k_r<LENGTH(isRhoFixed); ++k_r) {
                  // to meet the tough calibration tolerance we need to lower the default
                  // error threshold for accepting a calibration (to be more specific, some
                  // of the new test cases arising from fixing a subset of the model's
                  // parameters do not calibrate with the desired error using the initial
                  // guess (i.e. optimization runs into a local minimum) - then a series of
                  // random start values for optimization is chosen until our tight custom
                  // error threshold is satisfied.
                  SABRInterpolation sabrInterpolation(
                      strikes.begin(), strikes.end(), volatilities.begin(),
                      expiry, forward, isAlphaFixed[k_a] ? initialAlpha : alphaGuess,
                      isBetaFixed[k_b] ? initialBeta : betaGuess,
                      isNuFixed[k_n] ? initialNu : nuGuess,
                      isRhoFixed[k_r] ? initialRho : rhoGuess, isAlphaFixed[k_a],
                      isBetaFixed[k_b], isNuFixed[k_n], isRhoFixed[k_r],
                      vegaWeighted[i], endCriteria, methods_[j], 1E-10);
                  sabrInterpolation.update();

                // Recover SABR calibration parameters
                bool failed = false;
                Real calibratedAlpha = sabrInterpolation.alpha();
                Real calibratedBeta = sabrInterpolation.beta();
                Real calibratedNu = sabrInterpolation.nu();
                Real calibratedRho = sabrInterpolation.rho();
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
                    BOOST_FAIL("\nSabr calibration failure:" <<
                               "\n    isAlphaFixed:    " << isAlphaFixed[k_a] <<
                               "\n    isBetaFixed:     " << isBetaFixed[k_b] <<
                               "\n    isNuFixed:       " << isNuFixed[k_n] <<
                               "\n    isRhoFixed:      " << isRhoFixed[k_r] <<
                               "\n    vegaWeighted[i]: " << vegaWeighted[i]);

              }
            }
          }
        }
      }
    }
}


void InterpolationTest::testKernelInterpolation() {

    BOOST_TEST_MESSAGE("Testing kernel 1D interpolation...");

    std::vector<Real> deltaGrid(5); // x-values, here delta in FX
    deltaGrid[0]=0.10; deltaGrid[1]=0.25; deltaGrid[2]=0.50;
    deltaGrid[3]=0.75; deltaGrid[4]=0.90;

    std::vector<Real> yd1(deltaGrid.size()); // test y-values 1
    yd1[0]=11.275; yd1[1]=11.125; yd1[2]=11.250;
    yd1[3]=11.825; yd1[4]=12.625;

    std::vector<Real> yd2(deltaGrid.size()); // test y-values 2
    yd2[0]=16.025; yd2[1]=13.450; yd2[2]=11.350;
    yd2[3]=10.150; yd2[4]=10.075;

    std::vector<Real> yd3(deltaGrid.size()); // test y-values 3
    yd3[0]=10.3000; yd3[1]=9.6375; yd3[2]=9.2000;
    yd3[3]=9.1125; yd3[4]=9.4000;

    std::vector<std::vector<Real> > yd;
    yd.push_back(yd1);
    yd.push_back(yd2);
    yd.push_back(yd3);

    std::vector<Real> lambdaVec(5);
    lambdaVec[0]=0.05; lambdaVec[1]=0.50; lambdaVec[2]=0.75;
    lambdaVec[3]=1.65; lambdaVec[4]=2.55;

    Real tolerance = 2.0e-5;

    Real expectedVal;
    Real calcVal;

    // Check that y-values at knots are exactly the feeded y-values,
    // irrespective of kernel parameters
    for (Size i=0; i<lambdaVec.size(); ++i) {
        GaussianKernel myKernel(0,lambdaVec[i]);

        for (Size j=0; j<yd.size(); ++j) {

            std::vector<Real> currY = yd[j];
            KernelInterpolation f(deltaGrid.begin(), deltaGrid.end(),
                                  currY.begin(), myKernel
#ifdef __FAST_MATH__
                                  ,1e-6
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

    std::vector<Real> testDeltaGrid(deltaGrid.size());
    testDeltaGrid[0]=0.121; testDeltaGrid[1]=0.279; testDeltaGrid[2]=0.678;
    testDeltaGrid[3]=0.790; testDeltaGrid[4]=0.980;

    // Gaussian Kernel values for testDeltaGrid with a standard
    // deviation of 2.05 (the value is arbitrary.)  Source: parrallel
    // implementation in R, no literature sources found

    std::vector<Real> ytd1(testDeltaGrid.size());
    ytd1[0]=11.23847; ytd1[1]=11.12003; ytd1[2]=11.58932;
    ytd1[3]=11.99168; ytd1[4]=13.29650;

    std::vector<Real> ytd2(testDeltaGrid.size());
    ytd2[0]=15.55922; ytd2[1]=13.11088; ytd2[2]=10.41615;
    ytd2[3]=10.05153; ytd2[4]=10.50741;

    std::vector<Real> ytd3(testDeltaGrid.size());
    ytd3[0]= 10.17473; ytd3[1]= 9.557842; ytd3[2]= 9.09339;
    ytd3[3]= 9.149687; ytd3[4]= 9.779971;

    std::vector<std::vector<Real> > ytd;
    ytd.push_back(ytd1);
    ytd.push_back(ytd2);
    ytd.push_back(ytd3);

    GaussianKernel myKernel(0,2.05);

    for (Size j=0; j< ytd.size(); ++j) {
        std::vector<Real> currY=yd[j];
        std::vector<Real> currTY=ytd[j];

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


void InterpolationTest::testKernelInterpolation2D(){

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


void InterpolationTest::testBicubicDerivatives() {
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


void InterpolationTest::testBicubicUpdate() {
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


namespace {
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
}

void InterpolationTest::testUnknownRichardsonExtrapolation() {
    BOOST_TEST_MESSAGE("Testing Richardson extrapolation with "
            "unknown order of convergence...");

    const Real stepSize = 0.01;

    const std::pair<Real, Real> testCases[] = {
            std::make_pair(1.0, 1.0), std::make_pair(1.0, -1.0),
            std::make_pair(2.0, 0.25), std::make_pair(2.0, -1.0),
            std::make_pair(3.0, 2.0), std::make_pair(3.0, -0.5),
            std::make_pair(4.0, 1.0), std::make_pair(4.0, 0.5)
    };

    for (Size i=0; i < LENGTH(testCases); ++i) {
        const std::pair<Real, Real> testCase = testCases[i];

        const RichardsonExtrapolation extrap(
            GF(testCase.first, testCase.second), stepSize);

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


namespace {
    Real f(Real h) {
        return std::pow( 1.0 + h, 1/h);
    }
}

void InterpolationTest::testRichardsonExtrapolation() {
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

void InterpolationTest::testNoArbSabrInterpolation(){

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
                                    boost::assign::list_of(initialAlpha)(
                                        initialBeta)(initialNu)(initialRho));
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
    std::vector<ext::shared_ptr<OptimizationMethod> > methods_;
    methods_.push_back( ext::shared_ptr<OptimizationMethod>(new Simplex(0.01)));
    methods_.push_back( ext::shared_ptr<OptimizationMethod>(new LevenbergMarquardt(1e-8, 1e-8, 1e-8)));
    // Initialize end criteria
    ext::shared_ptr<EndCriteria> endCriteria(new
                  EndCriteria(100000, 100, 1e-8, 1e-8, 1e-8));
    // Test looping over all possibilities
    for (Size j=1; j<methods_.size(); ++j) { // skip simplex (gets caught in some cases)
        for (Size i=0; i<LENGTH(vegaWeighted); ++i) {
            for (Size k_a=0; k_a<LENGTH(isAlphaFixed); ++k_a) {
                for (Size k_b=0; k_b<1/*LENGTH(isBetaFixed)*/; ++k_b) { // keep beta fixed (all 4 params free is a problem for this kind of test)
                    for (Size k_n=0; k_n<LENGTH(isNuFixed); ++k_n) {
                        for (Size k_r=0; k_r<LENGTH(isRhoFixed); ++k_r) {
                            NoArbSabrInterpolation noarbSabrInterpolation(
                                                                          strikes.begin(), strikes.end(),
                                                                          volatilities.begin(), expiry, forward,
                                                                          isAlphaFixed[k_a] ? initialAlpha
                                                                          : alphaGuess,
                                                                          isBetaFixed[k_b] ? initialBeta
                                                                          : betaGuess,
                                                                          isNuFixed[k_n] ? initialNu : nuGuess,
                                                                          isRhoFixed[k_r] ? initialRho : rhoGuess,
                                                                          isAlphaFixed[k_a], isBetaFixed[k_b],
                                                                          isNuFixed[k_n], isRhoFixed[k_r],
                                                                          vegaWeighted[i], endCriteria,
                                                                          methods_[j], 1E-10);
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
                                BOOST_TEST_MESSAGE("\nnoarb-Sabr calibration failure:" <<
                                           "\n    isAlphaFixed:    " << isAlphaFixed[k_a] <<
                                           "\n    isBetaFixed:     " << isBetaFixed[k_b] <<
                                           "\n    isNuFixed:       " << isNuFixed[k_n] <<
                                           "\n    isRhoFixed:      " << isRhoFixed[k_r] <<
                                           "\n    vegaWeighted[i]: " << vegaWeighted[i]);
                        }
                    }
                }
            }
        }
    }

}


void InterpolationTest::testSabrSingleCases() {

    BOOST_TEST_MESSAGE("Testing Sabr calibration single cases...");

    // case #1
    // this fails with an exception thrown in 1.4, fixed in 1.5

    using namespace boost::assign;
    std::vector<Real> strikes, vols;
    strikes += 0.01, 0.01125, 0.0125, 0.01375, 0.0150;
    vols += 0.1667, 0.2020, 0.2785, 0.3279, 0.3727;

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

void InterpolationTest::testTransformations() {

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
        y = detail::SABRSpecs().direct(x, fixed, params, forward);
        validateSabrParameters(y[0], y[1], y[2], y[3]);
        z = detail::SABRSpecs().inverse(y, fixed, params, forward);
        z = detail::SABRSpecs().direct(z, fixed, params, forward);
        if (!close(z[0], y[0], N) || !close(z[1], y[1], N) || !close(z[2], y[2], N) ||
            !close(z[3], y[3], N))
            BOOST_ERROR("SabrInterpolation: direct(inverse("
                        << y[0] << "," << y[1] << "," << y[2] << "," << y[3]
                        << ")) = (" << z[0] << "," << z[1] << "," << z[2] << ","
                        << z[3] << "), difference is (" << z[0] - y[0] << ","
                        << z[1] - y[1] << "," << z[2] - y[2] << ","
                        << z[3] - y[3] << ")");

        // noarb sabr
        y = detail::NoArbSabrSpecs().direct(x, fixed, params, forward);

        // we can not invoke the constructor, this would be too slow, so
        // we copy the parameter check here ...
        Real alpha = y[0];
        Real beta = y[1];
        Real nu = y[2];
        Real rho = y[3];
        QL_REQUIRE(beta >= detail::NoArbSabrModel::beta_min &&
                       beta <= detail::NoArbSabrModel::beta_max,
                   "beta (" << beta << ") out of bounds");
        Real sigmaI = alpha * std::pow(forward, beta - 1.0);
        QL_REQUIRE(sigmaI >= detail::NoArbSabrModel::sigmaI_min &&
                       sigmaI <= detail::NoArbSabrModel::sigmaI_max,
                   "sigmaI = alpha*forward^(beta-1.0) ("
                       << sigmaI << ") out of bounds, alpha=" << alpha
                       << " beta=" << beta << " forward=" << forward);
        QL_REQUIRE(nu >= detail::NoArbSabrModel::nu_min &&
                       nu <= detail::NoArbSabrModel::nu_max,
                   "nu (" << nu << ") out of bounds");
        QL_REQUIRE(rho >= detail::NoArbSabrModel::rho_min &&
                       rho <= detail::NoArbSabrModel::rho_max,
                   "rho (" << rho << ") out of bounds");

        z = detail::NoArbSabrSpecs().inverse(y, fixed, params, forward);
        z = detail::NoArbSabrSpecs().direct(z, fixed, params, forward);
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

void InterpolationTest::testFlochKennedySabrIsSmoothAroundATM() {
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

void InterpolationTest::testLeFlochKennedySabrExample() {
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

    for (Size i=0; i < LENGTH(strikes); ++i) {
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

namespace {
    Real lagrangeTestFct(Real x) {
        return std::fabs(x) + 0.5*x - x*x;
    }
}

void InterpolationTest::testLagrangeInterpolation() {

    BOOST_TEST_MESSAGE("Testing Lagrange interpolation...");

    const Real x[] = {-1.0 , -0.5, -0.25, 0.1, 0.4, 0.75, 0.96};
    Array y(LENGTH(x));
    std::transform(x, x+LENGTH(x), y.begin(), &lagrangeTestFct);

    LagrangeInterpolation interpl(&x[0], x+LENGTH(x), y.begin());

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

    const Real tol = 50*QL_EPSILON;
    for (Size i=0; i < 79; ++i) {
        const Real xx = -1.0 + i*0.025;
        const Real calculated = interpl(xx);
        if (   boost::math::isnan(calculated)
            || std::fabs(references[i] - calculated) > tol) {
            BOOST_FAIL("failed to reproduce the Lagrange interpolation"
                    << "\n    x         : " << xx
                    << "\n    calculated: " << calculated
                    << "\n    expected  : " << references[i]);
        }
    }
}

void InterpolationTest::testLagrangeInterpolationAtSupportPoint() {
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

            if (   boost::math::isnan(calculated)
                || std::fabs(expected - calculated) > relTol) {
                BOOST_FAIL("failed to reproduce the Lagrange interplation"
                        << "\n    x         : " << z
                        << "\n    calculated: " << calculated
                        << "\n    expected  : " << expected);
            }
        }
    }
}

void InterpolationTest::testLagrangeInterpolationDerivative() {
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

        if (   boost::math::isnan(calculated)
            || std::fabs(expected - calculated) > 25*eps) {
            BOOST_FAIL("failed to reproduce the Lagrange"
                    " interplation derivative"
                    << "\n    x         : " << x
                    << "\n    calculated: " << calculated
                    << "\n    expected  : " << expected);
        }
    }
}

void InterpolationTest::testLagrangeInterpolationOnChebyshevPoints() {
    BOOST_TEST_MESSAGE(
        "Testing Lagrange interpolation on Chebyshev points...");

    // Test example taken from
    // J.P. Berrut, L.N. Trefethen, Barycentric Lagrange Interpolation
    // https://people.maths.ox.ac.uk/trefethen/barycentric.pdf

    const Size n=50;
    Array x(n+1), y(n+1);
    for (Size i=0; i <= n; ++i) {
        // Chebyshev points
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
        if (   boost::math::isnan(calculated) || diff > tol) {
            BOOST_FAIL("failed to reproduce the Lagrange"
                    " interpolation on Chebyshev points"
                    << "\n    x         : " << x
                    << "\n    calculated: " << calculated
                    << "\n    expected  : " << expected
                    << std::scientific
                    << "\n    difference: " << diff);
        }

        const Real calculatedDeriv = interpl.derivative(x, true);
        const Real expectedDeriv = std::exp(x)*(std::cos(x) + std::sin(x))
                / square<Real>()(std::cos(x));

        const Real diffDeriv = std::fabs(expectedDeriv - calculatedDeriv);
        if (   boost::math::isnan(calculated) || diffDeriv > tolDeriv) {
            BOOST_FAIL("failed to reproduce the Lagrange"
                    " interpolation derivative on Chebyshev points"
                    << "\n    x         : " << x
                    << "\n    calculated: " << calculatedDeriv
                    << "\n    expected  : " << expectedDeriv
                    << std::scientific
                    << "\n    difference: " << diffDeriv);
        }
    }
}

void InterpolationTest::testBSplines() {
    BOOST_TEST_MESSAGE("Testing B-Splines...");

    // reference values have been generate with the R package splines2
    // https://cran.r-project.org/web/packages/splines2/splines2.pdf

    using namespace boost::assign;

    std::vector<Real> knots;
    knots += -1.0, 0.5, 0.75, 1.2, 3.0, 4.0, 5.0;

    const Natural p = 2;
    const BSpline bspline(p, knots.size()-p-2, knots);

    std::vector<boost::tuple<Natural, Real, Real> > referenceValues;
    referenceValues += boost::make_tuple(0, -0.95, 9.5238095238e-04),
        boost::make_tuple(0, -0.01, 0.37337142857),
        boost::make_tuple(0, 0.49, 0.84575238095),
        boost::make_tuple(0, 1.21, 0.0),
        boost::make_tuple(1, 1.49, 0.562987654321),
        boost::make_tuple(1, 1.59, 0.490888888889),
        boost::make_tuple(2, 1.99, 0.62429409171),
        boost::make_tuple(3, 1.19, 0.0),
        boost::make_tuple(3, 1.99, 0.12382936508),
        boost::make_tuple(3, 3.59, 0.765914285714);


    const Real tol = 1e-10;
    for (Size i=0; i < referenceValues.size(); ++i) {
        const Natural idx = referenceValues[i].get<0>();
        const Real x = referenceValues[i].get<1>();
        const Real expected = referenceValues[i].get<2>();

        const Real calculated = bspline(idx, x);

        if (   boost::math::isnan(calculated)
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

void InterpolationTest::testBackwardFlatOnSinglePoint() {
    BOOST_TEST_MESSAGE("Testing piecewise constant interpolation on a "
                       "single point...");
    const std::vector<Real> knots(1, 1.0), values(1, 2.5);

    const Interpolation impl(BackwardFlat().interpolate(
        knots.begin(), knots.end(), values.begin()));

    const Real x[] = { -1.0, 1.0, 2.0, 3.0 };

    for (Size i=0; i < LENGTH(x); ++i) {
        const Real calculated = impl(x[i], true);
        const Real expected = values[0];

        if (!close_enough(calculated, expected)) {
            BOOST_FAIL("failed to reproduce a piecewise constant "
                    "interpolation on a single point "
                    << "\n   x         : " << x[i]
                    << "\n   expected  : " << expected
                    << "\n   calculated: " << calculated);
        }

        const Real expectedPrimitive = values[0]*(x[i] - knots[0]);
        const Real calculatedPrimitive = impl.primitive(x[i], true);

        if (!close_enough(calculatedPrimitive, expectedPrimitive)) {
            BOOST_FAIL("failed to reproduce primitive on a piecewise constant "
                    "interpolation for a single point "
                    << "\n   x         : " << x[i]
                    << "\n   expected  : " << expectedPrimitive
                    << "\n   calculated: " << calculatedPrimitive);
        }
    }
}

test_suite* InterpolationTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Interpolation tests");

    suite->add(QUANTLIB_TEST_CASE(
                        &InterpolationTest::testSplineOnGenericValues));
    suite->add(QUANTLIB_TEST_CASE(
                        &InterpolationTest::testSimmetricEndConditions));
    suite->add(QUANTLIB_TEST_CASE(
                        &InterpolationTest::testDerivativeEndConditions));
    suite->add(QUANTLIB_TEST_CASE(
                        &InterpolationTest::testNonRestrictiveHymanFilter));
    suite->add(QUANTLIB_TEST_CASE(
                        &InterpolationTest::testSplineOnRPN15AValues));
    suite->add(QUANTLIB_TEST_CASE(
                        &InterpolationTest::testSplineOnGaussianValues));
    suite->add(QUANTLIB_TEST_CASE(
                        &InterpolationTest::testSplineErrorOnGaussianValues));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testMultiSpline));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testAsFunctor));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testFritschButland));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testBackwardFlat));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testForwardFlat));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testSabrInterpolation));
    suite->add(QUANTLIB_TEST_CASE(
        &InterpolationTest::testFlochKennedySabrIsSmoothAroundATM));
    suite->add(QUANTLIB_TEST_CASE(
        &InterpolationTest::testLeFlochKennedySabrExample));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testKernelInterpolation));
    suite->add(QUANTLIB_TEST_CASE(
                              &InterpolationTest::testKernelInterpolation2D));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testBicubicDerivatives));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testBicubicUpdate));
    suite->add(QUANTLIB_TEST_CASE(
        &InterpolationTest::testUnknownRichardsonExtrapolation));
    suite->add(QUANTLIB_TEST_CASE(
                            &InterpolationTest::testRichardsonExtrapolation));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testNoArbSabrInterpolation));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testSabrSingleCases));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testTransformations));
    suite->add(QUANTLIB_TEST_CASE(
        &InterpolationTest::testLagrangeInterpolation));
    suite->add(QUANTLIB_TEST_CASE(
        &InterpolationTest::testLagrangeInterpolationAtSupportPoint));
    suite->add(QUANTLIB_TEST_CASE(
        &InterpolationTest::testLagrangeInterpolationDerivative));
    suite->add(QUANTLIB_TEST_CASE(
        &InterpolationTest::testLagrangeInterpolationOnChebyshevPoints));
    suite->add(QUANTLIB_TEST_CASE(&InterpolationTest::testBSplines));

    suite->add(QUANTLIB_TEST_CASE(
        &InterpolationTest::testBackwardFlatOnSinglePoint));


    return suite;
}
