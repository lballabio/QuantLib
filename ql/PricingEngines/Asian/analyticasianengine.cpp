
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

#include <ql/PricingEngines/Asian/asianengines.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

    void AnalyticDiscreteAveragingAsianEngine::calculate() const {


        QL_REQUIRE(arguments_.averageType == Average::Geometric,
                   "AnalyticDiscreteAveragingAsianEngine::calculate() : "
                   "not a geometric average option");

        QL_REQUIRE(arguments_.exerciseType == Exercise::European,
                   "AnalyticDiscreteAveragingAsianEngine::calculate() : "
                   "not an European Option");



        #if defined(HAVE_BOOST)
        Handle<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticDiscreteAveragingAsianEngine: non-plain payoff given");
        #else
        Handle<PlainVanillaPayoff> payoff = arguments_.payoff;
        #endif


        double pastWeight = arguments_.pastWeight;
        double futureWeight = 1.0-pastWeight;

        Size remainingFixings = arguments_.fixingTimes.size();
        Size N = remainingFixings / futureWeight;
        Size m = N - remainingFixings;

        double timeSum = std::accumulate(arguments_.fixingTimes.begin(),
            arguments_.fixingTimes.end(), 0.0);


        double vola = arguments_.volTS->blackVol(arguments_.maturity,
            payoff->strike());
        double temp = 0.0;
        for (Size i=m+1; i<N; i++)
            temp += arguments_.fixingTimes[i-m-1]*(N-i);
        double variance = vola*vola /N/N * 
            (timeSum+ 2.0*temp);


        Rate dividendRate =
            arguments_.dividendTS->zeroYield(arguments_.maturity);
        Rate riskFreeRate =
            arguments_.riskFreeTS->zeroYield(arguments_.maturity);
        double nu = riskFreeRate - dividendRate - 0.5*vola*vola;
        double runningAverage = arguments_.runningAverage;
        double runningLogAverage = QL_LOG(runningAverage);
        double muG = pastWeight * runningLogAverage +
            futureWeight * QL_LOG(arguments_.underlying) +
            nu*timeSum/N;
        double forwardPrice = QL_EXP(muG + variance / 2.0);


        DiscountFactor riskFreeDiscount =
            arguments_.riskFreeTS->discount(arguments_.maturity);

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

