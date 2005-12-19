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

#include <ql/Calendars/hongkong.hpp>

namespace QuantLib {

    HongKong::HongKong(Market) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new HongKong::HkexImpl);
        impl_ = impl;
    }

    bool HongKong::HkexImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);

        if ((w == Saturday || w == Sunday)
            // New Year's Day
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday))
                && m == January)
            // Ching Ming Festival
            || (d == 5 && m == April)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // Labor Day
            || (d == 1 && m == May)
            // SAR Establishment Day
            || (d == 1 && m == July)
            // National Day
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday))
                && m == October)
            // Christmas Day
            || (d == 25 && m == December)
            // Boxing Day
            || ((d == 26 || ((d == 27 || d == 28) && w == Monday))
                && m == December))
            return false;

        if (y == 2004) {
            if (// Lunar New Year
                ((d==22 || d==23 || d==24) && m == January)
                // Buddha's birthday
                || (d == 26 && m == May)
                // Tuen NG festival
                || (d == 22 && m == June)
                // Mid-autumn festival
                || (d == 29 && m == September)
                // Chung Yeung
                || (d == 29 && m == September))
                return false;
        }

        if (y == 2005) {
            if (// Lunar New Year
                ((d==9 || d==10 || d==11) && m == February)
                // Buddha's birthday
                || (d == 16 && m == May)
                // Tuen NG festival
                || (d == 11 && m == June)
                // Mid-autumn festival
                || (d == 19 && m == September)
                // Chung Yeung festival
                || (d == 11 && m == October))
            return false;
        }

        if (y == 2006) {
            if (// Lunar New Year
                ((d >= 28 && d <= 31) && m == January)
                // Buddha's birthday
                || (d == 5 && m == May)
                // Tuen NG festival
                || (d == 31 && m == May)
                // Mid-autumn festival
                || (d == 7 && m == October)
                // Chung Yeung festival
                || (d == 30 && m == October))
            return false;
        }

        return true;
    }

}

