/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

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

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    Real tolerance = 1.0e-4;

    template <class T, class F>
    void testSingle(const T& I, const std::string& tag,
                    const F& f, Real xMin, Real xMax, Real expected) {
        Real calculated = I(f,xMin,xMax);
        if (std::fabs(calculated-expected) > tolerance) {
            BOOST_FAIL("integrating " << tag
                       << "    calculated: " << calculated
                       << "    expected:   " << expected);
        }
    }

    template <class T>
    void testSeveral(const T& I) {
        testSingle(I, "f(x) = 1",
                   constant<Real,Real>(1.0), 0.0, 1.0, 1.0);
        testSingle(I, "f(x) = x",
                   identity<Real>(),           0.0, 1.0, 0.5);
        testSingle(I, "f(x) = x^2",
                   square<Real>(),             0.0, 1.0, 1.0/3.0);
        testSingle(I, "f(x) = sin(x)",
                   std::ptr_fun<Real,Real>(std::sin), 0.0, M_PI, 2.0);
        testSingle(I, "f(x) = cos(x)",
                   std::ptr_fun<Real,Real>(std::cos), 0.0, M_PI, 0.0);
        testSingle(I, "f(x) = Gaussian(x)",
                   NormalDistribution(), -10.0, 10.0, 1.0);
    }
}

void IntegralTest::testSegment() {
    BOOST_MESSAGE("Testing segment integration...");
    testSeveral(SegmentIntegral(10000));
}

void IntegralTest::testTrapezoid() {
    BOOST_MESSAGE("Testing trapezoid integration...");
    testSeveral(TrapezoidIntegral(tolerance));
}

void IntegralTest::testMidPointTrapezoid() {
    BOOST_MESSAGE("Testing mid-point trapezoid integration...");
    testSeveral(TrapezoidIntegral(tolerance,TrapezoidIntegral::MidPoint));
}

void IntegralTest::testSimpson() {
    BOOST_MESSAGE("Testing Simpson integration...");
    testSeveral(SimpsonIntegral(tolerance));
}

void IntegralTest::testKronrod() {
    BOOST_MESSAGE("Testing Gauss-Kronrod integration...");
    testSeveral(KronrodIntegral(tolerance));
}


test_suite* IntegralTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Integration tests");
    suite->add(BOOST_TEST_CASE(&IntegralTest::testSegment));
    suite->add(BOOST_TEST_CASE(&IntegralTest::testTrapezoid));
    suite->add(BOOST_TEST_CASE(&IntegralTest::testMidPointTrapezoid));
    suite->add(BOOST_TEST_CASE(&IntegralTest::testSimpson));
    suite->add(BOOST_TEST_CASE(&IntegralTest::testKronrod));
    return suite;
}

