
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "bsmeuropeanoption.h"
#include "finitedifferencemodel.h"
#include "cranknicolson.h"
#include "qlerrors.h"
#include <algorithm>

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Pricers)

QL_USING(PDE,FiniteDifferenceModel)
QL_USING(PDE,CrankNicolson)
QL_USING(PDE,BoundaryCondition)
QL_USING(Operators,BSMOperator)
QL_USING(Operators,TridiagonalOperator)

BSMEuropeanOption::BSMEuropeanOption(Option::Type type, double underlying, double strike, Yield underlyingGrowthRate, 
  Yield riskFreeRate, Time residualTime, double volatility, int timeSteps, int gridPoints)
: theType(type), theUnderlying(underlying), theStrike(strike), theUnderlyingGrowthRate(underlyingGrowthRate), 
  theRiskFreeRate(riskFreeRate), theResidualTime(residualTime), theVolatility(volatility), theTimeSteps(timeSteps), 
  theGridPoints(gridPoints), hasBeenCalculated(false) {}

double BSMEuropeanOption::value() const {
	if (!hasBeenCalculated) {
		// calculate grid limits
		double prefactor = 1.0+0.05/theVolatility;			// correction for small volatilities
		double minMaxFactor = QL_EXP(4.0*prefactor*theVolatility*QL_SQRT(theResidualTime));

		double sMin = theUnderlying/minMaxFactor;					// underlying grid min value
		double sMax = theUnderlying*minMaxFactor;					// underlying grid max value

		double safetyZoneFactor = 1.1;						// to insure strike is included in the grid
		if(sMin > theStrike/safetyZoneFactor){
			sMin = theStrike/safetyZoneFactor;  
			sMax = theUnderlying/(sMin/theUnderlying);		// to enforce central placement of the underlying
		}
		if(sMax < theStrike*safetyZoneFactor){
			sMax = theStrike*safetyZoneFactor;
			sMin = theUnderlying/(sMax/theUnderlying);		// to enforce central placement of the underlying
		}
		
		// set initial condition
		Array thePrices(theGridPoints);
		double dx = (QL_LOG(sMax)-QL_LOG(sMin))/(theGridPoints-1);
		double edx = QL_EXP(dx);
		thePrices[0] = sMin;
		int j;
		for (j=1; j<theGridPoints; j++)
			thePrices[j] = thePrices[j-1]*edx;

		switch (theType) {
		  case Option::Call:
			for(j=0; j<theGridPoints; j++)
				thePrices[j] = QL_MAX(thePrices[j]-theStrike,0.0);
			break;
		  case Option::Put:
			for(j=0; j<theGridPoints; j++)
				thePrices[j] = QL_MAX(theStrike-thePrices[j],0.0);
			break;
		  case Option::Straddle:
			for(j=0; j<theGridPoints; j++)
				thePrices[j] = QL_FABS(theStrike-thePrices[j]);
			break;
		  default:
			throw IllegalArgumentError("BSMEuropeanOption: invalid option type");  
		}

		// build evolution operator
		BSMOperator D(theGridPoints, dx, theRiskFreeRate, theUnderlyingGrowthRate, theVolatility);
		D.setLowerBC(BoundaryCondition(BoundaryCondition::Neumann,thePrices[1]-thePrices[0]));
		D.setHigherBC(BoundaryCondition(BoundaryCondition::Neumann,thePrices[theGridPoints-1]-thePrices[theGridPoints-2]));
		// rollback
		FiniteDifferenceModel<CrankNicolson<TridiagonalOperator> > model(D);
		model.rollback(thePrices,theResidualTime,0.0,theTimeSteps);
		int midPoint = theGridPoints/2;
		if (theGridPoints % 2 == 1)
			theValue = thePrices[midPoint];
		else 
			theValue = 0.5*(thePrices[midPoint]+thePrices[midPoint-1]);
	}
	return theValue;
}


QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)
