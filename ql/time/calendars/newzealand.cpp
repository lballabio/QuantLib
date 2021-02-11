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
        static ext::shared_ptr<Calendar::Impl> impl(new NewZealand::Impl);
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
                && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}
