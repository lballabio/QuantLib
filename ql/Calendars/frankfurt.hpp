
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
/*! \file frankfurt.hpp
    \brief Frankfurt calendar

    \fullpath
    ql/Calendars/%frankfurt.hpp
*/

// $Id$

#ifndef quantlib_frankfurt_calendar_h
#define quantlib_frankfurt_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    namespace Calendars {

        //! %Frankfurt calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st</li>
            <li>Good Friday</li>
            <li>Easter Monday</li>
            <li>Ascension Thursday</li>
            <li>Whit Monday</li>
            <li>Corpus Christi</li>
            <li>Labour Day, May 1st</li>
            <li>National Day, October 3rd</li>
            <li>Christmas Eve, December 24th</li>
            <li>Christmas, December 25th</li>
            <li>Boxing Day, December 26th</li>
            <li>New Year's Eve, December 31st</li>
            </ul>
        */
        class Frankfurt : public Calendar {
          private:
            class FFTCalendarImpl : public Calendar::WesternCalendarImpl {
              public:
                std::string name() const { return "Frankfurt"; }
                bool isBusinessDay(const Date&) const;
            };
          public:
            Frankfurt()
            : Calendar(Handle<CalendarImpl>(new FFTCalendarImpl)) {}
        };

    }

}


#endif
