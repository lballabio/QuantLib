/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
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

#include <ql/pricingengines/vanilla/bjerksundstenslandengine.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    namespace {

        CumulativeNormalDistribution cumNormalDist;

        Real phi(Real S, Real gamma, Real H, Real I,
                 Real rT, Real bT, Real variance) {

            Real lambda = (-rT + gamma * bT + 0.5 * gamma * (gamma - 1.0)
                * variance);
            Real d = -(std::log(S / H) + (bT + (gamma - 0.5) * variance) )
                / std::sqrt(variance);
            Real kappa = 2.0 * bT / variance + (2.0 * gamma - 1.0);
            return std::exp(lambda) * std::pow(S, gamma) * (cumNormalDist(d)
                - std::pow((I / S), kappa) *
                cumNormalDist(d - 2.0 * std::log(I/S) / std::sqrt(variance)));
        }


        Real americanCallApproximation(Real S, Real X,
                                       Real rfD, Real dD, Real variance) {

            Real bT = std::log(dD/rfD);
            Real rT = std::log(1.0/rfD);

            Real beta = (0.5 - bT/variance) +
                std::sqrt(std::pow((bT/variance - 0.5), Real(2.0))
                          + 2.0 * rT/variance);
            Real BInfinity = beta / (beta - 1.0) * X;
            // Real B0 = std::max(X, std::log(rfD) / std::log(dD) * X);
            Real B0 = std::max(X, rT / (rT - bT) * X);
            Real ht = -(bT + 2.0*std::sqrt(variance)) * B0 / (BInfinity - B0);

            // investigate what happen to I for dD->0.0
            Real I = B0 + (BInfinity - B0) * (1 - std::exp(ht));
            QL_REQUIRE(I >= X,
                       "Bjerksund-Stensland approximation not applicable "
                       "to this set of parameters");
            if (S >= I) {
                return S - X;
            } else {
                // investigate what happen to alpha for dD->0.0
                Real alpha = (I - X) * std::pow(I, (-beta));
                return alpha * std::pow(S, beta)
                    - alpha * phi(S, beta, I, I, rT, bT, variance)
                    +         phi(S,  1.0, I, I, rT, bT, variance)
                    -         phi(S,  1.0, X, I, rT, bT, variance)
                    -    X *  phi(S,  0.0, I, I, rT, bT, variance)
                    +    X *  phi(S,  0.0, X, I, rT, bT, variance);
            }
        }
    }

    BjerksundStenslandApproximationEngine::
    BjerksundStenslandApproximationEngine(
              const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process) {
        registerWith(process_);
    }

    void BjerksundStenslandApproximationEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::American,
                   "not an American Option");

        boost::shared_ptr<AmericanExercise> ex =
            boost::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex, "non-American exercise given");
        QL_REQUIRE(!ex->payoffAtExpiry(),
                   "payoff at expiry not handled");

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Real variance =
            process_->blackVolatility()->blackVariance(ex->lastDate(),
                                                       payoff->strike());
        DiscountFactor dividendDiscount =
            process_->dividendYield()->discount(ex->lastDate());
        DiscountFactor riskFreeDiscount =
            process_->riskFreeRate()->discount(ex->lastDate());
        Real spot = process_->stateVariable()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");
        Real strike = payoff->strike();

        if (payoff->optionType()==Option::Put) {
            // use put-call simmetry
            std::swap(spot, strike);
            std::swap(riskFreeDiscount, dividendDiscount);
            payoff = boost::shared_ptr<PlainVanillaPayoff>(
                                new PlainVanillaPayoff(Option::Call, strike));
        }

        if (dividendDiscount>=1.0) {
            // early exercise is never optimal - use Black formula
            Real forwardPrice = spot * dividendDiscount / riskFreeDiscount;
            BlackCalculator black(payoff, forwardPrice, std::sqrt(variance),
                                  riskFreeDiscount);

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
            // early exercise can be optimal - use approximation
            results_.value = americanCallApproximation(spot,
                                                       strike,
                                                       riskFreeDiscount,
                                                       dividendDiscount,
                                                       variance);
        }
    }

}
