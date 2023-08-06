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

#ifndef quantlib_xoshiro256starstar_prng_hpp
#define quantlib_xoshiro256starstar_prng_hpp

#include <ql/methods/montecarlo/sample.hpp>
#include <ql/types.hpp>

namespace QuantLib {
    class Xoshiro256StarStar {
      public:
        typedef Sample<Real> sample_type;
        /*! if the given seed is 0, a random seed will be chosen
            based on clock() */
        explicit Xoshiro256StarStar(unsigned long seed = 0);

        Xoshiro256StarStar(unsigned long s0, unsigned long s1, unsigned long s2, unsigned long s3);

        /*! returns a sample with weight 1.0 containing a random number
            in the (0.0, 1.0) interval  */
        sample_type next() const { return {nextReal(), 1.0}; }

        //! return a random number in the (0.0, 1.0)-interval
        Real nextReal() const { return (Real(nextInt64()) + 0.5) / max; }

        //! return a random integer in the [0,0xffffffffffffffffUL]-interval
        unsigned long long nextInt64() const {
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

      private:
        static uint64_t rotl(uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }
        void
        seedInitialization(unsigned long s0, unsigned long s1, unsigned long s2, unsigned long s3);

        const Real max = (Real)0xffffffffffffffffUL;
        mutable uint64_t s[4];
    };
}

#endif // quantlib_xoshiro256starstar_prng_hpp
