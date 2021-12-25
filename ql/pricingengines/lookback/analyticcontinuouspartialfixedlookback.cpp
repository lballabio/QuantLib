/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou
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
#include <ql/pricingengines/lookback/analyticcontinuouspartialfixedlookback.hpp>
#include <utility>

namespace QuantLib {

    AnalyticContinuousPartialFixedLookbackEngine::AnalyticContinuousPartialFixedLookbackEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticContinuousPartialFixedLookbackEngine::calculate() const {

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "Non-plain payoff given");

        QL_REQUIRE(process_->x0() > 0.0, "negative or null underlying");

        switch (payoff->optionType()) {
          case Option::Call:
            QL_REQUIRE(payoff->strike()>=0.0,
                       "Strike must be positive or null");
            results_.value = A(1);
            break;
          case Option::Put:
            QL_REQUIRE(payoff->strike()>0.0,
                       "Strike must be positive");
            results_.value = A(-1);
            break;
          default:
            QL_FAIL("Unknown type");
        }
    }


    Real AnalyticContinuousPartialFixedLookbackEngine::underlying() const {
        return process_->x0();
    }

    Real AnalyticContinuousPartialFixedLookbackEngine::strike() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "Non-plain payoff given");
        return payoff->strike();
    }

    Time AnalyticContinuousPartialFixedLookbackEngine::residualTime() const {
        return process_->time(arguments_.exercise->lastDate());
    }

    Volatility AnalyticContinuousPartialFixedLookbackEngine::volatility() const {
        return process_->blackVolatility()->blackVol(residualTime(), strike());
    }

    Real AnalyticContinuousPartialFixedLookbackEngine::stdDeviation() const {
        return volatility() * std::sqrt(residualTime());
    }

    Rate AnalyticContinuousPartialFixedLookbackEngine::riskFreeRate() const {
        return process_->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                  NoFrequency);
    }

    DiscountFactor AnalyticContinuousPartialFixedLookbackEngine::riskFreeDiscount()
                              const {
        return process_->riskFreeRate()->discount(residualTime());
    }

    Rate AnalyticContinuousPartialFixedLookbackEngine::dividendYield() const {
        return process_->dividendYield()->zeroRate(residualTime(),
                                                   Continuous, NoFrequency);
    }

    DiscountFactor AnalyticContinuousPartialFixedLookbackEngine::dividendDiscount()
                              const {
        return process_->dividendYield()->discount(residualTime());
    }

    Time AnalyticContinuousPartialFixedLookbackEngine::lookbackPeriodStartTime() const {
        return process_->time(arguments_.lookbackPeriodStart);
    }

    Real AnalyticContinuousPartialFixedLookbackEngine::A(Real eta) const {
        bool differentStartOfLookback = lookbackPeriodStartTime() != residualTime();
        Real carry = riskFreeRate() - dividendYield();

        Volatility vol = volatility();
        Real x = 2.0*carry/(vol*vol);
        Real s = underlying()/strike();
        Real ls = std::log(s);
        Real d1 = ls/stdDeviation() + 0.5*(x+1.0)*stdDeviation();
        Real d2 = d1 - stdDeviation();

        Real e1 = 0, e2 = 0;
        if (differentStartOfLookback)
        {
            e1 = (carry + vol * vol / 2) * (residualTime() - lookbackPeriodStartTime()) / (vol * std::sqrt(residualTime() - lookbackPeriodStartTime()));
            e2 = e1 - vol * std::sqrt(residualTime() - lookbackPeriodStartTime());
        }

        Real f1 = (ls + (carry + vol * vol / 2) * lookbackPeriodStartTime()) / (vol * std::sqrt(lookbackPeriodStartTime()));
        Real f2 = f1 - vol * std::sqrt(lookbackPeriodStartTime());

        Real n1 = f_(eta*d1);
        Real n2 = f_(eta*d2);

        BivariateCumulativeNormalDistributionWe04DP cnbn1(-1), cnbn2(0), cnbn3(0);
        if (differentStartOfLookback) {
            cnbn1 = BivariateCumulativeNormalDistributionWe04DP (-std::sqrt(lookbackPeriodStartTime() / residualTime()));
            cnbn2 = BivariateCumulativeNormalDistributionWe04DP (std::sqrt(1 - lookbackPeriodStartTime() / residualTime()));
            cnbn3 = BivariateCumulativeNormalDistributionWe04DP (-std::sqrt(1 - lookbackPeriodStartTime() / residualTime()));
        }

        Real n3 = cnbn1(eta*(d1-x*stdDeviation()), eta*(-f1+2.0* carry * std::sqrt(lookbackPeriodStartTime()) / vol));
        Real n4 = cnbn2(eta*e1, eta*d1);
        Real n5 = cnbn3(-eta*e1, eta*d1);
        Real n6 = cnbn1(eta*f2, -eta*d2);
        Real n7 = f_(eta*f1);
        Real n8 = f_(-eta*e2);

        Real pow_s = std::pow(s, -x);
        Real carryDiscount = std::exp(-carry * (residualTime() - lookbackPeriodStartTime()));
        return eta*(underlying() * dividendDiscount() * n1 
                    - strike() * riskFreeDiscount() * n2
                    + underlying() * riskFreeDiscount() / x 
                    * (-pow_s * n3 + dividendDiscount() / riskFreeDiscount() * n4)
                    - underlying() * dividendDiscount() * n5 
                    - strike() * riskFreeDiscount() * n6 
                    + carryDiscount * dividendDiscount() 
                    * (1 - 0.5 * vol * vol / carry) * 
                    underlying() * n7 * n8);
    }
}

