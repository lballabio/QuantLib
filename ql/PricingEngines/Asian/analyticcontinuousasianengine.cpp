
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano

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

#include <ql/PricingEngines/Asian/analyticcontinuousasianengine.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

    void AnalyticContinuousAveragingAsianEngine::calculate() const {

        QL_REQUIRE(arguments_.averageType == Average::Geometric,
                   "not a geometric average option");

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");
        Date exercise = arguments_.exercise->lastDate();

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        boost::shared_ptr<BlackScholesProcess> process =
            arguments_.blackScholesProcess;


        double volatility = 
            process->blackVolatility()->blackVol(exercise, payoff->strike());
        double variance = 
            process->blackVolatility()->blackVariance(exercise,
                                                      payoff->strike());
        DiscountFactor riskFreeDiscount =
            process->riskFreeRate()->discount(exercise);

        Spread dividendYield =
            0.5 * (process->riskFreeRate()->zeroYield(exercise) +
                   process->dividendYield()->zeroYield(exercise) +
                   volatility*volatility/6.0);

        Time t = process->dividendYield()->dayCounter().yearFraction(
                         process->dividendYield()->referenceDate(), exercise);
        DiscountFactor dividendDiscount = QL_EXP(-dividendYield*t);

        double spot = process->stateVariable()->value();
        double forward = spot * dividendDiscount / riskFreeDiscount;

        BlackFormula black(forward, riskFreeDiscount, variance/3.0, payoff);

        results_.value = black.value();
        results_.delta = black.delta(spot);
        results_.gamma = black.gamma(spot);

        t = process->riskFreeRate()->dayCounter().yearFraction(
                                     process->riskFreeRate()->referenceDate(),
                                     arguments_.exercise->lastDate());
        results_.rho = black.rho(t)/2.0;

        t = process->dividendYield()->dayCounter().yearFraction(
                                    process->dividendYield()->referenceDate(),
                                    arguments_.exercise->lastDate());
        results_.dividendRho = black.dividendRho(t)/2.0;

        t = process->blackVolatility()->dayCounter().yearFraction(
                                  process->blackVolatility()->referenceDate(),
                                  arguments_.exercise->lastDate());
        results_.vega = black.vega(t)/QL_SQRT(3.0) -
                        results_.rho*volatility*volatility/6.0;
        try {
            results_.theta = black.theta(spot, t);
        } catch (Error&) {
            results_.theta = Null<double>();
        }
    }

}

