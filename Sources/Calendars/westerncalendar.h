
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_western_calendar_h
#define quantlib_western_calendar_h

#include "qldefines.h"
#include "calendar.h"

namespace QuantLib {

	namespace Calendars {
	
		class WesternCalendar : public Calendar {
		  public:
			WesternCalendar() {}
		  protected:
			static const Day easterMonday[];			// relative to first day of year
		};
	
	}

}


#endif
