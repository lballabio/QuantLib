

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file helsinki.hpp
    \brief Helsinki calendar

    \fullpath
    ql/Calendars/%helsinki.hpp
*/

// $Id$

#ifndef quantlib_helsinki_calendar_h
#define quantlib_helsinki_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    namespace Calendars {

        //! %Helsinki calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st</li>
            <li>Epiphany, January 6th</li>
            <li>Good Friday</li>
            <li>Easter Monday</li>
            <li>Ascension Thursday</li>
            <li>Labour Day, May 1st</li>
            <li>Midsummer Eve, June 21st</li>
            <li>Independence Day, December 6th</li>
            <li>Christmas Eve, December 24th</li>
            <li>Christmas, December 25th</li>
            <li>Boxing Day, December 26th</li>
            </ul>
            \note The holiday rules for Wellington were documented by
            Veli-Pekka Mattila for IDB (http://www.jrefinery.com/ibd/)
        */
        class Helsinki : public Calendar {
          private:
            class HSKCalendarImpl : public Calendar::WesternCalendarImpl {
              public:
                std::string name() const { return "Helsinki"; }
                bool isBusinessDay(const Date&) const;
            };
          public:
            Helsinki()
            : Calendar(Handle<CalendarImpl>(new HSKCalendarImpl)) {}
        };

    }

}


#endif
