
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

    \fullpath
    Sources/Pricers/%stepconditionoption.cpp
    \brief Option requiring additional code to be executed at each time step

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 14:04:01  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.21  2001/08/31 15:23:47  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.20  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.19  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.18  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.17  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.16  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.15  2001/06/05 09:35:14  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.14  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.13  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Pricers/stepconditionoption.hpp"
#include "ql/Pricers/europeanoption.hpp"
#include "ql/FiniteDifferences/standardfdmodel.hpp"
#include "ql/FiniteDifferences/valueatcenter.hpp"

namespace QuantLib {
    namespace Pricers {

        using FiniteDifferences::StandardStepCondition;
        using FiniteDifferences::StandardFiniteDifferenceModel;
        using FiniteDifferences::valueAtCenter;
        using FiniteDifferences::firstDerivativeAtCenter;
        using FiniteDifferences::secondDerivativeAtCenter;

        StepConditionOption::StepConditionOption(Type type, double underlying,
                 double strike, Rate dividendYield, Rate riskFreeRate,
                 Time residualTime, double volatility, int timeSteps,
                 int gridPoints)
            : BSMNumericalOption(type, underlying, strike, dividendYield,
                                 riskFreeRate, residualTime, volatility,
                                 gridPoints),
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
