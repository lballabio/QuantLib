
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

/*! \file wellington.cpp
	\brief Wellington calendar

    \fullpath
    ql/Calendars/%wellington.cpp
*/

// $Id$

#include "ql/Calendars/wellington.hpp"

namespace QuantLib {

	namespace Calendars {

		bool Wellington::isBusinessDay(const Date& date) const {
			Weekday w = date.weekday();
			Day d = date.dayOfMonth(), dd = date.dayOfYear();
			Month m = date.month();
			Year y = date.year();
			if ((w == Saturday || w == Sunday)
				// New Year's Day (possibly moved to Monday or Tuesday)
				|| ((d == 1 || (d == 3 && (w == Monday || w == Tuesday))) &&
				    m == January)
				// Day after New Year's Day (possibly Monday or Tuesday)
				|| ((d == 2 || (d == 4 && (w == Monday || w == Tuesday))) &&
				    m == January)
                // Anniversary Day, Monday nearest January 22nd
                || ((d >= 19 && d <= 25) && w == Monday && m == January)
                // Waitangi Day. February 6th
                || (d == 6 && m == February)
				// Good Friday
				|| (dd == easterMonday[y-1900]-3)
				// Easter Monday
				|| (dd == easterMonday[y-1900])
                // ANZAC Day. April 25th
                || (d == 25 && m == April)
                // Queen's Birthday, first Monday in June
				|| (d <= 7 && w == Monday && m == June)
                // Labour Day, fourth Monday in October
				|| ((d <= 22 && d >= 28) && w == Monday && m == October)
                // Christmas, December 25th (possibly Monday or Tuesday)
                || ((d == 25 || (d == 27 && (w == Monday || w == Tuesday))) &&
                    m == December)
                // Boxing Day, December 26th (possibly Monday or Tuesday)
                || ((d == 26 || (d == 28 && (w == Monday || w == Tuesday))) &&
                    m == December))
					return false;
			return true;
		}

	}

}
