
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file inversecumulativegaussianrng.hpp
    \brief Inverse Cumulative Gaussian random-number generator

    \fullpath
    ql/RandomNumbers/%inversecumulativegaussianrng.hpp
*/

// $Id$

#ifndef quantlib_inversecumulative_gaussian_rng_h
#define quantlib_inversecumulative_gaussian_rng_h

#include <ql/Math/normaldistribution.hpp>
#include <ql/MonteCarlo/sample.hpp>

namespace QuantLib {

    namespace RandomNumbers {

        //! Inverse Cumulative Gaussian random number generator
        /*! It uses a uniform deviate in (0, 1) as the source of cumulative
            normal distribution values.
            Then an Inverse Cumulative Normal Distribution is used as it is
            approximately a Gaussian deviate with average 0.0 and standard
            deviation 1.0.

            The uniform deviate is supplied by U.

            Class U must implement the following interface:
            \code
                U::U(long seed);
                U::sample_type U::next() const;
            \endcode
        */
        template <class U>
        class ICGaussianRng {
          public:
            typedef MonteCarlo::Sample<double> sample_type;
            explicit ICGaussianRng(long seed=0);
            //! returns next sample from the Gaussian distribution
            sample_type next() const;
          private:
            U basicGenerator_;
            QuantLib::Math::InvCumulativeNormalDistribution ICND_;
        };

        template <class U>
        ICGaussianRng<U>::ICGaussianRng(long seed)
        : basicGenerator_(seed) {}

        template <class U>
        inline ICGaussianRng<U>::sample_type ICGaussianRng<U>::next() const {
            typename U::sample_type sample = basicGenerator_.next();
            return sample_type(ICND_(sample.value),sample.weight);
        }

    }

}

#endif
