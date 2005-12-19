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

#include <ql/Calendars/singapore.hpp>

namespace QuantLib {

    Singapore::Singapore(Market) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Singapore::SgxImpl);
        impl_ = impl;
    }

    bool Singapore::SgxImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);

        if ((w == Saturday || w == Sunday)
            // New Year's Day
            || (d == 1 && m == January)
            // Good Friday
            || (dd == em-3)
            // Labor Day
            || (d == 1 && m == May)
            // National Day
            || (d == 9 && m == August)
            // Christmas Day
            || (d == 25 && m == December)
            // Boxing Day
            || (d == 26 && m == December)

            // Chinese New Year 2004
            || ((d==22 || d==23) && m == January && y==2004)
            // Chinese New Year 2005
            || ((d==9 || d==10) && m == February && y==2005)

            // Hari Raya Haji 2004
            || ((d==1 || d==2) && m == February && y==2004)
            // Hari Raya Haji 2005
            || ((d==21) && m == January && y==2005)

            // Vesak Poya Day 2004
            || ((d==2) && m == June && y==2004)
            // Vesak Poya Day 2005
            || ((d==22) && m == May && y==2005)

            // Deepavali
            || ((d==11) && m == November && y==2004)
            // Diwali
            || ((d==1) && m == November && y==2005)

            // Hari Raya Puasa 2004
            || ((d==14 || d==15) && m == November && y==2004)
            // Hari Raya Puasa 2005
            || (d==3 && m == November && y==2005)
            )
            return false;
        return true;
    }

}

