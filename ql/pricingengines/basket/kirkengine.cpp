/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen
 
 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/exercise.hpp>
#include <ql/math/functional.hpp>
#include <ql/pricingengines/basket/kirkengine.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <utility>

namespace QuantLib {

    KirkEngine::KirkEngine(std::shared_ptr<BlackProcess> process1,
                           std::shared_ptr<BlackProcess> process2,
                           Real correlation)
    : process1_(std::move(process1)), process2_(std::move(process2)), rho_(correlation) {
        registerWith(process1_);
        registerWith(process2_);
    }

    void KirkEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not a European option");

        std::shared_ptr<EuropeanExercise> exercise =
            std::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not a European exercise");

        std::shared_ptr<SpreadBasketPayoff> spreadPayoff =
            std::dynamic_pointer_cast<SpreadBasketPayoff>(arguments_.payoff);
        QL_REQUIRE(spreadPayoff," spread payoff expected");

        std::shared_ptr<PlainVanillaPayoff> payoff =
            std::dynamic_pointer_cast<PlainVanillaPayoff>(
                                                   spreadPayoff->basePayoff());
        QL_REQUIRE(payoff, "non-plain payoff given");
        const Real strike = payoff->strike();
        
        const Real f1 = process1_->stateVariable()->value();
        const Real f2 = process2_->stateVariable()->value();

        // use atm vols
        const Real variance1 = process1_->blackVolatility()->blackVariance(
                                                exercise->lastDate(), f1);
        const Real variance2 = process2_->blackVolatility()->blackVariance(
                                                exercise->lastDate(), f2);

        const DiscountFactor riskFreeDiscount =
            process1_->riskFreeRate()->discount(exercise->lastDate());

        const Real f = f1/(f2 + strike);
        const Real v 
            = std::sqrt(variance1 
                        + variance2*squared(f2/(f2+strike))
                        - 2*rho_*std::sqrt(variance1*variance2)
                            *(f2/(f2+strike)));
        
        BlackCalculator black(
             std::make_shared<PlainVanillaPayoff>(
                 payoff->optionType(),1.0),
             f, v, riskFreeDiscount);
        
        results_.value = (f2 + strike)*black.value();
    }
}

