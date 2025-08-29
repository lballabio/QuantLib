/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/exercise.hpp>
#include <ql/pricingengines/basket/spreadblackscholesvanillaengine.hpp>

namespace QuantLib {

    SpreadBlackScholesVanillaEngine::SpreadBlackScholesVanillaEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
        ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
        Real correlation)
    : process1_(std::move(process1)),
      process2_(std::move(process2)),
      rho_(correlation) {
        registerWith(process1_);
        registerWith(process2_);
    }

    void SpreadBlackScholesVanillaEngine::calculate() const {
        const ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not an European exercise");

        const ext::shared_ptr<SpreadBasketPayoff> spreadPayoff =
            ext::dynamic_pointer_cast<SpreadBasketPayoff>(arguments_.payoff);
        QL_REQUIRE(spreadPayoff," spread payoff expected");

        const ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(
                                                   spreadPayoff->basePayoff());
        QL_REQUIRE(payoff, "non-plain payoff given");
        const Real strike = payoff->strike();
        const Option::Type optionType = payoff->optionType();


        const Date maturityDate = exercise->lastDate();
        const Real f1 = process1_->stateVariable()->value()
            / process1_->riskFreeRate()->discount(maturityDate)
            * process1_->dividendYield()->discount(maturityDate);

        const Real f2 = process2_->stateVariable()->value()
            / process2_->riskFreeRate()->discount(maturityDate)
            * process2_->dividendYield()->discount(maturityDate);

        const Real variance1 =
            process1_->blackVolatility()->blackVariance(maturityDate, f1);
        const Real variance2 =
            process2_->blackVolatility()->blackVariance(maturityDate, f2);

        const DiscountFactor df =
            process1_->riskFreeRate()->discount(exercise->lastDate());

        results_.value = calculate(f1, f2, strike, optionType, variance1, variance2, df);
    }
}
