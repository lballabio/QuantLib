
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

/*! \file thirty360italian.cpp
    \fullpath Sources/DayCounters/%thirty360italian.cpp
    \brief 30/360 italian day counter

    $Id$
*/

// $Source$
// $Log$
// Revision 1.19  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.18  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.17  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.16  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/DayCounters/thirty360italian.hpp"

namespace QuantLib {

    namespace DayCounters {

        int Thirty360Italian::dayCount(const Date& d1, const Date& d2) const {
            int gg1 = d1.dayOfMonth(), gg2 = d2.dayOfMonth();
            if (d1.month() == 2 && gg1 > 27)
                gg1 = 30;
            if (d2.month() == 2 && gg2 > 27)
                gg2 = 30;
            return 360*(d2.year()-d1.year()) + 30*(d2.month()-d1.month()-1) +
                QL_MAX(0,30-gg1) + QL_MIN(30,gg2);
        }

    }

}
