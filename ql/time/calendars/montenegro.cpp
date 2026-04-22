/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/time/calendars/montenegro.hpp>

namespace QuantLib {

	Montenegro::Montenegro(Market) {
		static ext::shared_ptr<Calendar::Impl> impl(new Montenegro::MnseImpl);
		impl_ = impl;
	}

	bool Montenegro::MnseImpl::isBusinessDay(const Date& date) const {
		Weekday w = date.weekday();
		Day d = date.dayOfMonth();
		Month m = date.month();
		Year y = date.year();
		Day em = easterMonday(y);
		Day gf = em - 3; // Good Friday
		Date jul13(y, July, 13);
		Date jul14(y, July, 14);
		Date jul15(y, July, 15);
		Date may21(y, May, 21);
		Weekday wd21 = may21.weekday();

		// Weekday logic
		bool isJul13Weekend = isWeekend(jul13.weekday());
		bool isJul14Weekend = isWeekend(jul14.weekday());

		if (isWeekend(w)
			// New Year and Orthodox Christmas period: Jan 1-8
			|| (m == January && d >= 1 && d <= 8)
			// Good Friday (Orthodox calendar)
			|| (date.dayOfYear() == gf)
			// Easter Monday
			|| (date.dayOfYear() == em)
			// Labour Day: May 1-2
			|| (d == 1 && m == May) ||
			(d == 2 && m == May)
			// Independence Day: May 21-22
			|| (d == 21 && m == May) || (d == 22 && m == May) ||
			// Shift to 22-23 if 21st is a Sunday
			(d == 22 && m == May && wd21 == Sunday) || (d == 23 && m == May && wd21 == Sunday) ||
			// Statehood Day: July 14-15 (per updated calendar)
			// Normal fixed case: 13 and/or 14 July
			(d == 13 && m == July && !isJul13Weekend) ||
			(d == 14 && m == July && !isJul14Weekend) ||
			// Special case: both fall on weekend ? Monday 15th becomes observed holiday
			(d == 15 && m == July && isJul13Weekend && isJul14Weekend)
			// Njegos Day: Nov 13-14
			|| (d == 13 && m == November) || (d == 14 && m == November)) {
			return false;
		}

		return true;
	}

}