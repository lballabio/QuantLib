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

/*! \file mt19937uniformrng.hpp
    \brief Mersenne Twister uniform random number generator
*/

#ifndef quantlib_mersennetwister_uniform_rng_hpp
#define quantlib_mersennetwister_uniform_rng_hpp

#include <ql/methods/montecarlo/sample.hpp>
#include <vector>

namespace QuantLib {

    //! Uniform random number generator
    /*! Mersenne Twister random number generator of period 2**19937-1

        For more details see http://www.math.keio.ac.jp/matumoto/emt.html

        \test the correctness of the returned values is tested by
              checking them against known good results.
    */
    class MersenneTwisterUniformRng {
      public:
        typedef Sample<Real> sample_type;
        /*! if the given seed is 0, a random seed will be chosen
            based on clock() */
        explicit MersenneTwisterUniformRng(unsigned long seed = 0);
        explicit MersenneTwisterUniformRng(
                                     const std::vector<unsigned long>& seeds);
        /*! returns a sample with weight 1.0 containing a random number
            in the (0.0, 1.0) interval  */
        sample_type next() const { return {nextReal(), 1.0}; }
        //! return a random number in the (0.0, 1.0)-interval
        Real nextReal() const {
            return (Real(nextInt32()) + 0.5)/4294967296.0;
        }
        //! return a random integer in the [0,0xffffffff]-interval
        unsigned long nextInt32() const;
      private:
        static constexpr Size N = 624; // state size
        void seedInitialization(unsigned long seed);
        void twist() const;
        mutable unsigned long mt[N];
        mutable Size mti;
    };

}


#endif
