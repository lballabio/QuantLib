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

/*! \file unitedkingdom.hpp
    \brief UK calendars
*/

#ifndef quantlib_united_kingdom_calendar_hpp
#define quantlib_united_kingdom_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! United Kingdom calendars
    /*! Repeating Public holidays (data from https://www.gov.uk/bank-holidays):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Early May Bank Holiday, first Monday of May</li>
        <li>Spring Bank Holiday, last Monday of May</li>
        <li>Summer Bank Holiday, last Monday of August</li>
        <li>Christmas Day, December 25th (possibly moved to Monday or
            Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>

        Holidays for the stock exchange:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Early May Bank Holiday, first Monday of May</li>
        <li>Spring Bank Holiday, last Monday of May</li>
        <li>Summer Bank Holiday, last Monday of August</li>
        <li>Christmas Day, December 25th (possibly moved to Monday or
            Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>

        Holidays for the metals exchange:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Early May Bank Holiday, first Monday of May</li>
        <li>Spring Bank Holiday, last Monday of May</li>
        <li>Summer Bank Holiday, last Monday of August</li>
        <li>Christmas Day, December 25th (possibly moved to Monday or
            Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>

        Note that there are some one-off holidays not listed above.
        See the implementation for the complete list.

        \ingroup calendars

        \todo add LIFFE

        \test the correctness of the returned results is tested
              against a list of known holidays.
    */
    class UnitedKingdom : public Calendar {
      private:
        class SettlementImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "UK settlement"; }
            bool isBusinessDay(const Date&) const override;
        };
        class ExchangeImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "London stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
        class MetalsImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "London metals exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        //! UK calendars
        enum Market { Settlement,     //!< generic settlement calendar
                      Exchange,       //!< London stock-exchange calendar
                      Metals          //|< London metals-exchange calendar
        };
        UnitedKingdom(Market market = Settlement);
    };

}


#endif
