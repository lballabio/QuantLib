
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

/*! \file itl.h
	\brief Italian Lira
*/

#ifndef quantlib_ITL_h
#define quantlib_ITL_h

#include "qldefines.h"
#include "currency.h"
#include "target.h"

namespace QuantLib {

	namespace Currencies {

		//! Italian Lira
		class ITL : public Currency {
		  public:
			ITL() {}
			std::string name() const { return std::string("ITL"); }
			//! returns a handle to the TARGET calendar
			Handle<Calendar> settlementCalendar() const { 
				return Handle<Calendar>(new Calendars::TARGET); }
			//! returns 2
			int settlementDays() const { return 2; }
		};
	
	}

}


#endif
