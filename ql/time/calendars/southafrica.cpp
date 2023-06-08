/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2023 Skandinaviska Enskilda Banken AB (publ)

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

#include <ql/time/calendars/southafrica.hpp>

namespace QuantLib {

    SouthAfrica::SouthAfrica() {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new SouthAfrica::Impl);
        impl_ = impl;
    }

    bool SouthAfrica::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday)
            || ((d == 1 || (d == 2 && w == Monday)) && m == January)
            // Good Friday
            || (dd == em-3)
            // Family Day
            || (dd == em)
            // Human Rights Day, March 21st (possibly moved to Monday)
            || ((d == 21 || (d == 22 && w == Monday))
                && m == March)
            // Freedom Day, April 27th (possibly moved to Monday)
            || ((d == 27 || (d == 28 && w == Monday))
                && m == April)
            // Election Day, April 14th 2004
            || (d == 14 && m == April && y == 2004)
            // Workers Day, May 1st (possibly moved to Monday)
            || ((d == 1 || (d == 2 && w == Monday))
                && m == May)
            // Youth Day, June 16th (possibly moved to Monday)
            || ((d == 16 || (d == 17 && w == Monday))
                && m == June)
            // National Women's Day, August 9th (possibly moved to Monday)
            || ((d == 9 || (d == 10 && w == Monday))
                && m == August)
            // Heritage Day, September 24th (possibly moved to Monday)
            || ((d == 24 || (d == 25 && w == Monday))
                && m == September)
            // Day of Reconciliation, December 16th
            // (possibly moved to Monday)
            || ((d == 16 || (d == 17 && w == Monday))
                && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Day of Goodwill (possibly moved to Monday)
            || ((d == 26 || (d == 27 && w == Monday))
                && m == December)
            // one-shot: Election day 2009
            || (d == 22 && m == April && y == 2009)
            // one-shot: Election day 2016
            || (d == 3 && m == August && y == 2016)
            // one-shot: Election day 2021
            || (d == 1 && m == November && y == 2021)
            // one-shot: In lieu of Christmas falling on Sunday in 2022
            || (d == 27 && m == December && y == 2022)
            )
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

