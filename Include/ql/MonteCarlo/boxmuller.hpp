
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

/*! \file boxmuller.hpp

    \fullpath
    Include/ql/MonteCarlo/%boxmuller.hpp
    \brief Box-Muller Gaussian random-number generator

*/

// $Id$
// $Log$
// Revision 1.12  2001/08/31 15:23:45  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.11  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.10  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.9  2001/08/07 17:33:03  nando
// 1) StandardPathGenerator now is GaussianPathGenerator;
// 2) StandardMultiPathGenerator now is GaussianMultiPathGenerator;
// 3) PathMonteCarlo now is MonteCarloModel;
// 4) added ICGaussian, a Gaussian distribution that use
//    QuantLib::Math::InvCumulativeNormalDistribution to convert uniform
//    distribution extractions into gaussian distribution extractions;
// 5) added a few trailing underscore to private members
// 6) style enforced here and there ....
//
// Revision 1.8  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.7  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.6  2001/07/13 14:29:08  sigmud
// removed a few gcc compile warnings
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_box_muller_transformation_h
#define quantlib_box_muller_transformation_h

#include "ql/qldefines.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! Gaussian random number generator
        /*! It uses the well-known Box-Muller transformation to return a normal
            distributed Gaussian deviate with average 0.0 and standard deviation
            of 1.0, from a uniform deviate in (0,1) supplied by U.

            Class U must have, at least, the following interface:
            \code
                U::U(long seed);
                double U::next() const;
                double U::weight() const;
            \endcode
        */
        template <class U>
        class BoxMuller {
          public:
            BoxMuller(long seed=0);
            typedef double SampleType;
            //! returns next sample from the Gaussian distribution
            double next() const;
            //! returns the weight of the last extracted sample
            double weight() const;
          private:
            U basicGenerator_;
            mutable bool returnFirst_;
            mutable double firstValue_,secondValue_;
            mutable double firstWeight_,secondWeight_;
            mutable double weight_;
        };

        template <class U>
        BoxMuller<U>::BoxMuller(long seed):
            basicGenerator_(seed), returnFirst_(true), weight_(0.0){}

        template <class U>
        inline double BoxMuller<U>::next() const {
            if(returnFirst_) {
                double x1,x2,r,ratio;
                do {
                    x1 = basicGenerator_.next()*2-1;
                    firstWeight_ = basicGenerator_.weight();
                    x2 = basicGenerator_.next()*2-1;
                    secondWeight_ = basicGenerator_.weight();
                    r = x1*x1+x2*x2;
                } while(r>=1 || r==0);

                ratio = QL_SQRT(-2.0*QL_LOG(r)/r);
                firstValue_ = x1*ratio;
                secondValue_ = x2*ratio;
                weight_ = firstWeight_*secondWeight_;

                returnFirst_ = false;
                return firstValue_;
            } else {
                returnFirst_ = true;
                return secondValue_;
            }
        }

        template <class U>
        inline double BoxMuller<U>::weight() const {
            return weight_;
        }

    }

}

#endif
