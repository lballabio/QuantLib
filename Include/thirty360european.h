
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

#ifndef quantlib_thirty360european_day_counter_h
#define quantlib_thirty360european_day_counter_h

#include "qldefines.h"
#include "daycounter.h"

namespace QuantLib {

	namespace DayCounters {
	
		class Thirty360European : public DayCounter {
		  public:
			std::string name() const { return std::string("30/360eu"); }
			int dayCount(const Date& d1, const Date& d2) const {
				return 360*(d2.year()-d1.year()) + 30*(d2.month()-d1.month()-1)
					+ QL_MAX(0,30-d1.dayOfMonth()) + QL_MIN(30,d2.dayOfMonth());
			}
			Time yearFraction(const Date& d1, const Date& d2, 
			  const Date& refPeriodStart = Date(), const Date& refPeriodEnd = Date()) const {
				return dayCount(d1,d2)/360.0;
			}
		};
	
	}

}


#endif
