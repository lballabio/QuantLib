
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "bsmamericanoption.h"
#include "americancondition.h"
#include "bsmeuropeanoption.h"
#include "finitedifferencemodel.h"
#include "cranknicolson.h"

namespace QuantLib {

	namespace Pricers {
	
		using PDE::FiniteDifferenceModel;
		using PDE::CrankNicolson;
		using PDE::StepCondition;
		using Operators::TridiagonalOperator;
		
		double BSMAmericanOption::value() const {
			if (!hasBeenCalculated) {
				//Small time step which will be used to evaluate theta using finite difference
				double smallDt = theResidualTime/(100*theTimeSteps); 
				FiniteDifferenceModel<CrankNicolson<TridiagonalOperator> > model(theOperator);
				// Control-variate variance reduction:
				// 1) calculate the greeks of the European option analytically
		
				BSMEuropeanOption analyticEuro(theType,theUnderlying,theStrike,theUnderlyingGrowthRate, 
				  theRiskFreeRate,theResidualTime,theVolatility);
				double analyticEuroValue = analyticEuro.value();
				double analyticEuroDelta = analyticEuro.delta();
				double analyticEuroGamma = analyticEuro.gamma();
				double analyticEuroTheta = analyticEuro.theta();
		
				// 2) calculate the greeks of the European option numerically
				Array theEuroPrices = thePrices;
				model.rollback(theEuroPrices,theResidualTime,0.0,theTimeSteps);
				double numericEuroValue = valueAtCenter(theEuroPrices);
				double numericEuroDelta = firstDerivativeAtCenter(theEuroPrices,theGrid);
				double numericEuroGamma = secondDerivativeAtCenter(theEuroPrices,theGrid);
				model.rollback(theEuroPrices,0.0,-smallDt ,1);
				double numericEuroTheta = (numericEuroValue-valueAtCenter(theEuroPrices))/smallDt;
		
				// 3) calculate the greeks of the American option numerically on the same grid
				Handle<StepCondition<Array> > americanCondition(new BSMAmericanCondition(thePrices));
				model.rollback(thePrices,theResidualTime,0.0,theTimeSteps,americanCondition);
				double numericAmericanValue = valueAtCenter(thePrices);
				double numericAmericanDelta = firstDerivativeAtCenter(thePrices,theGrid);
				double numericAmericanGammma = secondDerivativeAtCenter(thePrices,theGrid);
				model.rollback(thePrices,0.0,-smallDt , 1,americanCondition);
				double numericAmericanTheta = (numericAmericanValue-valueAtCenter(theEuroPrices))/smallDt;
		
				// 4) combine the results
				theValue = numericAmericanValue - numericEuroValue + analyticEuroValue;
				theDelta = numericAmericanDelta - numericEuroDelta + analyticEuroDelta;
				theGamma = numericAmericanGammma- numericEuroGamma + analyticEuroGamma;
				theTheta = numericAmericanTheta - numericEuroTheta + analyticEuroTheta;
				hasBeenCalculated = true;
			}
			return theValue;
		}

	}

}
