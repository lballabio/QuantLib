
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

#include <ql/PricingEngines/Asian/analytic_discr_geom_av_price.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <numeric>

namespace QuantLib {

    void AnalyticDiscreteGeometricAveragePriceAsianEngine::calculate() const {

        /* this engine cannot really check for the averageType==Geometric
           since it can be used as control variate for the Arithmetic version
        QL_REQUIRE(arguments_.averageType == Average::Geometric,
                   "not a geometric average option");
       */

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        Real runningLog;
        Size pastFixings;
        if (arguments_.averageType == Average::Geometric) {
            QL_REQUIRE(arguments_.runningAccumulator>0.0,
                "positive running product required: "
                + DecimalFormatter::toString(arguments_.runningAccumulator) +
                " not allowed");
            runningLog = QL_LOG(arguments_.runningAccumulator);
            pastFixings = arguments_.pastFixings;
        } else {  // it is being used as control variate
            runningLog = 1.0;
            pastFixings = 0;
        }

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        const boost::shared_ptr<BlackScholesProcess>& process =
            arguments_.blackScholesProcess;
        Date referenceDate = process->riskFreeRate()->referenceDate();
        DayCounter dc = process->blackVolatility()->dayCounter();
        std::vector<Time> fixingTimes;
        Size i;
        for (i=0; i<arguments_.fixingDates.size(); i++) {
            if (arguments_.fixingDates[i]>=referenceDate) {
                Time t = dc.yearFraction(referenceDate,
                    arguments_.fixingDates[i]);
                fixingTimes.push_back(t);
            }
        }

        Size remainingFixings = fixingTimes.size();
        Real N = Real(pastFixings + remainingFixings);

        Real pastWeight   = pastFixings/N;
        Real futureWeight = 1.0-pastWeight;


        Time timeSum = std::accumulate(fixingTimes.begin(),
                                       fixingTimes.end(), 0.0);


        Volatility vola = process->blackVolatility()->blackVol(
                                              arguments_.exercise->lastDate(),
                                              payoff->strike());
        Real temp = 0.0;
        for (i=pastFixings+1; i<N; i++)
            temp += fixingTimes[i-pastFixings-1]*(N-i);
        Real variance = vola*vola /N/N * (timeSum+ 2.0*temp);

        Rate dividendRate = process->dividendYield()->zeroYield(
                                             arguments_.exercise->lastDate());
        Rate riskFreeRate = process->riskFreeRate()->zeroYield(
                                             arguments_.exercise->lastDate());
        Rate nu = riskFreeRate - dividendRate - 0.5*vola*vola;
        Real muG = pastWeight * runningLog +
            futureWeight * QL_LOG(process->stateVariable()->value()) +
            nu*timeSum/N;
        Real forwardPrice = QL_EXP(muG + variance / 2.0);

        DiscountFactor riskFreeDiscount = process->riskFreeRate()->discount(
                                             arguments_.exercise->lastDate());

        BlackFormula black(forwardPrice, riskFreeDiscount, variance, payoff);

        results_.value = black.value();
        results_.delta = black.delta(process->stateVariable()->value());
        // results_.deltaForward = black.value();
        results_.gamma = black.gamma(process->stateVariable()->value());

        #ifndef QL_DISABLE_DEPRECATED
        DayCounter rfdc = process->riskFreeRate()->dayCounter();
        DayCounter divdc = process->dividendYield()->dayCounter();
        #else
        DayCounter rfdc = Settings::instance().dayCounter();
        DayCounter divdc = Settings::instance().dayCounter();
        #endif
        Time t = rfdc.yearFraction(process->riskFreeRate()->referenceDate(),
                                   arguments_.exercise->lastDate());
        results_.rho = black.rho(t);

        t = divdc.yearFraction(process->dividendYield()->referenceDate(),
                               arguments_.exercise->lastDate());
        results_.dividendRho = black.dividendRho(t);

        t = process->blackVolatility()->dayCounter().yearFraction(
                                  process->blackVolatility()->referenceDate(),
                                  arguments_.exercise->lastDate());
        results_.vega = black.vega(t);
        results_.theta = black.theta(process->stateVariable()->value(), t);

        results_.strikeSensitivity = black.strikeSensitivity();

    }

}

