
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

/*! \file analyticeuropeanengine.cpp
    \brief European option engine using analytic formulas
*/

#include <ql/PricingEngines/Vanilla/vanillaengines.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

    void AnalyticEuropeanEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "AnalyticEuropeanEngine::calculate() : "
                   "not an European Option");

        #if defined(HAVE_BOOST)
        Handle<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticEuropeanEngine: non-striked payoff given");
        #else
        Handle<StrikedTypePayoff> payoff = arguments_.payoff;
        #endif

        double variance = arguments_.blackScholesProcess->volTS->blackVariance(arguments_.exercise->lastDate(),
                                                          payoff->strike());
        DiscountFactor dividendDiscount =
            arguments_.blackScholesProcess->dividendTS->discount(arguments_.exercise->lastDate());
        DiscountFactor riskFreeDiscount =
            arguments_.blackScholesProcess->riskFreeTS->discount(arguments_.exercise->lastDate());
        double forwardPrice = arguments_.blackScholesProcess->stateVariable->value() *
            dividendDiscount / riskFreeDiscount;

        BlackFormula black(forwardPrice, riskFreeDiscount, variance, payoff);


        results_.value = black.value();
        results_.delta = black.delta(arguments_.blackScholesProcess->stateVariable->value());
        results_.deltaForward = black.deltaForward();
        results_.elasticity = black.elasticity(arguments_.blackScholesProcess->stateVariable->value());
        results_.gamma = black.gamma(arguments_.blackScholesProcess->stateVariable->value());

        Time t = arguments_.blackScholesProcess->riskFreeTS->dayCounter().yearFraction(
            arguments_.blackScholesProcess->riskFreeTS->referenceDate(),
            arguments_.exercise->lastDate());
        results_.rho = black.rho(t);

        t = arguments_.blackScholesProcess->dividendTS->dayCounter().yearFraction(
            arguments_.blackScholesProcess->dividendTS->referenceDate(),
            arguments_.exercise->lastDate());
        results_.dividendRho = black.dividendRho(t);

        t = arguments_.blackScholesProcess->volTS->dayCounter().yearFraction(
            arguments_.blackScholesProcess->volTS->referenceDate(),
            arguments_.exercise->lastDate());
        results_.vega = black.vega(t);
        results_.theta = black.theta(arguments_.blackScholesProcess->stateVariable->value(), t);
        results_.thetaPerDay = black.thetaPerDay(arguments_.blackScholesProcess->stateVariable->value(), t);

        results_.strikeSensitivity = black.strikeSensitivity();
        results_.itmProbability = black.itmProbability();
    }

}

