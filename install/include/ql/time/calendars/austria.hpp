/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file austria.hpp
    \brief Austrian calendars
*/

#ifndef quantlib_austria_calendar_hpp
#define quantlib_austria_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Austrian calendars
    /*! Public holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Epiphany, January 6th</li>
        <li>Easter Monday</li>
        <li>Ascension Thursday</li>
        <li>Whit Monday</li>
        <li>Corpus Christi</li>
        <li>Labour Day, May 1st</li>
        <li>Assumption Day, August 15th</li>
        <li>National Holiday, October 26th, since 1967</li>
        <li>All Saints Day, November 1st</li>
        <li>National Holiday, November 12th, 1919-1934</li>
        <li>Immaculate Conception Day, December 8th</li>
        <li>Christmas, December 25th</li>
        <li>St. Stephen, December 26th</li>
        </ul>

        Holidays for the stock exchange (data from https://www.wienerborse.at/en/trading/trading-information/trading-calendar/):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Whit Monday</li>
        <li>Labour Day, May 1st</li>
        <li>National Holiday, October 26th, since 1967</li>
        <li>National Holiday, November 12th, 1919-1934</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>St. Stephen, December 26th</li>
        <li>Exchange Holiday</li>
        </ul>

        \ingroup calendars
    */
    class Austria : public Calendar {
      private:
        class SettlementImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Austrian settlement"; }
            bool isBusinessDay(const Date&) const override;
        };
        class ExchangeImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Vienna stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        //! Austrian calendars
        enum Market { Settlement,     //!< generic settlement calendar
                      Exchange        //!< Vienna stock-exchange calendar
        };
        explicit Austria(Market market = Settlement);
    };

}


#endif
