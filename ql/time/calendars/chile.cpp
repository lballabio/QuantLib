/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007, 2008, 2009, 2010, 2011 StatPro Italia srl

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

#include <ql/time/calendars/chile.hpp>

namespace QuantLib {

    Chile::Chile(Market) {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new Chile::SseImpl);
        impl_ = impl;
    }

    bool Chile::SseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day dd = date.dayOfYear();
        Day em = easterMonday(y);

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Saturday
            || (dd == em-2)
            // Labour Day
            || (d == 1 && m == May)
            // Navy Day
            || (d == 21 && m == May)
            // Our Lady of Mount Carmel
            || (d == 16 && m == July)
            // Assumption Day
            || (d == 15 && m == August)
            // Independence Day
            || (d == 18 && m == September)
            // Army Day
            || (d == 19 && m == September)
            // All Saints' Day
            || (d == 1 && m == November)
            // Immaculate Conception
            || (d == 8 && m == December)
            // Christmas Day
            || (d == 25 && m == December)
            )
            return false;

        if (y == 2018) {
            if (// Feast of St Peter and St Paul
                (d == 2 && m == July)
                // Independence Day Holiday (additional holiday)
                || (d == 17 && m == September)
                // Day of the Race
                || (d == 15 && m == October)
                // Reformation Day
                || (d == 2 && m == November)
                )
                return false;
        }

        if (y == 2019) {
            // Feast of St Peter and St Paul and Day of the Race fall
            // on Saturday in 2019
            if (// Army Day Holiday (additional holiday)
                (d == 20 && m == September)
                // Reformation Day
                || (d == 1 && m == November)
                )
                return false;
        }

        if (y == 2020) {
            // Reformation Day falls on Saturday in 2020
            if (// Feast of St Peter and St Paul
                (d == 29 && m == June)
                // Day of the Race
                || (d == 12 && m == October)
                )
                return false;
        }

        if (y == 2021) {
            // Reformation Day falls on Sunday in 2021
            if (// Feast of St Peter and St Paul
                (d == 28 && m == June)
                // Day of the Race
                || (d == 11 && m == October)
                )
                return false;
        }

        if (y == 2022) {
            if (// Feast of St Peter and St Paul
                (d == 27 && m == June)
                // Day of the Race
                || (d == 10 && m == October)
                // Reformation Day
                || (d == 31 && m == October)
                )
                return false;
        }

        if (y == 2023) {
            if (// New Year Holiday (additional holiday)
                (d == 2 && m == January)
                // Feast of St Peter and St Paul
                || (d == 26 && m == June)
                // Day of the Race
                || (d == 9 && m == October)
                // Reformation Day
                || (d == 31 && m == October)
                )
                return false;
        }

        if (y == 2024) {
            // Feast of St Peter and St Paul falls on Saturday in 2024
            if (// Day of the Race
                (d == 14 && m == October)
                // Reformation Day
                || (d == 31 && m == October)
                )
                return false;
        }


        return true;
    }

}

