/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group

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

#include <ql/time/calendars/china.hpp>

namespace QuantLib {

    China::China() {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new China::Impl);
        impl_ = impl;
    }

    bool China::Impl::isWeekend(Weekday w) const {
        return w == Saturday || w == Sunday;
    }

    bool China::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day dd = date.dayOfYear();

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Labor Day
            || (d >= 1 && d <= 7 && m == May)
            // National Day
            || (d >= 1 && d <= 7 && m == October)
            // Lunar New Year 2004
            || (d >= 22 && d <= 28 && m == January && y==2004)
            // Spring Festival
            || (dd == springFestival(y))
            // Last day of Lunar Year
            || (dd == springFestival(y)-1)
            )
            return false;
        return true;
    }

    Day China::Impl::springFestival(Year y) {
        static const Day SpringFestival[] = {
                  50,  39,  29,  47,  35,  25,  44,  33,  22,   // 1901-1909
             41,  30,  49,  37,  26,  45,  34,  23,  42,  32,   // 1910-1919
             51,  39,  28,  47,  36,  24,  44,  33,  23,  41,   // 1920-1929
             30,  48,  37,  26,  45,  35,  24,  42,  31,  50,   // 1930-1939
             39,  27,  46,  36,  25,  44,  33,  22,  41,  29,   // 1940-1949
             48,  37,  27,  45,  34,  24,  43,  31,  49,  39,   // 1950-1959
             28,  46,  36,  25,  44,  33,  21,  40,  30,  48,   // 1960-1969
             37,  27,  46,  34,  23,  42,  31,  49,  38,  28,   // 1970-1979
             47,  36,  25,  44,  33,  51,  40,  29,  48,  37,   // 1980-1989
             27,  46,  35,  23,  41,  31,  50,  38,  28,  47,   // 1990-1999
             36,  24,  43,  32,  22,  40,  29,  49,  38,  26,   // 2000-2009
             45,  34,  23,  41,  31,  50,  39,  28,  47,  36,   // 2010-2019
             25,  43,  32,  22,  41,  29,  48,  37,  26,  44,   // 2020-2029
             34,  23,  42,  31,  50,  39,  28,  46,  35,  24,   // 2030-2039
             43,  32,  22,  41,  30,  48,  37,  26,  45,  33,   // 2040-2049
             23,  42,  32,  50,  39,  28,  46,  35,  24,  43,   // 2050-2059
             33,  21,  40,  29,  48,  36,  26,  45,  34,  23,   // 2060-2069
             42,  31,  50,  38,  27,  46,  36,  24,  43,  33,   // 2070-2079
             22,  40,  29,  48,  37,  26,  45,  34,  24,  41,   // 2080-2089
             30,  49,  38,  27,  46,  36,  25,  43,  32,  21,   // 2090-2099
             40,  29,  48,  38,  28,  46,  35,  24,  43,  31,   // 2100-2109
             50,  39,  29,  47,  37,  26,  45,  33,  22,  41,   // 2110-2119
             30,  48,  38,  27,  46,  34,  23,  42,  32,  50,   // 2120-2129
             39,  29,  48,  36,  25,  44,  33,  22,  41,  30,   // 2130-2139
             49,  38,  27,  46,  35,  23,  42,  32,  51,  39,   // 2140-2149
             29,  47,  36,  25,  43,  33,  23,  41,  30,  49,   // 2150-2159
             38,  26,  45,  34,  24,  42,  32,  51,  40,  28,   // 2160-2169
             47,  36,  25,  43,  33,  23,  42,  30,  49,  38,   // 2170-2179
             27,  45,  34,  24,  43,  31,  21,  39,  28,  46,   // 2180-2189
             36,  25,  44,  33,  22,  41,  30,  48,  37,  27    // 2190-2199
        };
        return SpringFestival[y-1901];
    }

}

