
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

/*! \file dplus.hpp

    \fullpath
    Include/ql/FiniteDifferences/%dplus.hpp
    \brief \f$ D_{+} \f$ matricial representation

*/

// $Id$
// $Log$
// Revision 1.9  2001/08/31 15:23:45  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.8  2001/08/28 13:37:35  nando
// unsigned int instead of int
//
// Revision 1.7  2001/08/09 14:59:46  sigmud
// header modification
//
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

#ifndef QUANTLIB_D_PLUS_H
#define QUANTLIB_D_PLUS_H

#include "ql/FiniteDifferences/tridiagonaloperator.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! \f$ D_{+} \f$ matricial representation
        /*! The differential operator \f$ D_{+} \f$ discretizes the
            first derivative with the first-order formula
            \f[ \frac{\partial u_{i}}{\partial x} =
                \frac{u_{i+1}-u_{i}}{h} \equiv D_{+} u_{i}
            \f]
        */
        class DPlus : public TridiagonalOperator {
          public:
            DPlus() {}
            DPlus(unsigned int gridPoints, double h);
        };

        // inline definitions

        inline DPlus::DPlus(unsigned int gridPoints, double h)
        : TridiagonalOperator(gridPoints) {
            setFirstRow(-1/h,1/h);
            setMidRows(0.0,-1/h,1/h);
            setLastRow(-1/h,1/h);                   // linear extrapolation
        }

    }

}


#endif
