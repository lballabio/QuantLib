
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
#include "yield.h"

QL_BEGIN_NAMESPACE(QuantLib)

class Deposit {
  public:
	Deposit() {}
	Deposit(const Date& maturity, Yield rate, Handle<DayCounter> dayCounter)
	: theMaturity(maturity), theRate(rate), theDayCounter(dayCounter) {}
	Date maturity() const { return theMaturity; }
	Yield rate() const { return theRate; }
	Handle<DayCounter> dayCounter() const { return theDayCounter; }
  private:
	Date theMaturity;
	Yield theRate;
	Handle<DayCounter> theDayCounter;
};

QL_END_NAMESPACE(QuantLib)


#endif
