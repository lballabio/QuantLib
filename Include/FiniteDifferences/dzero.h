
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file dzero.h
    \brief \f$ D_{0} \f$ matricial representation

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/04 11:57:07  lballabio
    Differential operators added

*/

#ifndef QUANTLIB_D_ZERO_H
#define QUANTLIB_D_ZERO_H

#include "qldefines.h"
#include "tridiagonaloperator.h"

namespace QuantLib {

    namespace FiniteDifferences {

        //! \f$ D_{0} \f$ matricial representation
        /*! The differential operator \f$ D_{0} \f$ discretizes the
            first derivative with the second-order formula
            \f[ \frac{\partial u_{i}}{\partial x} = 
                \frac{u_{i+1}-u_{i-1}}{2h} \equiv D_{0} u_{i}
            \f]
        */
        class DZero : public TridiagonalOperator {
          public:
            DZero() {}
            DZero(int gridPoints, double h);
        };

        // inline definitions

        inline DZero::DZero(int gridPoints, double h)
        : TridiagonalOperator(gridPoints) {
            setFirstRow(-1/h,1/h);                  // linear extrapolation
            setMidRows(-1/(2*h),0.0,1/(2*h));
            setLastRow(-1/h,1/h);                   // linear extrapolation
        }

    }

}


#endif
