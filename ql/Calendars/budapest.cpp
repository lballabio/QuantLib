
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

#include <ql/Calendars/budapest.hpp>

namespace QuantLib {

    bool Budapest::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if ((w == Saturday || w == Sunday)
            // Easter Monday
            || (dd == em)
            // Whit Monday
            || (dd == em+49)
            // New Year's Day
            || (d == 1  && m == January)
            // National Day
            || (d == 15  && m == March)
            // Labour Day
            || (d == 1  && m == May)
            // Constitution Day
            || (d == 20  && m == August)
            // Republic Day
            || (d == 23  && m == October)
            // All Saints Day
            || (d == 1  && m == November)
            // Christmas
            || (d == 25 && m == December)
            // 2nd Day of Christmas
            || (d == 26 && m == December))
            return false;
        return true;
    }

}

