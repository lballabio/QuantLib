
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file centrallimitgaussian.hpp
    \brief Central limit Gaussian random-number generator

    $Id$
*/

// $Source$
// $Log$
// Revision 1.5  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_central_limit_gaussian_transformation_h
#define quantlib_central_limit_gaussian_transformation_h

#include "ql/qldefines.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! Gaussian random number generator
        /*! It uses the well-known fact that the sum of 12 uniform deviate
            in (-.5,.5) is approximately a Gaussian deviate with average 0.0
            and standard deviation 1.0,
            The uniform deviate is supplied by U.

            Class U should satisfies
            \code
                U::U(long seed);
                double U::next() const;
                double U::weight() const;
            \endcode
        */
        template <class U>
        class CLGaussian {
          public:
            typedef double SampleType;
            CLGaussian(long seed=0);
            //! returns next sample from the Gaussian distribution
            double next() const;
            //! returns the weight of the last extracted sample
            double weight() const;
          private:
            U basicGenerator;
            mutable double gaussWeight_;
        };

        template <class U>
        CLGaussian<U>::CLGaussian(long seed):
            basicGenerator(seed), gaussWeight_(0.0) {}

        template <class U>
        inline double CLGaussian<U>::next() const {

            double gaussPoint = -6.0;
            gaussWeight_ = 1.0;
            for(int i=1;i<=12;i++){
                gaussPoint += basicGenerator.next();
                gaussWeight_ *= basicGenerator.weight();
            }
            return gaussPoint;
        }

        template <class U>
        inline double CLGaussian<U>::weight() const {
            return gaussWeight_;
        }

    }

}

#endif
