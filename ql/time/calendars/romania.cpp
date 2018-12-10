/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 StatPro Italia srl
 Copyright (C) 2015 Riccardo Barone
 Copyright (C) 2018 Matthias Lungwitz

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

#include <ql/time/calendars/romania.hpp>

namespace QuantLib {

    Romania::Romania() {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new Romania::Impl);
        impl_ = impl;
    }


    bool Romania::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Day after New Year's Day
            || (d == 2 && m == January)
            // Unification Day
            || (d == 24 && m == January)
            // Orthodox Easter Monday
            || (dd == em)
            // Labour Day
            || (d == 1 && m == May)
            // Pentecost
            || (dd == em+49)
            // Children's Day (since 2017)
            || (d == 1 && m == June && y >= 2017)
            // St Marys Day
            || (d == 15 && m == August)
            // Feast of St Andrew
            || (d == 30 && m == November)
            // National Day
            || (d == 1 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // 2nd Day of Chritsmas
            || (d == 26 && m == December))
            return false;
        return true;
    }

}
