
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef BSM_european_option_pricer_h
#define BSM_european_option_pricer_h

#include "qldefines.h"
#include "bsmnumericaloption.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Pricers)

class BSMEuropeanOption : public BSMNumericalOption {
  public:
	// constructor
	BSMEuropeanOption(Option::Type type, double underlying, double strike, Yield underlyingGrowthRate, 
	  Yield riskFreeRate, Time residualTime, double volatility, int timeSteps, int gridPoints)
	: BSMNumericalOption(type,underlying,strike,underlyingGrowthRate,riskFreeRate,residualTime,volatility,
	  gridPoints), theTimeSteps(timeSteps)  {}
	// accessors
	double value() const;
  private:
	int  theTimeSteps;
};


QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)


#endif
