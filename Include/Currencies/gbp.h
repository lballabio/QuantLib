
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

/*! \file gbp.h
	\brief British Pound

	$Source$
	$Name$
	$Log$
	Revision 1.4  2000/12/14 12:32:30  lballabio
	Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_GBP_h
#define quantlib_GBP_h

#include "qldefines.h"
#include "currency.h"
#include "london.h"

namespace QuantLib {

	namespace Currencies {
	
		//! British Pound
		class GBP : public Currency {
		  public:
			GBP() {}
			std::string name() const { return std::string("GBP"); }
			//! returns a handle to the London calendar
			Handle<Calendar> settlementCalendar() const { 
				return Handle<Calendar>(new Calendars::London); }
			// returns 0
			int settlementDays() const { return 0; }
		};
	
	}

}


#endif
