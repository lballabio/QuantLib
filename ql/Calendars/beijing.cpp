
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
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day dd = date.dayOfYear();

        if ((w == Saturday || w == Sunday)
            // New Year's Day
            || (d == 1 && m == January)
            // Labor Day
            || (d >= 1 && d <= 7 && m == May)
            // National Day
            || (d >= 1 && d <= 7 && m == October)
            // Lunar New Year 2004
            || (d >= 22 && d <= 28 && m == January && y==2004)
            // Spring Festival
            || (dd == springFestival(y-1900))
            // Last day of Lunar Year
            || (dd == springFestival(y-1900)-1)
            )
            return false;
        return true;
    }

    Day Beijing::Impl::springFestival(Year y) {
        static const Day SpringFestival[] = {
            31,  51,  39,  29,  47,  36,  25,  44,  33,  22,   // 1900-1909
            41,  30,  49,  37,  26,  45,  34,  23,  42,  32,   // 1910-1919
            52,  39,  28,  47,  37,  24,  44,  33,  23,  41,   // 1920-1929
            30,  48,  37,  26,  45,  35,  24,  42,  31,  51,   // 1930-1939
            39,  27,  46,  37,  25,  44,  33,  22,  41,  29,   // 1940-1949
            48,  37,  27,  45,  34,  24,  43,  31,  49,  39,   // 1950-1959
            28,  46,  36,  25,  44,  33,  21,  40,  30,  48,   // 1960-1969
            37,  27,  46,  34,  23,  42,  31,  49,  38,  28,   // 1970-1979
            47,  36,  25,  44,  33,  51,  40,  29,  48,  37,   // 1980-1989
            27,  46,  36,  23,  41,  31,  51,  38,  28,  47,   // 1990-1999
            36,  24,  43,  32,  22,  40,  29,  49,  38,  26,   // 2000-2009
            45,  34,  23,  41,  31,  51,  39,  28,  47,  36,   // 2010-2019
            25,  43,  32,  22,  41,  29,  48,  37,  26,  44,   // 2020-2029
            34,  23,  42,  31,  50,  39,  28,  46,  36,  24,   // 2030-2039
            43,  32,  22,  41,  30,  48,  37,  26,  45,  33,   // 2040-2049
            23,  42,  32,  50,  39,  28,  46,  35,  24,  43,   // 2050-2059
            33,  21,  40,  29,  48,  36,  26,  45,  34,  23,   // 2060-2069
            42,  31,  51,  38,  27,  46,  36,  24,  43,  33,   // 2070-2079
            22,  40,  29,  48,  37,  26,  45,  35,  24,  41,   // 2080-2089
            30,  50,  38,  27,  46,  36,  25,  43,  32,  21,   // 2090-2099
        };
        return SpringFestival[y-1900];
    }

}

