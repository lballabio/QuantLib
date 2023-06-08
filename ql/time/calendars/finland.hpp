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

/*! \file finland.hpp
    \brief Finnish calendar
*/

#ifndef quantlib_finnish_calendar_hpp
#define quantlib_finnish_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Finnish calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Epiphany, January 6th</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Ascension Thursday</li>
        <li>Labour Day, May 1st</li>
        <li>Midsummer Eve (Friday between June 19-25)</li>
        <li>Independence Day, December 6th</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Boxing Day, December 26th</li>
        </ul>

        \ingroup calendars
    */
    class Finland : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Finland"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        Finland();
    };

}


#endif
