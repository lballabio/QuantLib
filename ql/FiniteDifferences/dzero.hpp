
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
    \brief \f$ D_{0} \f$ matricial representation

    \fullpath
    ql/FiniteDifferences/%dzero.hpp
*/

// $Id$

#ifndef quantlib_d_zero_h
#define quantlib_d_zero_h

#include <ql/FiniteDifferences/tridiagonaloperator.hpp>

namespace QuantLib {

    namespace FiniteDifferences {

        //! \f$ D_{0} \f$ matricial representation
        /*! The differential operator \f$ D_{0} \f$ discretizes the
            first derivative with the second-order formula
            \f[ \frac{\partial u_{i}}{\partial x} \approx
                \frac{u_{i+1}-u_{i-1}}{2h} = D_{0} u_{i}
            \f]
        */
        class DZero : public TridiagonalOperator {
          public:
            DZero(size_t gridPoints, double h);
        };

        // inline definitions

        inline DZero::DZero(size_t gridPoints, double h)
        : TridiagonalOperator(gridPoints) {
            setFirstRow(-1/h,1/h);                  // linear extrapolation
            setMidRows(-1/(2*h),0.0,1/(2*h));
            setLastRow(-1/h,1/h);                   // linear extrapolation
        }

    }

}


#endif
