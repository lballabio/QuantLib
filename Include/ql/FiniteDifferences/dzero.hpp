
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

/*! \file dzero.hpp
    \fullpath Include/ql/FiniteDifferences/%dzero.hpp
    \brief \f$ D_{0} \f$ matricial representation

    $Id$
*/

// $Source$
// $Log$
// Revision 1.6  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.5  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.4  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.3  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef QUANTLIB_D_ZERO_H
#define QUANTLIB_D_ZERO_H

#include "ql/FiniteDifferences/tridiagonaloperator.hpp"

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
