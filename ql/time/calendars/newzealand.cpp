/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/time/calendars/newzealand.hpp>

namespace QuantLib {

    NewZealand::NewZealand() {
        // all calendar instances share the same implementation instance
        static std::shared_ptr<Calendar::Impl> impl(new NewZealand::Impl);
        impl_ = impl;
    }

    bool NewZealand::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday or Tuesday)
            || ((d == 1 || (d == 3 && (w == Monday || w == Tuesday))) &&
                m == January)
            // Day after New Year's Day (possibly moved to Mon or Tuesday)
            || ((d == 2 || (d == 4 && (w == Monday || w == Tuesday))) &&
                m == January)
            // Anniversary Day, Monday nearest January 22nd
            || ((d >= 19 && d <= 25) && w == Monday && m == January)
            // Waitangi Day. February 6th ("Mondayised" since 2013)
            || (d == 6 && m == February)
            || ((d == 7 || d == 8) && w == Monday && m == February && y > 2013)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // ANZAC Day. April 25th ("Mondayised" since 2013) 
            || (d == 25 && m == April)
            || ((d == 26 || d == 27) && w == Monday && m == April && y > 2013)
            // Queen's Birthday, first Monday in June
            || (d <= 7 && w == Monday && m == June)
            // Labour Day, fourth Monday in October
            || ((d >= 22 && d <= 28) && w == Monday && m == October)
            // Christmas, December 25th (possibly Monday or Tuesday)
            || ((d == 25 || (d == 27 && (w == Monday || w == Tuesday)))
                && m == December)
            // Boxing Day, December 26th (possibly Monday or Tuesday)
            || ((d == 26 || (d == 28 && (w == Monday || w == Tuesday)))
                && m == December)
            // Matariki, it happens on Friday in June or July
            // official calendar released by the NZ government for the
            // next 30 years
            || (d == 20 && m == June && y == 2025)
            || (d == 21 && m == June && (y == 2030 || y == 2052))
            || (d == 24 && m == June && (y == 2022 || y == 2033 || y == 2044))
            || (d == 25 && m == June && (y == 2027 || y == 2038 || y == 2049))
            || (d == 28 && m == June && y == 2024)
            || (d == 29 && m == June && (y == 2035 || y == 2046))
            || (d == 30 && m == June && y == 2051)
            || (d == 2  && m == July && y == 2032)
            || (d == 3  && m == July && (y == 2043 || y == 2048))
            || (d == 6  && m == July && (y == 2029 || y == 2040))
            || (d == 7  && m == July && (y == 2034 || y == 2045))
            || (d == 10 && m == July && (y == 2026 || y == 2037))
            || (d == 11 && m == July && (y == 2031 || y == 2042))
            || (d == 14 && m == July && (y == 2023 || y == 2028))
            || (d == 15 && m == July && (y == 2039 || y == 2050))
            || (d == 18 && m == July && y == 2036)
            || (d == 19 && m == July && (y == 2041 || y == 2047)))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}
