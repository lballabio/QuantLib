/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Matthias Groncki

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

/*! \file thailand.hpp
    \brief Thailand calendars
*/

#ifndef quantlib_thailand_calendar_hpp
#define quantlib_thailand_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! %Thailand calendars
    /*! Holidays for the Thailand exchange
        Holidays observed by financial institutions (not to be confused with bank holidays in the United Kingdom) are regulated by the Bank of Thailand.
        If a holiday fall on a weekend the government will annouce a replacement day (usally the following monday).

        Sometimes the government add one or two extra holidays in a year.

        (data from
         https://www.bot.or.th/English/FinancialInstitutions/FIholiday/Pages/2018.aspx:
        Fixed holidays
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>Chakri Memorial Day, April 6th</li>
        <li>Songkran holiday, April 13th - 15th</li>
        <li>Labour Day, May 1st</li>
        <li>H.M. the King's Birthday, July 28th (from 2017)</li>
        <li>H.M. the Queen's Birthday, August 12th </li>
        <li>The Passing of H.M. the Late King Bhumibol Adulyadej (Rama IX), October 13th (from 2017) </li>
        <li>H.M. the Late King Bhumibol Adulyadej's Birthday, December 5th</li>
        <li>Constitution Day, December 10th</li>
        <li>New Year's Eve, December 31th</li>
        </ul>

        Other holidays for which no rule is given
        (data available for 2000-2018 with some years missing)
        <ul>
        <li>Makha Bucha Day</li>
        <li>Wisakha Bucha Day</li>
        <li>Buddhist Lent Day (until 2006)</li>
        <li>Asarnha Bucha Day (from 2007)</li>
        <li>Chulalongkorn Day</li>
        <li>Other special holidays</li>
        </ul>

        \ingroup calendars
    */
    class Thailand : public Calendar {
      private:
        class SetImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Thailand stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        Thailand();
    };

}


#endif


#ifndef id_87e3d31bb19710d65a73136d862b904c
#define id_87e3d31bb19710d65a73136d862b904c
inline bool test_87e3d31bb19710d65a73136d862b904c(const int* i) {
    return i != nullptr;
}
#endif
