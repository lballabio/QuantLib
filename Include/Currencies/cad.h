
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

/*! \file cad.h
	\brief Canadian Dollar
*/

#ifndef quantlib_CAD_h
#define quantlib_CAD_h

#include "qldefines.h"
#include "currency.h"
#include "calendar.h"

namespace QuantLib {

	namespace Currencies {
	
		//! Canadian Dollar
		/*! \todo The canadian calendar must be implemented. */
		class CAD : public Currency {
		  public:
			CAD() {}
			std::string name() const { return std::string("CAD"); }
			//! for the time being returns a handle to the null calendar
			Handle<Calendar> settlementCalendar() const { 
				return Handle<Calendar>(new Calendars::NullCalendar); }
			//! returns 2
			int settlementDays() const { return 2; }
		};
	
	}

}


#endif
