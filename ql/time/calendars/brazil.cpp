/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Piter Dias
 Copyright (C) 2007 Richard Gomes

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

#include <ql/time/calendars/brazil.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Brazil::Brazil(Brazil::Market market) {
        // all calendar instances on the same market share the same
        // implementation instance
        static ext::shared_ptr<Calendar::Impl> settlementImpl(
                                                  new Brazil::SettlementImpl);
        static ext::shared_ptr<Calendar::Impl> exchangeImpl(
                                                    new Brazil::ExchangeImpl);
        switch (market) {
          case Settlement:
            impl_ = settlementImpl;
            break;
          case Exchange:
            impl_ = exchangeImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    bool Brazil::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day dd = date.dayOfYear();
        Day em = easterMonday(y);

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Tiradentes Day
            || (d == 21 && m == April)
            // Labor Day
            || (d == 1 && m == May)
            // Independence Day
            || (d == 7 && m == September)
            // Nossa Sra. Aparecida Day
            || (d == 12 && m == October)
            // All Souls Day
            || (d == 2 && m == November)
            // Republic Day
            || (d == 15 && m == November)
            // Christmas
            || (d == 25 && m == December)
            // Passion of Christ
            || (dd == em-3)
            // Carnival
            || (dd == em-49 || dd == em-48)
            // Corpus Christi
            || (dd == em+59)
            )
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

    bool Brazil::ExchangeImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day dd = date.dayOfYear();
        Day em = easterMonday(y);

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Sao Paulo City Day
            || (d == 25 && m == January)
            // Tiradentes Day
            || (d == 21 && m == April)
            // Labor Day
            || (d == 1 && m == May)
            // Revolution Day
            || (d == 9 && m == July)
            // Independence Day
            || (d == 7 && m == September)
            // Nossa Sra. Aparecida Day
            || (d == 12 && m == October)
            // All Souls Day
            || (d == 2 && m == November)
            // Republic Day
            || (d == 15 && m == November)
            // Black Consciousness Day
            || (d == 20 && m == November && y >= 2007)
            // Christmas Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Passion of Christ
            || (dd == em-3)
            // Carnival
            || (dd == em-49 || dd == em-48)
            // Corpus Christi
            || (dd == em+59)
            // last business day of the year
            || (m == December && (d == 31 || (d >= 29 && w == Friday)))
            )
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

