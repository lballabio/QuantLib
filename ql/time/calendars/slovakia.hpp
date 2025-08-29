/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file slovakia.hpp
    \brief Slovak calendars
*/

#ifndef quantlib_slovak_calendar_hpp
#define quantlib_slovak_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Slovak calendars
    /*! Holidays for the Bratislava stock exchange
        (data from <http://www.bsse.sk/>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Epiphany, January 6th</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>May Day, May 1st</li>
        <li>Liberation of the Republic, May 8th</li>
        <li>SS. Cyril and Methodius, July 5th</li>
        <li>Slovak National Uprising, August 29th</li>
        <li>Constitution of the Slovak Republic, September 1st</li>
        <li>Our Lady of the Seven Sorrows, September 15th</li>
        <li>All Saints Day, November 1st</li>
        <li>Freedom and Democracy of the Slovak Republic, November 17th</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>St. Stephen, December 26th</li>
        </ul>

        \ingroup calendars
    */
    class Slovakia : public Calendar {
      private:
        class BsseImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Bratislava stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        enum Market { BSSE    //!< Bratislava stock exchange
        };
        Slovakia(Market m = BSSE);
    };

}


#endif
