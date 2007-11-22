/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    AnalyticEuropeanEngine::AnalyticEuropeanEngine(
              const boost::shared_ptr<GeneralizedBlackScholesProcess>&process)
    : process_(process) {
        registerWith(process_);
    }

    void AnalyticEuropeanEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Real variance =
            process_->blackVolatility()->blackVariance(
                                              arguments_.exercise->lastDate(),
                                              payoff->strike());
        DiscountFactor dividendDiscount =
            process_->dividendYield()->discount(
                                             arguments_.exercise->lastDate());
        DiscountFactor riskFreeDiscount =
            process_->riskFreeRate()->discount(arguments_.exercise->lastDate());
        Real spot = process_->stateVariable()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");
        Real forwardPrice = spot * dividendDiscount / riskFreeDiscount;

        BlackCalculator black(payoff, forwardPrice, std::sqrt(variance),
                              riskFreeDiscount);


        results_.value = black.value();
        results_.delta = black.delta(spot);
        results_.deltaForward = black.deltaForward();
        results_.elasticity = black.elasticity(spot);
        results_.gamma = black.gamma(spot);

        DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
        DayCounter divdc = process_->dividendYield()->dayCounter();
        DayCounter voldc = process_->blackVolatility()->dayCounter();
        Time t = rfdc.yearFraction(process_->riskFreeRate()->referenceDate(),
                                   arguments_.exercise->lastDate());
        results_.rho = black.rho(t);

        t = divdc.yearFraction(process_->dividendYield()->referenceDate(),
                               arguments_.exercise->lastDate());
        results_.dividendRho = black.dividendRho(t);

        t = voldc.yearFraction(process_->blackVolatility()->referenceDate(),
                               arguments_.exercise->lastDate());
        results_.vega = black.vega(t);
        try {
            results_.theta = black.theta(spot, t);
            results_.thetaPerDay =
                black.thetaPerDay(spot, t);
        } catch (Error&) {
            results_.theta = Null<Real>();
            results_.thetaPerDay = Null<Real>();
        }

        results_.strikeSensitivity  = black.strikeSensitivity();
        results_.itmCashProbability = black.itmCashProbability();
    }

}

