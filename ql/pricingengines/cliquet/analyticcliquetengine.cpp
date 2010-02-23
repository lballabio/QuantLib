/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2007 StatPro Italia srl

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

#include <ql/pricingengines/cliquet/analyticcliquetengine.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    AnalyticCliquetEngine::AnalyticCliquetEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process) {
        registerWith(process_);
    }

    void AnalyticCliquetEngine::calculate() const {

        QL_REQUIRE(arguments_.accruedCoupon == Null<Real>() &&
                   arguments_.lastFixing == Null<Real>(),
                   "this engine cannot price options already started");
        QL_REQUIRE(arguments_.localCap == Null<Real>() &&
                   arguments_.localFloor == Null<Real>() &&
                   arguments_.globalCap == Null<Real>() &&
                   arguments_.globalFloor == Null<Real>(),
                   "this engine cannot price capped/floored options");

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        boost::shared_ptr<PercentageStrikePayoff> moneyness =
            boost::dynamic_pointer_cast<PercentageStrikePayoff>(
                                                           arguments_.payoff);
        QL_REQUIRE(moneyness, "wrong payoff given");

        std::vector<Date> resetDates = arguments_.resetDates;
        resetDates.push_back(arguments_.exercise->lastDate());

        Real underlying = process_->stateVariable()->value();
        QL_REQUIRE(underlying > 0.0, "negative or null underlying");
        Real strike = underlying * moneyness->strike();
        boost::shared_ptr<StrikedTypePayoff> payoff(
                      new PlainVanillaPayoff(moneyness->optionType(),strike));

        results_.value = 0.0;
        results_.delta = results_.gamma = 0.0;
        results_.theta = 0.0;
        results_.rho = results_.dividendRho = 0.0;
        results_.vega = 0.0;

        for (Size i = 1; i < resetDates.size(); i++) {

            Real weight =
                process_->dividendYield()->discount(resetDates[i-1]);
            DiscountFactor discount =
                process_->riskFreeRate()->discount(resetDates[i]) /
                process_->riskFreeRate()->discount(resetDates[i-1]);
            DiscountFactor qDiscount =
                process_->dividendYield()->discount(resetDates[i]) /
                process_->dividendYield()->discount(resetDates[i-1]);
            Real forward = underlying*qDiscount/discount;
            Real variance =
                process_->blackVolatility()->blackForwardVariance(
                                        resetDates[i-1],resetDates[i],strike);

            BlackCalculator black(payoff, forward, std::sqrt(variance), discount);

            DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
            DayCounter divdc = process_->dividendYield()->dayCounter();
            DayCounter voldc = process_->blackVolatility()->dayCounter();

            results_.value += weight * black.value();
            results_.delta += weight * (black.delta(underlying) +
                                        moneyness->strike() * discount *
                                        black.beta());
            results_.gamma += 0.0;
            results_.theta += process_->dividendYield()->forwardRate(
                resetDates[i-1], resetDates[i], rfdc, Continuous, NoFrequency) *
                weight * black.value();

            Time dt = rfdc.yearFraction(resetDates[i-1],resetDates[i]);
            results_.rho += weight * black.rho(dt);

            Time t = divdc.yearFraction(
                                    process_->dividendYield()->referenceDate(),
                                    resetDates[i-1]);
            dt = divdc.yearFraction(resetDates[i-1],resetDates[i]);
            results_.dividendRho += weight * (black.dividendRho(dt) -
                                              t * black.value());

            dt = voldc.yearFraction(resetDates[i-1], resetDates[i]);
            results_.vega += weight * black.vega(dt);
        }

    }

}

