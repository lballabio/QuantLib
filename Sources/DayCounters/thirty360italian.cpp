
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file thirty360italian.cpp
	\brief 30/360 italian day counter

	$Source$
	$Name$
	$Log$
	Revision 1.9  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks

*/

#include "thirty360italian.h"

namespace QuantLib {

	namespace DayCounters {
	
		int Thirty360Italian::dayCount(const Date& d1, const Date& d2) const {
			int gg1 = d1.dayOfMonth(), gg2 = d2.dayOfMonth();
			if (d1.month() == 2 && gg1 > 27)
				gg1 = 30;
			if (d2.month() == 2 && gg2 > 27)
				gg2 = 30;
			return 360*(d2.year()-d1.year()) + 30*(d2.month()-d1.month()-1) + QL_MAX(0,30-gg1) + QL_MIN(30,gg2);
		}
	
	}

}
