
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

/*! \file dplusdminus.hpp
    \brief \f$ D_{+}D_{-} \f$ matricial representation

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/01/04 11:57:07  lballabio
    Differential operators added

*/

#ifndef QUANTLIB_D_PLUS_D_MINUS_H
#define QUANTLIB_D_PLUS_D_MINUS_H

#include "qldefines.hpp"
#include "tridiagonaloperator.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! \f$ D_{+}D_{-} \f$ matricial representation
        /*! The differential operator \f$  D_{+}D_{-} \f$ discretizes the
            second derivative with the second-order formula
            \f[ \frac{\partial^2 u_{i}}{\partial x^2} = 
                \frac{u_{i+1}-2u_{i}+u_{i-1}}{h^2} \equiv D_{+}D_{-} u_{i}
            \f]
        */
        class DPlusDMinus : public TridiagonalOperator {
          public:
            DPlusDMinus() {}
            DPlusDMinus(int gridPoints, double h);
        };

        // inline definitions

        inline DPlusDMinus::DPlusDMinus(int gridPoints, double h)
        : TridiagonalOperator(gridPoints) {
            setFirstRow(0.0,0.0);                   // linear extrapolation
            setMidRows(1/(h*h),-2/(h*h),1/(h*h));
            setLastRow(0.0,0.0);                    // linear extrapolation
        }

    }

}


#endif
