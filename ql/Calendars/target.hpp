
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

/*! \file target.hpp
    \brief TARGET calendar

    \fullpath
    ql/Calendars/%target.hpp
*/

// $Id$

#ifndef quantlib_target_calendar_h
#define quantlib_target_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    namespace Calendars {

        //! %TARGET calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st</li>
            <li>Good Friday</li>
            <li>Easter Monday</li>
            <li>Labour Day, May 1st</li>
            <li>Christmas, December 25th</li>
            <li>Day of Goodwill, December 26th</li>
            </ul>
        */
        class TARGET : public Calendar {
          private:
            class Impl : public Calendar::WesternImpl {
              public:
                std::string name() const { return "TARGET"; }
                bool isBusinessDay(const Date&) const;
            };
          public:
            TARGET()
            : Calendar(Handle<Calendar::Impl>(new TARGET::Impl)) {}
        };

    }

}


#endif
