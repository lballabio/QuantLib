
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

/*! \file london.cpp
	\brief London calendar

    \fullpath
    Calendars/%london.cpp
*/

// $Id$

#include "ql/Calendars/london.hpp"

namespace QuantLib {

	namespace Calendars {

		bool London::isBusinessDay(const Date& date) const {
			Weekday w = date.weekday();
			Day d = date.dayOfMonth(), dd = date.dayOfYear();
			Month m = date.month();
			Year y = date.year();
			if ((w == Saturday || w == Sunday)
				// New Year's Day (possibly moved to Monday)
				|| ((d == 1 || ((d == 2 || d == 3) && w == Monday)) &&
				    m == January)
				// Good Friday
				|| (dd == easterMonday[y-1900]-3)
				// Easter Monday
				|| (dd == easterMonday[y-1900])
				// first Monday of May (Bank Holiday)
				|| (d <= 7 && w == Monday && m == May)
				// last Monday of May or August (Bank Holidays)
				|| (d >= 25 && w == Monday && (m == May || m == August))
				// Christmas (possibly moved to Monday or Tuesday)
				|| ((d == 25 || (d == 27 && (w == Monday || w == Tuesday))) &&
				    m == December)
				// Boxing Day (possibly moved to Monday or Tuesday)
				|| ((d == 26 || (d == 28 && (w == Monday || w == Tuesday))) &&
				    m == December)
				// December 31st, 1999 only
				|| (d == 31 && m == December && y == 1999))
					return false;
			return true;
		}

	}

}

