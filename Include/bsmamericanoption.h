
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


#ifndef BSM_american_option_pricer_h
#define BSM_american_option_pricer_h

#include "qldefines.h"
#include "bsmnumericaloption.h"
#include "stepcondition.h"
#include "americancondition.h"
#include <algorithm>
#include <functional>

namespace QuantLib {

	namespace Pricers {
	
		class BSMAmericanOption : public BSMNumericalOption {
		  public:
			// constructor
			BSMAmericanOption(Type type, double underlying, double strike, Rate underlyingGrowthRate, 
			  Rate riskFreeRate, Time residualTime, double volatility, int timeSteps, int gridPoints)
			: BSMNumericalOption(type,underlying,strike,underlyingGrowthRate,riskFreeRate,residualTime,volatility,
			  gridPoints), theTimeSteps(timeSteps) {}
			// accessors
			double value() const;
			Handle<BSMOption> clone() const{	// This method must be implemented to calculate implied volatility
				return Handle<BSMOption>(new BSMAmericanOption(*this));
			}
		  private:
			int theTimeSteps;
		};

	}

}


#endif
