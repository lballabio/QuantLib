/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2007, 2011 StatPro Italia srl

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

#include <ql/time/calendars/saudiarabia.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    SaudiArabia::SaudiArabia(Market market) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> tadawulImpl(
                                                new SaudiArabia::TadawulImpl);
        switch (market) {
          case Tadawul:
            impl_ = tadawulImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    bool SaudiArabia::TadawulImpl::isWeekend(Weekday w) const {
        return w == Thursday || w == Friday;
    }

    bool SaudiArabia::TadawulImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        if (isWeekend(w)
            // National Day
            || (d == 23 && m == September)
            // Eid Al-Adha
            || (d >= 1 && d <= 6 && m == February && y==2004)
            || (d >= 21 && d <= 25 && m == January && y==2005)
            || (d >= 26 && m == November && y==2009)
            || (d <= 4 && m == December && y==2009)
            || (d >= 11 && d <= 19 && m == November && y==2010)
            // Eid Al-Fitr
            || (d >= 25 && d <= 29 && m == November && y==2004)
            || (d >= 14 && d <= 18 && m == November && y==2005)
            || (d >= 25 && m == August && y==2011)
            || (d <= 2 && m == September && y==2011)
            // other one-shot holidays
            || (d == 26 && m == February && y==2011)
            || (d == 19 && m == March && y==2011)
            )
            return false;
        return true;
    }

}

