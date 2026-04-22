/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file montenegro.hpp
	\brief Montenegro calendars
*/

#ifndef quantlib_montenegro_calendar_hpp
#define quantlib_montenegro_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

	//! Montenegro calendar
	/*! Holidays for the Montenegrin Stock Exchange (https://mnse.me):
		<ul>
		<li>Saturdays</li>
		<li>Sundays</li>
		<li>New Year, Christmas Eve and Christmas: Jan 1-8</li>
		<li>Good Friday</li>
		<li>Orthodox Easter Monday</li>
		<li>Labour Day: May 1-2</li>
		<li>Independence Day: May 21-22</li>
		<li>Statehood Day: July 14-15</li>
		<li>Day of Njego, Montenegrin cultural holiday: November 13th and 14th</li>
		</ul>
	*/
	class Montenegro : public Calendar {
	private:
		class MnseImpl final : public Calendar::WesternImpl {
		public:
			std::string name() const override { return "Montenegro Stock Exchange"; }
			bool isBusinessDay(const Date& date) const override;
		};

	public:
		enum Market {
			MNSE //!< Montenegro Stock Exchange
		};
		Montenegro(Market m = MNSE);
	};

}

#endif // quantlib_montenegro_calendar_hpp