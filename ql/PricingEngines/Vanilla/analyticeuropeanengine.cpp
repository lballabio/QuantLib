
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

        double variance = arguments_.volTS->blackVariance(arguments_.exercise->lastDate(),
                                                          payoff->strike());
        DiscountFactor dividendDiscount =
            arguments_.dividendTS->discount(arguments_.exercise->lastDate());
        DiscountFactor riskFreeDiscount =
            arguments_.riskFreeTS->discount(arguments_.exercise->lastDate());
        double forwardPrice = arguments_.underlying *
            dividendDiscount / riskFreeDiscount;

        BlackFormula black(forwardPrice, riskFreeDiscount, variance, payoff);


        results_.value = black.value();
        results_.delta = black.delta(arguments_.underlying);
        results_.deltaForward = black.deltaForward();
        results_.elasticity = black.elasticity(arguments_.underlying);
        results_.gamma = black.gamma(arguments_.underlying);

        Time t = arguments_.riskFreeTS->dayCounter().yearFraction(
            arguments_.riskFreeTS->referenceDate(),
            arguments_.exercise->lastDate());
        results_.rho = black.rho(t);

        t = arguments_.dividendTS->dayCounter().yearFraction(
            arguments_.dividendTS->referenceDate(),
            arguments_.exercise->lastDate());
        results_.dividendRho = black.dividendRho(t);

        t = arguments_.volTS->dayCounter().yearFraction(
            arguments_.volTS->referenceDate(),
            arguments_.exercise->lastDate());
        results_.vega = black.vega(t);
        results_.theta = black.theta(arguments_.underlying, t);
        results_.thetaPerDay = black.thetaPerDay(arguments_.underlying, t);

        results_.strikeSensitivity = black.strikeSensitivity();
        results_.itmProbability = black.itmProbability();
    }

}

