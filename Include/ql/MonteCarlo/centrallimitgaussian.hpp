
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

/*! \file centrallimitgaussian.hpp
    \brief Central limit Gaussian random-number generator

    $Id$
*/

// $Source$
// $Log$
// Revision 1.8  2001/08/07 17:33:03  nando
// 1) StandardPathGenerator now is GaussianPathGenerator;
// 2) StandardMultiPathGenerator now is GaussianMultiPathGenerator;
// 3) PathMonteCarlo now is MonteCarloModel;
// 4) added ICGaussian, a Gaussian distribution that use
//    QuantLib::Math::InvCumulativeNormalDistribution to convert uniform
//    distribution extractions into gaussian distribution extractions;
// 5) added a few trailing underscore to private members
// 6) style enforced here and there ....
//
// Revision 1.7  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.6  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
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
            U basicGenerator_;
            mutable double gaussWeight_;
        };

        template <class U>
        CLGaussian<U>::CLGaussian(long seed):
            basicGenerator_(seed), gaussWeight_(0.0) {}

        template <class U>
        inline double CLGaussian<U>::next() const {

            double gaussPoint = -6.0;
            gaussWeight_ = 1.0;
            for(int i=1;i<=12;i++){
                gaussPoint += basicGenerator_.next();
                gaussWeight_ *= basicGenerator_.weight();
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
