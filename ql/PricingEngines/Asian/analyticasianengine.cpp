
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

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
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


        Date referenceDate = arguments_.riskFreeTS->referenceDate();
        DayCounter dc = arguments_.volTS->dayCounter();
        std::vector<Time> fixingTimes;
        Size i;
        for (i=0; i<arguments_.fixingDates.size(); i++) {
            if (arguments_.fixingDates[i]>=referenceDate) {
                Time t = dc.yearFraction(referenceDate,
                    arguments_.fixingDates[i]);
                fixingTimes.push_back(t);
            }
        }

        Size pastFixings = arguments_.pastFixings;
        Size remainingFixings = fixingTimes.size();
        double N = pastFixings + remainingFixings;

        double pastWeight   = pastFixings/N;
        double futureWeight = 1.0-pastWeight;


        double timeSum = std::accumulate(fixingTimes.begin(),
            fixingTimes.end(), 0.0);


        double vola = arguments_.volTS->blackVol(
            arguments_.exercise->lastDate(),
            payoff->strike());
        double temp = 0.0;
        for (i=pastFixings+1; i<N; i++)
            temp += fixingTimes[i-pastFixings-1]*(N-i);
        double variance = vola*vola /N/N * 
            (timeSum+ 2.0*temp);


        Rate dividendRate =
            arguments_.dividendTS->zeroYield(arguments_.exercise->lastDate());
        Rate riskFreeRate =
            arguments_.riskFreeTS->zeroYield(arguments_.exercise->lastDate());
        double nu = riskFreeRate - dividendRate - 0.5*vola*vola;
        double runningLog = QL_LOG(arguments_.runningProduct);
        double muG = pastWeight * runningLog +
            futureWeight * QL_LOG(arguments_.underlying) +
            nu*timeSum/N;
        double forwardPrice = QL_EXP(muG + variance / 2.0);


        DiscountFactor riskFreeDiscount =
            arguments_.riskFreeTS->discount(arguments_.exercise->lastDate());

        BlackFormula black(forwardPrice, riskFreeDiscount, variance, payoff);

        results_.value = black.value();
        results_.delta = black.delta(arguments_.underlying);
        // results_.deltaForward = black.value();
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

        results_.strikeSensitivity = black.strikeSensitivity();

    }

}

