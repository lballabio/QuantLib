/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/time/calendars/hungary.hpp>

namespace QuantLib {

    Hungary::Hungary() {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new Hungary::Impl);
        impl_ = impl;
    }

    bool Hungary::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // Good Friday (since 2017)
            || (dd == em - 3 && y >= 2017)
            // Easter Monday
            || (dd == em)
            // Whit Monday
            || (dd == em+49)
            // New Year's Day
            || (d == 1  && m == January)
            // National Day
            || (d == 15  && m == March)
            // Labour Day
            || (d == 1  && m == May)
            // Constitution Day
            || (d == 20  && m == August)
            // Republic Day
            || (d == 23  && m == October)
            // All Saints Day
            || (d == 1  && m == November)
            // Christmas
            || (d == 25 && m == December)
            // 2nd Day of Christmas
            || (d == 26 && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

