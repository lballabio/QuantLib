
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "thirty360.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(DayCounters)

int Thirty360::dayCount(const Date& d1, const Date& d2) const {
	int dd2 = d2.dayOfMonth(), mm2 = d2.month();
	if (dd2 == 31 && d1.dayOfMonth() < 30){
		dd2 = 1;
		mm2++;
	}
	return 360*(d2.year()-d1.year()) + 30*(mm2-d1.month()-1) + QL_MAX(0,30-d1.dayOfMonth()) + QL_MIN(30,dd2);
}

QL_END_NAMESPACE(DayCounters)

QL_END_NAMESPACE(QuantLib)
