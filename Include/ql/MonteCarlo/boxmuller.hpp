
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

/*! \file boxmuller.hpp
    \brief Box-Muller Gaussian random-number generator

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/05/23 19:30:27  nando
    smoothing #include xx.hpp

    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.5  2001/01/17 14:37:55  nando
    tabs removed

*/

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
            U basicGenerator;
            mutable double weight_;
            mutable bool returnFirst;
            mutable double firstValue,secondValue;
            mutable double firstWeight,secondWeight;
        };

        template <class U>
        BoxMuller<U>::BoxMuller(long seed):
            basicGenerator(seed), returnFirst(true), weight_(0.0){}

        template <class U>
        inline double BoxMuller<U>::next() const {
            if(returnFirst) {
                double x1,x2,r,ratio;
                do {
                    x1 = basicGenerator.next()*2-1;
                    firstWeight = basicGenerator.weight();
                    x2 = basicGenerator.next()*2-1;
                    secondWeight = basicGenerator.weight();
                    r = x1*x1+x2*x2;
                } while(r>=1 || r==0);

                ratio = QL_SQRT(-2.0*QL_LOG(r)/r);
                firstValue = x1*ratio;
                secondValue = x2*ratio;
                weight_ = firstWeight*secondWeight;

                returnFirst = false;
                return firstValue;
            } else {
                returnFirst = true;
                return secondValue;
            }
        }

        template <class U>
        inline double BoxMuller<U>::weight() const {
            return weight_;
        }

    }

}

#endif
