
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_pricers_i
#define quantlib_pricers_i

%module Pricers

%include Date.i
%include Options.i
%include Financial.i

%{
#include "bsmeuropeanoption.h"
QL_USING(QuantLib::Pricers,BSMEuropeanOption)
%}

class BSMEuropeanOption {
  public:
	BSMEuropeanOption(OptionType type, double underlying, double strike, Yield underlyingGrowthRate, 
	  Yield riskFreeRate, Time residualTime, double volatility, int timeSteps, int gridPoints);
	double value() const;
};


#endif

