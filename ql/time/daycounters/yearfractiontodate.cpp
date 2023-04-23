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

#include "yearfractiontodate.hpp"

#include <ql/math/comparison.hpp>
#include <cmath>

namespace QuantLib {

    YearFractionToDate::YearFractionToDate(
        const DayCounter& dayCounter, const Date& referenceDate):
        dayCounter_(dayCounter),
        referenceDate_(referenceDate) {
    }

    Date YearFractionToDate::operator()(Time t) const {
        Date guessDate = referenceDate_ + Period(round(t * 365.25), Days);
        Time guessTime = dayCounter_.yearFraction(referenceDate_, guessDate);

        guessDate += Period(round((t - guessTime)*365.25), Days);
        guessTime = dayCounter_.yearFraction(referenceDate_, guessDate);

        if (close_enough(guessTime, t))
            return guessDate;

        const int searchDirection = copysign(1.0, t - guessTime);

        t += searchDirection*100*QL_EPSILON;

        Date nextDate;
        for (TimeUnit u: {Years, Months, Days}) {
            while (searchDirection*(
                dayCounter_.yearFraction(
                    referenceDate_,
                    nextDate = guessDate + Period(searchDirection, u)) - t) < 0.0)
                guessDate = nextDate;
        }

        guessTime = dayCounter_.yearFraction(referenceDate_, guessDate);
        if (close_enough(guessTime, t)
                || std::abs(dayCounter_.yearFraction(referenceDate_,
                    guessDate + Period(searchDirection, Days)) - t) >
                    std::abs(guessTime - t))
            return guessDate;
        else
            return guessDate + Period(searchDirection, Days);
    }
}

