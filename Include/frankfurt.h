
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License: see the file LICENSE.TXT for details.
 * Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file.
 * LICENCE.TXT is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/


/*! \file frankfurt.h
	\brief Frankfurt calendar
*/

#ifndef quantlib_frankfurt_calendar_h
#define quantlib_frankfurt_calendar_h

#include "qldefines.h"
#include "westerncalendar.h"

namespace QuantLib {

	namespace Calendars {
	
		//! Frankfurt calendar
		/*! Holidays:
			<ul>
			<li>Saturdays</li>
			<li>Sundays</li>
			<li>New Year's Day, January 1st</li>
			<li>Good Friday</li>
			<li>Easter Monday</li>
			<li>Ascension Thursday</li>
			<li>Whit Monday</li>
			<li>Corpus Christi</li>
			<li>Labour Day, May 1st</li>
			<li>National Day, October 3rd</li>
			<li>Christmas Eve, December 24th</li>
			<li>Christmas, December 25th</li>
			<li>Boxing Day, December 26th</li>
			<li>New Year's Eve, December 31st</li>
			</ul>
		*/
		class Frankfurt : public WesternCalendar {
		  public:
			Frankfurt() {}
			std::string name() const { return "Frankfurt"; }
			bool isBusinessDay(const Date&) const;
		};
	
	}

}


#endif
