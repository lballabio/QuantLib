
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file london.cpp
	\brief London calendar

	$Source$
	$Log$
	Revision 1.17  2001/04/09 14:13:33  nando
	all the *.hpp moved below the Include/ql level

	Revision 1.16  2001/04/06 18:46:21  nando
	changed Authors, Contributors, Licence and copyright header

	Revision 1.15  2001/04/04 12:13:23  nando
	Headers policy part 2:
	The Include directory is added to the compiler's include search path.
	Then both your code and user code specifies the sub-directory in
	#include directives, as in
	#include <Solvers1d/newton.hpp>

	Revision 1.14  2001/04/04 11:07:24  nando
	Headers policy part 1:
	Headers should have a .hpp (lowercase) filename extension
	All *.h renamed to *.hpp

	Revision 1.13  2001/03/26 09:13:05  lballabio
	big fixed

	Revision 1.12  2001/03/26 08:51:49  lballabio
	really fixed line wrappings

	Revision 1.11  2001/03/26 08:47:00  lballabio
	Fixed line wrappings

	Revision 1.10  2001/01/04 20:09:31  nando
	few changes: enumerations, tab/spaces, more checks, python test, bug fixed

	Revision 1.9  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks

*/

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

