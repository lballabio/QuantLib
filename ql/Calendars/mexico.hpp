/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 RiskMap srl

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

/*! \file mexico.hpp
    \brief Mexican calendar
*/

#ifndef quantlib_mexico_calendar_hpp
#define quantlib_mexico_calendar_hpp

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Mexican calendar
    /*! Holidays
        (data from <http://www.bmv.com.mx/>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Constitution Day, February 5th</li>
        <li>Birthday of Benito Juarez, March 21st</li>
        <li>Holy Thursday</li>
        <li>Good Friday</li>
        <li>Labour Day, May 1st</li>
        <li>National Day, September 16th</li>
        <li>Our Lady of Guadalupe, December 12th</li>
        <li>Christmas, December 25th</li>
        </ul>

        \ingroup calendars
    */
    class Mexico : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Mexico"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Mexico();
    };

}


#endif
