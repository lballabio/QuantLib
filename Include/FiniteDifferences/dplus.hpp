
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.3  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file dplus.hpp
    \brief \f$ D_{+} \f$ matricial representation
*/

#ifndef QUANTLIB_D_PLUS_H
#define QUANTLIB_D_PLUS_H

#include "qldefines.hpp"
#include "FiniteDifferences/tridiagonaloperator.hpp"

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
            DPlus(int gridPoints, double h);
        };

        // inline definitions

        inline DPlus::DPlus(int gridPoints, double h)
        : TridiagonalOperator(gridPoints) {
            setFirstRow(-1/h,1/h);
            setMidRows(0.0,-1/h,1/h);
            setLastRow(-1/h,1/h);                   // linear extrapolation
        }

    }

}


#endif
