
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file target.cpp
	\brief TARGET calendar

    \fullpath
    ql/Calendars/%target.cpp
*/

// $Id$

#include "ql/Calendars/target.hpp"

namespace QuantLib {

	namespace Calendars {

		bool TARGET::isBusinessDay(const Date& date) const {
			Weekday w = date.weekday();
			Day d = date.dayOfMonth(), dd = date.dayOfYear();
			Month m = date.month();
			Year y = date.year();
			if ((w == Saturday || w == Sunday)
				// New Year's Day
				|| (d == 1  && m == January)
				// Good Friday
				|| (dd == easterMonday[y-1900]-3)
				// Easter Monday
				|| (dd == easterMonday[y-1900])
				// Labour Day
				|| (d == 1  && m == May)
				// Christmas
				|| (d == 25 && m == December)
				// Day of Goodwill
				|| (d == 26 && m == December)
				// December 31st, 1998 and 1999 only
				|| (d == 31 && m == December && (y == 1998 || y == 1999)))
					return false;
			return true;
		}

	}

}

