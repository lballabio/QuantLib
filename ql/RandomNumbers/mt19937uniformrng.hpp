
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file mt19937uniformrng.hpp
    \brief Mersenne Twister uniform random number generator

    \fullpath
    ql/RandomNumbers/%mt19937uniformrng.hpp
*/

// $Id$

#ifndef quantlib_mersennetwister_uniform_rng_h
#define quantlib_mersennetwister_uniform_rng_h

#include <ql/MonteCarlo/sample.hpp>
#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    namespace RandomNumbers {

        //! Uniform random number generator
        /*! Mersenne Twister random number generatoro of period 2**19937-1

            For more details see http://www.math.keio.ac.jp/matumoto/emt.html
        */
        class MersenneTwisterUniformRng {
          public:
            typedef MonteCarlo::Sample<double> sample_type;
            /*! if the given seed is 0, a random seed will be chosen
                based on clock() */
            explicit MersenneTwisterUniformRng(unsigned long seed = 0);
            explicit MersenneTwisterUniformRng(
                const std::vector<unsigned long>& seeds);
            /*! returns a sample with weight 1.0 containing a random number
                uniformly chosen from (0.0,1.0) */
            sample_type next() const;
          private:
            void seedInitialization(unsigned long seed);
            mutable std::vector<unsigned long> mt;
            static const Size N, M;
            static const unsigned long MATRIX_A, UPPER_MASK, LOWER_MASK;
            static int mti;
        };

        inline MersenneTwisterUniformRng::MersenneTwisterUniformRng(
            unsigned long seed)
        : mt(N) {
            seedInitialization(seed);
        }

        inline void MersenneTwisterUniformRng::seedInitialization(
            unsigned long seed) {
            /* initializes mt with a seed */
            unsigned long s = (seed != 0 ? seed : long(QL_TIME(0)));
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

        inline MersenneTwisterUniformRng::MersenneTwisterUniformRng(
            const std::vector<unsigned long>& seeds)
        : mt(N) {
            seedInitialization(19650218UL);
            int i=1, j=0, k = (N>seeds.size() ? N : seeds.size());
            for (; k; k--) {
                mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
                  + seeds[j] + j; /* non linear */
                mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
                i++; j++;
                if (i>=N) { mt[0] = mt[N-1]; i=1; }
                if (j>=seeds.size()) j=0;
            }
            for (k=N-1; k; k--) {
                mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
                  - i; /* non linear */
                mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
                i++;
                if (i>=N) { mt[0] = mt[N-1]; i=1; }
            }

            mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
        }

        
        /* generates a random number on (0,1)-real-interval */
        inline MersenneTwisterUniformRng::sample_type
        MersenneTwisterUniformRng::next() const {

            unsigned long y;
            static unsigned long mag01[2]={0x0UL, MATRIX_A};
            /* mag01[x] = x * MATRIX_A  for x=0,1 */

            if (mti >= N) { /* generate N words at one time */
                int kk;

                for (kk=0;kk<N-M;kk++) {
                    y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                    mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
                }
                for (;kk<N-1;kk++) {
                    y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                    mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
                }
                y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
                mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

                mti = 0;
            }

            y = mt[mti++];

            /* Tempering */
            y ^= (y >> 11);
            y ^= (y << 7) & 0x9d2c5680UL;
            y ^= (y << 15) & 0xefc60000UL;
            y ^= (y >> 18);
            double result = (double(y)+ 0.5)/4294967296.0;
            /* divided by 2^32 */
            return sample_type(result,1.0);
        }

    }

}


#endif
