/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
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

#include <ql/exercise.hpp>
#include <ql/pricingengines/asian/analytic_cont_geom_av_price.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <utility>

namespace QuantLib {

    AnalyticContinuousGeometricAveragePriceAsianEngine::
        AnalyticContinuousGeometricAveragePriceAsianEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticContinuousGeometricAveragePriceAsianEngine::calculate()
                                                                       const {
        QL_REQUIRE(arguments_.averageType == Average::Geometric,
                   "not a geometric average option");
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        Date exercise = arguments_.exercise->lastDate();

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Volatility volatility =
            process_->blackVolatility()->blackVol(exercise, payoff->strike());
        Real variance =
            process_->blackVolatility()->blackVariance(exercise,
                                                       payoff->strike());
        DiscountFactor riskFreeDiscount =
            process_->riskFreeRate()->discount(exercise);

        DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
        DayCounter divdc = process_->dividendYield()->dayCounter();
        DayCounter voldc = process_->blackVolatility()->dayCounter();

        Spread dividendYield = 0.5 * (
            process_->riskFreeRate()->zeroRate(exercise, rfdc,
                                               Continuous, NoFrequency) +
            process_->dividendYield()->zeroRate(exercise, divdc,
                                                Continuous, NoFrequency) +
            volatility*volatility/6.0);

        Time t_q = divdc.yearFraction(
            process_->dividendYield()->referenceDate(), exercise);
        DiscountFactor dividendDiscount = std::exp(-dividendYield*t_q);

        Real spot = process_->stateVariable()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying");
        Real forward = spot * dividendDiscount / riskFreeDiscount;

        BlackCalculator black(payoff, forward, std::sqrt(variance/3.0),
                              riskFreeDiscount);

        results_.value = black.value();
        results_.delta = black.delta(spot);
        results_.gamma = black.gamma(spot);

        results_.dividendRho = black.dividendRho(t_q)/2.0;

        Time t_r = rfdc.yearFraction(process_->riskFreeRate()->referenceDate(),
                                     arguments_.exercise->lastDate());
        results_.rho = black.rho(t_r) + 0.5 * black.dividendRho(t_q);

        Time t_v = voldc.yearFraction(
            process_->blackVolatility()->referenceDate(),
            arguments_.exercise->lastDate());
        results_.vega = black.vega(t_v)/std::sqrt(3.0) +
                        black.dividendRho(t_q)*volatility/6.0;
        try {
            results_.theta = black.theta(spot, t_v);
        } catch (Error&) {
            results_.theta = Null<Real>();
        }
    }

}

