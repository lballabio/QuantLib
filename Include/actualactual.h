
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_actualactual_day_counter_h
#define quantlib_actualactual_day_counter_h

#include "qldefines.h"
#include "daycounter.h"

namespace QuantLib {

	namespace DayCounters {
	
		// WARNING: this probably contains bugs. Use at your own risk.
		
		class ActualActual : public DayCounter {
		  public:
			std::string name() const { return std::string("act/act"); }
			int dayCount(const Date& d1, const Date& d2) const { return (d2-d1); }
			Time yearFraction(const Date& d1, const Date& d2, 
			  const Date& refPeriodStart = Date(), const Date& refPeriodEnd = Date()) const;
		};
	
	}

}


#endif
