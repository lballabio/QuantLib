/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Ralf Konrad Eckel

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

#include "inflationzciisinterpolation.hpp"
#include "utilities.hpp"
#include <iostream>

using boost::unit_test_framework::test_suite;
using namespace QuantLib;

void InflationZCIISInterpolationTest::testZCIISAsIndex() {
    BOOST_TEST_MESSAGE("Testing ZCIIS CPI::AsIndex interpolation type...");
}

void InflationZCIISInterpolationTest::testZCIISFlat() {
    BOOST_TEST_MESSAGE("Testing ZCIIS CPI::Flat interpolation type...");
}

void InflationZCIISInterpolationTest::testZCIISLinear() {
    BOOST_TEST_MESSAGE("Testing ZCIIS CPI::Linear interpolation type...");
}

test_suite* InflationZCIISInterpolationTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Zero Coupon Inflation Index Swap (ZCIIS) interpolation tests");

    suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::testZCIISAsIndex));
    suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::testZCIISFlat));
    suite->add(QUANTLIB_TEST_CASE(&InflationZCIISInterpolationTest::testZCIISLinear));

    return suite;
}
