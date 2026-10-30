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

/*! \file slovenia.hpp
    \brief Slovenia calendars
*/

#ifndef quantlib_slovenia_calendar_hpp
#define quantlib_slovenia_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Slovenia calendars
    /*! Holidays for the Slovenia stock exchange
        (data from <https://ljse.si/en/non-trading-days/110>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>New Year's Holiday, January 2nd</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>May Day, May 1st</li>
        <li>May Day Holiday, May 2nd</li>
        <li>Statehood Day, June 25th  </li>
        <li>Assumption of Mary, August 15th</li>
        <li>Reformation Day, October 31st</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>St. Stephen, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        \ingroup calendars
    */
    class Slovenia : public Calendar {
    private:
        class LseImpl final : public Calendar::WesternImpl {
        public:
            std::string name() const override { return "Ljubljana stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
    public:
        enum Market {
            LSE    //!< Ljubljana stock exchange
        };
        explicit Slovenia(Market m = LSE);
    };

}


#endif
