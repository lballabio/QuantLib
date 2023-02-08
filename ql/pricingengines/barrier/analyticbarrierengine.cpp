/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003 Neil Firth
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
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <utility>

namespace QuantLib {

    AnalyticBarrierEngine::AnalyticBarrierEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticBarrierEngine::calculate() const {

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        QL_REQUIRE(payoff->strike()>0.0,
                   "strike must be positive");

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "only european style option are supported");

        Real strike = payoff->strike();
        Real spot = process_->x0();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");
        QL_REQUIRE(!triggered(spot), "barrier touched");

        Barrier::Type barrierType = arguments_.barrierType;

        switch (payoff->optionType()) {
          case Option::Call:
            switch (barrierType) {
              case Barrier::DownIn:
                if (strike >= barrier())
                    results_.value = C(1,1) + E(1);
                else
                    results_.value = A(1) - B(1) + D(1,1) + E(1);
                break;
              case Barrier::UpIn:
                if (strike >= barrier())
                    results_.value = A(1) + E(-1);
                else
                    results_.value = B(1) - C(-1,1) + D(-1,1) + E(-1);
                break;
              case Barrier::DownOut:
                if (strike >= barrier())
                    results_.value = A(1) - C(1,1) + F(1);
                else
                    results_.value = B(1) - D(1,1) + F(1);
                break;
              case Barrier::UpOut:
                if (strike >= barrier())
                    results_.value = F(-1);
                else
                    results_.value = A(1) - B(1) + C(-1,1) - D(-1,1) + F(-1);
                break;
            }
            break;
          case Option::Put:
            switch (barrierType) {
              case Barrier::DownIn:
                if (strike >= barrier())
                    results_.value = B(-1) - C(1,-1) + D(1,-1) + E(1);
                else
                    results_.value = A(-1) + E(1);
                break;
              case Barrier::UpIn:
                if (strike >= barrier())
                    results_.value = A(-1) - B(-1) + D(-1,-1) + E(-1);
                else
                    results_.value = C(-1,-1) + E(-1);
                break;
              case Barrier::DownOut:
                if (strike >= barrier())
                    results_.value = A(-1) - B(-1) + C(1,-1) - D(1,-1) + F(1);
                else
                    results_.value = F(1);
                break;
              case Barrier::UpOut:
                if (strike >= barrier())
                    results_.value = B(-1) - D(-1,-1) + F(-1);
                else
                    results_.value = A(-1) - C(-1,-1) + F(-1);
                break;
            }
            break;
          default:
            QL_FAIL("unknown type");
        }
    }


    Real AnalyticBarrierEngine::underlying() const {
        return process_->x0();
    }

    Real AnalyticBarrierEngine::strike() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");
        return payoff->strike();
    }

    Volatility AnalyticBarrierEngine::volatility() const {
        return process_->blackVolatility()->blackVol(
                    arguments_.exercise->lastDate(),
                    strike());
    }

    Real AnalyticBarrierEngine::stdDeviation() const {
        return std::sqrt(process_->blackVolatility()->blackVariance(
                        arguments_.exercise->lastDate(),
                        strike()));
    }

    Real AnalyticBarrierEngine::barrier() const {
        return arguments_.barrier;
    }

    Real AnalyticBarrierEngine::rebate() const {
        return arguments_.rebate;
    }

    Rate AnalyticBarrierEngine::riskFreeRate() const {
        return process_->riskFreeRate()->zeroRate(
                    arguments_.exercise->lastDate(),
                    process_->riskFreeRate()->dayCounter(),
                    Continuous, NoFrequency);
    }

    DiscountFactor AnalyticBarrierEngine::riskFreeDiscount() const {
        return process_->riskFreeRate()->discount(
                    arguments_.exercise->lastDate());
    }

    Rate AnalyticBarrierEngine::dividendYield() const {
        return process_->dividendYield()->zeroRate(
                    arguments_.exercise->lastDate(),
                    process_->dividendYield()->dayCounter(),
                    Continuous, NoFrequency);
    }

    DiscountFactor AnalyticBarrierEngine::dividendDiscount() const {
        return process_->dividendYield()->discount(
                    arguments_.exercise->lastDate());
    }

    Rate AnalyticBarrierEngine::mu() const {
        Volatility vol = volatility();
        return (riskFreeRate() - dividendYield())/(vol * vol) - 0.5;
    }

    Real AnalyticBarrierEngine::muSigma() const {
        return (1 + mu()) * stdDeviation();
    }

    Real AnalyticBarrierEngine::A(Real phi) const {
        Real x1 =
            std::log(underlying()/strike())/stdDeviation() + muSigma();
        Real N1 = f_(phi*x1);
        Real N2 = f_(phi*(x1-stdDeviation()));

        return phi*(underlying() * dividendDiscount() * N1
                      - strike() * riskFreeDiscount() * N2);
    }

    Real AnalyticBarrierEngine::B(Real phi) const {
        Real x2 =
            std::log(underlying()/barrier())/stdDeviation() + muSigma();
        Real N1 = f_(phi*x2);
        Real N2 = f_(phi*(x2-stdDeviation()));
        return phi*(underlying() * dividendDiscount() * N1
                      - strike() * riskFreeDiscount() * N2);
    }

    Real AnalyticBarrierEngine::C(Real eta, Real phi) const {
        Real HS = barrier()/underlying();
        Real powHS0 = std::pow(HS, 2 * mu());
        Real powHS1 = powHS0 * HS * HS;
        Real y1 = std::log(barrier()*HS/strike())/stdDeviation() + muSigma();
        Real N1 = f_(eta*y1);
        Real N2 = f_(eta*(y1-stdDeviation()));
        // when N1 or N2 are zero, the corresponding powHS might
        // be infinity, resulting in a NaN for their products.  The limit should be 0.
        return phi*(underlying() * dividendDiscount() * (N1 == 0.0 ? 0.0 : powHS1 * N1)
                      - strike() * riskFreeDiscount() * (N2 == 0.0 ? 0.0 : powHS0 * N2));
    }

    Real AnalyticBarrierEngine::D(Real eta, Real phi) const {
        Real HS = barrier()/underlying();
        Real powHS0 = std::pow(HS, 2 * mu());
        Real powHS1 = powHS0 * HS * HS;
        Real y2 = std::log(barrier()/underlying())/stdDeviation() + muSigma();
        Real N1 = f_(eta*y2);
        Real N2 = f_(eta*(y2-stdDeviation()));
        // when N1 or N2 are zero, the corresponding powHS might
        // be infinity, resulting in a NaN for their products.  The limit should be 0.
        return phi*(underlying() * dividendDiscount() * (N1 == 0.0 ? 0.0 : powHS1 * N1)
                      - strike() * riskFreeDiscount() * (N2 == 0.0 ? 0.0 : powHS0 * N2));
    }

    Real AnalyticBarrierEngine::E(Real eta) const {
        if (rebate() > 0) {
            Real powHS0 = std::pow(barrier()/underlying(), 2 * mu());
            Real x2 =
                std::log(underlying()/barrier())/stdDeviation() + muSigma();
            Real y2 =
                std::log(barrier()/underlying())/stdDeviation() + muSigma();
            Real N1 = f_(eta*(x2 - stdDeviation()));
            Real N2 = f_(eta*(y2 - stdDeviation()));
            // when N2 is zero, powHS0 might be infinity, resulting in
            // a NaN for their product.  The limit should be 0.
            return rebate() * riskFreeDiscount() * (N1 - (N2 == 0.0 ? 0.0 : powHS0 * N2));
        } else {
            return 0.0;
        }
    }

    Real AnalyticBarrierEngine::F(Real eta) const {
        if (rebate() > 0) {
            Rate m = mu();
            Volatility vol = volatility();
            Real lambda = std::sqrt(m*m + 2.0*riskFreeRate()/(vol * vol));
            Real HS = barrier()/underlying();
            Real powHSplus = std::pow(HS, m + lambda);
            Real powHSminus = std::pow(HS, m - lambda);

            Real sigmaSqrtT = stdDeviation();
            Real z = std::log(barrier()/underlying())/sigmaSqrtT
                + lambda * sigmaSqrtT;

            Real N1 = f_(eta * z);
            Real N2 = f_(eta * (z - 2.0 * lambda * sigmaSqrtT));
            // when N1 or N2 are zero, the corresponding powHS might
            // be infinity, resulting in a NaN for their product.  The limit should be 0.
            return rebate() * ((N1 == 0.0 ? 0.0 : powHSplus * N1) + (N2 == 0.0 ? 0.0 : powHSminus * N2));
        } else {
            return 0.0;
        }
    }

}
