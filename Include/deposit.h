
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


#ifndef quantlib_deposit_h
#define quantlib_deposit_h

#include "qldefines.h"
#include "date.h"
#include "daycounter.h"
#include "calendar.h"
#include "rate.h"

namespace QuantLib {

	//! deposit rate
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
