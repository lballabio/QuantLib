
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
/*! \file wellington.hpp
    \brief Wellington calendar

    \fullpath
    ql/Calendars/%wellington.hpp
*/

// $Id$

#ifndef quantlib_wellington_calendar_h
#define quantlib_wellington_calendar_h

#include <ql/calendar.hpp>

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
        class Wellington : public Calendar {
          private:
            class WelCalendarImpl : public Calendar::WesternCalendarImpl {
              public:
                std::string name() const { return "Wellington"; }
                bool isBusinessDay(const Date&) const;
            };
          public:
            Wellington()
            : Calendar(Handle<CalendarImpl>(new WelCalendarImpl)) {}
        };

    }

}


#endif
