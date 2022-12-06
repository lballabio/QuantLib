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

#include <ql/exercise.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/bjerksundstenslandengine.hpp>
#include <utility>

#include <iostream>

namespace QuantLib {

    namespace {

        CumulativeNormalDistribution cumNormalDist;

        #define erfc(x) (2*(1-cumNormalDist(M_SQRT2*x)))

        Real phi(Real S, Real gamma, Real H, Real I,
                 Real rT, Real bT, Real variance) {

            Real lambda = (-rT + gamma * bT + 0.5 * gamma * (gamma - 1.0)
                * variance);
            Real d = -(std::log(S / H) + (bT + (gamma - 0.5) * variance) )
                / std::sqrt(variance);
            Real kappa = 2.0 * bT / variance + (2.0 * gamma - 1.0);
            return std::exp(lambda) * (cumNormalDist(d)
                - std::pow((I / S), kappa) *
                cumNormalDist(d - 2.0 * std::log(I/S) / std::sqrt(variance)));
        }

        Real phi_S(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            return std::exp(bT*gamma - rT + ((-1 +gamma)*gamma*v)/2.)*((-(std::pow(I/S,2*(gamma + bT/v))
                /(std::exp(std::pow(2*bT - v + 2*gamma*v + 4*std::log(I/S) + 2*std::log(S/H),2)/(8.*v))*I))
                - 1/(std::exp(squared(2*bT - v + 2*gamma*v + 2*std::log(S/H))/(8.*v))*S))/(std::sqrt(2*M_PI)*std::sqrt(v)) +
                (std::pow(I/S,2*(gamma + bT/v))*(2*bT + (-1 + 2*gamma)*v)*erfc((2*bT
                - v + 2*gamma*v + 4*std::log(I/S) + 2*std::log(S/H))/(2.*std::sqrt(2*v))))/(2.*I*v));
        }

        Real phi_H(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            return (std::exp(bT*gamma - rT + ((-1 + gamma)*gamma*v)/2.)*(I/std::exp(std::pow(2*bT - v + 2*gamma*v +
                    2*std::log(S/H),2)/(8.*v)) - (std::pow(I/S,2*(gamma + bT/v))*S)/
                    std::exp(std::pow(2*bT - v + 2*gamma*v + 4*std::log(I/S) +
                    2*std::log(S/H),2)/(8.*v))))/(H*I*std::sqrt(2*M_PI)*std::sqrt(v));
        }

        Real phi_I(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            return (std::exp(bT*gamma - rT + ((-1 + gamma)*gamma*v)/2.)*std::pow(I/S,2*(gamma +
                    bT/v))*S*((2*std::sqrt(2/M_PI))/(std::exp(squared(2*bT - v + 2*gamma*v + 4*std::log(I/S) + 2*std::log(S/H))
                    /(8.*v))*std::sqrt(v)) + (1 - 2*gamma - (2*bT)/v)*erfc((2*bT - v + 2*gamma*v + 4*std::log(I/S) +
                    2*std::log(S/H))/(2.*std::sqrt(2*v)))))/(2.*I*I);
        }

        Real phi_rt(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            return (std::exp(bT*gamma - rT + ((-1 + gamma)*gamma*v)/2.)*(-(I*erfc((2*bT
                    - v + 2*gamma*v + 2*std::log(S/H))/(2.*std::sqrt(2*v)))) +
                    std::pow(I/S,2*(gamma + bT/v))*S*erfc((2*bT - v + 2*gamma*v + 4*std::log(I/S) +
                    2*std::log(S/H))/(2.*std::sqrt(2*v)))))/(2.*I);
        }

        Real phi_bt(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            return (std::exp(bT*gamma - rT + ((-1 + gamma)*gamma*v)/2.)*(std::sqrt(2)*(-(I/std::exp(std::pow(2*bT - v +
                    2*gamma*v + 2*std::log(S/H),2)/(8.*v))) + (std::pow(I/S,2*(gamma +
                    bT/v))*S)/std::exp(std::pow(2*bT - v + 2*gamma*v + 4*std::log(I/S) +
                    2*std::log(S/H),2)/(8.*v)))*std::sqrt(v) + gamma*I*std::sqrt(M_PI)*v*erfc((2*bT - v + 2*gamma*v +
                    2*std::log(S/H))/(2.*std::sqrt(2*v))) - std::sqrt(M_PI)*std::pow(I/S,2*(gamma + bT/v))*S*erfc((2*bT - v +
                    2*gamma*v + 4*std::log(I/S) + 2*std::log(S/H))/(2.*std::sqrt(2*v)))*(gamma*v +
                    2*std::log(I/S))))/(2.*I*std::sqrt(M_PI)*v);
        }
    }

    BjerksundStenslandApproximationEngine::BjerksundStenslandApproximationEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    OneAssetOption::results
    BjerksundStenslandApproximationEngine::americanCallApproximation(
        Real S, Real X, Real rfD, Real dD, Real variance) const {

        OneAssetOption::results results;

        Real bT = std::log(dD/rfD);
        Real rT = std::log(1.0/rfD);

        Real beta = (0.5 - bT/variance) +
            std::sqrt(squared(bT/variance - 0.5) + 2.0 * rT/variance);
        Real BInfinity = beta / (beta - 1.0) * X;
        Real B0 = close_enough(bT, rT) ?
            X : std::max(X, rT / (rT - bT) * X);
        Real ht = -(bT + 2.0*std::sqrt(variance)) * B0 / (BInfinity - B0);

        Real I = B0 + (BInfinity - B0) * (1 - std::exp(ht));

        if (S >= I) {
            results.value = S - X;
            results.delta = 1.0;
            results.strikeSensitivity = -1.0;
            results.dividendRho = 0.0;
        } else {
                results.value = (I - X) * std::pow(S/I, beta)
                        *(1 - phi(S, beta, I, I, rT, bT, variance))
                    +    S *  phi(S,  1.0, I, I, rT, bT, variance)
                    -    S *  phi(S,  1.0, X, I, rT, bT, variance)
                    -    X *  phi(S,  0.0, I, I, rT, bT, variance)
                    +    X *  phi(S,  0.0, X, I, rT, bT, variance);

            results.delta = (I - X) * std::pow(S/I, beta-1)*beta/I
                    * (1 - phi(S, beta, I, I, rT, bT, variance))
                - (I - X) * std::pow(S/I, beta)
                    * phi_S(S, beta, I, I, rT, bT, variance)
                +   phi(S,  1.0, I, I, rT, bT, variance)
                + S*phi_S(S,  1.0, I, I, rT, bT, variance)
                -   phi(S,  1.0, X, I, rT, bT, variance)
                - S*phi_S(S,  1.0, X, I, rT, bT, variance)
                - X*phi_S(S,  0.0, I, I, rT, bT, variance)
                + X*phi_S(S,  0.0, X, I, rT, bT, variance);


            const Real BInfinityDX = beta / (beta - 1.0);
            const Real B0DX = std::max(Real(1.0), rT / (rT - bT));
            const Real htDX = -(bT + 2.0*std::sqrt(variance))
                * (B0DX/(BInfinity - B0)
                   - B0*(BInfinityDX-B0DX)/squared(BInfinity - B0));
            const Real IDX = B0DX + (BInfinityDX - B0DX)*(1-std::exp(ht))
                - (BInfinity - B0)*std::exp(ht)*htDX;

            results.strikeSensitivity =
                ((IDX-1)*std::pow(S/I, beta)
                    -(I-X)*beta*std::pow(S/I, beta-1)*S/(I*I)*IDX)
                    *(1 - phi(S, beta, I, I, rT, bT, variance))
               + (I - X) * std::pow(S/I, beta)*(
                       -phi_H(S, beta, I, I, rT, bT, variance)*IDX
                       -phi_I(S, beta, I, I, rT, bT, variance)*IDX)
               + S*(  phi_H(S, 1.0, I, I, rT, bT, variance)*IDX
                    + phi_I(S, 1.0, I, I, rT, bT, variance)*IDX)
               - S*(  phi_H(S, 1.0, X, I, rT, bT, variance)
                    + phi_I(S, 1.0, X, I, rT, bT, variance)*IDX)
               - phi(S,  0.0, I, I, rT, bT, variance)
               - X*(  phi_H(S,  0.0, I, I, rT, bT, variance)*IDX
                    + phi_I(S,  0.0, I, I, rT, bT, variance)*IDX)
               + phi(S,  0.0, X, I, rT, bT, variance)
               + X*(  phi_H(S,  0.0, X, I, rT, bT, variance)
                    + phi_I(S,  0.0, X, I, rT, bT, variance)*IDX);

            Time t = process_->dividendYield()->dayCounter().yearFraction(
                process_->dividendYield()->referenceDate(),
                arguments_.exercise->lastDate());

            const Real betaDq = t*(1/variance
                 - 1/(2*std::sqrt(squared(bT/variance - 0.5) + 2.0 * rT/variance))
                   * 2*(bT/variance-0.5)/variance);

            results.dividendRho = betaDq;
        }

        return results;
    }


    void BjerksundStenslandApproximationEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::American,
                   "not an American Option");

        ext::shared_ptr<AmericanExercise> ex =
            ext::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(ex, "non-American exercise given");
        QL_REQUIRE(!ex->payoffAtExpiry(),
                   "payoff at expiry not handled");

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
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
            // use put-call symmetry
            std::swap(spot, strike);
            std::swap(riskFreeDiscount, dividendDiscount);
            payoff = ext::make_shared<PlainVanillaPayoff>(
                                Option::Call, strike);
        }

        if (dividendDiscount > 1.0 && riskFreeDiscount > dividendDiscount)
            QL_FAIL("double-boundary case r<q<0 for a call given");


        if (dividendDiscount>=1.0 && dividendDiscount >= riskFreeDiscount) {
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
            results_.additionalResults["strikeGamma"] = black.strikeGamma();

        } else {
            // early exercise can be optimal - use approximation
            results_ = americanCallApproximation(spot,
                                                 strike,
                                                 riskFreeDiscount,
                                                 dividendDiscount,
                                                 variance);
        }

        if (ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff)
                ->optionType() == Option::Put) {

            // swap greeks w.r.t. S <-> K and r <-> q
            std::swap(spot, strike);
            std::swap(riskFreeDiscount, dividendDiscount);

            std::swap(results_.delta, results_.strikeSensitivity);

            results_.deltaForward =
                results_.delta*riskFreeDiscount/dividendDiscount;

            results_.elasticity = results_.delta/results_.value*spot;

            Real tmp = results_.gamma;
            results_.gamma =
                boost::any_cast<Real>(results_.additionalResults["strikeGamma"]);
            results_.additionalResults["strikeGamma"] = tmp;

            std::swap(results_.rho, results_.dividendRho);

            Time tr = process_->riskFreeRate()->dayCounter().yearFraction(
                process_->riskFreeRate()->referenceDate(),
                arguments_.exercise->lastDate());
            Time tq = process_->dividendYield()->dayCounter().yearFraction(
                process_->dividendYield()->referenceDate(),
                arguments_.exercise->lastDate());

            results_.rho *= tr/tq;
            results_.dividendRho *= tq/tr;
        }
    }
}
