
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Pricers/fdbermudanoption.hpp>
#include <ql/Pricers/europeanoption.hpp>

namespace QuantLib {

    FdBermudanOption::FdBermudanOption(Option::Type type, double underlying,
                                       double strike, Spread dividendYield, 
                                       Rate riskFreeRate, Time residualTime, 
                                       double volatility,
                                       const std::vector<Time>& dates,
                                       int timeSteps, int gridPoints)
    : FdMultiPeriodOption(type, underlying, strike, dividendYield,
                          riskFreeRate, residualTime, volatility, 
                          gridPoints, dates, timeSteps) {}

    void FdBermudanOption::initializeStepCondition() const{
        stepCondition_ = Handle<StandardStepCondition> ();
    }

    void FdBermudanOption::executeIntermediateStep(int ) const{

        int size = intrinsicValues_.size();
        for(int j = 0; j < size; j++)
            prices_[j] = QL_MAX(prices_[j], intrinsicValues_[j]);
    }

    Handle<SingleAssetOption> FdBermudanOption::clone() const {
        return Handle<SingleAssetOption>(new FdBermudanOption(
                       payoff_.optionType(), underlying_, payoff_.strike(), 
                       dividendYield_, riskFreeRate_, residualTime_, 
                       volatility_, dates_, timeStepPerPeriod_, gridPoints_));
    }

}
