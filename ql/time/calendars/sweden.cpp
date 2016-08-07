/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/time/calendars/sweden.hpp>

namespace QuantLib {

    Sweden::Sweden() {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Sweden::Impl);
        impl_ = impl;
    }

    bool Sweden::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // Ascension Thursday
            || (dd == em+38)
            // Whit Monday (till 2004)
            || (dd == em+49 && y < 2005)
            // New Year's Day
            || (d == 1  && m == January)
            // Epiphany
            || (d == 6  && m == January)
            // May Day
            || (d == 1  && m == May)
            // June 6 id National Day but is not a holiday.
            // It has been debated wheter or not this day should be
            // declared as a holiday.
            // As of 2002 the Stockholmborsen is open that day
            // || (d == 6  && m == June)
            // Midsummer Eve (Friday between June 19-25)
            || (w == Friday && (d >= 19 && d <= 25) && m == June)
            // Christmas Eve
            || (d == 24 && m == December)
            // Christmas Day
            || (d == 25 && m == December)
            // Boxing Day
            || (d == 26 && m == December)
            // New Year's Eve
            || (d == 31 && m == December))
            return false;
        return true;
    }

}

