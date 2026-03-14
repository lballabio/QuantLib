/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/time/calendars/france.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    France::France(France::Market market) {
        // all calendar instances on the same market share the same
        // implementation instance
        static ext::shared_ptr<Calendar::Impl> settlementImpl(
                                                   new France::SettlementImpl);
        static ext::shared_ptr<Calendar::Impl> exchangeImpl(
                                                   new France::ExchangeImpl);
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


    bool France::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // Jour de l'An
            || (d == 1 && m == January)
            // Lundi de Paques
            || (dd == em)
            // Fete du Travail
            || (d == 1 && m == May)
            // Victoire 1945
            || (d == 8 && m == May)
            // Ascension
            || (d == 10 && m == May)
            // Pentecote
            || (d == 21 && m == May)
            // Fete nationale
            || (d == 14 && m == July)
            // Assomption
            || (d == 15 && m == August)
            // Toussaint
            || (d == 1 && m == November)
            // Armistice 1918
            || (d == 11 && m == November)
            // Noel
            || (d == 25 && m == December))
            
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }


    bool France::ExchangeImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // Jour de l'An
            || (d == 1 && m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // Labor Day
            || (d == 1 && m == May)
            // Christmas Eve
            || (d == 24 && m == December)
            // Christmas Day
            || (d == 25 && m == December)
            // Boxing Day
            || (d == 26 && m == December)
            // New Year's Eve
            || (d == 31 && m == December))
         
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

