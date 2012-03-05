/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano

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

/*! \file germany.hpp
    \brief German calendars
*/

#ifndef quantlib_germany_calendar_hpp
#define quantlib_germany_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! German calendars
    /*! Public holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Ascension Thursday</li>
        <li>Whit Monday</li>
        <li>Corpus Christi</li>
        <li>Labour Day, May 1st</li>
        <li>National Day, October 3rd</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Boxing Day, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        Holidays for the Frankfurt Stock exchange
        (data from http://deutsche-boerse.com/):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Christmas' Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Christmas Holiday, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        Holidays for the Xetra exchange
        (data from http://deutsche-boerse.com/):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Christmas' Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Christmas Holiday, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        Holidays for the Eurex exchange
        (data from http://www.eurexchange.com/index.html):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Christmas' Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Christmas Holiday, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        Holidays for the Euwax exchange
        (data from http://www.boerse-stuttgart.de):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Whit Monday</li>
        <li>Christmas' Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Christmas Holiday, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        \ingroup calendars

        \test the correctness of the returned results is tested
              against a list of known holidays.
    */
    class Germany : public Calendar {
      private:
        class SettlementImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "German settlement"; }
            bool isBusinessDay(const Date&) const;
        };
        class FrankfurtStockExchangeImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Frankfurt stock exchange"; }
            bool isBusinessDay(const Date&) const;
        };
        class XetraImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Xetra"; }
            bool isBusinessDay(const Date&) const;
        };
        class EurexImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Eurex"; }
            bool isBusinessDay(const Date&) const;
        };
        class EuwaxImpl : public Calendar::WesternImpl {
        public:
            std::string name() const { return "Euwax"; }
            bool isBusinessDay(const Date&) const;
        };

      public:
        //! German calendars
        enum Market { Settlement,             //!< generic settlement calendar
                      FrankfurtStockExchange, //!< Frankfurt stock-exchange
                      Xetra,                  //!< Xetra
                      Eurex,                  //!< Eurex
                      Euwax                   //!< Euwax
        };
        Germany(Market market = FrankfurtStockExchange);
    };

}


#endif
