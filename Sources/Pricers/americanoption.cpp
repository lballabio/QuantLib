/*
 * Copyright (C) 2000, 2001
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
 * QuantLib license is also available at
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file americanoption.cpp
    \brief american option

    $Source$
    $Log$
    Revision 1.2  2001/03/02 08:39:19  enri
    comments slightly changed

    Revision 1.1  2001/03/02 08:36:45  enri
    Shout options added:
    	* BSMAmericanOption is now AmericanOption, same interface
    	* ShoutOption added
    	* both ShoutOption and AmericanOption inherit from
    	  StepConditionOption
    offline.doxy.linux added.
*/

#include "bsmamericanoption.h"
#include "bsmeuropeanoption.h"
#include "standardfdmodel.h"
#include "standardstepcondition.h"

namespace QuantLib {
    namespace Pricers {
        using FiniteDifferences::StandardStepCondition;
        using FiniteDifferences::StandardFiniteDifferenceModel;
        double AmericanOption::value() const {
            if (!hasBeenCalculated_) {
                setGridLimits();
                initializeGrid();
                initializeInitialCondition();
                initializeOperator();
                /* model used for calculation: it could have been
                   BackwardEuler or ForwardEuler instead of CrankNicolson */
                StandardFiniteDifferenceModel model(finiteDifferenceOperator_);
                double dt = residualTime_/timeSteps_;
                // Control-variate variance reduction:
                // 1) calculate value/greeks of the European option analytically
                BSMEuropeanOption analyticEuro(type_, underlying_,
                                               strike_, dividendYield_,
                                               riskFreeRate_, residualTime_, volatility_);
                double analyticEuroValue = analyticEuro.value();
                double analyticEuroDelta = analyticEuro.delta();
                double analyticEuroGamma = analyticEuro.gamma();
                double analyticEuroTheta = analyticEuro.theta();

                // 2) calculate value/greeks of the European option numerically
                Array theEuroPrices = initialPrices_;
                // rollback until dt
                model.rollback(theEuroPrices,residualTime_,dt,timeSteps_-1);
                double numericEuroValuePlus = valueAtCenter(theEuroPrices);
                // complete rollback
                model.rollback(theEuroPrices,dt,0.0,1);
                double numericEuroValue = valueAtCenter(theEuroPrices);
                double numericEuroDelta = 
                    firstDerivativeAtCenter(theEuroPrices, grid_);
                double numericEuroGamma =
                    secondDerivativeAtCenter(theEuroPrices, grid_);
                // rollback another step
                model.rollback(theEuroPrices,0.0,-dt,1);
                double numericEuroValueMinus = valueAtCenter(theEuroPrices);
                double numericEuroTheta =
                      (numericEuroValuePlus - numericEuroValueMinus) / (2.0*dt);
                // 3) greeks of the American option numerically on the same grid
                Array thePrices = initialPrices_;
                Handle<StandardStepCondition >
                  americanCondition(new BSMAmericanCondition(initialPrices_));
                // rollback until dt
                model.rollback(thePrices, residualTime_, dt, timeSteps_ -1,
                               americanCondition);
                double numericAmericanValuePlus = valueAtCenter(thePrices);
                // complete rollback
                model.rollback(thePrices, dt, 0.0, 1,americanCondition);
                double numericAmericanValue = valueAtCenter(thePrices);
                double numericAmericanDelta = 
                    firstDerivativeAtCenter(thePrices, grid_);
                double numericAmericanGamma =
                    secondDerivativeAtCenter(thePrices, grid_);
                // rollback another step
                model.rollback(thePrices,0.0,-dt,1,americanCondition);
                double numericAmericanValueMinus = valueAtCenter(thePrices);
                double numericAmericanTheta =
                    (numericAmericanValuePlus -numericAmericanValueMinus) /
                    (2.0*dt);
                // 4) combine the results
                value_ = numericAmericanValue - numericEuroValue +
                    analyticEuroValue;
                delta_ = numericAmericanDelta - numericEuroDelta +
                    analyticEuroDelta;
                gamma_ = numericAmericanGamma - numericEuroGamma +
                    analyticEuroGamma;
                theta_ = numericAmericanTheta - numericEuroTheta +
                    analyticEuroTheta;
                hasBeenCalculated_ = true;
            }
            return value_;
        }
    }
}
