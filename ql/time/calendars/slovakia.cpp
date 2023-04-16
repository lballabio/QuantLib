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

#include <ql/time/calendars/slovakia.hpp>

namespace QuantLib {

    Slovakia::Slovakia(Market) {
        // all calendar instances share the same implementation instance
        static std::shared_ptr<Calendar::Impl> impl(new Slovakia::BsseImpl);
        impl_ = impl;
    }

    bool Slovakia::BsseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Epiphany
            || (d == 6 && m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // May Day
            || (d == 1 && m == May)
            // Liberation of the Republic
            || (d == 8 && m == May)
            // SS. Cyril and Methodius
            || (d == 5 && m == July)
            // Slovak National Uprising
            || (d == 29 && m == August)
            // Constitution of the Slovak Republic
            || (d == 1 && m == September)
            // Our Lady of the Seven Sorrows
            || (d == 15 && m == September)
            // All Saints Day
            || (d == 1 && m == November)
            // Freedom and Democracy of the Slovak Republic
            || (d == 17 && m == November)
            // Christmas Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // St. Stephen
            || (d == 26 && m == December)
            // unidentified closing days for stock exchange
            || (d >= 24 && d <= 31 && m == December && y == 2004)
            || (d >= 24 && d <= 31 && m == December && y == 2005))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

