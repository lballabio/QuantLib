/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 SoftSolution srl

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

/*! \file malta.hpp
    \brief Maltese calendar
*/

#ifndef quantlib_malta_calendar_hpp
#define quantlib_malta_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Malta calendar
    /*! Holidays for the Malta Stock Exchange (https://borzamalta.com.mt):
      Calendars here https://borzamalta.com.mt/publications-and-statistics?category=54
      https://cdn.borzamalta.com.mt/download/2025%20Trading%20Calendar%20EUR_GBP_USD%20Final.pdf
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>St. Paul's Shipwreck, February 10th</li>
        <li>St. Joseph's Day, March 19th</li>
        <li>Freedom Day, March 31st</li>
        <li>Good Friday (Orthodox calendar, variable date)</li>
        <li>Easter Monday (variable date)</li>
        <li>Labour Day, May 1st</li>
        <li>Feast of Saints Peter and Paul (Imnarja), June 29th</li>
        <li>Assumption of Mary, August 15th</li>
        <li>Feast of Our Lady of Victories (Nativity of Mary), September 8th</li>
        <li>Independence Day, September 21st</li>
        <li>Immaculate Conception, December 8th</li>
        <li>Republic Day, December 13th</li>
        <li>Christmas Vigil, December 24th</li>
        <li>Christmas Day, December 25th</li>
        <li>Boxing Day, December 26th (occasionally observed)</li>
        <li>New Year's Eve, December 31st (non-trading day)</li>
        </ul>

        Notes:
        - Holidays falling on a weekend are generally not shifted.
        - Trading hours and holiday observance are set by the Malta Stock Exchange each year.
        \ingroup calendars
    */


    class Malta : public Calendar {
    private:
        class MseImpl final : public Calendar::WesternImpl {
        public:
            std::string name() const override { return "Malta Stock Exchange"; }
            bool isWeekend(Weekday) const override;
            bool isBusinessDay(const Date&) const override;
        };

    public:
        enum Market {
            MSE //!< Malta Stock Exchange
        };
        explicit Malta(Market market = MSE);
    };

}


#endif
