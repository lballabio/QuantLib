
/*
 Copyright (C) 2004 Ferdinando Ametrano

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

/*! \file randomizedlds.hpp
    \brief Randomized low discrepancy sequence
*/

#ifndef quantlib_randomized_lds_hpp
#define quantlib_randomized_lds_hpp

#include <ql/basicdataformatters.hpp>
#include <ql/Math/array.hpp>
#include <ql/MonteCarlo/sample.hpp>
#include <ql/RandomNumbers/randomsequencegenerator.hpp>

namespace QuantLib {

    //! Randomized (random shift) low discrepancy sequence
    /*! It random shifts a uniform low discrepancy sequence of dimension N
        adding (modulo 1 component-wise) a pseudo-random uniform deviate in
        (0, 1)^N
        It is used for implementing Randomized Quasi Monte Carlo.

        The uniform low discrepancy sequence is supplied by LDS, the uniform
        pseudo-random sequence is supplied by PRS.

        Both class LDS and PRS must implement the following interface:
        \code
            LDS::sample_type LDS::nextSequence() const;
            Size LDS::dimension() const;
        \endcode

        \pre LDS and PRS must have the same dimension N

        \warnig Inverting LDS and PRS is possible, but it doesn't make sense

        \todo implement the other randomization algorithms

    */
    template <class LDS,
              class PRS = RandomSequenceGenerator<MersenneTwisterUniformRng> >
    class RamdomizedLDS {
      public:
        typedef Sample<Array> sample_type;
        RamdomizedLDS(const LDS& ldsg,
                      const PRS& prsg);
        RamdomizedLDS(const LDS& ldsg);
        RamdomizedLDS(Size dimensionality,
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
        LDS ldsg_, pristineldsg_;
        PRS prsg_;
        Size dimension_;
        mutable sample_type x, randomizer_;
    };

    template <class LDS, class PRS>
    RamdomizedLDS<LDS, PRS>::RamdomizedLDS(const LDS& ldsg, const PRS& prsg)
    : ldsg_(ldsg), pristineldsg_(ldsg),
      prsg_(prsg), dimension_(ldsg_.dimension()),
      x(Array(dimension_), 1.0), randomizer_(Array(dimension_), 1.0) {

        QL_REQUIRE(prsg_.dimension()==dimension_,
            "generator mismatch: "
            + IntegerFormatter::toString(dimension_) +
            "-dim low discrepancy "
            "and "
            + IntegerFormatter::toString(prsg_.dimension()) +
            "-dim pseudo random")

        randomizer_ = prsg_.nextSequence();

    }

    template <class LDS, class PRS>
    RamdomizedLDS<LDS, PRS>::RamdomizedLDS(const LDS& ldsg)
    : ldsg_(ldsg), pristineldsg_(ldsg),
      prsg_(ldsg_.dimension()), dimension_(ldsg_.dimension()),
      x(Array(dimension_), 1.0), randomizer_(Array(dimension_), 1.0) {

        randomizer_ = prsg_.nextSequence();

    }

    template <class LDS, class PRS>
    RamdomizedLDS<LDS, PRS>::RamdomizedLDS(Size dimensionality,
                                           BigNatural ldsSeed,
                                           BigNatural prsSeed)
    : ldsg_(dimensionality, ldsSeed), pristineldsg_(dimensionality, ldsSeed),
      prsg_(dimensionality, prsSeed), dimension_(dimensionality),
      x(Array(dimensionality), 1.0), randomizer_(Array(dimensionality), 1.0) {

        randomizer_ = prsg_.nextSequence();
    }

    template <class LDS, class PRS>
    inline const typename RamdomizedLDS<LDS, PRS>::sample_type&
    RamdomizedLDS<LDS, PRS>::nextSequence() const {
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
