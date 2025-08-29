/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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
#include <ql/pricingengines/exotic/analyticeuropeanmargrabeengine.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <utility>

namespace QuantLib {

    AnalyticEuropeanMargrabeEngine::AnalyticEuropeanMargrabeEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
        ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
        Real correlation)
    : process1_(std::move(process1)), process2_(std::move(process2)), rho_(correlation) {
        registerWith(process1_);
        registerWith(process2_);
    }

    void AnalyticEuropeanMargrabeEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not an European Option");

        ext::shared_ptr<NullPayoff> payoff =
            ext::dynamic_pointer_cast<NullPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non a Null Payoff type");

        Integer quantity1 = arguments_.Q1;
        Integer quantity2 = arguments_.Q2;

        Real s1  = process1_->stateVariable()->value();
        Real s2  = process2_->stateVariable()->value();

        Real variance1 = process1_->blackVolatility()->blackVariance(
                                                exercise->lastDate(), s1);
        Real variance2 = process2_->blackVolatility()->blackVariance(
                                                exercise->lastDate(), s2);

        DiscountFactor riskFreeDiscount =
            process1_->riskFreeRate()->discount(exercise->lastDate());

        DiscountFactor dividendDiscount1 =
            process1_->dividendYield()->discount(exercise->lastDate());
        DiscountFactor dividendDiscount2 =
            process2_->dividendYield()->discount(exercise->lastDate());

        Real forward1 = process1_->stateVariable()->value() *
            dividendDiscount1 / riskFreeDiscount;
        Real forward2 = process2_->stateVariable()->value() *
            dividendDiscount2 / riskFreeDiscount;

        Real stdDev1 = std::sqrt(variance1);
        Real stdDev2 = std::sqrt(variance2);
        Real variance = variance1 + variance2 - 2*rho_*stdDev1*stdDev2;
        Real stdDev = std::sqrt(variance);
        Real d1 = (std::log((quantity1*forward1)/(quantity2*forward2))
                   + 0.5*variance) / stdDev;
        Real d2 = d1 - stdDev;
        Real Nd1, Nd2, nd1, nd2;
        CumulativeNormalDistribution cum;
        NormalDistribution norm;
        Nd1 = cum(d1);
        Nd2 = cum(d2);
        nd1 = norm(d1);
        nd2 = norm(d2);
        DayCounter rfdc  = process1_->riskFreeRate()->dayCounter();
        Time t = rfdc.yearFraction(process1_->riskFreeRate()->referenceDate(),
                                  arguments_.exercise->lastDate());
        Real sqt = std::sqrt(t);
        Real q1  = -std::log(dividendDiscount1)/(sqt*sqt);
        Real q2  = -std::log(dividendDiscount2)/(sqt*sqt);

        results_.value =
            riskFreeDiscount * (quantity1*forward1*Nd1 - quantity2*forward2*Nd2);

        // Greeks
        results_.delta1 = riskFreeDiscount*(quantity1*forward1*Nd1)/s1;
        results_.delta2 = -riskFreeDiscount*(quantity2*forward2*Nd2)/s2;
        results_.gamma1 = (riskFreeDiscount*(quantity1*forward1*nd1)/s1)/(quantity1*s1*stdDev);
        results_.gamma2 = (-riskFreeDiscount*(quantity2*forward2*nd2)/s2)/(-quantity2*s2*stdDev);
        Real vega       = riskFreeDiscount*(quantity1*forward1*nd1)*sqt;
        results_.theta  = -((stdDev*vega/sqt)/(2*t)-(q1*quantity1*s1*results_.delta1)-(q2*quantity2*s2*results_.delta2));
        results_.rho    = 0.0;
    }

}
