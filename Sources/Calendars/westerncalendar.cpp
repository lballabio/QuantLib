
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

/*! \file westerncalendar.cpp
	\brief Western calendar

	$Source$
	$Log$
	Revision 1.10  2001/03/26 08:47:00  lballabio
	Fixed line wrappings

	Revision 1.9  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#include "westerncalendar.h"

namespace QuantLib {

	namespace Calendars {
	
		const Day WesternCalendar::easterMonday[] = {
			106,  97,  89, 102,  94, 113, 105,  90, 110, 101,		// 1900-1909
			 86, 106,  98,  82, 102,  94, 114,  98,  90, 110,		// 1910-1919
			 95,  86, 106,  91, 111, 102,  94, 107,  99,  90,		// 1920-1929
			110,  95,  87, 106,  91, 111, 103,  87, 107,  99,		// 1930-1939
			 84, 103,  95, 115, 100,  91, 111,  96,  88, 107,		// 1940-1949
			 99,  84, 104,  95, 108, 100,  92, 111,  96,  88,		// 1950-1959
			108,  92, 112, 104,  89, 108, 100,  85, 105,  96,		// 1960-1969
			 88, 101,  93, 112, 104,  89, 109, 100,  85, 105,		// 1970-1979
			 97, 109, 101,  93, 113,  97,  89, 109,  94,  85,		// 1980-1989
			105,  90, 110, 101,  93, 106,  98,  89, 102,  94,		// 1990-1999
			114, 105,  90, 110, 102,  86, 106,  98,  83, 102,		// 2000-2009
			 94, 114,  99,  90, 110,  95,  87, 106,  91, 111,		// 2010-2019
			103,  94, 107,  99,  91, 110,  95,  87, 107,  91,		// 2020-2029
			111, 103,  88, 107,  99,  84, 104,  95, 115, 100,		// 2030-2039
			 92, 111,  96,  88, 108,  99,  84, 104,  96, 108,		// 2040-2049
			100,  92, 112,  96,  88, 108,  93, 112, 104,  89,		// 2050-2059
			109, 110,  85, 105,  97,  88, 101,  93, 113, 104,		// 2060-2069
			 89, 109, 101,  85, 105,  97, 110, 101,  93, 106,		// 2070-2079
			 98,  89, 109,  94,  86, 105,  90, 110, 102,  93,		// 2080-2089
			106,  98,  90, 102,  94, 114, 106,  90, 110, 102 };		// 2090-2099
	
	}

}

