
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

        QL_REQUIRE(arguments_.exerciseType == Exercise::European,
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

        double variance = arguments_.volTS->blackVariance(arguments_.maturity,
                                                          payoff->strike());
        DiscountFactor dividendDiscount =
            arguments_.dividendTS->discount(arguments_.maturity);
        DiscountFactor riskFreeDiscount =
            arguments_.riskFreeTS->discount(arguments_.maturity);
        double forwardPrice = arguments_.underlying *
            dividendDiscount / riskFreeDiscount;

        BlackFormula black(arguments_.underlying, forwardPrice, riskFreeDiscount,
            variance, arguments_.maturity, payoff);


        results_.value = black.value();
        results_.delta = black.delta();
        // results_.deltaForward = black.value();
        results_.gamma = black.gamma();
        results_.theta = black.theta();
        results_.rho = black.rho();
        results_.dividendRho = black.dividendRho();
        results_.vega = black.vega();
        results_.strikeSensitivity = black.strikeSensitivity();
    }

}

