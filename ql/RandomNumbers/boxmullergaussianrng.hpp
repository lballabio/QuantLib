
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

/*! \file boxmullergaussianrng.hpp
    \brief Box-Muller Gaussian random-number generator

    \fullpath
    ql/RandomNumbers/%boxmullergaussianrng.hpp
*/

// $Id$

#ifndef quantlib_box_muller_gaussian_rng_h
#define quantlib_box_muller_gaussian_rng_h

#include "ql/MonteCarlo/sample.hpp"

namespace QuantLib {

    namespace RandomNumbers {

        //! Gaussian random number generator
        /*! It uses the well-known Box-Muller transformation to return a 
            normal distributed Gaussian deviate with average 0.0 and standard 
            deviation of 1.0, from a uniform deviate in (0,1) supplied by U.

            Class U must implement the following interface:
            \code
                U::U(long seed);
                U::sample_type U::next() const;
            \endcode
        */
        template <class U>
        class BoxMullerGaussianRng {
          public:
            typedef MonteCarlo::Sample<double> sample_type;
            explicit BoxMullerGaussianRng(long seed=0);
            //! returns next sample from the Gaussian distribution
            sample_type next() const;
          private:
            U basicGenerator_;
            mutable bool returnFirst_;
            mutable double firstValue_,secondValue_;
            mutable double firstWeight_,secondWeight_;
            mutable double weight_;
        };

        template <class U>
        BoxMullerGaussianRng<U>::BoxMullerGaussianRng(long seed):
            basicGenerator_(seed), returnFirst_(true), weight_(0.0){}

        template <class U>
        inline BoxMullerGaussianRng<U>::sample_type
        BoxMullerGaussianRng<U>::next() const {
            if(returnFirst_) {
                double x1,x2,r,ratio;
                do {
                    typename U::sample_type s1 = basicGenerator_.next();
                    x1 = s1.value*2.0-1.0;
                    firstWeight_ = s1.weight;
                    typename U::sample_type s2 = basicGenerator_.next();
                    x2 = s2.value*2.0-1.0;
                    secondWeight_ = s2.weight;
                    r = x1*x1+x2*x2;
                } while(r>=1.0 || r==0.0);

                ratio = QL_SQRT(-2.0*QL_LOG(r)/r);
                firstValue_ = x1*ratio;
                secondValue_ = x2*ratio;
                weight_ = firstWeight_*secondWeight_;

                returnFirst_ = false;
                return sample_type(firstValue_,weight_);
            } else {
                returnFirst_ = true;
                return sample_type(secondValue_,weight_);
            }
        }

    }

}

#endif
