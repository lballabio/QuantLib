
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_ITL_h
#define quantlib_ITL_h

#include "qldefines.h"
#include "currency.h"
#include "target.h"

namespace QuantLib {

	namespace Currencies {
	
		class ITL : public Currency {
		  public:
			ITL() {}
			std::string name() const { return std::string("ITL"); }
			Handle<Calendar> settlementCalendar() const { 
				return Handle<Calendar>(new Calendars::TARGET); }
			int settlementDays() const { return 2; }
		};
	
	}

}


#endif
