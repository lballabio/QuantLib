/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023, 2026 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/comparison.hpp>
#include <ql/time/daycounters/yearfractiontodate.hpp>

#ifdef QL_HIGH_RESOLUTION_DATE
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual364.hpp>
#include <ql/time/daycounters/actual366.hpp>
#include <ql/time/daycounters/actual36525.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <vector>
#include <algorithm>
#endif

#include <boost/numeric/conversion/cast.hpp>
#include <cmath>

namespace QuantLib {

    Date yearFractionToDate(
        const DayCounter& dayCounter, const Date& referenceDate, Time t) {
        Date guessDate = referenceDate
            + Period(boost::numeric_cast<Integer>(round(t * 365.25)), Days);
        Time guessTime = dayCounter.yearFraction(referenceDate, guessDate);

        guessDate += Period(boost::numeric_cast<Integer>(
            round((t - guessTime)*365.25)), Days);
        guessTime = dayCounter.yearFraction(referenceDate, guessDate);

        if (close_enough(guessTime, t))
            return guessDate;

        const Integer searchDirection
            = boost::numeric_cast<Integer>(std::copysign(1.0, t - guessTime));

        const Time compT = t + searchDirection*500*QL_EPSILON;

        Date nextDate;
        for (TimeUnit u: {Years, Months, Days}) {
            while (searchDirection*(
                dayCounter.yearFraction(
                    referenceDate,
                    nextDate = guessDate + Period(searchDirection, u)) - compT) < 0.0)
                guessDate = nextDate;
        }

        guessTime = dayCounter.yearFraction(referenceDate, guessDate);

#ifdef QL_HIGH_RESOLUTION_DATE
        const static std::vector<std::string> linearDayCounters = {
            Actual365Fixed(Actual365Fixed::Standard).name(),
            Actual366().name(),
            Actual364().name(),
            Actual36525().name(),
            Actual360().name()
        };

        const bool linearInTime = std::find(
            linearDayCounters.begin(), linearDayCounters.end(), dayCounter.name()
        ) != linearDayCounters.end();

        if (linearInTime) {
            const Time dtPerDay = searchDirection * dayCounter.yearFraction(
                guessDate, guessDate + Period(searchDirection, Days));

            const Time dt = t - guessTime;

            const Real rs = dt/dtPerDay * (24*60*60);
            const Integer s = boost::numeric_cast<Integer>(rs);
            const Integer us = boost::numeric_cast<Integer>(std::round((rs - s) * 1e6));

            return guessDate + Period(s, Seconds) + Period(us, Microseconds);
        }
#endif
        if (close_enough(guessTime, t)
                || std::abs(dayCounter.yearFraction(referenceDate, guessDate + Period(searchDirection, Days)) - compT) >
                    std::abs(guessTime - compT))
            return guessDate;
        else
            return guessDate + Period(searchDirection, Days);
    }

    Date roundToDayStart(const Date& d) {
#ifdef QL_HIGH_RESOLUTION_DATE
        const Date m(d.dayOfMonth(), d.month(), d.year());

        if (d.fractionOfDay() >= 0.5)
            return m + Period(1, Days);
        else
            return m;
#else
        return d;
#endif
    }
}

