
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_thirty360italian_day_counter_h
#define quantlib_thirty360italian_day_counter_h

#include "qldefines.h"
#include "daycounter.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(DayCounters)

class Thirty360Italian : public DayCounter {
  public:
	std::string name() const { return std::string("30/360it"); }
	int dayCount(const Date& d1, const Date& d2) const;
	Time yearFraction(const Date& d1, const Date& d2, 
	  const Date& refPeriodStart = Date(), const Date& refPeriodEnd = Date()) const {
		return dayCount(d1,d2)/360.0;
	}
};

QL_END_NAMESPACE(DayCounters)

QL_END_NAMESPACE(QuantLib)


#endif
