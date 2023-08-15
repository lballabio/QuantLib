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

#ifndef quantlib_xoshiro256starstar_uniform_rng_hpp
#define quantlib_xoshiro256starstar_uniform_rng_hpp

#include <ql/methods/montecarlo/sample.hpp>
#include <ql/types.hpp>

namespace QuantLib {
    class Xoshiro256StarStarUniformRng {
      public:
        typedef Sample<Real> sample_type;
        /*! if the given seed is 0, a random seed will be chosen
            based on clock() */
        explicit Xoshiro256StarStarUniformRng(unsigned long long seed = 0);

        Xoshiro256StarStarUniformRng(unsigned long long s0,
                                     unsigned long long s1,
                                     unsigned long long s2,
                                     unsigned long long s3);

        /*! returns a sample with weight 1.0 containing a random number
            in the (0.0, 1.0) interval  */
        sample_type next() const { return {nextReal(), 1.0}; }

        //! return a random number in the (0.0, 1.0)-interval
        Real nextReal() const { return (Real(nextInt64() >> 11) + 0.5) * (1.0 / (1ULL << 53)); }

        //! return a random integer in the [0,0xffffffffffffffffULL]-interval
        unsigned long long nextInt64() const {
            const unsigned long long result = rotl(s1_ * 5, 7) * 9;

            const unsigned long long t = s1_ << 17;

            s2_ ^= s0_;
            s3_ ^= s1_;
            s1_ ^= s2_;
            s0_ ^= s3_;

            s2_ ^= t;

            s3_ = rotl(s3_, 45);

            return result;
        }

      private:
        static unsigned long long rotl(unsigned long long x, int k) {
            return (x << k) | (x >> (64 - k));
        }
        void seedInitialization(unsigned long long s0,
                                unsigned long long s1,
                                unsigned long long s2,
                                unsigned long long s3) const;

        mutable unsigned long long s0_, s1_, s2_, s3_;
    };
}

#endif // quantlib_xoshiro256starstar_uniform_rng_hpp
