
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

// $Source$

// $Log$
// Revision 1.3  2000/12/13 18:09:53  nando
// CVS keyword added
//

/*! \file dkk.h
	\brief Danish Krona
*/

#ifndef quantlib_DKK_h
#define quantlib_DKK_h

#include "qldefines.h"
#include "currency.h"
#include "calendar.h"

namespace QuantLib {

	namespace Currencies {
	
		//! Danish Krona
		/*! \todo The danish calendar must be implemented.
			\todo The number of settlement days must be verified.
		*/
		class DKK : public Currency {
		  public:
			DKK() {}
			std::string name() const { return std::string("DKK"); }
			//! for the time being returns a handle to the null calendar
			Handle<Calendar> settlementCalendar() const { 
				return Handle<Calendar>(new Calendars::NullCalendar); }
			//! returns 2
			int settlementDays() const { return 2; }
		};
	
	}

}


#endif
