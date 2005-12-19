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

/*! \file beijing.hpp
    \brief Chinese calendar
*/

#ifndef quantlib_chinese_calendar_hpp
#define quantlib_chinese_calendar_hpp

#include <ql/calendar.hpp>

namespace QuantLib {

    //! Chinese calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's day, January 1st</li>
        <li>Labour Day, first week in May</li>
        <li>National Day, one week from October 1st</li>
        </ul>

        Other holidays for which no rule is given:
        <ul>
        <li>Lunar New Year (data available for 2004 only)</li>
        <li>Spring Festival</li>
        <li>Last day of Lunar Year</li>
        </ul>

        \ingroup calendars
    */
    class China : public Calendar {
      private:
        class Impl : public Calendar::Impl {
          public:
            std::string name() const { return "China"; }
            bool isBusinessDay(const Date&) const;
            //! expressed relative to first day of year
            static Day springFestival(Year y);
        };
      public:
        China();
    };

    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to China */
    typedef China Beijing;
    #endif

}


#endif
