
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

/*! \file newyork.cpp
	\brief New York calendar

    $Id$
*/

// $Source$
// $Log$
// Revision 1.22  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.21  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.20  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Calendars/newyork.hpp"

namespace QuantLib {

	namespace Calendars {

		bool NewYork::isBusinessDay(const Date& date) const {
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
				// Christmas (moved to Monday if Sunday or Friday if Saturday)
				|| ((d == 25 || (d == 26 && w == Monday) ||
				    (d == 24 && w == Friday)) && m == December))
					return false;
			return true;
		}

	}

}
