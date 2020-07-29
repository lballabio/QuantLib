/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
Copyright (C) 2017 Francois Botha

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

#include <ql/time/calendars/botswana.hpp>

namespace QuantLib {

    Botswana::Botswana() {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new Botswana::Impl);
        impl_ = impl;
    }

    bool Botswana::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday or Tuesday)
            || ((d == 1 || (d == 2 && w == Monday) || (d == 3 && w == Tuesday)) 
                && m == January)
            // Good Friday
            || (dd == em - 3)
            // Easter Monday
            || (dd == em)
            // Labour Day, May 1st (possibly moved to Monday)
            || ((d == 1 || (d == 2 && w == Monday))
                && m == May)
            // Ascension
            || (dd == em + 38)
            // Sir Seretse Khama Day, July 1st (possibly moved to Monday)
            || ((d == 1 || (d == 2 && w == Monday))
                && m == July)
            // Presidents' Day (third Monday of July)
            || ((d >= 15 && d <= 21) && w == Monday && m == July)
            // Independence Day, September 30th (possibly moved to Monday)
            || ((d == 30 && m == September) || 
                (d == 1  && w == Monday && m == October))
            // Botswana Day, October 1st (possibly moved to Monday or Tuesday)
            || ((d == 1 || (d == 2 && w == Monday) || (d == 3 && w == Tuesday)) 
                && m == October)
            // Christmas
            || (d == 25 && m == December)
            // Boxing Day (possibly moved to Monday)
            || ((d == 26 || (d == 27 && w == Monday))
                && m == December)
            )
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

