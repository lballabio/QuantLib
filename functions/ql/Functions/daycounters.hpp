/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_functions_daycounters_h
#define quantlib_functions_daycounters_h

#include <ql/daycounter.hpp>
#include <string>

namespace QuantLib {

    DayCounter dayCounterFromString(std::string s);

    /*! return the day count with respect to a given day-counter
        convention */
    inline BigInteger dayCount(
            const DayCounter &dayCount,
            const Date &startDate,
            const Date &endDate) {
        return dayCount.dayCount(startDate, endDate);
    }

    /*! return the year fraction with respect to a given day-counter convention
    */
    inline Time yearFraction(
            const DayCounter &dayCounter,
            const Date &startDate,
            const Date &endDate,
            const Date &refStartDate,
            const Date &refEndDate) {
        return dayCounter.yearFraction(startDate, endDate, refStartDate, refEndDate);
    }
}


#endif
