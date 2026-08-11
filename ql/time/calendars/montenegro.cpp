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

#include <ql/time/calendars/montenegro.hpp>

namespace QuantLib {

    Montenegro::Montenegro(Market) {
        static ext::shared_ptr<Calendar::Impl> impl(new Montenegro::MnseImpl);
        impl_ = impl;
    }

    bool Montenegro::MnseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // New Year Holiday
            || (d == 2 && m == January)
            // Labour Day
            || (d == 1 && m == May)
            // Labour Day Holiday
            || (d == 2 && m == May)
            // Independence Day
            || (d == 21 && m == May)
            // Independence Day Holiday
            || (d == 22 && m == May)
            // Statehood Day
            || (d == 13 && m == July)
            // Statehood Day Holiday
            || (d == 14 && m == July)
            // Njegos Day
            || (d == 13 && m == November)
            // Njegos Day Holiday
            || (d == 14 && m == November))
            return false;
        return true;
    }

}