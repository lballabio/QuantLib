
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

/*! \file actualactualhistorical.cpp
    \brief act/act historical day count convention

    \fullpath
    Sources/DayCounters/%actualactualhistorical.cpp
*/

// $Id$

#include "ql/DayCounters/actualactualhistorical.hpp"

namespace QuantLib {

    namespace DayCounters {

        Time ActualActualHistorical::yearFraction(const Date& d1,
            const Date& d2, const Date& refPeriodStart,
            const Date& refPeriodEnd) const {
            QL_REQUIRE(d2>=d1,
                "Invalid reference period");

            if (d2.year()==d1.year()) {
                if (d2.isLeap(d2.year()))
                    return dayCount(d1,d2)/366;
                else
                    return dayCount(d1,d2)/365;
            } else {
                double sum = 0.0;
                if (d2.isLeap(d2.year()))
                    sum += dayCount(Date(1, (Month)1, d2.year()), d2)/366;
                else
                    sum += dayCount(Date(1, (Month)1, d2.year()), d2)/365;
                if (d1.isLeap(d1.year()))
                    sum += dayCount(d1, Date(1, (Month)1, d1.year()+1))/366;
                else
                    sum += dayCount(d1, Date(1, (Month)1, d1.year()+1))/365;
                return sum + (d2.year() - d1.year() - 1);
            }

        } // Time ActualActualHistorical::yearFraction

    } // namespace DayCounters

} // namespace QuantLib
