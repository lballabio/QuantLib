
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

/*! \file gbp.h
	\brief British Pound
*/
#ifndef quantlib_GBP_h
#define quantlib_GBP_h

#include "qldefines.h"
#include "currency.h"
#include "london.h"

namespace QuantLib {

	namespace Currencies {
	
		//! British Pound
		class GBP : public Currency {
		  public:
			GBP() {}
			std::string name() const { return std::string("GBP"); }
			//! returns a handle to the London calendar
			Handle<Calendar> settlementCalendar() const { 
				return Handle<Calendar>(new Calendars::London); }
			// returns 0
			int settlementDays() const { return 0; }
		};
	
	}

}


#endif
