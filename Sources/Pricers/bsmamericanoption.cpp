
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "bsmamericanoption.h"
#include "bsmeuropeanoption.h"
#include "finitedifferencemodel.h"
#include "cranknicolson.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Pricers)

QL_USING(PDE,FiniteDifferenceModel)
QL_USING(PDE,CrankNicolson)
QL_USING(PDE,StepCondition)
QL_USING(Operators,TridiagonalOperator)

double BSMAmericanOption::value() const {
	if (!hasBeenCalculated) {
		FiniteDifferenceModel<CrankNicolson<TridiagonalOperator> > model(theOperator);
		// Control-variate variance reduction:
		// 1) calculate the value of the European option numerically
		Array theEuroPrices = thePrices;
		model.rollback(theEuroPrices,theResidualTime,0.0,theTimeSteps);
		double numericEuropean = valueAtCenter(theEuroPrices);
		// 2) calculate the value of the European option analytically
		double analyticEuropean = BSMEuropeanOption(theType,theUnderlying,theStrike,theUnderlyingGrowthRate, 
		  theRiskFreeRate,theResidualTime,theVolatility).value();
		// 3) calculate the value of the American option numerically on the same grid
		model.rollback(thePrices,theResidualTime,0.0,theTimeSteps,
		  Handle<StepCondition<Array> >(new BSMAmericanCondition(thePrices)));
		double numericAmerican = valueAtCenter(thePrices);
		// 4) combine the results
		theValue = numericAmerican-numericEuropean+analyticEuropean;
		hasBeenCalculated = true;
	}
	return theValue;
}


QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)
