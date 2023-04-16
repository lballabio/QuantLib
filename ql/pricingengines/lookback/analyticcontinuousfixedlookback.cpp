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
#include <ql/pricingengines/lookback/analyticcontinuousfixedlookback.hpp>
#include <utility>

namespace QuantLib {

    AnalyticContinuousFixedLookbackEngine::AnalyticContinuousFixedLookbackEngine(
        std::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticContinuousFixedLookbackEngine::calculate() const {

        std::shared_ptr<PlainVanillaPayoff> payoff =
            std::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "Non-plain payoff given");

        QL_REQUIRE(process_->x0() > 0.0, "negative or null underlying");

        Real strike = payoff->strike();

        switch (payoff->optionType()) {
          case Option::Call:
            QL_REQUIRE(payoff->strike()>=0.0,
                       "Strike must be positive or null");
            if (strike <= minmax())
                results_.value = A(1) + C(1);
            else
                results_.value = B(1);
            break;
          case Option::Put:
            QL_REQUIRE(payoff->strike()>0.0,
                       "Strike must be positive");
            if (strike >= minmax())
                results_.value = A(-1) + C(-1);
            else
                results_.value = B(-1);
            break;
          default:
            QL_FAIL("Unknown type");
        }
    }


    Real AnalyticContinuousFixedLookbackEngine::underlying() const {
        return process_->x0();
    }

    Real AnalyticContinuousFixedLookbackEngine::strike() const {
        std::shared_ptr<PlainVanillaPayoff> payoff =
            std::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "Non-plain payoff given");
        return payoff->strike();
    }

    Time AnalyticContinuousFixedLookbackEngine::residualTime() const {
        return process_->time(arguments_.exercise->lastDate());
    }

    Volatility AnalyticContinuousFixedLookbackEngine::volatility() const {
        return process_->blackVolatility()->blackVol(residualTime(), strike());
    }

    Real AnalyticContinuousFixedLookbackEngine::stdDeviation() const {
        return volatility() * std::sqrt(residualTime());
    }

    Rate AnalyticContinuousFixedLookbackEngine::riskFreeRate() const {
        return process_->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                  NoFrequency);
    }

    DiscountFactor AnalyticContinuousFixedLookbackEngine::riskFreeDiscount()
                              const {
        return process_->riskFreeRate()->discount(residualTime());
    }

    Rate AnalyticContinuousFixedLookbackEngine::dividendYield() const {
        return process_->dividendYield()->zeroRate(residualTime(),
                                                   Continuous, NoFrequency);
    }

    DiscountFactor AnalyticContinuousFixedLookbackEngine::dividendDiscount()
                              const {
        return process_->dividendYield()->discount(residualTime());
    }

    Real AnalyticContinuousFixedLookbackEngine::minmax() const {
        return arguments_.minmax;
    }

    Real AnalyticContinuousFixedLookbackEngine::A(Real eta) const {
        Volatility vol = volatility();
        Real lambda = 2.0*(riskFreeRate() - dividendYield())/(vol*vol);
        Real ss = underlying()/minmax();
        Real d1 =
            std::log(ss)/stdDeviation() + 0.5*(lambda+1.0)*stdDeviation();
        Real N1 = f_(eta*d1);
        Real N2 = f_(eta*(d1-stdDeviation()));
        Real N3 = f_(eta*(d1-lambda*stdDeviation()));
        Real N4 = f_(eta*d1);
        Real powss = std::pow(ss, -lambda);
        return eta*(underlying() * dividendDiscount() * N1 -
                    minmax() * riskFreeDiscount() * N2 -
                    underlying() * riskFreeDiscount() *
                    (powss * N3 - dividendDiscount()* N4/riskFreeDiscount())/
            lambda);
    }

    Real AnalyticContinuousFixedLookbackEngine::B(Real eta) const {
        Volatility vol = volatility();
        Real lambda = 2.0*(riskFreeRate() - dividendYield())/(vol*vol);
        Real ss = underlying()/strike();
        Real d1 =
            std::log(ss)/stdDeviation() + 0.5*(lambda+1.0)*stdDeviation();
        Real N1 = f_(eta*d1);
        Real N2 = f_(eta*(d1-stdDeviation()));
        Real N3 = f_(eta*(d1-lambda*stdDeviation()));
        Real N4 = f_(eta*d1);
        Real powss = std::pow(ss, -lambda);
        return eta*(underlying() * dividendDiscount() * N1 -
                    strike() * riskFreeDiscount() * N2 -
                    underlying() * riskFreeDiscount() *
                    (powss * N3 - dividendDiscount()* N4/riskFreeDiscount())/
            lambda);
    }

    Real AnalyticContinuousFixedLookbackEngine::C(Real eta) const {
        return eta*(riskFreeDiscount()*(minmax() - strike()));
    }

}

