
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

/*! \file newyork.cpp
	\brief New York calendar

	$Source$
	$Log$
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
	
	Revision 1.13  2001/03/26 08:51:49  lballabio
	really fixed line wrappings
	
	Revision 1.12  2001/03/26 08:47:00  lballabio
	Fixed line wrappings
	
	Revision 1.11  2001/01/04 20:09:31  nando
	few changes: enumerations, tab/spaces, more checks, python test, bug fixed
	
	Revision 1.10  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#include "Calendars/newyork.hpp"

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
