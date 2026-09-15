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

/*! \file philippines.hpp
    \brief Philippine calendar
*/

#ifndef quantlib_philippines_calendar_hpp
#define quantlib_philippines_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Philippine calendar
    /*! Holidays for the Philippine Stock Exchange.

        The exchange is closed on both classes of holiday defined by the
        Labor Code, the regular holidays and the special (non-working) days,
        which are also the days on which banks supervised by the Bangko
        Sentral ng Pilipinas are closed. The list additionally contains a few
        dates on which the exchange alone suspended trading, so it is not a
        banking calendar.

        Holidays with a fixed date or a computable rule:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Maundy Thursday</li>
        <li>Good Friday</li>
        <li>Araw ng Kagitingan (Day of Valor), April 9th</li>
        <li>Labor Day, May 1st</li>
        <li>Independence Day, June 12th</li>
        <li>Ninoy Aquino Day, August 21st (a special non-working day since
            RA 9256 of 2004)</li>
        <li>National Heroes Day, the last Monday of August</li>
        <li>All Saints' Day, November 1st</li>
        <li>Bonifacio Day, November 30th</li>
        <li>Feast of the Immaculate Conception, December 8th (a special
            non-working day since RA 10966 of 2017)</li>
        <li>Christmas Day, December 25th</li>
        <li>Rizal Day, December 30th</li>
        <li>Last Day of the Year, December 31st</li>
        </ul>

        Black Saturday is proclaimed a special (non-working) day every year,
        but by construction it always falls on a Saturday and so is already
        covered by the weekend rule.

        Holidays with no fixed rule, tabulated for 2020 to 2027:
        <ul>
        <li>Eid'l Fitr</li>
        <li>Eid'l Adha</li>
        <li>Chinese New Year (a special non-working day since 2012)</li>
        <li>All Souls' Day and Christmas Eve, which are proclaimed in most
            but not all years</li>
        <li>EDSA People Power Revolution Anniversary, a special non-working
            day up to 2023 and a special working day from 2025</li>
        <li>Election days</li>
        <li>One-off additional special non-working days, and the two days on
            which the exchange was closed by the COVID-19 trading suspension
            of March 2020</li>
        </ul>

        A handful of the rule-based holidays above were transferred to another
        date by proclamation (Bonifacio Day in 2023, Ninoy Aquino Day in 2024)
        or downgraded to a special working day (the Last Day of the Year in
        2021); those years are handled as explicit exceptions.

        The Islamic festivals, Chinese New Year and the additional special
        days are set each year by presidential proclamation rather than by
        rule, so they are tabulated from the published proclamations rather
        than computed. Outside the tabulated range only the rule-based
        holidays above are returned. The 2027 proclamation had not been
        issued when this calendar was written, so the 2027 entries cover only
        the festivals whose dates can be predicted (Chinese New Year and the
        two Islamic festivals, the latter still subject to moon sighting);
        any additional special (non-working) day declared for that year is
        missing. The rules also predate the
        "holiday economics" transfers of RA 9492, so years before 2011 are
        not reliable.

        Holiday dates compiled from the presidential proclamations published
        by the Official Gazette and the Presidential Communications Office
        (Proclamations 845 s.2019, 986 and 1107 s.2021, 1236 s.2021, 1357
        s.2022, 42 and 90 s.2022, 167 and 425 s.2023, 368 and 665 s.2024,
        727 s.2024 and 1006 s.2025), cross-checked against
        <https://www.officeholidays.com/countries/philippines>. They should
        be verified against the trading calendar published by the Philippine
        Stock Exchange at <https://www.pse.com.ph/> before being relied upon.

        \ingroup calendars
    */
    class Philippines : public Calendar {
      private:
        class PseImpl final : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Philippine stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };

      public:
        enum Market {
            PSE //!< Philippine Stock Exchange
        };
        explicit Philippines(Market m = PSE);
    };

}

#endif
