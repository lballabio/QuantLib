/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007, 2008, 2009, 2010, 2011 StatPro Italia srl

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

/*! \file chile.hpp
    \brief Chilean calendars
*/

#ifndef quantlib_chilean_calendar_hpp
#define quantlib_chilean_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Chilean calendars
    /*! Holidays for the Santiago Stock Exchange
        (data from <https://publicholidays.cl/>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Saturday</li>
        <li>Labour Day, May 1st</li>
        <li>Navy Day, May 21st</li>
        <li>Our Lady of Mount Carmel, July 16th</li>
        <li>Assumption Day, August 15th</li>
        <li>Independence Day, September 18th</li>
        <li>Army Day, September 19th</li>
        <li>All Saints' Day, November 1st</li>
        <li>Immaculate Conception, December 8th</li>
        <li>Christmas Day, December 25th</li>
        </ul>

        Other holidays for which no rule is given
        (data available for 2018-2024 only:)
        <ul>
        <li>New Year Holiday</li>
        <li>Feast of St Peter and St Paul</li>
        <li>Independence Day Holiday</li>
        <li>Army Day Holiday</li>
        <li>Day of the Race</li>
        <li>Reformation Day</li>
        </ul>

        \ingroup calendars
    */
    class Chile : public Calendar {
      private:
        class SseImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Santiago Stock Exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        enum Market { SSE    //!< Santiago Stock Exchange
        };
        Chile(Market m = SSE);
    };

}


#endif
