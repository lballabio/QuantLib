
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

#include <ql/PricingEngines/Asian/analyticasianengine.hpp>
#include <ql/PricingEngines/blackformula.hpp>

namespace QuantLib {

    void AnalyticDiscreteAveragingAsianEngine::calculate() const {

        QL_REQUIRE(arguments_.averageType == Average::Geometric,
                   "not a geometric average option");

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        boost::shared_ptr<BlackScholesStochasticProcess> process =
            arguments_.blackScholesProcess;
        Date referenceDate = process->riskFreeTS->referenceDate();
        DayCounter dc = process->volTS->dayCounter();
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


        double vola = process->volTS->blackVol(arguments_.exercise->lastDate(),
                                               payoff->strike());
        double temp = 0.0;
        for (i=pastFixings+1; i<N; i++)
            temp += fixingTimes[i-pastFixings-1]*(N-i);
        double variance = vola*vola /N/N * 
            (timeSum+ 2.0*temp);

        Rate dividendRate =
            process->dividendTS->zeroYield(arguments_.exercise->lastDate());
        Rate riskFreeRate =
            process->riskFreeTS->zeroYield(arguments_.exercise->lastDate());
        double nu = riskFreeRate - dividendRate - 0.5*vola*vola;
        double runningLog = QL_LOG(arguments_.runningProduct);
        double muG = pastWeight * runningLog +
            futureWeight * QL_LOG(process->stateVariable->value()) +
            nu*timeSum/N;
        double forwardPrice = QL_EXP(muG + variance / 2.0);

        DiscountFactor riskFreeDiscount =
            process->riskFreeTS->discount(arguments_.exercise->lastDate());

        BlackFormula black(forwardPrice, riskFreeDiscount, variance, payoff);

        results_.value = black.value();
        results_.delta = black.delta(process->stateVariable->value());
        // results_.deltaForward = black.value();
        results_.gamma = black.gamma(process->stateVariable->value());

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
        results_.theta = black.theta(process->stateVariable->value(), t);

        results_.strikeSensitivity = black.strikeSensitivity();

    }

}

