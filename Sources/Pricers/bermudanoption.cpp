
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

/*! \file bermudanoption.cpp
    \brief  class for finite-difference evaluation of bermudan option

    $Source$
    $Log$
    Revision 1.10  2001/05/24 13:57:52  nando
    smoothing #include xx.hpp and cutting old Log messages

*/

#include "ql/Pricers/bermudanoption.hpp"
#include "ql/Pricers/bsmeuropeanoption.hpp"

namespace QuantLib {

    namespace Pricers {

        BermudanOption::BermudanOption(Type type, double underlying,
            double strike, Rate dividendYield, Rate riskFreeRate,
            Time residualTime, double volatility,
            const std::vector<Time>& dates,
            int timeSteps, int gridPoints)
        : MultiPeriodOption(type, underlying, strike, dividendYield,
          riskFreeRate, residualTime, volatility, dates, timeSteps,
          gridPoints) {}

        using FiniteDifferences::StandardStepCondition;

        void BermudanOption::initializeStepCondition() const{
            stepCondition_ = Handle<StandardStepCondition> ();
        }

        void BermudanOption::executeIntermediateStep(int step) const{

            // to avoid warning
            step;
            int size = initialPrices_.size();
            for(int j = 0; j < size; j++)
                prices_[j] = QL_MAX(prices_[j], initialPrices_[j]);
        }

        Handle<BSMOption> BermudanOption::clone() const {
            return Handle<BSMOption>(new BermudanOption(type_, underlying_,
                strike_, dividendYield_, riskFreeRate_, residualTime_,
                volatility_, dates_, timeStepPerPeriod_, gridPoints_));
        }

    }

}
