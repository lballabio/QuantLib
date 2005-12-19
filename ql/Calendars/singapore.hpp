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

/*! \file singapore.hpp
    \brief Singapore calendars
*/

#ifndef quantlib_singapore_calendar_hpp
#define quantlib_singapore_calendar_hpp

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Singapore calendars
    /*! Holidays for the Singapore exchange
        (data from <http://www.ses.com.sg>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's day, January 1st</li>
        <li>Good Friday</li>
        <li>Labour Day, May 1st</li>
        <li>National Day, August 9th</li>
        <li>Christmas, December 25th </li>
        <li>Boxing Day, December 26th </li>
        </ul>

        Other holidays for which no rule is given
        (data available for 2004-2005 only:)
        <ul>
        <li>Chinese New Year</li>
        <li>Hari Raya Haji</li>
        <li>Vesak Poya Day</li>
        <li>Deepavali</li>
        <li>Diwali</li>
        <li>Hari Raya Puasa</li>
        </ul>

        \ingroup calendars
    */
    class Singapore : public Calendar {
      private:
        class SgxImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Singapore exchange"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        enum Market { SGX    //!< Singapore exchange
        };
        Singapore(Market m = SGX);
    };

}


#endif
