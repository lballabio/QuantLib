/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2007, 2009, 2010 StatPro Italia srl

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

#include <ql/time/calendars/hongkong.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    HongKong::HongKong(Market m) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new HongKong::HkexImpl);
        switch (m) {
          case HKEx:
            impl_ = impl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    bool HongKong::HkexImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);

        if (isWeekend(w)
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
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) && m == July)
            // National Day
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday))
                && m == October)
            // Christmas Day
            || (d == 25 && m == December)
            // Boxing Day
            || (d == 26 && m == December)
            )
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

        if (y == 2007) {
            if (// Lunar New Year
                ((d >= 17 && d <= 20) && m == February)
                // Buddha's birthday
                || (d == 24 && m == May)
                // Tuen NG festival
                || (d == 19 && m == June)
                // Mid-autumn festival
                || (d == 26 && m == September)
                // Chung Yeung festival
                || (d == 19 && m == October))
            return false;
        }

        if (y == 2008) {
            if (// Lunar New Year
                ((d >= 7 && d <= 9) && m == February)
                // Ching Ming Festival
                || (d == 4 && m == April)
                // Buddha's birthday
                || (d == 12 && m == May)
                // Tuen NG festival
                || (d == 9 && m == June)
                // Mid-autumn festival
                || (d == 15 && m == September)
                // Chung Yeung festival
                || (d == 7 && m == October))
            return false;
        }

        if (y == 2009) {
            if (// Lunar New Year
                ((d >= 26 && d <= 28) && m == January)
                // Ching Ming Festival
                || (d == 4 && m == April)
                // Buddha's birthday
                || (d == 2 && m == May)
                // Tuen NG festival
                || (d == 28 && m == May)
                // Mid-autumn festival
                || (d == 3 && m == October)
                // Chung Yeung festival
                || (d == 26 && m == October))
            return false;
        }

        if (y == 2010) {
            if (// Lunar New Year
                ((d == 15 || d == 16) && m == February)
                // Ching Ming Festival
                || (d == 6 && m == April)
                // Buddha's birthday
                || (d == 21 && m == May)
                // Tuen NG festival
                || (d == 16 && m == June)
                // Mid-autumn festival
                || (d == 23 && m == September))
            return false;
        }

        return true;
    }

}

