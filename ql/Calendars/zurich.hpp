
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file zurich.hpp
    \brief Zurich calendar

    \fullpath
    ql/Calendars/%zurich.hpp
*/

// $Id$

#ifndef quantlib_zurich_calendar_h
#define quantlib_zurich_calendar_h

#include "ql/calendar.hpp"

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
            class ZurCalendarFactory : public Calendar::factory {
              public:
                Calendar create() const { return Zurich(); }
            };
            class ZurCalendarImpl : public Calendar::WesternCalendarImpl {
              public:
                std::string name() const { return "Zurich"; }
                bool isBusinessDay(const Date&) const;
            };
          public:
            Zurich()
            : Calendar(Handle<CalendarImpl>(new ZurCalendarImpl)) {}
            //! returns a factory of %Zurich calendars
            Handle<factory> getFactory() const {
                return Handle<factory>(new ZurCalendarFactory);
            }
        };

    }

}


#endif
