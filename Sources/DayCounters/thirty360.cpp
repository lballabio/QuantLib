
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License: see the file LICENSE.TXT for details.
 * Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file.
 * LICENCE.TXT is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

#include "thirty360.h"

namespace QuantLib {

	namespace DayCounters {
	
		int Thirty360::dayCount(const Date& d1, const Date& d2) const {
			int dd2 = d2.dayOfMonth(), mm2 = d2.month();
			if (dd2 == 31 && d1.dayOfMonth() < 30){
				dd2 = 1;
				mm2++;
			}
			return 360*(d2.year()-d1.year()) + 30*(mm2-d1.month()-1) + QL_MAX(0,30-d1.dayOfMonth()) + QL_MIN(30,dd2);
		}
	
	}

}
