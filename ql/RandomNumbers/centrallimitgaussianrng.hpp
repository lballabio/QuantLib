
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file centrallimitgaussianrng.hpp
    \brief Central limit Gaussian random-number generator

    \fullpath
    ql/RandomNumbers/%centrallimitgaussianrng.hpp
*/

// $Id$

#ifndef quantlib_central_limit_gaussian_rng_h
#define quantlib_central_limit_gaussian_rng_h

#include <ql/MonteCarlo/sample.hpp>

namespace QuantLib {

    namespace RandomNumbers {

        //! Gaussian random number generator
        /*! It uses the well-known fact that the sum of 12 uniform deviate
            in (-.5,.5) is approximately a Gaussian deviate with average 0
            and standard deviation 1.
            The uniform deviate is supplied by U.

            Class U must implement the following interface:
            \code
                U::U(long seed);
                U::sample_type U::next() const;
            \endcode
        */
        template <class U>
        class CLGaussianRng {
          public:
            typedef MonteCarlo::Sample<double> sample_type;
            explicit CLGaussianRng(long seed=0);
            //! returns next sample from the Gaussian distribution
            sample_type next() const;
          private:
            U basicGenerator_;
        };

        template <class U>
        CLGaussianRng<U>::CLGaussianRng(long seed)
        : basicGenerator_(seed) {}

        template <class U>
        inline CLGaussianRng<U>::sample_type CLGaussianRng<U>::next() const {
            double gaussPoint = -6.0, gaussWeight = 1.0;
            for(int i=1;i<=12;i++){
                typename U::sample_type sample = basicGenerator_.next();
                gaussPoint  += sample.value;
                gaussWeight *= sample.weight;
            }
            return sample_type(gaussPoint,gaussWeight);
        }

    }

}


#endif
