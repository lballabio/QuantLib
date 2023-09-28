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

    namespace {

        // NOTE: The following copyright notice applies to the
        // original C implementation https://prng.di.unimi.it/splitmix64.c
        // that has been used for this class.

        /*  Written in 2015 by Sebastiano Vigna (vigna@acm.org)

            To the extent possible under law, the author has dedicated all copyright
            and related and neighboring rights to this software to the public domain
            worldwide. This software is distributed without any warranty.

            See <http://creativecommons.org/publicdomain/zero/1.0/>.
        */
        class SplitMix64 {
          public:
            explicit SplitMix64(std::uint64_t x) : x_(x) {}
            std::uint64_t next() const {
                auto z = (x_ += 0x9e3779b97f4a7c15);
                z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
                z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
                return z ^ (z >> 31);
            };

          private:
            mutable std::uint64_t x_;
        };
    }

    Xoshiro256StarStarUniformRng::Xoshiro256StarStarUniformRng(std::uint64_t seed) {
        SplitMix64 splitMix64(seed != 0 ? seed : SeedGenerator::instance().get());
        s0_ = splitMix64.next();
        s1_ = splitMix64.next();
        s2_ = splitMix64.next();
        s3_ = splitMix64.next();
    }

    Xoshiro256StarStarUniformRng::Xoshiro256StarStarUniformRng(std::uint64_t s0,
                                                               std::uint64_t s1,
                                                               std::uint64_t s2,
                                                               std::uint64_t s3)
    : s0_(s0), s1_(s1), s2_(s2), s3_(s3) {}

}
