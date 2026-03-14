/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file canada.hpp
    \brief Canadian calendar
*/

#ifndef quantlib_canadian_calendar_hpp
#define quantlib_canadian_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Canadian calendar
    /*! Banking holidays
        (data from <http://www.bankofcanada.ca/en/about/holiday.html>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Family Day, third Monday of February (since 2008)</li>
        <li>Good Friday</li>
        <li>Victoria Day, the Monday on or preceding May 24th</li>
        <li>Canada Day, July 1st (possibly moved to Monday)</li>
        <li>Provincial Holiday, first Monday of August</li>
        <li>Labour Day, first Monday of September</li>
        <li>National Day for Truth and Reconciliation, September 30th (possibly moved to Monday)</li>
        <li>Thanksgiving Day, second Monday of October</li>
        <li>Remembrance Day, November 11th (possibly moved to Monday)</li>
        <li>Christmas, December 25th (possibly moved to Monday or Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>

        Holidays for the Toronto stock exchange
        (data from <http://www.tsx.com/en/about_tsx/market_hours.html>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Family Day, third Monday of February (since 2008)</li>
        <li>Good Friday</li>
        <li>Victoria Day, the Monday on or preceding May 24th</li>
        <li>Canada Day, July 1st (possibly moved to Monday)</li>
        <li>Provincial Holiday, first Monday of August</li>
        <li>Labour Day, first Monday of September</li>
        <li>Thanksgiving Day, second Monday of October</li>
        <li>Christmas, December 25th (possibly moved to Monday or Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>

        \ingroup calendars
    */
    class Canada : public Calendar {
      private:
        class SettlementImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Canada"; }
            bool isBusinessDay(const Date&) const override;
        };
        class TsxImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "TSX"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        enum Market { Settlement,       //!< generic settlement calendar
                      TSX               //!< Toronto stock exchange calendar
        };
        Canada(Market market = Settlement);
    };

}


#endif
