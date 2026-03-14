/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 StatPro Italia srl
 Copyright (C) 2015 Dmitri Nesteruk

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

#include <ql/time/calendars/russia.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Russia::Russia(Russia::Market market) {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> settlementImpl(
                                                  new Russia::SettlementImpl);
        static ext::shared_ptr<Calendar::Impl> exchangeImpl(
                                                    new Russia::ExchangeImpl);

        switch (market) {
          case Settlement:
            impl_ = settlementImpl;
            break;
          case MOEX:
            impl_ = exchangeImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    namespace {

        bool isExtraHolidaySettlementImpl(Day d, Month month, Year year) {
            switch (year) {
              case 2017:
                switch (month) {
                  case February: return d == 24;
                  case May:      return d == 8;
                  case November: return d == 6;
                  default:       return false;
                }
              case 2018:
                switch (month) {
                  case March:     return d == 9;
                  case April:     return d == 30;
                  case May:       return d == 2;
                  case June:      return d == 11;
                  case December:  return d == 31;
                  default:        return false;
                }
              case 2019:
                switch (month) {
                  case May: return d == 2 || d == 3 || d == 10;
                  default: return false;
                }
              case 2020:
                switch (month) {
                  case March:     return d == 30 || d == 31;
                  case April:     return d == 1 || d == 2 || d == 3;
                  case May:       return d == 4 || d == 5;
                  default:        return false;
                }
              default:
                return false;
            }
        }

    }

    bool Russia::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        if (isWeekend(w)
            // New Year's holidays
            || (y <= 2005 && d <= 2 && m == January)
            || (y >= 2005 && d <= 5 && m == January)
            // in 2012, the 6th was also a holiday
            || (y == 2012 && d == 6 && m == January)
            // Christmas (possibly moved to Monday)
            || ((d == 7 || ((d == 8 || d == 9) && w == Monday)) &&
                m == January)
            // Defender of the Fatherland Day (possibly moved to Monday)
            || ((d == 23 || ((d == 24 || d == 25) && w == Monday)) &&
                m == February)
            // International Women's Day (possibly moved to Monday)
            || ((d == 8 || ((d == 9 || d == 10) && w == Monday)) &&
                m == March)
            // Labour Day (possibly moved to Monday)
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) &&
                m == May)
            // Victory Day (possibly moved to Monday)
            || ((d == 9 || ((d == 10 || d == 11) && w == Monday)) &&
                m == May)
            // Russia Day (possibly moved to Monday)
            || ((d == 12 || ((d == 13 || d == 14) && w == Monday)) &&
                m == June)
            // Unity Day (possibly moved to Monday)
            || ((d == 4 || ((d == 5 || d == 6) && w == Monday)) &&
                m == November))
            return false; // NOLINT(readability-simplify-boolean-expr)

        if (isExtraHolidaySettlementImpl(d,m,y))
            return false;

        return true;
    }

    namespace {

        bool isWorkingWeekend(Day d, Month month, Year year) {
            switch (year) {
              case 2012:
                switch (month) {
                  case March: return d == 11;
                  case April: return d == 28;
                  case May:   return d == 5 || d == 12;
                  case June:  return d == 9;
                  default:    return false;
                }
              case 2016:
                switch (month)
                {
                case February: return d == 20;
                default: return false;
                }
              case 2018:
                switch (month) {
                  case April: return d == 28;
                  case June: return d == 9;
                  case December: return d == 29;
                  default: return false;
                }
              default:
                return false;
            }
        }

        bool isExtraHolidayExchangeImpl(Day d, Month month, Year year) {
            switch (year) {
              case 2012:
                switch (month) {
                  case January: return d == 2;
                  case March:   return d == 9;
                  case April:   return d == 30;
                  case June:    return d == 11;
                  default:      return false;
                }
              case 2013:
                switch (month) {
                  case January: return d == 1 || d == 2 || d == 3
                                    || d == 4 || d == 7;
                  default:      return false;
                }
              case 2014:
                switch (month) {
                  case January: return d == 1 || d == 2 || d == 3 || d == 7;
                  default:      return false;
                }
              case 2015:
                switch (month) {
                  case January: return d == 1 || d == 2 || d == 7;
                  default:      return false;
                }
              case 2016:
                switch (month)
                {
                case January: return d == 1 || d == 7 || d == 8;
                case May:     return d == 2 || d == 3;
                case June:    return d == 13;
                case December: return d == 30;
                default:      return false;
                }
              case 2017:
                switch (month) {
                  case January: return d == 2;
                  case May:     return d == 8;
                  default:      return false;
                }
              case 2018:
                switch (month) {
                  case January:   return d == 1 || d == 2 || d == 8;
                  case December:  return d == 31;
                  default:        return false;
                }
              case 2019:
                switch (month) {
                case January:   return d == 1 || d == 2 || d == 7;
                case December:  return d == 31;
                default:        return false;
                }
              case 2020:
                switch (month) {
                  case January:   return d == 1 || d == 2 || d == 7;
                  case February:  return d == 24;
                  case June:      return d == 24;
                  case July:      return d == 1;
                  default:        return false;
                }
              default:
                return false;
            }
        }

    }

    bool Russia::ExchangeImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        // the exchange was formally established in 2011, so data are only
        // available from 2012 to present
        if (y < 2012)
            QL_FAIL("MOEX calendar for the year " << y
                    << " does not exist.");

        if (isWorkingWeekend(d,m,y))
            return true;

        // Known holidays
        if (isWeekend(w)
            // Defender of the Fatherland Day
            || (d == 23 && m == February)
            // International Women's Day (possibly moved to Monday)
            || ((d == 8 || ((d == 9 || d == 10) && w == Monday)) && m == March)
            // Labour Day
            || (d == 1 && m == May)
            // Victory Day (possibly moved to Monday)
            || ((d == 9 || ((d == 10 || d == 11) && w == Monday)) && m == May)
            // Russia Day
            || (d == 12 && m == June)
            // Unity Day (possibly moved to Monday)
            || ((d == 4 || ((d == 5 || d == 6) && w == Monday))
                && m == November)
            // New Years Eve
            || (d == 31 && m == December))
            return false;

        if (isExtraHolidayExchangeImpl(d,m,y))
            return false;

        return true;
    }

}
