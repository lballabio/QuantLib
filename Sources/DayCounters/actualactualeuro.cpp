
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

/*! \file actualactualeuro.cpp
    \brief act/act euro day count convention

    \fullpath
    Sources/DayCounters/%actualactualeuro.cpp
*/

// $Id$

#include "ql/DayCounters/actualactualeuro.hpp"

namespace QuantLib {

    namespace DayCounters {

        Time ActualActualEuro::yearFraction(const Date& d1, const Date& d2,
            const Date& refPeriodStart, const Date& refPeriodEnd) const {

            QL_REQUIRE(d1<d2,
                "Invalid reference period");

            Date newD2=d2;
            Date temp=d2;
            Time sum = 0.0;
            while (temp>d1) {
                temp = newD2.plusYears(-1);
                if (temp.dayOfMonth()==28 && temp.month()==2
                    && Date::isLeap(temp.year())) {
                    temp.plusDays(1);
                }
                
                if (temp>=d1) {
                    sum += 1.0;
                    newD2 = temp;
                }
            }

            double den = 365.0;
            if ((Date::isLeap(newD2.year()) &&
                newD2>Date(29, (Month)2, newD2.year()))
                || (Date::isLeap(d1.year()) &&
                d1<=Date(29, (Month)2, d1.year()))) {

                den += 1.0;
            }

            return sum+dayCount(d1, newD2)/den;
        }
    }
}
