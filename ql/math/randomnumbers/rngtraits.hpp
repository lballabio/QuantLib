/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2004 Walter Penschke

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

/*! \file rngtraits.hpp
    \brief random-number generation policies
*/

#ifndef quantlib_rng_traits_hpp
#define quantlib_rng_traits_hpp

#include <ql/methods/montecarlo/pathgenerator.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/randomnumbers/inversecumulativerng.hpp>
#include <ql/math/randomnumbers/randomsequencegenerator.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <ql/math/randomnumbers/inversecumulativersg.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/distributions/poissondistribution.hpp>

namespace QuantLib {

    // random number traits

    template <class URNG, class IC>
    struct GenericPseudoRandom {
        // typedefs
        typedef URNG urng_type;
        typedef InverseCumulativeRng<urng_type,IC> rng_type;
        typedef RandomSequenceGenerator<urng_type> ursg_type;
        typedef InverseCumulativeRsg<ursg_type,IC> rsg_type;
        // more traits
        enum { allowsErrorEstimate = 1 };
        // factory
        static rsg_type make_sequence_generator(Size dimension,
                                                BigNatural seed) {
            ursg_type g(dimension, seed);
            return (icInstance ? rsg_type(g, *icInstance) : rsg_type(g));
        }
        // data
        static ext::shared_ptr<IC> icInstance;
    };

    // static member initialization
    template<class URNG, class IC>
    ext::shared_ptr<IC> GenericPseudoRandom<URNG, IC>::icInstance;


    //! default traits for pseudo-random number generation
    /*! \test a sequence generator is generated and tested by comparing
              samples against known good values.
    */
    typedef GenericPseudoRandom<MersenneTwisterUniformRng,
                                InverseCumulativeNormal> PseudoRandom;

    //! traits for Poisson-distributed pseudo-random number generation
    /*! \test sequence generators are generated and tested by comparing
              samples against known good values.
    */
    typedef GenericPseudoRandom<MersenneTwisterUniformRng,
                                InverseCumulativePoisson> PoissonPseudoRandom;


    template <class URSG, class IC>
    struct GenericLowDiscrepancy {
        // typedefs
        typedef URSG ursg_type;
        typedef InverseCumulativeRsg<ursg_type,IC> rsg_type;
        // more traits
        enum { allowsErrorEstimate = 0 };
        // factory
        static rsg_type make_sequence_generator(Size dimension,
                                                BigNatural seed) {
            ursg_type g(dimension, seed);
            return (icInstance ? rsg_type(g, *icInstance) : rsg_type(g));
        }
        // data
        static ext::shared_ptr<IC> icInstance;
    };

    // static member initialization
    template<class URSG, class IC>
    ext::shared_ptr<IC> GenericLowDiscrepancy<URSG, IC>::icInstance;


    //! default traits for low-discrepancy sequence generation
    typedef GenericLowDiscrepancy<SobolRsg,
                                  InverseCumulativeNormal> LowDiscrepancy;

}


#endif


#ifndef id_341ceb67eafdb026c0554ad6f87b7ab0
#define id_341ceb67eafdb026c0554ad6f87b7ab0
inline bool test_341ceb67eafdb026c0554ad6f87b7ab0(const int* i) {
    return i != nullptr;
}
#endif
