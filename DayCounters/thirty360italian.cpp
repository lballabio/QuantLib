
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "thirty360italian.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(DayCounters)

int Thirty360Italian::dayCount(const Date& d1, const Date& d2) const {
	int gg1 = d1.dayOfMonth(), gg2 = d2.dayOfMonth();
	if (d1.month() == 2 && gg1 > 27)
		gg1 = 30;
	if (d2.month() == 2 && gg2 > 27)
		gg2 = 30;
	return 360*(d2.year()-d1.year()) + 30*(d2.month()-d1.month()-1) + QL_MAX(0,30-gg1) + QL_MIN(30,gg2);
}

QL_END_NAMESPACE(DayCounters)

QL_END_NAMESPACE(QuantLib)
