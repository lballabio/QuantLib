
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

/*! \file westerncalendar.cpp
	\brief Western calendar

	$Source$
	$Log$
	Revision 1.15  2001/04/12 08:58:54  lballabio
	Fixed offset in Easter mondays

	Revision 1.14  2001/04/09 14:13:33  nando
	all the *.hpp moved below the Include/ql level
	
	Revision 1.13  2001/04/06 18:46:21  nando
	changed Authors, Contributors, Licence and copyright header

	Revision 1.12  2001/04/04 12:13:23  nando
	Headers policy part 2:
	The Include directory is added to the compiler's include search path.
	Then both your code and user code specifies the sub-directory in
	#include directives, as in
	#include <Solvers1d/newton.hpp>

	Revision 1.11  2001/04/04 11:07:24  nando
	Headers policy part 1:
	Headers should have a .hpp (lowercase) filename extension
	All *.h renamed to *.hpp

	Revision 1.10  2001/03/26 08:47:00  lballabio
	Fixed line wrappings

	Revision 1.9  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks

*/

#include "ql/Calendars/westerncalendar.hpp"

namespace QuantLib {

	namespace Calendars {

		const Day WesternCalendar::easterMonday[] = {
			107,  98,  90, 103,  95, 114, 106,  91, 111, 102,		// 1900-1909
			 87, 107,  99,  83, 103,  95, 115,  99,  91, 111,		// 1910-1919
			 96,  87, 107,  92, 112, 103,  95, 108, 100,  91,		// 1920-1929
			111,  96,  88, 107,  92, 112, 104,  88, 108, 100,		// 1930-1939
			 85, 104,  96, 116, 101,  92, 112,  97,  89, 108,		// 1940-1949
			100,  85, 105,  96, 109, 101,  93, 112,  97,  89,		// 1950-1959
			109,  93, 113, 105,  90, 109, 101,  86, 106,  97,		// 1960-1969
			 89, 102,  94, 113, 105,  90, 110, 101,  86, 106,		// 1970-1979
			 98, 110, 102,  94, 114,  98,  90, 110,  95,  86,		// 1980-1989
			106,  91, 111, 102,  94, 107,  99,  90, 103,  95,		// 1990-1999
			115, 106,  91, 111, 103,  87, 107,  99,  84, 103,		// 2000-2009
			 95, 115, 100,  91, 111,  96,  88, 107,  92, 112,		// 2010-2019
			104,  95, 108, 100,  92, 111,  96,  88, 108,  92,		// 2020-2029
			112, 104,  89, 108, 100,  85, 105,  96, 116, 101,		// 2030-2039
			 93, 112,  97,  89, 109, 100,  85, 105,  97, 109,		// 2040-2049
			101,  93, 113,  97,  89, 109,  94, 113, 105,  90,		// 2050-2059
			110, 101,  86, 106,  98,  89, 102,  94, 114, 105,		// 2060-2069
			 90, 110, 102,  86, 106,  98, 111, 102,  94, 107,		// 2070-2079
			 99,  90, 110,  95,  87, 106,  91, 111, 103,  94,		// 2080-2089
			107,  99,  91, 103,  95, 115, 107,  91, 111, 103 };		// 2090-2099

	}

}

