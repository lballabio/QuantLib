
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

/*! \file milan.h
	\brief Milan calendar
*/

#ifndef quantlib_milan_calendar_h
#define quantlib_milan_calendar_h

#include "qldefines.h"
#include "westerncalendar.h"

namespace QuantLib {

	namespace Calendars {
	
		//! Milan calendar
		/*! Holidays:
			<ul>
			<li>Saturdays</li>
			<li>Sundays</li>
			<li>New Year's Day, January 1st</li>
			<li>Epiphany, January 6th</li>
			<li>Easter Monday</li>
			<li>Liberation Day, April 25th</li>
			<li>Labour Day, May 1st</li>
			<li>Assumption, August 15th</li>
			<li>All Saint's Day, November 1st</li>
			<li>Immaculate Conception, December 8th</li>
			<li>Christmas, December 25th</li>
			<li>St. Stephen, December 26th</li>
			</ul>
		*/
		class Milan : public WesternCalendar {
		  public:
			Milan() {}
			std::string name() const { return "Milan"; }
			bool isBusinessDay(const Date&) const;
		};
	
	}

}


#endif
