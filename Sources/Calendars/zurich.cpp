
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

/*! \file zurich.cpp
	\brief Zurich calendar

	$Source$
	$Log$
	Revision 1.10  2001/05/09 11:06:19  nando
	A few comments modified/removed

	Revision 1.9  2001/04/09 14:13:33  nando
	all the *.hpp moved below the Include/ql level

	Revision 1.8  2001/04/06 18:46:21  nando
	changed Authors, Contributors, Licence and copyright header

	Revision 1.7  2001/04/04 12:13:23  nando
	Headers policy part 2:
	The Include directory is added to the compiler's include search path.
	Then both your code and user code specifies the sub-directory in
	#include directives, as in
	#include <Solvers1d/newton.hpp>

	Revision 1.6  2001/04/04 11:07:24  nando
	Headers policy part 1:
	Headers should have a .hpp (lowercase) filename extension
	All *.h renamed to *.hpp

	Revision 1.5  2001/03/26 08:47:00  lballabio
	Fixed line wrappings

*/

#include "ql/Calendars/zurich.hpp"

namespace QuantLib {

	namespace Calendars {

		bool Zurich::isBusinessDay(const Date& date) const {
			Weekday w = date.weekday();
			Day d = date.dayOfMonth(), dd = date.dayOfYear();
			Month m = date.month();
			Year y = date.year();
			if ((w == Saturday || w == Sunday)
				// New Year's Day
				|| (d == 1  && m == January)
				// Berchtoldstag
				|| (d == 2  && m == January)
				// Good Friday
				|| (dd == easterMonday[y-1900]-3)
				// Easter Monday
				|| (dd == easterMonday[y-1900])
				// Ascension Day
				|| (d == easterMonday[y-1900]+38)
				// Whit Monday
				|| (d == easterMonday[y-1900]+49)
				// Labour Day
				|| (d == 1  && m == May)
				// National Day
				|| (d == 1  && m == August)
				// Christmas
				|| (d == 25 && m == December)
				// St. Stephen's Day
				|| (d == 26 && m == December))
					return false;
			return true;
		}

	}

}

