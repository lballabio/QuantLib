
/*
 Copyright (C) 2003 RiskMap srl

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

#include "integrals.hpp"
#include <ql/Math/segmentintegral.hpp>
#include <ql/Math/simpsonintegral.hpp>
#include <ql/Math/trapezoidintegral.hpp>
#include <ql/Math/kronrodintegral.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/Math/functional.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

namespace {

    double tolerance = 1.0e-4;

    template <class T, class F>
    void testSingle(const T& I, const std::string& tag,
                    const F& f, double xMin, double xMax, double expected) {
        double calculated = I(f,xMin,xMax);
        if (QL_FABS(calculated-expected) > tolerance) {
            CPPUNIT_FAIL(
                "integrating " + tag +
                "    calculated: " + DoubleFormatter::toString(calculated) +
                "    expected:   " + DoubleFormatter::toString(expected));
        }
    }

    template <class T>
    void testSeveral(const T& I) {
        testSingle(I, "f(x) = 1",
                   constant<double,double>(1.0), 0.0, 1.0, 1.0);
        testSingle(I, "f(x) = x",
                   identity<double>(),           0.0, 1.0, 0.5);
        testSingle(I, "f(x) = x^2", 
                   square<double>(),             0.0, 1.0, 1.0/3.0);
        testSingle(I, "f(x) = sin(x)", 
                   std::ptr_fun<double,double>(QL_SIN), 0.0, M_PI, 2.0);
        testSingle(I, "f(x) = cos(x)", 
                   std::ptr_fun<double,double>(QL_COS), 0.0, M_PI, 0.0);
        testSingle(I, "f(x) = Gaussian(x)", 
                   NormalDistribution(), -10.0, 10.0, 1.0);
    }
}

void IntegralTest::testSegment() {
    testSeveral(SegmentIntegral(10000));
}

void IntegralTest::testTrapezoid() {
    testSeveral(TrapezoidIntegral(tolerance));
}

void IntegralTest::testMidPointTrapezoid() {
    testSeveral(TrapezoidIntegral(tolerance,TrapezoidIntegral::MidPoint));
}

void IntegralTest::testSimpson() {
    testSeveral(SimpsonIntegral(tolerance));
}

void IntegralTest::testKronrod() {
    testSeveral(KronrodIntegral(tolerance));
}

CppUnit::Test* IntegralTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Integration tests");
    tests->addTest(new CppUnit::TestCaller<IntegralTest>
                   ("Testing segment integration",
                    &IntegralTest::testSegment));
    tests->addTest(new CppUnit::TestCaller<IntegralTest>
                   ("Testing trapezoid integration",
                    &IntegralTest::testTrapezoid));
    tests->addTest(new CppUnit::TestCaller<IntegralTest>
                   ("Testing mid-point trapezoid integration",
                    &IntegralTest::testMidPointTrapezoid));
    tests->addTest(new CppUnit::TestCaller<IntegralTest>
                   ("Testing Simpson integration",
                    &IntegralTest::testSimpson));
    tests->addTest(new CppUnit::TestCaller<IntegralTest>
                   ("Testing Gauss-Kronrod integration",
                    &IntegralTest::testKronrod));
    return tests;
}

