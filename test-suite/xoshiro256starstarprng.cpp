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
#include <numeric>

using QuantLib::Real;
using QuantLib::Xoshiro256StarStar;

void Xoshiro256StarStarTest::testMeanAndStdDevOfNextReal() {
    BOOST_TEST_MESSAGE("Testing Xoshiro256StarStar");

    auto random = Xoshiro256StarStar(1);
    const auto iterations = 10000000;
    auto randoms = std::vector<Real>();
    randoms.reserve(iterations);
    for (int j = 0; j < iterations; ++j) {
        auto next = random.nextReal();
        if (next < 0.0 || 1.0 < next) {
            BOOST_ERROR("next not in range");
            return;
        }
        randoms.push_back(next);
    }
    auto mean = std::accumulate(randoms.begin(), randoms.end(), 0.0) / randoms.size();
    auto meanError = std::fabs(0.5 - mean);
    if (meanError > 0.005) {
        BOOST_ERROR("Mean " << mean << " for seed 1 is not close to 0.5.");
    }
    std::vector<double> diff(randoms.size());
    std::transform(randoms.begin(), randoms.end(), diff.begin(),
                   [mean](double x) { return x - mean; });
    auto stdDev = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0) / randoms.size();
    auto stdDevError = std::fabs(1.0 / 12.0 - stdDev);
    if (stdDevError > 0.00005) {
        BOOST_ERROR("Standard deviation " << stdDev << " for seed 1 is not close to 1/12.");
    }
}

boost::unit_test_framework::test_suite* Xoshiro256StarStarTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Xoshiro256StarStar Test");

    suite->add(QUANTLIB_TEST_CASE(&Xoshiro256StarStarTest::testMeanAndStdDevOfNextReal));

    return suite;
}
