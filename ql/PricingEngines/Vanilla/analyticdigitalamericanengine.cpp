/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2003 Neil Firth

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

#include <ql/PricingEngines/Vanilla/analyticdigitalamericanengine.hpp>
#include <ql/PricingEngines/americanpayoffathit.hpp>
#include <ql/PricingEngines/americanpayoffatexpiry.hpp>
#include <ql/Processes/blackscholesprocess.hpp>

namespace QuantLib {

    void AnalyticDigitalAmericanEngine::calculate() const {

        boost::shared_ptr<BlackScholesProcess> process =
            boost::dynamic_pointer_cast<BlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");

        boost::shared_ptr<AmericanExercise> ex =
            boost::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex, "non-American exercise given");
        QL_REQUIRE(ex->dates()[0] <=
                   process->blackVolatility()->referenceDate(),
                   "American option with window exercise not handled yet");

        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Real spot = process->stateVariable()->value();
        Real variance =
            process->blackVolatility()->blackVariance(ex->lastDate(),
                                                 payoff->strike());
        Rate dividendDiscount =
            process->dividendYield()->discount(ex->lastDate());
        Rate riskFreeDiscount =
            process->riskFreeRate()->discount(ex->lastDate());

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

            DayCounter rfdc = process->riskFreeRate()->dayCounter();
            Time t = rfdc.yearFraction(
                                     process->riskFreeRate()->referenceDate(),
                                     arguments_.exercise->lastDate());
            results_.rho = pricer.rho(t);
        }
    }

}

