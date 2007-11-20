/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2007 StatPro Italia srl

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
#include <ql/errors.hpp>

namespace QuantLib {

    China::China(Market m) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> sseImpl(new China::SseImpl);
        switch (m) {
          case SSE:
            impl_ = sseImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    bool China::SseImpl::isWeekend(Weekday w) const {
        return w == Saturday || w == Sunday;
    }

    bool China::SseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            || (d == 3 && m == January && y == 2005)
            || ((d == 2 || d == 3) && m == January && y == 2006)
            || (d <= 3 && m == January && y == 2007)
            // Labor Day
            || (d >= 1 && d <= 7 && m == May)
            // National Day
            || (d >= 1 && d <= 7 && m == October)
            // Chinese New Year
            || (d >= 19 && d <= 28 && m == January && y == 2004)
            || (d >=  7 && d <= 15 && m == February && y == 2005)
            || (((d >= 26 && m == January) || (d <= 3 && m == February))
                && y == 2006)
            || (d >= 17 && d <= 25 && m == February && y == 2007)
            )
            return false;
        return true;
    }

}

