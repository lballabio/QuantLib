
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

/*! \file bsmoperator.cpp
    \fullpath Sources/FiniteDifferences/%bsmoperator.cpp
    \brief differential operator for Black-Scholes-Merton equation

*/

// $Id$
// $Log$
// Revision 1.16  2001/08/28 13:37:35  nando
// unsigned int instead of int
//
// Revision 1.15  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.14  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.13  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.12  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.11  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/FiniteDifferences/bsmoperator.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        BSMOperator::BSMOperator(unsigned int size, double dx, double r,
            double q, double sigma)
        : TridiagonalOperator(size) {
            double sigma2 = sigma*sigma;
            double nu = r-q-sigma2/2;
            double pd = -(sigma2/dx-nu)/(2*dx);
            double pu = -(sigma2/dx+nu)/(2*dx);
            double pm = sigma2/(dx*dx)+r;
            setMidRows(pd,pm,pu);
        }

    }

}
