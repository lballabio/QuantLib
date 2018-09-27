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



namespace QuantLib {



    ext::shared_ptr<DayCounter::Impl>
    ActualActual::implementation(ActualActual::Convention c, 
                                 const Schedule& schedule) {
        switch (c) {
          case ISMA:
          case Bond:
            return ext::shared_ptr<DayCounter::Impl>(new ISMA_Impl(schedule));
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

	int ActualActual::ISMA_Impl::findCouponsPerYear(Date refStart, Date refEnd) const {
		// This will only work for day counts longer than 15 days.
		Integer months = Integer(0.5 + 12 * Real(dayCount(refStart, refEnd))/365.0);
		return (int)round(12.0 / Real(months));

	}

	Time ActualActual::ISMA_Impl::yearFractionWithReferenceDates(
		const Date& d1,
		const Date& d2,
		const Date& d3,
		const Date& d4
	) const {
		QL_REQUIRE(
			d1 <= d2, 
			"This function is only correct if d1 <= d2 \n d1: " << d1 << " d2: " << d2
		);
		//
		Real referenceDayCount = Real(dayCount(d3, d4));
		//guess how many coupon periods per year:
		int couponsPerYear;
		if (referenceDayCount < 16) {
			couponsPerYear = 1;
			referenceDayCount = dayCount(d1, d1 + 1 * Years);
		}
		else {
			couponsPerYear = findCouponsPerYear(d3, d4);
		}
		return Real(dayCount(d1, d2)) / (referenceDayCount*couponsPerYear);

	}

	std::vector<Date> ActualActual::ISMA_Impl::getListOfPeriodDatesIncludingQuasiPayments()
	const {
		//This function will process the schedule into an array of dates.
		Date issueDate = schedule_.date(0);
		Date firstCoupon = schedule_.date(1);
		Date notionalCoupon = schedule_.calendar().advance(firstCoupon,
			-schedule_.tenor(),
			schedule_.businessDayConvention(),
			schedule_.endOfMonth());

		std::vector<Date> newDates = schedule_.dates();
		newDates[0] = notionalCoupon;

		//long first coupon
		if (notionalCoupon > issueDate) {
			Date priorNotionalCoupon = schedule_.calendar().advance(notionalCoupon,
				-schedule_.tenor(),
				schedule_.businessDayConvention(),
				schedule_.endOfMonth());
			newDates.insert(newDates.begin(), priorNotionalCoupon); //insert as teh first element?
		}
		return newDates;
	}

	/*! \An ISMA day counter either needs a schedule or to have been explicitly passed a reference period. This usuage leads to innaccurate year fractions.
	*/
	Time ActualActual::ISMA_Impl::yearFractionGuess(
		const Date& start,
		const Date& end
	) const {
		//asymptopically correct.
		return Real(dayCount(start, end)) / 365.0;
	}

	Time ActualActual::ISMA_Impl::yearFractionUsingSchedule(
		const Date& start, const Date& end
	) const {
		std::vector<Date> couponDates = getListOfPeriodDatesIncludingQuasiPayments();

		Real yearFractionSum = 0;
		for (int i = 0; i < couponDates.size() - 1; i++) {
			Date startReferencePeriod = couponDates[i];
			Date endReferencePeriod = couponDates[i + 1];
			if (endReferencePeriod > start && end > startReferencePeriod) {
				yearFractionSum += yearFractionWithReferenceDates(
					(start > startReferencePeriod) ? start : startReferencePeriod,
					(end < endReferencePeriod) ? end : endReferencePeriod,
					startReferencePeriod,
					endReferencePeriod
				);
			}
		}
		return yearFractionSum;
	}

	bool ActualActual::ISMA_Impl::isReferencePeriodSpecified(const Date& refPeriodStart, const Date& refPeriodEnd) const {
		//True only if neither date is a null date;
		return refPeriodStart != Date() && refPeriodEnd != Date();
	}

    Time ActualActual::ISMA_Impl::yearFraction(const Date& d1,
                                               const Date& d2,
                                               const Date& d3,
                                               const Date& d4) const {
		// Base Cases;
		if (d1 == d2) {
			return 0.0;
		} else if (d2 < d1) {
			return -yearFraction(d2, d1, d3, d4);
		}

		//To account for the fact that some higher level classes to not work out the reference periods correctly.
		//A day counter with a schedule takes precedence over an explicit reference period.
		if (!schedule_.empty()) {
			return yearFractionUsingSchedule(d1, d2);
		}
		else if (isReferencePeriodSpecified(d3, d4)) {
			return yearFractionWithReferenceDates(d1, d2, d3, d4);
		}
		else {
			return yearFractionGuess(d1, d2);
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
        // FLOATING_POINT_EXCEPTION
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

}
