
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

/*! \file bsmoperator.cpp
    \brief differential operator for Black-Scholes-Merton equation

    $Source$
    $Name$
    $Log$
    Revision 1.9  2001/04/09 14:13:33  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.8  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.7  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.6  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.5  2001/01/08 10:28:17  lballabio
    Moved Array to Math namespace

    Revision 1.4  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#include "ql/FiniteDifferences/bsmoperator.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        BSMOperator::BSMOperator(int size, double dx, double r, double q, double sigma)
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
