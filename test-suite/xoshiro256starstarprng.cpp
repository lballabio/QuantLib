/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Ralf Konrad Eckel

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

#include "xoshiro256starstarprng.hpp"
#include "utilities.hpp"
#include <ql/math/randomnumbers/xoshiro256starstarprng.hpp>

void Xoshiro256StarStarTest::testPRNG() {
    BOOST_TEST_MESSAGE("Testing Xoshiro256StarStar");
    auto random = QuantLib::Xoshiro256StarStar(1);
    for (int i = 0; i < 100000; ++i) {
        auto next = random.nextReal();
        if (next < 0.0 && 1.0 < next) {
            BOOST_ERROR("next not in range ");
        }
    }
}

boost::unit_test_framework::test_suite* Xoshiro256StarStarTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Xoshiro256StarStar Test");

    suite->add(QUANTLIB_TEST_CASE(&Xoshiro256StarStarTest::testPRNG));

    return suite;
}
