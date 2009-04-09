/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    YieldTermStructure::YieldTermStructure(const DayCounter& dc)
    : TermStructure(dc) {}

    YieldTermStructure::YieldTermStructure(const Date& referenceDate,
                                           const Calendar& cal,
                                           const DayCounter& dc)
    : TermStructure(referenceDate, cal, dc) {}

    YieldTermStructure::YieldTermStructure(Natural settlementDays,
                                           const Calendar& cal,
                                           const DayCounter& dc)
    : TermStructure(settlementDays, cal, dc) {}


    InterestRate YieldTermStructure::zeroRate(const Date& d,
                                              const DayCounter& dayCounter,
                                              Compounding comp,
                                              Frequency freq,
                                              bool extrapolate) const {
        if (d==referenceDate()) {
            Time t = 0.0001;
            Real compound = 1.0/discount(t, extrapolate);
            return InterestRate::impliedRate(compound, t, dayCounter,
                                             comp, freq);
        }
        Real compound = 1.0/discount(d, extrapolate);
        return InterestRate::impliedRate(compound, referenceDate(), d,
                                         dayCounter, comp, freq);
    }

    InterestRate YieldTermStructure::zeroRate(Time t,
                                              Compounding comp,
                                              Frequency freq,
                                              bool extrapolate) const {
        if (t==0.0) t = 0.0001;
        Real compound = 1.0/discount(t, extrapolate);
        return InterestRate::impliedRate(compound, t, dayCounter(),
                                         comp, freq);
    }

    InterestRate YieldTermStructure::forwardRate(const Date& d1,
                                                 const Date& d2,
                                                 const DayCounter& dayCounter,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {
        if (d1==d2) {
            Time t = timeFromReference(d1);
            Time t1 = std::max(t - 0.0001, 0.0);
            Time t2 = t + 0.0001;
            Real compound =
                discount(t1, extrapolate)/discount(t2, true);
            return InterestRate::impliedRate(compound, t2-t1,
                                             dayCounter, comp, freq);
        }
        QL_REQUIRE(d1 < d2,  d1 << " later than " << d2);
        Real compound = discount(d1, extrapolate)/discount(d2, extrapolate);
        return InterestRate::impliedRate(compound,
                                         d1, d2, dayCounter,
                                         comp, freq);
    }

    InterestRate YieldTermStructure::forwardRate(Time t1,
                                                 Time t2,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {
        Real compound;
        if (t2==t1) {
            t2 = t1+0.0001;
            t1 = std::max(t1-0.0001, 0.0);
            compound = discount(t1, extrapolate)/discount(t2, true);
        } else {
            QL_REQUIRE(t2>t1, "t2 (" << t2 << ") < t1 (" << t2 << ")");
            compound = discount(t1, extrapolate)/discount(t2, extrapolate);
        }
        return InterestRate::impliedRate(compound, t2-t1,
                                         dayCounter(), comp, freq);
    }

    Rate YieldTermStructure::parRate(Natural tenor,
                                     const Date& startDate,
                                     const DayCounter& resultDayCounter,
                                     Frequency freq,
                                     bool extrapolate) const {
        std::vector<Date> dates(tenor+1);
        dates[0] = startDate;
        for (Natural i=1; i<=tenor; ++i)
            dates.push_back(startDate + i*Years);
        return parRate(dates, resultDayCounter, freq, extrapolate);
    }

    Rate YieldTermStructure::parRate(const std::vector<Date>& dates,
                                     const DayCounter& resultDayCounter,
                                     Frequency freq,
                                     bool extrapolate) const {
        QL_REQUIRE(dates.size() >= 2, "at least two dates are required");
        Real sum = 0.0;
        Time dt;
        for (Size i=1; i<dates.size(); ++i) {
            dt = resultDayCounter.yearFraction(dates[i-1], dates[i]);
            QL_REQUIRE(dt>0.0, "unsorted dates");
            sum += discount(dates[i], extrapolate) * dt;
        }
        Real result = discount(dates.front(), extrapolate)-discount(dates.back(), extrapolate);
        return result/sum;
    }

    Rate YieldTermStructure::parRate(const std::vector<Time>& times,
                                     Frequency freq,
                                     bool extrapolate) const {
        QL_REQUIRE(times.size() >= 2, "at least two times are required");
        Real sum = 0.0;
        Time dt;
        for (Size i=1; i<times.size(); ++i) {
            dt = times[i]-times[i-1];
            QL_REQUIRE(dt>0.0, "unsorted times");
            sum += discount(times[i], extrapolate) * dt;
        }
        Real result = discount(times.front(), extrapolate)-discount(times.back(), extrapolate);
        return result/sum;
    }

}
