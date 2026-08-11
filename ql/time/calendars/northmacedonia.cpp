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

#include <ql/time/calendars/islamicholidays.hpp>
#include <ql/time/calendars/northmacedonia.hpp>

namespace QuantLib {

    NorthMacedonia::NorthMacedonia(Market) {
        static ext::shared_ptr<Calendar::Impl> impl(new NorthMacedonia::MseImpl);
        impl_ = impl;
    }

    bool NorthMacedonia::MseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);

        if (isWeekend(w)
            || MoonSightingMethod::isEidAlFitr(date)
            || MoonSightingMethod::isEidAlAdha(date)
            // New Year
            || (d == 1 && m == January)
            // Orthodox Christmas
            || (d == 7 && m == January)
            // Easter Monday
            || (dd == em)
            // Labour Day
            || (d == 1 && m == May)
            // Saints Cyril and Methodius Day
            || (d == 24 && m == May)
            // Republic Day
            || (d == 2 && m == August)
            // Independence Day
            || (d == 8 && m == September)
            // Day of Peopleâ€™s Uprising
            || (d == 11 && m == October)
            // Day of the Macedonian Revolutionary Struggle
            || (d == 23 && m == October)
            // Saint Clement of Ohrid Day,
            || (d == 8 && m == December))
            return false;
        return true;
    }

}
