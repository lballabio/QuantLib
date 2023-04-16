/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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
#include <ql/experimental/exoticoptions/kirkspreadoptionengine.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <utility>

using namespace std;

namespace QuantLib {

    KirkSpreadOptionEngine::KirkSpreadOptionEngine(std::shared_ptr<BlackProcess> process1,
                                                   std::shared_ptr<BlackProcess> process2,
                                                   Handle<Quote> correlation)
    : process1_(std::move(process1)), process2_(std::move(process2)), rho_(std::move(correlation)) {
        registerWith(process1_);
        registerWith(process2_);
        registerWith(rho_);
    }

    void KirkSpreadOptionEngine::calculate() const {

        // First: tests on types
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        std::shared_ptr<PlainVanillaPayoff> payoff =
            std::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "not a plain-vanilla payoff");

        // forward values - futures, so b=0
        Real forward1 = process1_->stateVariable()->value();
        Real forward2 = process2_->stateVariable()->value();

        Date exerciseDate = arguments_.exercise->lastDate();

        // Volatilities
        Real sigma1 =
            process1_->blackVolatility()->blackVol(exerciseDate,forward1);
        Real sigma2 =
            process2_->blackVolatility()->blackVol(exerciseDate,forward2);

        DiscountFactor riskFreeDiscount =
            process1_->riskFreeRate()->discount(exerciseDate);

        Real strike = payoff->strike();

        // Unique F (forward) value for pricing
        Real F = forward1/(forward2+strike);

        // Its volatility
        Real sigma =
            sqrt(pow(sigma1,2)
                 + pow((sigma2*(forward2/(forward2+strike))),2)
                 - 2*rho_->value()*sigma1*sigma2*(forward2/(forward2+strike)));

        // Day counter and Dates handling variables
        DayCounter rfdc = process1_->riskFreeRate()->dayCounter();
        Time t = rfdc.yearFraction(process1_->riskFreeRate()->referenceDate(),
                                   arguments_.exercise->lastDate());

        // Black-Scholes solution values
        Real d1 = (log(F)+ 0.5*pow(sigma,2)*t) / (sigma*sqrt(t));
        Real d2 = d1 - sigma*sqrt(t);

        NormalDistribution pdf;
        CumulativeNormalDistribution cum;
        Real Nd1 = cum(d1);
        Real Nd2 = cum(d2);
        Real NMd1 = cum(-d1);
        Real NMd2 = cum(-d2);

        Option::Type optionType = payoff->optionType();

        if (optionType==Option::Call) {
            results_.value = riskFreeDiscount*(F*Nd1-Nd2)*(forward2+strike);
        } else {
            results_.value = riskFreeDiscount*(NMd2 -F*NMd1)*(forward2+strike);
        }

        Real callValue = optionType == Option::Call ? results_.value : riskFreeDiscount*(F*Nd1-Nd2)*(forward2+strike);
        results_.theta = -((log(riskFreeDiscount)/t)*callValue
                           + riskFreeDiscount*(forward1*sigma)/(2*sqrt(t))*pdf(d1));
    }

}

