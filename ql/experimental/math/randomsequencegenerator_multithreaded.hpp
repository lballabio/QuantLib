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

/*! \file randomsequencegenerator_multithreaded
    \brief Random sequence generator based on a pseudo-random number generator
   (multithreaded)
*/

#ifndef quantlib_random_sequence_generator_multithreaded_hpp
#define quantlib_random_sequence_generator_multithreaded_hpp

#include <ql/methods/montecarlo/sample.hpp>
#include <ql/errors.hpp>
#include <vector>

namespace QuantLib {

//! Random sequence generator based on a pseudo-random number generator
//! (multithreaded)
/*! Random sequence generator based on a pseudo-random number
    generator RNG_MT (multithreaded).

    Class RNG_MT must implement the following interface:
    \code
        RNG_MT::sample_type RNG::next(int threadId) const;
    \endcode
    If a client of this class wants to use the nextInt32Sequence method,
    class RNG must also implement
    \code
        unsigned long RNG_MT::nextInt32(int threadId) const;
    \endcode

    \warning do not use with low-discrepancy sequence generator.
*/

template <class RNG_MT> class RandomSequenceGeneratorMultiThreaded {
  public:
    typedef Sample<std::vector<Real> > sample_type;
    static const Size maxNumberOfThreads = RNG_MT::maxNumberOfThreads;
    RandomSequenceGeneratorMultiThreaded(Size dimensionality,
                                         const RNG_MT &rng_mt)
        : dimensionality_(dimensionality), rng_mt_(rng_mt),
          sequence_(std::vector<sample_type>(
              maxNumberOfThreads,
              sample_type(std::vector<Real>(dimensionality), 1.0))),
          int32Sequence_(std::vector<std::vector<BigNatural> >(
              RNG_MT::maxNumberOfThreads,
              std::vector<BigNatural>(dimensionality))) {
        QL_REQUIRE(dimensionality > 0, "dimensionality must be greater than 0");
    }

    RandomSequenceGeneratorMultiThreaded(Size dimensionality,
                                         BigNatural seed = 0)
        : dimensionality_(dimensionality), rng_mt_(seed),
          sequence_(std::vector<sample_type>(
              maxNumberOfThreads,
              sample_type(std::vector<Real>(dimensionality), 1.0))),
          int32Sequence_(std::vector<std::vector<BigNatural> >(
              RNG_MT::maxNumberOfThreads,
              std::vector<BigNatural>(dimensionality))) {}

    const sample_type &nextSequence(unsigned int threadId) const {
        QL_REQUIRE(threadId < RNG_MT::maxNumberOfThreads,
                   "thread id (" << threadId << ") out of bounds [0..."
                                 << RNG_MT::maxNumberOfThreads - 1 << "]");
        sequence_[threadId].weight = 1.0;
        for (Size i = 0; i < dimensionality_; i++) {
            typename RNG_MT::sample_type x(rng_mt_.next(threadId));
            sequence_[threadId].value[i] = x.value;
            sequence_[threadId].weight *= x.weight;
        }
        return sequence_[threadId];
    }

    std::vector<BigNatural> nextInt32Sequence(unsigned int threadId) const {
        QL_REQUIRE(threadId < RNG_MT::maxNumberOfThreads,
                   "thread id (" << threadId << ") out of bounds [0..."
                                 << RNG_MT::maxNumberOfThreads - 1 << "]");
        for (Size i = 0; i < dimensionality_; i++) {
            int32Sequence_[threadId][i] = rng_mt_[threadId].nextInt32(threadId);
        }
        return int32Sequence_[threadId];
    }

    const sample_type &lastSequence(unsigned int threadId) const {
        QL_REQUIRE(threadId < RNG_MT::maxNumberOfThreads,
                   "thread id (" << threadId << ") out of bounds [0..."
                                 << RNG_MT::maxNumberOfThreads - 1 << "]");
        return sequence_[threadId];
    }

    Size dimension() const { return dimensionality_; }

  private:
    Size dimensionality_;
    RNG_MT rng_mt_;
    mutable std::vector<sample_type> sequence_;
    mutable std::vector<std::vector<BigNatural> > int32Sequence_;
};

} // namespace QuantLib

#endif
