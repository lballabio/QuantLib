
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Calendars/italy.hpp>

namespace QuantLib {

    Italy::Italy() {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Italy::Impl);
        impl_ = impl;
    }

    bool Italy::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if ((w == Saturday || w == Sunday)
            // New Year's Day
            || (d == 1 && m == January)
            // Epiphany
            || (d == 6 && m == January)
            // Easter Monday
            || (dd == em)
            // Liberation Day
            || (d == 25 && m == April)
            // Labour Day
            || (d == 1 && m == May)
            // Republic Day
            || (d == 2 && m == June && y >= 2000)
            // Assumption
            || (d == 15 && m == August)
            // All Saints' Day
            || (d == 1 && m == November)
            // Immaculate Conception
            || (d == 8 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // St. Stephen
            || (d == 26 && m == December)
            // December 31st, 1999 only
            || (d == 31 && m == December && y == 1999))
            return false;
        return true;
    }

}

