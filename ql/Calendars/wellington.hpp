
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

/*! \file wellington.hpp
    \brief Wellington calendar

    \fullpath
    ql/Calendars/%wellington.hpp
*/

// $Id$

#ifndef quantlib_wellington_calendar_h
#define quantlib_wellington_calendar_h

#include "ql/Calendars/westerncalendar.hpp"

namespace QuantLib {

    namespace Calendars {

        //! %Wellington calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st (possibly moved to Monday or
                Tuesday)</li>
            <li>Day after New Year's Day, January 2st (possibly moved to
                Monday or Tuesday)</li>
            <li>Anniversary Day, Monday nearest January 22nd</li>
            <li>Waitangi Day. February 6th</li>
            <li>Good Friday</li>
            <li>Easter Monday</li>
            <li>ANZAC Day. April 25th</li>
            <li>Queen's Birthday, first Monday in June</li>
            <li>Labour Day, fourth Monday in October</li>
            <li>Christmas, December 25th (possibly moved to Monday or
                Tuesday)</li>
            <li>Boxing Day, December 26th (possibly moved to Monday or
                Tuesday)</li>
            </ul>
            \note The holiday rules for Wellington were documented by
            David Gilbert for IDB (http://www.jrefinery.com/ibd/)
        */
        class Wellington : public WesternCalendar {
          public:
            Wellington() {}
            std::string name() const { return "Wellington"; }
            bool isBusinessDay(const Date&) const;
          private:
            class WelCalendarFactory : public Calendar::factory {
              public:
                Handle<Calendar> create() const {
                    return Handle<Calendar>(new Wellington);
                }
            };
          public:
            //! returns a factory of %Wellington calendars
            Handle<factory> getFactory() const {
                return Handle<factory>(new WelCalendarFactory);
            }
        };

    }

}


#endif
