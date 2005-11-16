/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/blackformula.hpp>
#include <ql/PricingEngines/Vanilla/fdstepconditionengine.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>

namespace QuantLib {

    void FDStepConditionEngine::calculate(OneAssetOption::results* results) const {
        setGridLimits();
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

        arraySet.push_back(prices_.values());
        arraySet.push_back(controlPrices_.values());

        bcSet.push_back(BCs_);
        bcSet.push_back(controlBCs_);

        conditionSet.push_back(stepCondition_);
        conditionSet.push_back(boost::shared_ptr<StandardStepCondition>(
                                                   new NullCondition<Array>));

        StandardSystemFiniteDifferenceModel model(operatorSet, bcSet);

        model.rollback(arraySet, getResidualTime(),
                       0.0, timeSteps_, conditionSet);

        prices_.values() = arraySet[0];
        controlPrices_.values() = arraySet[1];

        boost::shared_ptr<StrikedTypePayoff> striked_payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(payoff_);
        QL_REQUIRE(striked_payoff, "non-striked payoff given");

        Real variance =
            process_->blackVolatility()->
            blackVariance(exerciseDate_,
                          striked_payoff->strike());
        DiscountFactor dividendDiscount =
            process_->dividendYield()->discount(exerciseDate_);
        DiscountFactor riskFreeDiscount =
            process_->riskFreeRate()->discount(exerciseDate_);
        Real spot = process_->stateVariable()->value();
        Real forwardPrice = spot * dividendDiscount / riskFreeDiscount;

        BlackFormula black(forwardPrice, riskFreeDiscount, 
                           variance, striked_payoff);

        results->value = prices_.valueAtCenter()
            - controlPrices_.valueAtCenter()
            + black.value();
        results->delta = prices_.firstDerivativeAtCenter()
            - controlPrices_.firstDerivativeAtCenter()
            + black.delta(spot);
        results->gamma = prices_.secondDerivativeAtCenter()
            - controlPrices_.secondDerivativeAtCenter()
            + black.gamma(spot);
        results->priceCurve = prices_;
    }

}

