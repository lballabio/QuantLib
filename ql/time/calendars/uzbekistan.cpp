/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Paolo Mazzocchi
 Copyright (C) 2015 Riccardo Barone

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

#include <ql/errors.hpp>
#include <ql/time/calendars/uzbekistan.hpp>
#include <algorithm>

namespace QuantLib {

	bool isRamazonHayit(const Date& d) {
		// Eid al-Fitr (Ramazon Hayit) dates
		static std::vector<Date> RamazonHayit = {
			Date(20, March,    2026),
			Date(10, March,    2027),
			Date(27, February, 2028),
			Date(15, February, 2029),
			Date(5, February, 2030),
			Date(25, January,  2031),
			Date(14, January,  2032),
			Date(2, January,  2033),
			Date(23, December, 2033),  // two Eids in 2033
			Date(12, December, 2034),
			Date(1, December, 2035),
			Date(19, November, 2036),
			Date(8, November, 2037),
			Date(29, October,  2038),
			Date(19, October,  2039),
			Date(7, October,  2040)
		};

		return std::any_of(RamazonHayit.begin(), RamazonHayit.end(),
			[&d](const Date& p) { return d == p; });
	}

	bool isQurbonHayit(const Date& d) {
		// Eid al-Adha (Qurbon Hayit) dates
		static std::vector<Date> QurbonHayit = {
			Date(27, May,      2026),
			Date(17, May,      2027),
			Date(5, May,      2028),
			Date(24, April,    2029),
			Date(13, April,    2030),
			Date(3, April,    2031),
			Date(22, March,    2032),
			Date(11, March,    2033),
			Date(28, February, 2034),
			Date(18, February, 2035),
			Date(7, February, 2036),
			Date(27, January,  2037),
			Date(16, January,  2038),
			Date(5, January,  2039),
			Date(26, December, 2039),  // two Eids in 2039
			Date(15, December, 2040)
		};

		return std::any_of(QurbonHayit.begin(), QurbonHayit.end(),
			[&d](const Date& p) { return d == p; });
	}

	Uzbekistan::Uzbekistan(Market) {
		static ext::shared_ptr<Calendar::Impl> impl(new Uzbekistan::Impl);
		impl_ = impl;
	}

	bool Uzbekistan::Impl::isBusinessDay(const Date& date) const {
		Weekday w = date.weekday();
		Day d = date.dayOfMonth();
		Month m = date.month();
		Year y = date.year();

		if (isWeekend(w)
			|| isRamazonHayit(date)
			|| isQurbonHayit(date)
			// New Year's Day
			|| (d == 1 && m == January)
			// International Womens Day
			|| (d == 8 && m == March)
			// Navruz(Persian New Year)
			|| (d == 21 && m == March)
			// Day of Remembrance and Honors
			|| (d == 9 && m == May)
			// Independence Day
			|| (d == 1 && m == September)
			// Teachers Day
			|| (d == 1 && m == October)
			// Constitution Day
			|| (d == 8 && m == December)) {
			return false;
		}
		return true;
	}

}
