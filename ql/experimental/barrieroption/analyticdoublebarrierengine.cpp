/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

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
#include <ql/experimental/barrieroption/analyticdoublebarrierengine.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <utility>

namespace QuantLib {

    AnalyticDoubleBarrierEngine::AnalyticDoubleBarrierEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process, int series)
    : process_(std::move(process)), series_(series) {
        registerWith(process_);
    }

    void AnalyticDoubleBarrierEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "this engine handles only european options");

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Real strike = payoff->strike();
        QL_REQUIRE(strike>0.0,
                   "strike must be positive");

        Real spot = underlying();
        QL_REQUIRE(spot >= 0.0, "negative or null underlying given");
        QL_REQUIRE(!triggered(spot), "barrier(s) already touched");

        DoubleBarrier::Type barrierType = arguments_.barrierType;

        if (triggered(spot)) {
           if (barrierType == DoubleBarrier::KnockIn)
               results_.value = vanillaEquivalent();  // knocked in
           else
               results_.value = 0.0;  // knocked out
        } else {
           switch (payoff->optionType()) {
             case Option::Call:
               switch (barrierType) {
                 case DoubleBarrier::KnockIn:
                   results_.value = callKI();
                   break;
                 case DoubleBarrier::KnockOut:
                   results_.value = callKO();
                   break;
                 case DoubleBarrier::KIKO:
                 case DoubleBarrier::KOKI:
                   QL_FAIL("unsupported double-barrier type: "
                           << barrierType);
                 default:
                   QL_FAIL("unknown double-barrier type: "
                           << barrierType);
               }
               break;
             case Option::Put:
               switch (barrierType) {
                 case DoubleBarrier::KnockIn:
                   results_.value = putKI();
                   break;
                 case DoubleBarrier::KnockOut:
                   results_.value = putKO();
                   break;
                 case DoubleBarrier::KIKO:
                 case DoubleBarrier::KOKI:
                   QL_FAIL("unsupported double-barrier type: "
                           << barrierType);
                 default:
                   QL_FAIL("unknown double-barrier type: "
                           << barrierType);
               }
               break;
             default:
               QL_FAIL("unknown type");
           }
        }
    }


    Real AnalyticDoubleBarrierEngine::underlying() const {
        return process_->x0();
    }

    Real AnalyticDoubleBarrierEngine::strike() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        return payoff->strike();
    }

    Time AnalyticDoubleBarrierEngine::residualTime() const {
        return process_->time(arguments_.exercise->lastDate());
    }

    Volatility AnalyticDoubleBarrierEngine::volatility() const {
        return process_->blackVolatility()->blackVol(residualTime(), strike());
    }

    Real AnalyticDoubleBarrierEngine::volatilitySquared() const {
        return volatility() * volatility();
    }

    Real AnalyticDoubleBarrierEngine::stdDeviation() const {
        return volatility() * std::sqrt(residualTime());
    }

    Real AnalyticDoubleBarrierEngine::barrierLo() const {
        return arguments_.barrier_lo;
    }

    Real AnalyticDoubleBarrierEngine::barrierHi() const {
        return arguments_.barrier_hi;
    }

    Rate AnalyticDoubleBarrierEngine::riskFreeRate() const {
        return process_->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                  NoFrequency);
    }

    DiscountFactor AnalyticDoubleBarrierEngine::riskFreeDiscount() const {
        return process_->riskFreeRate()->discount(residualTime());
    }

    Rate AnalyticDoubleBarrierEngine::dividendYield() const {
        return process_->dividendYield()->zeroRate(residualTime(),
                                                   Continuous, NoFrequency);
    }

    DiscountFactor AnalyticDoubleBarrierEngine::dividendDiscount() const {
        return process_->dividendYield()->discount(residualTime());
    }

    Rate AnalyticDoubleBarrierEngine::costOfCarry() const {
        return riskFreeRate() - dividendYield();
    }

    Real AnalyticDoubleBarrierEngine::vanillaEquivalent() const {
        // Call KI equates to vanilla - callKO
        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        Real forwardPrice = underlying() * dividendDiscount() / riskFreeDiscount();
        BlackCalculator black(payoff, forwardPrice, stdDeviation(), riskFreeDiscount());
        Real vanilla = black.value();
        if (vanilla < 0.0)
           vanilla = 0.0;
        return vanilla;
    }

    Real AnalyticDoubleBarrierEngine::callKO() const {
       // N.B. for flat barriers mu3=mu1 and mu2=0
       Real mu1 = 2 * costOfCarry() / volatilitySquared() + 1;
       Real bsigma = (costOfCarry() + volatilitySquared() / 2.0) * residualTime() / stdDeviation();

       Real acc1 = 0;
       Real acc2 = 0;
       for (int n = -series_ ; n <= series_ ; ++n) {
          Real L2n = std::pow(barrierLo(), 2 * n);
          Real U2n = std::pow(barrierHi(), 2 * n);
          Real d1 = std::log( underlying()* U2n / (strike() * L2n) ) / stdDeviation() + bsigma;
          Real d2 = std::log( underlying()* U2n / (barrierHi() * L2n) ) / stdDeviation() + bsigma;
          Real d3 = std::log( std::pow(barrierLo(), 2 * n + 2) / (strike() * underlying() * U2n) ) / stdDeviation() + bsigma;
          Real d4 = std::log( std::pow(barrierLo(), 2 * n + 2) / (barrierHi() * underlying() * U2n) ) / stdDeviation() + bsigma;

          acc1 += std::pow( std::pow(barrierHi(), n) / std::pow(barrierLo(), n), mu1 ) * 
                  (f_(d1) - f_(d2)) -
                  std::pow( std::pow(barrierLo(), n+1) / (std::pow(barrierHi(), n) * underlying()), mu1 ) * 
                  (f_(d3) - f_(d4));

          acc2 += std::pow( std::pow(barrierHi(), n) / std::pow(barrierLo(), n), mu1-2) * 
                  (f_(d1 - stdDeviation()) - f_(d2 - stdDeviation())) -
                  std::pow( std::pow(barrierLo(), n+1) / (std::pow(barrierHi(), n) * underlying()), mu1-2 ) * 
                  (f_(d3-stdDeviation()) - f_(d4-stdDeviation()));
       }

       Real rend = std::exp(-dividendYield() * residualTime());
       Real kov = underlying() * rend * acc1 - strike() * riskFreeDiscount() * acc2;
       return std::max(0.0, kov);
    }
    
    Real AnalyticDoubleBarrierEngine::callKI() const {
        // Call KI equates to vanilla - callKO
        return std::max(0.0, vanillaEquivalent() - callKO());
    }

    Real AnalyticDoubleBarrierEngine::putKO() const {
       Real mu1 = 2 * costOfCarry() / volatilitySquared() + 1;
       Real bsigma = (costOfCarry() + volatilitySquared() / 2.0) * residualTime() / stdDeviation();

       Real acc1 = 0;
       Real acc2 = 0;
       for (int n = -series_ ; n <= series_ ; ++n) {
          Real L2n = std::pow(barrierLo(), 2 * n);
          Real U2n = std::pow(barrierHi(), 2 * n);
          Real y1 = std::log( underlying()* U2n / (std::pow(barrierLo(), 2 * n + 1)) ) / stdDeviation() + bsigma;
          Real y2 = std::log( underlying()* U2n / (strike() * L2n) ) / stdDeviation() + bsigma;
          Real y3 = std::log( std::pow(barrierLo(), 2 * n + 2) / (barrierLo() * underlying() * U2n) ) / stdDeviation() + bsigma;
          Real y4 = std::log( std::pow(barrierLo(), 2 * n + 2) / (strike() * underlying() * U2n) ) / stdDeviation() + bsigma;

          acc1 += std::pow( std::pow(barrierHi(), n) / std::pow(barrierLo(), n), mu1-2) * 
                  (f_(y1 - stdDeviation()) - f_(y2 - stdDeviation())) -
                  std::pow( std::pow(barrierLo(), n+1) / (std::pow(barrierHi(), n) * underlying()), mu1-2 ) * 
                  (f_(y3-stdDeviation()) - f_(y4-stdDeviation()));

          acc2 += std::pow( std::pow(barrierHi(), n) / std::pow(barrierLo(), n), mu1 ) * 
                  (f_(y1) - f_(y2)) -
                  std::pow( std::pow(barrierLo(), n+1) / (std::pow(barrierHi(), n) * underlying()), mu1 ) * 
                  (f_(y3) - f_(y4));

       }

       Real rend = std::exp(-dividendYield() * residualTime());
       Real kov = strike() * riskFreeDiscount() * acc1 - underlying() * rend  * acc2;
       return std::max(0.0, kov);
    }
    
    Real AnalyticDoubleBarrierEngine::putKI() const {
        // Put KI equates to vanilla - putKO
        return std::max(0.0, vanillaEquivalent() - putKO());
    }

    
}

