
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_deposit_h
#define quantlib_deposit_h

#include "qldefines.h"
#include "date.h"
#include "daycounter.h"
#include "calendar.h"
#include "rate.h"

namespace QuantLib {

	class Deposit {
	  public:
		Deposit() {}
		Deposit(const Date& maturity, Rate rate, const Handle<DayCounter>& dayCounter)
		: theMaturity(maturity), theRate(rate), theDayCounter(dayCounter) {}
		Date maturity() const { return theMaturity; }
		Rate rate() const { return theRate; }
		Handle<DayCounter> dayCounter() const { return theDayCounter; }
	  private:
		Date theMaturity;
		Rate theRate;
		Handle<DayCounter> theDayCounter;
	};

}


#endif
