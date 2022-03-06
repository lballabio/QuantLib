/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file zigguratrng.hpp
    \brief Ziggurat random-number generator
*/

#ifndef quantlib_ziggurat_generator_hpp
#define quantlib_ziggurat_generator_hpp

#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/randomnumbers/randomsequencegenerator.hpp>

namespace QuantLib {

    //! Ziggurat random-number generator
    /*! This generator returns standard normal variates using the
        Ziggurat method.  The underlying RNG is mt19937 (32 bit
        version). The algorithm is described in Marsaglia and Tsang
        (2000). "The Ziggurat Method for Generating Random
        Variables". Journal of Statistical Software 5 (8).  Note that
        step 2 from the above paper reuses the rightmost 8 bits of the
        random integer, which creates correlation between steps 1 and
        2.  This implementation was written from scratch, following
        Marsaglia and Tsang.  It avoids the correlation by using only
        the leftmost 24 bits of mt19937's output.

        Note that the GNU GSL implementation uses a different value
        for the right-most step. The GSL value is somewhat different
        from the one reported by Marsaglia and Tsang because GSL uses
        a different tail. This implementation uses the same right-most
        step as reported by Marsaglia and Tsang.  The generator was
        put through Marsaglia's Diehard battery of tests and didn't
        exibit any abnormal behavior.
    */
    class ZigguratRng {
      public:
        typedef Sample<Real> sample_type;
        explicit ZigguratRng(unsigned long seed = 0);
        sample_type next() const { return {nextGaussian(), 1.0}; }

      private:
        mutable MersenneTwisterUniformRng mt32_;
        Real nextGaussian() const;
    };

    // RNG traits for Ziggurat generator
    struct Ziggurat {
        // typedefs
        typedef ZigguratRng rng_type;
        typedef RandomSequenceGenerator<rng_type> rsg_type;
        // more traits
        enum { allowsErrorEstimate = 1 };
        // factory
        static rsg_type make_sequence_generator(Size dimension,
                                                BigNatural seed) {
            return rsg_type(dimension, seed);
        }
    };

}

#endif


#ifndef id_2f50b1a69b2f67bca9c52734b55eda62
#define id_2f50b1a69b2f67bca9c52734b55eda62
inline bool test_2f50b1a69b2f67bca9c52734b55eda62(int* i) { return i != 0; }
#endif
