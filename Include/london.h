
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


/*! \file london.h
	\brief London calendar
*/

#ifndef quantlib_london_calendar_h
#define quantlib_london_calendar_h

#include "qldefines.h"
#include "westerncalendar.h"

namespace QuantLib {

	namespace Calendars {
	
		//! London calendar
		/*! Holidays:
			<ul>
			<li>Saturdays</li>
			<li>Sundays</li>
			<li>New Year's Day, January 1st (possibly moved to Monday)</li>
			<li>Good Friday</li>
			<li>Easter Monday</li>
			<li>Bank Holiday, first Monday of May</li>
			<li>Bank Holiday, last Monday of May</li>
			<li>Bank Holiday, last Monday of August</li>
			<li>Christmas, December 25th (possibly moved to Monday or Tuesday)</li>
			<li>Boxing Day, December 26th (possibly moved to Monday or Tuesday)</li>
			</ul>
		*/
		class London : public WesternCalendar {
		  public:
			London() {}
			std::string name() const { return "London"; }
			bool isBusinessDay(const Date&) const;
		};
	
	}

}


#endif
