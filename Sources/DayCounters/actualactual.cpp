
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "actualactual.h"
#include "thirty360european.h"
#include "null.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(DayCounters)

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

QL_END_NAMESPACE(DayCounters)

QL_END_NAMESPACE(QuantLib)
