
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
#include <ql/Math/segmentintegral.hpp>
#include <ql/Math/simpsonintegral.hpp>
#include <ql/Math/trapezoidintegral.hpp>
#include <ql/Math/kronrodintegral.hpp>
#include <ql/Math/functional.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

namespace {

    #define BEGIN(x) (x+0)
    #define END(x) (x+LENGTH(x))

    std::vector<double> xRange(double start, double end, Size points) {
        std::vector<double> x(points);
        double dx = (end-start)/(points-1);
        for (Size i=0; i<points-1; i++)
            x[i] = start+i*dx;
        x[points-1] = end;
        return x;
    }

    std::vector<double> gaussian(const std::vector<double>& x) {
        std::vector<double> y(x.size());
        for (Size i=0; i<x.size(); i++)
            y[i] = QL_EXP(-x[i]*x[i]);
        return y;
    }

    std::vector<double> parabolic(const std::vector<double>& x) {
        std::vector<double> y(x.size());
        for (Size i=0; i<x.size(); i++)
            y[i] = -x[i]*x[i];
        return y;
    }
    template <class I, class J>
    void checkValues(const char* type,
                     const CubicSpline& spline,
                     I xBegin, I xEnd, J yBegin) {
        double tolerance = 2.0e-15;
        while (xBegin != xEnd) {
            double interpolated = spline(*xBegin);
            if (QL_FABS(interpolated-*yBegin) > tolerance) {
                CPPUNIT_FAIL(std::string(type) + 
                             " interpolation failed at x = " +
                             DoubleFormatter::toString(*xBegin) +
                             "\n    interpolated value: " +
                             DoubleFormatter::toExponential(interpolated) + 
                             "\n    expected value:     " +
                             DoubleFormatter::toExponential(*yBegin) +
                             "\n    error:              "
                             + DoubleFormatter::toExponential(
                                QL_FABS(interpolated-*yBegin)));
            }
            ++xBegin; ++yBegin;
        }
    }

    void check1stDerivativeValue(const char* type,
                                 const CubicSpline& spline,
                                 double x,
                                 double value) {
        double tolerance = 1.0e-14;
        double interpolated = spline.derivative(x);
        double error = QL_FABS(interpolated-value);
        if (error > tolerance) {
            CPPUNIT_FAIL(std::string(type) + 
                         " interpolation first derivative failure\n"
                         "at x="
                         + DoubleFormatter::toString(x) +
                         "\n    interpolated value: "
                         + DoubleFormatter::toString(interpolated) +
                         "\n    expected value:     "
                         + DoubleFormatter::toString(value) +
                         "\n    error:              "
                         + DoubleFormatter::toExponential(error));
        }
    }

    void check2ndDerivativeValue(const char* type,
                                 const CubicSpline& spline,
                                 double x,
                                 double value) {
        double tolerance = 1.0e-13;
        double interpolated = spline.secondDerivative(x);
        double error = QL_FABS(interpolated-value);
        if (error > tolerance) {
            CPPUNIT_FAIL(std::string(type) + 
                         " interpolation second derivative failure\n"
                         "at x="
                         + DoubleFormatter::toString(x) +
                         "\n    interpolated value: "
                         + DoubleFormatter::toString(interpolated) +
                         "\n    expected value:     "
                         + DoubleFormatter::toString(value) +
                         "\n    error:              "
                         + DoubleFormatter::toExponential(error));
        }
    }

    void checkNotAKnotCondition(const char* type,
                                const CubicSpline& spline) {

        double tolerance = 1.0e-14;
        const std::vector<double>& c = spline.cCoefficients();
        if (QL_FABS(c[0]-c[1]) > tolerance) {
            CPPUNIT_FAIL(std::string(type) +
                         " interpolation failure"
                         "\n    cubic coefficient of the first"
                         " polinomial is "
                         + DoubleFormatter::toString(c[0]) +
                         "\n    cubic coefficient of the second"
                         " polinomial is "
                         + DoubleFormatter::toString(c[1]));
        }
        Size n = c.size();
        if (QL_FABS(c[n-2]-c[n-1]) > tolerance) {
            CPPUNIT_FAIL(std::string(type) +
                         " interpolation failure"
                         "\n    cubic coefficient of the 2nd to last"
                         " polinomial is "
                         + DoubleFormatter::toString(c[n-2]) +
                         "\n    cubic coefficient of the last"
                         " polinomial is "
                         + DoubleFormatter::toString(c[n-1]));
        }
    }

    void checkSymmetry(const char* type,
                       const CubicSpline& spline,
                       double xMin) {
        double tolerance = 1.0e-15;
        for (double x = xMin; x < 0.0; x += 0.1) {
            double y1 = spline(x), y2 = spline(-x);
            if (QL_FABS(y1-y2) > tolerance) {
                CPPUNIT_FAIL(std::string(type) +
                    " interpolation not symmetric"
                    "\n    x = "   + DoubleFormatter::toString(x) +
                    "\n    g(x)  = " + DoubleFormatter::toString(y1) +
                    "\n    g(-x) = " + DoubleFormatter::toString(y2) +
                    "\n    error:              "
                    + DoubleFormatter::toExponential(QL_FABS(y1-y2)));
            }
        }
    }

    template <class F>
    class errorFunction : public std::unary_function<double,double> {
      public:
        errorFunction(const F& f) : f_(f) {}
        double operator()(double x) const { 
            double temp = f_(x)-QL_EXP(-x*x); 
            return temp*temp; 
        }
      private:
        F f_;
    };

    template <class F>
    errorFunction<F> make_error_function(const F& f) {
        return errorFunction<F>(f);
    }
}

/* See J. M. Hyman, "Accurate monotonicity preserving cubic interpolation"
   SIAM J. of Scientific and Statistical Computing, v. 4, 1983, pp. 645-654.
   http://math.lanl.gov/~mac/papers/numerics/H83.pdf
*/
void InterpolationTest::testSplineErrorOnGaussianValues() {

    Size points[]                = {      5,      9,     17,     33 };

    // complete spline data from the original 1983 Hyman paper
    double tabulatedErrors[]     = { 3.5e-2, 2.0e-3, 4.0e-5, 1.8e-6 };
    double toleranceOnTabErr[]   = { 0.1e-2, 0.1e-3, 0.1e-5, 0.1e-6 };

    // (complete) MC spline data from the original 1983 Hyman paper
    // NB: with the improved Hyman filter from the Dougherty, Edelman, and
    //     Hyman 1989 paper the n=17 nonmonotonicity is not filtered anymore
    //     so the error agrees with the non MC method.
    double tabulatedMCErrors[]   = { 1.7e-2, 2.0e-3, 4.0e-5, 1.8e-6 };
    double toleranceOnTabMCErr[] = { 0.1e-2, 0.1e-3, 0.1e-5, 0.1e-6 };

    SimpsonIntegral integral(1e-12);
    std::vector<double> x, y;

    // still unexplained scale factor needed to obtain the numerical 
    // results from the paper
    double scaleFactor = 1.9;

    for (Size i=0; i<LENGTH(points); i++) {
        Size n = points[i];
        std::vector<double> x = xRange(-1.7, 1.9, n);
        std::vector<double> y = gaussian(x);

        // Not-a-knot
        CubicSpline f = CubicSpline(x.begin(), x.end(), y.begin(),
                                    CubicSpline::NotAKnot, Null<double>(),
                                    CubicSpline::NotAKnot, Null<double>(),
                                    false);
        double result = QL_SQRT(integral(make_error_function(f), -1.7, 1.9));
        result /= scaleFactor;
        if (QL_FABS(result-tabulatedErrors[i]) > toleranceOnTabErr[i])
            CPPUNIT_FAIL("Not-a-knot spline interpolation "
                         "\n    sample points:      " +
                         IntegerFormatter::toString(n) +
                         "\n    norm of difference: " +
                         DoubleFormatter::toExponential(result,1) +
                         "\n    it should be:       " +
                         DoubleFormatter::toExponential(tabulatedErrors[i],1));

        // MC not-a-knot
        f = MonotonicCubicSpline(x.begin(), x.end(), y.begin(),
                                 CubicSpline::NotAKnot, Null<double>(),
                                 CubicSpline::NotAKnot, Null<double>());
        result = QL_SQRT(integral(make_error_function(f), -1.7, 1.9));
        result /= scaleFactor;
        if (QL_FABS(result-tabulatedMCErrors[i]) > toleranceOnTabMCErr[i])
            CPPUNIT_FAIL("MC Not-a-knot spline interpolation "
                         "\n    sample points:      " +
                         IntegerFormatter::toString(n) +
                         "\n    norm of difference: " +
                         DoubleFormatter::toExponential(result,1) +
                         "\n    it should be:       " +
                         DoubleFormatter::toExponential(tabulatedMCErrors[i],1));
    }

}

/* See J. M. Hyman, "Accurate monotonicity preserving cubic interpolation"
   SIAM J. of Scientific and Statistical Computing, v. 4, 1983, pp. 645-654.
   http://math.lanl.gov/~mac/papers/numerics/H83.pdf
*/
void InterpolationTest::testSplineOnGaussianValues() {
    double interpolated, interpolated2;
    Size n = 5;

    std::vector<double> x(n), y(n);
    double x1_bad=-1.7, x2_bad=1.7;

    for (double start = -1.9, j=0; j<2; start+=0.2, j++) {
        x = xRange(start, start+3.6, n);
        y = gaussian(x);

        // Not-a-knot spline
        CubicSpline f = CubicSpline(x.begin(), x.end(), y.begin(),
                                    CubicSpline::NotAKnot, Null<double>(),
                                    CubicSpline::NotAKnot, Null<double>(),
                                    false);
        checkValues("Not-a-knot spline", f,
                    x.begin(), x.end(), y.begin());
        checkNotAKnotCondition("Not-a-knot spline", f);
        // bad performance
        interpolated = f(x1_bad);
        interpolated2= f(x2_bad);
        if (interpolated>0.0 && interpolated2>0.0 ) {
            CPPUNIT_FAIL("Not-a-knot spline interpolation "
                "bad performance unverified"
                "\nat x="
                + DoubleFormatter::toString(x1_bad) +
                " interpolated value: "
                + DoubleFormatter::toString(interpolated) +
                "\nat x="
                + DoubleFormatter::toString(x2_bad) +
                " interpolated value: "
                + DoubleFormatter::toString(interpolated) +
                "\n at least one of them was expected to be < 0.0");
        }

        // MC not-a-knot spline
        f = MonotonicCubicSpline(x.begin(), x.end(), y.begin(),
                                 CubicSpline::NotAKnot, Null<double>(),
                                 CubicSpline::NotAKnot, Null<double>());
        checkValues("MC not-a-knot spline", f,
                    x.begin(), x.end(), y.begin());
        // good performance
        interpolated = f(x1_bad);
        if (interpolated<0.0) {
            CPPUNIT_FAIL("MC not-a-knot spline interpolation "
                "good performance unverified\n"
                "at x="
                + DoubleFormatter::toString(x1_bad) +
                "\ninterpolated value: "
                + DoubleFormatter::toString(interpolated) +
                "\nexpected value > 0.0");
        }
        interpolated = f(x2_bad);
        if (interpolated<0.0) {
            CPPUNIT_FAIL("MC not-a-knot spline interpolation "
                "good performance unverified\n"
                "at x="
                + DoubleFormatter::toString(x2_bad) +
                "\ninterpolated value: "
                + DoubleFormatter::toString(interpolated) +
                "\nexpected value > 0.0");
        }
    }
}


/* See J. M. Hyman, "Accurate monotonicity preserving cubic interpolation"
   SIAM J. of Scientific and Statistical Computing, v. 4, 1983, pp. 645-654.
   http://math.lanl.gov/~mac/papers/numerics/H83.pdf
*/
void InterpolationTest::testSplineOnRPN15AValues() {

    const double RPN15A_x[] = { 
        7.99,       8.09,       8.19,      8.7,
        9.2,     10.0,     12.0,     15.0,     20.0 
    };
    const double RPN15A_y[] = {
        0.0, 2.76429e-5, 4.37498e-5, 0.169183,
        0.469428, 0.943740, 0.998636, 0.999919, 0.999994 
    };

    double interpolated;

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
    double x_bad = 11.0;
    interpolated = f(x_bad);
    if (interpolated<1.0) {
        CPPUNIT_FAIL("Natural spline interpolation "
            "poor performance unverified\n"
            "at x="
            + DoubleFormatter::toString(x_bad) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
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
        CPPUNIT_FAIL("Clamped spline interpolation "
            "poor performance unverified\n"
            "at x="
            + DoubleFormatter::toString(x_bad) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value > 1.0");
    }


    // Not-a-knot spline
    f = CubicSpline(BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y),
                    CubicSpline::NotAKnot, Null<double>(),
                    CubicSpline::NotAKnot, Null<double>(),
                    false);
    checkValues("Not-a-knot spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
    checkNotAKnotCondition("Not-a-knot spline", f);
    // poor performance
    interpolated = f(x_bad);
    if (interpolated<1.0) {
        CPPUNIT_FAIL("Not-a-knot spline interpolation "
            "poor performance unverified\n"
            "at x="
            + DoubleFormatter::toString(x_bad) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
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
        CPPUNIT_FAIL("MC natural spline interpolation "
            "good performance unverified\n"
            "at x="
            + DoubleFormatter::toString(x_bad) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
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
        CPPUNIT_FAIL("MC clamped spline interpolation "
            "good performance unverified\n"
            "at x="
            + DoubleFormatter::toString(x_bad) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value < 1.0");
    }


    // MC not-a-knot spline values
    f = MonotonicCubicSpline(BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y),
                             CubicSpline::NotAKnot, Null<double>(),
                             CubicSpline::NotAKnot, Null<double>());
    checkValues("MC not-a-knot spline", f,
                BEGIN(RPN15A_x), END(RPN15A_x), BEGIN(RPN15A_y));
    // good performance
    interpolated = f(x_bad);
    if (interpolated>1.0) {
        CPPUNIT_FAIL("MC clamped spline interpolation "
            "good performance unverified\n"
            "at x="
            + DoubleFormatter::toString(x_bad) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value < 1.0");
    }
}

/* Blossey, Frigyik, Farnum "A Note On Cubic Splines"
   Applied Linear Algebra and Numerical Analysis AMATH 352 Lecture Notes
   http://www.amath.washington.edu/courses/352-winter-2002/spline_note.pdf
*/
void InterpolationTest::testSplineOnGenericValues() {

    const double generic_x[] = { 0.0, 1.0, 3.0, 4.0 };
    const double generic_y[] = { 0.0, 0.0, 2.0, 2.0 };
    const double generic_natural_y2[] = { 0.0, 1.5, -1.5, 0.0 };

    double interpolated, error;
    Size i, n = LENGTH(generic_x);
    std::vector<double> x35(3);

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
        if (QL_FABS(error)>1e-17) {
            CPPUNIT_FAIL("Natural spline interpolation "
                "second derivative failed at x="
                + DoubleFormatter::toString(generic_x[i]) +
                "\ninterpolated value: "
                + DoubleFormatter::toString(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toString(generic_natural_y2[i]));
        }
    }
    x35[1] = f(3.5);


    // Clamped spline
    double y1a = 0.0, y1b = 0.0;
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
                    CubicSpline::NotAKnot, Null<double>(),
                    CubicSpline::NotAKnot, Null<double>(),
                    false);
    checkValues("Not-a-knot spline", f,
                BEGIN(generic_x), END(generic_x), BEGIN(generic_y));
    checkNotAKnotCondition("Not-a-knot spline", f);

    x35[2] = f(3.5);

    if (x35[0]>x35[1] || x35[1]>x35[2]) {
        CPPUNIT_FAIL("Spline interpolation failure"
            "\nat x="
            + DoubleFormatter::toString(3.5) +
            "\nclamped spline    " + DoubleFormatter::toString(x35[0]) +
            "\nnatural spline    " + DoubleFormatter::toString(x35[1]) +
            "\nnot-a-knot spline " + DoubleFormatter::toString(x35[2]) +
            "\nvalues should be in increasing order");
    }
}


void InterpolationTest::testingSimmetricEndConditions() {
    Size n = 9;

    std::vector<double> x, y;
    x = xRange(-1.8, 1.8, n);
    y = gaussian(x);

    // Not-a-knot spline
    CubicSpline f = CubicSpline(x.begin(), x.end(), y.begin(),
                                CubicSpline::NotAKnot, Null<double>(),
                                CubicSpline::NotAKnot, Null<double>(),
                                false);
    checkValues("Not-a-knot spline", f,
                x.begin(), x.end(), y.begin());
    checkNotAKnotCondition("Not-a-knot spline", f);
    checkSymmetry("Not-a-knot spline", f, x[0]);


    // MC not-a-knot spline
    f = MonotonicCubicSpline(x.begin(), x.end(), y.begin(),
                             CubicSpline::NotAKnot, Null<double>(),
                             CubicSpline::NotAKnot, Null<double>());
    checkValues("MC not-a-knot spline", f,
                x.begin(), x.end(), y.begin());
    checkSymmetry("MC not-a-knot spline", f, x[0]);
}


void InterpolationTest::testingDerivativeEndConditions() {
    Size n = 4;

    std::vector<double> x, y;
    x = xRange(-2.0, 2.0, n);
    y = parabolic(x);

    // Not-a-knot spline
    CubicSpline f = CubicSpline(x.begin(), x.end(), y.begin(),
                                CubicSpline::NotAKnot, Null<double>(),
                                CubicSpline::NotAKnot, Null<double>(),
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
                    CubicSpline::NotAKnot, Null<double>(),
                    CubicSpline::NotAKnot, Null<double>(),
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
void InterpolationTest::testingNonRestrictiveHymanFilter() {
    Size n = 4;

    std::vector<double> x, y;
    x = xRange(-2.0, 2.0, n);
    y = parabolic(x);
    double zero=0.0, interpolated, expected=0.0;

    // MC Not-a-knot spline
    Interpolation f = CubicSpline(x.begin(), x.end(), y.begin(),
                                  CubicSpline::NotAKnot, Null<double>(),
                                  CubicSpline::NotAKnot, Null<double>(),
                                  true);
    interpolated = f(zero);
    if (QL_FABS(interpolated-expected)>1e-15) {
        CPPUNIT_FAIL("MC not-a-knot spline"
            " interpolation failed at x = " +
            DoubleFormatter::toString(zero) +
            "\n    interpolated value: " +
            DoubleFormatter::toString(interpolated) + 
            "\n    expected value:     " +
            DoubleFormatter::toString(expected) +
            "\n    error:              "
            + DoubleFormatter::toExponential(QL_FABS(interpolated-expected)));
    }


    // MC Clamped spline
    f = CubicSpline(x.begin(), x.end(), y.begin(),
                    CubicSpline::FirstDerivative,  4.0,
                    CubicSpline::FirstDerivative, -4.0,
                    true);
    interpolated = f(zero);
    if (QL_FABS(interpolated-expected)>1e-15) {
        CPPUNIT_FAIL("MC clamped spline"
            " interpolation failed at x = " +
            DoubleFormatter::toString(zero) +
            "\n    interpolated value: " +
            DoubleFormatter::toString(interpolated) + 
            "\n    expected value:     " +
            DoubleFormatter::toString(expected) +
            "\n    error:              "
            + DoubleFormatter::toExponential(QL_FABS(interpolated-expected)));
    }


    // MC SecondDerivative spline
    f = CubicSpline(x.begin(), x.end(), y.begin(),
                    CubicSpline::SecondDerivative, -2.0,
                    CubicSpline::SecondDerivative, -2.0,
                    true);
    if (QL_FABS(interpolated-expected)>1e-15) {
        CPPUNIT_FAIL("MC SecondDerivative spline"
            " interpolation failed at x = " +
            DoubleFormatter::toString(zero) +
            "\n    interpolated value: " +
            DoubleFormatter::toString(interpolated) + 
            "\n    expected value:     " +
            DoubleFormatter::toString(expected) +
            "\n    error:              "
            + DoubleFormatter::toExponential(QL_FABS(interpolated-expected)));
    }

}

CppUnit::Test* InterpolationTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Interpolation tests");
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing spline interpolation on generic values",
                    &InterpolationTest::testSplineOnGenericValues));
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing symmetry of spline interpolation end-conditions",
                   &InterpolationTest::testingSimmetricEndConditions));
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing derivative end-conditions "
                    "for spline interpolation",
                   &InterpolationTest::testingDerivativeEndConditions));
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing non-restrictive Hyman filter",
                   &InterpolationTest::testingNonRestrictiveHymanFilter));
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing spline interpolation on RPN15A data set",
                    &InterpolationTest::testSplineOnRPN15AValues));
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing spline interpolation on a Gaussian data set",
                    &InterpolationTest::testSplineOnGaussianValues));
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing spline interpolation error on Gaussian data sets",
                   &InterpolationTest::testSplineErrorOnGaussianValues));
    return tests;
}

