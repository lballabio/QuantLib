
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

/*! \file finitedifferenceeuropean.cpp
    \brief Example of european option calculated using finite differences

    $Source$
    $Log$
    Revision 1.10  2001/05/22 13:24:29  marmar
    setGridLimits interface changed

    Revision 1.9  2001/04/26 16:05:42  marmar
    underlying_ not mutable anymore, setGridLimits accepts the value for center

    Revision 1.8  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.7  2001/04/06 18:46:22  nando
    changed Authors, Contributors, Licence and copyright header

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

    Revision 1.2  2001/03/12 13:12:00  marmar
    Public method getPrices added

    Revision 1.1  2001/03/07 17:15:44  marmar
    Example of european option using finite differences

*/

#include "ql/Pricers/finitedifferenceeuropean.hpp"
#include "ql/Pricers/bsmnumericaloption.hpp"
#include "ql/FiniteDifferences/standardfdmodel.hpp"
#include "ql/FiniteDifferences/valueatcenter.hpp"

namespace QuantLib {

    namespace Pricers {

        using FiniteDifferences::valueAtCenter;
        using FiniteDifferences::firstDerivativeAtCenter;
        using FiniteDifferences::secondDerivativeAtCenter;

        FiniteDifferenceEuropean::FiniteDifferenceEuropean(
            Type type, double underlying, double strike,
            Rate dividendYield, Rate riskFreeRate, Time residualTime,
            double volatility, int timeSteps, int gridPoints)
            : BSMNumericalOption(type, underlying, strike, dividendYield,
                                 riskFreeRate, residualTime, volatility,
                                 gridPoints),
            timeSteps_(timeSteps), euroPrices_(gridPoints_){}


        void FiniteDifferenceEuropean::calculate() const {
            setGridLimits(underlying_, residualTime_);
            initializeGrid();
            initializeInitialCondition();
            initializeOperator();

            FiniteDifferences::StandardFiniteDifferenceModel
                                model(finiteDifferenceOperator_);

            euroPrices_ = initialPrices_;

            model.rollback(euroPrices_, residualTime_, 0, timeSteps_);

            value_ = valueAtCenter(euroPrices_);
            delta_ = firstDerivativeAtCenter(euroPrices_, grid_);
            gamma_ = secondDerivativeAtCenter(euroPrices_, grid_);

            double dt = residualTime_/timeSteps_;
            model.rollback(euroPrices_, 0.0, -dt, 1);
            double valueMinus = valueAtCenter(euroPrices_);
            theta_ = (value_ - valueMinus) / dt;

            hasBeenCalculated_ = true;
        }

    }

}
