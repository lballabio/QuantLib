
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/DayCounters/actualactual.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    boost::shared_ptr<DayCounter::Impl>
    ActualActual::implementation(ActualActual::Convention c) {
        switch (c) {
          case ISMA:
          case Bond:
            return boost::shared_ptr<DayCounter::Impl>(new ISMA_Impl);
          case ISDA:
          case Historical:
            return boost::shared_ptr<DayCounter::Impl>(new ISDA_Impl);
          case AFB:
          case Euro:
            return boost::shared_ptr<DayCounter::Impl>(new AFB_Impl);
          default:
            QL_FAIL("Unknown act/act convention");
        }
    }


    Time ActualActual::ISMA_Impl::yearFraction(const Date& d1, const Date& d2,
                                               const Date& d3, 
                                               const Date& d4) const {

        if (d1 == d2)
            return 0.0;

        QL_REQUIRE(d1<=d2, 
                   "invalid dates: start = " + DateFormatter::toString(d1) +
                   ", end = " + DateFormatter::toString(d2));
        Date refPeriodStart = (d3 != Date() ? d3 : d1);
        Date refPeriodEnd = (d4 != Date() ? d4 : d2);
        QL_REQUIRE(refPeriodEnd > refPeriodStart && refPeriodEnd > d1,
                   "Invalid reference period."
                   "Date 1: " + DateFormatter::toString(d1) +
                   "  Date 2: " + DateFormatter::toString(d2) +
                   "  Reference period Start: " +
                   DateFormatter::toString(refPeriodStart) +
                   ",  Reference period end: " +
                   DateFormatter::toString(refPeriodEnd));

        // estimate roughly the length in months of a period
        int months = int(0.5+12*double(refPeriodEnd-refPeriodStart)/365);
        QL_REQUIRE(months != 0,
                   "number of months does not divide 12 exactly");
        double period = double(months)/12.0;

        if (d2 <= refPeriodEnd) {
            // here refPeriodEnd is a future (notional?) payment date
            if (d1 >= refPeriodStart) {
                // here refPeriodStart is the last (maybe notional)
                // payment date.
                // refPeriodStart <= d1 <= d2 <= refPeriodEnd
                // [maybe the equality should be enforced, since
                // refPeriodStart < d1 <= d2 < refPeriodEnd
                // could give wrong results] ???
                return period*double(dayCount(d1,d2)) /
                    dayCount(refPeriodStart,refPeriodEnd);
            } else {
                // here refPeriodStart is the next (maybe notional)
                // payment date and refPeriodEnd is the second next
                // (maybe notional) payment date.
                // d1 < refPeriodStart < refPeriodEnd
                // AND d2 <= refPeriodEnd
                // this case is long first coupon

                // the last notional payment date
                Date previousRef = refPeriodStart.plusMonths(-months);
                if (d2 > refPeriodStart)
                    return yearFraction(d1, refPeriodStart, previousRef,
                                        refPeriodStart) +
                        yearFraction(refPeriodStart, d2, refPeriodStart,
                                     refPeriodEnd);
                else
                    return yearFraction(d1,d2,previousRef,refPeriodStart);
            }
        } else {
            // here refPeriodEnd is the last (notional?) payment date
            // d1 < refPeriodEnd < d2 AND refPeriodStart < refPeriodEnd
            QL_REQUIRE(refPeriodStart<=d1,
                       "invalid dates: "
                       "d1 < refPeriodStart < refPeriodEnd < d2");
            // now it is: refPeriodStart <= d1 < refPeriodEnd < d2

            // the part from d1 to refPeriodEnd
            double sum = yearFraction(d1, refPeriodEnd, 
                                      refPeriodStart, refPeriodEnd);

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

    Time ActualActual::ISDA_Impl::yearFraction(const Date& d1, const Date& d2, 
                                               const Date&, 
                                               const Date&) const {
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

    Time ActualActual::AFB_Impl::yearFraction(const Date& d1, const Date& d2, 
                                              const Date&, const Date&) const {
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

        if (Date::isLeap(newD2.year())) {
            temp = Date(29, (Month)2, newD2.year());
            if (newD2>temp && d1<=temp)
                den += 1.0;
        } else if (Date::isLeap(d1.year())) {
            temp = Date(29, (Month)2, d1.year());
            if (newD2>temp && d1<=temp)
                den += 1.0;
        }

        return sum+dayCount(d1, newD2)/den;
    }

}
