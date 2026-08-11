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

/*! \file croatia.hpp
    \brief Croatia calendars
*/

#ifndef quantlib_croatia_calendar_hpp
#define quantlib_croatia_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Croatian calendars
    /*! Holidays for the Zagreb stock exchange (see https://zse.hr/en/non-trading-days/110):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Epiphany, January 6th</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>National Day May 30th</li>
        <li>Corpus Christi, 60 days after Easter Sunday.</li>
        <li>Anti-Fascist Struggle Day, June 22nd</li>
        <li>Victory and Homeland Thanksgiving Day and the Day of Croatian Defenders August 5th</li>
        <li>Assumption of Mary, August 15th</li>
        <li>Remembrance Day for the Victims of the Vukovar and Skabrnja War Memorials, November 18th</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>St. Stephen, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        \ingroup calendars
    */
    class Croatia : public Calendar {
    private:
        class ZseImpl final : public Calendar::WesternImpl {
        public:
            std::string name() const override { return "Zagreb stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
    public:
        enum Market {
            ZSE    //!< Zagreb stock exchange
        };
        explicit Croatia(Market m = ZSE);
    };

}


#endif
