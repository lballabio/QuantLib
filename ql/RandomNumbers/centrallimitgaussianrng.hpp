
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

#include "ql/qldefines.hpp"

namespace QuantLib {

    namespace RandomNumbers {

        //! Gaussian random number generator
        /*! It uses the well-known fact that the sum of 12 uniform deviate
            in (-.5,.5) is approximately a Gaussian deviate with average 0
            and standard deviation 1.
            The uniform deviate is supplied by U.

            Class U should satisfies
            \code
                U::U(long seed);
                double U::next() const;
                double U::weight() const;
            \endcode
        */
        template <class U>
        class CLGaussianRng {
          public:
            typedef double sample_type;
            explicit CLGaussianRng(long seed=0);
            //! returns next sample from the Gaussian distribution
            double next() const;
            //! returns the weight of the last extracted sample
            double weight() const;
          private:
            U basicGenerator_;
            mutable double gaussWeight_;
        };

        template <class U>
        CLGaussianRng<U>::CLGaussianRng(long seed):
            basicGenerator_(seed), gaussWeight_(0.0) {}

        template <class U>
        inline double CLGaussianRng<U>::next() const {

            double gaussPoint = -6.0;
            gaussWeight_ = 1.0;
            for(int i=1;i<=12;i++){
                gaussPoint += basicGenerator_.next();
                gaussWeight_ *= basicGenerator_.weight();
            }
            return gaussPoint;
        }

        template <class U>
        inline double CLGaussianRng<U>::weight() const {
            return gaussWeight_;
        }

    }

}

#endif
