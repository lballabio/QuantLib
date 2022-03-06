/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
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

/*! \file centrallimitgaussianrng.hpp
    \brief Central limit Gaussian random-number generator
*/

#ifndef quantlib_central_limit_gaussian_rng_h
#define quantlib_central_limit_gaussian_rng_h

#include <ql/methods/montecarlo/sample.hpp>

namespace QuantLib {

    //! Gaussian random number generator
    /*! It uses the well-known fact that the sum of 12 uniform deviate
        in (-.5,.5) is approximately a Gaussian deviate with average 0
        and standard deviation 1.  The uniform deviate is supplied by
        RNG.

        Class RNG must implement the following interface:
        \code
            RNG::sample_type RNG::next() const;
        \endcode
    */
    template <class RNG>
    class CLGaussianRng {
      public:
        typedef Sample<Real> sample_type;
        typedef RNG urng_type;
        explicit CLGaussianRng(const RNG& uniformGenerator);
        //! returns a sample from a Gaussian distribution
        sample_type next() const;
      private:
        RNG uniformGenerator_;
    };

    template <class RNG>
    CLGaussianRng<RNG>::CLGaussianRng(const RNG& uniformGenerator)
    : uniformGenerator_(uniformGenerator) {}

    template <class RNG>
    inline typename CLGaussianRng<RNG>::sample_type
    CLGaussianRng<RNG>::next() const {
        Real gaussPoint = -6.0, gaussWeight = 1.0;
        for (Integer i=1;i<=12;i++) {
            typename RNG::sample_type sample = uniformGenerator_.next();
            gaussPoint  += sample.value;
            gaussWeight *= sample.weight;
        }
        return {gaussPoint, gaussWeight};
    }

}


#endif


#ifndef id_94ffa95b4e39d70d7bf63e5725f478a6
#define id_94ffa95b4e39d70d7bf63e5725f478a6
inline bool test_94ffa95b4e39d70d7bf63e5725f478a6(int* i) { return i != 0; }
#endif
