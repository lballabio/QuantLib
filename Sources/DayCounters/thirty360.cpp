
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

/*! \file thirty360.cpp
    \brief 30/360 day counter

    $Source$
    $Name$
    $Log$
    Revision 1.14  2001/04/09 14:13:33  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.13  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.12  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.11  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.10  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.9  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#include "ql/DayCounters/thirty360.hpp"

namespace QuantLib {

    namespace DayCounters {

        int Thirty360::dayCount(const Date& d1, const Date& d2) const {
            int dd2 = d2.dayOfMonth(), mm2 = d2.month();
            if (dd2 == 31 && d1.dayOfMonth() < 30){
                dd2 = 1;
                mm2++;
            }
            return 360*(d2.year()-d1.year()) + 30*(mm2-d1.month()-1) + QL_MAX(0,30-d1.dayOfMonth()) + QL_MIN(30,dd2);
        }

    }

}
