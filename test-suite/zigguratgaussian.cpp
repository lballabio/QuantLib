/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Ralf Konrad Eckel

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
#include <ql/math/randomnumbers/xoshiro256starstaruniformrng.hpp>
#include <ql/math/randomnumbers/zigguratgaussianrng.hpp>
#include <bitset>
#include <iostream>
#include <numeric>

using namespace QuantLib;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(ZigguratGaussianTests)

BOOST_AUTO_TEST_CASE(testMeanAndStdDevOfNextReal) {
    BOOST_TEST_MESSAGE("Testing ZigguratGaussianRng<Xoshiro256StarStarUniformRng>::nextReal() for "
                       "mean=0.0 and stddev=1.0...");

    auto uniformRandom = Xoshiro256StarStarUniformRng(1);
    auto random = ZigguratGaussianRng<Xoshiro256StarStarUniformRng>(uniformRandom);

    const auto iterations = 10'000'000;
    auto randoms = std::vector<Real>();
    randoms.reserve(iterations);
    for (auto j = 0; j < iterations; ++j) {
        auto next = random.nextReal();
        randoms.push_back(next);
    }
    Real mean = std::accumulate(randoms.begin(), randoms.end(), Real(0.0)) / randoms.size();
    Real meanError = std::fabs(0.0 - mean);
    if (meanError > 0.005) {
        BOOST_ERROR("Mean " << mean << " for seed 1 is not close to 0.");
    }
    std::vector<Real> diff(randoms.size());
    std::transform(randoms.begin(), randoms.end(), diff.begin(),
                   [mean](Real x) -> Real { return x - mean; });
    Real stdDev =
        std::inner_product(diff.begin(), diff.end(), diff.begin(), Real(0.0)) / randoms.size();
    Real stdDevError = std::fabs(1.0 - stdDev);
    if (stdDevError > 0.00005) {
        BOOST_ERROR("Standard deviation " << stdDev << " for seed 1 is not close to 1.");
    }
}

BOOST_AUTO_TEST_CASE(testBitsShifting) {
    std::uint64_t uint64 = 0xffffffffffffffff - 0xffffffffffffff - 1 + 25;
    BOOST_TEST_MESSAGE("" << std::bitset<64>(uint64) << " : " << uint64);
    BOOST_TEST_MESSAGE("" << std::bitset<64>(uint64 >> 8) << " : " << (uint64 >> 8));
    BOOST_TEST_MESSAGE("" << std::bitset<64>(uint64 & 0xff) << " : " << (uint64 & 0xff));
    BOOST_TEST_MESSAGE("" << std::bitset<64>(uint64 & 0xff) << " : " << (int)(uint64 & 0xff));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
