
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "bsmeuropeanoption.h"
#include "discountfactor.h"
#include "normaldistribution.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Pricers)

double BSMEuropeanOption::value() const {
	if (!hasBeenCalculated) {
		DiscountFactor growthDiscount = (QL_EXP(-theUnderlyingGrowthRate*theResidualTime));
		DiscountFactor riskFreeDiscount = (QL_EXP(-theRiskFreeRate*theResidualTime));
		Time volResidualTime = theVolatility*QL_SQRT(theResidualTime);
		CumulativeNormalDistribution f;
		double D1 = QL_LOG(theUnderlying/theStrike)/volResidualTime + volResidualTime/2.0
		  + (theRiskFreeRate-theUnderlyingGrowthRate)*theResidualTime/volResidualTime;
		double D2 = D1 - volResidualTime;
		double fD1 = f(D1), fD2 = f(D2);
		double alpha, beta;
		switch (theType) {
		  case Call:
			alpha = fD1;
			beta = fD2;
			break;
		  case Put:
			alpha = fD1-1.0;
			beta = fD2-1.0;
			break;
		  case Straddle:
			alpha = 2.0*fD1-1.0;
			beta = 2.0*fD2-1.0;
			break;
		  default:
			throw IllegalArgumentError("AnalyticBSM: invalid option type");
		}
		theValue = theUnderlying*growthDiscount*alpha - theStrike*riskFreeDiscount*beta;
	}
	return theValue;
}


QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)
