
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef BSM_european_option_pricer_h
#define BSM_european_option_pricer_h

#include "qldefines.h"
#include "bsmoption.h"
#include "discountfactor.h"

namespace QuantLib {

	namespace Pricers {
	
		class BSMEuropeanOption : public BSMOption {
		  public:
			  // constructor
			  BSMEuropeanOption(Type type, double underlying, double strike, Rate underlyingGrowthRate, 
			    Rate riskFreeRate, Time residualTime, double volatility)
			    : BSMOption(type,underlying,strike,underlyingGrowthRate,riskFreeRate,residualTime,volatility) {}
			  // accessors
			  double value() const;
			  double delta() const;
			  double gamma() const;
			  double theta() const;
			  double vega() const;
			  double rho() const;
		    Handle<BSMOption> clone() const {	return Handle<BSMOption>(new BSMEuropeanOption(*this)); }
		  private:
		    // declared as mutable to preserve
		    // the logical constness (does this word exist?) of value()
		    mutable DiscountFactor growthDiscount, riskFreeDiscount;
		    mutable double standardDeviation;
		    mutable double alpha, beta, NID1;
		};

	}

}


#endif
