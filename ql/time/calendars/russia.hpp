/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 StatPro Italia srl

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

/*! \file russia.hpp
    \brief Russian calendar
*/

#ifndef quantlib_russia_calendar_hpp
#define quantlib_russia_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Russian calendars
    /*! Public holidays (see <http://www.cbr.ru/eng/>:):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year holidays, January 1st to 5th (only 1st and 2nd
            until 2005)</li>
        <li>Christmas, January 7th (possibly moved to Monday)</li>
        <li>Defender of the Fatherland Day, February 23rd (possibly
            moved to Monday)</li>
        <li>International Women's Day, March 8th (possibly moved to
            Monday)</li>
        <li>Labour Day, May 1st (possibly moved to Monday)</li>
        <li>Victory Day, May 9th (possibly moved to Monday)</li>
        <li>Russia Day, June 12th (possibly moved to Monday)</li>
        <li>Unity Day, November 4th (possibly moved to Monday)</li>
        </ul>

        Holidays for the Moscow Exchange (MOEX) taken from
        <http://moex.com/s726> and related pages.  These holidays are
        <em>not</em> consistent year-to-year, may or may not correlate
        to public holidays, and are only available for dates since the
        introduction of the MOEX 'brand' (a merger of the stock and
        futures markets).

        \ingroup calendars
    */
    class Russia : public Calendar {
      private:
        class SettlementImpl : public Calendar::OrthodoxImpl {
          public:
            std::string name() const override { return "Russian settlement"; }
            bool isBusinessDay(const Date&) const override;
        };
        class ExchangeImpl : public Calendar::OrthodoxImpl {
          public:
            std::string name() const override { return "Moscow exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        //! Russian calendars
        enum Market { Settlement, //!< generic settlement calendar
                      MOEX        //!< Moscow Exchange calendar
        };
        Russia(Market = Settlement);
    };

}


#endif


#ifndef id_208d04bfb40ae9f02aca3b220a041de6
#define id_208d04bfb40ae9f02aca3b220a041de6
inline bool test_208d04bfb40ae9f02aca3b220a041de6(int* i) { return i != 0; }
#endif
