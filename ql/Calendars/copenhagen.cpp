/*
 Copyright (C) 2003 StatPro Italia s.r.l.

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

#include <ql/Calendars/copenhagen.hpp>

namespace QuantLib {

    Copenhagen::Copenhagen() {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Copenhagen::Impl);
        impl_ = impl;
    }

    bool Copenhagen::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if ((w == Saturday || w == Sunday)
            // Maunday Thursday
            || (dd == em-4)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // General Prayer Day
            || (dd == em+25)
            // Ascension 
            || (dd == em+38)
            // Whit Monday
            || (dd == em+49)
            // New Year's Day
            || (d == 1  && m == January)
            // Constitution Day, June 5th
            || (d == 5  && m == June)
            // Christmas
            || (d == 25 && m == December)
            // Boxing Day
            || (d == 26 && m == December))
            return false;
        return true;
    }

}

