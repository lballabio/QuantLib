/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group

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
            // Peace Day
            || (d == 28 && m == February)
            // Labour Day
            || (d == 1 && m == May)
            // National Day
            || (d == 10 && m == October)

            // Lunar New Year 2004
            || ((d == 21 || d==22 || d==23 || d==24 || d==26 )
                && m == January && y==2004)
            // Lunar New Year 2005
            || ((d == 8 || d==9 || d==10 || d==11 )
                && m == February && y==2005)
            // Lunar New Year 2006
            || ((d == 28 || d==29 || d==30 || d==31 )
                && m == January && y==2006)
            // Tomb Sweeping Day 2004
            || (d == 4 && m == April && y==2004)
            // Tomb Sweeping Day 2005
            || (d == 5 && m == April && y==2005)
            // Tomb Sweeping Day 2006
            || (d == 5 && m == April && y==2006)
            // Dragon Boat Day 2004
            || (d == 22 && m == June && y==2004)
            // Dragon Boat Day 2005
            || (d == 11 && m == June && y==2005)
            // Dragon Boat Day 2006
            || (d == 31 && m == May && y==2006)
            // Mid-Autumn Festival 2004
            || (d == 28 && m == September && y==2004)
            // Mid-Autumn Festival 2005
            || (d == 18 && m == September && y==2005)
            // Mid-Autumn Festival 2006
            || (d == 6 && m == October && y==2006)
            )
            return false;
        return true;
    }

}

