/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file inversecumulativerng.hpp
    \brief Inverse cumulative Gaussian random-number generator
*/

#ifndef quantlib_inversecumulative_rng_h
#define quantlib_inversecumulative_rng_h

#include <ql/methods/montecarlo/sample.hpp>

namespace QuantLib {

    //! Inverse cumulative random number generator
    /*! It uses a uniform deviate in (0, 1) as the source of cumulative
        distribution values.
        Then an inverse cumulative distribution is used to calculate
        the distribution deviate.

        The uniform deviate is supplied by RNG.

        Class RNG must implement the following interface:
        \code
            RNG::sample_type RNG::next() const;
        \endcode

        The inverse cumulative distribution is supplied by IC.

        Class IC must implement the following interface:
        \code
            IC::IC();
            Real IC::operator() const;
        \endcode
    */
    template <class RNG, class IC>
    class InverseCumulativeRng {
      public:
        typedef Sample<Real> sample_type;
        typedef RNG urng_type;
        explicit InverseCumulativeRng(const RNG& uniformGenerator);
        //! returns a sample from a Gaussian distribution
        sample_type next() const;
      private:
        RNG uniformGenerator_;
        IC ICND_;
    };

    template <class RNG, class IC>
    InverseCumulativeRng<RNG, IC>::InverseCumulativeRng(const RNG& ug)
    : uniformGenerator_(ug) {}

    template <class RNG, class IC>
    inline typename InverseCumulativeRng<RNG, IC>::sample_type
    InverseCumulativeRng<RNG, IC>::next() const {
        typename RNG::sample_type sample = uniformGenerator_.next();
        return sample_type(ICND_(sample.value),sample.weight);
    }

}


#endif


#ifndef id_1abdb5f0b048921a6b5851c51a67d087
#define id_1abdb5f0b048921a6b5851c51a67d087
inline bool test_1abdb5f0b048921a6b5851c51a67d087(int* i) { return i != 0; }
#endif
