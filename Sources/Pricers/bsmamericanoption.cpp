
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file bsmamericanoption.cpp
	\brief american option
	
	$Source$
	$Name$
	$Log$
	Revision 1.17  2001/01/08 10:28:17  lballabio
	Moved Array to Math namespace

	Revision 1.16  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#include "bsmamericanoption.h"
#include "bsmeuropeanoption.h"
#include "finitedifferencemodel.h"
#include "cranknicolson.h"

namespace QuantLib {

	namespace Pricers {
	
		using Math::Array;
		using FiniteDifferences::FiniteDifferenceModel;
		using FiniteDifferences::CrankNicolson;
		using FiniteDifferences::StepCondition;
		using FiniteDifferences::TridiagonalOperator;
		
		double BSMAmericanOption::value() const {
			if (!hasBeenCalculated) {
				setGridLimits();
				initializeGrid();
				initializeInitialCondition();
				initializeOperator();
				// model used for calculation
				FiniteDifferenceModel<CrankNicolson<TridiagonalOperator> > model(theOperator);
				double dt = theResidualTime/theTimeSteps;
				// Control-variate variance reduction:
				// 1) calculate value and greeks of the European option analytically
				BSMEuropeanOption analyticEuro(theType,theUnderlying,theStrike,theUnderlyingGrowthRate, 
				  theRiskFreeRate,theResidualTime,theVolatility);
				double analyticEuroValue = analyticEuro.value();
				double analyticEuroDelta = analyticEuro.delta();
				double analyticEuroGamma = analyticEuro.gamma();
				double analyticEuroTheta = analyticEuro.theta();
				// 2) calculate value and greeks of the European option numerically
				Array theEuroPrices = theInitialPrices;
				// rollback until dt
				model.rollback(theEuroPrices,theResidualTime,dt,theTimeSteps-1);
				double numericEuroValuePlus = valueAtCenter(theEuroPrices);
				// complete rollback
				model.rollback(theEuroPrices,dt,0.0,1);
				double numericEuroValue = valueAtCenter(theEuroPrices);
				double numericEuroDelta = firstDerivativeAtCenter(theEuroPrices,theGrid);
				double numericEuroGamma = secondDerivativeAtCenter(theEuroPrices,theGrid);
				// rollback another step
				model.rollback(theEuroPrices,0.0,-dt,1);
				double numericEuroValueMinus = valueAtCenter(theEuroPrices);
				double numericEuroTheta = (numericEuroValuePlus-numericEuroValueMinus)/(2.0*dt);
				// 3) calculate the greeks of the American option numerically on the same grid
				Array thePrices = theInitialPrices;
				Handle<StepCondition<Array> > americanCondition(new BSMAmericanCondition(theInitialPrices));
				// rollback until dt
				model.rollback(thePrices,theResidualTime,dt,theTimeSteps-1,americanCondition);
				double numericAmericanValuePlus = valueAtCenter(thePrices);
				// complete rollback
				model.rollback(thePrices,dt,0.0,1,americanCondition);
				double numericAmericanValue = valueAtCenter(thePrices);
				double numericAmericanDelta = firstDerivativeAtCenter(thePrices,theGrid);
				double numericAmericanGamma = secondDerivativeAtCenter(thePrices,theGrid);
				// rollback another step
				model.rollback(thePrices,0.0,-dt,1,americanCondition);
				double numericAmericanValueMinus = valueAtCenter(thePrices);
				double numericAmericanTheta = (numericAmericanValuePlus-numericAmericanValueMinus)/(2.0*dt);
				// 4) combine the results
				theValue = numericAmericanValue - numericEuroValue + analyticEuroValue;
				theDelta = numericAmericanDelta - numericEuroDelta + analyticEuroDelta;
				theGamma = numericAmericanGamma - numericEuroGamma + analyticEuroGamma;
				theTheta = numericAmericanTheta - numericEuroTheta + analyticEuroTheta;
				hasBeenCalculated = true;
			}
			return theValue;
		}

	}

}
