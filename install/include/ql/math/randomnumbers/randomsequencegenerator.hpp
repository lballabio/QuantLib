/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file randomsequencegenerator.hpp
    \brief Random sequence generator based on a pseudo-random number generator
*/

#ifndef quantlib_random_sequence_generator_h
#define quantlib_random_sequence_generator_h

#include <ql/methods/montecarlo/sample.hpp>
#include <ql/errors.hpp>
#include <vector>

namespace QuantLib {

    //! Random sequence generator based on a pseudo-random number generator
    /*! Random sequence generator based on a pseudo-random number
        generator RNG.

        Class RNG must implement the following interface:
        \code
            RNG::sample_type RNG::next() const;
        \endcode
        If a client of this class wants to use the nextInt32Sequence method,
        class RNG must also implement
        \code
            unsigned long RNG::nextInt32() const;
        \endcode

        \warning do not use with low-discrepancy sequence generator.
    */
    template<class RNG>
    class RandomSequenceGenerator {
      public:
        typedef Sample<std::vector<Real> > sample_type;
        RandomSequenceGenerator(Size dimensionality,
                                const RNG& rng)
        : dimensionality_(dimensionality), rng_(rng),
          sequence_(std::vector<Real> (dimensionality), 1.0),
          int32Sequence_(dimensionality) {
          QL_REQUIRE(dimensionality>0, 
                     "dimensionality must be greater than 0");
        }

        explicit RandomSequenceGenerator(Size dimensionality,
                                         BigNatural seed = 0)
        : dimensionality_(dimensionality), rng_(seed),
          sequence_(std::vector<Real> (dimensionality), 1.0),
          int32Sequence_(dimensionality) {}

        const sample_type& nextSequence() const {
            sequence_.weight = 1.0;
            for (Size i=0; i<dimensionality_; i++) {
                typename RNG::sample_type x(rng_.next());
                sequence_.value[i] = x.value;
                sequence_.weight  *= x.weight;
            }
            return sequence_;
        }
        std::vector<BigNatural> nextInt32Sequence() const {
            for (Size i=0; i<dimensionality_; i++) {
                int32Sequence_[i] = rng_.nextInt32();
            }
            return int32Sequence_;
        }
        const sample_type& lastSequence() const {
            return sequence_;
        }
        Size dimension() const {return dimensionality_;}
      private:
        Size dimensionality_;
        RNG rng_;
        mutable sample_type sequence_;
        mutable std::vector<BigNatural> int32Sequence_;
    };

}


#endif
