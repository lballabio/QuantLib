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

#include <ql/time/calendars/serbia.hpp>

namespace QuantLib {

    Serbia::Serbia(Market) {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new Serbia::BseImpl);
        impl_ = impl;
    }

    bool Serbia::BseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        // Serbian holidays
        if (isWeekend(w)
            // New Year
            || (d == 1 && m == January)
            || (d == 2 && m == January)
            // Serbian Orthodox Christmas
            || (d == 7 && m == January)
            // Serbian Statehood Day    
            || (d == 15 && m == February)
            || (d == 16 && m == February)
            // Serbian Statehood Day (observed) 
            // Serbia's Statehood Day (Sretenje) is officially observed on February 15 and February 16. 
            // However, since these dates fall on a Saturday and Sunday, the government designates Monday, 
            // February 17, as an additional non-working day. This practice ensures that citizens receive a weekday off 
            // when public holidays coincide with weekends.
            || ((d == 17 && m == February) && isWeekend(Date(15, February, y).weekday()) && isWeekend(Date(16, February, y).weekday()))
            // Good Friday
            || (dd == em - 3 && y >= 2016)
            // Easter Monday
            || (dd == em)
            // Labour Day
            || (d == 1 && m == May)
            // Armistice Day in World War I
            || (d == 11 && m == November)
            // Trading system maintenance, statistics and data migration
            || (d == 31 && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}