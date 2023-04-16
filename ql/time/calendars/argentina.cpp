/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/time/calendars/argentina.hpp>

namespace QuantLib {

    Argentina::Argentina(Market) {
        // all calendar instances share the same implementation instance
        static std::shared_ptr<Calendar::Impl> impl(
                                                   new Argentina::MervalImpl);
        impl_ = impl;
    }

    bool Argentina::MervalImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Holy Thursday
            || (dd == em-4)
            // Good Friday
            || (dd == em-3)
            // Labour Day
            || (d == 1 && m == May)
            // May Revolution
            || (d == 25 && m == May)
            // Death of General Manuel Belgrano
            || (d >= 15 && d <= 21 && w == Monday && m == June)
            // Independence Day
            || (d == 9 && m == July)
            // Death of General José de San Martín
            || (d >= 15 && d <= 21 && w == Monday && m == August)
            // Columbus Day
            || ((d == 10 || d == 11 || d == 12 || d == 15 || d == 16)
                && w == Monday && m == October)
            // Immaculate Conception
            || (d == 8 && m == December)
            // Christmas Eve
            || (d == 24 && m == December)
            // New Year's Eve
            || ((d == 31 || (d == 30 && w == Friday)) && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

