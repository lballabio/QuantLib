/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano

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

/*! \file randomizedlds.hpp
    \brief Randomized low-discrepancy sequence
*/

#ifndef quantlib_randomized_lds_hpp
#define quantlib_randomized_lds_hpp

#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/randomnumbers/randomsequencegenerator.hpp>
#include <utility>

namespace QuantLib {

    //! Randomized (random shift) low-discrepancy sequence
    /*! Random-shifts a uniform low-discrepancy sequence of dimension
        \f$ N \f$ by adding (modulo 1 for each coordinate) a pseudo-random
        uniform deviate in \f$ (0, 1)^N. \f$
        It is used for implementing Randomized Quasi Monte Carlo.

        The uniform low discrepancy sequence is supplied by LDS; the
        uniform pseudo-random sequence is supplied by PRS.

        Both class LDS and PRS must implement the following interface:
        \code
            LDS::sample_type LDS::nextSequence() const;
            Size LDS::dimension() const;
        \endcode

        \pre LDS and PRS must have the same dimension \f$ N \f$

        \warning Inverting LDS and PRS is possible, but it doesn't
                 make sense.

        \todo implement the other randomization algorithms

        \test correct initialization is tested.
    */
    template <class LDS,
              class PRS = RandomSequenceGenerator<MersenneTwisterUniformRng> >
    class RandomizedLDS {
      public:
        typedef Sample<std::vector<Real> > sample_type;
        RandomizedLDS(const LDS& ldsg, PRS prsg);
        RandomizedLDS(const LDS& ldsg);
        RandomizedLDS(Size dimensionality,
                      BigNatural ldsSeed = 0,
                      BigNatural prsSeed = 0);
        //! returns next sample using a given randomizing vector
        const sample_type& nextSequence() const;
        const sample_type& lastSequence() const {
            return x;
        }
        /*! update the randomizing vector and re-initialize
            the low discrepancy generator */
        void nextRandomizer() {
            randomizer_ = prsg_.nextSequence();
            ldsg_ = pristineldsg_;
        }
        Size dimension() const {return dimension_;}
      private:
        mutable LDS ldsg_, pristineldsg_; // mutable because nextSequence is const
        PRS prsg_;
        Size dimension_;
        mutable sample_type x, randomizer_;
    };

    template <class LDS, class PRS>
    RandomizedLDS<LDS, PRS>::RandomizedLDS(const LDS& ldsg, PRS prsg)
    : ldsg_(ldsg), pristineldsg_(ldsg), prsg_(std::move(prsg)), dimension_(ldsg_.dimension()),
      x(std::vector<Real>(dimension_), 1.0), randomizer_(std::vector<Real>(dimension_), 1.0) {

        QL_REQUIRE(prsg_.dimension()==dimension_,
                   "generator mismatch: "
                   << dimension_ << "-dim low discrepancy "
                   << "and " << prsg_.dimension() << "-dim pseudo random");

        randomizer_ = prsg_.nextSequence();
    }

    template <class LDS, class PRS>
    RandomizedLDS<LDS, PRS>::RandomizedLDS(const LDS& ldsg)
    : ldsg_(ldsg), pristineldsg_(ldsg),
      prsg_(ldsg_.dimension()), dimension_(ldsg_.dimension()),
      x(std::vector<Real> (dimension_), 1.0), randomizer_(std::vector<Real> (dimension_), 1.0) {

        randomizer_ = prsg_.nextSequence();

    }

    template <class LDS, class PRS>
    RandomizedLDS<LDS, PRS>::RandomizedLDS(Size dimensionality,
                                           BigNatural ldsSeed,
                                           BigNatural prsSeed)
    : ldsg_(dimensionality, ldsSeed), pristineldsg_(dimensionality, ldsSeed),
      prsg_(dimensionality, prsSeed), dimension_(dimensionality),
      x(std::vector<Real> (dimensionality), 1.0), randomizer_(std::vector<Real> (dimensionality), 1.0) {

        randomizer_ = prsg_.nextSequence();
    }

    template <class LDS, class PRS>
    inline const typename RandomizedLDS<LDS, PRS>::sample_type&
    RandomizedLDS<LDS, PRS>::nextSequence() const {
    typename LDS::sample_type sample =
        ldsg_.nextSequence();
    x.weight = randomizer_.weight * sample.weight;
    for (Size i = 0; i < dimension_; i++) {
        x.value[i] =  randomizer_.value[i] + sample.value[i];
        if (x.value[i]>1.0)
            x.value[i] -= 1.0;
    }
    return x;
    }

}


#endif


#ifndef id_50e10ef4980dffb83de1af0809a19f7b
#define id_50e10ef4980dffb83de1af0809a19f7b
inline bool test_50e10ef4980dffb83de1af0809a19f7b(const int* i) {
    return i != nullptr;
}
#endif
