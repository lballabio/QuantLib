
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_london_calendar_h
#define quantlib_london_calendar_h

#include "qldefines.h"
#include "westerncalendar.h"

namespace QuantLib {

	namespace Calendars {
	
		class London : public WesternCalendar {
		  public:
			London() {}
			std::string name() const { return "London"; }
			bool isBusinessDay(const Date&) const;
		};
	
	}

}


#endif
