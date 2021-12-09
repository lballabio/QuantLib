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

#include <ql/exercise.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/analyticdividendeuropeanengine.hpp>
#include <utility>

namespace QuantLib {

    AnalyticDividendEuropeanEngine::AnalyticDividendEuropeanEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticDividendEuropeanEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Date settlementDate = process_->riskFreeRate()->referenceDate();
        Real riskless = 0.0;
        Size i;
        for (i=0; i<arguments_.cashFlow.size(); i++) {
            const Date cashFlowDate = arguments_.cashFlow[i]->date();

            if (   cashFlowDate >= settlementDate
                && cashFlowDate <= arguments_.exercise->lastDate()) {

                riskless += arguments_.cashFlow[i]->amount() *
                    process_->riskFreeRate()->discount(cashFlowDate) /
                    process_->dividendYield()->discount(cashFlowDate);
            }
        }

        Real spot = process_->stateVariable()->value() - riskless;
        QL_REQUIRE(spot > 0.0,
                   "negative or null underlying after subtracting dividends");

        DiscountFactor dividendDiscount =
            process_->dividendYield()->discount(
                                             arguments_.exercise->lastDate());
        DiscountFactor riskFreeDiscount =
            process_->riskFreeRate()->discount(arguments_.exercise->lastDate());
        Real forwardPrice = spot * dividendDiscount / riskFreeDiscount;

        Real variance =
            process_->blackVolatility()->blackVariance(
                                              arguments_.exercise->lastDate(),
                                              payoff->strike());

        BlackCalculator black(payoff, forwardPrice, std::sqrt(variance),
                              riskFreeDiscount);

        results_.value = black.value();
        results_.delta = black.delta(spot);
        results_.gamma = black.gamma(spot);

        DayCounter rfdc = process_->riskFreeRate()->dayCounter();
        DayCounter dydc = process_->dividendYield()->dayCounter();
        DayCounter voldc = process_->blackVolatility()->dayCounter();
        Time t = voldc.yearFraction(
                                 process_->blackVolatility()->referenceDate(),
                                 arguments_.exercise->lastDate());
        results_.vega = black.vega(t);

        Real delta_theta = 0.0, delta_rho = 0.0;
        for (i = 0; i < arguments_.cashFlow.size(); i++) {
            Date d = arguments_.cashFlow[i]->date();

            if (   d >= settlementDate
                && d <= arguments_.exercise->lastDate()) {

                delta_theta -= arguments_.cashFlow[i]->amount() *
                  (  process_->riskFreeRate()->zeroRate(d,rfdc,Continuous,Annual)
                   - process_->dividendYield()->zeroRate(d,dydc,Continuous,Annual)) *
                  process_->riskFreeRate()->discount(d) /
                  process_->dividendYield()->discount(d);

                Time t = process_->time(d);
                delta_rho += arguments_.cashFlow[i]->amount() * t *
                             process_->riskFreeRate()->discount(t) /
                             process_->dividendYield()->discount(t);
            }
        }
        t = process_->time(arguments_.exercise->lastDate());
        try {
            results_.theta = black.theta(spot, t) +
                             delta_theta * black.delta(spot);
        } catch (Error&) {
            results_.theta = Null<Real>();
        }

        results_.rho = black.rho(t) +
                       delta_rho * black.delta(spot);
    }

}

