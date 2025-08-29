/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/exercise.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/asian/analytic_discr_geom_av_strike.hpp>
#include <utility>

namespace QuantLib {

    AnalyticDiscreteGeometricAverageStrikeAsianEngine::
        AnalyticDiscreteGeometricAverageStrikeAsianEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticDiscreteGeometricAverageStrikeAsianEngine::calculate() const {

        QL_REQUIRE(arguments_.averageType == Average::Geometric,
                   "not a geometric average option");

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        QL_REQUIRE(arguments_.runningAccumulator > 0.0,
                   "positive running product required: "
                   << arguments_.runningAccumulator << "not allowed");
        Real runningLog = std::log(arguments_.runningAccumulator);
        Size pastFixings = arguments_.pastFixings;
        QL_REQUIRE(pastFixings == 0, "past fixings currently not managed");

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
        DayCounter divdc = process_->dividendYield()->dayCounter();
        DayCounter voldc = process_->blackVolatility()->dayCounter();

        std::vector<Time> fixingTimes;
        for (auto& fixingDate : arguments_.fixingDates) {
            if (fixingDate >= arguments_.fixingDates[0]) {
                Time t = voldc.yearFraction(arguments_.fixingDates[0], fixingDate);
                fixingTimes.push_back(t);
            }
        }

        Size remainingFixings = fixingTimes.size();
        Size numberOfFixings = pastFixings + remainingFixings;
        Real N = static_cast<Real>(numberOfFixings);

        Real pastWeight   = pastFixings/N;
        Real futureWeight = 1.0-pastWeight;

        Time timeSum = std::accumulate(fixingTimes.begin(),
                                       fixingTimes.end(), Real(0.0));

        Time residualTime = rfdc.yearFraction(arguments_.fixingDates[pastFixings],
                                              arguments_.exercise->lastDate());


        Real underlying = process_->stateVariable()->value();
        QL_REQUIRE(underlying > 0.0, "positive underlying value required");

        Volatility volatility = process_->blackVolatility()->blackVol(
                                                arguments_.exercise->lastDate(),
                                                underlying);

        Date exDate = arguments_.exercise->lastDate();
        Rate dividendRate = process_->dividendYield()->
            zeroRate(exDate, divdc, Continuous, NoFrequency);

        Rate riskFreeRate = process_->riskFreeRate()->
            zeroRate(exDate, rfdc, Continuous, NoFrequency);

        Rate nu = riskFreeRate - dividendRate - 0.5*volatility*volatility;

        Real temp = 0.0;
        for (Size i=pastFixings+1; i<numberOfFixings; i++)
            temp += fixingTimes[i-pastFixings-1]*(N-i);
        Real variance = volatility*volatility /N/N * (timeSum + 2.0*temp);
        Real covarianceTerm = volatility*volatility/N * timeSum;
        Real sigmaSum_2 = variance + volatility*volatility*residualTime -
                                     2.0*covarianceTerm;

        Size M = (pastFixings == 0 ? 1 : pastFixings);
        Real runningLogAverage = runningLog/M;

        Real muG = pastWeight * runningLogAverage +
                   futureWeight * std::log(underlying) +
                   nu*timeSum/N;

        CumulativeNormalDistribution f;

        Real y1 = (std::log(underlying)+
                     (riskFreeRate-dividendRate)*residualTime-
                      muG - variance/2.0 + sigmaSum_2/2.0)
                      /std::sqrt(sigmaSum_2);
        Real y2 = y1-std::sqrt(sigmaSum_2);

        switch (payoff->optionType()) {
          case Option::Call:
            results_.value = underlying*std::exp(-dividendRate*residualTime)
                *f(y1)-
                std::exp(muG + variance/2.0 - riskFreeRate*residualTime)
                *f(y2);
            break;
          case Option::Put:
            results_.value = -underlying*std::exp(-dividendRate*residualTime)
                *f(-y1)+
                std::exp(muG + variance/2.0 - riskFreeRate*residualTime)
                *f(-y2);
            break;
          default:
            QL_FAIL("invalid option type");
        }
    }

}

