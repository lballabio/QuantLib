
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_milan_calendar_h
#define quantlib_milan_calendar_h

#include "qldefines.h"
#include "westerncalendar.h"

namespace QuantLib {

	namespace Calendars {
	
		class Milan : public WesternCalendar {
		  public:
			Milan() {}
			std::string name() const { return "Milan"; }
			bool isBusinessDay(const Date&) const;
		};
	
	}

}


#endif
