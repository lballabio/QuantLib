
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

/*! \file helsinki.cpp
	\brief Helsinki calendar

	$Source$
	$Log$
	Revision 1.2  2001/04/04 11:07:24  nando
	Headers policy part 1:
	Headers should have a .hpp (lowercase) filename extension
	All *.h renamed to *.hpp

	Revision 1.1  2001/03/26 09:59:35  lballabio
	Added Helsinki and Wellington calendars
	
*/

#include "helsinki.hpp"

namespace QuantLib {

	namespace Calendars {
	
		bool Helsinki::isBusinessDay(const Date& date) const {
			Weekday w = date.weekday();
			Day d = date.dayOfMonth(), dd = date.dayOfYear();
			Month m = date.month();
			Year y = date.year()-1900;
			if ((w == Saturday || w == Sunday)
				// New Year's Day 
				|| (d == 1 && m == January)
				// Epiphany
				|| (d == 6 && m == January)
				// Good Friday
				|| (dd == easterMonday[y]-3)
				// Easter Monday
				|| (dd == easterMonday[y])
				// Ascension Thursday
				|| (dd == easterMonday[y]+38)
				// Labour Day
				|| (d == 1 && m == May)
				// Midsummer Eve
				|| (d == 21 && m == June)
				// Independence Day
				|| (d == 6 && m == December)
				// Christmas Eve
				|| (d == 24 && m == December)
				// Christmas
				|| (d == 25 && m == December)
				// Boxing Day
				|| (d == 26 && m == December))
					return false;
			return true;
		}
	
	}

}

