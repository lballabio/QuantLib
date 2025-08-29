/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2023 Klaus Spanderen

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

#include <ql/any.hpp>
#include <ql/exercise.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/bjerksundstenslandengine.hpp>
#include <utility>
#include <cmath>

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
            return std::exp(lambda) * (cumNormalDist(d)
                - std::pow((I / S), kappa) *
                cumNormalDist(d - 2.0 * std::log(I/S) / std::sqrt(variance)));
        }

        Real phi_S(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            const Real lsh = std::log(S/H);
            const Real lis = std::log(I/S);
            const Real sv = std::sqrt(v);

            return std::exp(bT*gamma - rT + ((-1 +gamma)*gamma*v)/2.)*((-(std::pow(I/S,2*(gamma + bT/v))/(std::exp(squared(2*bT - v + 2*gamma*v + 4*lis + 2*lsh)/(8.*v))*I))- 1/(std::exp(squared(2*bT - v + 2*gamma*v + 2*lsh)/(8.*v))*S))/(M_SQRT2*M_SQRTPI*sv) +(std::pow(I/S,2*(gamma + bT/v))*(2*bT + (-1 + 2*gamma)*v)*std::erfc((2*bT- v + 2*gamma*v + 4*lis + 2*lsh)/(2.*M_SQRT2*sv)))/(2.*I*v));
        }

        Real phi_SS(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            const Real lsh = std::log(S/H);
            const Real lis = std::log(I/S);
            const Real sv = std::sqrt(v);
            const Real ex = std::exp(squared(2*bT - v + 2*gamma*v + 4*lis + 2*lsh)/(8.*v));
            const Real ey = std::exp(squared(2*bT + (-1 + 2*gamma)*v + 2*lsh)/(8.*v));

            return (std::exp(bT*gamma - rT + ((-1 +gamma)*gamma*v)/2.)*((M_SQRT2*I*v*sv)/ey +(2*M_SQRT2*std::pow(I/S,2*(gamma + bT/v))*S*sv*(2*bT +(-1 + 2*gamma)*v))/ex -2*std::sqrt(M_PI)*std::pow(I/S,2*(gamma + bT/v))*S*(bT +gamma*v)*(2*bT + (-1 + 2*gamma)*v)*std::erfc((2*bT - v + 2*gamma*v +4*lis + 2*lsh)/(2.*M_SQRT2*sv)) +(M_SQRT2*I*sv*(bT + (-0.5 + gamma)*v +lsh))/ey - (std::pow(I/S,2*(gamma + bT/v))*S*sv*(2*bT - 3*v + 2*gamma*v + 4*lis +2*lsh))/(M_SQRT2*ex)))/(2.*I*M_SQRTPI*squared(S*v));
        }

        Real phi_gamma(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            const Real lsh = std::log(S/H);
            const Real lis = std::log(I/S);
            const Real sv = std::sqrt(v);

            return std::exp(bT*gamma - rT + ((-1 + gamma)*gamma*v)/2)*(((-std::exp(-squared(2*bT - v + 2*gamma*v +2*lsh)/(8*v)) + std::pow(I/S,-1 + 2*gamma +(2*bT)/v)/std::exp(squared(2*bT - v + 2*gamma*v + 4*lis +2*lsh)/(8*v)))*sv)/(M_SQRT2*M_SQRTPI) + ((2*bT+ (-1 + 2*gamma)*v)*std::erfc((2*bT + (-1 + 2*gamma)*v +2*lsh)/(2.*M_SQRT2*sv)))/4. -(std::pow(I/S,-1 + 2*gamma + (2*bT)/v)*std::erfc((2*bT - v + 2*gamma*v + 4*lis +2*lsh)/(2.*M_SQRT2*sv))*(2*bT + (-1 + 2*gamma)*v + 4*lis))/4.);
        }

        Real phi_H(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            const Real lsh = std::log(S/H);

            return (std::exp(bT*gamma - rT + ((-1 + gamma)*gamma*v)/2.)*(I/std::exp(squared(2*bT - v + 2*gamma*v + 2*lsh)/(8.*v))- (std::pow(I/S,2*(gamma + bT/v))*S)/std::exp(squared(2*bT - v + 2*gamma*v + 4*std::log(I/S) + 2*lsh)/(8.*v))))/(H*I*std::sqrt(2*M_PI)*std::sqrt(v));
        }

        Real phi_I(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            const Real lsh = std::log(S/H);
            const Real lis = std::log(I/S);
            const Real sv = std::sqrt(v);

            return (std::exp(bT*gamma - rT + ((-1 + gamma)*gamma*v)/2.)*std::pow(I/S,2*(gamma + bT/v))*S*((2*std::sqrt(2/M_PI))/(std::exp(squared(2*bT - v + 2*gamma*v + 4*lis + 2*lsh)/(8.*v))*sv) + (1 - 2*gamma - (2*bT)/v)*std::erfc((2*bT - v + 2*gamma*v + 4*lis +2*lsh)/(2.*M_SQRT2*sv))))/(2.*I*I);
        }

        Real phi_rt(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            const Real lsh = std::log(S/H);
            return (std::exp(bT*gamma - rT + ((-1 + gamma)*gamma*v)/2.)*(-(I*std::erfc((2*bT- v + 2*gamma*v + 2*lsh)/(2.*std::sqrt(2*v)))) +std::pow(I/S,2*(gamma + bT/v))*S*std::erfc((2*bT - v + 2*gamma*v + 4*std::log(I/S) +2*lsh)/(2.*std::sqrt(2*v)))))/(2.*I);
        }

        Real phi_bt(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            const Real lsh = std::log(S/H);
            const Real lis = std::log(I/S);
            const Real sv = std::sqrt(v);

            return (std::exp(bT*gamma - rT + ((-1 + gamma)*gamma*v)/2.)*(M_SQRT2*(-(I/std::exp(squared(2*bT - v +2*gamma*v + 2*lsh)/(8.*v))) + (std::pow(I/S,2*(gamma +bT/v))*S)/std::exp(squared(2*bT - v + 2*gamma*v + 4*lis +2*lsh)/(8.*v)))*sv + gamma*I*std::sqrt(M_PI)*v*std::erfc((2*bT - v + 2*gamma*v +2*lsh)/(2.*M_SQRT2*sv)) - M_SQRTPI*std::pow(I/S,2*(gamma + bT/v))*S*std::erfc((2*bT - v +2*gamma*v + 4*lis + 2*lsh)/(2.*M_SQRT2*sv))*(gamma*v +2*lis)))/(2.*I*std::sqrt(M_PI)*v);
        }

        Real phi_v(Real S, Real gamma, Real H, Real I, Real rT, Real bT, Real v) {
            const Real lsh = std::log(S/H);
            const Real lis = std::log(I/S);
            const Real sv = std::sqrt(v);
            const Real er = std::erfc((2*bT - v + 2*gamma*v + 4*lis + 2*lsh)/(2.*M_SQRT2*sv));

            return (std::exp(bT*gamma - rT + ((-1 + gamma)*gamma*v)/2.)*(((-1 +gamma)*gamma*(I*std::erfc((2*bT - v + 2*gamma*v + 2*lsh)/(2.*M_SQRT2*sv)) -std::pow(I/S,2*(gamma + bT/v))*S*er))/(2.*I) +(2*bT*std::pow(I/S,-1 + 2*gamma + (2*bT)/v)*er*lis)/(v*v)+ (2*bT + v - 2*gamma*v + 2*lsh)/(2.*std::exp(std::pow(2*bT + (-1 + 2*gamma)*v +2*lsh,2)/(8.*v))*M_SQRT2*M_SQRTPI*v*sv) -(std::pow(I/S,-1 + 2*gamma + (2*bT)/v)*(2*bT + v - 2*gamma*v +4*lis + 2*lsh))/(2.*std::exp(squared(2*bT - v + 2*gamma*v + 4*lis + 2*lsh)/(8.*v))*M_SQRT2*M_SQRTPI*v*sv)))/2.;
        }
    }

    BjerksundStenslandApproximationEngine::BjerksundStenslandApproximationEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    OneAssetOption::results
    BjerksundStenslandApproximationEngine::europeanCallResults(
        Real S, Real X, Real rfD, Real dD, Real variance) const {

        OneAssetOption::results results;

        const Real forwardPrice = S * dD/rfD;
        const BlackCalculator black(
            Option::Call, X, forwardPrice, std::sqrt(variance), rfD);

        results.value        = black.value();
        results.delta        = black.delta(S);
        results.gamma        = black.gamma(S);

        const DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
        const DayCounter divdc = process_->dividendYield()->dayCounter();
        const DayCounter voldc = process_->blackVolatility()->dayCounter();
        Time t =
            rfdc.yearFraction(process_->riskFreeRate()->referenceDate(),
                              arguments_.exercise->lastDate());
        results.rho = black.rho(t);

        t = divdc.yearFraction(process_->dividendYield()->referenceDate(),
                               arguments_.exercise->lastDate());
        results.dividendRho = black.dividendRho(t);

        t = voldc.yearFraction(process_->blackVolatility()->referenceDate(),
                               arguments_.exercise->lastDate());
        results.vega        = black.vega(t);
        results.theta       = black.theta(S, t);
        results.thetaPerDay = black.thetaPerDay(S, t);

        results.strikeSensitivity  = black.strikeSensitivity();
        results.additionalResults["strikeGamma"] = Real(results.gamma*squared(S/X));
        results.additionalResults["exerciseType"] = std::string("European");

        return results;
    }

    OneAssetOption::results
    BjerksundStenslandApproximationEngine::immediateExercise(Real S, Real X) const {
        OneAssetOption::results results;
        results.value = std::max(0.0, S - X);
        results.delta = (S >= X)? 1.0 : 0.0;
        results.gamma = 0.0;
        results.rho = 0.0;
        results.dividendRho = 0.0;
        results.vega = 0.0;
        results.theta = 0.0;
        results.thetaPerDay = 0.0;

        results.strikeSensitivity = -results.delta;
        results.additionalResults["strikeGamma"] = Real(0.0);
        results.additionalResults["exerciseType"] = std::string("Immediate");

        return results;
    }

    OneAssetOption::results
    BjerksundStenslandApproximationEngine::americanCallApproximation(
        Real S, Real X, Real rfD, Real dD, Real variance) const {

        const OneAssetOption::results europeanResults
            = europeanCallResults(S, X, rfD, dD, variance);

        OneAssetOption::results results;

        const Real bT = std::log(dD/rfD);
        const Real rT = std::log(1.0/rfD);

        const Real beta = (0.5 - bT/variance) +
            std::sqrt(squared(bT/variance - 0.5) + 2.0 * rT/variance);

        const Real BInfinity = beta / (beta - 1.0) * X;
        const Real B0 = (bT == rT) ? X : std::max(X, rT / (rT - bT) * X);
        const Real ht = -(bT + 2.0*std::sqrt(variance)) * B0 / (BInfinity - B0);

        const Real I = B0 + (BInfinity - B0) * (1 - std::exp(ht));

        const Real fwd = S * dD/rfD;
        const Real q = std::log(I/fwd)/std::sqrt(variance);

        if (S >= I) {
            results = immediateExercise(S, X);
        }
        else if (q > 12.5) {
            // We have a run away exercise boundary. It is numerically
            // more accurate to use the Greeks of the European engine.
            results = europeanResults;
        }
        else {
            const Real phi_S_beta_I_I_rT_bT_v
                = phi(S, beta, I, I, rT, bT, variance);
            const Real phi_S_1_I_I_rT_bT_v
                = phi(S,  1.0, I, I, rT, bT, variance);
            const Real phi_S_1_X_I_rT_bT_V
                = phi(S, 1.0, X, I, rT, bT, variance);
            results.value = (I - X) * std::pow(S/I, beta)
                    *(1 - phi_S_beta_I_I_rT_bT_v)
                +    S * phi_S_1_I_I_rT_bT_v
                -    S * phi_S_1_X_I_rT_bT_V
                -    X * phi(S, 0.0, I, I, rT, bT, variance)
                +    X * phi(S, 0.0, X, I, rT, bT, variance);

            const Real phi_S_S_beta_I_I_rT_bT_v
                = phi_S(S, beta, I, I, rT, bT, variance);
            const Real phi_S_S_1_I_I_rT_bT_v
                = phi_S(S, 1.0, I, I, rT, bT, variance);
            const Real phi_S_S_1_X_I_rT_bT_v
                = phi_S(S, 1.0, X, I, rT, bT, variance);
            results.delta = (I - X) * std::pow(S/I, beta-1)*beta/I
                    * (1 - phi_S_beta_I_I_rT_bT_v)
                - (I - X) * std::pow(S/I, beta)
                    * phi_S_S_beta_I_I_rT_bT_v
                +   phi_S_1_I_I_rT_bT_v
                + S*phi_S_S_1_I_I_rT_bT_v
                -   phi_S_1_X_I_rT_bT_V
                - S*phi_S_S_1_X_I_rT_bT_v
                - X*phi_S(S, 0.0, I, I, rT, bT, variance)
                + X*phi_S(S, 0.0, X, I, rT, bT, variance);

            const Date refDate = process_->riskFreeRate()->referenceDate();
            const Date exerciseDate = arguments_.exercise->lastDate();
            const DayCounter qdc = process_->dividendYield()->dayCounter();
            const Time tq = qdc.yearFraction(refDate, exerciseDate);

            const Real betaDq = tq*(1/variance
                - 1/(2*std::sqrt(squared(bT/variance - 0.5) + 2.0 * rT/variance))
                  * 2*(bT/variance-0.5)/variance);
            const Real BInfinityDq = -X/squared(beta-1.0)*betaDq;
            const Real B0Dq = (dD <= rfD) ? Real(0.0)
                : Real(X*std::log(rfD)/squared(std::log(dD))*tq);

            const Real htDq = tq * B0 / (BInfinity - B0)
                - (bT + 2.0*std::sqrt(variance))
                    *(B0Dq*(BInfinity - B0) - B0*(BInfinityDq - B0Dq))
                        /squared(BInfinity - B0);
            const Real IDq = B0Dq + (BInfinityDq - B0Dq) * (1 - std::exp(ht))
                - (BInfinity - B0) * std::exp(ht)*htDq;

            const Real phi_H_S_beta_I_I_rT_bT_v
                = phi_H(S, beta, I, I, rT, bT, variance);
            const Real phi_I_S_beta_I_I_rT_bT_v
                = phi_I(S, beta, I, I, rT, bT, variance);
            const Real phi_gamma_S_beta_I_I_rT_bT_v
                = phi_gamma(S, beta, I, I, rT, bT, variance);
            const Real phi_bt_S_beta_I_I_rT_bT_v
                = phi_bt(S, beta, I, I, rT, bT, variance);
            const Real phi_H_S_1_I_I_rT_bT_v
                = phi_H(S, 1.0, I, I, rT, bT, variance);
            const Real phi_I_S_1_I_I_rT_bT_v
                = phi_I(S, 1.0, I, I, rT, bT, variance);
            const Real phi_bt_S_1_I_I_rT_bT_v
                = phi_bt(S, 1.0, I, I, rT, bT, variance);
            const Real phi_I_S_1_X_I_rT_bT_v
                = phi_I(S, 1.0, X, I, rT, bT, variance);
            const Real phi_bt_S_1_X_I_rT_bT_v
                = phi_bt(S, 1.0, X, I, rT, bT, variance);
            const Real phi_H_S_0_I_I_rT_bT_v
                = phi_H(S, 0.0, I, I, rT, bT, variance);
            const Real phi_I_S_0_I_I_rT_bT_v
                = phi_I(S, 0.0, I, I, rT, bT, variance);
            const Real phi_bt_S_0_I_I_rT_bT_v
                = phi_bt(S, 0.0, I, I, rT, bT, variance);
            const Real phi_I_S_0_X_I_rT_bT_v
                = phi_I(S, 0.0, X, I, rT, bT, variance);
            const Real phi_bt_S_0_X_I_rT_bT_v
                = phi_bt(S, 0.0, X, I, rT, bT, variance);

            results.dividendRho =
                (IDq*std::pow(S/I, beta)
                 + (I-X)*std::pow(S/I, beta)*(betaDq*std::log(S/I) - beta*1/I*IDq))
                 * (1 - phi_S_beta_I_I_rT_bT_v)
                - (I - X) * std::pow(S/I, beta)
                 *( phi_H_S_beta_I_I_rT_bT_v*IDq
                   +phi_I_S_beta_I_I_rT_bT_v*IDq
                   +phi_gamma_S_beta_I_I_rT_bT_v*betaDq
                   -phi_bt_S_beta_I_I_rT_bT_v*tq)
                + S*(  phi_H_S_1_I_I_rT_bT_v*IDq
                     + phi_I_S_1_I_I_rT_bT_v*IDq
                     - phi_bt_S_1_I_I_rT_bT_v*tq)
                - S*(  phi_I_S_1_X_I_rT_bT_v*IDq
                     - phi_bt_S_1_X_I_rT_bT_v*tq)
                - X*(  phi_H_S_0_I_I_rT_bT_v*IDq
                     + phi_I_S_0_I_I_rT_bT_v*IDq
                     - phi_bt_S_0_I_I_rT_bT_v*tq)
                + X*(  phi_I_S_0_X_I_rT_bT_v*IDq
                     - phi_bt_S_0_X_I_rT_bT_v*tq);

            const DayCounter rdc = process_->riskFreeRate()->dayCounter();
            const Time tr = rdc.yearFraction(refDate, exerciseDate);

            const Real betaDr = tr*(-1/variance
                + 1/(2*std::sqrt(squared(bT/variance - 0.5) + 2.0 * rT/variance))
                  * 2*((bT/variance-0.5)/variance + 1/variance));
            const Real BInfinityDr = -X/squared(beta-1.0)*betaDr;
            const Real B0Dr = (dD <= rfD) ? Real(0) : Real(-X*tr/std::log(dD));
            const Real htDr = -tr * B0 / (BInfinity - B0)
                - (bT + 2.0*std::sqrt(variance))
                    *(B0Dr*(BInfinity - B0) - B0*(BInfinityDr - B0Dr))
                        /squared(BInfinity - B0);
            const Real IDr = B0Dr + (BInfinityDr - B0Dr) * (1 - std::exp(ht))
                - (BInfinity - B0) * std::exp(ht)*htDr;

            results.rho =
                (IDr*std::pow(S/I, beta)
                 + (I-X)*std::pow(S/I, beta)*(betaDr*std::log(S/I) - beta/I*IDr))
                 * (1 - phi_S_beta_I_I_rT_bT_v)
                - (I - X) * std::pow(S/I, beta)
                 *(  phi_H_S_beta_I_I_rT_bT_v*IDr
                   + phi_I_S_beta_I_I_rT_bT_v*IDr
                   + phi_gamma_S_beta_I_I_rT_bT_v*betaDr
                   + phi_rt(S, beta, I, I, rT, bT, variance)*tr
                   + phi_bt_S_beta_I_I_rT_bT_v*tr)
                + S*(  phi_H_S_1_I_I_rT_bT_v*IDr
                     + phi_I_S_1_I_I_rT_bT_v*IDr
                     + phi_rt(S, 1.0, I, I, rT, bT, variance)*tr
                     + phi_bt_S_1_I_I_rT_bT_v*tr)
                - S*(  phi_I_S_1_X_I_rT_bT_v*IDr
                     + phi_rt(S, 1.0, X, I, rT, bT, variance)*tr
                     + phi_bt_S_1_X_I_rT_bT_v*tr)
                - X*(  phi_H_S_0_I_I_rT_bT_v*IDr
                     + phi_I_S_0_I_I_rT_bT_v*IDr
                     + phi_rt(S, 0.0, I, I, rT, bT, variance)*tr
                     + phi_bt_S_0_I_I_rT_bT_v*tr)
                + X*(  phi_I_S_0_X_I_rT_bT_v*IDr
                     + phi_rt(S, 0.0, X, I, rT, bT, variance)*tr
                     + phi_bt_S_0_X_I_rT_bT_v*tr);

            const Real beta = (0.5 - bT/variance) +
                std::sqrt(squared(bT/variance - 0.5) + 2.0 * rT/variance);

            const DayCounter vdc = process_->blackVolatility()->dayCounter();
            const Time tv = vdc.yearFraction(refDate, exerciseDate);
            const Real varianceDv = 2*std::sqrt(variance*tv);

            const Real betaDv = bT/squared(variance)*varianceDv +
                - 1/(2*std::sqrt(squared(bT/variance - 0.5) + 2.0 * rT/variance))
                  *( 2*(bT/variance - 0.5)*bT*varianceDv/squared(variance)
                    +2*rT/squared(variance)*varianceDv );
            const Real BInfinityDv = -X/squared(beta-1.0)*betaDv;
            const Real htDv = -1/std::sqrt(variance)*varianceDv*B0/(BInfinity-B0)
                    + (bT + 2*std::sqrt(variance))*B0/squared(BInfinity-B0)*BInfinityDv;

            const Real IDv = BInfinityDv*(1-std::exp(ht))
                - (BInfinity-B0)*std::exp(ht)*htDv;

            results.vega =
                (IDv*std::pow(S/I, beta)
                 + (I-X)*std::pow(S/I, beta)*(betaDv*std::log(S/I) - beta/I*IDv))
                 * (1 - phi_S_beta_I_I_rT_bT_v)
                - (I - X) * std::pow(S/I, beta)
                *(  phi_H_S_beta_I_I_rT_bT_v*IDv
                  + phi_I_S_beta_I_I_rT_bT_v*IDv
                  + phi_gamma_S_beta_I_I_rT_bT_v*betaDv
                  + phi_v(S, beta, I, I, rT, bT, variance)*varianceDv)
               + S*(  phi_H_S_1_I_I_rT_bT_v*IDv
                    + phi_I_S_1_I_I_rT_bT_v*IDv
                    + phi_v(S, 1.0, I, I, rT, bT, variance)*varianceDv)
               - S*(  phi_I_S_1_X_I_rT_bT_v*IDv
                    + phi_v(S, 1.0, X, I, rT, bT, variance)*varianceDv)
               - X*(  phi_H_S_0_I_I_rT_bT_v*IDv
                    + phi_I_S_0_I_I_rT_bT_v*IDv
                    + phi_v(S, 0.0, I, I, rT, bT, variance)*varianceDv)
               + X*(  phi_I_S_0_X_I_rT_bT_v*IDv
                    + phi_v(S, 0.0, X, I, rT, bT, variance)*varianceDv);

            results.gamma =
                  (I - X) * std::pow(S/I, beta-2)*beta*(beta-1)/squared(I)
                  * (1 - phi_S_beta_I_I_rT_bT_v)
                - 2*(I - X) * std::pow(S/I, beta-1)*beta/I
                  *phi_S_S_beta_I_I_rT_bT_v
                - (I - X) * std::pow(S/I, beta)
                  * phi_SS(S, beta, I, I, rT, bT, variance)

                + 2*phi_S_S_1_I_I_rT_bT_v
                + S*phi_SS(S, 1.0, I, I, rT, bT, variance)

                - 2*phi_S_S_1_X_I_rT_bT_v
                - S*phi_SS(S, 1.0, X, I, rT, bT, variance)

                - X*phi_SS(S, 0.0, I, I, rT, bT, variance)
                + X*phi_SS(S, 0.0, X, I, rT, bT, variance);

            const Volatility vol = std::sqrt(variance/tv);

            const Date tomorrow = refDate + Period(1, Days);
            const Time dtq = qdc.yearFraction(refDate, exerciseDate)
                    - qdc.yearFraction(tomorrow,  exerciseDate);
            const Time dtr = rdc.yearFraction(refDate, exerciseDate)
                    - rdc.yearFraction(tomorrow,  exerciseDate);
            const Time dtv = vdc.yearFraction(refDate, exerciseDate)
                    - vdc.yearFraction(tomorrow,  exerciseDate);

            results.thetaPerDay = -(0.5*results.vega*vol/tv*dtv
                + results.rho*rT/(tr*tr)*dtr + results.dividendRho*(rT-bT)/(tq*tq)*dtq);
            results.theta = 365*results.thetaPerDay;

            results.strikeSensitivity = results.value/X - S/X*results.delta;
            results.additionalResults["strikeGamma"] = Real(results.gamma*squared(S/X));

            results.additionalResults["exerciseType"] = std::string("American");
        }

        // check if European engine gives higher NPV
        if (results.value < europeanResults.value) {
            results = europeanResults;
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


        if (dividendDiscount >= 1.0 && dividendDiscount >= riskFreeDiscount) {
            results_ = europeanCallResults(
                spot, strike, riskFreeDiscount, dividendDiscount, variance);
        } else {
            // early exercise can be optimal - use approximation
            results_ = americanCallApproximation(
                spot, strike, riskFreeDiscount, dividendDiscount, variance);
        }

        // check if immediate exercise gives higher NPV
        if (results_.value < (spot - strike)*(1+10*QL_EPSILON) ) {
            results_ = immediateExercise(spot, strike);
        }

        if (ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff)
                ->optionType() == Option::Put) {

            std::swap(results_.delta, results_.strikeSensitivity);

            Real tmp = results_.gamma;
            results_.gamma =
                ext::any_cast<Real>(results_.additionalResults["strikeGamma"]);
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
