
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_thirty360european_day_counter_h
#define quantlib_thirty360european_day_counter_h

#include "qldefines.h"
#include "daycounter.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(DayCounters)

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

QL_END_NAMESPACE(DayCounters)

QL_END_NAMESPACE(QuantLib)


#endif
