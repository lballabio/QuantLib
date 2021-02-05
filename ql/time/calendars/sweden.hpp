/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file sweden.hpp
    \brief Swedish calendar
*/

#ifndef quantlib_swedish_calendar_hpp
#define quantlib_swedish_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Swedish calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Epiphany, January 6th</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Ascension</li>
        <li>Whit(Pentecost) Monday (until 2004)</li>
        <li>May Day, May 1st</li>
        <li>National Day, June 6th</li>
        <li>Midsummer Eve (Friday between June 19-25)</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas Day, December 25th</li>
        <li>Boxing Day, December 26th</li>
        <li>New Year's Eve, December 31th</li>
        </ul>

        \ingroup calendars
    */
    class Sweden : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Sweden"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        Sweden();
    };

}


#endif
