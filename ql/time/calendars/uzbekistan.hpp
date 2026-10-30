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

/*! \file uzbekistan.hpp
    \brief Uzbekistan calendar
*/

#ifndef quantlib_uzbekistan_calendar_hpp
#define quantlib_uzbekistan_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Uzbekistan calendar
    /*! Holidays for the Uzbekistan exchanges
        (data from https://www.advantour.com/uzbekistan/holidays.htm):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>International Women's Day, March 8th</li>
        <li>Navruz (Persian New Year), March 21st</li>
        <li>Day of Remembrance and Honors, May 9th</li>
        <li>Independence Day, September 1st</li>
        <li>Teachers' Day, October 1st</li>
        <li>Constitution Day, December 8th</li>
        <li>Eid al-Fitr (Ramazon Hayit), variable date</li>
        <li>Eid al-Adha (Qurbon Hayit), variable date</li>
        </ul>
        \ingroup calendars
    */

    class Uzbekistan : public Calendar {
    private:
        class Impl final : public Calendar::WesternImpl {
        public:
            std::string name() const override { return "Uzbekistan Stock Exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
    public:
        enum Market {
            UZSE, //!< Uzbekistan Stock Exchange
        };
        explicit Uzbekistan(Market market = UZSE);
    };

}

#endif
