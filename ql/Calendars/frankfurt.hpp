
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

/*! \file frankfurt.hpp
    \brief Frankfurt calendar

    \fullpath
    ql/Calendars/%frankfurt.hpp
*/

// $Id$

#ifndef quantlib_frankfurt_calendar_h
#define quantlib_frankfurt_calendar_h

#include "ql/Calendars/westerncalendar.hpp"

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
        class Frankfurt : public WesternCalendar {
          public:
            Frankfurt() {}
            std::string name() const { return "Frankfurt"; }
            bool isBusinessDay(const Date&) const;
          private:
              class FFTCalendarFactory : public Calendar::factory {
              public:
                Handle<Calendar> create() const {
                    return Handle<Calendar>(new Frankfurt);
                }
            };
          public:
            //! returns a factory of %Frankfurt calendars
            Handle<factory> getFactory() const {
                return Handle<factory>(new FFTCalendarFactory);
            }
        };

    }

}


#endif
