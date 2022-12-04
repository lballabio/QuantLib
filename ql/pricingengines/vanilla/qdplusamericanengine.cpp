/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Klaus Spanderen

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

/*! \file qrplusamericanengine.cpp
*/

#include <ql/exercise.hpp>
#include <ql/utilities/null.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/solvers1d/ridder.hpp>
#include <ql/math/solvers1d/newton.hpp>
#include <ql/math/interpolations/chebyshevinterpolation.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/qdplusamericanengine.hpp>
#include <ql/math/integrals/tanhsinhintegral.hpp>
#ifndef QL_BOOST_HAS_TANH_SINH
#include <ql/math/integrals/gausslobattointegral.hpp>
#endif

namespace QuantLib {

    class QdPlusBoundaryEvaluator {
      public:
        QdPlusBoundaryEvaluator(
            Real S, Real strike, Rate rf, Rate dy, Volatility vol, Time t, Time T)
        : tau(t), K(strike), sigma(vol), sigma2(sigma * sigma), v(sigma * std::sqrt(tau)), r(rf),
          q(dy), dr(std::exp(-r * tau)), dq(std::exp(-q * tau)),
          ddr((std::abs(r*tau) > 1e-5)? r/(1-dr) : 1/(tau*(1-0.5*r*tau*(1-r*tau/3)))),
          omega(2 * (r - q) / sigma2),
          lambda(0.5 *
                 (-(omega - 1) - std::sqrt(squared(omega - 1) + 8 * ddr / sigma2))),
          lambdaPrime(2 * ddr*ddr /
                      (sigma2 * std::sqrt(squared(omega - 1) + 8 * ddr / sigma2))),
          alpha(2 * dr / (sigma2 * (2 * lambda + omega - 1))),
          beta(alpha * (ddr + lambdaPrime / (2 * lambda + omega - 1)) - lambda),
          xMax(QdPlusAmericanEngine::xMax(strike, r, q)),
          xMin(QL_EPSILON * 1e4 * std::min(0.5 * (strike + S), xMax)),

          sc(Null<Real>()) {}

        Real operator()(Real S) const {
            ++nrEvaluations;

            if (S != sc)
                preCalculate(S);

            if (close_enough(K-S, npv)) {
                return (1-dq*Phi_dp)*S + alpha*theta/dr;
            }
            else {
                const Real c0 = -beta - lambda + alpha*theta/(dr*(K-S-npv));
                return (1-dq*Phi_dp)*S + (lambda+c0)*(K-S-npv);
            }
        }
        Real derivative(Real S) const {
            if (S != sc)
                preCalculate(S);

            return 1 - dq*Phi_dp + dq/v*phi_dp + beta*(1-dq*Phi_dp)
                    + alpha/dr*charm;
        }
        Real fprime2(Real S) const {
            if (S != sc)
                preCalculate(S);

            const Real gamma = phi_dp*dq/(v*S);
            const Real colour = gamma*(q + (r-q)*dp/v + (1-dp*dm)/(2*tau));

            return dq*(phi_dp/(S*v) - phi_dp*dp/(S*v*v))
                    + beta*gamma + alpha/dr*colour;
        }

        Real xmin() const { return xMin; }
        Real xmax() const { return xMax; }
        Size evaluations() const { return nrEvaluations; }

      private:
        void preCalculate(Real S) const {
            S = std::max(QL_EPSILON, S);
            sc = S;
            dp = std::log(S*dq/(K*dr))/v + 0.5*v;
            dm = dp - v;
            Phi_dp = Phi(-dp);
            Phi_dm = Phi(-dm);
            phi_dp = phi(dp);

            npv = dr*K*Phi_dm - S*dq*Phi_dp;
            theta = r*K*dr*Phi_dm - q*S*dq*Phi_dp - sigma2*S/(2*v)*dq*phi_dp;
            charm = -dq*(phi_dp*((r-q)/v - dm/(2*tau)) +q*Phi_dp);
        }

        const CumulativeNormalDistribution Phi;
        const NormalDistribution phi;
        const Time tau;
        const Real K;
        const Volatility sigma, sigma2, v;
        const Rate r, q;
        const DiscountFactor dr, dq, ddr;
        const Real omega, lambda, lambdaPrime, alpha, beta, xMax, xMin;
        mutable Size nrEvaluations = 0;
        mutable Real sc, dp, dm, Phi_dp, Phi_dm, phi_dp;
        mutable Real npv, theta, charm;
    };


    detail::QdPlusAddOnValue::QdPlusAddOnValue(
        Time T, Real S, Real K, Rate r, Rate q, Volatility vol,
        const Real xmax, ext::shared_ptr<Interpolation> q_z)
    : T_(T), S_(S), K_(K), xmax_(xmax),
      r_(r), q_(q), vol_(vol), q_z_(std::move(q_z)) {}


    Real detail::QdPlusAddOnValue::operator()(Real z) const {
        const Real t = z*z;
        const Real q = (*q_z_)(2*std::sqrt(std::max(0.0, T_-t)/T_) - 1, true);
        const Real b_t = xmax_*std::exp(-std::sqrt(std::max(0.0, q)));

        const Real dr = std::exp(-r_*t);
        const Real dq = std::exp(-q_*t);
        const Real v = vol_*std::sqrt(t);

        Real r;
        if (v >= QL_EPSILON) {
            if (b_t > QL_EPSILON) {
                const Real dp = std::log(S_*dq/(b_t*dr))/v + 0.5*v;
                r = 2*z*(r_*K_*dr*Phi_(-dp+v) - q_*S_*dq*Phi_(-dp));
            }
            else
                r = 0.0;
        }
        else if (close_enough(S_*dq, b_t*dr))
            r = z*(r_*K_*dr - q_*S_*dq);
        else if (b_t*dr > S_*dq)
            r = 2*z*(r_*K_*dr - q_*S_*dq);
        else
            r = 0.0;

        return r;
    }


    detail::QdPutCallParityEngine::QdPutCallParityEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void detail::QdPutCallParityEngine::calculate() const {
        QL_REQUIRE(arguments_.exercise->type() == Exercise::American,
                   "not an American option");

        const auto payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        const Real spot = process_->x0();
        QL_REQUIRE(spot >= 0.0, "negative underlying given");

        const auto maturity = arguments_.exercise->lastDate();
        const Time T = process_->time(maturity);
        const Real S = process_->x0();
        const Real K = payoff->strike();
        const Rate r = -std::log(process_->riskFreeRate()->discount(maturity))/T;
        const Rate q = -std::log(process_->dividendYield()->discount(maturity))/T;
        const Volatility vol = process_->blackVolatility()->blackVol(T, K);

        QL_REQUIRE(S >= 0, "zero or positive underlying value is required");
        QL_REQUIRE(K >= 0, "zero or positive strike is required");
        QL_REQUIRE(vol >= 0, "zero or positive volatility is required");

        if (payoff->optionType() == Option::Put)
            results_.value = calculatePutWithEdgeCases(S, K, r, q, vol, T);
        else if (payoff->optionType() == Option::Call)
            results_.value = calculatePutWithEdgeCases(K, S, q, r, vol, T);
        else
            QL_FAIL("unknown option type");
    }

    Real detail::QdPutCallParityEngine::calculatePutWithEdgeCases(
        Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const {

        if (close(K, 0.0))
            return 0.0;

        if (close(S, 0.0))
            return std::max(K, K*std::exp(-r*T));

        if (r <= 0.0 && r <= q)
            return std::max(0.0,
                BlackCalculator(Option::Put, K, S*std::exp((r-q)*T),
                                vol*std::sqrt(T), std::exp(-r*T)).value());

        if (close(vol, 0.0)) {
            const auto intrinsic = [&](Real t) {
                return std::max(0.0, K*std::exp(-r*t)-S*std::exp(-q*t));
            };
            const Real npv0 = intrinsic(0.0);
            const Real npvT = intrinsic(T);
            const Real extremT
                = close_enough(r, q)? QL_MAX_REAL : Real(std::log(r*K/(q*S))/(r-q));

             if (extremT > 0.0 && extremT < T)
                return std::max(npv0, std::max(npvT, intrinsic(extremT)));
            else
                return std::max(npv0, npvT);
        }

        return calculatePut(S, K, r, q, vol, T);
    }


    Real QdPlusAmericanEngine::xMax(Real K, Rate r, Rate q) {
        //Table 2 from Leif Andersen, Mark Lake (2021)
        //"Fast American Option Pricing: The Double-Boundary Case"

        if (r > 0.0 && q > 0.0)
            return K*std::min(1.0, r/q);
        else if (r > 0.0 && q <= 0.0)
            return K;
        else if (r == 0.0 && q < 0.0)
            return K;
        else if (r == 0.0 && q >= 0.0)
            return 0.0; // Eurpoean case
        else if (r < 0.0 && q >= 0.0)
            return 0.0; // European case
        else if (r < 0.0 && q < r)
            return K; // double boundary case
        else if (r < 0.0 && r <= q && q < 0.0)
            return 0; // European case
        else
            QL_FAIL("internal error");
    }

    QdPlusAmericanEngine::QdPlusAmericanEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Size interpolationPoints,
        QdPlusAmericanEngine::SolverType solverType,
        Real eps, Size maxIter)
    : detail::QdPutCallParityEngine(std::move(process)),
      interpolationPoints_(interpolationPoints),
      solverType_(solverType),
      eps_(eps),
      maxIter_((maxIter == Null<Size>()) ? (
          (solverType == Newton
           || solverType == Brent || solverType== Ridder)? 100 : 10)
          : maxIter ) { }

    template <class Solver>
    Real QdPlusAmericanEngine::buildInSolver(
        const QdPlusBoundaryEvaluator& eval,
        Solver solver, Real S, Real strike, Size maxIter, Real guess) const {

        solver.setMaxEvaluations(maxIter);
        solver.setLowerBound(eval.xmin());

        const Real fxmin = eval(eval.xmin());
        Real xmax = std::max(0.5*(eval.xmax() + S), eval.xmax());
        while (eval(xmax)*fxmin > 0.0 && eval.evaluations() < maxIter_)
            xmax*=2;

        if (guess == Null<Real>())
            guess = 0.5*(xmax + S);

        if (guess >= xmax)
            guess = std::nextafter(xmax, Real(-1));
        else if (guess <= eval.xmin())
            guess = std::nextafter(eval.xmin(), QL_MAX_REAL);

        return solver.solve(eval, eps_, guess, eval.xmin(), xmax);
    }

    std::pair<Size, Real> QdPlusAmericanEngine::putExerciseBoundaryAtTau(
            Real S, Real K, Rate r, Rate q,
            Volatility vol, Time T, Time tau) const {

        if (tau < QL_EPSILON)
            return std::pair<Size, Real>(
                Size(0), xMax(K, r, q));

        const QdPlusBoundaryEvaluator eval(S, K, r, q, vol, tau, T);

        Real x;
        switch (solverType_) {
          case Brent:
            x = buildInSolver(eval, QuantLib::Brent(), S, K, maxIter_);
            break;
          case Newton:
            x = buildInSolver(eval, QuantLib::Newton(), S, K, maxIter_);
            break;
          case Ridder:
            x = buildInSolver(eval, QuantLib::Ridder(), S, K, maxIter_);
            break;
          case Halley:
          case SuperHalley:
            {
                bool resultCloseEnough;
                x = eval.xmax();
                Real xOld, fx;
                const Real xmin = eval.xmin();

                do {
                    xOld = x;
                    fx = eval(x);
                    const Real fPrime = eval.derivative(x);
                    const Real lf = fx*eval.fprime2(x)/(fPrime*fPrime);
                    const Real step = (solverType_ == Halley)
                        ? Real(1/(1 - 0.5*lf)*fx/fPrime)
                        : Real((1 + 0.5*lf/(1-lf))*fx/fPrime);

                    x = std::max(xmin, x - step);
                    resultCloseEnough = std::fabs(x-xOld) < 0.5*eps_;
                }
                while (!resultCloseEnough && eval.evaluations() < maxIter_);

                if (!resultCloseEnough && !close(std::fabs(fx), 0.0)) {
                    x = buildInSolver(eval, QuantLib::Brent(), S, K, 10*maxIter_, x);
                }
            }
            break;
          default:
            QL_FAIL("unknown solver type");
        }

        return std::pair<Size, Real>(eval.evaluations(), x);
    }

    ext::shared_ptr<ChebyshevInterpolation>
        QdPlusAmericanEngine::getPutExerciseBoundary(
        Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const {

        const Real xmax = xMax(K, r, q);

        return ext::make_shared<ChebyshevInterpolation>(
            interpolationPoints_,
            [&, this](Real z) {
                const Real x_sq = 0.25*T*squared(1+z);
                return squared(std::log(
                    this->putExerciseBoundaryAtTau(S, K, r, q, vol, T, x_sq)
                        .second/xmax));
            },
            ChebyshevInterpolation::SecondKind
        );
    }

    Real QdPlusAmericanEngine::calculatePut(
        Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const {

        if (r < 0.0 && q < r)
            QL_FAIL("double-boundary case q<r<0 for a put option is given");

        const ext::shared_ptr<Interpolation> q_z
            = getPutExerciseBoundary(S, K, r, q, vol, T);

        const Real xmax = xMax(K, r, q);
        const detail::QdPlusAddOnValue aov(T, S, K, r, q, vol, xmax, q_z);

#ifdef QL_BOOST_HAS_TANH_SINH
        const Real addOn = TanhSinhIntegral(eps_)(aov, 0.0, std::sqrt(T));
#else
        const Real addOn = GaussLobattoIntegral(100000, QL_MAX_REAL, 0.1*eps_)
                (aov, 0.0, std::sqrt(T));
#endif

        QL_REQUIRE(addOn > -10*eps_,
            "negative early exercise value " << addOn);

        const Real europeanValue = std::max(
            0.0,
            BlackCalculator(
                Option::Put, K,
                S*std::exp((r-q)*T),
                vol*std::sqrt(T), std::exp(-r*T)).value()
        );

        return europeanValue + std::max(0.0, addOn);
    }
}
