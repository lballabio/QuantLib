
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "interpolations.hpp"
#include "utilities.hpp"
#include <ql/null.hpp>
#include <ql/Math/cubicspline.hpp>
#ifndef QL_PATCH_BORLAND
#include <ql/Math/multicubicspline.hpp>
#endif
#include <ql/Math/simpsonintegral.hpp>
#include <ql/Math/functional.hpp>
#include <ql/RandomNumbers/sobolrsg.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    #define BEGIN(x) (x+0)
    #define END(x) (x+LENGTH(x))

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
                     const CubicSpline& spline,
                     I xBegin, I xEnd, J yBegin) {
        Real tolerance = 2.0e-15;
        while (xBegin != xEnd) {
            Real interpolated = spline(*xBegin);
            if (std::fabs(interpolated-*yBegin) > tolerance) {
                BOOST_FAIL(std::string(type) +
                           " interpolation failed at x = " +
                           DecimalFormatter::toString(*xBegin) +
                           "\n    interpolated value: " +
                           DecimalFormatter::toExponential(interpolated) +
                           "\n    expected value:     " +
                           DecimalFormatter::toExponential(*yBegin) +
                           "\n    error:              "
                           + DecimalFormatter::toExponential(
                                            std::fabs(interpolated-*yBegin)));
            }
            ++xBegin; ++yBegin;
        }
    }

    void check1stDerivativeValue(const char* type,
                                 const CubicSpline& spline,
                                 Real x,
                                 Real value) {
        Real tolerance = 1.0e-14;
        Real interpolated = spline.derivative(x);
        Real error = std::fabs(interpolated-value);
        if (error > tolerance) {
            BOOST_FAIL(std::string(type) +
                       " interpolation first derivative failure\n"
                       "at x="
                       + DecimalFormatter::toString(x) +
                       "\n    interpolated value: "
                       + DecimalFormatter::toString(interpolated) +
                       "\n    expected value:     "
                       + DecimalFormatter::toString(value) +
                       "\n    error:              "
                       + DecimalFormatter::toExponential(error));
        }
    }

    void check2ndDerivativeValue(const char* type,
                                 const CubicSpline& spline,
                                 Real x,
                                 Real value) {
        Real tolerance = 1.0e-13;
        Real interpolated = spline.secondDerivative(x);
        Real error = std::fabs(interpolated-value);
        if (error > tolerance) {
            BOOST_FAIL(std::string(type) +
                       " interpolation second derivative failure\n"
                       "at x="
                       + DecimalFormatter::toString(x) +
                       "\n    interpolated value: "
                       + DecimalFormatter::toString(interpolated) +
                       "\n    expected value:     "
                       + DecimalFormatter::toString(value) +
                       "\n    error:              "
                       + DecimalFormatter::toExponential(error));
        }
    }

    void checkNotAKnotCondition(const char* type,
                                const CubicSpline& spline) {

        Real tolerance = 1.0e-14;
        const std::vector<Real>& c = spline.cCoefficients();
        if (std::fabs(c[0]-c[1]) > tolerance) {
            BOOST_FAIL(std::string(type) +
                       " interpolation failure"
                       "\n    cubic coefficient of the first"
                       " polinomial is "
                       + DecimalFormatter::toString(c[0]) +
                       "\n    cubic coefficient of the second"
                       " polinomial is "
                       + DecimalFormatter::toString(c[1]));
        }
        Size n = c.size();
        if (std::fabs(c[n-2]-c[n-1]) > tolerance) {
            BOOST_FAIL(std::string(type) +
                       " interpolation failure"
                       "\n    cubic coefficient of the 2nd to last"
                       " polinomial is "
                       + DecimalFormatter::toString(c[n-2]) +
                       "\n    cubic coefficient of the last"
                       " polinomial is "
                       + DecimalFormatter::toString(c[n-1]));
        }
    }

    void checkSymmetry(const char* type,
                       const CubicSpline& spline,
                       Real xMin) {
        Real tolerance = 1.0e-15;
        for (Real x = xMin; x < 0.0; x += 0.1) {
            Real y1 = spline(x), y2 = spline(-x);
            if (std::fabs(y1-y2) > tolerance) {
                BOOST_FAIL(std::string(type) +
                           " interpolation not symmetric"
                           "\n    x = "   + DecimalFormatter::toString(x) +
                           "\n    g(x)  = " + DecimalFormatter::toString(y1) +
                           "\n    g(-x) = " + DecimalFormatter::toString(y2) +
                           "\n    error:              "
                           + DecimalFormatter::toExponential(
                                                           std::fabs(y1-y2)));
            }
        }
    }

    template <class F>
    class errorFunction : public std::unary_function<Real,Real> {
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

}

/* See J. M. Hyman, "Accurate monotonicity preserving cubic interpolation"
   SIAM J. of Scientific and Statistical Computing, v. 4, 1983, pp. 645-654.
   http://math.lanl.gov/~mac/papers/numerics/H83.pdf
*/
void InterpolationTest::testSplineErrorOnGaussianValues() {

    BOOST_MESSAGE("Testing spline approximation on Gaussian data sets...");

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

    SimpsonIntegral integral(1e-12);
    std::vector<Real> x, y;

    // still unexplained scale factor needed to obtain the numerical
    // results from the paper
    Real scaleFactor = 1.9;

    for (Size i=0; i<LENGTH(points); i++) {
        Size n = points[i];
        std::vector<Real> x = xRange(-1.7, 1.9, n);
        std::vector<Real> y = gaussian(x);

        // Not-a-knot
        CubicSpline f = CubicSpline(x.begin(), x.end(), y.begin(),
                                    CubicSpline::NotAKnot, Null<Real>(),
                                    CubicSpline::NotAKnot, Null<Real>(),
                                    false);
        Real result = std::sqrt(integral(make_error_function(f), -1.7, 1.9));
        result /= scaleFactor;
        if (std::fabs(result-tabulatedErrors[i]) > toleranceOnTabErr[i])
            BOOST_FAIL("Not-a-knot spline interpolation "
                       "\n    sample points:      " +
                       SizeFormatter::toString(n) +
                       "\n    norm of difference: " +
                       DecimalFormatter::toExponential(result,1) +
                       "\n    it should be:       " +
                       DecimalFormatter::toExponential(tabulatedErrors[i],1));

        // MC not-a-knot
        f = MonotonicCubicSpline(x.begin(), x.end(), y.begin(),
                                 CubicSpline::NotAKnot, Null<Real>(),
                                 CubicSpline::NotAKnot, Null<Real>());
        result = std::sqrt(integral(make_error_function(f), -1.7, 1.9));
        result /= scaleFactor;
        if (std::fabs(result-tabulatedMCErrors[i]) > toleranceOnTabMCErr[i])
            BOOST_FAIL("MC Not-a-knot spline interpolation "
                       "\n    sample points:      " +
                       SizeFormatter::toString(n) +
                       "\n    norm of difference: " +
                       DecimalFormatter::toExponential(result,1) +
                       "\n    it should be:       " +
                       DecimalFormatter::toExponential(
                                                     tabulatedMCErrors[i],1));
    }

}

/* See J. M. Hyman, "Accurate monotonicity preserving cubic interpolation"
   SIAM J. of Scientific and Statistical Computing, v. 4, 1983, pp. 645-654.
   http://math.lanl.gov/~mac/papers/numerics/H83.pdf
*/
void InterpolationTest::testSplineOnGaussianValues() {

    BOOST_MESSAGE("Testing spline interpolation on a Gaussian data set...");

    Real interpolated, interpolated2;
    Size n = 5;

    std::vector<Real> x(n), y(n);
    Real x1_bad=-1.7, x2_bad=1.7;

    for (Real start = -1.9, j=0; j<2; start+=0.2, j++) {
        x = xRange(start, start+3.6, n);
        y = gaussian(x);

        // Not-a-knot spline
        CubicSpline f = CubicSpline(x.begin(), x.end(), y.begin(),
                                    CubicSpline::NotAKnot, Null<Real>(),
                                    CubicSpline::NotAKnot, Null<Real>(),
                                    false);
        checkValues("Not-a-knot spline", f,
                    x.begin(), x.end(), y.begin());
        checkNotAKnotCondition("Not-a-knot spline", f);
        // bad performance
        interpolated = f(x1_bad);
        interpolated2= f(x2_bad);
        if (interpolated>0.0 && interpolated2>0.0 ) {
            BOOST_FAIL("Not-a-knot spline interpolation "
                       "bad performance unverified"
                       "\nat x="
                       + DecimalFormatter::toString(x1_bad) +
                       " interpolated value: "
                       + DecimalFormatter::toString(interpolated) +
                       "\nat x="
                       + DecimalFormatter::toString(x2_bad) +
                       " interpolated value: "
                       + DecimalFormatter::toString(interpolated) +
                       "\n at least one of them was expected to be < 0.0");
        }

        // MC not-a-knot spline
        f = MonotonicCubicSpline(x.begin(), x.end(), y.begin(),
                                 CubicSpline::NotAKnot, Null<Real>(),
                                 CubicSpline::NotAKnot, Null<Real>());
        checkValues("MC not-a-knot spline", f,
                    x.begin(), x.end(), y.begin());
        // good performance
        interpolated = f(x1_bad);
        if (interpolated<0.0) {
            BOOST_FAIL("MC not-a-knot spline interpolation "
                       "good performance unverified\n"
                       "at x="
                       + DecimalFormatter::toString(x1_bad) +
                       "\ninterpolated value: "
                       + DecimalFormatter::toString(interpolated) +
                       "\nexpected value > 0.0");
        }
        interpolated = f(x2_bad);
        if (interpolated<0.0) {
            BOOST_FAIL("MC not-a-knot spline interpolation "
                       "good performance unverified\n"
                       "at x="
                       + DecimalFormatter::toString(x2_bad) +
                       "\ninterpolated value: "
                       + DecimalFormatter::toString(interpolated) +
                       "\nexpected value > 0.0");
        }
    }
}


/* See J. M. Hyman, "Accurate monotonicity preserving cubic interpolation"
   SIAM J. of Scientific and Statistical Computing, v. 4, 1983, pp. 645-654.
   http://math.lanl.gov/~mac/papers/numerics/H83.pdf
*/
void InterpolationTest::testSplineOnRPN15AValues() {

    BOOST_MESSAGE("Testing spline interpolation on RPN15A data set...");

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
    CubicSpline f = NaturalCubicSpline(BEGIN(RPN15A_x), END(RPN15A_x),
                                       BEGIN(RPN15A_y));
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
        BOOST_FAIL("Natural spline interpolation "
                   "poor performance unverified\n"
                   "at x="
                   + DecimalFormatter::toString(x_bad) +
                   "\ninterpolated value: "
                   + DecimalFormatter::toString(interpolated) +
                   "\nexpected value > 1.0");
    }


    // Clamped spline
    f = CubicSpline(BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y),
                    CubicSpline::FirstDerivative, 0.0,
                    CubicSpline::FirstDerivative, 0.0,
                    false);
    checkValues("Clamped spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
    check1stDerivativeValue("Clamped spline", f,
                            *BEGIN(RPN15A_x), 0.0);
    check1stDerivativeValue("Clamped spline", f,
                            *(END(RPN15A_x)-1), 0.0);
    // poor performance
    interpolated = f(x_bad);
    if (interpolated<1.0) {
        BOOST_FAIL("Clamped spline interpolation "
                   "poor performance unverified\n"
                   "at x="
                   + DecimalFormatter::toString(x_bad) +
                   "\ninterpolated value: "
                   + DecimalFormatter::toString(interpolated) +
                   "\nexpected value > 1.0");
    }


    // Not-a-knot spline
    f = CubicSpline(BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y),
                    CubicSpline::NotAKnot, Null<Real>(),
                    CubicSpline::NotAKnot, Null<Real>(),
                    false);
    checkValues("Not-a-knot spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
    checkNotAKnotCondition("Not-a-knot spline", f);
    // poor performance
    interpolated = f(x_bad);
    if (interpolated<1.0) {
        BOOST_FAIL("Not-a-knot spline interpolation "
                   "poor performance unverified\n"
                   "at x="
                   + DecimalFormatter::toString(x_bad) +
                   "\ninterpolated value: "
                   + DecimalFormatter::toString(interpolated) +
                   "\nexpected value > 1.0");
    }


    // MC natural spline values
    f = NaturalMonotonicCubicSpline(BEGIN(RPN15A_x), END(RPN15A_x),
                                    BEGIN(RPN15A_y));
    checkValues("MC natural spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
    // good performance
    interpolated = f(x_bad);
    if (interpolated>1.0) {
        BOOST_FAIL("MC natural spline interpolation "
                   "good performance unverified\n"
                   "at x="
                   + DecimalFormatter::toString(x_bad) +
                   "\ninterpolated value: "
                   + DecimalFormatter::toString(interpolated) +
                   "\nexpected value < 1.0");
    }


    // MC clamped spline values
    f = MonotonicCubicSpline(BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y),
                             CubicSpline::FirstDerivative, 0.0,
                             CubicSpline::FirstDerivative, 0.0);
    checkValues("MC clamped spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
    check1stDerivativeValue("MC clamped spline", f,
                            *BEGIN(RPN15A_x), 0.0);
    check1stDerivativeValue("MC clamped spline", f,
                            *(END(RPN15A_x)-1), 0.0);
    // good performance
    interpolated = f(x_bad);
    if (interpolated>1.0) {
        BOOST_FAIL("MC clamped spline interpolation "
                   "good performance unverified\n"
                   "at x="
                   + DecimalFormatter::toString(x_bad) +
                   "\ninterpolated value: "
                   + DecimalFormatter::toString(interpolated) +
                   "\nexpected value < 1.0");
    }


    // MC not-a-knot spline values
    f = MonotonicCubicSpline(BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y),
                             CubicSpline::NotAKnot, Null<Real>(),
                             CubicSpline::NotAKnot, Null<Real>());
    checkValues("MC not-a-knot spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
    // good performance
    interpolated = f(x_bad);
    if (interpolated>1.0) {
        BOOST_FAIL("MC clamped spline interpolation "
                   "good performance unverified\n"
                   "at x="
                   + DecimalFormatter::toString(x_bad) +
                   "\ninterpolated value: "
                   + DecimalFormatter::toString(interpolated) +
                   "\nexpected value < 1.0");
    }
}

/* Blossey, Frigyik, Farnum "A Note On Cubic Splines"
   Applied Linear Algebra and Numerical Analysis AMATH 352 Lecture Notes
   http://www.amath.washington.edu/courses/352-winter-2002/spline_note.pdf
*/
void InterpolationTest::testSplineOnGenericValues() {

    BOOST_MESSAGE("Testing spline interpolation on generic values...");

    const Real generic_x[] = { 0.0, 1.0, 3.0, 4.0 };
    const Real generic_y[] = { 0.0, 0.0, 2.0, 2.0 };
    const Real generic_natural_y2[] = { 0.0, 1.5, -1.5, 0.0 };

    Real interpolated, error;
    Size i, n = LENGTH(generic_x);
    std::vector<Real> x35(3);

    // Natural spline
    CubicSpline f = CubicSpline(BEGIN(generic_x), END(generic_x),
                                BEGIN(generic_y),
                                CubicSpline::SecondDerivative,
                                generic_natural_y2[0],
                                CubicSpline::SecondDerivative,
                                generic_natural_y2[n-1],
                                false);
    checkValues("Natural spline", f,
                BEGIN(generic_x), END(generic_x), BEGIN(generic_y));
    // cached second derivative
    for (i=0; i<n; i++) {
        interpolated = f.secondDerivative(generic_x[i]);
        error = interpolated - generic_natural_y2[i];
        if (std::fabs(error)>3e-16) {
            BOOST_FAIL("Natural spline interpolation "
                       "second derivative failed at x="
                       + DecimalFormatter::toString(generic_x[i]) +
                       "\ninterpolated value: "
                       + DecimalFormatter::toString(interpolated) +
                       "\nexpected value:     "
                       + DecimalFormatter::toString(generic_natural_y2[i]) +
                       "\nerror:              "
                       + DecimalFormatter::toExponential(error));
        }
    }
    x35[1] = f(3.5);


    // Clamped spline
    Real y1a = 0.0, y1b = 0.0;
    f = CubicSpline(BEGIN(generic_x), END(generic_x), BEGIN(generic_y),
                    CubicSpline::FirstDerivative, y1a,
                    CubicSpline::FirstDerivative, y1b,
                    false);
    checkValues("Clamped spline", f,
                BEGIN(generic_x), END(generic_x), BEGIN(generic_y));
    check1stDerivativeValue("Clamped spline", f,
                            *BEGIN(generic_x), 0.0);
    check1stDerivativeValue("Clamped spline", f,
                            *(END(generic_x)-1), 0.0);
    x35[0] = f(3.5);


    // Not-a-knot spline
    f = CubicSpline(BEGIN(generic_x), END(generic_x), BEGIN(generic_y),
                    CubicSpline::NotAKnot, Null<Real>(),
                    CubicSpline::NotAKnot, Null<Real>(),
                    false);
    checkValues("Not-a-knot spline", f,
                BEGIN(generic_x), END(generic_x), BEGIN(generic_y));
    checkNotAKnotCondition("Not-a-knot spline", f);

    x35[2] = f(3.5);

    if (x35[0]>x35[1] || x35[1]>x35[2]) {
        BOOST_FAIL("Spline interpolation failure"
                   "\nat x="
                   + DecimalFormatter::toString(3.5) +
                   "\nclamped spline    " + DecimalFormatter::toString(x35[0])+
                   "\nnatural spline    " + DecimalFormatter::toString(x35[1])+
                   "\nnot-a-knot spline " + DecimalFormatter::toString(x35[2])+
                   "\nvalues should be in increasing order");
    }
}


void InterpolationTest::testSimmetricEndConditions() {

    BOOST_MESSAGE("Testing symmetry of spline interpolation "
                  "end-conditions...");

    Size n = 9;

    std::vector<Real> x, y;
    x = xRange(-1.8, 1.8, n);
    y = gaussian(x);

    // Not-a-knot spline
    CubicSpline f = CubicSpline(x.begin(), x.end(), y.begin(),
                                CubicSpline::NotAKnot, Null<Real>(),
                                CubicSpline::NotAKnot, Null<Real>(),
                                false);
    checkValues("Not-a-knot spline", f,
                x.begin(), x.end(), y.begin());
    checkNotAKnotCondition("Not-a-knot spline", f);
    checkSymmetry("Not-a-knot spline", f, x[0]);


    // MC not-a-knot spline
    f = MonotonicCubicSpline(x.begin(), x.end(), y.begin(),
                             CubicSpline::NotAKnot, Null<Real>(),
                             CubicSpline::NotAKnot, Null<Real>());
    checkValues("MC not-a-knot spline", f,
                x.begin(), x.end(), y.begin());
    checkSymmetry("MC not-a-knot spline", f, x[0]);
}


void InterpolationTest::testDerivativeEndConditions() {

    BOOST_MESSAGE("Testing derivative end-conditions "
                  "for spline interpolation...");

    Size n = 4;

    std::vector<Real> x, y;
    x = xRange(-2.0, 2.0, n);
    y = parabolic(x);

    // Not-a-knot spline
    CubicSpline f = CubicSpline(x.begin(), x.end(), y.begin(),
                                CubicSpline::NotAKnot, Null<Real>(),
                                CubicSpline::NotAKnot, Null<Real>(),
                                false);
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
    f = CubicSpline(x.begin(), x.end(), y.begin(),
                    CubicSpline::FirstDerivative,  4.0,
                    CubicSpline::FirstDerivative, -4.0,
                    false);
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
    f = CubicSpline(x.begin(), x.end(), y.begin(),
                    CubicSpline::SecondDerivative, -2.0,
                    CubicSpline::SecondDerivative, -2.0,
                    false);
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
    f = CubicSpline(x.begin(), x.end(), y.begin(),
                    CubicSpline::NotAKnot, Null<Real>(),
                    CubicSpline::NotAKnot, Null<Real>(),
                    true);
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
    f = CubicSpline(x.begin(), x.end(), y.begin(),
                    CubicSpline::FirstDerivative,  4.0,
                    CubicSpline::FirstDerivative, -4.0,
                    true);
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
    f = CubicSpline(x.begin(), x.end(), y.begin(),
                    CubicSpline::SecondDerivative, -2.0,
                    CubicSpline::SecondDerivative, -2.0,
                    true);
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
   "Nonnegativity-, Monotonicity-, or Convexity-Preserving Cubic and Quintic
   Hermite Interpolation"
   Mathematics Of Computation, v. 52, n. 186, April 1989, pp. 471-494.
*/
void InterpolationTest::testNonRestrictiveHymanFilter() {

    BOOST_MESSAGE("Testing non-restrictive Hyman filter...");

    Size n = 4;

    std::vector<Real> x, y;
    x = xRange(-2.0, 2.0, n);
    y = parabolic(x);
    Real zero=0.0, interpolated, expected=0.0;

    // MC Not-a-knot spline
    Interpolation f = CubicSpline(x.begin(), x.end(), y.begin(),
                                  CubicSpline::NotAKnot, Null<Real>(),
                                  CubicSpline::NotAKnot, Null<Real>(),
                                  true);
    interpolated = f(zero);
    if (std::fabs(interpolated-expected)>1e-15) {
        BOOST_FAIL("MC not-a-knot spline"
                   " interpolation failed at x = " +
                   DecimalFormatter::toString(zero) +
                   "\n    interpolated value: " +
                   DecimalFormatter::toString(interpolated) +
                   "\n    expected value:     " +
                   DecimalFormatter::toString(expected) +
                   "\n    error:              "
                   + DecimalFormatter::toExponential(
                                           std::fabs(interpolated-expected)));
    }


    // MC Clamped spline
    f = CubicSpline(x.begin(), x.end(), y.begin(),
                    CubicSpline::FirstDerivative,  4.0,
                    CubicSpline::FirstDerivative, -4.0,
                    true);
    interpolated = f(zero);
    if (std::fabs(interpolated-expected)>1e-15) {
        BOOST_FAIL("MC clamped spline"
                   " interpolation failed at x = " +
                   DecimalFormatter::toString(zero) +
                   "\n    interpolated value: " +
                   DecimalFormatter::toString(interpolated) +
                   "\n    expected value:     " +
                   DecimalFormatter::toString(expected) +
                   "\n    error:              "
                   + DecimalFormatter::toExponential(
                                           std::fabs(interpolated-expected)));
    }


    // MC SecondDerivative spline
    f = CubicSpline(x.begin(), x.end(), y.begin(),
                    CubicSpline::SecondDerivative, -2.0,
                    CubicSpline::SecondDerivative, -2.0,
                    true);
    if (std::fabs(interpolated-expected)>1e-15) {
        BOOST_FAIL("MC SecondDerivative spline"
                   " interpolation failed at x = " +
                   DecimalFormatter::toString(zero) +
                   "\n    interpolated value: " +
                   DecimalFormatter::toString(interpolated) +
                   "\n    expected value:     " +
                   DecimalFormatter::toString(expected) +
                   "\n    error:              "
                   + DecimalFormatter::toExponential(
                                           std::fabs(interpolated-expected)));
    }

}

#ifdef QL_PATCH_BORLAND
void InterpolationTest::testMultiSpline() {
    BOOST_FAIL("\n  N-dimensional cubic spline test SKIPPED!!!");
}
#else
void InterpolationTest::testMultiSpline() {

    BOOST_MESSAGE("Testing N-dimensional cubic spline...");

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

    r = 0.01;

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
                            BOOST_FAIL("\n  At ("
                                + DecimalFormatter::toString(s) + ","
                                + DecimalFormatter::toString(t) + ","
                                + DecimalFormatter::toString(u) + ","
                                + DecimalFormatter::toString(v) + ","
                                + DecimalFormatter::toString(w) + "):"
                                "\n    interpolated: "
                                + DecimalFormatter::toString(interpolated,4) +
                                "\n    actual value: "
                                + DecimalFormatter::toString(expected,4) +
                                "\n       error: "
                                + DecimalFormatter::toExponential(error,1) +
                                "\n    tolerance: "
                                + DecimalFormatter::toExponential(tolerance,1)
                                );
                        }
                    }


    unsigned long seed = 42;
    SobolRsg rsg(5, seed);

    Real tolerance = 1.7e-4;
    // actually tested up to 2^21-1=2097151 Sobol draws
    for (i = 0; i < 1023; ++i) {
        const Array& next = rsg.nextSequence().value;
        s = grid[0].front() + next[0]*(grid[0].back()-grid[0].front());
        t = grid[1].front() + next[1]*(grid[1].back()-grid[1].front());
        u = grid[2].front() + next[2]*(grid[2].back()-grid[2].front());
        v = grid[3].front() + next[3]*(grid[3].back()-grid[3].front());
        w = grid[4].front() + next[4]*(grid[4].back()-grid[4].front());
        Real interpolated = cs(args), expected = multif(s, t, u, v, w);
        Real error = std::fabs(interpolated-expected);
        if (error > tolerance) {
            BOOST_FAIL("\n  At (" + DecimalFormatter::toString(s) + ","
                       + DecimalFormatter::toString(t) + ","
                       + DecimalFormatter::toString(u) + ","
                       + DecimalFormatter::toString(v) + ","
                       + DecimalFormatter::toString(w) + "):"
                       "\n    interpolated: "
                       + DecimalFormatter::toString(interpolated,4) +
                       "\n    actual value: "
                       + DecimalFormatter::toString(expected,4) +
                       "\n    error:     "
                       + DecimalFormatter::toExponential(error,1) +
                       "\n    tolerance: "
                       + DecimalFormatter::toExponential(tolerance,1));
        }
    }
}
#endif // QL_PATCH_BORLAND

test_suite* InterpolationTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Interpolation tests");
    suite->add(BOOST_TEST_CASE(&InterpolationTest::testSplineOnGenericValues));
    suite->add(BOOST_TEST_CASE(
                        &InterpolationTest::testSimmetricEndConditions));
    suite->add(BOOST_TEST_CASE(
                        &InterpolationTest::testDerivativeEndConditions));
    suite->add(BOOST_TEST_CASE(
                        &InterpolationTest::testNonRestrictiveHymanFilter));
    suite->add(BOOST_TEST_CASE(&InterpolationTest::testSplineOnRPN15AValues));
    suite->add(BOOST_TEST_CASE(
                        &InterpolationTest::testSplineOnGaussianValues));
    suite->add(BOOST_TEST_CASE(
                        &InterpolationTest::testSplineErrorOnGaussianValues));
    suite->add(BOOST_TEST_CASE(&InterpolationTest::testMultiSpline));
    return suite;
}

