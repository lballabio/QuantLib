
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

/*! \file deposit.h
	\brief Deposit rate
	
	$Source$
	$Name$
	$Log$
	Revision 1.6  2000/12/14 12:32:29  lballabio
	Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_deposit_h
#define quantlib_deposit_h

#include "qldefines.h"
#include "date.h"
#include "daycounter.h"
#include "calendar.h"
#include "rate.h"

namespace QuantLib {

	//! %deposit rate
	class Deposit {
	  public:
		Deposit() {}
		Deposit(const Date& maturity, Rate rate, const Handle<DayCounter>& dayCounter)
		: theMaturity(maturity), theRate(rate), theDayCounter(dayCounter) {}
		//! \name Inspectors
		//@{
		Date maturity() const { return theMaturity; }
		Rate rate() const { return theRate; }
		Handle<DayCounter> dayCounter() const { return theDayCounter; }
		//@}
	  private:
		Date theMaturity;
		Rate theRate;
		Handle<DayCounter> theDayCounter;
	};

}


#endif
