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

#include <ql/Calendars/seoul.hpp>

namespace QuantLib {

    SouthKorea::SouthKorea(Market) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new SouthKorea::KrxImpl);
        impl_ = impl;
    }

    bool SouthKorea::KrxImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        if ((w == Saturday || w == Sunday)
            // New Year's Day
            || (d == 1 && m == January)
            // Independence Day
            || (d == 1 && m == March)
            // Arbour Day
            || (d == 5 && m == April)
            // Labor Day
            || (d == 1 && m == May)
            // Children's Day
            || (d == 5 && m == May)
            // Memorial Day
            || (d == 6 && m == June)
            // Constitution Day
            || (d == 17 && m == July)
            // Liberation Day
            || (d == 15 && m == August)
            // National Foundation Day
            || (d == 3 && m == October)
            // Christmas Day
            || (d == 25 && m == December)

            // Lunar New Year 2004
            || ((d == 21 || d==22 || d==23 || d==24 || d==26 )
                && m == January && y==2004)
            // Lunar New Year 2005
            || ((d == 8 || d==9 || d==10) && m == February && y==2005)
            // Lunar New Year 2006
            || ((d==29 || d==30 || d==31 ) && m == January && y==2006)
            // Election Day 2004
            || (d == 15 && m == April && y==2004)
            // Buddha's birthday 2004
            || (d == 26 && m == May && y==2004)
            // Buddha's birthday 2005
            || (d == 15 && m == May && y==2005)
            // Buddha's birthday 2006
            || (d == 5 && m == May && y==2006)
            // Harvest Moon Day 2004
            || ((d == 27 || d==28 || d==29) && m == September && y==2004)
            // Harvest Moon Day 2005
            || ((d == 17 || d==18 || d==19) && m == September && y==2005)
            // Harvest Moon Day 2006
            || ((d == 5 || d==6 || d==7) && m == October && y==2006)
            )
            return false;
        return true;
    }

}

