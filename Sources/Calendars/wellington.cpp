
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

/*! \file wellington.cpp
	\brief Wellington calendar

	$Source$
	$Log$
	Revision 1.5  2001/04/09 14:13:33  nando
	all the *.hpp moved below the Include/ql level

	Revision 1.4  2001/04/06 18:46:21  nando
	changed Authors, Contributors, Licence and copyright header

	Revision 1.3  2001/04/04 12:13:23  nando
	Headers policy part 2:
	The Include directory is added to the compiler's include search path.
	Then both your code and user code specifies the sub-directory in
	#include directives, as in
	#include <Solvers1d/newton.hpp>

	Revision 1.2  2001/04/04 11:07:24  nando
	Headers policy part 1:
	Headers should have a .hpp (lowercase) filename extension
	All *.h renamed to *.hpp

	Revision 1.1  2001/03/26 09:59:35  lballabio
	Added Helsinki and Wellington calendars

*/

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
