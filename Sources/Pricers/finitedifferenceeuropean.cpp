/*
 * Copyright (C) 2001
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

/*! \file finitedifferenceeuropean.cpp
    \brief Example of european option calculated using finite differences

    $Source$
    $Log$
    Revision 1.2  2001/03/12 13:12:00  marmar
    Public method getPrices added

    Revision 1.1  2001/03/07 17:15:44  marmar
    Example of european option using finite differences

*/

#include "finitedifferenceeuropean.h"
#include "bsmnumericaloption.h"
#include "standardfdmodel.h"

namespace QuantLib {

    namespace Pricers {

        FiniteDifferenceEuropean::FiniteDifferenceEuropean(
            Type type, double underlying, double strike,
            Rate dividendYield, Rate riskFreeRate, Time residualTime,
            double volatility, int timeSteps, int gridPoints)
            : BSMNumericalOption(type, underlying, strike, dividendYield,
                                 riskFreeRate, residualTime, volatility,
                                 gridPoints),
            timeSteps_(timeSteps), euroPrices_(gridPoints_){}


        double FiniteDifferenceEuropean::value() const {
            if (!hasBeenCalculated_) {
                setGridLimits();
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
            return value_;
        }

    }

}
