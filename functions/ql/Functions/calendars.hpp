/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano

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

#ifndef quantlib_functions_calendar_h
#define quantlib_functions_calendar_h

#include <ql/Functions/qlfunctions.hpp>
#include <ql/calendar.hpp>
#include <ql/daycounter.hpp>
#include <vector>

namespace QuantLib {

    //! Returns the holidays between two dates
    std::vector<Date> holidayList(const Calendar& calendar,
                                  const Date& from,
                                  const Date& to,
                                  bool includeWeekEnds = false);

    /*! return the evaluation date
    */
    Date evaluationDate();

    /*! set the evaluation date
    */
    Date setEvaluationDate(const Date &evalDate);

    /*! return the advanced date over a given calendar
    */
    Date advanceCalendar(
            const Date &startDate,
            const long &n,
            const TimeUnit &timeUnits,
            const Calendar &calendar,
            const BusinessDayConvention &convention);

    /*! return the year fraction with respect to a given day-counter convention
    */
    double yearFraction(
        const DayCounter &dayCounter,
        const Date &startDate,
        const Date &endDate,
        const Date &refStartDate,
        const Date &refEndDate);    

    //! whether or not the given date is an IMM date
    inline bool isIMMdate(const Date& d) {
        return QuantLib::Date::isIMMdate(d);
    }

    //! next IMM date following (or equal to) the given date
    /*! returns the 1st delivery date for next contract listed in the
        International Money Market section of the Chicago Mercantile
        Exchange.

        \warning The result date is following or equal to the
                    original date.
    */
    inline QuantLib::Date nextIMMdate(const Date& d) {
        return QuantLib::Date::nextIMMdate(d);
    }

    /*! return the day count with respect to a given day-counter
        convention */
    inline double dayCount(
            const DayCounter &dayCount,
            const Date &startDate,
            const Date &endDate) {
        return dayCount.dayCount(startDate, endDate);
    }

}

#endif

