/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Piter Dias

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Calendars/brazil.hpp>

namespace QuantLib {

    Brazil::Brazil() {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Brazil::Impl);
        impl_ = impl;
    }

    bool Brazil::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day dd = date.dayOfYear();
        Day em = easterMonday(y);

        if ((w == Saturday || w == Sunday)
            // New Year's Day
            || (d == 1 && m == January)
            // Tiradentes Day
            || (d == 21 && m == April)
            // Labor Day
            || (d == 1 && m == May)
            // Independence Day
            || (d == 7 && m == September)
            // Nossa Sra. Aparecida Day
            || (d == 12 && m == October)
            // Dead Day
            || (d == 2 && m == November)
            // Republic Day
            || (d == 15 && m == November)
            // Christmas
            || (d == 25 && m == December)
            // Passion of Christ
            || (dd == em-3)
            // Carnival
            || (dd == em-49 || dd == em-48)
            // Corpus Christi
            || (dd == em+59)
            )
            return false;
        return true;
    }

}

