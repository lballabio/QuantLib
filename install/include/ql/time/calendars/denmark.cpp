/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 StatPro Italia srl
 Copyright (C) 2022 Skandinaviska Enskilda Banken AB (publ)

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

#include <ql/time/calendars/denmark.hpp>

namespace QuantLib {

    Denmark::Denmark() {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new Denmark::Impl);
        impl_ = impl;
    }

    bool Denmark::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // Maunday Thursday
            || (dd == em-4)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // General Prayer Day
            || (dd == em+25 && y <= 2023)
            // Ascension
            || (dd == em+38)
            // Day after Ascension
            || (dd == em+39 && y >= 2009)
            // Whit Monday
            || (dd == em+49)
            // New Year's Day
            || (d == 1  && m == January)
            // Constitution Day, June 5th
            || (d == 5  && m == June)
            // Christmas Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Boxing Day
            || (d == 26 && m == December)
            // New Year's Eve
            || (d == 31 && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

