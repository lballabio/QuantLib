/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano

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

#include <ql/time/calendars/germany.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Germany::Germany(Germany::Market market) {
        // all calendar instances on the same market share the same
        // implementation instance
        static ext::shared_ptr<Calendar::Impl> settlementImpl(
            new Germany::SettlementImpl);
        static ext::shared_ptr<Calendar::Impl> frankfurtStockExchangeImpl(
            new Germany::FrankfurtStockExchangeImpl);
        static ext::shared_ptr<Calendar::Impl> xetraImpl(
            new Germany::XetraImpl);
        static ext::shared_ptr<Calendar::Impl> eurexImpl(
            new Germany::EurexImpl);
        static ext::shared_ptr<Calendar::Impl> euwaxImpl(
            new Germany::EuwaxImpl);

        switch (market) {
          case Settlement:
            impl_ = settlementImpl;
            break;
          case FrankfurtStockExchange:
            impl_ = frankfurtStockExchangeImpl;
            break;
          case Xetra:
            impl_ = xetraImpl;
            break;
          case Eurex:
            impl_ = eurexImpl;
            break;
          case Euwax:
            impl_ = euwaxImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }


    bool Germany::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // Ascension Thursday
            || (dd == em+38)
            // Whit Monday
            || (dd == em+49)
            // Corpus Christi
            || (dd == em+59)
            // Labour Day
            || (d == 1 && m == May)
            // National Day
            || (d == 3 && m == October)
            // Christmas Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Boxing Day
            || (d == 26 && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

    bool Germany::FrankfurtStockExchangeImpl::isBusinessDay(
      const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // Labour Day
            || (d == 1 && m == May)
            // Christmas' Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Christmas Day
            || (d == 26 && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

    bool Germany::XetraImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // Labour Day
            || (d == 1 && m == May)
            // Christmas' Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Christmas Day
            || (d == 26 && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

    bool Germany::EurexImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // Labour Day
            || (d == 1 && m == May)
            // Christmas' Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Christmas Day
            || (d == 26 && m == December)
            // New Year's Eve
            || (d == 31 && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }
    
    bool Germany::EuwaxImpl::isBusinessDay(const Date& date) const {
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
            // Easter Monday
            || (dd == em)
            // Labour Day
            || (d == 1 && m == May)
            // Whit Monday
            || (dd == em+49)
            // Christmas' Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Christmas Day
            || (d == 26 && m == December))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }
}

