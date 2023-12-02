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

#include "prices.hpp"
#include "utilities.hpp"
#include <ql/prices.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {
    namespace tt = boost::test_tools;

    [[maybe_unused]] void testMidEquivalent() {
        BOOST_TEST_MESSAGE("Testing midEquivalent()...");

        BOOST_TEST(1.5 == midEquivalent(1, 2, 3, 4), tt::tolerance(1e-14));
        BOOST_TEST(1.5 == midEquivalent(1, 2, 0, 4), tt::tolerance(1e-14));
        BOOST_TEST(1.5 == midEquivalent(1, 2, 3, 0), tt::tolerance(1e-14));
        BOOST_TEST(1.5 == midEquivalent(1, 2, 0, 4), tt::tolerance(1e-14));

        BOOST_TEST(1 == midEquivalent(1, 0, 3, 4));
        BOOST_TEST(1 == midEquivalent(1, 0, 0, 4));
        BOOST_TEST(1 == midEquivalent(1, 0, 3, 0));
        BOOST_TEST(1 == midEquivalent(1, 0, 0, 4));

        BOOST_TEST(2 == midEquivalent(0, 2, 3, 4));
        BOOST_TEST(2 == midEquivalent(0, 2, 0, 4));
        BOOST_TEST(2 == midEquivalent(0, 2, 3, 0));
        BOOST_TEST(2 == midEquivalent(0, 2, 0, 4));

        BOOST_TEST(3 == midEquivalent(0, 0, 3, 4));
        BOOST_TEST(4 == midEquivalent(0, 0, 0, 4));
        BOOST_TEST(3 == midEquivalent(0, 0, 3, 0));
        BOOST_TEST(4 == midEquivalent(0, 0, 0, 4));

        BOOST_CHECK_THROW(midEquivalent(0, 0, 0, 0), QuantLib::Error);
    }

    [[maybe_unused]] void testMidSafe() {
        BOOST_TEST_MESSAGE("Testing midSafe()...");

        BOOST_TEST(1.5 == midSafe(1, 2), tt::tolerance(1e-14));

        BOOST_CHECK_THROW(midSafe(0, 0), QuantLib::Error);
        BOOST_CHECK_THROW(midSafe(1, 0), QuantLib::Error);
        BOOST_CHECK_THROW(midSafe(0, 2), QuantLib::Error);
    }
} // namespace

test_suite* priceTestSuite() {
    auto* suite = BOOST_TEST_SUITE("Prices tests");
    suite->add(QUANTLIB_TEST_CASE(&testMidEquivalent));
    suite->add(QUANTLIB_TEST_CASE(&testMidSafe));
    return suite;
}
