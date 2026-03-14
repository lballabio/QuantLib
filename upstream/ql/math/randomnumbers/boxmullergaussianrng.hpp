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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file boxmullergaussianrng.hpp
    \brief Box-Muller Gaussian random-number generator
*/

#ifndef quantlib_box_muller_gaussian_rng_h
#define quantlib_box_muller_gaussian_rng_h

#include <ql/methods/montecarlo/sample.hpp>

namespace QuantLib {

    //! Gaussian random number generator
    /*! It uses the well-known Box-Muller transformation to return a
        normal distributed Gaussian deviate with average 0.0 and
        standard deviation of 1.0, from a uniform deviate in (0,1)
        supplied by RNG.

        Class RNG must implement the following interface:
        \code
            RNG::sample_type RNG::next() const;
        \endcode
    */
    template <class RNG>
    class BoxMullerGaussianRng {
      public:
        typedef Sample<Real> sample_type;
        typedef RNG urng_type;
        explicit BoxMullerGaussianRng(const RNG& uniformGenerator);
        //! returns a sample from a Gaussian distribution
        sample_type next() const;
      private:
        RNG uniformGenerator_;
        mutable bool returnFirst_ = true;
        mutable Real firstValue_,secondValue_;
        mutable Real firstWeight_,secondWeight_;
        mutable Real weight_ = 0.0;
    };

    template <class RNG>
    BoxMullerGaussianRng<RNG>::BoxMullerGaussianRng(const RNG& uniformGenerator)
    : uniformGenerator_(uniformGenerator) {}

    template <class RNG>
    inline typename BoxMullerGaussianRng<RNG>::sample_type
    BoxMullerGaussianRng<RNG>::next() const {
        if (returnFirst_) {
            Real x1,x2,r,ratio;
            do {
                typename RNG::sample_type s1 = uniformGenerator_.next();
                x1 = s1.value*2.0-1.0;
                firstWeight_ = s1.weight;
                typename RNG::sample_type s2 = uniformGenerator_.next();
                x2 = s2.value*2.0-1.0;
                secondWeight_ = s2.weight;
                r = x1*x1+x2*x2;
            } while (r>=1.0 || r==0.0);

            ratio = std::sqrt(-2.0*std::log(r)/r);
            firstValue_ = x1*ratio;
            secondValue_ = x2*ratio;
            weight_ = firstWeight_*secondWeight_;

            returnFirst_ = false;
            return {firstValue_, weight_};
        } else {
            returnFirst_ = true;
            return {secondValue_, weight_};
        }
    }

}


#endif
