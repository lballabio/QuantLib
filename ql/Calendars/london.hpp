
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
/*! \file london.hpp
    \brief London calendar

    \fullpath
    ql/Calendars/%london.hpp
*/

// $Id$

#ifndef quantlib_london_calendar_h
#define quantlib_london_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    namespace Calendars {

        //! %London calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st (possibly moved to Monday)</li>
            <li>Good Friday</li>
            <li>Easter Monday</li>
            <li>Bank Holiday, first Monday of May</li>
            <li>Bank Holiday, last Monday of May</li>
            <li>Bank Holiday, last Monday of August</li>
            <li>Christmas, December 25th (possibly moved to Monday or
                Tuesday)</li>
            <li>Boxing Day, December 26th (possibly moved to Monday or
                Tuesday)</li>
            </ul>
        */
        class London : public Calendar {
          private:
            class LonCalendarFactory : public Calendar::factory {
              public:
                Calendar create() const { return London(); }
            };
            class LonCalendarImpl : public Calendar::WesternCalendarImpl {
              public:
                std::string name() const { return "London"; }
                bool isBusinessDay(const Date&) const;
            };
          public:
            London()
            : Calendar(Handle<CalendarImpl>(new LonCalendarImpl)) {}
            //! returns a factory of %London calendars
            Handle<factory> getFactory() const {
                return Handle<factory>(new LonCalendarFactory);
            }
        };

    }

}


#endif
