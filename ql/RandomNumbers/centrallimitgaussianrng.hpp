
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file centrallimitgaussianrng.hpp
    \brief Central limit Gaussian random-number generator
*/

#ifndef quantlib_central_limit_gaussian_rng_h
#define quantlib_central_limit_gaussian_rng_h

#include <ql/MonteCarlo/sample.hpp>

namespace QuantLib {

    namespace RandomNumbers {

        //! Gaussian random number generator
        /*! It uses the well-known fact that the sum of 12 uniform deviate
            in (-.5,.5) is approximately a Gaussian deviate with average 0
            and standard deviation 1.
            The uniform deviate is supplied by RNG.

            Class RNG must implement the following interface:
            \code
                RNG::sample_type RNG::next() const;
            \endcode
        */
        template <class RNG>
        class CLGaussianRng {
          public:
            typedef MonteCarlo::Sample<double> sample_type;
            explicit CLGaussianRng(const RNG& uniformGenerator);
            /*! \deprecated initialize with a random number
                            generator instead.
            */
            explicit CLGaussianRng(long seed = 0);
            //! returns next sample from the Gaussian distribution
            sample_type next() const;
          private:
            RNG uniformGenerator_;
        };

        template <class RNG>
        CLGaussianRng<RNG>::CLGaussianRng(const RNG& uniformGenerator)
        : uniformGenerator_(uniformGenerator) {}

        template <class RNG>
        CLGaussianRng<RNG>::CLGaussianRng(long seed)
        : uniformGenerator_(seed) {}

        template <class RNG>
        inline typename CLGaussianRng<RNG>::sample_type
        CLGaussianRng<RNG>::next() const {
            double gaussPoint = -6.0, gaussWeight = 1.0;
            for(int i=1;i<=12;i++){
                typename RNG::sample_type sample = uniformGenerator_.next();
                gaussPoint  += sample.value;
                gaussWeight *= sample.weight;
            }
            return sample_type(gaussPoint,gaussWeight);
        }

    }

}


#endif
