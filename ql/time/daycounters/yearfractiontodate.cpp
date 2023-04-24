/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Klaus Spanderen

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

#include <ql/math/comparison.hpp>
#include "ql/time/daycounters/yearfractiontodate.hpp"

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
            = boost::numeric_cast<Integer>(copysign(1.0, t - guessTime));

        t += searchDirection*100*QL_EPSILON;

        Date nextDate;
        for (TimeUnit u: {Years, Months, Days}) {
            while (searchDirection*(
                dayCounter.yearFraction(
                    referenceDate,
                    nextDate = guessDate + Period(searchDirection, u)) - t) < 0.0)
                guessDate = nextDate;
        }

        guessTime = dayCounter.yearFraction(referenceDate, guessDate);
        if (close_enough(guessTime, t)
                || std::abs(dayCounter.yearFraction(referenceDate,
                    guessDate + Period(searchDirection, Days)) - t) >
                    std::abs(guessTime - t))
            return guessDate;
        else
            return guessDate + Period(searchDirection, Days);
    }
}

