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
// any warnings from this file and also prevent linting and formatting.
// clang-format off
// NOLINTBEGIN
#if defined(__GNUC__)
    _Pragma("GCC diagnostic push")
    _Pragma("GCC diagnostic ignored \"-Wsign-compare\"")
#elif defined(__clang__)
    _Pragma("clang diagnostic push")
    _Pragma("clang diagnostic ignored \"-Wsign-compare\"")
#endif

// The block inside extern "C" { ... } inlines the reference implementation
// from https://prng.di.unimi.it/xoshiro256starstar.c

extern "C" {
    /*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

    To the extent possible under law, the author has dedicated all copyright
    and related and neighboring rights to this software to the public domain
    worldwide. This software is distributed without any warranty.

    See <http://creativecommons.org/publicdomain/zero/1.0/>. */

    #include <stdint.h>

    /* This is xoshiro256** 1.0, one of our all-purpose, rock-solid
       generators. It has excellent (sub-ns) speed, a state (256 bits) that is
       large enough for any parallel application, and it passes all tests we
       are aware of.

       For generating just floating-point numbers, xoshiro256+ is even faster.

       The state must be seeded so that it is not everywhere zero. If you have
       a 64-bit seed, we suggest to seed a splitmix64 generator and use its
       output to fill s. */

    static inline uint64_t rotl(const uint64_t x, int k) {
            return (x << k) | (x >> (64 - k));
    }


    static uint64_t s[4];

    uint64_t next(void) {
            const uint64_t result = rotl(s[1] * 5, 7) * 9;

            const uint64_t t = s[1] << 17;

            s[2] ^= s[0];
            s[3] ^= s[1];
            s[1] ^= s[2];
            s[0] ^= s[3];

            s[2] ^= t;

            s[3] = rotl(s[3], 45);

            return result;
    }


    /* This is the jump function for the generator. It is equivalent
       to 2^128 calls to next(); it can be used to generate 2^128
       non-overlapping subsequences for parallel computations. */

    void jump(void) {
            static const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
                    for(int b = 0; b < 64; b++) {
                            if (JUMP[i] & UINT64_C(1) << b) {
                                    s0 ^= s[0];
                                    s1 ^= s[1];
                                    s2 ^= s[2];
                                    s3 ^= s[3];
                            }
                            next();
                    }

            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
    }



    /* This is the long-jump function for the generator. It is equivalent to
       2^192 calls to next(); it can be used to generate 2^64 starting points,
       from each of which jump() will generate 2^64 non-overlapping
       subsequences for parallel distributed computations. */

    void long_jump(void) {
            static const uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for(int i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
                    for(int b = 0; b < 64; b++) {
                            if (LONG_JUMP[i] & UINT64_C(1) << b) {
                                    s0 ^= s[0];
                                    s1 ^= s[1];
                                    s2 ^= s[2];
                                    s3 ^= s[3];
                            }
                            next();
                    }

            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
    }
}

#if defined(__GNUC__)
    _Pragma("GCC diagnostic pop")
#elif defined(__clang__)
    _Pragma("clang diagnostic pop")
#endif
// NOLINTEND
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
    for (auto j = 0; j < iterations; ++j) {
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
    static const auto s0 = 10108360646465513120ULL;
    static const auto s1 = 4416403493985791904ULL;
    static const auto s2 = 7597776674045431742ULL;
    static const auto s3 = 6431387443075032236ULL;

    // simulate the warmup in our implementation
    // by burning the first 1,000 random numbers in the reference implementation
    s[0] = s0;
    s[1] = s1;
    s[2] = s2;
    s[3] = s3;
    for (auto i = 0; i < 1'000; ++i) {
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
