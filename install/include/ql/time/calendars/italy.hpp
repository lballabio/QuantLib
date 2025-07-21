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

/*! \file italy.hpp
    \brief Italian calendars
*/

#ifndef quantlib_italy_calendar_hpp
#define quantlib_italy_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Italian calendars
    /*! Public holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Epiphany, January 6th</li>
        <li>Easter Monday</li>
        <li>Liberation Day, April 25th</li>
        <li>Labour Day, May 1st</li>
        <li>Republic Day, June 2nd (since 2000)</li>
        <li>Assumption, August 15th</li>
        <li>All Saint's Day, November 1st</li>
        <li>Immaculate Conception Day, December 8th</li>
        <li>Christmas Day, December 25th</li>
        <li>St. Stephen's Day, December 26th</li>
        </ul>

        Holidays for the stock exchange (data from http://www.borsaitalia.it):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Assumption, August 15th</li>
        <li>Christmas' Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>St. Stephen, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        \ingroup calendars

        \test the correctness of the returned results is tested against a
              list of known holidays.
    */
    class Italy : public Calendar {
      private:
        class SettlementImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Italian settlement"; }
            bool isBusinessDay(const Date&) const override;
        };
        class ExchangeImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Milan stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        //! Italian calendars
        enum Market { Settlement,     //!< generic settlement calendar
                      Exchange        //!< Milan stock-exchange calendar
        };
        Italy(Market market = Settlement);
    };

}


#endif
