
/*
 Copyright (C) 2004 FIMAT Group

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

#include <ql/Calendars/beijing.hpp>

namespace QuantLib {

    Beijing::Beijing() {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Beijing::Impl);
        impl_ = impl;
    }

    bool Beijing::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();

        if ((w == Saturday || w == Sunday)
            // New Year's Day
            || (d == 1 && m == January)
            // Labor Day
            || (d >= 1 && d <= 7 && m == May)
            // National Day
            || (d >= 1 && d <= 7 && m == October)
				
            // Lunar New Year 2004
            || (d >= 22 && d <= 28 && m == January && y==2004)
            )
            return false;
        return true;
    }

}

