
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

/*! \file hongkong.hpp
    \brief Hong Kong calendar
*/

#ifndef quantlib_hongkong_calendar_h
#define quantlib_hongkong_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! Hong Kong calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Ching Ming Festival, April 5th </li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labor Day, May 1st</li>
        <li>SAR Establishment Day, July 1st </li>
        <li>National Day, October 1st </li>
        <li>Christmas, December 25th </li>
        <li>Boxing Day, December 26th </li>
        <li>Christmas Holiday, December 27th </li>
        </ul>

        Other holidays for which no rule is given 
        (data available for 2004/2005 only:)
        <ul>
        <li>Lunar New Year</li>
        <li>Chinese New Year</li>
        <li>Buddha's birthday</li>
        <li>Tuen NG Festival</li>
        <li>Mid-autumn fest</li>
        <li>Chung Yeung fest</li>
        </ul>

        Data from http://www.hkex.com.hk

        \ingroup calendars
    */
    class HongKong : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "HongKong"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        HongKong();
    };

}


#endif
