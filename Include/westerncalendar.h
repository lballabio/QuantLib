
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

/*! \file westerncalendar.h
	\brief Western calendar
*/

#ifndef quantlib_western_calendar_h
#define quantlib_western_calendar_h

#include "qldefines.h"
#include "calendar.h"

namespace QuantLib {

	namespace Calendars {
	
		//! Frankfurt calendar
		/*! This calendar provides derived calendars with the means of determining 
			the Easter Monday for a given year.
		*/
		class WesternCalendar : public Calendar {
		  public:
			WesternCalendar() {}
		  protected:
			//! expressed relative to first day of year
			static const Day easterMonday[];
		};
	
	}

}


#endif
