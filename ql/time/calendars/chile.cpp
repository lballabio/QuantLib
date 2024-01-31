/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Anubhav Pandey

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
            || (d == 2 && m == January && w == Monday && y > 2016)
            // Good Friday
            || (dd == em-3)
            // Easter Saturday
            || (dd == em-2)
            // Labour Day
            || (d == 1 && m == May)
            // Navy Day
            || (d == 21 && m == May)
            // Day of Aboriginal People
            || (d == 21 && m == June && y >= 2021)
            // St. Peter and St. Paul
            || (d >= 26 && d <= 29 && m == June && w == Monday)
            || (d == 2 && m == July && w == Monday)
            // Our Lady of Mount Carmel
            || (d == 16 && m == July)
            // Assumption Day
            || (d == 15 && m == August)
            // Independence Day
            || (d == 17 && m == September && ((w == Monday && y >= 2007) || (w == Friday && y > 2016)))
            || (d == 18 && m == September)
            // Army Day
            || (d == 19 && m == September)
            || (d == 20 && m == September && w == Friday && y >= 2007)
            // Discovery of Two Worlds
            || (d >= 9 && d <= 12 && m == October && w == Monday)
            || (d == 15 && m == October && w == Monday)
            // Reformation Day
            || (((d == 27 && m == October && w == Friday)
                 || (d == 31 && m == October && w != Tuesday && w != Wednesday)
                 || (d == 2 && m == November && w == Friday)) && y >= 2008)
            // All Saints' Day
            || (d == 1 && m == November)
            // Immaculate Conception
            || (d == 8 && m == December)
            // Christmas Day
            || (d == 25 && m == December)
            // New Year's Eve 
            || (d == 31 && m == December)
            )
            return false;

        return true;
    }

}

