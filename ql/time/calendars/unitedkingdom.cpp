/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    namespace {

        // common rules

        bool isBankHoliday(Day d, Weekday w, Month m, Year y) {
            return
                // first Monday of May (Early May Bank Holiday)
                // moved to May 8th in 1995 and 2020 for V.E. day
                (d <= 7 && w == Monday && m == May && y != 1995 && y != 2020)
                || (d == 8 && m == May && (y == 1995 || y == 2020))
                // last Monday of May (Spring Bank Holiday)
                // moved to in 2002, 2012 and 2022 for the Golden, Diamond and Platinum
                // Jubilee with an additional holiday
                || (d >= 25 && w == Monday && m == May && y != 2002 && y != 2012 && y != 2022)
                || ((d == 3 || d == 4) && m == June && y == 2002)
                || ((d == 4 || d == 5) && m == June && y == 2012)
                || ((d == 2 || d == 3) && m == June && y == 2022)
                // last Monday of August (Summer Bank Holiday)
                || (d >= 25 && w == Monday && m == August)
                // April 29th, 2011 only (Royal Wedding Bank Holiday)
                || (d == 29 && m == April && y == 2011)
                // September 19th, 2022 only (The Queen's Funeral Bank Holiday)
                || (d == 19 && m == September && y == 2022)
                ;
        }

    }

    UnitedKingdom::UnitedKingdom(UnitedKingdom::Market market) {
        // all calendar instances on the same market share the same
        // implementation instance
        static ext::shared_ptr<Calendar::Impl> settlementImpl(
                                           new UnitedKingdom::SettlementImpl);
        static ext::shared_ptr<Calendar::Impl> exchangeImpl(
                                           new UnitedKingdom::ExchangeImpl);
        static ext::shared_ptr<Calendar::Impl> metalsImpl(
                                           new UnitedKingdom::MetalsImpl);
        switch (market) {
          case Settlement:
            impl_ = settlementImpl;
            break;
          case Exchange:
            impl_ = exchangeImpl;
            break;
          case Metals:
            impl_ = metalsImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    bool UnitedKingdom::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday)
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) &&
                m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            || isBankHoliday(d, w, m, y)
            // Christmas (possibly moved to Monday or Tuesday)
            || ((d == 25 || (d == 27 && (w == Monday || w == Tuesday)))
                && m == December)
            // Boxing Day (possibly moved to Monday or Tuesday)
            || ((d == 26 || (d == 28 && (w == Monday || w == Tuesday)))
                && m == December)
            // December 31st, 1999 only
            || (d == 31 && m == December && y == 1999))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }


    bool UnitedKingdom::ExchangeImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday)
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) &&
                m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            || isBankHoliday(d, w, m, y)
            // Christmas (possibly moved to Monday or Tuesday)
            || ((d == 25 || (d == 27 && (w == Monday || w == Tuesday)))
                && m == December)
            // Boxing Day (possibly moved to Monday or Tuesday)
            || ((d == 26 || (d == 28 && (w == Monday || w == Tuesday)))
                && m == December)
            // December 31st, 1999 only
            || (d == 31 && m == December && y == 1999))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }


    bool UnitedKingdom::MetalsImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday)
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) &&
                m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            || isBankHoliday(d, w, m, y)
            // Christmas (possibly moved to Monday or Tuesday)
            || ((d == 25 || (d == 27 && (w == Monday || w == Tuesday)))
                && m == December)
            // Boxing Day (possibly moved to Monday or Tuesday)
            || ((d == 26 || (d == 28 && (w == Monday || w == Tuesday)))
                && m == December)
            // December 31st, 1999 only
            || (d == 31 && m == December && y == 1999))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

