/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2006 Ferdinando Ametrano
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
#include <ql/math/comparison.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <utility>

namespace QuantLib {

    BaroneAdesiWhaleyApproximationEngine::BaroneAdesiWhaleyApproximationEngine(
        std::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }


    // critical commodity price
    Real BaroneAdesiWhaleyApproximationEngine::criticalPrice(
        const std::shared_ptr<StrikedTypePayoff>& payoff,
        DiscountFactor riskFreeDiscount,
        DiscountFactor dividendDiscount,
        Real variance, Real tolerance) {

        // Calculation of seed value, Si
        Real n= 2.0*std::log(dividendDiscount/riskFreeDiscount)/(variance);
        Real m=-2.0*std::log(riskFreeDiscount)/(variance);
        Real bT = std::log(dividendDiscount/riskFreeDiscount);

        Real qu, Su, h, Si;
        switch (payoff->optionType()) {
          case Option::Call:
            qu = (-(n-1.0) + std::sqrt(((n-1.0)*(n-1.0)) + 4.0*m))/2.0;
            Su = payoff->strike() / (1.0 - 1.0/qu);
            h = -(bT + 2.0*std::sqrt(variance)) * payoff->strike() /
                (Su - payoff->strike());
            Si = payoff->strike() + (Su - payoff->strike()) *
                (1.0 - std::exp(h));
            break;
          case Option::Put:
            qu = (-(n-1.0) - std::sqrt(((n-1.0)*(n-1.0)) + 4.0*m))/2.0;
            Su = payoff->strike() / (1.0 - 1.0/qu);
            h = (bT - 2.0*std::sqrt(variance)) * payoff->strike() /
                (payoff->strike() - Su);
            Si = Su + (payoff->strike() - Su) * std::exp(h);
            break;
          default:
            QL_FAIL("unknown option type");
        }


        // Newton Raphson algorithm for finding critical price Si
        Real Q, LHS, RHS, bi;
        Real forwardSi = Si * dividendDiscount / riskFreeDiscount;
        Real d1 = (std::log(forwardSi/payoff->strike()) + 0.5*variance) /
            std::sqrt(variance);
        CumulativeNormalDistribution cumNormalDist;
        Real K = (!close(riskFreeDiscount, 1.0, 1000))
                ? Real(-2.0*std::log(riskFreeDiscount)
                   / (variance*(1.0-riskFreeDiscount)))
                 : 2.0/variance;
        Real temp = blackFormula(payoff->optionType(), payoff->strike(),
                forwardSi, std::sqrt(variance))*riskFreeDiscount;
        switch (payoff->optionType()) {
          case Option::Call:
            Q = (-(n-1.0) + std::sqrt(((n-1.0)*(n-1.0)) + 4 * K)) / 2;
            LHS = Si - payoff->strike();
            RHS = temp + (1 - dividendDiscount * cumNormalDist(d1)) * Si / Q;
            bi =  dividendDiscount * cumNormalDist(d1) * (1 - 1/Q) +
                (1 - dividendDiscount *
                 cumNormalDist.derivative(d1) / std::sqrt(variance)) / Q;
            while (std::fabs(LHS - RHS)/payoff->strike() > tolerance) {
                Si = (payoff->strike() + RHS - bi * Si) / (1 - bi);
                forwardSi = Si * dividendDiscount / riskFreeDiscount;
                d1 = (std::log(forwardSi/payoff->strike())+0.5*variance)
                    /std::sqrt(variance);
                LHS = Si - payoff->strike();
                Real temp2 = blackFormula(payoff->optionType(), payoff->strike(),
                    forwardSi, std::sqrt(variance))*riskFreeDiscount;
                RHS = temp2 + (1 - dividendDiscount * cumNormalDist(d1)) * Si / Q;
                bi = dividendDiscount * cumNormalDist(d1) * (1 - 1 / Q)
                    + (1 - dividendDiscount *
                       cumNormalDist.derivative(d1) / std::sqrt(variance))
                    / Q;
            }
            break;
          case Option::Put:
            Q = (-(n-1.0) - std::sqrt(((n-1.0)*(n-1.0)) + 4 * K)) / 2;
            LHS = payoff->strike() - Si;
            RHS = temp - (1 - dividendDiscount * cumNormalDist(-d1)) * Si / Q;
            bi = -dividendDiscount * cumNormalDist(-d1) * (1 - 1/Q)
                - (1 + dividendDiscount * cumNormalDist.derivative(-d1)
                   / std::sqrt(variance)) / Q;
            while (std::fabs(LHS - RHS)/payoff->strike() > tolerance) {
                Si = (payoff->strike() - RHS + bi * Si) / (1 + bi);
                forwardSi = Si * dividendDiscount / riskFreeDiscount;
                d1 = (std::log(forwardSi/payoff->strike())+0.5*variance)
                    /std::sqrt(variance);
                LHS = payoff->strike() - Si;
                Real temp2 = blackFormula(payoff->optionType(), payoff->strike(),
                    forwardSi, std::sqrt(variance))*riskFreeDiscount;
                RHS = temp2 - (1 - dividendDiscount * cumNormalDist(-d1)) * Si / Q;
                bi = -dividendDiscount * cumNormalDist(-d1) * (1 - 1 / Q)
                    - (1 + dividendDiscount * cumNormalDist.derivative(-d1)
                       / std::sqrt(variance)) / Q;
            }
            break;
          default:
            QL_FAIL("unknown option type");
        }

        return Si;
    }

    void BaroneAdesiWhaleyApproximationEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::American,
                   "not an American Option");

        std::shared_ptr<AmericanExercise> ex =
            std::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex, "non-American exercise given");
        QL_REQUIRE(!ex->payoffAtExpiry(),
                   "payoff at expiry not handled");

        std::shared_ptr<StrikedTypePayoff> payoff =
            std::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Real variance = process_->blackVolatility()->blackVariance(
            ex->lastDate(), payoff->strike());
        DiscountFactor dividendDiscount = process_->dividendYield()->discount(
            ex->lastDate());
        DiscountFactor riskFreeDiscount = process_->riskFreeRate()->discount(
            ex->lastDate());
        Real spot = process_->stateVariable()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");
        Real forwardPrice = spot * dividendDiscount / riskFreeDiscount;
        BlackCalculator black(payoff, forwardPrice, std::sqrt(variance),
                              riskFreeDiscount);

        if (dividendDiscount>=1.0 && payoff->optionType()==Option::Call) {
            // early exercise never optimal
            results_.value        = black.value();
            results_.delta        = black.delta(spot);
            results_.deltaForward = black.deltaForward();
            results_.elasticity   = black.elasticity(spot);
            results_.gamma        = black.gamma(spot);

            DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
            DayCounter divdc = process_->dividendYield()->dayCounter();
            DayCounter voldc = process_->blackVolatility()->dayCounter();
            Time t =
                rfdc.yearFraction(process_->riskFreeRate()->referenceDate(),
                                  arguments_.exercise->lastDate());
            results_.rho = black.rho(t);

            t = divdc.yearFraction(process_->dividendYield()->referenceDate(),
                                   arguments_.exercise->lastDate());
            results_.dividendRho = black.dividendRho(t);

            t = voldc.yearFraction(process_->blackVolatility()->referenceDate(),
                                   arguments_.exercise->lastDate());
            results_.vega        = black.vega(t);
            results_.theta       = black.theta(spot, t);
            results_.thetaPerDay = black.thetaPerDay(spot, t);

            results_.strikeSensitivity  = black.strikeSensitivity();
            results_.itmCashProbability = black.itmCashProbability();
        } else {
            // early exercise can be optimal
            CumulativeNormalDistribution cumNormalDist;
            Real tolerance = 1e-6;
            Real Sk = criticalPrice(payoff, riskFreeDiscount,
                dividendDiscount, variance, tolerance);
            Real forwardSk = Sk * dividendDiscount / riskFreeDiscount;
            Real d1 = (std::log(forwardSk/payoff->strike()) + 0.5*variance)
                /std::sqrt(variance);
            Real n = 2.0*std::log(dividendDiscount/riskFreeDiscount)/variance;
            Real K = (!close(riskFreeDiscount, 1.0, 1000))
                    ? Real(-2.0*std::log(riskFreeDiscount)
                       / (variance*(1.0-riskFreeDiscount)))
                     : 2.0/variance;
            Real Q, a;
            switch (payoff->optionType()) {
                case Option::Call:
                    Q = (-(n-1.0) + std::sqrt(((n-1.0)*(n-1.0))+4.0*K))/2.0;
                    a =  (Sk/Q) * (1.0 - dividendDiscount * cumNormalDist(d1));
                    if (spot<Sk) {
                        results_.value = black.value() +
                            a * std::pow((spot/Sk), Q);
                    } else {
                        results_.value = spot - payoff->strike();
                    }
                    break;
                case Option::Put:
                    Q = (-(n-1.0) - std::sqrt(((n-1.0)*(n-1.0))+4.0*K))/2.0;
                    a = -(Sk/Q) *
                        (1.0 - dividendDiscount * cumNormalDist(-d1));
                    if (spot>Sk) {
                        results_.value = black.value() +
                            a * std::pow((spot/Sk), Q);
                    } else {
                        results_.value = payoff->strike() - spot;
                    }
                    break;
                default:
                  QL_FAIL("unknown option type");
            }
        } // end of "early exercise can be optimal"
    }

}
