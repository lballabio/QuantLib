
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef BSM_option_pricer_h
#define BSM_option_pricer_h

#include "qldefines.h"
#include "options.h"
#include "date.h"
#include "yield.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Pricers)

class BSMOption : public Option {
  public:
	BSMOption(Type type, double underlying, double strike, Yield underlyingGrowthRate, 
	  Yield riskFreeRate, Time residualTime, double volatility) 
	: theType(type), theUnderlying(underlying), theStrike(strike), theUnderlyingGrowthRate(underlyingGrowthRate), 
	  theRiskFreeRate(riskFreeRate), theResidualTime(residualTime), theVolatility(volatility),
	  hasBeenCalculated(false) {}
	virtual ~BSMOption() {}	// just in case
	// accessors
	virtual double value() const = 0;
  protected:
	// input data
	Type theType;
	double theUnderlying, theStrike;
	Yield theUnderlyingGrowthRate, theRiskFreeRate;
	Time theResidualTime;
	double theVolatility;
	// results
	mutable bool hasBeenCalculated;			// declared as mutable to preserve the logical
	mutable double theValue;				// constness (does this word exist?) of value()
};


QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)


#endif
