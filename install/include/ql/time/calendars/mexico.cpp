/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl
 Copyright (C) 2024 Skandinaviska Enskilda Banken AB (publ)

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

#include <ql/time/calendars/mexico.hpp>

namespace QuantLib {

    Mexico::Mexico(Market) {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new Mexico::BmvImpl);
        impl_ = impl;
    }

    bool Mexico::BmvImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Constitution Day
            || (y <= 2005 && d == 5 && m == February)
            || (y >= 2006 && d <= 7 && w == Monday && m == February)
            // Birthday of Benito Juarez
            || (y <= 2005 && d == 21 && m == March)
            || (y >= 2006 && (d >= 15 && d <= 21) && w == Monday && m == March)
            // Holy Thursday
            || (dd == em-4)
            // Good Friday
            || (dd == em-3)
            // Labour Day
            || (d == 1 && m == May)
            // National Day
            || (d == 16 && m == September)
            // Inauguration Day
            || (d == 1 && m == October && y >= 2024 && (y - 2024) % 6 == 0)
            // All Souls Day
            || (d == 2 && m == November)
            // Revolution Day
            || (y <= 2005 && d == 20 && m == November)
            || (y >= 2006 && (d >= 15 && d <= 21) && w == Monday && m == November)
            // Our Lady of Guadalupe
            || (d == 12 && m == December)
            // Christmas
            || (d == 25 && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

