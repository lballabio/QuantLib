/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2004 Ferdinando Ametrano
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

#include <ql/pricingengines/vanilla/analyticdigitalamericanengine.hpp>
#include <ql/pricingengines/americanpayoffathit.hpp>
#include <ql/pricingengines/americanpayoffatexpiry.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    AnalyticDigitalAmericanEngine::AnalyticDigitalAmericanEngine(
              const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process) {
        registerWith(process_);
    }

    void AnalyticDigitalAmericanEngine::calculate() const {

        boost::shared_ptr<AmericanExercise> ex =
            boost::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex, "non-American exercise given");
        QL_REQUIRE(ex->dates()[0] <=
                   process_->blackVolatility()->referenceDate(),
                   "American option with window exercise not handled yet");

        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Real spot = process_->stateVariable()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        Real variance =
            process_->blackVolatility()->blackVariance(ex->lastDate(),
                                                       payoff->strike());
        Rate dividendDiscount =
            process_->dividendYield()->discount(ex->lastDate());
        Rate riskFreeDiscount =
            process_->riskFreeRate()->discount(ex->lastDate());

        if(ex->payoffAtExpiry()) {
            AmericanPayoffAtExpiry pricer(spot, riskFreeDiscount,
                                          dividendDiscount, variance, payoff);
            results_.value = pricer.value();
        } else {
            AmericanPayoffAtHit pricer(spot, riskFreeDiscount,
                                       dividendDiscount, variance, payoff);
            results_.value = pricer.value();
            results_.delta = pricer.delta();
            results_.gamma = pricer.gamma();

            DayCounter rfdc = process_->riskFreeRate()->dayCounter();
            Time t = rfdc.yearFraction(
                                    process_->riskFreeRate()->referenceDate(),
                                    arguments_.exercise->lastDate());
            results_.rho = pricer.rho(t);
        }
    }

}

