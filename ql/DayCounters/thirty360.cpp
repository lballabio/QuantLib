
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

/*! \file thirty360.cpp
    \brief 30/360 day counter

    \fullpath
    ql/DayCounters/%thirty360.cpp
*/

// $Id$

#include "ql/DayCounters/thirty360.hpp"

namespace QuantLib {

    namespace DayCounters {

        int Thirty360::dayCount(const Date& d1, const Date& d2) const {
            int dd2 = d2.dayOfMonth(), mm2 = d2.month();
            if (dd2 == 31 && d1.dayOfMonth() < 30){
                dd2 = 1;
                mm2++;
            }
            return 360*(d2.year()-d1.year()) + 30*(mm2-d1.month()-1) +
                QL_MAX(0,30-d1.dayOfMonth()) + QL_MIN(30,dd2);
        }

    }

}
