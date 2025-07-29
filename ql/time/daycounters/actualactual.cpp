/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/time/daycounters/actualactual.hpp>
#include <algorithm>
#include <cmath>

namespace QuantLib {

    namespace {

        // the template argument works around passing a protected type

        template <class T>
        Integer findCouponsPerYear(const T& impl,
                                   Date refStart, Date refEnd) {
            // This will only work for day counts longer than 15 days.
            auto months = (Integer)std::lround(12 * Real(impl.dayCount(refStart, refEnd)) / 365.0);
            return (Integer)std::lround(12.0 / Real(months));
        }

        /* An ISMA day counter either needs a schedule or to have
           been explicitly passed a reference period. This usage
           leads to inaccurate year fractions.
        */
        template <class T>
        Time yearFractionGuess(const T& impl,
                               const Date& start, const Date& end) {
            // asymptotically correct.
            return Real(impl.dayCount(start, end)) / 365.0;
        }

        std::vector<Date> getListOfPeriodDatesIncludingQuasiPayments(
                                                   const Schedule& schedule) {
            // Process the schedule into an array of dates.
            Date issueDate = schedule.date(0);
            std::vector<Date> newDates = schedule.dates();

            if (!schedule.hasIsRegular() || !schedule.isRegular(1))
            {
                Date firstCoupon = schedule.date(1);

                Date notionalFirstCoupon =
                    schedule.calendar().advance(firstCoupon,
                        -schedule.tenor(),
                        schedule.businessDayConvention(),
                        schedule.endOfMonth());

                newDates[0] = notionalFirstCoupon;

                //long first coupon
                if (notionalFirstCoupon > issueDate) {
                    Date priorNotionalCoupon =
                        schedule.calendar().advance(notionalFirstCoupon,
                                                    -schedule.tenor(),
                                                    schedule.businessDayConvention(),
                                                    schedule.endOfMonth());
                    newDates.insert(newDates.begin(),
                                    priorNotionalCoupon); //insert as the first element?
                }
            }

            if (!schedule.hasIsRegular() || !schedule.isRegular(schedule.size() - 1))
            {
                Date notionalLastCoupon =
                    schedule.calendar().advance(schedule.date(schedule.size() - 2),
                        schedule.tenor(),
                        schedule.businessDayConvention(),
                        schedule.endOfMonth());

                newDates[schedule.size() - 1] = notionalLastCoupon;

                if (notionalLastCoupon < schedule.endDate())
                {
                    Date nextNotionalCoupon =
                        schedule.calendar().advance(notionalLastCoupon,
                                                    schedule.tenor(),
                                                    schedule.businessDayConvention(),
                                                    schedule.endOfMonth());
                    newDates.push_back(nextNotionalCoupon);
                }
            }

            return newDates;
        }

        template <class T>
        Time yearFractionWithReferenceDates(const T& impl,
                                            const Date& d1, const Date& d2,
                                            const Date& d3, const Date& d4) {
            QL_REQUIRE(d1 <= d2,
                       "This function is only correct if d1 <= d2\n"
                       "d1: " << d1 << " d2: " << d2);

            Real referenceDayCount = Real(impl.dayCount(d3, d4));
            //guess how many coupon periods per year:
            Integer couponsPerYear;
            if (referenceDayCount < 16) {
                couponsPerYear = 1;
                referenceDayCount = impl.dayCount(d1, d1 + 1 * Years);
            }
            else {
                couponsPerYear = findCouponsPerYear(impl, d3, d4);
            }
            return Real(impl.dayCount(d1, d2)) / (referenceDayCount*couponsPerYear);
        }

    }

    ext::shared_ptr<DayCounter::Impl>
    ActualActual::implementation(ActualActual::Convention c, Schedule schedule) {
        switch (c) {
          case ISMA:
          case Bond:
            if (!schedule.empty())
                return ext::shared_ptr<DayCounter::Impl>(new ISMA_Impl(std::move(schedule)));
            else
                return ext::shared_ptr<DayCounter::Impl>(new Old_ISMA_Impl);
          case ISDA:
          case Historical:
          case Actual365:
            return ext::shared_ptr<DayCounter::Impl>(new ISDA_Impl);
          case AFB:
          case Euro:
            return ext::shared_ptr<DayCounter::Impl>(new AFB_Impl);
          default:
            QL_FAIL("unknown act/act convention");
        }
    }


    Time ActualActual::ISMA_Impl::yearFraction(const Date& d1,
                                               const Date& d2,
                                               const Date& d3,
                                               const Date& d4) const {
        if (d1 == d2) {
            return 0.0;
        } else if (d2 < d1) {
            return -yearFraction(d2, d1, d3, d4);
        }

        std::vector<Date> couponDates =
            getListOfPeriodDatesIncludingQuasiPayments(schedule_);

        Date firstDate = *std::min_element(couponDates.begin(), couponDates.end());
        Date lastDate = *std::max_element(couponDates.begin(), couponDates.end());

        QL_REQUIRE(d1 >= firstDate && d2 <= lastDate, "Dates out of range of schedule: "
                       << "date 1: " << d1 << ", date 2: " << d2 << ", first date: "
                       << firstDate << ", last date: " << lastDate);

        Real yearFractionSum = 0.0;
        for (Size i = 0; i < couponDates.size() - 1; i++) {
            Date startReferencePeriod = couponDates[i];
            Date endReferencePeriod = couponDates[i + 1];
            if (d1 < endReferencePeriod && d2 > startReferencePeriod) {
                yearFractionSum +=
                    yearFractionWithReferenceDates(*this,
                                                   std::max(d1, startReferencePeriod),
                                                   std::min(d2, endReferencePeriod),
                                                   startReferencePeriod,
                                                   endReferencePeriod);
            }
        }
        return yearFractionSum;
    }


    Time ActualActual::Old_ISMA_Impl::yearFraction(const Date& d1,
                                                   const Date& d2,
                                                   const Date& d3,
                                                   const Date& d4) const {
        if (d1 == d2)
            return 0.0;

        if (d1 > d2)
            return -yearFraction(d2,d1,d3,d4);

        // when the reference period is not specified, try taking
        // it equal to (d1,d2)
        Date refPeriodStart = (d3 != Date() ? d3 : d1);
        Date refPeriodEnd = (d4 != Date() ? d4 : d2);

        QL_REQUIRE(refPeriodEnd > refPeriodStart && refPeriodEnd > d1,
                   "invalid reference period: "
                   << "date 1: " << d1
                   << ", date 2: " << d2
                   << ", reference period start: " << refPeriodStart
                   << ", reference period end: " << refPeriodEnd);

        // estimate roughly the length in months of a period
        auto months = (Integer)std::lround(12 * Real(refPeriodEnd - refPeriodStart) / 365);

        // for short periods...
        if (months == 0) {
            // ...take the reference period as 1 year from d1
            refPeriodStart = d1;
            refPeriodEnd = d1 + 1*Years;
            months = 12;
        }

        Time period = Real(months)/12.0;

        if (d2 <= refPeriodEnd) {
            // here refPeriodEnd is a future (notional?) payment date
            if (d1 >= refPeriodStart) {
                // here refPeriodStart is the last (maybe notional)
                // payment date.
                // refPeriodStart <= d1 <= d2 <= refPeriodEnd
                // [maybe the equality should be enforced, since
                // refPeriodStart < d1 <= d2 < refPeriodEnd
                // could give wrong results] ???
                return period*Real(daysBetween(d1,d2)) /
                    daysBetween(refPeriodStart,refPeriodEnd);
            } else {
                // here refPeriodStart is the next (maybe notional)
                // payment date and refPeriodEnd is the second next
                // (maybe notional) payment date.
                // d1 < refPeriodStart < refPeriodEnd
                // AND d2 <= refPeriodEnd
                // this case is long first coupon

                // the last notional payment date
                Date previousRef = refPeriodStart - months*Months;

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
            Time sum = yearFraction(d1, refPeriodEnd,
                                    refPeriodStart, refPeriodEnd);

            // the part from refPeriodEnd to d2
            // count how many regular periods are in [refPeriodEnd, d2],
            // then add the remaining time
            Integer i=0;
            Date newRefStart, newRefEnd;
            for (;;) {
                newRefStart = refPeriodEnd + (months*i)*Months;
                newRefEnd = refPeriodEnd + (months*(i+1))*Months;
                if (d2 < newRefEnd) {
                    break;
                } else {
                    sum += period;
                    i++;
                }
            }
            sum += yearFraction(newRefStart,d2,newRefStart,newRefEnd);
            return sum;
        }
    }


    Time ActualActual::ISDA_Impl::yearFraction(const Date& d1,
                                               const Date& d2,
                                               const Date&,
                                               const Date&) const {
        if (d1 == d2)
            return 0.0;

        if (d1 > d2)
            return -yearFraction(d2,d1,Date(),Date());

        Integer y1 = d1.year(), y2 = d2.year();
        Real dib1 = (Date::isLeap(y1) ? 366.0 : 365.0),
             dib2 = (Date::isLeap(y2) ? 366.0 : 365.0);

        Time sum = y2 - y1 - 1;
        sum += daysBetween(d1, Date(1,January,y1+1))/dib1;
        sum += daysBetween(Date(1,January,y2),d2)/dib2;
        return sum;
    }


    Time ActualActual::AFB_Impl::yearFraction(const Date& d1,
                                              const Date& d2,
                                              const Date&,
                                              const Date&) const {
        if (d1 == d2)
            return 0.0;

        if (d1 > d2)
            return -yearFraction(d2,d1,Date(),Date());

        Date newD2=d2, temp=d2;
        Time sum = 0.0;
        while (temp > d1) {
            temp = newD2 - 1*Years;
            if (temp.dayOfMonth()==28 && temp.month()==2
                && Date::isLeap(temp.year())) {
                temp += 1;
            }
            if (temp>=d1) {
                sum += 1.0;
                newD2 = temp;
            }
        }

        Real den = 365.0;

        if (Date::isLeap(newD2.year())) {
            temp = Date(29, February, newD2.year());
            if (newD2>temp && d1<=temp)
                den += 1.0;
        } else if (Date::isLeap(d1.year())) {
            temp = Date(29, February, d1.year());
            if (newD2>temp && d1<=temp)
                den += 1.0;
        }

        return sum+daysBetween(d1, newD2)/den;
    }

    Date::serial_type ActualActual::ISMA_Impl::dayCount(const Date& d1, const Date& d2) const {
        return daysBetween(d1, d2);
    }

    Date::serial_type ActualActual::Old_ISMA_Impl::dayCount(const Date& d1, const Date& d2) const {
        return daysBetween(d1, d2);
    }

    Date::serial_type ActualActual::ISDA_Impl::dayCount(const Date& d1, const Date& d2) const {
        return daysBetween(d1, d2);
    }

    Date::serial_type ActualActual::AFB_Impl::dayCount(const Date& d1, const Date& d2) const {
        return daysBetween(d1, d2);
    }

}
