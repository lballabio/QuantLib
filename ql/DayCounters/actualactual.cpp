
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
    \brief act/act day counters

    \fullpath
    ql/DayCounters/%actualactual.cpp
*/

// $Id$

#include "ql/DayCounters/actualactual.hpp"

namespace QuantLib {

    namespace DayCounters {

        Handle<DayCounter::DayCounterImpl> 
        ActualActual::implementation(ActualActual::Convention c) {
            switch (c) {
              case ISMA:
              case Bond:
                return Handle<DayCounterImpl>(new ActActISMAImpl);
              case ISDA:
              case Historical:
                return Handle<DayCounterImpl>(new ActActISDAImpl);
              case AFB:
              case Euro:
                return Handle<DayCounterImpl>(new ActActAFBImpl);
              default:
                throw Error("Unknown act/act convention");
            }
        }


        Time ActualActual::ActActISMAImpl::yearFraction(
          const Date& d1, const Date& d2,
          const Date& refPeriodStart, const Date& refPeriodEnd) const {
            QL_REQUIRE(d1<=d2, "invalid dates");
            QL_REQUIRE(refPeriodStart != Date() && refPeriodEnd != Date() &&
                refPeriodEnd > refPeriodStart && refPeriodEnd > d1,
                "Invalid reference period");

            if (d1 == d2)
                return 0.0;

            // estimate roughly the length in months of a period
            int months = int(0.5+12*double(refPeriodEnd-refPeriodStart)/365);
            QL_REQUIRE(months != 0,
                "number of months does not divide 12 exactly");
            double period = double(months)/12.0;

            if (d2 <= refPeriodEnd) {
                // here refPeriodEnd is a future (notional?) payment date 
                if (d1 >= refPeriodStart)
                    // here refPeriodStart is the last (maybe notional) 
                    // payment date.
                    // refPeriodStart <= d1 <= d2 <= refPeriodEnd
                    // [maybe the equality should be enforced, since
                    // refPeriodStart < d1 <= d2 < refPeriodEnd
                    // could give wrong results] ???
                    return period*double(dayCount(d1,d2)) /
                        dayCount(refPeriodStart,refPeriodEnd);
                else {
                    // here refPeriodStart is the next (maybe notional) 
                    // payment date and refPeriodEnd is the second next 
                    // (maybe notional) payment date.
                    // d1 < refPeriodStart < refPeriodEnd 
                    // AND d2 <= refPeriodEnd
                    // this case is long first coupon

                    // the last notional payment date
                    Date previousRef = refPeriodStart.plusMonths(-months);
                    return yearFraction(d1, refPeriodStart, previousRef, 
                                        refPeriodStart) +
                           yearFraction(refPeriodStart, d2, refPeriodStart, 
                                        refPeriodEnd);
                }
            } else {
                // here refPeriodEnd is the last (notional?) payment date 
                // d1 < refPeriodEnd < d2 AND refPeriodStart < refPeriodEnd
                QL_REQUIRE(refPeriodStart<=d1,
                    "invalid dates: "
                    "d1 < refPeriodStart < refPeriodEnd < d2");
                // now it is: refPeriodStart <= d1 < refPeriodEnd < d2

                // the part from d1 to refPeriodEnd
                double sum =
                    yearFraction(d1, refPeriodEnd, refPeriodStart, 
                                 refPeriodEnd);

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

        Time ActualActual::ActActISDAImpl::yearFraction(
          const Date& d1, const Date& d2, const Date&, const Date&) const {
            QL_REQUIRE(d2>=d1, "Invalid reference period");

            if (d1 == d2)
                return 0.0;

	        int y1 = d1.year(), y2 = d2.year();
	        double dib1 = (Date::isLeap(y1) ? 366.0 : 365.0),
		           dib2 = (Date::isLeap(y2) ? 366.0 : 365.0);

	        double sum = y2 - y1 - 1;
	        sum += dayCount(d1, Date(1,(Month)1,y1+1))/dib1;
	        sum += dayCount(Date(1,(Month)1,y2),d2)/dib2;
	        return sum;
        }

        Time ActualActual::ActActAFBImpl::yearFraction(
          const Date& d1, const Date& d2, const Date&, const Date&) const {
            QL_REQUIRE(d1<=d2, "Invalid reference period");

            if (d1 == d2)
                return 0.0;

            Date newD2=d2, temp=d2;
            Time sum = 0.0;
            while (temp > d1) {
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
