/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2010 Kakhkhor Abdijalilov

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


// NOTE: The following copyright notice applies to
// the original C implementation that has been used for this class

/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: matumoto@math.keio.ac.jp
*/


#include <ql/math/randomnumbers/seedgenerator.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>

namespace QuantLib {

    namespace { // file scope

        namespace MersenneTwisterUniformRngPrivate {

            // shift size
            constexpr Size M = 397;
            // constant vector a
            constexpr unsigned long MATRIX_A = 0x9908b0dfUL;
            // most significant w-r bits
            constexpr unsigned long UPPER_MASK=0x80000000UL;
            // least significant r bits
            constexpr unsigned long LOWER_MASK=0x7fffffffUL;

            constexpr unsigned long mag01[2]={0x0UL, MATRIX_A};
        }

    } // namespace { // file scope

    MersenneTwisterUniformRng::MersenneTwisterUniformRng(unsigned long seed) {
        seedInitialization(seed);
    }

    void MersenneTwisterUniformRng::seedInitialization(unsigned long seed) {
        using namespace MersenneTwisterUniformRngPrivate;
        /* initializes mt with a seed */
        unsigned long s = (seed != 0 ? seed : SeedGenerator::instance().get());
        mt[0]= s & 0xffffffffUL;
        for (mti=1; mti<N; mti++) {
            mt[mti] =
                (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
            /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
            /* In the previous versions, MSBs of the seed affect   */
            /* only MSBs of the array mt[].                        */
            /* 2002/01/09 modified by Makoto Matsumoto             */
            mt[mti] &= 0xffffffffUL;
            /* for >32 bit machines */
        }
    }

    MersenneTwisterUniformRng::MersenneTwisterUniformRng(
                                      const std::vector<unsigned long>& seeds) {
        using namespace MersenneTwisterUniformRngPrivate;
        seedInitialization(19650218UL);
        Size i=1, j=0, k = (N>seeds.size() ? N : seeds.size());
        for (; k != 0U; k--) {
            mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
                + seeds[j] + j; /* non linear */
            mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
            i++; j++;
            if (i>=N) { mt[0] = mt[N-1]; i=1; }
            if (j>=seeds.size()) j=0;
        }
        for (k = N - 1; k != 0U; k--) {
            mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
                - i; /* non linear */
            mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
            i++;
            if (i>=N) { mt[0] = mt[N-1]; i=1; }
        }

        mt[0] = UPPER_MASK; /*MSB is 1; assuring non-zero initial array*/
    }

    void MersenneTwisterUniformRng::twist() const {
        using namespace MersenneTwisterUniformRngPrivate;
        /* mag01[x] = x * MATRIX_A  for x=0,1 */
        Size kk;
        unsigned long y;

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[(kk+M)-N] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    unsigned long MersenneTwisterUniformRng::nextInt32() const {
        using namespace MersenneTwisterUniformRngPrivate;
        if (mti==N)
            twist(); /* generate N words at a time */

        unsigned long y = mt[mti++];

        /* Tempering */
        y ^= (y >> 11);
        y ^= (y << 7) & 0x9d2c5680UL;
        y ^= (y << 15) & 0xefc60000UL;
        y ^= (y >> 18);
        return y;
    }
}
