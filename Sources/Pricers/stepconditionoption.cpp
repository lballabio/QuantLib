
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file stepconditionoption.cpp
    \brief Base class for options requiring additional code to be executed at each timestep

    $Source$
    $Log$
    Revision 1.9  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.8  2001/04/06 18:46:22  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.7  2001/04/05 13:06:55  marmar
    Code simplified

    Revision 1.6  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.5  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.4  2001/03/21 11:31:55  marmar
    Main loop tranfered from method value to method calculate.
    Methods vega and rho belong now to class BSMOption

    Revision 1.3  2001/03/21 10:48:08  marmar
    valueAtCenter, firstDerivativeAtCenter, secondDerivativeAtCenter,
    are no longer methods of BSMNumericalOption but separate
    functions

    Revision 1.2  2001/03/02 13:50:01  marmar
    Purely virtual method initializeStepCondition()
    introduced in the design of StepConditionOption

    Revision 1.1  2001/03/02 08:36:45  enri
    Shout options added:
    	* BSMAmericanOption is now AmericanOption, same interface
    	* ShoutOption added
    	* both ShoutOption and AmericanOption inherit from
    	  StepConditionOption
    offline.doxy.linux added.


*/

#include "ql/Pricers/stepconditionoption.hpp"
#include "ql/Pricers/bsmeuropeanoption.hpp"
#include "ql/FiniteDifferences/standardfdmodel.hpp"
#include "ql/FiniteDifferences/standardstepcondition.hpp"
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

            setGridLimits();
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
            BSMEuropeanOption analyticEuro(type_, underlying_,
                                           strike_, dividendYield_,
                                           riskFreeRate_, residualTime_,
                                           volatility_);

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
