
/*
 Copyright (C) 2005 Sercan Atalik

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

#include <ql/Calendars/istanbul.hpp>

namespace QuantLib {

    Turkey::Turkey() {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Turkey::Impl);
        impl_ = impl;
    }

    bool Turkey::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

		if ((w == Saturday || w == Sunday)
            // New Year's Day
            || (d == 1 && m == January)
			// 23 nisan / National Holiday
			|| (d == 23 && m == April)
			// 19 may/ National Holiday
			|| (d == 19 && m == May)
			// 30 aug/ National Holiday
			|| (d == 30 && m == August)
			///29 ekim  National Holiday
			|| (d == 29 && m == October))
            return false;

        // Local Holidays
        if (y == 2004) {
			// kurban
            if ((m == February && d <= 4)
			// ramazan
                || (m == November && d >= 14 && d <= 16))
                return false;
        } else if (y == 2005) {
			// kurban
            if ((m == January && d >= 19 && d <= 21)
			// ramazan
                || (m == November && d >= 2 && d <= 5))
                return false;
        } else if (y == 2006) {
			// kurban
            if ((m == January && d >= 9 && d <= 13)
			// ramazan
                || (m == October && d >= 23 && d <= 25)
            // kurban
                || (m == December && d >= 30))
                return false;
        } else if (y == 2007) {
			// kurban
            if ((m == January && d <= 4)
			// ramazan
                || (m == October && d >= 11 && d <= 14)
            // kurban
                || (m == December && d >= 19 && d <= 23))
                return false;
        } else if (y == 2008) {
            // ramazan
            if ((m == September && d >= 29)
                || (m == October && d <= 2)
                // kurban
                || (m == December && d >= 7 && d <= 11))
                return false;
        }
        return true;
    }

}


