/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Neil Firth
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2013 Fabien Le Floc'h

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
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/pricingengines/vanilla/juquadraticengine.hpp>
#include <utility>

namespace QuantLib {

    /*  An Approximate Formula for Pricing American Options
        Journal of Derivatives Winter 1999
        Ju, N.
    */


    JuQuadraticApproximationEngine::JuQuadraticApproximationEngine(
        std::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void JuQuadraticApproximationEngine::calculate() const {

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
        BlackCalculator black(payoff, forwardPrice,
                              std::sqrt(variance), riskFreeDiscount);

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
            NormalDistribution normalDist;

            Real tolerance = 1e-6;
            Real Sk = BaroneAdesiWhaleyApproximationEngine::criticalPrice(
                payoff, riskFreeDiscount, dividendDiscount, variance,
                tolerance);

            Real forwardSk = Sk * dividendDiscount / riskFreeDiscount;

            Real alpha = -2.0*std::log(riskFreeDiscount)/(variance);
            Real beta = 2.0*std::log(dividendDiscount/riskFreeDiscount)/
                                                (variance);
            Real h = 1 - riskFreeDiscount;
            Real phi;
            switch (payoff->optionType()) {
                case Option::Call:
                    phi = 1;
                    break;
                case Option::Put:
                    phi = -1;
                    break;
                default:
                  QL_FAIL("unknown option type");
            }
            //it can throw: to be fixed
            Real temp_root = std::sqrt ((beta-1)*(beta-1) + (4*alpha)/h);
            Real lambda = (-(beta-1) + phi * temp_root) / 2;
            Real lambda_prime = - phi * alpha / (h*h * temp_root);

            Real black_Sk = blackFormula(payoff->optionType(), payoff->strike(),
                                         forwardSk, std::sqrt(variance)) * riskFreeDiscount;
            Real hA = phi * (Sk - payoff->strike()) - black_Sk;

            Real d1_Sk = (std::log(forwardSk/payoff->strike()) + 0.5*variance)
                /std::sqrt(variance);
            Real d2_Sk = d1_Sk - std::sqrt(variance);
            Real part1 = forwardSk * normalDist(d1_Sk) /
                                        (alpha * std::sqrt(variance));
            Real part2 = - phi * forwardSk * cumNormalDist(phi * d1_Sk) *
                      std::log(dividendDiscount) / std::log(riskFreeDiscount);
            Real part3 = + phi * payoff->strike() * cumNormalDist(phi * d2_Sk);
            Real V_E_h = part1 + part2 + part3;

            Real b = (1-h) * alpha * lambda_prime / (2*(2*lambda + beta - 1));
            Real c = - ((1 - h) * alpha / (2 * lambda + beta - 1)) *
                (V_E_h / (hA) + 1 / h + lambda_prime / (2*lambda + beta - 1));
            Real temp_spot_ratio = std::log(spot / Sk);
            Real chi = temp_spot_ratio * (b * temp_spot_ratio + c);

            if (phi*(Sk-spot) > 0) {
                results_.value = black.value() +
                    hA * std::pow((spot/Sk), lambda) / (1 - chi);
                Real temp_chi_prime = (2 * b / spot) * std::log(spot/Sk);
                Real chi_prime = temp_chi_prime + c / spot;
                Real chi_double_prime = 2*b/(spot*spot)
                    - temp_chi_prime / spot - c / (spot*spot);
                Real d1_S = (std::log(forwardPrice/payoff->strike()) + 0.5*variance)
                    / std::sqrt(variance);
                //There is a typo in the original paper from Ju-Zhong
                //the first term is the Black-Scholes delta/gamma.    
                results_.delta = phi * dividendDiscount * cumNormalDist (phi * d1_S)
                    + (lambda / (spot * (1 - chi)) + chi_prime / ((1 - chi)*(1 - chi))) *
                    (phi * (Sk - payoff->strike()) - black_Sk) * std::pow((spot/Sk), lambda);

                results_.gamma = dividendDiscount * normalDist (phi*d1_S) 
                    / (spot * std::sqrt(variance))
                    + (2 * lambda * chi_prime / (spot * (1 - chi) * (1 - chi))
                        + 2 * chi_prime * chi_prime / ((1 - chi) * (1 - chi) * (1 - chi))
                        + chi_double_prime / ((1 - chi) * (1 - chi))
                        + lambda * (lambda - 1) / (spot * spot * (1 - chi)))
                    * (phi * (Sk - payoff->strike()) - black_Sk)
                    * std::pow((spot/Sk), lambda);
            } else {
                results_.value = phi * (spot - payoff->strike());
                results_.delta = phi;
                results_.gamma = 0;
            }

        } // end of "early exercise can be optimal"
    }

}
