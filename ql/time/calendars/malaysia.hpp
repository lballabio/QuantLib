/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Lawrenz Law

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

/*! \file malaysia.hpp
    \brief Malaysian calendar
*/

#ifndef quantlib_malaysia_calendar_hpp
#define quantlib_malaysia_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Malaysian calendar
    /*! Holidays for the Kuala Lumpur stock exchange (Bursa Malaysia).

        Bursa Malaysia observes the federal public holidays gazetted for the
        Federal Territory of Kuala Lumpur, so state-only holidays (for example
        Good Friday in Sabah and Sarawak, or the Harvest Festival in Sabah and
        Labuan) are deliberately not included.

        Holidays with a fixed date:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Federal Territory Day, February 1st</li>
        <li>Labour Day, May 1st</li>
        <li>National Day (Hari Merdeka), August 31st</li>
        <li>Malaysia Day, September 16th (a federal holiday since 2010)</li>
        <li>Christmas Day, December 25th</li>
        </ul>

        Holidays with no fixed rule, tabulated for 2015 to 2027:
        <ul>
        <li>Chinese New Year (two days)</li>
        <li>Thaipusam</li>
        <li>Wesak Day</li>
        <li>Deepavali</li>
        <li>Nuzul Al-Quran</li>
        <li>Hari Raya Aidilfitri (two days)</li>
        <li>Hari Raya Haji</li>
        <li>Awal Muharram</li>
        <li>Maulidur Rasul</li>
        <li>Birthday of the Yang di-Pertuan Agong</li>
        <li>Substitute days granted when a gazetted holiday falls on a Sunday</li>
        <li>One-off federal holidays (royal installations, the 2018 general
            election, the 2017 SEA Games)</li>
        </ul>

        The Islamic, Chinese and Hindu festivals are set each year by
        proclamation rather than by calculation, so they are tabulated from the
        published holiday lists rather than computed. Outside the tabulated
        range only the fixed-date holidays above are returned.

        Holiday dates compiled from the gazetted Malaysian public holiday
        lists published at
        <https://www.officeholidays.com/countries/malaysia>, cross-checked for
        2021-2026 against Google's public Malaysia holiday calendar. They
        should be verified against the trading calendar published by Bursa
        Malaysia at <https://www.bursamalaysia.com/> before being relied upon.

        \ingroup calendars
    */
    class Malaysia : public Calendar {
      private:
        class KlseImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Kuala Lumpur stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };

      public:
        enum Market {
            KLSE //!< Kuala Lumpur stock exchange (Bursa Malaysia)
        };
        explicit Malaysia(Market m = KLSE);
    };

}

#endif
