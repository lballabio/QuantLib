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

#include <ql/math/randomnumbers/seedgenerator.hpp>
#include <ql/math/randomnumbers/xoshiro256starstaruniformrng.hpp>

namespace QuantLib {

    Xoshiro256StarStarUniformRng::Xoshiro256StarStarUniformRng(uint64_t seed)
    : Xoshiro256StarStarUniformRng(seed, seed, seed, seed) {}

    Xoshiro256StarStarUniformRng::Xoshiro256StarStarUniformRng(uint64_t s0,
                                                               uint64_t s1,
                                                               uint64_t s2,
                                                               uint64_t s3)
    : s0_(s0), s1_(s1), s2_(s2), s3_(s3) {
        seedInitialization();

        // We call nextInt64() 1,000 times as the first random numbers
        // might be "close" to the given seeds.
        // See https://github.com/lballabio/QuantLib/pull/1769/files#r1296997133 */
        for (auto i = 0; i < 1'000; ++i) {
            nextInt64();
        }
    }

    void Xoshiro256StarStarUniformRng::seedInitialization() const {
        if (s0_ == 0 && s1_ == 0 && s2_ == 0 && s3_ == 0) {
            do {
                s0_ = SeedGenerator::instance().get();
                s1_ = SeedGenerator::instance().get();
                s2_ = SeedGenerator::instance().get();
                s3_ = SeedGenerator::instance().get();
            } while (s0_ == 0 || s1_ == 0 || s2_ == 0 || s3_ == 0);
        }
    }
}
