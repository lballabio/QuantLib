/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 StatPro Italia srl

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

#include <ql/time/calendars/taiwan.hpp>

namespace QuantLib {

    Taiwan::Taiwan(Market) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Taiwan::TsecImpl);
        impl_ = impl;
    }

    bool Taiwan::TsecImpl::isWeekend(Weekday w) const {
        return w == Saturday || w == Sunday;
    }

    bool Taiwan::TsecImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        if (isWeekend(w)
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

        if (y == 2007) {
            if (// Chinese Lunar New Year
                (d >= 17 && d <= 25 && m == February)
                // Tomb Sweeping Day
                || (d == 5 && m == April)
                // adjusted holidays
                || (d == 6 && m == April)
                || (d == 18 && m == June)
                // Dragon Boat Festival
                || (d == 19 && m == June)
                // adjusted holiday
                || (d == 24 && m == September)
                // Moon Festival
                || (d == 25 && m == September)
                )
                return false;
        }

        if (y == 2008) {
            if (// Chinese Lunar New Year
                (d >= 4 && d <= 11 && m == February)
                // Tomb Sweeping Day
                || (d == 4 && m == April)
                )
                return false;
        }

        if (y == 2009) {
            if (// Public holiday
                (d == 2 && m == January)
                // Chinese Lunar New Year
                || (d >= 24 && m == January)
                // Tomb Sweeping Day
                || (d == 4 && m == April)
                // Dragon Boat Festival
                || ((d == 28 || d == 29) && m == May)
                // Moon Festival
                || (d == 3 && m == October)
                )
                return false;
        }

        if (y == 2010) {
            if (// Chinese Lunar New Year
                   (d >= 13 && d <= 21 && m == January)
                // Tomb Sweeping Day
                || (d == 5 && m == April)
                // Dragon Boat Festival
                || (d == 16 && m == May)
                // Moon Festival
                || (d == 22 && m == September)
                )
                return false;
        }

        if (y == 2011) {
            if (// Spring Festival
                   (d >= 2 && d <= 7 && m == February)
                // Children's Day
                || (d == 4 && m == April)
                // Tomb Sweeping Day
                || (d == 5 && m == April)
                // Labour Day
                || (d == 2 && m == May)
                // Dragon Boat Festival
                || (d == 6 && m == June)
                // Mid-Autumn Festival
                || (d == 12 && m == September)
                )
                return false;
        }

        if (y == 2012) {
            if (// Spring Festival
                (d >= 23 && d <= 27 && m == January)
                // Peace Memorial Day
                || (d == 27 && m == February)
                // Children's Day
                // Tomb Sweeping Day
                || (d == 4 && m == April)
                // Labour Day
                || (d == 1 && m == May)
                // Dragon Boat Festival
                || (d == 23 && m == June)
                // Mid-Autumn Festival
                || (d == 30 && m == September)
                // Memorial Day:
                // Founding of the Republic of China
                || (d == 31 && m == December)
                )
                return false;
        }

        if (y == 2013) {
            if (// Spring Festival
                (d >= 10 && d <= 15 && m == February)
                // Children's Day
                || (d == 4 && m == April)
                // Tomb Sweeping Day
                || (d == 5 && m == April)
                // Labour Day
                || (d == 1 && m == May)
                // Dragon Boat Festival
                || (d == 12 && m == June)
                // Mid-Autumn Festival
                || (d >= 19 && d <= 20 && m == September)
                )
                return false;
        }

        if (y == 2014) {
            if (// Lunar New Year
                (d >= 28 && d <= 30 && m == January)
                // Spring Festival
                || ((d == 31 && m == January) || (d <= 4 && m == February))
                // Children's Day
                || (d == 4 && m == April)
                // Tomb Sweeping Day
                || (d == 5 && m == April)
                // Labour Day
                || (d == 1 && m == May)
                // Dragon Boat Festival
                || (d == 2 && m == June)
                // Mid-Autumn Festival
                || (d == 8 && m == September)
                )
                return false;
        }

        return true;
    }

}

