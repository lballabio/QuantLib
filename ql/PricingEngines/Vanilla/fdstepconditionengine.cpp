
/*
 Copyright (C) 2005 Joseph Wang

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

#include <ql/PricingEngines/blackformula.hpp>
#include <ql/PricingEngines/Vanilla/fdstepconditionengine.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>

namespace QuantLib {

    void FDStepConditionEngine::calculate() const {
        setGridLimits();
        initializeGrid();
        initializeInitialCondition();
        initializeOperator();
        initializeStepCondition();

        typedef StandardSystemFiniteDifferenceModel model_type;

        model_type::operator_type operatorSet;
        model_type::array_type arraySet;
        model_type::bc_set bcSet;
        model_type::condition_type conditionSet;

        prices_ = intrinsicValues_;

        controlPrices_ = intrinsicValues_;
        controlOperator_ = finiteDifferenceOperator_;
        controlBCs_[0] = BCs_[0];
        controlBCs_[1] = BCs_[1];

        operatorSet.push_back(finiteDifferenceOperator_);
        operatorSet.push_back(controlOperator_);

        arraySet.push_back(prices_);
        arraySet.push_back(controlPrices_);

        bcSet.push_back(BCs_);
        bcSet.push_back(controlBCs_);

        conditionSet.push_back(stepCondition_);
        conditionSet.push_back(boost::shared_ptr<StandardStepCondition>(
                                                   new NullCondition<Array>));

        StandardSystemFiniteDifferenceModel model(operatorSet, bcSet);

        model.rollback(arraySet, getResidualTime(),
                       0.0, timeSteps_, conditionSet);

        Array prices = arraySet[0];
        Array controlPrices = arraySet[1];
        const boost::shared_ptr<BlackScholesProcess>& process =
            arguments_.blackScholesProcess;
        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Real variance =
            process->blackVolatility()->blackVariance(
                                             arguments_.exercise->lastDate(),
                                             payoff->strike());
        DiscountFactor dividendDiscount =
            process->dividendYield()->discount(
                                             arguments_.exercise->lastDate());
        DiscountFactor riskFreeDiscount =
            process->riskFreeRate()->discount(arguments_.exercise->lastDate());
        Real spot = process->stateVariable()->value();
        Real forwardPrice = spot * dividendDiscount / riskFreeDiscount;

        BlackFormula black(forwardPrice, riskFreeDiscount, variance, payoff);

        results_.value = valueAtCenter(prices)
            - valueAtCenter(controlPrices)
            + black.value();
        results_.delta = firstDerivativeAtCenter(prices, grid_)
            - firstDerivativeAtCenter(controlPrices, grid_)
            + black.delta(spot);
        results_.gamma = secondDerivativeAtCenter(prices, grid_)
            - secondDerivativeAtCenter(controlPrices, grid_)
            + black.gamma(spot);
    }

}

