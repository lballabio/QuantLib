
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
/*! \file zurich.hpp
    \brief Zurich calendar

    \fullpath
    ql/Calendars/%zurich.hpp
*/

// $Id$

#ifndef quantlib_zurich_calendar_h
#define quantlib_zurich_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    namespace Calendars {

        //! %Zurich calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st</li>
            <li>Berchtoldstag, January 2nd</li>
            <li>Good Friday</li>
            <li>Easter Monday</li>
            <li>Ascension Day</li>
            <li>Whit Monday</li>
            <li>Labour Day, May 1st</li>
            <li>National Day, August 1st</li>
            <li>Christmas, December 25th</li>
            <li>St. Stephen's Day, December 26th</li>
            </ul>
        */
        class Zurich : public Calendar {
          private:
            class ZurCalendarImpl : public Calendar::WesternCalendarImpl {
              public:
                std::string name() const { return "Zurich"; }
                bool isBusinessDay(const Date&) const;
            };
          public:
            Zurich()
            : Calendar(Handle<CalendarImpl>(new ZurCalendarImpl)) {}
        };

    }

}


#endif
