
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_PCF_Deposit_curve_h
#define quantlib_PCF_Deposit_curve_h

#include "qldefines.h"
#include "termstructure.h"
#include "deposit.h"
#include <vector>

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(TermStructures)

class PiecewiseConstantForwards : public TermStructure {
  public:
	// constructor
	PiecewiseConstantForwards(Handle<Currency> currency, Handle<DayCounter> dayCounter, const Date& today, 
	  const std::vector<Deposit>& deposits);
	// clone			
	Handle<TermStructure> clone() const;
	// inspectors
	Handle<Currency> currency() const;
	Handle<DayCounter> dayCounter() const;
	Date todaysDate() const;
	Date settlementDate() const;
	Handle<Calendar> calendar() const;
	Date maxDate() const;
	Date minDate() const;
	// zero yield
	Yield zeroYield(const Date&) const;
	// discount
	DiscountFactor discount(const Date&) const;
	// forward (instantaneous)
	Yield forward(const Date&) const;
  private:
	// methods
	Time timeFromSettlement(const Date& d) const;
	int nextNode(const Date& d) const;
	// data members
	Handle<Currency> theCurrency;
	Handle<DayCounter> theDayCounter;
	Date today;
	std::vector<Date> theNodes;
	std::vector<Time> theTimes;
	std::vector<DiscountFactor> theDiscounts;
	std::vector<Yield> theForwards, theZeroYields;
	std::vector<Deposit> theDeposits;
};

// inline definitions

inline Handle<TermStructure> PiecewiseConstantForwards::clone() const {
	return Handle<TermStructure>(new PiecewiseConstantForwards(theCurrency,theDayCounter,today,theDeposits));
}

inline Handle<Currency> PiecewiseConstantForwards::currency() const {
	return theCurrency;
}

inline Handle<DayCounter> PiecewiseConstantForwards::dayCounter() const {
	return theDayCounter;
}

inline Date PiecewiseConstantForwards::todaysDate() const {
	return today;
}

inline Date PiecewiseConstantForwards::settlementDate() const {
	return theCurrency->settlementDate(today);
}

inline Handle<Calendar> PiecewiseConstantForwards::calendar() const {
	return theCurrency->settlementCalendar();
}

inline Date PiecewiseConstantForwards::maxDate() const {
	return theNodes.back();
}

inline Date PiecewiseConstantForwards::minDate() const {
	return settlementDate();
}

inline Time PiecewiseConstantForwards::timeFromSettlement(const Date& d) const {
	return theDayCounter->yearFraction(settlementDate(),d);
}

QL_END_NAMESPACE(TermStructures)

QL_END_NAMESPACE(QuantLib)


#endif
