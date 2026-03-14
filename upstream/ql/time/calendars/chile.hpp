/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Anubhav Pandey

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

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
        (data from <https://en.wikipedia.org/wiki/Public_holidays_in_Chile>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>January 2nd, when falling on a Monday (since 2017)</li>
        <li>Good Friday</li>
        <li>Easter Saturday</li>
        <li>Labour Day, May 1st</li>
        <li>Navy Day, May 21st</li>
        <li>Day of Aboriginal People, around June 21st (observed on each Winter Solstice) (since 2021)</li>
        <li>Saint Peter and Saint Paul, June 29th (moved to the nearest Monday if it falls on a weekday)</li>
        <li>Our Lady of Mount Carmel, July 16th</li>
        <li>Assumption Day, August 15th</li>
        <li>Independence Day, September 18th (also the 17th if the latter falls on a Monday or Friday)</li>
        <li>Army Day, September 19th (also the 20th if the latter falls on a Friday)</li>
        <li>Discovery of Two Worlds, October 12th (moved to the nearest Monday if it falls on a weekday)</li>
        <li>Reformation Day, October 31st (since 2008; moved to the preceding Friday if it falls on a Tuesday,
            or to the following Friday if it falls on a Wednesday)</li>
        <li>All Saints' Day, November 1st</li>
        <li>Immaculate Conception, December 8th</li>
        <li>Christmas Day, December 25th</li>
        <li>New Year's Eve, December 31st; (see https://www.cmfchile.cl/portal/prensa/615/w3-article-49984.html)</li>
        </ul>

        \ingroup calendars
    */
    class Chile : public Calendar {
      private:
        class SseImpl final : public Calendar::WesternImpl {
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
