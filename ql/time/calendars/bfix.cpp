/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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
// #include <ql/errors.hpp>
#include <ql/time/calendars/bfix.hpp>

namespace QuantLib {

    BFix::BFix() {
        // all calendar instances on the same market share the same
        // implementation instance
        impl_ = boost::make_shared<BFix::Impl>();
    }

    bool BFix::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);

        if (isWeekend(w)
            // New Year's Day
            || ((d == 1 || (d == 2 && w == Monday)) && m == January)
            // Good Friday
            || (dd == em - 3)
            // Christmas Day
            || ((d == 25 || (d == 26 && w == Monday)) && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}
