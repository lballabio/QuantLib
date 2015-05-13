/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 StatPro Italia srl

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

#include <ql/time/calendars/russia.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Russia::Russia(Russia::Market market) {
        // all calendar instances share the same implementation
        // instance
        static boost::shared_ptr<Calendar::Impl> settlementImpl(
                                                  new Russia::SettlementImpl);
        static boost::shared_ptr<Calendar::Impl> exchangeImpl(
          new Russia::ExchangeImpl);
        
        switch (market)
        {
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


    bool Russia::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        if (isWeekend(w)
            // New Year's holidays
            || (d >= 1 && d <= 8 && m == January)
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
            return false;
        return true;
    }

  bool Russia::ExchangeImpl::isHoliday2015(const Date& date) const
  {
	  Month month = date.month();
	  Day d = date.dayOfMonth();
	  Weekday wd = date.weekday();

	  if (wd == Saturday || wd == Sunday)
	  {
		  return true;
	  }
	  else {
		  switch (month) {
		  case Jan: return d != 1 && d != 2 && d != 7;
		  case Feb: return d != 23;
		  case Mar: return d != 9;
		  case May: return d != 1 && d != 4 && d != 11;
		  case Jun: return d != 12;
		  case Nov: return d != 4;
		  case Dec: return d != 31;
		  default: return false;
		  }
	  }

  }

  bool Russia::ExchangeImpl::isHoliday2014(const Date& date) const
  {
	  Month month = date.month();
	  Day d = date.dayOfMonth();
	  Weekday wd = date.weekday();

	  if (wd == Saturday || wd == Sunday)
	  {
		  return true;
	  }
	  else {
		  switch (month) {
		  case Jan: return d != 1 && d != 2 && d != 3 && d != 7;
		  case Mar: return d != 10;
		  case May: return d != 1 && d != 9;
		  case Jun: return d != 12;
		  case Nov: return d != 4;
		  case Dec: return d != 31;
		  default: return false;
		  }
	  }

  }

  bool Russia::ExchangeImpl::isHoliday2013(const Date& date) const
  {
	  Month month = date.month();
	  Day d = date.dayOfMonth();
	  Weekday wd = date.weekday();

	  if (wd == Saturday || wd == Sunday)
	  {
		  return true;
	  }
	  else {
		  switch (month) {
		  case Jan: return d != 1 && d != 2 && d != 3 && d != 4 && d != 7;
		  case Mar: return d != 8;
		  case May: return d != 1 && d != 9;
		  case Jun: return d != 12;
		  case Nov: return d != 4;
		  case Dec: return d != 31;
		  default: return false;
		  }
	  }

  }

  bool Russia::ExchangeImpl::isHoliday2012(const Date& date) const
  {
	  Month month = date.month();
	  Day d = date.dayOfMonth();
	  Weekday wd = date.weekday();

	  if (wd == Saturday || wd == Sunday)
	  {
		  switch (month)
		  {
		  case Mar: return d != 11;
		  case Apr: return d != 28;
		  case May: return d != 5 && d != 12;
		  case Jun: return d != 9;
		  default: return true;
		  }
	  }
	  else {
		  switch (month) {
		  case Jan: return d != 2;
		  case Feb: return d != 23;
		  case Mar: return d != 8 && d != 9;
		  case Apr: return d != 30;
		  case May: return d != 1 && d != 9;
		  case Jun: return d != 11 && d != 12;
		  case Nov: return d != 5;
		  case Dec: return d != 31;
		  default: return false;
		  }
	  }

  }

  bool Russia::ExchangeImpl::isBusinessDay(const Date& date) const
  {
    // the exchange was formally established 2011, so data is only available
    // from 2012 to present
    auto year = date.year();

    switch (year)
    {
    case 2012: return !isHoliday2012(date);
    case 2013: return !isHoliday2013(date);
    case 2014: return !isHoliday2014(date);
    case 2015: return !isHoliday2015(date);
    default: 
      std::ostringstream oss;
      oss << "MOEX calendar for the year " << year << " does not exist.";
      QL_FAIL(oss.str());
    }
  }
}
