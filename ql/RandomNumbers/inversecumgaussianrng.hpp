
/*
 Copyright (C) 2003 Ferdinando Ametrano
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

/*! \file inversecumgaussianrng.hpp
    \brief Inverse cumulative Gaussian random-number generator
*/

#ifndef quantlib_inversecumulative_gaussian_rng_h
#define quantlib_inversecumulative_gaussian_rng_h

#include <ql/MonteCarlo/sample.hpp>

namespace QuantLib {

    //! Inverse cumulative Gaussian random number generator
    /*! It uses a uniform deviate in (0, 1) as the source of cumulative
        normal distribution values.
        Then an inverse cumulative normal distribution is used as it is
        approximately a Gaussian deviate with average 0.0 and standard
        deviation 1.0.

        The uniform deviate is supplied by RNG.

        Class RNG must implement the following interface:
        \code
            RNG::sample_type RNG::next() const;
        \endcode

        The inverse cumulative normal distribution is supplied by I.

        Class I must implement the following interface:
        \code
            I::I();
            Real I::operator() const;
        \endcode
    */
    template <class RNG, class I>
    class ICGaussianRng {
      public:
        typedef Sample<Real> sample_type;
        typedef RNG urng_type;
        explicit ICGaussianRng(const RNG& uniformGenerator);
        //! returns a sample from a Gaussian distribution
        sample_type next() const;
      private:
        RNG uniformGenerator_;
        I ICND_;
    };

    template <class RNG, class I>
    ICGaussianRng<RNG, I>::ICGaussianRng(const RNG& uniformGenerator)
    : uniformGenerator_(uniformGenerator) {}

    template <class RNG, class I>
    inline typename ICGaussianRng<RNG, I>::sample_type
    ICGaussianRng<RNG, I>::next() const {
        typename RNG::sample_type sample = uniformGenerator_.next();
        return sample_type(ICND_(sample.value),sample.weight);
    }

}


#endif
