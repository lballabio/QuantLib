
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/Calendars/unitedstates.hpp>

namespace QuantLib {

    UnitedStates::UnitedStates(UnitedStates::Market market) {
        // all calendar instances on the same market share the same
        // implementation instance
        static boost::shared_ptr<Calendar::Impl> settlementImpl(
                                        new UnitedStates::SettlementImpl);
        static boost::shared_ptr<Calendar::Impl> exchangeImpl(
                                        new UnitedStates::ExchangeImpl);
        static boost::shared_ptr<Calendar::Impl> governmentImpl(
                                        new UnitedStates::GovernmentBondImpl);
        switch (market) {
          case Settlement:
            impl_ = settlementImpl;
            break;
          case Exchange:
            impl_ = exchangeImpl;
            break;
          case GovernmentBond:
            impl_ = governmentImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }


    bool UnitedStates::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        if ((w == Saturday || w == Sunday)
            // New Year's Day (possibly moved to Monday if on Sunday)
            || ((d == 1 || (d == 2 && w == Monday)) && m == January)
            // (or to Friday if on Saturday)
            || (d == 31 && w == Friday && m == December)
            // Martin Luther King's birthday (third Monday in January)
            || ((d >= 15 && d <= 21) && w == Monday && m == January)
            // Washington's birthday (third Monday in February)
            || ((d >= 15 && d <= 21) && w == Monday && m == February)
            // Memorial Day (last Monday in May)
            || (d >= 25 && w == Monday && m == May)
            // Independence Day (Monday if Sunday or Friday if Saturday)
            || ((d == 4 || (d == 5 && w == Monday) ||
                 (d == 3 && w == Friday)) && m == July)
            // Labor Day (first Monday in September)
            || (d <= 7 && w == Monday && m == September)
            // Columbus Day (second Monday in October)
            || ((d >= 8 && d <= 14) && w == Monday && m == October)
            // Veteran's Day (Monday if Sunday or Friday if Saturday)
            || ((d == 11 || (d == 12 && w == Monday) ||
				 (d == 10 && w == Friday)) && m == November)
            // Thanksgiving Day (fourth Thursday in November)
            || ((d >= 22 && d <= 28) && w == Thursday && m == November)
            // Christmas (Monday if Sunday or Friday if Saturday)
            || ((d == 25 || (d == 26 && w == Monday) ||
                 (d == 24 && w == Friday)) && m == December))
            return false;
        return true;
    }


    bool UnitedStates::ExchangeImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if ((w == Saturday || w == Sunday)
            // New Year's Day (possibly moved to Monday if on Sunday)
            || ((d == 1 || (d == 2 && w == Monday)) && m == January)
            // Martin Luther King's birthday (third Monday in January)
            || ((d >= 15 && d <= 21) && w == Monday && m == January)
            // Washington's birthday (third Monday in February)
            || ((d >= 15 && d <= 21) && w == Monday && m == February)
            // Good Friday
            || (dd == em-3)
            // Memorial Day (last Monday in May)
            || (d >= 25 && w == Monday && m == May)
            // Independence Day (Monday if Sunday or Friday if Saturday)
            || ((d == 4 || (d == 5 && w == Monday) ||
                 (d == 3 && w == Friday)) && m == July)
            // Labor Day (first Monday in September)
            || (d <= 7 && w == Monday && m == September)
            // Thanksgiving Day (fourth Thursday in November)
            || ((d >= 22 && d <= 28) && w == Thursday && m == November)
            // Christmas (Monday if Sunday or Friday if Saturday)
            || ((d == 25 || (d == 26 && w == Monday) ||
                 (d == 24 && w == Friday)) && m == December))
            return false;
        return true;
    }


    bool UnitedStates::GovernmentBondImpl::isBusinessDay(const Date& date)
                                                                      const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if ((w == Saturday || w == Sunday)
            // New Year's Day (possibly moved to Monday if on Sunday)
            || ((d == 1 || (d == 2 && w == Monday)) && m == January)
            // Martin Luther King's birthday (third Monday in January)
            || ((d >= 15 && d <= 21) && w == Monday && m == January)
            // Washington's birthday (third Monday in February)
            || ((d >= 15 && d <= 21) && w == Monday && m == February)
            // Good Friday
            || (dd == em-3)
            // Memorial Day (last Monday in May)
            || (d >= 25 && w == Monday && m == May)
            // Independence Day (Monday if Sunday or Friday if Saturday)
            || ((d == 4 || (d == 5 && w == Monday) ||
                 (d == 3 && w == Friday)) && m == July)
            // Labor Day (first Monday in September)
            || (d <= 7 && w == Monday && m == September)
            // Columbus Day (second Monday in October)
            || ((d >= 8 && d <= 14) && w == Monday && m == October)
            // Veteran's Day (Monday if Sunday or Friday if Saturday)
            || ((d == 11 || (d == 12 && w == Monday) ||
				 (d == 10 && w == Friday)) && m == November)
            // Thanksgiving Day (fourth Thursday in November)
            || ((d >= 22 && d <= 28) && w == Thursday && m == November)
            // Christmas (Monday if Sunday or Friday if Saturday)
            || ((d == 25 || (d == 26 && w == Monday) ||
                 (d == 24 && w == Friday)) && m == December))
            return false;
        return true;
    }

}
