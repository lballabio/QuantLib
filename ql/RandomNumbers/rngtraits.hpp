
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file rngtraits.hpp
    \brief random-number generation policies
*/

#ifndef quantlib_rng_traits_h
#define quantlib_rng_traits_h

#include <ql/MonteCarlo/pathgenerator.hpp>
#include <ql/RandomNumbers/mt19937uniformrng.hpp>
#include <ql/RandomNumbers/inversecumgaussianrng.hpp>
#include <ql/RandomNumbers/randomsequencegenerator.hpp>
#include <ql/RandomNumbers/sobolrsg.hpp>
#include <ql/RandomNumbers/inversecumgaussianrsg.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    // random number traits

    template <class RNG, class IC>
    struct GenericPseudoRandom {
        // typedefs
        typedef RNG urng_type;
        typedef ICGaussianRng<urng_type,IC> rng_type;
        typedef RandomSequenceGenerator<urng_type> ursg_type;
        typedef ICGaussianRsg<ursg_type,IC> rsg_type;
        // more traits
        enum { allowsErrorEstimate = 1 };
        // factory
        static rsg_type make_sequence_generator(int dimension,
                                                unsigned long seed) {
            ursg_type g(dimension, seed);
            return rsg_type(g);
        }
    };

    // default choice
    typedef GenericPseudoRandom<MersenneTwisterUniformRng,
                                InverseCumulativeNormal> PseudoRandom;

    struct LowDiscrepancy {
        // typedefs
        typedef SobolRsg ursg_type;
        typedef InverseCumulativeNormal ic_type;
        typedef ICGaussianRsg<ursg_type,ic_type> rsg_type;
        // more traits
        enum { allowsErrorEstimate = 0 };
        // factory
        static rsg_type make_sequence_generator(int dimension,
                                                unsigned long seed) {
            ursg_type g(dimension, seed);
            return rsg_type(g);
        }
    };

}


#endif
