
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file zurich.h
	\brief Zurich calendar

	$Source$
	$Name$
	$Log$
	Revision 1.4  2000/12/14 12:32:29  lballabio
	Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_zurich_calendar_h
#define quantlib_zurich_calendar_h

#include "qldefines.h"
#include "westerncalendar.h"

namespace QuantLib {

	namespace Calendars {
	
		//! %Zurich calendar
		/*! Holidays:
			<ul>
			<li>Saturdays</li>
			<li>Sundays</li>
			<li>New Year's Day, January 1st</li>
			<li>Berchtoldstag, January 2nd</li>
			<li>Good Friday</li>
			<li>Easter Monday</li>
			<li>Ascension Day</li>
			<li>Whit Monday</li>
			<li>Labour Day, May 1st</li>
			<li>National Day, August 1st</li>
			<li>Christmas, December 25th</li>
			<li>St. Stephen's Day, December 26th</li>
			</ul>
		*/
		class Zurich : public WesternCalendar {
		  public:
			Zurich() {}
			std::string name() const { return "Zurich"; }
			bool isBusinessDay(const Date&) const;
		};
	
	}

}


#endif
