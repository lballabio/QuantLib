
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

/*! \file milan.cpp
  	\brief Milan calendar

    \fullpath
    ql/Calendars/%milan.cpp
*/

// $Id$

#include "ql/Calendars/milan.hpp"

namespace QuantLib {

	namespace Calendars {

		bool Milan::isBusinessDay(const Date& date) const {
			Weekday w = date.weekday();
			Day d = date.dayOfMonth(), dd = date.dayOfYear();
			Month m = date.month();
			Year y = date.year();
			if ((w == Saturday || w == Sunday)
				// New Year's Day
				|| (d == 1 && m == January)
				// Epiphany
				|| (d == 6 && m == January)
				// Easter Monday
				|| (dd == easterMonday[y-1900])
				// Liberation Day
				|| (d == 25 && m == April)
				// Labour Day
				|| (d == 1 && m == May)
				// Assumption
				|| (d == 15 && m == August)
				// All Saints' Day
				|| (d == 1 && m == November)
				// Immaculate Conception
				|| (d == 8 && m == December)
				// Christmas
				|| (d == 25 && m == December)
				// St. Stephen
				|| (d == 26 && m == December)
				// December 31st, 1999 only
				|| (d == 31 && m == December && y == 1999))
					return false;
			return true;
		}

	}

}

