
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2003 Neil Firth

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

#include <ql/PricingEngines/Vanilla/analyticdigitalamericanengine.hpp>
#include <ql/PricingEngines/americanpayoffathit.hpp>
#include <ql/PricingEngines/americanpayoffatexpiry.hpp>

namespace QuantLib {

    void AnalyticDigitalAmericanEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::American,
                   "not an American Option");

        boost::shared_ptr<AmericanExercise> ex = 
            boost::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex, "non-American exercise given");
        QL_REQUIRE(ex->dates()[0]<=
                   arguments_.blackScholesProcess->volTS->referenceDate(),
                   "American option with window exercise not handled yet");

        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        const boost::shared_ptr<BlackScholesStochasticProcess>& process = 
            arguments_.blackScholesProcess;

        double spot = process->stateVariable->value();
        double variance = process->volTS->blackVariance(ex->lastDate(), 
                                                        payoff->strike());
        Rate dividendDiscount = process->dividendTS->discount(ex->lastDate());
        Rate riskFreeDiscount = process->riskFreeTS->discount(ex->lastDate());

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

            Time t = process->riskFreeTS->dayCounter().yearFraction(
                                         process->riskFreeTS->referenceDate(),
                                         arguments_.exercise->lastDate());
            results_.rho = pricer.rho(t);
        }
    }

}

