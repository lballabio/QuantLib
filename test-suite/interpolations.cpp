
/*
 Copyright (C) 2004 Ferdinando Ametrano

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
#include <ql/Math/kronrodintegral.hpp>
#include <ql/Math/functional.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

namespace {

    const double RPN15A_x[] = { 7.99,       8.09,       8.19,      8.7,
             9.2,     10.0,     12.0,     15.0,     20.0 };
    const double RPN15A_y[] = {  0.0, 2.76429e-5, 4.37498e-5, 0.169183,
        0.469428, 0.943740, 0.998636, 0.999919, 0.999994 };

    const double generic_x[] = { 0.0, 1.0, 3.0, 4.0 };
    const double generic_y[] = { 0.0, 0.0, 2.0, 2.0 };
    const double generic_natural_y2[] = { 0.0, 1.5, -1.5, 0.0 };
    const double generic_natural_a[] = { -0.25,  0.50,  0.50 };
    const double generic_natural_b[] = {  0.00,  0.75, -0.75 };
    const double generic_natural_c[] = {  0.25, -0.25,  0.25 };


    template <class F>
    class errorFunction : public std::unary_function<double,double> {
      public:
          errorFunction(const F& f) : f_(f) {}
        // instead of the hardcoded x-QL_EXP(-x*x), should be f(x)-QL_EXP(-x*x)
        // where f(x) is the interpolant
        double operator()(double x) const { double temp = f_(x)-QL_EXP(-x*x); return temp*temp; }
      private:
        F f_;
    };

 }

void InterpolationTest::testSplineErrorOnGaussianValues() {

    Size i, n;
    KronrodIntegral integral(1e-10);
//    SimpsonIntegral integral(1e-7);
    std::vector<double> x, y;

    std::cout << std::endl;

    n = 5;
    x = std::vector<double>(n);
    y = std::vector<double>(n);
    for (i=0; i<n; i++) {
        x[i] = -1.7+i*3.6/(n-1);
        y[i] = QL_EXP(-x[i]*x[i]);
    }


    // Not-a-knot spline interpolation
    CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> interpolant(
            x.begin(), x.end(),
            y.begin(),
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            false);
    errorFunction<CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> > myErrorFunction(interpolant); 
    double result = QL_SQRT(integral(myErrorFunction, -1.7, 1.9));
    std::cout << n << "    complete " << DoubleFormatter::toExponential(result, 1) << std::endl;

    // MC not-a-knot spline interpolation
    interpolant = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            x.begin(), x.end(),
            y.begin(),
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            true);
    myErrorFunction = errorFunction<CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> >(interpolant); 
    result = QL_SQRT(integral(myErrorFunction, -1.7, 1.9));
    std::cout << n << " MC complete " << DoubleFormatter::toExponential(result, 1) << std::endl;


    n = 9;
    x = std::vector<double>(n);
    y = std::vector<double>(n);
    for (i=0; i<n; i++) {
        x[i] = -1.7+i*3.6/(n-1);
        y[i] = QL_EXP(-x[i]*x[i]);
    }


    // Not-a-knot spline interpolation
    interpolant = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            x.begin(), x.end(),
            y.begin(),
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            false);
    myErrorFunction = errorFunction<CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> >(interpolant); 
    result = QL_SQRT(integral(myErrorFunction, -1.7, 1.9));
    std::cout << n << "    complete " << DoubleFormatter::toExponential(result, 1) << std::endl;

    // MC not-a-knot spline interpolation
    interpolant = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            x.begin(), x.end(),
            y.begin(),
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            true);
    myErrorFunction = errorFunction<CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> >(interpolant); 
    result = QL_SQRT(integral(myErrorFunction, -1.7, 1.9));
    std::cout << n << " MC complete " << DoubleFormatter::toExponential(result, 1) << std::endl;


    n = 17;
    x = std::vector<double>(n);
    y = std::vector<double>(n);
    for (i=0; i<n; i++) {
        x[i] = -1.7+i*3.6/(n-1);
        y[i] = QL_EXP(-x[i]*x[i]);
    }


    // Not-a-knot spline interpolation
    interpolant = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            x.begin(), x.end(),
            y.begin(),
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            false);
    myErrorFunction = errorFunction<CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> >(interpolant); 
    result = QL_SQRT(integral(myErrorFunction, -1.7, 1.9));
    std::cout << n << "    complete " << DoubleFormatter::toExponential(result, 1) << std::endl;

    // MC not-a-knot spline interpolation
    interpolant = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            x.begin(), x.end(),
            y.begin(),
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            true);
    myErrorFunction = errorFunction<CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> >(interpolant); 
    result = QL_SQRT(integral(myErrorFunction, -1.7, 1.9));
    std::cout << n << " MC complete " << DoubleFormatter::toExponential(result, 1) << std::endl;

    n = 33;
    x = std::vector<double>(n);
    y = std::vector<double>(n);
    for (i=0; i<n; i++) {
        x[i] = -1.7+i*3.6/(n-1);
        y[i] = QL_EXP(-x[i]*x[i]);
    }


    // Not-a-knot spline interpolation
    interpolant = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            x.begin(), x.end(),
            y.begin(),
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            false);
    myErrorFunction = errorFunction<CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> >(interpolant); 
    result = QL_SQRT(integral(myErrorFunction, -1.7, 1.9));
    std::cout << n << "    complete " << DoubleFormatter::toExponential(result, 1) << std::endl;

    // MC not-a-knot spline interpolation
    interpolant = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            x.begin(), x.end(),
            y.begin(),
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            true);
    myErrorFunction = errorFunction<CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> >(interpolant); 
    result = QL_SQRT(integral(myErrorFunction, -1.7, 1.9));
    std::cout << n << " MC complete " << DoubleFormatter::toExponential(result, 1) << std::endl;


}

void InterpolationTest::testSplineOnGaussianValues() {
    double interpolated, interpolated2;
    Size i, n = 5;
    bool allowExtrapolation = false;

    std::vector<double> x(n), y(n);
    double x1_bad=-1.7, x2_bad=1.7;

    for (double start = -1.9, j=0; j<2; start+=0.2, j++) { 
        for (i=0; i<n; i++) {
            x[i] = start+i*3.6/(n-1);
            y[i] = QL_EXP(-x[i]*x[i]);
        }

        // Not-a-knot spline values
        CubicSplineInterpolation<
            std::vector<double>::const_iterator,
            std::vector<double>::const_iterator> interp(
                x.begin(), x.end(),
                y.begin(),
                Null<double>(), Null<double>(),
                Null<double>(), Null<double>(),
                false);
        for (i=0; i<n; i++) {
            interpolated = interp(x[i], allowExtrapolation);
            if (QL_FABS(interpolated-y[i])>1e-15) {
                CPPUNIT_FAIL("Not-a-knot spline interpolation failed at x="
                    + DoubleFormatter::toString(x[i]) +
                    "\ninterpolated value:  "
                    + DoubleFormatter::toExponential(interpolated) +
                    "\nexpected value:     "
                    + DoubleFormatter::toExponential(y[i]));
            }
        }
        // Not-a-knot spline third derivative
        i = 0;
        if (QL_FABS(interp.cCoefficients()[i]-interp.cCoefficients()[i+1])>1e-15) {
            CPPUNIT_FAIL("Not-a-knot spline interpolation failure"
                "\n cubic coefficient of the "
                + IntegerFormatter::toOrdinal(i+1) +
                " polinomial is "
                + DoubleFormatter::toString(interp.cCoefficients()[i]) +
                "\n cubic coefficient of the "
                + IntegerFormatter::toOrdinal(i+2) +
                " polinomial is "
                + DoubleFormatter::toString(interp.cCoefficients()[i+1])
                );
        }
        i = n-3;
        if (QL_FABS(interp.cCoefficients()[i]-interp.cCoefficients()[i+1])>1e-15) {
            CPPUNIT_FAIL("Not-a-knot spline interpolation failure"
                "\n cubic coefficient of the 2nd to last"
                " polinomial is "
                + DoubleFormatter::toString(interp.cCoefficients()[i]) +
                "\n cubic coefficient of the last"
                " polinomial is "
                + DoubleFormatter::toString(interp.cCoefficients()[i+1])
                );
        }
        // Not-a-knot spline good performance
        interpolated = interp(x1_bad, allowExtrapolation);
        interpolated2= interp(x2_bad, allowExtrapolation);
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




        // MC not-a-knot spline values
        interp = CubicSplineInterpolation<
            std::vector<double>::const_iterator,
            std::vector<double>::const_iterator>(
                x.begin(), x.end(),
                y.begin(),
                Null<double>(), Null<double>(),
                Null<double>(), Null<double>(),
                true);
        for (i=0; i<n; i++) {
            interpolated = interp(x[i], allowExtrapolation);
            if (QL_FABS(interpolated-y[i])>1e-15) {
                CPPUNIT_FAIL("MC not-a-knot spline interpolation failed at x="
                    + DoubleFormatter::toString(x[i]) +
                    "\ninterpolated value:  "
                    + DoubleFormatter::toExponential(interpolated) +
                    "\nexpected value:     "
                    + DoubleFormatter::toExponential(y[i]));
            }
        }
        // MC not-a-knot spline good performance
        interpolated = interp(x1_bad, allowExtrapolation);
        if (interpolated<0.0) {
            CPPUNIT_FAIL("MC not-a-knot spline interpolation "
                "good performance unverified\n"
                "at x="
                + DoubleFormatter::toString(x1_bad) +
                "\ninterpolated value: "
                + DoubleFormatter::toString(interpolated) +
                "\nexpected value > 0.0");
        }
        interpolated = interp(x2_bad, allowExtrapolation);
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


void InterpolationTest::testSplineOnRPN15AValues() {
    double interpolated;
    Size i, n = LENGTH(RPN15A_x);
    bool allowExtrapolation = false;
    bool monotonicityConstraint = false;

    // Natural spline values
    CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> interp(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            Null<double>(), 0.0,
            Null<double>(), 0.0,
            monotonicityConstraint);
    for (i=0; i<n; i++) {
        interpolated = interp(RPN15A_x[i], allowExtrapolation);
        if (interpolated!=RPN15A_y[i]) {
            CPPUNIT_FAIL("Natural spline interpolation failed at x="
                + DoubleFormatter::toString(RPN15A_x[i]) +
                "\ninterpolated value: "
                + DoubleFormatter::toExponential(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toExponential(RPN15A_y[i]));
        }
    }
    // Natural spline second derivative values
    i = 0;
    interpolated = interp.secondDerivative(RPN15A_x[i], allowExtrapolation);
    if (QL_FABS(interpolated)>1e-17) {
        CPPUNIT_FAIL("Natural spline interpolation second derivative failure\n"
            "at x="
            + DoubleFormatter::toString(RPN15A_x[i]) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value:     "
            + DoubleFormatter::toString(0.0));
    }
    i = n-1;
    interpolated = interp.secondDerivative(RPN15A_x[i], allowExtrapolation);
    if (QL_FABS(interpolated)>1e-17) {
        CPPUNIT_FAIL("Natural spline interpolation second derivative failure\n"
            "at x="
            + DoubleFormatter::toString(RPN15A_x[i]) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value:     "
            + DoubleFormatter::toString(0.0));
    }
    // Natural spline poor performance
    double x_bad = 11.0;
    interpolated = interp(x_bad, allowExtrapolation);
    if (interpolated<1.0) {
        CPPUNIT_FAIL("Natural spline interpolation "
            "poor performance unverified\n"
            "at x="
            + DoubleFormatter::toString(x_bad) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value > 1.0");
    }

    // Clamped spline values
    interp = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            0.0, Null<double>(),
            0.0, Null<double>(),
            monotonicityConstraint);
    for (i=0; i<n; i++) {
        interpolated = interp(RPN15A_x[i], allowExtrapolation);
        if (interpolated!=RPN15A_y[i]) {
            CPPUNIT_FAIL("Clamped spline interpolation failed at x="
                + DoubleFormatter::toString(RPN15A_x[i]) +
                "\ninterpolated value: "
                + DoubleFormatter::toExponential(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toExponential(RPN15A_y[i]));
        }
    }
    // Clamped spline first derivative values
    i = 0;
    interpolated = interp.derivative(RPN15A_x[i], allowExtrapolation);
    if (QL_FABS(interpolated)>1e-17) {
        CPPUNIT_FAIL("Clamped spline interpolation first derivative failure\n"
            "at x="
            + DoubleFormatter::toString(RPN15A_x[i]) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value:     "
            + DoubleFormatter::toString(0.0));
    }
    i = n-1;
    interpolated = interp.derivative(RPN15A_x[i], allowExtrapolation);
    if (QL_FABS(interpolated)>1e-16) {
        CPPUNIT_FAIL("Clamped spline interpolation first derivative failure\n"
            "at x="
            + DoubleFormatter::toString(RPN15A_x[i]) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value:     "
            + DoubleFormatter::toString(0.0));
    }
    // Clamped spline poor performance
    interpolated = interp(x_bad, allowExtrapolation);
    if (interpolated<1.0) {
        CPPUNIT_FAIL("Clamped spline interpolation "
            "poor performance unverified\n"
            "at x="
            + DoubleFormatter::toString(x_bad) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value > 1.0");
    }



    // Not-a-knot spline values
    interp = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            monotonicityConstraint);
    for (i=0; i<n; i++) {
        interpolated = interp(RPN15A_x[i], allowExtrapolation);
        if (QL_FABS(interpolated-RPN15A_y[i])>1e-15) {
            CPPUNIT_FAIL("Not-a-knot spline interpolation failed at x="
                + DoubleFormatter::toString(RPN15A_x[i]) +
                "\ninterpolated value:  "
                + DoubleFormatter::toExponential(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toExponential(RPN15A_y[i]));
        }
    }
    // Not-a-knot spline third derivative
    i = 0;
    if (QL_FABS(interp.cCoefficients()[i]-interp.cCoefficients()[i+1])>1e-15) {
        CPPUNIT_FAIL("Not-a-knot spline interpolation failure"
            "\n cubic coefficient of the "
            + IntegerFormatter::toOrdinal(i+1) +
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i]) +
            "\n cubic coefficient of the "
            + IntegerFormatter::toOrdinal(i+2) +
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i+1])
            );
    }
    i = n-3;
    if (QL_FABS(interp.cCoefficients()[i]-interp.cCoefficients()[i+1])>1e-15) {
        CPPUNIT_FAIL("Not-a-knot spline interpolation failure"
            "\n cubic coefficient of the 2nd to last"
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i]) +
            "\n cubic coefficient of the last"
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i+1])
            );
    }
    // Not-a-knot spline poor performance
    interpolated = interp(x_bad, allowExtrapolation);
    if (interpolated<1.0) {
        CPPUNIT_FAIL("Not-a-knot spline interpolation "
            "poor performance unverified\n"
            "at x="
            + DoubleFormatter::toString(x_bad) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value > 1.0");
    }





    monotonicityConstraint = true;

    // MC natural spline values
    interp = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            Null<double>(), 0.0,
            Null<double>(), 0.0,
            monotonicityConstraint);
    for (i=0; i<n; i++) {
        interpolated = interp(RPN15A_x[i], allowExtrapolation);
        if (interpolated!=RPN15A_y[i]) {
            CPPUNIT_FAIL("MC natural spline interpolation failed at x="
                + DoubleFormatter::toString(RPN15A_x[i]) +
                "\ninterpolated value: "
                + DoubleFormatter::toExponential(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toExponential(RPN15A_y[i]));
        }
    }
    // MC natural spline good performance
    interpolated = interp(x_bad, allowExtrapolation);
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
    interp = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            0.0, Null<double>(),
            0.0, Null<double>(),
            monotonicityConstraint);
    for (i=0; i<n; i++) {
        interpolated = interp(RPN15A_x[i], allowExtrapolation);
        if (interpolated!=RPN15A_y[i]) {
            CPPUNIT_FAIL("MC clamped spline interpolation failed at x="
                + DoubleFormatter::toString(RPN15A_x[i]) +
                "\ninterpolated value: "
                + DoubleFormatter::toExponential(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toExponential(RPN15A_y[i]));
        }
    }
    // MC clamped spline first derivative values
    i = 0;
    interpolated = interp.derivative(RPN15A_x[i], allowExtrapolation);
    if (QL_FABS(interpolated)>1e-17) {
        CPPUNIT_FAIL("MC clamped spline interpolation first derivative failure\n"
            "at x="
            + DoubleFormatter::toString(RPN15A_x[i]) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value:     "
            + DoubleFormatter::toString(0.0));
    }
    i = n-1;
    interpolated = interp.derivative(RPN15A_x[i], allowExtrapolation);
    if (QL_FABS(interpolated)>1e-16) {
        CPPUNIT_FAIL("MC clamped spline interpolation first derivative failure\n"
            "at x="
            + DoubleFormatter::toString(RPN15A_x[i]) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value:     "
            + DoubleFormatter::toString(0.0));
    }
    // MC clamped spline good performance
    interpolated = interp(x_bad, allowExtrapolation);
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
    interp = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            monotonicityConstraint);
    for (i=0; i<n; i++) {
        interpolated = interp(RPN15A_x[i], allowExtrapolation);
        if (interpolated!=RPN15A_y[i]) {
            CPPUNIT_FAIL("MC not-a-knot spline interpolation failed at x="
                + DoubleFormatter::toString(RPN15A_x[i]) +
                "\ninterpolated value:  "
                + DoubleFormatter::toExponential(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toExponential(RPN15A_y[i]));
        }
    }
    // MC not-a-knot spline good performance
    interpolated = interp(x_bad, allowExtrapolation);
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

void InterpolationTest::testSplineOnGenericValues() {
    double interpolated, error;
    Size i, n = LENGTH(generic_x);
    bool monotonicityConstraint = false;
    bool allowExtrapolation = false;
    std::vector<double> x35(3);

    // Natural spline
    CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> interp(
            generic_x, generic_x+n,
            generic_y,
            Null<double>(), generic_natural_y2[0],
            Null<double>(), generic_natural_y2[n-1],
            monotonicityConstraint);
    // Natural spline values
    for (i=0; i<n; i++) {
        interpolated = interp(generic_x[i], allowExtrapolation);
        if (interpolated!=generic_y[i]) {
            CPPUNIT_FAIL("Natural spline interpolation failed at x="
                + DoubleFormatter::toString(generic_x[i]) +
                "\ninterpolated value: "
                + DoubleFormatter::toExponential(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toExponential(generic_y[i]));
        }
    }
    // Natural spline second derivative
    for (i=0; i<n; i++) {
        interpolated = interp.secondDerivative(generic_x[i], allowExtrapolation);
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
    // Natural spline polinomial coefficients
    for (i=0; i<n-1; i++) {
        interpolated = interp.aCoefficients()[i];
        error = interpolated - generic_natural_a[i];
        if (QL_FABS(error)>1e-17) {
            CPPUNIT_FAIL("Natural spline interpolation failure:"
                "\ncoefficient 'a' of the "
                + IntegerFormatter::toOrdinal(i+1) +
                " polinomial"
                "\ninterpolated value: "
                + DoubleFormatter::toString(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toString(generic_natural_a[i]));
        }
        interpolated = interp.bCoefficients()[i];
        error = interpolated - generic_natural_b[i];
        if (QL_FABS(error)>1e-17) {
            CPPUNIT_FAIL("Natural spline interpolation failure:"
                "\ncoefficient 'b' of the "
                + IntegerFormatter::toOrdinal(i+1) +
                " polinomial"
                "\ninterpolated value: "
                + DoubleFormatter::toString(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toString(generic_natural_b[i]));
        }
        interpolated = interp.cCoefficients()[i];
        error = interpolated - generic_natural_c[i];
        if (QL_FABS(error)>1e-17) {
            CPPUNIT_FAIL("Natural spline interpolation failure:"
                "\ncoefficient 'c' of the "
                + IntegerFormatter::toOrdinal(i+1) +
                " polinomial"
                "\ninterpolated value: "
                + DoubleFormatter::toString(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toString(generic_natural_c[i]));
        }
    }
    x35[1] = interp(3.5);



    // Clamped spline
    double y1a = 0.0, y1b = 0.0;
    interp = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            generic_x, generic_x+n,
            generic_y,
            y1a, Null<double>(),
            y1b, Null<double>(),
            monotonicityConstraint);
    // Clamped spline values
    for (i=0; i<n; i++) {
        interpolated = interp(generic_x[i], allowExtrapolation);
        if (interpolated!=generic_y[i]) {
            CPPUNIT_FAIL("Clamped spline interpolation failed at x="
                + DoubleFormatter::toString(generic_x[i]) +
                "\ninterpolated value: "
                + DoubleFormatter::toExponential(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toExponential(generic_y[i]));
        }
    }
    // Clamped spline first derivative
    i = 0;
    interpolated = interp.derivative(generic_x[i], allowExtrapolation);
    if (QL_FABS(interpolated-y1a)>1e-17) {
        CPPUNIT_FAIL("Clamped spline interpolation first derivative failed at x="
            + DoubleFormatter::toString(generic_x[i]) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value:     "
            + DoubleFormatter::toString(y1a));
    }
    i = n-1;
    interpolated = interp.derivative(generic_x[i], allowExtrapolation);
    if (QL_FABS(interpolated-y1b)>1e-16) {
        CPPUNIT_FAIL("Clamped spline interpolation first derivative failed at x="
            + DoubleFormatter::toString(generic_x[i]) +
            "\ninterpolated value: "
            + DoubleFormatter::toString(interpolated) +
            "\nexpected value:     "
            + DoubleFormatter::toString(y1b));
    }
    x35[0] = interp(3.5);



    // Not-a-knot spline
    interp = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            generic_x, generic_x+n,
            generic_y,
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            monotonicityConstraint);
    // Not-a-knot spline values
    for (i=0; i<n; i++) {
        interpolated = interp(generic_x[i], allowExtrapolation);
        if (interpolated!=generic_y[i]) {
            CPPUNIT_FAIL("Not-a-knot spline interpolation failed at x="
                + DoubleFormatter::toString(generic_x[i]) +
                "\ninterpolated value: "
                + DoubleFormatter::toExponential(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toExponential(generic_y[i]));
        }
    }
    // Not-a-knot spline third derivative
    i = 0;
    if (QL_FABS(interp.cCoefficients()[i]-interp.cCoefficients()[i+1])>1e-15) {
        CPPUNIT_FAIL("Not-a-knot spline interpolation failure"
            "\n cubic coefficient of the "
            + IntegerFormatter::toOrdinal(i+1) +
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i]) +
            "\n cubic coefficient of the "
            + IntegerFormatter::toOrdinal(i+2) +
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i+1])
            );
    }
    i = n-3;
    if (QL_FABS(interp.cCoefficients()[i]-interp.cCoefficients()[i+1])>1e-15) {
        CPPUNIT_FAIL("Not-a-knot spline interpolation failure"
            "\n cubic coefficient of the 2nd to last"
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i]) +
            "\n cubic coefficient of the last"
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i+1])
            );
    }
    x35[2] = interp(3.5);

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
    double interpolated, interpolated2;
    Size i, n = 9;
    bool allowExtrapolation = false;

    std::vector<double> x(n), y(n);

    for (i=0; i<n; i++) {
        x[i] = -1.8+i*3.6/(n-1);
        y[i] = QL_EXP(-x[i]*x[i]);
    }

    // Not-a-knot spline values
    CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> interp(
            x.begin(), x.end(),
            y.begin(),
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            false);
    for (i=0; i<n; i++) {
        interpolated = interp(x[i], allowExtrapolation);
        if (QL_FABS(interpolated-y[i])>1e-15) {
            CPPUNIT_FAIL("Not-a-knot spline interpolation failed at x="
                + DoubleFormatter::toString(x[i]) +
                "\ninterpolated value:  "
                + DoubleFormatter::toExponential(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toExponential(y[i]));
        }
    }
    // Not-a-knot spline third derivative
    i = 0;
    if (QL_FABS(interp.cCoefficients()[i]-interp.cCoefficients()[i+1])>1e-14) {
        CPPUNIT_FAIL("Not-a-knot spline interpolation failure"
            "\n cubic coefficient of the "
            + IntegerFormatter::toOrdinal(i+1) +
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i]) +
            "\n cubic coefficient of the "
            + IntegerFormatter::toOrdinal(i+2) +
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i+1])
            );
    }
    i = n-3;
    if (QL_FABS(interp.cCoefficients()[i]-interp.cCoefficients()[i+1])>1e-14) {
        CPPUNIT_FAIL("Not-a-knot spline interpolation failure"
            "\n cubic coefficient of the 2nd to last"
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i]) +
            "\n cubic coefficient of the last"
            " polinomial is "
            + DoubleFormatter::toString(interp.cCoefficients()[i+1])
            );
    }
    // now test the simmetry
    double xx = x[0];
    while (xx<0.0) {
        interpolated  = interp( xx);
        interpolated2 = interp(-xx);
        if (QL_FABS(interpolated-interpolated2)>1e-15) {
            CPPUNIT_FAIL("Not-a-knot spline interpolation not simmetric"
                "\nx="   + DoubleFormatter::toString(xx) +
                " g(x)=" + DoubleFormatter::toString(interpolated) +
                "\nx="   + DoubleFormatter::toString(-xx) +
                " g(x)=" + DoubleFormatter::toString(interpolated2)
                );
        }
        xx+=0.1;
    }




    // MC not-a-knot spline values
    interp = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            x.begin(), x.end(),
            y.begin(),
            Null<double>(), Null<double>(),
            Null<double>(), Null<double>(),
            true);
    for (i=0; i<n; i++) {
        interpolated = interp(x[i], allowExtrapolation);
        if (QL_FABS(interpolated-y[i])>1e-15) {
            CPPUNIT_FAIL("MC not-a-knot spline interpolation failed at x="
                + DoubleFormatter::toString(x[i]) +
                "\ninterpolated value:  "
                + DoubleFormatter::toExponential(interpolated) +
                "\nexpected value:     "
                + DoubleFormatter::toExponential(y[i]));
        }
    }

}














CppUnit::Test* InterpolationTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Interpolation tests");
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing spline interpolation on generic values",
                    &InterpolationTest::testSplineOnGenericValues));
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing simmetry of spline interpolation end conditions",
                    &InterpolationTest::testingSimmetricEndConditions));
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing spline interpolation on RPN15A data set",
                    &InterpolationTest::testSplineOnRPN15AValues));
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing spline interpolation on a gaussian data set",
                    &InterpolationTest::testSplineOnGaussianValues));
/*
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing spline interpolation error on gaussian data sets",
                    &InterpolationTest::testSplineErrorOnGaussianValues));
*/
    return tests;
}

