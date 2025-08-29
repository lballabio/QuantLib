/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Anubhav Pandey

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/time/calendars/chile.hpp>

namespace QuantLib {

    namespace {

        // Celebrated on the Winter Solstice day, except in 2021, when it was the day after.
        inline bool isAboriginalPeopleDay(Day d, Month m, Year y) {
            static const unsigned char aboriginalPeopleDay[] = {
                    21, 21, 21, 20, 20, 21, 21, 20, 20,   // 2021-2029
                21, 21, 20, 20, 21, 21, 20, 20, 21, 21,   // 2030-2039
                20, 20, 21, 21, 20, 20, 21, 21, 20, 20,   // 2040-2049
                20, 21, 20, 20, 20, 21, 20, 20, 20, 21,   // 2050-2059
                20, 20, 20, 21, 20, 20, 20, 21, 20, 20,   // 2060-2069
                20, 21, 20, 20, 20, 21, 20, 20, 20, 20,   // 2070-2079
                20, 20, 20, 20, 20, 20, 20, 20, 20, 20,   // 2080-2089
                20, 20, 20, 20, 20, 20, 20, 20, 20, 20,   // 2090-2099
                21, 21, 21, 21, 21, 21, 21, 21, 20, 21,   // 2100-2109
                21, 21, 20, 21, 21, 21, 20, 21, 21, 21,   // 2110-2119
                20, 21, 21, 21, 20, 21, 21, 21, 20, 21,   // 2120-2129
                21, 21, 20, 21, 21, 21, 20, 20, 21, 21,   // 2130-2139
                20, 20, 21, 21, 20, 20, 21, 21, 20, 20,   // 2140-2149
                21, 21, 20, 20, 21, 21, 20, 20, 21, 21,   // 2150-2159
                20, 20, 21, 21, 20, 20, 21, 21, 20, 20,   // 2160-2169
                20, 21, 20, 20, 20, 21, 20, 20, 20, 21,   // 2170-2179
                20, 20, 20, 21, 20, 20, 20, 21, 20, 20,   // 2180-2189
                20, 21, 20, 20, 20, 21, 20, 20, 20, 20    // 2190-2199
            };
            return m == June && y >= 2021 && d == aboriginalPeopleDay[y-2021];
        }

    }

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
            // Papal visit in 2018
            || (d == 16 && m == January && y == 2018)
            // Good Friday
            || (dd == em-3)
            // Easter Saturday
            || (dd == em-2)
            // Census Day in 2017
            || (d == 19 && m == April && y == 2017)
            // Labour Day
            || (d == 1 && m == May)
            // Navy Day
            || (d == 21 && m == May)
            // Day of Aboriginal People
            || isAboriginalPeopleDay(d, m, y)
            // St. Peter and St. Paul
            || (d >= 26 && d <= 29 && m == June && w == Monday)
            || (d == 2 && m == July && w == Monday)
            // Our Lady of Mount Carmel
            || (d == 16 && m == July)
            // Assumption Day
            || (d == 15 && m == August)
            // Independence Day
            || (d == 16 && m == September && y == 2022)
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

