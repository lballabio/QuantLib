
/*
 Copyright (C) 2004 FIMAT Group

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

/*! \file seoul.hpp
    \brief South Korea calendar
*/

#ifndef quantlib_seoul_calendar_h
#define quantlib_seoul_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Seoul calendar
    /*! Holidays:
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

        Data from http://www.kofex.com

        \ingroup calendars
    */
    class Seoul : public Calendar {
      private:
        class Impl : public Calendar::Impl {
          public:
            std::string name() const { return "Seoul"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Seoul();
    };

}


#endif
