
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file frankfurt.cpp
	\brief Frankfurt calendar

	$Source$
	$Log$
	Revision 1.13  2001/04/04 12:13:23  nando
	Headers policy part 2:
	The Include directory is added to the compiler's include search path.
	Then both your code and user code specifies the sub-directory in
	#include directives, as in
	#include <Solvers1d/newton.hpp>

	Revision 1.12  2001/04/04 11:07:24  nando
	Headers policy part 1:
	Headers should have a .hpp (lowercase) filename extension
	All *.h renamed to *.hpp
	
	Revision 1.11  2001/03/26 08:47:00  lballabio
	Fixed line wrappings
	
	Revision 1.10  2001/01/04 20:09:31  nando
	few changes: enumerations, tab/spaces, more checks, python test, bug fixed
	
	Revision 1.9  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#include "Calendars/frankfurt.hpp"

namespace QuantLib {

	namespace Calendars {
	
		bool Frankfurt::isBusinessDay(const Date& date) const {
			Weekday w = date.weekday();
			Day d = date.dayOfMonth(), dd = date.dayOfYear();
			Month m = date.month();
			Year y = date.year()-1900;
			if ((w == Saturday || w == Sunday)
				// New Year's Day 
				|| (d == 1 && m == January)
				// Good Friday
				|| (dd == easterMonday[y]-3)
				// Easter Monday
				|| (dd == easterMonday[y])
				// Ascension Thursday
				|| (dd == easterMonday[y]+38)
				// Whit Monday
				|| (dd == easterMonday[y]+49)
				// Corpus Christi
				|| (dd == easterMonday[y]+59)
				// Labour Day
				|| (d == 1 && m == May)
				// National Day
				|| (d == 3 && m == October)
				// Christmas Eve
				|| (d == 24 && m == December)
				// Christmas
				|| (d == 25 && m == December)
				// Boxing Day
				|| (d == 26 && m == December)
				// New Year's Eve 
				|| (d == 31 && m == December))
					return false;
			return true;
		}
	
	}

}

