/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 StatPro Italia srl

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

#include "rounding.hpp"
#include "utilities.hpp"
#include <ql/math/rounding.hpp>
#include <ql/math/comparison.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct TestCase {
        Decimal x;
        Integer precision;
        Decimal closest;
        Decimal up;
        Decimal down;
        Decimal floor;
        Decimal ceiling;
    };

    TestCase testData[] = {
        {  0.86313513, 5,  0.86314,  0.86314,  0.86313,  0.86314,  0.86313 },
        {  0.86313,    5,  0.86313,  0.86313,  0.86313,  0.86313,  0.86313 },
        { -7.64555346, 1, -7.6,     -7.7,     -7.6,     -7.6,     -7.6     },
        {  0.13961605, 2,  0.14,     0.14,     0.13,     0.14,     0.13    },
        {  0.14344179, 4,  0.1434,   0.1435,   0.1434,   0.1434,   0.1434  },
        { -4.74315016, 2, -4.74,    -4.75,    -4.74,    -4.74,    -4.74    },
        { -7.82772074, 5, -7.82772, -7.82773, -7.82772, -7.82772, -7.82772 },
        {  2.74137947, 3,  2.741,    2.742,    2.741,    2.741,    2.741   },
        {  2.13056714, 1,  2.1,      2.2,      2.1,      2.1,      2.1     },
        { -1.06228670, 1, -1.1,     -1.1,     -1.0,     -1.0,     -1.1     },
        {  8.29234094, 4,  8.2923,   8.2924,   8.2923,   8.2923,   8.2923  },
        {  7.90185598, 2,  7.90,     7.91,     7.90,     7.90,     7.90    },
        { -0.26738058, 1, -0.3,     -0.3,     -0.2,     -0.2,     -0.3     },
        {  1.78128713, 1,  1.8,      1.8,      1.7,      1.8,      1.7     },
        {  4.23537260, 1,  4.2,      4.3,      4.2,      4.2,      4.2     },
        {  3.64369953, 4,  3.6437,   3.6437,   3.6436,   3.6437,   3.6436  },
        {  6.34542470, 2,  6.35,     6.35,     6.34,     6.35,     6.34    },
        { -0.84754962, 4, -0.8475,  -0.8476,  -0.8475,  -0.8475,  -0.8475  },
        {  4.60998652, 1,  4.6,      4.7,      4.6,      4.6,      4.6     },
        {  6.28794223, 3,  6.288,    6.288,    6.287,    6.288,    6.287   },
        {  7.89428221, 2,  7.89,     7.90,     7.89,     7.89,     7.89    }
    };

}


void RoundingTest::testClosest() {

    BOOST_TEST_MESSAGE("Testing closest decimal rounding...");

    for (Size i=0; i<LENGTH(testData); i++) {
        Integer digits = testData[i].precision;
        ClosestRounding closest(digits);
        Real calculated = closest(testData[i].x);
        Real expected = testData[i].closest;
        if (!close(calculated,expected,1))
            BOOST_ERROR(QL_FIXED << std::setprecision(8)
                        << "Original number: " << testData[i].x << "\n"
                        << std::setprecision(digits)
                        << "Expected:        " << expected << "\n"
                        << "Calculated:      " << calculated);
    }
}

void RoundingTest::testUp() {

    BOOST_TEST_MESSAGE("Testing upward decimal rounding...");

    for (Size i=0; i<LENGTH(testData); i++) {
        Integer digits = testData[i].precision;
        UpRounding up(digits);
        Real calculated = up(testData[i].x);
        Real expected = testData[i].up;
        if (!close(calculated,expected,1))
            BOOST_ERROR(QL_FIXED << std::setprecision(8)
                        << "Original number: " << testData[i].x << "\n"
                        << std::setprecision(digits)
                        << "Expected:        " << expected << "\n"
                        << "Calculated:      " << calculated);
    }
}

void RoundingTest::testDown() {

    BOOST_TEST_MESSAGE("Testing downward decimal rounding...");

    for (Size i=0; i<LENGTH(testData); i++) {
        Integer digits = testData[i].precision;
        DownRounding down(digits);
        Real calculated = down(testData[i].x);
        Real expected = testData[i].down;
        if (!close(calculated,expected,1))
            BOOST_ERROR(QL_FIXED << std::setprecision(8)
                        << "Original number: " << testData[i].x << "\n"
                        << std::setprecision(digits)
                        << "Expected:        " << expected << "\n"
                        << "Calculated:      " << calculated);
    }
}

void RoundingTest::testFloor() {

    BOOST_TEST_MESSAGE("Testing floor decimal rounding...");

    for (Size i=0; i<LENGTH(testData); i++) {
        Integer digits = testData[i].precision;
        FloorTruncation floor(digits);
        Real calculated = floor(testData[i].x);
        Real expected = testData[i].floor;
        if (!close(calculated,expected,1))
            BOOST_ERROR(QL_FIXED << std::setprecision(8)
                        << "Original number: " << testData[i].x << "\n"
                        << std::setprecision(digits)
                        << "Expected:        " << expected << "\n"
                        << "Calculated:      " << calculated);
    }
}

void RoundingTest::testCeiling() {

    BOOST_TEST_MESSAGE("Testing ceiling decimal rounding...");

    for (Size i=0; i<LENGTH(testData); i++) {
        Integer digits = testData[i].precision;
        CeilingTruncation ceiling(digits);
        Real calculated = ceiling(testData[i].x);
        Real expected = testData[i].ceiling;
        if (!close(calculated,expected,1))
            BOOST_ERROR(QL_FIXED << std::setprecision(8)
                        << "Original number: " << testData[i].x << "\n"
                        << std::setprecision(digits)
                        << "Expected:        " << expected << "\n"
                        << "Calculated:      " << calculated);
    }
}


test_suite* RoundingTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Rounding tests");
    suite->add(QUANTLIB_TEST_CASE(&RoundingTest::testClosest));
    suite->add(QUANTLIB_TEST_CASE(&RoundingTest::testUp));
    suite->add(QUANTLIB_TEST_CASE(&RoundingTest::testDown));
    suite->add(QUANTLIB_TEST_CASE(&RoundingTest::testFloor));
    suite->add(QUANTLIB_TEST_CASE(&RoundingTest::testCeiling));
    return suite;
}

