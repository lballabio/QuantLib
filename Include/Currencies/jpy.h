
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

/*! \file jpy.h
	\brief Japanese Yen
*/

#ifndef quantlib_JPY_h
#define quantlib_JPY_h

#include "qldefines.h"
#include "currency.h"
#include "calendars.h"

namespace QuantLib {

	namespace Currencies {
	
		//! Japanese Yen
		/*! \todo The japanese calendar must be implemented. */
		class JPY : public Currency {
		  public:
			JPY() {}
			std::string name() const { return std::string("JPY"); }
			//! for the time being returns a handle to the null calendar
			Handle<Calendar> settlementCalendar() const { 
				return Handle<Calendar>(new Calendars::NullCalendar); }
			//! returns 0
			int settlementDays() const { return 0; }
		};
	
	}

}


#endif
