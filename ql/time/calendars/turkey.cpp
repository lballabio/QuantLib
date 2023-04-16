/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Sercan Atalik
 Copyright (C) 2010 StatPro Italia srl
 Copyright (C) 2018 Matthias Lungwitz
 Copyright (C) 2022 Skandinaviska Enskilda Banken AB (publ)

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

#include <ql/time/calendars/turkey.hpp>

namespace QuantLib {

    Turkey::Turkey() {
        // all calendar instances share the same implementation instance
        static std::shared_ptr<Calendar::Impl> impl(new Turkey::Impl);
        impl_ = impl;
    }

    bool Turkey::Impl::isWeekend(Weekday w) const {
        return w == Saturday || w == Sunday;
    }

    bool Turkey::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // 23 nisan / National Holiday
            || (d == 23 && m == April)
            // 1 may/ National Holiday
            || (d == 1 && m == May)
            // 19 may/ National Holiday
            || (d == 19 && m == May)
            // 15 july / National Holiday (since 2017)
            || (d == 15 && m == July && y >= 2017)
            // 30 aug/ National Holiday
            || (d == 30 && m == August)
            ///29 ekim  National Holiday
            || (d == 29 && m == October))
            return false;

        // Local Holidays
        if (y == 2004) {
            // Kurban
            if ((m == February && d <= 4)
            // Ramadan
                || (m == November && d >= 14 && d <= 16))
                return false;
        } else if (y == 2005) {
            // Kurban
            if ((m == January && d >= 19 && d <= 21)
            // Ramadan
                || (m == November && d >= 2 && d <= 5))
                return false;
        } else if (y == 2006) {
            // Kurban
            if ((m == January && d >= 10 && d <= 13)
            // Ramadan
                || (m == October && d >= 23 && d <= 25)
            // Kurban
                || (m == December && d == 31))
                return false;
        } else if (y == 2007) {
            // Kurban
            if ((m == January && d <= 3)
            // Ramadan
                || (m == October && d >= 12 && d <= 14)
            // Kurban
                || (m == December && d >= 20 && d <= 23))
                return false;
        } else if (y == 2008) {
            // Ramadan
            if ((m == September && d == 30)
                || (m == October && d <= 2)
            // Kurban
                || (m == December && d >= 8 && d <= 11))
                return false;
        } else if (y == 2009) {
            // Ramadan
            if ((m == September && d >= 20 && d <= 22)
            // Kurban
                || (m == November && d >= 27 && d <= 30))
                return false;
        } else if (y == 2010) {
            // Ramadan
            if ((m == September && d >= 9 && d <= 11)
            // Kurban
                || (m == November && d >= 16 && d <= 19))
                return false;
        } else if (y == 2011) {
            // not clear from borsainstanbul.com
            if ((m == October && d == 1)
                || (m == November && d >= 9 && d <= 13))
                return false;
        } else if (y == 2012) {
            // Ramadan
            if ((m == August && d >= 18 && d <= 21)
            // Kurban
                || (m == October && d >= 24 && d <= 28))
                return false;
        } else if (y == 2013) {
            // Ramadan
            if ((m == August && d >= 7 && d <= 10)
            // Kurban
                || (m == October && d >= 14 && d <= 18)
            // additional holiday for Republic Day
                || (m == October && d == 28))
                return false;
        } else if (y == 2014) {
            // Ramadan
            if ((m == July && d >= 27 && d <= 30)
            // Kurban
                || (m == October && d >= 4 && d <= 7)
            // additional holiday for Republic Day
                || (m == October && d == 29))
                return false;
		} else if (y == 2015) {
			// Ramadan
			if ((m == July && d >= 17 && d <= 19)
				// Kurban
				|| (m == October && d >= 24 && d <= 27))
				return false;
		} else if (y == 2016) {
			// Ramadan
			if ((m == July && d >= 5 && d <= 7)
				// Kurban
				|| (m == September && d >= 12 && d <= 15))
				return false;
		} else if (y == 2017) {
			// Ramadan
			if ((m == June && d >= 25 && d <= 27)
				// Kurban
				|| (m == September && d >= 1 && d <= 4))
				return false;
		} else if (y == 2018) {
			// Ramadan
			if ((m == June && d >= 15 && d <= 17)
				// Kurban
				|| (m == August && d >= 21 && d <= 24))
				return false;
		} else if (y == 2019) {
			// Ramadan
			if ((m == June && d >= 4 && d <= 6)
				// Kurban
				|| (m == August && d >= 11 && d <= 14))
				return false;
		} else if (y == 2020) {
			// Ramadan
			if ((m == May && d >= 24 && d <= 26)
				// Kurban
				|| (m == July && d == 31) || (m == August && d >= 1 && d <= 3))
				return false;
		} else if (y == 2021) {
			// Ramadan
			if ((m == May && d >= 13 && d <= 15)
				// Kurban
				|| (m == July && d >= 20 && d <= 23))
				return false;
		} else if (y == 2022) {
			// Ramadan
			if ((m == May && d >= 2 && d <= 4)
				// Kurban
				|| (m == July && d >= 9 && d <= 12))
				return false;
		} else if (y == 2023) {
			// Ramadan
			if ((m == April && d >= 21 && d <= 23)
				// Kurban
                // July 1 is also a holiday but falls on a Saturday which is already flagged
				|| (m == June && d >= 28 && d <= 30))
				return false;
		} else if (y == 2024) {
		// Note: Holidays >= 2024 are not yet officially anounced by borsaistanbul.com
		// and need further validation
			// Ramadan
			if ((m == April && d >= 10 && d <= 12)
				// Kurban
				|| (m == June && d >= 17 && d <= 19))
				return false;
		} else if (y == 2025) {
			// Ramadan
			if ((m == March && d == 31) || (m == April && d >= 1 && d <= 2)
				// Kurban
				|| (m == June && d >= 6 && d <= 9))
				return false;
		} else if (y == 2026) {
			// Ramadan
			if ((m == March && d >= 20 && d <= 22)
				// Kurban
				|| (m == May && d >= 26 && d <= 29))
				return false;
		} else if (y == 2027) {
			// Ramadan
			if ((m == March && d >= 10 && d <= 12)
				// Kurban
				|| (m == May && d >= 16 && d <= 19))
				return false;
		} else if (y == 2028) {
			// Ramadan
			if ((m == February && d >= 27 && d <= 29)
				// Kurban
				|| (m == May && d >= 4 && d <= 7))
				return false;
		} else if (y == 2029) {
			// Ramadan
			if ((m == February && d >= 15 && d <= 17)
				// Kurban
				|| (m == April && d >= 23 && d <= 26))
				return false;
		} else if (y == 2030) {
			// Ramadan
			if ((m == February && d >= 5 && d <= 7)
				// Kurban
				|| (m == April && d >= 13 && d <= 16))
				return false;
		} else if (y == 2031) {
			// Ramadan
			if ((m == January && d >= 25 && d <= 27)
				// Kurban
				|| (m == April && d >= 2 && d <= 5))
				return false;
		} else if (y == 2032) {
			// Ramadan
			if ((m == January && d >= 14 && d <= 16)
				// Kurban
				|| (m == March && d >= 21 && d <= 24))
				return false;
		} else if (y == 2033) {
			// Ramadan
			if ((m == January && d >= 3 && d <= 5) || (m == December && d == 23)
				// Kurban
				|| (m == March && d >= 11 && d <= 14))
				return false;
		} else if (y == 2034) {
			// Ramadan
			if ((m == December && d >= 12 && d <= 14) 
				// Kurban
				|| (m == February && d == 28) || (m == March && d >= 1 && d <= 3))
				return false;
		}
        return true;
    }

}

