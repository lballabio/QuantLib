
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file zurich.hpp
    \brief Zurich calendar
*/

#ifndef quantlib_zurich_calendar_h
#define quantlib_zurich_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

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

        \ingroup calendars
    */
    class Zurich : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Zurich"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Zurich();
    };

}


#endif
