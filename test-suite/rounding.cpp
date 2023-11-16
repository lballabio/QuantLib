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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/math/rounding.hpp>
#include <ql/math/comparison.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace rounding_test {

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

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(RoundingTest)

BOOST_AUTO_TEST_CASE(testClosest) {

    BOOST_TEST_MESSAGE("Testing closest decimal rounding...");

    using namespace rounding_test;

    for (auto& i : testData) {
        Integer digits = i.precision;
        ClosestRounding closest(digits);
        Real calculated = closest(i.x);
        Real expected = i.closest;
        if (!close(calculated,expected,1))
            BOOST_ERROR(std::fixed << std::setprecision(8) << "Original number: " << i.x << "\n"
                                   << std::setprecision(digits) << "Expected:        " << expected
                                   << "\n"
                                   << "Calculated:      " << calculated);
    }
}

BOOST_AUTO_TEST_CASE(testUp) {

    BOOST_TEST_MESSAGE("Testing upward decimal rounding...");

    using namespace rounding_test;

    for (auto& i : testData) {
        Integer digits = i.precision;
        UpRounding up(digits);
        Real calculated = up(i.x);
        Real expected = i.up;
        if (!close(calculated,expected,1))
            BOOST_ERROR(std::fixed << std::setprecision(8) << "Original number: " << i.x << "\n"
                                   << std::setprecision(digits) << "Expected:        " << expected
                                   << "\n"
                                   << "Calculated:      " << calculated);
    }
}

BOOST_AUTO_TEST_CASE(testDown) {

    BOOST_TEST_MESSAGE("Testing downward decimal rounding...");

    using namespace rounding_test;

    for (auto& i : testData) {
        Integer digits = i.precision;
        DownRounding down(digits);
        Real calculated = down(i.x);
        Real expected = i.down;
        if (!close(calculated,expected,1))
            BOOST_ERROR(std::fixed << std::setprecision(8) << "Original number: " << i.x << "\n"
                                   << std::setprecision(digits) << "Expected:        " << expected
                                   << "\n"
                                   << "Calculated:      " << calculated);
    }
}

BOOST_AUTO_TEST_CASE(testFloor) {

    BOOST_TEST_MESSAGE("Testing floor decimal rounding...");

    using namespace rounding_test;

    for (auto& i : testData) {
        Integer digits = i.precision;
        FloorTruncation floor(digits);
        Real calculated = floor(i.x);
        Real expected = i.floor;
        if (!close(calculated,expected,1))
            BOOST_ERROR(std::fixed << std::setprecision(8) << "Original number: " << i.x << "\n"
                                   << std::setprecision(digits) << "Expected:        " << expected
                                   << "\n"
                                   << "Calculated:      " << calculated);
    }
}

BOOST_AUTO_TEST_CASE(testCeiling) {

    BOOST_TEST_MESSAGE("Testing ceiling decimal rounding...");

    using namespace rounding_test;

    for (auto& i : testData) {
        Integer digits = i.precision;
        CeilingTruncation ceiling(digits);
        Real calculated = ceiling(i.x);
        Real expected = i.ceiling;
        if (!close(calculated,expected,1))
            BOOST_ERROR(std::fixed << std::setprecision(8) << "Original number: " << i.x << "\n"
                                   << std::setprecision(digits) << "Expected:        " << expected
                                   << "\n"
                                   << "Calculated:      " << calculated);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()