
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
    Revision 1.11  2001/05/24 13:57:52  nando
    smoothing #include xx.hpp and cutting old Log messages

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
