
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

/*! \file milan.hpp
    \brief Milan calendar

    \fullpath
    ql/Calendars/%milan.hpp
*/

// $Id$

#ifndef quantlib_milan_calendar_h
#define quantlib_milan_calendar_h

#include "ql/calendar.hpp"

namespace QuantLib {

    namespace Calendars {

        //! %Milan calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st</li>
            <li>Epiphany, January 6th</li>
            <li>Easter Monday</li>
            <li>Liberation Day, April 25th</li>
            <li>Labour Day, May 1st</li>
            <li>Republic Day, June 2nd (since 2000)</li>
            <li>Assumption, August 15th</li>
            <li>All Saint's Day, November 1st</li>
            <li>Immaculate Conception, December 8th</li>
            <li>Christmas, December 25th</li>
            <li>St. Stephen, December 26th</li>
            </ul>
        */
        class Milan : public Calendar {
          private:
            class MilCalendarFactory : public Calendar::factory {
              public:
                Calendar create() const { return Milan(); }
            };
            class MilCalendarImpl : public Calendar::WesternCalendarImpl {
              public:
                std::string name() const { return "Milan"; }
                bool isBusinessDay(const Date&) const;
            };
          public:
            Milan()
            : Calendar(Handle<CalendarImpl>(new MilCalendarImpl)) {}
            //! returns a factory of %Milan calendars
            Handle<factory> getFactory() const {
                return Handle<factory>(new MilCalendarFactory);
            }
        };

    }

}


#endif
