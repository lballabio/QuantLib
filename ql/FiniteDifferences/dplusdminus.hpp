
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

/*! \file dplusdminus.hpp
    \brief \f$ D_{+}D_{-} \f$ matricial representation

    \fullpath
    ql/FiniteDifferences/%dplusdminus.hpp
*/

// $Id$

#ifndef quantlib_d_plus_d_minus_h
#define quantlib_d_plus_d_minus_h

#include "ql/FiniteDifferences/tridiagonaloperator.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! \f$ D_{+}D_{-} \f$ matricial representation
        /*! The differential operator \f$  D_{+}D_{-} \f$ discretizes the
            second derivative with the second-order formula
            \f[ \frac{\partial^2 u_{i}}{\partial x^2} \approx
                \frac{u_{i+1}-2u_{i}+u_{i-1}}{h^2} = D_{+}D_{-} u_{i}
            \f]
        */
        class DPlusDMinus : public TridiagonalOperator {
          public:
            DPlusDMinus(size_t gridPoints, double h);
        };

        // inline definitions

        inline DPlusDMinus::DPlusDMinus(size_t gridPoints, double h)
        : TridiagonalOperator(gridPoints) {
            setFirstRow(0.0,0.0);                   // linear extrapolation
            setMidRows(1/(h*h),-2/(h*h),1/(h*h));
            setLastRow(0.0,0.0);                    // linear extrapolation
        }

    }

}


#endif
