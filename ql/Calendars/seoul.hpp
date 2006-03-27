/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group

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

/*! \file seoul.hpp
    \brief South Korean calendars
*/

#ifndef quantlib_south_korean_calendar_hpp
#define quantlib_south_korean_calendar_hpp

#include <ql/calendar.hpp>

namespace QuantLib {

    //! South Korean calendars
    /*! Holidays for the Korea exchange
        (data from <http://www.kofex.com>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Independence Day, March 1st</li>
        <li>Arbour Day, April 5th</li>
        <li>Labor Day, May 1st</li>
        <li>Children's Day, May 5th</li>
        <li>Memorial Day, June 6th</li>
        <li>Constitution Day, July 17th</li>
        <li>Liberation Day, August 15th</li>
        <li>National Fondation Day, October 3th</li>
        <li>Christmas Day, December 25th</li>
        </ul>

        Other holidays for which no rule is given
        (data available for 2004-2006 only:)
        <ul>
        <li>Lunar New Year</li>
        <li>Election Day 2004</li>
        <li>Buddha's birthday</li>
        <li>Harvest Moon Day</li>
        </ul>

        \ingroup calendars
    */
    class SouthKorea : public Calendar {
      private:
        class KrxImpl : public Calendar::Impl {
          public:
            std::string name() const { return "Korea exchange"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        enum Market { KRX    //!< Korea exchange
        };
        SouthKorea(Market m = KRX);
    };

}


#endif
