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

#include <ql/Calendars/riyadh.hpp>

namespace QuantLib {

    SaudiArabia::SaudiArabia() {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new SaudiArabia::Impl);
        impl_ = impl;
    }

    bool SaudiArabia::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        if ((w == Friday)

            // eid al-adha 2004
            || ((d==29 || d==30) && m == January && y==2004)
            || (d >= 1 && d <= 6 && m == February && y==2004)
            // eid al-adha 2005
            || (d >= 21 && d <= 25 && m == January && y==2005)

            // eid al-fitr 2004
            || (d >= 25 && d <= 29 && m == November && y==2004)
            // eid al-fitr 2005
            || (d >= 14 && d <= 18 && m == November && y==2005)
            )
            return false;
        return true;
    }

}

