
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

/*! \file eur.h
	\brief Euro
*/

#ifndef quantlib_EUR_h
#define quantlib_EUR_h

#include "qldefines.h"
#include "currency.h"
#include "target.h"

namespace QuantLib {

	namespace Currencies {

		//! Euro
		class EUR : public Currency {
		  public:
			EUR() {}
			std::string name() const { return std::string("EUR"); }
			//! returns a handle to the TARGET calendar
			Handle<Calendar> settlementCalendar() const {
				return Handle<Calendar>(new Calendars::TARGET); }
			//! returns 2
			int settlementDays() const { return 2; }
		};
	
	}

}


#endif
