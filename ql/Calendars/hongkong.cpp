
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

#include <ql/Calendars/hongkong.hpp>

namespace QuantLib {

    bool HongKong::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);

        if ((w == Saturday || w == Sunday)
            // New Year's Day
            || (d == 1 && m == January)
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
            || (d == 1 && m == October)
            // Christmas Day
            || (d == 25 && m == December)
            // Boxing Day
            || (d == 26 && m == December)
            // Christmas Holiday
            || (d == 27 && m == December)

            // Lunar New Year 2004
            || ((d==22 || d==23 || d==24) && m == January && y==2004)
            // Chinese New Year 2005
            || ((d==9 || d==10 || d==11) && m == February && y==2005)
				
            // Buddha's birthday 2004
            || (d == 26 && m == May && y==2004)
            // Buddha's birthday 2005
            || (d == 16 && m == May && y==2005)
            // Tuen NG Festival 2004
            || (d == 22 && m == June && y==2004)
            // Tuen NG Festival 2005
            || (d == 11 && m == June && y==2005)
            // Day after mid-autumn fest 2004
            || (d == 29 && m == September && y==2004)
            // Mid-autumn fest 2005
            || (d == 19 && m == September && y==2005)
            // Chung Yeung fest 2004
            || (d == 29 && m == September && y==2004)
            // Chung Yeung fest 2005
            || (d == 11 && m == October && y==2005)
            )
            return false;
        return true;
    }

}

