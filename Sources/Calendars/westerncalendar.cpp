
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

/*! \file westerncalendar.cpp
    \fullpath Sources/Calendars/%westerncalendar.cpp
	\brief Western calendar

*/

// $Id$
// $Log$
// Revision 1.21  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.20  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.19  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.18  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.17  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

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

