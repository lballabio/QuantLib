
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

/*! \file dminus.hpp
    \brief \f$ D_{-} \f$ matricial representation

    $Id$
*/

// $Source$
// $Log$
// Revision 1.3  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef QUANTLIB_D_MINUS_H
#define QUANTLIB_D_MINUS_H

#include "ql/FiniteDifferences/tridiagonaloperator.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! \f$ D_{-} \f$ matricial representation
        /*! The differential operator \f$ D_{-} \f$ discretizes the
            first derivative with the first-order formula
            \f[ \frac{\partial u_{i}}{\partial x} =
                \frac{u_{i}-u_{i-1}}{h} \equiv D_{-} u_{i}
            \f]
        */
        class DMinus : public TridiagonalOperator {
          public:
            DMinus() {}
            DMinus(int gridPoints, double h);
        };

        // inline definitions

        inline DMinus::DMinus(int gridPoints, double h)
        : TridiagonalOperator(gridPoints) {
            setFirstRow(-1/h,1/h);                  // linear extrapolation
            setMidRows(-1/h,1/h,0.0);
            setLastRow(-1/h,1/h);
        }

    }

}


#endif
