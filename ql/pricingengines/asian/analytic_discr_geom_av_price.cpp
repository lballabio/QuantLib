/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2005 Gary Kennedy
 Copyright (C) 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/pricingengines/asian/analytic_discr_geom_av_price.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/greeks.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/exercise.hpp>
#include <numeric>

namespace QuantLib {

    AnalyticDiscreteGeometricAveragePriceAsianEngine::
    AnalyticDiscreteGeometricAveragePriceAsianEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process) {
        registerWith(process_);
    }

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
                       << arguments_.runningAccumulator << " not allowed");
            runningLog =
                std::log(arguments_.runningAccumulator);
            pastFixings = arguments_.pastFixings;
        } else {  // it is being used as control variate
            runningLog = 1.0;
            pastFixings = 0;
        }

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Date referenceDate = process_->riskFreeRate()->referenceDate();
        DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
        DayCounter divdc = process_->dividendYield()->dayCounter();
        DayCounter voldc = process_->blackVolatility()->dayCounter();
        std::vector<Time> fixingTimes;
        Size i;
        for (i=0; i<arguments_.fixingDates.size(); i++) {
            if (arguments_.fixingDates[i]>=referenceDate) {
                Time t = voldc.yearFraction(referenceDate,
                    arguments_.fixingDates[i]);
                fixingTimes.push_back(t);
            }
        }

        Size remainingFixings = fixingTimes.size();
        Size numberOfFixings = pastFixings + remainingFixings;
        Real N = static_cast<Real>(numberOfFixings);

        Real pastWeight   = pastFixings/N;
        Real futureWeight = 1.0-pastWeight;

        Time timeSum = std::accumulate(fixingTimes.begin(),
                                       fixingTimes.end(), 0.0);

        Volatility vola = process_->blackVolatility()->blackVol(
                                              arguments_.exercise->lastDate(),
                                              payoff->strike());
        Real temp = 0.0;
        for (i=pastFixings+1; i<numberOfFixings; i++)
            temp += fixingTimes[i-pastFixings-1]*(N-i);
        Real variance = vola*vola /N/N * (timeSum+ 2.0*temp);
        Real dsigG_dsig = std::sqrt((timeSum + 2.0*temp))/N;
        Real sigG = vola * dsigG_dsig;
        Real dmuG_dsig = -(vola * timeSum)/N;

        Date exDate = arguments_.exercise->lastDate();
        Rate dividendRate = process_->dividendYield()->
            zeroRate(exDate, divdc, Continuous, NoFrequency);
        Rate riskFreeRate = process_->riskFreeRate()->
            zeroRate(exDate, rfdc, Continuous, NoFrequency);
        Rate nu = riskFreeRate - dividendRate - 0.5*vola*vola;

        Real s = process_->stateVariable()->value();
        QL_REQUIRE(s > 0.0, "positive underlying value required");

        Size M = (pastFixings == 0 ? 1 : pastFixings);
        Real muG = pastWeight * runningLog/M +
            futureWeight * std::log(s) + nu*timeSum/N;
        Real forwardPrice = std::exp(muG + variance / 2.0);

        DiscountFactor riskFreeDiscount = process_->riskFreeRate()->discount(
                                             arguments_.exercise->lastDate());

        BlackCalculator black(payoff, forwardPrice, std::sqrt(variance),
                              riskFreeDiscount);

        results_.value = black.value();
        results_.delta = futureWeight*black.delta(forwardPrice)*forwardPrice/s;
        results_.gamma = forwardPrice*futureWeight/(s*s)
                *(  black.gamma(forwardPrice)*futureWeight*forwardPrice
                  - pastWeight*black.delta(forwardPrice) );

        Real Nx_1, nx_1;
        CumulativeNormalDistribution CND;
        NormalDistribution ND;
        if (sigG > QL_EPSILON) {
            Real x_1  = (muG-std::log(payoff->strike())+variance)/sigG;
            Nx_1 = CND(x_1);
            nx_1 = ND(x_1);
        } else {
            Nx_1 = (muG > std::log(payoff->strike()) ? 1.0 : 0.0);
            nx_1 = 0.0;
        }
        results_.vega = forwardPrice * riskFreeDiscount *
                   ( (dmuG_dsig + sigG * dsigG_dsig)*Nx_1 + nx_1*dsigG_dsig );

        if (payoff->optionType() == Option::Put)
            results_.vega -= riskFreeDiscount * forwardPrice *
                                              (dmuG_dsig + sigG * dsigG_dsig);

        Time tRho = rfdc.yearFraction(process_->riskFreeRate()->referenceDate(),
                                      arguments_.exercise->lastDate());
        results_.rho = black.rho(tRho)*timeSum/(N*tRho)
                      - (tRho-timeSum/N)*results_.value;

        Time tDiv = divdc.yearFraction(
                           process_->dividendYield()->referenceDate(),
                           arguments_.exercise->lastDate());

        results_.dividendRho = black.dividendRho(tDiv)*timeSum/(N*tDiv);

        results_.strikeSensitivity = black.strikeSensitivity();

        results_.theta = blackScholesTheta(process_,
                                           results_.value,
                                           results_.delta,
                                           results_.gamma);
    }
}

