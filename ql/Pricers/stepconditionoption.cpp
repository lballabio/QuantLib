
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file stepconditionoption.cpp
    \brief Option requiring additional code to be executed at each time step

    \fullpath
    Pricers/%stepconditionoption.cpp
*/

// $Id$

#include "ql/Pricers/stepconditionoption.hpp"
#include "ql/Pricers/europeanoption.hpp"
#include "ql/FiniteDifferences/valueatcenter.hpp"

namespace QuantLib {

    namespace Pricers {

        using FiniteDifferences::StandardStepCondition;
        using FiniteDifferences::StandardFiniteDifferenceModel;
        using FiniteDifferences::valueAtCenter;
        using FiniteDifferences::firstDerivativeAtCenter;
        using FiniteDifferences::secondDerivativeAtCenter;

        StepConditionOption::StepConditionOption(Option::Type type, 
            double underlying, double strike, Spread dividendYield, 
            Rate riskFreeRate, Time residualTime, double volatility, 
            int timeSteps, int gridPoints)
        : BSMNumericalOption(type, underlying, strike, dividendYield,
            riskFreeRate, residualTime, volatility, gridPoints),
            timeSteps_(timeSteps) {}

        void StepConditionOption::calculate() const {

            setGridLimits(underlying_, residualTime_);
            initializeGrid();
            initializeInitialCondition();
            initializeOperator();
            initializeStepCondition();
            /* StandardFiniteDifferenceModel is Crank-Nicolson.
               Alternatively, BackwardEuler or ForwardEuler
               could have been used instead*/
            StandardFiniteDifferenceModel model(finiteDifferenceOperator_);

            // Control-variate variance reduction:
            // 1) calculate value/greeks of the European option analytically
            EuropeanOption analyticEuro(type_, underlying_, strike_,
                dividendYield_, riskFreeRate_, residualTime_, volatility_);

            // 2) Initialize prices on the grid
            Array europeanPrices = initialPrices_;
            Array americanPrices = initialPrices_;

            // 3) Rollback until dt
            double dt = residualTime_/timeSteps_;
            model.rollback(europeanPrices,residualTime_,dt,timeSteps_-1);
            model.rollback(americanPrices, residualTime_, dt, timeSteps_ -1,
                           stepCondition_);

            // 4) Store option value at time = dt for theta computation
            double europeanPlusDt = valueAtCenter(europeanPrices);
            double americanPlusDt = valueAtCenter(americanPrices);

            // 5) Complete rollback
            model.rollback(europeanPrices, dt, 0.0, 1);
            model.rollback(americanPrices, dt, 0.0, 1, stepCondition_);

            /* 6) Numerically calculate option value and greeks using
                  the european option as control variate                */

            value_ =  valueAtCenter(americanPrices)
                    - valueAtCenter(europeanPrices)
                    + analyticEuro.value();

            delta_ =   firstDerivativeAtCenter(americanPrices, grid_)
                     - firstDerivativeAtCenter(europeanPrices, grid_)
                     + analyticEuro.delta();

            gamma_ =   secondDerivativeAtCenter(americanPrices, grid_)
                     - secondDerivativeAtCenter(europeanPrices, grid_)
                     + analyticEuro.gamma();

            // 7) Rollback another step to time = -dt for theta computation
            model.rollback(europeanPrices, 0.0, -dt, 1);
            model.rollback(americanPrices, 0.0, -dt, 1, stepCondition_);

            // 8) combine the results for theta
            theta_=(americanPlusDt - valueAtCenter(americanPrices))/(2.0*dt)
                  -(europeanPlusDt - valueAtCenter(europeanPrices))/(2.0*dt)
                  + analyticEuro.theta();

            hasBeenCalculated_ = true;
        }

    }

}
