
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

/*! \file london.cpp
	\brief London calendar

	$Source$
	$Log$
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

#include "london.hpp"

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

