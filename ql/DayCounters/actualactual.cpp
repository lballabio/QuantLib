
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

/*! \file actualactual.cpp
    \brief act/act day counter

    \fullpath
    Sources/DayCounters/%actualactual.cpp
*/

// $Id$

#include "ql/DayCounters/actualactual.hpp"

namespace QuantLib {

    namespace DayCounters {

        Time ActualActual::yearFraction(const Date& d1, const Date& d2,
          const Date& refPeriodStart, const Date& refPeriodEnd) const {

            QL_REQUIRE(d1<=d2, "invalid dates");

            QL_REQUIRE(refPeriodStart != Date() && refPeriodEnd != Date() &&
                refPeriodEnd > refPeriodStart && refPeriodEnd > d1,
                "Invalid reference period");

            
            
            // estimate roughly the length in months of a period
            int months = int(0.5+12*double(refPeriodEnd-refPeriodStart)/365);
            QL_REQUIRE(months != 0,
                "number of months does not divide 12 exactly");
            double period = double(months)/12.0;

            if (d2 <= refPeriodEnd) {
                // here refPeriodEnd is a future (maybe notional) payment date 
                if (d1 >= refPeriodStart)
                    // here refPeriodStart is the last (maybe notional) payment date.
                    // refPeriodStart <= d1 <= d2 <= refPeriodEnd
                    // [maybe the equality should be enforced, since
                    // refPeriodStart < d1 <= d2 < refPeriodEnd
                    // could give wrong results] ???
                    return period*double(dayCount(d1,d2)) /
                        dayCount(refPeriodStart,refPeriodEnd);
                else {
                    // here refPeriodStart is the next (maybe notional) payment date.
                    // and so refPeriodEnd is the next-next (maybe notional) payment date.
                    // d1 < refPeriodStart < refPeriodEnd AND d2 <= refPeriodEnd
                    // this case is long first coupon

                    // the the last notional payment date
                    Date previousRef = refPeriodStart.plusMonths(-months);
                    return yearFraction(
                                d1,refPeriodStart,previousRef,refPeriodStart) +
                           yearFraction(
                                refPeriodStart,d2,refPeriodStart,refPeriodEnd);
                }
            } else {
                // here refPeriodEnd is the last (maybe notional) payment date 
                // d1 < refPeriodEnd < d2 AND refPeriodStart < refPeriodEnd
                QL_REQUIRE(refPeriodStart<=d1,
                    "invalid dates: cannot be d1 < refPeriodStart < refPeriodEnd < d2");
                // now it is: refPeriodStart <= d1 < refPeriodEnd < d2

                // the part from d1 to refPeriodEnd
                double sum =
                    yearFraction(d1,refPeriodEnd,refPeriodStart,refPeriodEnd);

                // the part from refPeriodEnd to d2
                // count how many regular periods are in [refPeriodEnd, d2],
                // then add the remaining time
                int i=0;
                Date newRefStart, newRefEnd;
                do {
                    newRefStart = refPeriodEnd.plusMonths(months*i);
                    newRefEnd   = refPeriodEnd.plusMonths(months*(i+1));
                    if (d2 < newRefEnd) {
                        break;
                    } else {
                        sum += period;
                        i++;
                    }
                } while (true);
                sum += yearFraction(newRefStart,d2,newRefStart,newRefEnd);
                return sum;
            }
        }

    }

}
