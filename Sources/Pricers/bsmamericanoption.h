
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef BSM_american_option_pricer_h
#define BSM_american_option_pricer_h

#include "qldefines.h"
#include "bsmnumericaloption.h"
#include "stepcondition.h"
#include "americancondition.h"
#include <algorithm>
#include <functional>

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Pricers)

class BSMAmericanOption : public BSMNumericalOption {
  public:
	// constructor
	BSMAmericanOption(Type type, double underlying, double strike, Yield underlyingGrowthRate, 
	  Yield riskFreeRate, Time residualTime, double volatility, int timeSteps, int gridPoints)
	: BSMNumericalOption(type,underlying,strike,underlyingGrowthRate,riskFreeRate,residualTime,volatility,
	  gridPoints), theTimeSteps(timeSteps) {}
	// accessors
	double value() const;
  private:
	int theTimeSteps;
};

QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)


#endif
