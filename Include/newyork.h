
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


/*! \file newyork.h
	\brief New York calendar
*/

#ifndef quantlib_newyork_calendar_h
#define quantlib_newyork_calendar_h

#include "qldefines.h"
#include "westerncalendar.h"

namespace QuantLib {

	namespace Calendars {
		
		//! New York calendar
		/*! Holidays:
			<ul>
			<li>Saturdays</li>
			<li>Sundays</li>
			<li>New Year's Day, January 1st (possibly moved to Monday if actually on Sunday,
				or to Friday if on Saturday)</li>
			<li>Martin Luther King's birthday, third Monday in January</li>
			<li>Washington's birthday, third Monday in February</li>
			<li>Memorial Day, last Monday in May</li>
			<li>Independence Day, July 4th (moved to Monday if Sunday or Friday if Saturday)</li>
			<li>Labor Day, first Monday in September</li>
			<li>Columbus Day, second Monday in October</li>
			<li>Veteran's Day, November 11th (moved to Monday if Sunday or Friday if Saturday)</li>
			<li>Thanksgiving Day, fourth Thursday in November</li>
			<li>Christmas, December 25th (moved to Monday if Sunday or Friday if Saturday)</li>
			</ul>
		*/
		class NewYork : public WesternCalendar {
		  public:
			NewYork() {}
			std::string name() const { return "NewYork"; }
			bool isBusinessDay(const Date&) const;
		};
	
	}

}


#endif
