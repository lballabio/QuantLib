
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#include <ql/PricingEngines/Vanilla/analyticeuropeanengine.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

    void AnalyticEuropeanEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "AnalyticEuropeanEngine::calculate() : "
                   "not an European Option");

        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticEuropeanEngine: non-striked payoff given");

        const boost::shared_ptr<BlackScholesStochasticProcess>& process =
            arguments_.blackScholesProcess;

        double variance = 
            process->volTS->blackVariance(arguments_.exercise->lastDate(),
                                          payoff->strike());
        DiscountFactor dividendDiscount =
            process->dividendTS->discount(arguments_.exercise->lastDate());
        DiscountFactor riskFreeDiscount =
            process->riskFreeTS->discount(arguments_.exercise->lastDate());
        double spot = process->stateVariable->value();
        double forwardPrice = spot *
            dividendDiscount / riskFreeDiscount;

        BlackFormula black(forwardPrice, riskFreeDiscount, variance, payoff);


        results_.value = black.value();
        results_.delta = black.delta(spot);
        results_.deltaForward = black.deltaForward();
        results_.elasticity = black.elasticity(spot);
        results_.gamma = black.gamma(spot);

        Time t = process->riskFreeTS->dayCounter().yearFraction(
            process->riskFreeTS->referenceDate(),
            arguments_.exercise->lastDate());
        results_.rho = black.rho(t);

        t = process->dividendTS->dayCounter().yearFraction(
                process->dividendTS->referenceDate(),
                arguments_.exercise->lastDate());
        results_.dividendRho = black.dividendRho(t);

        t = process->volTS->dayCounter().yearFraction(
                process->volTS->referenceDate(),
                arguments_.exercise->lastDate());
        results_.vega = black.vega(t);
        try {
            results_.theta = black.theta(spot, t);
            results_.thetaPerDay = 
                black.thetaPerDay(spot, t);
        } catch (Error&) {
            results_.theta = Null<double>();
            results_.thetaPerDay = Null<double>();
        }

        results_.strikeSensitivity  = black.strikeSensitivity();
        results_.itmCashProbability = black.itmCashProbability();
    }

}

