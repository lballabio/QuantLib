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

#include <ql/time/calendars/poland.hpp>

namespace QuantLib {

    Poland::Poland() {
        // all calendar instances share the same implementation instance
        static std::shared_ptr<Calendar::Impl> impl(new Poland::Impl);
        impl_ = impl;
    }

    bool Poland::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // Easter Monday
            || (dd == em)
            // Corpus Christi
            || (dd == em+59)
            // New Year's Day
            || (d == 1  && m == January)
            // Epiphany
            || (d == 6  && m == January && y >= 2011)
            // May Day
            || (d == 1  && m == May)
            // Constitution Day
            || (d == 3  && m == May)
            // Assumption of the Blessed Virgin Mary
            || (d == 15  && m == August)
            // All Saints Day
            || (d == 1  && m == November)
            // Independence Day
            || (d ==11  && m == November)
            // Christmas
            || (d == 25 && m == December)
            // 2nd Day of Christmas
            || (d == 26 && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

