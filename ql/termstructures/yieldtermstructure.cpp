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

    Rate YieldTermStructure::parRate(Integer tenor,
                                     const Date& startDate,
                                     Frequency freq,
                                     bool extrapolate) const {
        std::vector<Date> dates(1, startDate);
        dates.reserve(tenor+1);
        for (Integer i=1; i<=tenor; ++i)
            dates.push_back(startDate + i*Years);
        return parRate(dates, freq, extrapolate);
    }

    Rate YieldTermStructure::parRate(const std::vector<Date>& dates,
                                     Frequency freq,
                                     bool extrapolate) const {
        std::vector<Time> times(dates.size());
        for (Size i=0; i<dates.size(); i++)
            times[i] = timeFromReference(dates[i]);
        return parRate(times,freq,extrapolate);
    }

    Rate YieldTermStructure::parRate(const std::vector<Time>& times,
                                     Frequency freq,
                                     bool extrapolate) const {
        QL_REQUIRE(times.size() >= 2, "at least two times are required");
        checkRange(times.back(), extrapolate);
        Real sum = 0.0;
        for (Size i=1; i<times.size(); i++)
            sum += discountImpl(times[i]);
        Real result = discountImpl(times.front())-discountImpl(times.back());
        result *=  Real(freq)/sum;
        return result;
    }

}
