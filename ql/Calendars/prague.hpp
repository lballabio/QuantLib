/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file prague.hpp
    \brief Prague calendar
*/

#ifndef quantlib_prague_calendar_hpp
#define quantlib_prague_calendar_hpp

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Prague calendar
    /*! Holidays (see http://www.pse.cz/):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Liberation Day, May 8th</li>
        <li>SS. Cyril and Methodius, July 5th</li>
        <li>Jan Hus Day, July 6th</li>
        <li>Czech Statehood Day, September 28th</li>
        <li>Independence Day, October 28th</li>
        <li>Struggle for Freedom and Democracy Day, November 17th</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>St. Stephen, December 26th</li>
        </ul>

        \ingroup calendars
    */
    class Prague : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Prague"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Prague();
    };

}


#endif
