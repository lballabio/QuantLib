
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

#include <ql/Pricers/fdeuropean.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED

    FdEuropean::FdEuropean(Option::Type type, Real underlying,
                           Real strike, Spread dividendYield,
                           Rate riskFreeRate, Time residualTime,
                           Volatility volatility,
                           Size timeSteps, Size gridPoints)
    : FdBsmOption(type, underlying, strike, dividendYield,
                  riskFreeRate, residualTime, volatility,
                  gridPoints),
      timeSteps_(timeSteps), euroPrices_(gridPoints_){}


    void FdEuropean::calculate() const {
        setGridLimits(underlying_, residualTime_);
        initializeGrid();
        initializeInitialCondition();
        initializeOperator();

        StandardFiniteDifferenceModel model(finiteDifferenceOperator_,
                                            BCs_);

        euroPrices_ = intrinsicValues_;

        model.rollback(euroPrices_, residualTime_, 0, timeSteps_);

        value_ = valueAtCenter(euroPrices_);
        delta_ = firstDerivativeAtCenter(euroPrices_, grid_);
        gamma_ = secondDerivativeAtCenter(euroPrices_, grid_);

        hasBeenCalculated_ = true;
    }

    #endif

}
