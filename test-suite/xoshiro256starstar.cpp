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

#include "xoshiro256starstar.hpp"
#include "utilities.hpp"
#include <ql/math/randomnumbers/xoshiro256starstaruniformrng.hpp>
#include <numeric>

// we do not want to change the original xoshiro256starstar.c implementation. Therefore, we suppress
// any warnings from this file.
// clang-format off
#if defined(__GNUC__)
    _Pragma("GCC diagnostic push")
    _Pragma("GCC diagnostic ignored \"-Wsign-compare\"")
#elif defined(__clang__)
    _Pragma("clang diagnostic push")
    _Pragma("clang diagnostic ignored \"-Wsign-compare\"")
#endif

extern "C" {
#include "xoshiro256starstar.c"
}

#if defined(__GNUC__)
    _Pragma("GCC diagnostic pop")
#elif defined(__clang__)
    _Pragma("clang diagnostic pop")
#endif
    // clang-format on

    using QuantLib::Real;
using QuantLib::Xoshiro256StarStarUniformRng;

void Xoshiro256StarStarTest::testMeanAndStdDevOfNextReal() {
    BOOST_TEST_MESSAGE(
        "Testing Xoshiro256StarStarUniformRng::nextReal() for mean=0.5 and stddev=1/12");

    auto random = Xoshiro256StarStarUniformRng(1);
    const auto iterations = 10'000'000;
    auto randoms = std::vector<Real>();
    randoms.reserve(iterations);
    for (int j = 0; j < iterations; ++j) {
        auto next = random.nextReal();
        if (next <= 0.0 || 1.0 <= next) {
            BOOST_FAIL("next " << next << " not in range");
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

void Xoshiro256StarStarTest::testAgainstReferenceImplementationInC() {
    BOOST_TEST_MESSAGE(
        "Testing Xoshiro256StarStarUniformRng::nextInt64() against reference implementation in C");

    // some random initial seed
    static const unsigned long long s0 = 10108360646465513120ULL;
    static const unsigned long long s1 = 4416403493985791904ULL;
    static const unsigned long long s2 = 7597776674045431742ULL;
    static const unsigned long long s3 = 6431387443075032236ULL;

    // simulate the warmup in our implementation
    // by burning the first 1,000 random numbers in the reference implementation
    s[0] = s0;
    s[1] = s1;
    s[2] = s2;
    s[3] = s3;
    for (int i = 0; i < 1'000; ++i) {
        next();
    }

    auto rng = Xoshiro256StarStarUniformRng(s0, s1, s2, s3);
    for (auto i = 0; i < 1'000; i++) {
        auto nextRefImpl = next();
        auto nextOurs = rng.nextInt64();
        if (nextRefImpl != nextOurs) {
            BOOST_FAIL("Test failed at index "
                       << i << " (expected from reference implementation: " << nextRefImpl
                       << "ULL, ours: " << nextOurs << "ULL)");
        }
    }
}

void Xoshiro256StarStarTest::testAbsenceOfInteractionBetweenInstances() {
    BOOST_TEST_MESSAGE(
        "Testing Xoshiro256StarStarUniformRng for absence of interaction between instances");

    auto seed = 16880566536755896171ULL;
    Xoshiro256StarStarUniformRng rng(seed);
    for (auto i = 0; i < 999; ++i)
        rng.nextInt64();
    auto referenceValue = rng.nextInt64();

    // sequential use
    Xoshiro256StarStarUniformRng rng1(seed), rng2(seed);
    for (auto i = 0; i < 1'000; i++)
        rng1.nextInt64();
    for (auto i = 0; i < 999; i++)
        rng2.nextInt64();
    if (referenceValue != rng2.nextInt64())
        BOOST_FAIL("Detected interaction between Xoshiro256StarStarUniformRng instances during "
                   "sequential computation");

    // parallel use
    Xoshiro256StarStarUniformRng rng3(seed), rng4(seed);
    for (auto i = 0; i < 999; i++) {
        rng3.nextInt64();
        rng4.nextInt64();
    }
    if (referenceValue != rng3.nextInt64() || referenceValue != rng4.nextInt64())
        BOOST_FAIL("Detected interaction between Xoshiro256StarStarUniformRng instances during "
                   "parallel computation");
}

boost::unit_test_framework::test_suite* Xoshiro256StarStarTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Xoshiro256StarStar Tests");

    suite->add(QUANTLIB_TEST_CASE(&Xoshiro256StarStarTest::testMeanAndStdDevOfNextReal));
    suite->add(QUANTLIB_TEST_CASE(&Xoshiro256StarStarTest::testAgainstReferenceImplementationInC));
    suite->add(
        QUANTLIB_TEST_CASE(&Xoshiro256StarStarTest::testAbsenceOfInteractionBetweenInstances));

    return suite;
}
