
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
	BSMOption(Type type, double underlying, double strike, Rate underlyingGrowthRate, 
	  Rate riskFreeRate, Time residualTime, double volatility) 
	: theType(type), theUnderlying(underlying), theStrike(strike), theUnderlyingGrowthRate(underlyingGrowthRate),
	  theRiskFreeRate(riskFreeRate), theResidualTime(residualTime), theVolatility(volatility),
    hasBeenCalculated(false) {

    Require(strike > 0.0, "BSMOption::BSMOption : strike must be positive");
    Require(underlying > 0.0, "BSMOption::BSMOption : underlying must be positive");
    Require(residualTime > 0.0, "BSMOption::BSMOption : residual time must be positive");
    Require(volatility > 0.0, "BSMOption::BSMOption : volatility must be positive");

  }
	virtual ~BSMOption() {}	// just in case
	// accessors
	virtual double value() const = 0;
	virtual double delta() const = 0;
	virtual double gamma() const = 0;
	virtual double theta() const = 0;
	virtual double vega() const = 0;
	virtual double rho() const = 0;
	//virtual double impliedVolatility(double targetValue, double xacc = 1e-4, int maxEvaluations = 100) const;
	//virtual Handle<BSMOption> clone() const = 0;
  protected:
	// input data
	Type theType;
	double theUnderlying, theStrike;
	Rate theUnderlyingGrowthRate, theRiskFreeRate;
	Time theResidualTime;
	double theVolatility;
	// results
	mutable bool hasBeenCalculated;	// declared as mutable to preserve the logical
	mutable double theValue;				// constness (does this word exist?) of value()
};


QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)


#endif
