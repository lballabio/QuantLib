
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "bsmnumericaloption.h"
#include "qlerrors.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Pricers)

QL_USING(PDE,BoundaryCondition)
QL_USING(Operators,BSMOperator)

BSMNumericalOption::BSMNumericalOption(BSMOption::Type type, double underlying, double strike, 
  Rate underlyingGrowthRate, Rate riskFreeRate, Time residualTime, double volatility, int gridPoints)
: BSMOption(type,underlying,strike,underlyingGrowthRate,riskFreeRate,residualTime,volatility), 
  theGridPoints(gridPoints) {
	// common setup
	setGridLimits();
	initializeGrid();
	initializeInitialCondition();
	initializeOperator();
	hasBeenCalculated = false;
}

void BSMNumericalOption::setGridLimits() {
	double prefactor = 1.0+0.05/theVolatility;			// correction for small volatilities
	double minMaxFactor = QL_EXP(4.0*prefactor*theVolatility*QL_SQRT(theResidualTime));

	sMin = theUnderlying/minMaxFactor;					// underlying grid min value
	sMax = theUnderlying*minMaxFactor;					// underlying grid max value

	double safetyZoneFactor = 1.1;						// to insure strike is included in the grid
	if(sMin > theStrike/safetyZoneFactor){
		sMin = theStrike/safetyZoneFactor;  
		sMax = theUnderlying/(sMin/theUnderlying);		// to enforce central placement of the underlying
	}
	if(sMax < theStrike*safetyZoneFactor){
		sMax = theStrike*safetyZoneFactor;
		sMin = theUnderlying/(sMax/theUnderlying);		// to enforce central placement of the underlying
	}
}

void BSMNumericalOption::initializeGrid() {
	theGrid = Array(theGridPoints);
	theGridLogSpacing = (QL_LOG(sMax)-QL_LOG(sMin))/(theGridPoints-1);
	double edx = QL_EXP(theGridLogSpacing);
	theGrid[0] = sMin;
	int j;
	for (j=1; j<theGridPoints; j++)
		theGrid[j] = theGrid[j-1]*edx;
}

void BSMNumericalOption::initializeInitialCondition() {
	thePrices = Array(theGridPoints);
	int j;
	switch (theType) {
	  case Call:
		for(j=0; j<theGridPoints; j++)
			thePrices[j] = QL_MAX(theGrid[j]-theStrike,0.0);
		break;
	  case Put:
		for(j=0; j<theGridPoints; j++)
			thePrices[j] = QL_MAX(theStrike-theGrid[j],0.0);
		break;
	  case Straddle:
		for(j=0; j<theGridPoints; j++)
			thePrices[j] = QL_FABS(theStrike-theGrid[j]);
		break;
	  default:
		throw IllegalArgumentError("BSMEuropeanOption: invalid option type");  
	}
}

void BSMNumericalOption::initializeOperator() {
	theOperator = BSMOperator(theGridPoints, theGridLogSpacing, theRiskFreeRate, theUnderlyingGrowthRate, theVolatility);
	theOperator.setLowerBC(BoundaryCondition(BoundaryCondition::Neumann,thePrices[1]-thePrices[0]));
	theOperator.setHigherBC(BoundaryCondition(BoundaryCondition::Neumann,thePrices[theGridPoints-1]-thePrices[theGridPoints-2]));
}

double BSMNumericalOption::valueAtCenter(const Array& a) const {
	int midPoint = theGridPoints/2;
	if (theGridPoints % 2 == 1)
		return a[midPoint];
	else 
		return (a[midPoint]+a[midPoint-1])/2.0;
}


QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)
