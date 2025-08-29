/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Ralf Konrad Eckel

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

// NOTE: The following copyright notice applies to the
// original C implementation https://prng.di.unimi.it/xoshiro256starstar.c
// that has been used for this class.

/*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

/*! \file xoshiro256starstaruniformrng.hpp
    \brief xoshiro256** uniform random number generator
*/

#ifndef quantlib_xoshiro256starstar_uniform_rng_hpp
#define quantlib_xoshiro256starstar_uniform_rng_hpp

#include <ql/methods/montecarlo/sample.hpp>
#include <ql/types.hpp>
#include <cstdint>

namespace QuantLib {

    //! Uniform random number generator
    /*! xoshiro256** random number generator of period 2**256-1

        For more details see
            https://prng.di.unimi.it/
        and its reference implementation
            https://prng.di.unimi.it/xoshiro256starstar.c

        \test the correctness of the returned values is tested by checking them
               against the reference implementation in c.
    */
    class Xoshiro256StarStarUniformRng {
      public:
        typedef Sample<Real> sample_type;

        /*! If the given seed is 0, a random seed will be chosen based on clock(). */
        explicit Xoshiro256StarStarUniformRng(std::uint64_t seed = 0);

        /*! Make sure that s0, s1, s2 and s3 are chosen randomly.
         * Otherwise, the results of the first random numbers might not be well distributed.
         * Especially s0 = s1 = s2 = s3 = 0 does not work and will always return 0. */
        Xoshiro256StarStarUniformRng(std::uint64_t s0, std::uint64_t s1, std::uint64_t s2, std::uint64_t s3);

        /*! returns a sample with weight 1.0 containing a random number
         * in the (0.0, 1.0) interval */
        sample_type next() const { return {nextReal(), 1.0}; }

        //! return a random number in the (0.0, 1.0)-interval
        Real nextReal() const { return (Real(nextInt64() >> 11) + 0.5) * (1.0 / Real(1ULL << 53)); }

        //! return a random integer in the [0,0xffffffffffffffffULL]-interval
        std::uint64_t nextInt64() const {
            const auto result = rotl(s1_ * 5, 7) * 9;

            const auto t = s1_ << 17;

            s2_ ^= s0_;
            s3_ ^= s1_;
            s1_ ^= s2_;
            s0_ ^= s3_;

            s2_ ^= t;

            s3_ = rotl(s3_, 45);

            return result;
        }

      private:
        static std::uint64_t rotl(std::uint64_t x, std::int32_t k) { return (x << k) | (x >> (64 - k)); }
        mutable std::uint64_t s0_, s1_, s2_, s3_;
    };

}

#endif
