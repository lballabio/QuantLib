
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_actual365_day_counter_h
#define quantlib_actual365_day_counter_h

#include "qldefines.h"
#include "daycounter.h"

namespace QuantLib {

	namespace DayCounters {
	
		class Actual365 : public DayCounter {
		  public:
			std::string name() const { return std::string("act/365"); }
			int dayCount(const Date& d1, const Date& d2) const { return (d2-d1); }
			Time yearFraction(const Date& d1, const Date& d2, 
			  const Date& refPeriodStart = Date(), const Date& refPeriodEnd = Date()) const {
				return dayCount(d1,d2)/365.0;
			}
		};
	
	}

}


#endif
