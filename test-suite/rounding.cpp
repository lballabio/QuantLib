
/*
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

#include "rounding.hpp"
#include "utilities.hpp"
#include <ql/Math/rounding.hpp>
#include <ql/Math/comparison.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct TestCase {
        Decimal x;
        Integer precision;
        Decimal up;
        Decimal down;
    };

    TestCase testData[] = {
        {  0.86313513, 5,  0.86314,  0.86313 },
        { -7.64555346, 1, -7.6,     -7.6     },
        {  0.13961605, 2,  0.14,     0.13    },
        {  0.14344179, 4,  0.1434,   0.1434  },
        { -4.74315016, 2, -4.74,    -4.74    },
        { -7.82772074, 5, -7.82772, -7.82772 },
        {  2.74137947, 3,  2.741,    2.741   },
        {  2.13056714, 1,  2.1,      2.1     },
        { -1.06228670, 1, -1.1,     -1.0     },
        {  8.29234094, 4,  8.2923,   8.2923  },
        {  7.90185598, 2,  7.90,     7.90    },
        { -0.26738058, 1, -0.3,     -0.2     },
        {  1.78128713, 1,  1.8,      1.7     },
        {  4.23537260, 1,  4.2,      4.2     },
        {  3.64369953, 4,  3.6437,   3.6436  },
        {  6.34542470, 2,  6.35,     6.34    },
        { -0.84754962, 4, -0.8475,  -0.8475  },
        {  4.60998652, 1,  4.6,      4.6     },
        {  6.28794223, 3,  6.288,    6.287   },
        {  7.89428221, 2,  7.89,     7.89    }
    };

}

void RoundingTest::testUp() {

    BOOST_MESSAGE("Testing upward decimal rounding...");

    for (Size i=0; i<LENGTH(testData); i++) {
        Integer digits = testData[i].precision;
        Rounding up(digits, Rounding::Up);
        Real calculated = up.round(testData[i].x);
        Real expected = testData[i].up;
        if (!close(calculated,expected,1))
            BOOST_FAIL("Original number: " + 
                       DecimalFormatter::toString(testData[i].x,8) + "\n"
                       "Expected:        " +
                       DecimalFormatter::toString(expected,digits) + "\n"
                       "Calculated:      " +
                       DecimalFormatter::toString(calculated,digits));
    }
}

void RoundingTest::testDown() {

    BOOST_MESSAGE("Testing downward decimal rounding...");

    for (Size i=0; i<LENGTH(testData); i++) {
        Integer digits = testData[i].precision;
        Rounding down(digits, Rounding::Down);
        Real calculated = down.round(testData[i].x);
        Real expected = testData[i].down;
        if (!close(calculated,expected,1))
            BOOST_FAIL("Original number: " + 
                       DecimalFormatter::toString(testData[i].x,8) + "\n"
                       "Expected:        " +
                       DecimalFormatter::toString(expected,digits) + "\n"
                       "Calculated:      " +
                       DecimalFormatter::toString(calculated,digits));
    }
}

void RoundingTest::testFloor() {

    BOOST_MESSAGE("Testing floor decimal rounding...");

    for (Size i=0; i<LENGTH(testData); i++) {
        Integer digits = testData[i].precision;
        Rounding floor(digits, Rounding::Floor);
        Real calculated = floor.round(testData[i].x);
        Real expected = testData[i].x >= 0.0 ?
                        testData[i].up :
                        testData[i].down;
        if (!close(calculated,expected,1))
            BOOST_FAIL("Original number: " + 
                       DecimalFormatter::toString(testData[i].x,8) + "\n"
                       "Expected:        " +
                       DecimalFormatter::toString(expected,digits) + "\n"
                       "Calculated:      " +
                       DecimalFormatter::toString(calculated,digits));
    }
}

void RoundingTest::testCeiling() {

    BOOST_MESSAGE("Testing ceiling decimal rounding...");

    for (Size i=0; i<LENGTH(testData); i++) {
        Integer digits = testData[i].precision;
        Rounding ceiling(digits, Rounding::Ceiling);
        Real calculated = ceiling.round(testData[i].x);
        Real expected = testData[i].x >= 0.0 ?
                        testData[i].down :
                        testData[i].up;
        if (!close(calculated,expected,1))
            BOOST_FAIL("Original number: " + 
                       DecimalFormatter::toString(testData[i].x,8) + "\n"
                       "Expected:        " +
                       DecimalFormatter::toString(expected,digits) + "\n"
                       "Calculated:      " +
                       DecimalFormatter::toString(calculated,digits));
    }
}


test_suite* RoundingTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Rounding tests");
    suite->add(BOOST_TEST_CASE(&RoundingTest::testUp));
    suite->add(BOOST_TEST_CASE(&RoundingTest::testDown));
    suite->add(BOOST_TEST_CASE(&RoundingTest::testFloor));
    suite->add(BOOST_TEST_CASE(&RoundingTest::testCeiling));
    return suite;
}

