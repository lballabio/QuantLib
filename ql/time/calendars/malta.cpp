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
#include <ql/time/calendars/malta.hpp>

namespace QuantLib {

    Malta::Malta(Market) {
        static ext::shared_ptr<Calendar::Impl> impl(new Malta::MseImpl);
        impl_ = impl;
    }

    bool Malta::MseImpl::isWeekend(Weekday w) const {
        return w == Friday || w == Saturday;
    }

    bool Malta::MseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        Day gf = em - 3; // Good Friday


        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // St. Paul's Shipwreck
            || (d == 10 && m == February)
            // St. Joseph's Day
            || (d == 19 && m == March)
            // Freedom Day
            || (d == 31 && m == March)
            // Good Friday
            || (date.dayOfYear() == gf)
            // Easter Monday (exchange holiday)
            || (date.dayOfYear() == em)
            // Labour Day
            || (d == 1 && m == May)
            // Imnarja (Feast of Saints Peter & Paul)
            || (d == 29 && m == June)
            // Assumption of Mary
            || (d == 15 && m == August)
            // Our Lady of Victories (Nativity of Mary)
            || (d == 8 && m == September)
            // Independence Day
            || (d == 21 && m == September)
            // Immaculate Conception
            || (d == 8 && m == December)
            // Republic Day
            || (d == 13 && m == December)
            // Christmas Vigil
            || (d == 24 && m == December)
            // Christmas Day
            || (d == 25 && m == December)
            // Boxing Day (occasionally observed by MSE)
            || (d == 26 && m == December)
            // New Year's Eve (non-trading)
            || (d == 31 && m == December))
            return false;
        return true;
    }

}
