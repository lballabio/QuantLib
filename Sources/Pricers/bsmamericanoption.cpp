
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "bsmamericanoption.h"
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
		// uncomment for variance reduction:
		// Array theEuroPrices = thePrices;
		model.rollback(thePrices,theResidualTime,0.0,theTimeSteps,
		  Handle<StepCondition<Array> >(new BMSAmericanCondition(thePrices)));
		// uncomment for variance reduction:
		// model.rollback(theEuroPrices,theResidualTime,0.0,theTimeSteps);
		theValue = valueAtCenter(thePrices);
		// uncomment and substitute to the above for variance reduction:
		// theValue = valueAtCenter(thePrices)-valueAtCenter(theEuroPrices)+analyticEuropeanValue;
	}
	return theValue;
}


QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)
