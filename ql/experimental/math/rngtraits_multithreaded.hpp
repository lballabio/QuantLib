/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file rngtraits_multithreaded.hpp
    \brief random-number generation policies (multithreaded)
*/

#ifndef quantlib_rng_traits_multithreaded_hpp
#define quantlib_rng_traits_multithreaded_hpp

#include <ql/experimental/math/mersennetwister_multithreaded.hpp>
#include <ql/experimental/math/inversecumulativerng_multithreaded.hpp>
#include <ql/experimental/math/randomsequencegenerator_multithreaded.hpp>
#include <ql/math/randomnumbers/inversecumulativersg_multithreaded.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/distributions/poissondistribution.hpp>

namespace QuantLib {

// random number traits

template <class URNG_MT, class IC> struct GenericPseudoRandomMultiThreaded {
    // typedefs
    typedef URNG_MT urng_mt_type;
    typedef InverseCumulativeRngMultiThreaded<urng_mt_type, IC> rng_mt_type;
    typedef RandomSequenceGeneratorMultiThreaded<urng_mt_type, IC> ursg_mt_type;
    typedef InverseCumulativeRsgMultiThreaded<ursg_type,IC> rsg_type;
    // more traits
    enum { allowsErrorEstimate = 1 };
    enum { maxNumberOfThreads = URNG_MT::maxNumberOfThreads };
    // factory
    static rsg_mt_type make_sequence_generator(Size dimension,
                                               BugNatural seed) {
        ursg_mt_type g(dimension, seed);
        return (icInstance ? rsg_mt_type(g, *icInstance) : rsg_type(g));
    }
    // data
    static boost::shared_ptr<IC> icInstance;
};

// static member initialization
template <class URNG_MT, class IC>
boost::shared_ptr<IC> GenericPseudoRandomMultiThreaded<URNG_MT, IC>::icInstance;

//! default traits for pseudo-random number generator
typedef GenericPseudoRandomMultiThreaded<MersenneTwisterMultiThreaded,
                                         InverseCumulativeNormal> PseudoRandom;

//! traits for Poisson-distributed pseudo-random number generation
typedef GenericPseudoRandomMultiThreaded<
    MersenneTwisterMultiThreaded, InverseCumulativePoisson> PoissonPseudoRandom;


} // namespace QuantLib

#endif
