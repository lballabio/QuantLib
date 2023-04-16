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

#include <ql/time/calendars/czechrepublic.hpp>

namespace QuantLib {

    CzechRepublic::CzechRepublic(Market) {
        // all calendar instances share the same implementation instance
        static std::shared_ptr<Calendar::Impl> impl(
                                                  new CzechRepublic::PseImpl);
        impl_ = impl;
    }

    bool CzechRepublic::PseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
			// Good Friday
			|| (dd == em - 3 && y >= 2016)
            // Easter Monday
            || (dd == em)
            // Labour Day
            || (d == 1 && m == May)
            // Liberation Day
            || (d == 8 && m == May)
            // SS. Cyril and Methodius
            || (d == 5 && m == July)
            // Jan Hus Day
            || (d == 6 && m == July)
            // Czech Statehood Day
            || (d == 28 && m == September)
            // Independence Day
            || (d == 28 && m == October)
            // Struggle for Freedom and Democracy Day
            || (d == 17 && m == November)
            // Christmas Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // St. Stephen
            || (d == 26 && m == December)
            // unidentified closing days for stock exchange
            || (d == 2 && m == January && y == 2004)
            || (d == 31 && m == December && y == 2004))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

