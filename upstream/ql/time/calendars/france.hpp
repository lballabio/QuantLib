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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file france.hpp
    \brief French calendars
*/

#ifndef quantlib_france_calendar_hpp
#define quantlib_france_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! French calendars
    /*! Public holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Armistice 1945, May 8th</li>
        <li>Ascension, May 10th</li>
        <li>Pentecôte, May 21st</li>
        <li>Fête nationale, July 14th</li>
        <li>Assumption, August 15th</li>
        <li>All Saint's Day, November 1st</li>
        <li>Armistice 1918, November 11th</li>
        <li>Christmas Day, December 25th</li>
        </ul>

        Holidays for the stock exchange (data from https://www.stockmarketclock.com/exchanges/euronext-paris/market-holidays/):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas Day, December 25th</li>
        <li>Boxing Day, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        \ingroup calendars
    */
    class France : public Calendar {
      private:
        class SettlementImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "French settlement"; }
            bool isBusinessDay(const Date&) const override;
        };
        class ExchangeImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Paris stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        //! French calendars
        enum Market { Settlement,     //!< generic settlement calendar
                      Exchange        //!< Paris stock-exchange calendar
        };
        explicit France(Market market = Settlement);
    };

}


#endif
