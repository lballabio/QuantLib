
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
    \fullpath Sources/DayCounters/%actualactual.cpp
    \brief act/act day counter

*/

// $Id$
// $Log$
// Revision 1.30  2001/08/30 13:27:23  nando
// daycounters works with Python test suite
// step 2: no reference dates were they are not needed
//
// Revision 1.29  2001/08/30 12:32:24  nando
// daycounters works with Python test suite
// step 1
//
// Revision 1.28  2001/08/29 15:18:04  nando
// _DEBUG instead of QL_DEBUG to select which lib is to link under MS VC++
//

#include "ql/DayCounters/actualactual.hpp"

namespace QuantLib {

    namespace DayCounters {

        Time ActualActual::yearFraction(const Date& d1, const Date& d2,
          const Date& refPeriodStart, const Date& refPeriodEnd) const {
            QL_REQUIRE(refPeriodStart != Date() && refPeriodEnd != Date() &&
                refPeriodEnd > refPeriodStart && refPeriodEnd > d1,
                "Invalid reference period");
            // estimate roughly the length in months of a period
            int months = int(0.5+12*double(refPeriodEnd-refPeriodStart)/365);
            QL_REQUIRE(months != 0,
                "number of months does not divide 12 exactly");
            double period = double(months)/12.0;
            if (d2 <= refPeriodEnd) {
                if (d1 >= refPeriodStart)
                    return period*double(dayCount(d1,d2)) /
                        dayCount(refPeriodStart,refPeriodEnd);
                else {
                    Date previousRef = refPeriodStart.plusMonths(-months);
                    return yearFraction(
                                d1,refPeriodStart,previousRef,refPeriodStart) +
                           yearFraction(
                                refPeriodStart,d2,refPeriodStart,refPeriodEnd);
                }
            } else {
                double sum =
                    yearFraction(d1,refPeriodEnd,refPeriodStart,refPeriodEnd);
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
