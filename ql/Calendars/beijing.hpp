
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

/*! \file beijing.hpp
    \brief Beijing calendar
*/

#ifndef quantlib_beijing_calendar_h
#define quantlib_beijing_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Beijing calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's day, January 1st</li>
        <li>Labour Day, first week in May</li>
        <li>National Day, one week from October 1st</li>
        </ul>

        Other holidays for which no rule is given 
        (data available for 2004 only:)
        <ul>
        <li>Lunar New Year</li>
        </ul>

        \ingroup calendars
    */
    class Beijing : public Calendar {
      private:
        class Impl : public Calendar::Impl {
          public:
            std::string name() const { return "Beijing"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Beijing();
    };

}


#endif
