
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

    template <class I, class J, class K, class L>
    void checkValues(const char* type,
                     const CubicSplineInterpolation<I,J>& spline,
                     K xBegin, K xEnd, L yBegin) {
        double tolerance = 1.0e-15;
        while (xBegin != xEnd) {
            double interpolated = spline(*xBegin);
            if (QL_FABS(interpolated-*yBegin) > tolerance) {
                CPPUNIT_FAIL(std::string(type) + 
                             " interpolation failed at x = " +
                             DoubleFormatter::toString(*xBegin) +
                             "\n    interpolated value: " +
                             DoubleFormatter::toExponential(interpolated) + 
                             "\n    expected value:     " +
                             DoubleFormatter::toExponential(*yBegin));
            }
            ++xBegin; ++yBegin;
        }
    }

    template <class I, class J, class K>
    void checkNull1stDerivative(const char* type,
                                const CubicSplineInterpolation<I,J>& spline,
                                K xBegin, K xEnd) {
        double tolerance = 1.0e-16;
        double x = *xBegin;
        double interpolated = spline.derivative(x);
        if (QL_FABS(interpolated) > tolerance) {
            CPPUNIT_FAIL(std::string(type) + 
                         " interpolation first derivative failure\n"
                         "at x="
                         + DoubleFormatter::toString(x) +
                         "\n    interpolated value: "
                         + DoubleFormatter::toString(interpolated) +
                         "\n    expected value:     "
                         + DoubleFormatter::toString(0.0));
        }
        x = *(xEnd-1);
        interpolated = spline.derivative(x);
        if (QL_FABS(interpolated) > tolerance) {
            CPPUNIT_FAIL(std::string(type) +
                         " interpolation first derivative failure\n"
                         "at x="
                         + DoubleFormatter::toString(x) +
                         "\n    interpolated value: "
                         + DoubleFormatter::toString(interpolated) +
                         "\n    expected value:     "
                         + DoubleFormatter::toString(0.0));
        }
    }

    template <class I, class J, class K>
    void checkNull2ndDerivative(const char* type,
                                const CubicSplineInterpolation<I,J>& spline,
                                K xBegin, K xEnd) {
        double tolerance = 1.0e-16;
        double x = *xBegin;
        double interpolated = spline.secondDerivative(x);
        if (QL_FABS(interpolated) > tolerance) {
            CPPUNIT_FAIL(std::string(type) + 
                         " interpolation second derivative failure\n"
                         "at x="
                         + DoubleFormatter::toString(x) +
                         "\n    interpolated value: "
                         + DoubleFormatter::toString(interpolated) +
                         "\n    expected value:     "
                         + DoubleFormatter::toString(0.0));
        }
        x = *(xEnd-1);
        interpolated = spline.secondDerivative(x);
        if (QL_FABS(interpolated) > tolerance) {
            CPPUNIT_FAIL(std::string(type) +
                         " interpolation second derivative failure\n"
                         "at x="
                         + DoubleFormatter::toString(x) +
                         "\n    interpolated value: "
                         + DoubleFormatter::toString(interpolated) +
                         "\n    expected value:     "
                         + DoubleFormatter::toString(0.0));
        }
    }

    template <class I, class J>
    void checkNotAKnotCondition(const char* type,
                                const CubicSplineInterpolation<I,J>& spline) {

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

    template <class I, class J>
    void checkSymmetry(const char* type,
                       const CubicSplineInterpolation<I,J>& spline,
                       double xMin) {
        double tolerance = 1.0e-15;
        for (double x = xMin; x < 0.0; x += 0.1) {
            double y1 = spline(x), y2 = spline(-x);
            if (QL_FABS(y1-y2) > tolerance) {
                CPPUNIT_FAIL(std::string(type) +
                             " interpolation not symmetric"
                             "\n    x = "   + DoubleFormatter::toString(x) +
                             "\n    g(x) = " + DoubleFormatter::toString(y1) +
                             "\n    g(-x) = " + DoubleFormatter::toString(y1));
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

void InterpolationTest::testSplineErrorOnGaussianValues() {

    Size points[]                = {      5,      9,     17,     33 };

    double tabulatedErrors[]     = { 3.5e-2, 2.0e-3, 4.0e-5, 1.8e-6 };
    double toleranceOnTabErr[]   = { 0.1e-2, 0.1e-3, 0.1e-5, 0.1e-6 };

    double tabulatedMCErrors[]   = { 1.7e-2, 2.0e-3, 1.9e-3, 1.8e-6 };
    double toleranceOnTabMCErr[] = { 0.1e-2, 0.1e-3, 0.1e-3, 0.1e-6 };

//    KronrodIntegral integral(1e-12);
//    KronrodIntegral integral(3e-16);
    SimpsonIntegral integral(1e-12);
//    SegmentIntegral integral(2000);
//    TrapezoidIntegral integral(1e-12);
//    TrapezoidIntegral integral(1e-12,TrapezoidIntegral::MidPoint);
    std::vector<double> x, y;

    double scaleFactor = 1.9;

    for (Size i=0; i<LENGTH(points); i++) {
        Size n = points[i];
        std::vector<double> x = xRange(-1.7, 1.9, n);
        std::vector<double> y = gaussian(x);

        // Not-a-knot
        CubicSplineInterpolation<
            std::vector<double>::const_iterator,
            std::vector<double>::const_iterator> f(
                x.begin(), x.end(),
                y.begin(),
                CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
                Null<double>(),
                CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
                Null<double>(),
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
        f = CubicSplineInterpolation<
            std::vector<double>::const_iterator,
            std::vector<double>::const_iterator>(
                x.begin(), x.end(),
                y.begin(),
                CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
                Null<double>(),
                CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
                Null<double>(),
                true);
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

void InterpolationTest::testSplineOnGaussianValues() {
    double interpolated, interpolated2;
    Size n = 5;

    std::vector<double> x(n), y(n);
    double x1_bad=-1.7, x2_bad=1.7;

    for (double start = -1.9, j=0; j<2; start+=0.2, j++) {
        x = xRange(start, start+3.6, n);
        y = gaussian(x);

        // Not-a-knot spline
        CubicSplineInterpolation<
            std::vector<double>::const_iterator,
            std::vector<double>::const_iterator> interp(
                x.begin(), x.end(),
                y.begin(),
                CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
                Null<double>(),
                CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
                Null<double>(),
                false);
        checkValues("Not-a-knot spline", interp,
                    x.begin(), x.end(), y.begin());
        checkNotAKnotCondition("Not-a-knot spline", interp);
        // bad performance
        interpolated = interp(x1_bad);
        interpolated2= interp(x2_bad);
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
        interp = CubicSplineInterpolation<
            std::vector<double>::const_iterator,
            std::vector<double>::const_iterator>(
                x.begin(), x.end(),
                y.begin(),
                CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
                Null<double>(),
                CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
                Null<double>(),
                true);
        checkValues("MC not-a-knot spline", interp,
                    x.begin(), x.end(), y.begin());
        // good performance
        interpolated = interp(x1_bad);
        if (interpolated<0.0) {
            CPPUNIT_FAIL("MC not-a-knot spline interpolation "
                "good performance unverified\n"
                "at x="
                + DoubleFormatter::toString(x1_bad) +
                "\ninterpolated value: "
                + DoubleFormatter::toString(interpolated) +
                "\nexpected value > 0.0");
        }
        interpolated = interp(x2_bad);
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

    const double RPN15A_x[] = { 
        7.99,       8.09,       8.19,      8.7,
        9.2,     10.0,     12.0,     15.0,     20.0 
    };
    const double RPN15A_y[] = {
        0.0, 2.76429e-5, 4.37498e-5, 0.169183,
        0.469428, 0.943740, 0.998636, 0.999919, 0.999994 
    };

    double interpolated;
    Size n = LENGTH(RPN15A_x);

    // Natural spline
    CubicSplineInterpolation<const double*, const double*> interp(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::SecondDerivative,
            0.0,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::SecondDerivative,
            0.0,
            false);
    checkValues("Natural spline", interp,
                RPN15A_x, RPN15A_x+n, RPN15A_y);
    checkNull2ndDerivative("Natural spline", interp,
                           RPN15A_x, RPN15A_x+n);
    // poor performance
    double x_bad = 11.0;
    interpolated = interp(x_bad);
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
    interp = CubicSplineInterpolation<const double*, const double*>(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::FirstDerivative,
            0.0,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::FirstDerivative,
            0.0,
            false);
    checkValues("Clamped spline", interp,
                RPN15A_x, RPN15A_x+n, RPN15A_y);
    checkNull1stDerivative("Clamped spline", interp,
                           RPN15A_x, RPN15A_x+n);
    // poor performance
    interpolated = interp(x_bad);
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
    interp = CubicSplineInterpolation<const double*, const double*>(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
            Null<double>(),
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
            Null<double>(),
            false);
    checkValues("Not-a-knot spline", interp,
                RPN15A_x, RPN15A_x+n, RPN15A_y);
    checkNotAKnotCondition("Not-a-knot spline", interp);
    // poor performance
    interpolated = interp(x_bad);
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
    interp = CubicSplineInterpolation<const double*, const double*>(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::SecondDerivative,
            0.0,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::SecondDerivative,
            0.0,
            true);
    checkValues("MC natural spline", interp,
                RPN15A_x, RPN15A_x+n, RPN15A_y);
    // good performance
    interpolated = interp(x_bad);
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
    interp = CubicSplineInterpolation<const double*, const double*>(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::FirstDerivative,
            0.0,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::FirstDerivative,
            0.0,
            true);
    checkValues("MC clamped spline", interp,
                RPN15A_x, RPN15A_x+n, RPN15A_y);
    checkNull1stDerivative("MC clamped spline", interp,
                           RPN15A_x, RPN15A_x+n);
    // good performance
    interpolated = interp(x_bad);
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
    interp = CubicSplineInterpolation<const double*, const double*>(
            RPN15A_x, RPN15A_x+n,
            RPN15A_y,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
            Null<double>(),
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
            Null<double>(),
            true);
    checkValues("MC not-a-knot spline", interp,
                RPN15A_x, RPN15A_x+n, RPN15A_y);
    // good performance
    interpolated = interp(x_bad);
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

    const double generic_x[] = { 0.0, 1.0, 3.0, 4.0 };
    const double generic_y[] = { 0.0, 0.0, 2.0, 2.0 };
    const double generic_natural_y2[] = { 0.0, 1.5, -1.5, 0.0 };
    const double generic_natural_a[] = { -0.25,  0.50,  0.50 };
    const double generic_natural_b[] = {  0.00,  0.75, -0.75 };
    const double generic_natural_c[] = {  0.25, -0.25,  0.25 };

    double interpolated, error;
    Size i, n = LENGTH(generic_x);
    std::vector<double> x35(3);

    // Natural spline
    CubicSplineInterpolation<const double*, const double*> interp(
            generic_x, generic_x+n,
            generic_y,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::SecondDerivative,
            generic_natural_y2[0],
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::SecondDerivative,
            generic_natural_y2[n-1],
            false);
    checkValues("Natural spline", interp,
                generic_x, generic_x+n, generic_y);
    // cached second derivative
    for (i=0; i<n; i++) {
        interpolated = interp.secondDerivative(generic_x[i]);
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
    // cached polinomial coefficients
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
    interp = CubicSplineInterpolation<const double*, const double*>(
            generic_x, generic_x+n,
            generic_y,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::FirstDerivative,
            y1a,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::FirstDerivative,
            y1b,
            false);
    checkValues("Clamped spline", interp,
                generic_x, generic_x+n, generic_y);
    checkNull1stDerivative("Clamped spline", interp,
                           generic_x, generic_x+n);
    x35[0] = interp(3.5);


    // Not-a-knot spline
    interp = CubicSplineInterpolation<const double*, const double*>(
            generic_x, generic_x+n,
            generic_y,
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
            Null<double>(),
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
            Null<double>(),
            false);
    checkValues("Not-a-knot spline", interp,
                generic_x, generic_x+n, generic_y);
    checkNotAKnotCondition("Not-a-knot spline", interp);

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
    Size n = 9;

    std::vector<double> x, y;
    x = xRange(-1.8, 1.8, n);
    y = gaussian(x);

    // Not-a-knot spline
    CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator> interp(
            x.begin(), x.end(),
            y.begin(),
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
            Null<double>(),
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
            Null<double>(),
            false);
    checkValues("Not-a-knot spline", interp,
                x.begin(), x.end(), y.begin());
    checkNotAKnotCondition("Not-a-knot spline", interp);
    checkSymmetry("Not-a-knot spline", interp, x[0]);


    // MC not-a-knot spline
    interp = CubicSplineInterpolation<
        std::vector<double>::const_iterator,
        std::vector<double>::const_iterator>(
            x.begin(), x.end(),
            y.begin(),
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
            Null<double>(),
            CubicSplineInterpolation<
                    std::vector<double>::const_iterator,
                    std::vector<double>::const_iterator>::BoundaryCondition::Not_a_knot,
            Null<double>(),
            true);
    checkValues("MC not-a-knot spline", interp,
                x.begin(), x.end(), y.begin());
    checkSymmetry("MC Not-a-knot spline", interp, x[0]);
}


CppUnit::Test* InterpolationTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Interpolation tests");
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing spline interpolation on generic values",
                    &InterpolationTest::testSplineOnGenericValues));
    tests->addTest(new CppUnit::TestCaller<InterpolationTest>
                   ("Testing symmetry of spline interpolation end conditions",
                    &InterpolationTest::testingSimmetricEndConditions));
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

