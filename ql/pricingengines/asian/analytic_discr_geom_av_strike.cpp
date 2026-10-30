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
#include <ql/pricingengines/asian/analytic_discr_geom_av_strike.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
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

        Date referenceDate = process_->riskFreeRate()->referenceDate();
        Date volReferenceDate = process_->blackVolatility()->referenceDate();
        DayCounter voldc = process_->blackVolatility()->dayCounter();

        std::vector<Time> fixingTimes;
        Real driftSum = 0.0;
        for (auto& fixingDate : arguments_.fixingDates) {
            if (fixingDate >= referenceDate) {
                Time t = voldc.yearFraction(volReferenceDate, fixingDate);
                fixingTimes.push_back(t);
                driftSum += std::log(process_->dividendYield()->discount(fixingDate) /
                                     process_->riskFreeRate()->discount(fixingDate));
            }
        }
        QL_REQUIRE(!fixingTimes.empty(), "no future fixings");

        Size remainingFixings = fixingTimes.size();
        Size numberOfFixings = pastFixings + remainingFixings;
        Real N = static_cast<Real>(numberOfFixings);

        Real pastWeight   = pastFixings/N;
        Real futureWeight = 1.0-pastWeight;

        Time timeSum = std::accumulate(fixingTimes.begin(),
                                       fixingTimes.end(), Real(0.0));

        Time residualTime = voldc.yearFraction(volReferenceDate,
                                               arguments_.exercise->lastDate());


        Real underlying = process_->stateVariable()->value();
        QL_REQUIRE(underlying > 0.0, "positive underlying value required");

        Volatility volatility = process_->blackVolatility()->blackVol(
                                                arguments_.exercise->lastDate(),
                                                underlying);

        Date exDate = arguments_.exercise->lastDate();
        DiscountFactor dividendDiscount = process_->dividendYield()->discount(exDate);
        DiscountFactor riskFreeDiscount = process_->riskFreeRate()->discount(exDate);

        Real temp = 0.0;
        for (Size i=pastFixings+1; i<numberOfFixings; i++)
            temp += fixingTimes[i-pastFixings-1]*(N-i);
        Real variance = volatility*volatility /N/N * (timeSum + 2.0*temp);
        // Variance of log(S_T/G): between the i-th and (i+1)-th fixings the Brownian
        // increment has weight i/N, after the last one weight 1. Summing these nonnegative
        // terms avoids cancellation in Var[ln S_T] + Var[ln G] - 2 Cov[ln S_T, ln G].
        Real sigmaSum_2 = residualTime - fixingTimes.back();
        for (Size i=1; i<remainingFixings; i++) {
            Real weight = static_cast<Real>(i)/N;
            sigmaSum_2 += weight*weight*(fixingTimes[i]-fixingTimes[i-1]);
        }
        sigmaSum_2 *= volatility*volatility;

        Size M = (pastFixings == 0 ? 1 : pastFixings);
        Real runningLogAverage = runningLog/M;

        Real muG = pastWeight * runningLogAverage +
                   futureWeight * std::log(underlying) +
                   (driftSum - 0.5*volatility*volatility*timeSum)/N;

        Real forward = underlying*dividendDiscount/riskFreeDiscount;
        Real forwardAverage = std::exp(muG + variance/2.0);
        // No randomness left in S_T/G (e.g. every fixing at maturity, or zero volatility).
        if (sigmaSum_2 == 0.0) {
            PlainVanillaPayoff limitPayoff(payoff->optionType(), forwardAverage);
            results_.value = riskFreeDiscount*limitPayoff(forward);
        } else {
            BlackCalculator black(payoff->optionType(), forwardAverage, forward,
                                  std::sqrt(sigmaSum_2), riskFreeDiscount);
            results_.value = black.value();
        }
    }

}

