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

#include <ql/time/calendars/northmacedonia.hpp>

namespace QuantLib {

	NorthMacedonia::NorthMacedonia(Market) {
		static ext::shared_ptr<Calendar::Impl> impl(new NorthMacedonia::MseImpl);
		impl_ = impl;
	}

	bool NorthMacedonia::MseImpl::isBusinessDay(const Date& date) const {
		Weekday w = date.weekday();
		Day d = date.dayOfMonth();
		Month m = date.month();
		Year y = date.year();

		// Observance rule: if a holiday falls on a Sunday, it is observed on Monday
		const Date prev = date - 1;
		const bool sundayObserved = (prev.weekday() == Sunday);

		// Holidays
		if (isWeekend(w)
			// January 1 – New Year
			|| (d == 1 && m == January)
			// January 6 – Epiphany
			|| (d == 6 && m == January)
			// January 7 – Orthodox Christmas
			|| (d == 7 && m == January)
			// March 31 – (Ramadan Bajram (Eid al-Fitr)
			|| (d == 31 && m == March)
			// April 18 – Good Friday
			|| (d == 18 && m == April)
			// April 21 – Easter Monday
			|| (d == 21 && m == April)
			// May 1 – Labour Day
			|| (d == 1 && m == May)
			// June 6 – religious holiday
			|| (d == 6 && m == June)
			// August 28 – religious holiday
			|| (d == 28 && m == August)
			// September 8 – Independence Day
			|| (d == 8 && m == September)
			// October 23 – Uprising Day
			|| (d == 23 && m == October)
			// December 8 – Saint Clement of Ohrid
			|| (d == 8 && m == December)
			// December 31 – Non-trading day
			|| (d == 31 && m == December)
			// Observed holidays (if the fixed holiday was Sunday)
			|| (sundayObserved && ((prev.dayOfMonth() == 1 && prev.month() == January) ||
			(prev.dayOfMonth() == 6 && prev.month() == January) ||
				(prev.dayOfMonth() == 7 && prev.month() == January) ||
				(prev.dayOfMonth() == 31 && prev.month() == March) ||
				(prev.dayOfMonth() == 18 && prev.month() == April) ||
				(prev.dayOfMonth() == 21 && prev.month() == April) ||
				(prev.dayOfMonth() == 1 && prev.month() == May) ||
				(prev.dayOfMonth() == 6 && prev.month() == June) ||
				(prev.dayOfMonth() == 28 && prev.month() == August) ||
				(prev.dayOfMonth() == 8 && prev.month() == September) ||
				(prev.dayOfMonth() == 23 && prev.month() == October) ||
				(prev.dayOfMonth() == 8 && prev.month() == December) ||
				(prev.dayOfMonth() == 31 && prev.month() == December)))) {
			return false;
		}

		return true;
	}

}
