/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 SoftSolution srl

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

#include <ql/errors.hpp>
#include <ql/time/calendars/islamicholidays.hpp>
#include <ql/time/calendars/uzbekistan.hpp>

namespace QuantLib {

    Uzbekistan::Uzbekistan(Market) {
        static ext::shared_ptr<Calendar::Impl> impl(new Uzbekistan::Impl);
        impl_ = impl;
    }

    bool Uzbekistan::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();

        if (isWeekend(w)
            || MoonSightingMethod::isEidAlFitr(date)
            || MoonSightingMethod::isEidAlAdha(date)
            // New Year's Day
            || (d == 1 && m == January)
            // International Womens Day
            || (d == 8 && m == March)
            // Navruz(Persian New Year)
            || (d == 21 && m == March)
            // Day of Remembrance and Honors
            || (d == 9 && m == May)
            // Independence Day
            || (d == 1 && m == September)
            // Teachers Day
            || (d == 1 && m == October)
            // Constitution Day
            || (d == 8 && m == December))
            return false;
        return true;
    }

}
