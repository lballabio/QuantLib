/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/Calendars/taiwan.hpp>

namespace QuantLib {

    Taiwan::Taiwan() {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Taiwan::Impl);
        impl_ = impl;
    }

    bool Taiwan::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        if ((w == Saturday || w == Sunday)
            // New Year's Day
            || (d == 1 && m == January)
            // Peace Memorial Day
            || (d == 28 && m == February)
            // Labor Day
            || (d == 1 && m == May)
            // Double Tenth
            || (d == 10 && m == October)
            )
            return false;

        if (y == 2002) {
            // Dragon Boat Festival and Moon Festival fall on Saturday
            if (// Chinese Lunar New Year
                (d >= 9 && d <= 17 && m == February)
                // Tomb Sweeping Day
                || (d == 5 && m == April)
                )
                return false;
        }
        if (y == 2003) {
            // Tomb Sweeping Day falls on Saturday
            if (// Chinese Lunar New Year
                ((d >= 31 && m == January) || (d <= 5 && m == February))
                // Dragon Boat Festival
                || (d == 4 && m == June)
                // Moon Festival
                || (d == 11 && m == September)
                )
                return false;
        }
        if (y == 2004) {
            // Tomb Sweeping Day falls on Sunday
            if (// Chinese Lunar New Year
                (d >= 21 && d <= 26 && m == January)
                // Dragon Boat Festival
                || (d == 22 && m == June)
                // Moon Festival
                || (d == 28 && m == September)
                )
                return false;
        }
        if (y == 2005) {
            // Dragon Boat and Moon Festival fall on Saturday or Sunday
            if (// Chinese Lunar New Year
                (d >= 6 && d <= 13 && m == February)
                // Tomb Sweeping Day
                || (d == 5 && m == April)
                // make up for Labor Day, not seen in other years
                || (d == 2 && m == May)
                )
                return false;
        }
        if (y == 2006) {
            // Dragon Boat and Moon Festival fall on Saturday or Sunday
            if (// Chinese Lunar New Year
                ((d >= 28 && m == January) || (d <= 5 && m == February))
                // Tomb Sweeping Day
                || (d == 5 && m == April)
                // Dragon Boat Festival
                || (d == 31 && m == May)
                // Moon Festival
                || (d == 6 && m == October)
                )
                return false;
        }
        return true;
    }

}

