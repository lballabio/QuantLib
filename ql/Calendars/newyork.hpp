
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file newyork.hpp
    \brief New York calendar

    \fullpath
    ql/Calendars/%newyork.hpp
*/

// $Id$

#ifndef quantlib_newyork_calendar_h
#define quantlib_newyork_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    namespace Calendars {

        //! New York calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st (possibly moved to Monday if
                actually on Sunday, or to Friday if on Saturday)</li>
            <li>Martin Luther King's birthday, third Monday in January</li>
            <li>Washington's birthday, third Monday in February</li>
            <li>Memorial Day, last Monday in May</li>
            <li>Independence Day, July 4th (moved to Monday if Sunday or
                Friday if Saturday)</li>
            <li>Labor Day, first Monday in September</li>
            <li>Columbus Day, second Monday in October</li>
            <li>Veteran's Day, November 11th (moved to Monday if Sunday or
                Friday if Saturday)</li>
            <li>Thanksgiving Day, fourth Thursday in November</li>
            <li>Christmas, December 25th (moved to Monday if Sunday or Friday
                if Saturday)</li>
            </ul>
        */
        class NewYork : public Calendar {
          private:
            class NYCalendarImpl : public Calendar::WesternCalendarImpl {
              public:
                std::string name() const { return "NewYork"; }
                bool isBusinessDay(const Date&) const;
            };
          public:
            NewYork()
            : Calendar(Handle<CalendarImpl>(new NYCalendarImpl)) {}
        };

    }

}


#endif
