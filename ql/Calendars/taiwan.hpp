
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

/*! \file taiwan.hpp
    \brief Taiwan calendar
*/

#ifndef quantlib_taiwan_calendar_h
#define quantlib_taiwan_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Taiwan calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Peace Day, February 28th</li>           
        <li>Labor Day, May 1st</li>
        <li>National Day, October 10th</li>
        </ul>

        Other holidays for which no rule is given 
        (data available for 2004-2006 only:)
        <ul>
        <li>Lunar New Year</li>
        <li>Tomb Sweeping Day</li>
        <li>Dragon Boat Day</li>
        <li>Mid-Autumn Festival</li>
        </ul>

        Data from http://www.taifex.com.tw

        \ingroup calendars
    */
    class Taiwan : public Calendar {
      private:
        class Impl : public Calendar::Impl {
          public:
            std::string name() const { return "Taiwan"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Taiwan()
        : Calendar(Handle<Calendar::Impl>(new Taiwan::Impl)) {}
    };

}


#endif
