
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

#include "actualactual.h"
#include "thirty360european.h"
#include "null.h"
#include <cmath>

namespace QuantLib {

	namespace DayCounters {
	
		Time ActualActual::yearFraction(const Date& d1, const Date& d2, const Date& refPeriodStart, 
		  const Date& refPeriodEnd) const {
			Require(!IsNull(refPeriodStart) && !IsNull(refPeriodEnd) && refPeriodEnd > refPeriodStart && refPeriodEnd > d1,
				"Invalid reference period");
			double period = Thirty360European().yearFraction(refPeriodStart,refPeriodEnd);
			int months;
			double temp;
			Ensure(QL_FABS(QL_MODF(period*12.0, &temp)) <= 1.0e-10,"non-integer number of months");
			months = (int)temp;
		
			if (d2 <= refPeriodEnd) {
				if (d1 >= refPeriodStart)
					return (double)dayCount(d1,d2)/(double)dayCount(refPeriodStart,refPeriodEnd)*period;
				else {
					Date previousBDate = refPeriodStart.plusMonths(-months);
					double result = (double)(refPeriodStart-d1)/(double)dayCount(previousBDate,refPeriodStart);
					result += (double)(d2-refPeriodStart)/(double)dayCount(refPeriodStart,refPeriodEnd);
					return result*period;
				}
			} else {
				double result = (double)dayCount(d1,refPeriodStart)/(double)dayCount(refPeriodStart,refPeriodEnd)*period;
				int i = 0;
				Date tempd;
				do {
					tempd = refPeriodEnd.plusMonths(months*(i++));
				} while (tempd <= d2);
				i--;
				Date previoustemp = refPeriodEnd.plusMonths(months*i);
				return result + i*period + yearFraction(previoustemp,d2,previoustemp,tempd);
			}
		}
	
	}

}
