
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

#include <ql/Pricers/fdstepconditionoption.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>

namespace QuantLib {

    FdStepConditionOption::FdStepConditionOption(
                         Option::Type type, double underlying, double strike, 
                         Spread dividendYield, Rate riskFreeRate, 
                         Time residualTime, double volatility,
                         Size timeSteps, Size gridPoints)
    : FdBsmOption(type, underlying, strike, dividendYield,
                  riskFreeRate, residualTime, volatility, gridPoints),
      timeSteps_(timeSteps) {}

    void FdStepConditionOption::calculate() const {

        setGridLimits(underlying_, residualTime_);
        initializeGrid();
        initializeInitialCondition();
        initializeOperator();
        initializeStepCondition();
        /* StandardFiniteDifferenceModel is Crank-Nicolson.
           Alternatively, ImplicitEuler or ExplicitEuler
           could have been used instead*/
        StandardFiniteDifferenceModel model(finiteDifferenceOperator_,
                                            BCs_);

        // Control-variate variance reduction:
        // 1) calculate value/greeks of the European option analytically
        double discount = QL_EXP(-riskFreeRate_*residualTime_);
        double qDiscount = QL_EXP(-dividendYield_*residualTime_);
        double forward = underlying_*qDiscount/discount;
        double variance = volatility_*volatility_*residualTime_;
        boost::shared_ptr<StrikedTypePayoff> payoff(
                                             new PlainVanillaPayoff(payoff_));
        BlackFormula black(forward, discount, variance, payoff);

        // 2) Initialize prices on the grid
        Array europeanPrices = intrinsicValues_;
        Array americanPrices = intrinsicValues_;

        // 3) Rollback
        model.rollback(europeanPrices, residualTime_, 0.0, timeSteps_);
        model.rollback(americanPrices, residualTime_, 0.0, timeSteps_,
                       stepCondition_);

        /* 4) Numerically calculate option value and greeks using
           the European option as control variate                */

        value_ =  valueAtCenter(americanPrices)
            - valueAtCenter(europeanPrices)
            + black.value();

        delta_ =   firstDerivativeAtCenter(americanPrices, grid_)
            - firstDerivativeAtCenter(europeanPrices, grid_)
            + black.delta(underlying_);

        gamma_ =   secondDerivativeAtCenter(americanPrices, grid_)
            - secondDerivativeAtCenter(europeanPrices, grid_)
            + black.gamma(underlying_);

        hasBeenCalculated_ = true;
    }

}
