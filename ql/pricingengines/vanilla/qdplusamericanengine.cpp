/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Klaus Spanderen

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

/*! \file qrplusamericanengine.cpp
*/

#include <ql/exercise.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/integrals/tanhsinhintegral.hpp>
#include <ql/math/interpolations/chebyshevinterpolation.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/solvers1d/newton.hpp>
#include <ql/math/solvers1d/ridder.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/qdplusamericanengine.hpp>
#include <ql/utilities/null.hpp>
#include <algorithm>
#include <iostream>
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
          ddr((std::abs(r*tau) > 1e-5)? Real(r/(1-dr)) : Real(1/(tau*(1-0.5*r*tau*(1-r*tau/3))))),
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


    detail::QdPlusAddOnValue::QdPlusAddOnValue(Time T,
                                               Time tauTilde,
                                               Real S,
                                               Real K,
                                               Rate r,
                                               Rate q,
                                               Volatility vol,
                                               const Real xmax,
                                               ext::shared_ptr<Interpolation> q_z)
    : T_(T), tauTilde_(tauTilde), S_(S), K_(K), xmax_(xmax), r_(r), q_(q), vol_(vol),
      q_z_(std::move(q_z)) {}


    Real detail::QdPlusAddOnValue::operator()(Real z) const {
        const Real t = z*z;
        const Real q = (*q_z_)(2 * std::sqrt(std::max(0.0, T_ - t) / tauTilde_) - 1, true);
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


    Real detail::sigmaHat(Rate r, Rate q, Time tau) {
        if (tau < QL_EPSILON)
            return QL_MAX_REAL;
        const Real erT = std::exp(r * tau);
        const Real eqT = std::exp(q * tau);
        if (erT >= 1.0 || eqT >= 1.0)
            return QL_MAX_REAL;
        const InverseCumulativeNormal PhiInv;
        return std::abs(PhiInv(eqT) - PhiInv(erT)) / std::sqrt(tau);
    }

    detail::SigmaHatDerivatives detail::sigmaHatDerivatives(Rate r, Rate q, Time tau) {
        const Real sqrtTau = std::sqrt(tau);
        const Real erT = std::exp(r * tau);
        const Real eqT = std::exp(q * tau);
        const InverseCumulativeNormal PhiInv;
        const NormalDistribution phi;
        const Real gr = PhiInv(erT), gq = PhiInv(eqT);
        const Real N = gr - gq;
        const Real sign = (N > 0) ? 1.0 : -1.0;
        const Real phi_gr = phi(gr), phi_gq = phi(gq);
        const Real gr_tau = r * erT / phi_gr;
        const Real gq_tau = q * eqT / phi_gq;
        const Real N_tau = gr_tau - gq_tau;
        return {sign * (2 * tau * N_tau - N) / (2 * tau * sqrtTau),
                sign * tau * erT / (phi_gr * sqrtTau), -sign * tau * eqT / (phi_gq * sqrtTau)};
    }

    detail::TauHatSensitivities detail::computeTauHatSensitivities(Rate r, Rate q, Time tauHat) {
        const auto d = sigmaHatDerivatives(r, q, tauHat);
        const Real inv = 1.0 / d.dSigmaHat_dTau;
        return {inv, -d.dSigmaHat_dR * inv, -d.dSigmaHat_dQ * inv};
    }

    Time detail::computeTauHat(Rate r, Rate q, Volatility vol, Time T) {
        if (sigmaHat(r, q, T) > vol)
            return T;
        Brent solver;
        solver.setMaxEvaluations(100);
        return solver.solve([&](Real tau) { return sigmaHat(r, q, tau) - vol; }, 1e-10, 0.5 * T,
                            QL_EPSILON, T);
    }

    detail::QdAddOnSetup::QdAddOnSetup(Real z,
                                       Time T,
                                       Time tauTilde,
                                       Real S,
                                       Rate r,
                                       Rate q,
                                       Volatility vol,
                                       Real xmax,
                                       const Interpolation& q_z)
    : t(z * z), dr(std::exp(-r * t)), dq(std::exp(-q * t)), v(vol * std::sqrt(t)),
      b_t(xmax * std::exp(-std::sqrt(std::max(
                     0.0, q_z(2 * std::sqrt(std::max(0.0, T - t) / tauTilde) - 1, true))))),
      dp(0), dm(0), valid(v >= QL_EPSILON && b_t > QL_EPSILON) {
        if (valid) {
            dp = std::log(S * dq / (b_t * dr)) / v + 0.5 * v;
            dm = dp - v;
        }
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

        if (payoff->optionType() == Option::Put) {
            const auto res = calculatePutWithEdgeCases(S, K, r, q, vol, T);
            results_.value = res.value;
            results_.delta = res.delta;
            results_.gamma = res.gamma;
            results_.vega = res.vega;
            results_.rho = res.rho;
            results_.dividendRho = res.dividendRho;
            results_.theta = res.theta;
            results_.strikeSensitivity = res.strikeSensitivity;
        } else if (payoff->optionType() == Option::Call) {
            // Put-call symmetry: Call(S,K,r,q) = Put(K,S,q,r)
            // Put is computed with S_put=K, K_put=S, r_put=q, q_put=r
            const auto res = calculatePutWithEdgeCases(K, S, q, r, vol, T);
            results_.value = res.value;
            // delta_call = d/dS [Put(K,S,q,r)] = strikeSensitivity_put
            results_.delta = res.strikeSensitivity;
            // gamma_call = d^2/dS^2 [Put(K,S,q,r)] = strikeGamma_put
            results_.gamma = res.strikeGamma;
            // vega is symmetric
            results_.vega = res.vega;
            // rho_call: r appears as q_put -> rho_call = dividendRho_put
            results_.rho = res.dividendRho;
            // dividendRho_call: q appears as r_put -> dividendRho_call = rho_put
            results_.dividendRho = res.rho;
            // theta is symmetric
            results_.theta = res.theta;
            // strikeSensitivity_call: K appears as S_put -> delta_put
            results_.strikeSensitivity = res.delta;
        } else
            QL_FAIL("unknown option type");
    }

    detail::QdPutResults detail::QdPutCallParityEngine::calculatePutWithEdgeCases(
        Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const {

        QdPutResults res;

        if (close(K, 0.0))
            return res;

        if (close(S, 0.0)) {
            res.value = std::max(K, K * std::exp(-r * T));
            return res;
        }

        if (r <= 0.0 && r <= q) {
            const Real fwd = S * std::exp((r - q) * T);
            const Real stdDev = vol * std::sqrt(T);
            const Real df = std::exp(-r * T);
            BlackCalculator bc(Option::Put, K, fwd, stdDev, df);
            res.value = std::max(0.0, bc.value());
            if (res.value > 0.0) {
                res.delta = bc.delta(S);
                res.gamma = bc.gamma(S);
                res.vega = bc.vega(T);
                res.rho = bc.rho(T);
                res.dividendRho = bc.dividendRho(T);
                res.theta = bc.theta(S, T);
                res.strikeSensitivity = bc.strikeSensitivity();
                res.strikeGamma = bc.strikeGamma();
            }
            return res;
        }

        if (close(vol, 0.0)) {
            const auto intrinsic = [&](Real t)  -> Real {
                return std::max(0.0, K*std::exp(-r*t)-S*std::exp(-q*t));
            };
            const Real npv0 = intrinsic(0.0);
            const Real npvT = intrinsic(T);
            const Real extremT
                = close_enough(r, q)? QL_MAX_REAL : Real(std::log(r*K/(q*S))/(r-q));

            if (extremT > 0.0 && extremT < T)
                res.value = std::max({npv0, npvT, intrinsic(extremT)});
            else
                res.value = std::max(npv0, npvT);
            return res;
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
                    try {
                        x = buildInSolver(eval, QuantLib::Brent(), S, K, 10 * maxIter_, x);
                    } catch (...) {
                        // For double-boundary case (q<r<0) with large tau,
                        // boundary B->0 and solver may fail to bracket.
                        // Return small fraction of xmax to keep interpolation stable.
                        if (r < 0.0 && q < r)
                            x = 0.01 * eval.xmax();
                        else
                            throw;
                    }
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

    detail::QdPutResults QdPlusAmericanEngine::calculatePut(
        Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const {

        const bool doubleBoundary = (r < 0.0 && q < r);
        const Real tauHat = doubleBoundary ? detail::computeTauHat(r, q, vol, T) : 0.0;
        const Time tauTilde = doubleBoundary ? tauHat : T;

        detail::QdPutResults res;

        // --- Lower boundary B: computed over [0, tauTilde] ---
        const ext::shared_ptr<Interpolation> q_z =
            getPutExerciseBoundary(S, K, r, q, vol, tauTilde);

        const Real xmax = xMax(K, r, q);
        const detail::QdPlusAddOnValue aov(T, tauTilde, S, K, r, q, vol, xmax, q_z);

        const NormalDistribution phi;
        const CumulativeNormalDistribution Phi;

#ifdef QL_BOOST_HAS_TANH_SINH
        auto integrate = [&](const auto& f) {
            return TanhSinhIntegral(eps_)(f, std::sqrt(T - tauTilde), std::sqrt(T));
        };
#else
        auto integrate = [&](const auto& f) {
            return GaussLobattoIntegral(100000, QL_MAX_REAL, 0.1 * eps_)(f, std::sqrt(T - tauTilde),
                                                                         std::sqrt(T));
        };
#endif

        const Real addOn = integrate(aov);
        if (!doubleBoundary) {
            QL_REQUIRE(addOn > -10 * eps_, "negative early exercise value " << addOn);
        }

        // --- Upper boundary Y (double-boundary case) ---
        Real addOnY = 0.0;
        ext::shared_ptr<ChebyshevInterpolation> interpY;
        Real ymax = 0.0;

        if (doubleBoundary && tauHat > QL_EPSILON) {
            ymax = K * r / q;

            // Compute Y boundary via QdPlus at each Chebyshev node.
            // Y satisfies the same equation as B but converges to ymax < K.
            // Use getPutExerciseBoundary but seeded from ymax.
            // For simplicity, use a separate Chebyshev interpolation.
            interpY = ext::make_shared<ChebyshevInterpolation>(
                interpolationPoints_,
                [&](Real zz) -> Real {
                    const Real tau = 0.25 * tauHat * squared(1 + zz);
                    if (tau < QL_EPSILON)
                        return 0.0;

                    // Solve boundary equation from ymax side
                    const QdPlusBoundaryEvaluator eval(S, K, r, q, vol, tau, T);
                    // Find root near ymax (the upper boundary)
                    const Real xmin = QL_EPSILON * 1e4;
                    try {
                        QuantLib::Brent solver;
                        solver.setMaxEvaluations(100);
                        const Real yBound =
                            solver.solve([&](Real x) { return eval(x); }, eps_, ymax, xmin,
                                         std::min(ymax * 1.5, K * 0.99));
                        return squared(std::log(yBound / ymax));
                    } catch (...) {
                        return 0.0;
                    }
                },
                ChebyshevInterpolation::SecondKind);

            // Y add-on: integrate over calendar time t in [T-tauHat, T]
            const Real zYlo = std::sqrt(std::max(0.0, T - tauHat));
            const Real zYhi = std::sqrt(T);
            auto yAddOnIntegrand = [&](Real z) -> Real {
                const Real t = z * z;
                const Real tau = T - t;
                if (tau < QL_EPSILON || tau > tauHat + QL_EPSILON)
                    return 0.0;

                const Real zc = 2 * std::sqrt(std::min(tau, tauHat) / tauHat) - 1;
                const Real qv = (*interpY)(zc, true);
                const Real y_t = ymax * std::exp(-std::sqrt(std::max(0.0, qv)));

                const Real dr_ = std::exp(-r * t);
                const Real dq_ = std::exp(-q * t);
                const Real v_ = vol * std::sqrt(t);

                if (v_ < QL_EPSILON || y_t < QL_EPSILON)
                    return 0.0;

                const Real dp_ = std::log(S * dq_ / (y_t * dr_)) / v_ + 0.5 * v_;
                return 2 * z * (r * K * dr_ * Phi(-(dp_ - v_)) - q * S * dq_ * Phi(-dp_));
            };
#ifdef QL_BOOST_HAS_TANH_SINH
            addOnY = TanhSinhIntegral(eps_)(yAddOnIntegrand, zYlo, zYhi);
#else
            addOnY =
                GaussLobattoIntegral(100000, QL_MAX_REAL, 0.1 * eps_)(yAddOnIntegrand, zYlo, zYhi);
#endif
        }

        const Real fwd = S * std::exp((r - q) * T);
        const Real stdDev = vol * std::sqrt(T);
        const Real df = std::exp(-r * T);
        BlackCalculator bc(Option::Put, K, fwd, stdDev, df);

        // Pricing formula depends on S location (paper step 6):
        if (doubleBoundary && tauHat > QL_EPSILON) {
            if (S >= xmax) {
                // Eq (15): upper continuation region, only B
                res.value = std::max(0.0, bc.value()) + std::max(0.0, addOn);
            } else if (S <= ymax) {
                // Eq (16): lower continuation region
                res.value = std::max(K - S, std::max(0.0, K - S - addOnY));
            } else {
                // Exercise region or general case eq (12)
                const Real intrinsic = std::max(0.0, K - S);
                const Real eqn12 = std::max(0.0, bc.value()) + std::max(0.0, addOn - addOnY);
                res.value = std::max(intrinsic, eqn12);
            }
        } else {
            res.value = std::max(0.0, bc.value()) + std::max(0.0, addOn - addOnY);
        }

        // Delta add-on (B boundary)
        const Real deltaAddOn = integrate([&](Real z) -> Real {
            const detail::QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *q_z);
            if (!s.valid)
                return 0.0;
            return 2 * z *
                   (-r * K * s.dr * phi(s.dm) / (S * s.v) - q * s.dq * Phi(-s.dp) +
                    q * s.dq * phi(s.dp) / s.v);
        });
        res.delta = bc.delta(S) + deltaAddOn;

        // Gamma add-on (B boundary)
        const Real gammaAddOn = integrate([&](Real z) -> Real {
            const detail::QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *q_z);
            if (!s.valid)
                return 0.0;
            return 2 * z *
                   (r * K * s.dr * phi(s.dm) * s.dp / (S * S * s.v * s.v) -
                    q * s.dq * phi(s.dp) * s.dm / (S * s.v * s.v));
        });
        res.gamma = bc.gamma(S) + gammaAddOn;

        // --- Y boundary contributions to delta, gamma (double-boundary) ---
        auto ySetup = [&](Real z) -> std::tuple<bool, Real, Real, Real, Real, Real, Real, Real> {
            const Real t = z * z;
            const Real tau = T - t;
            if (tau < QL_EPSILON || tau > tauHat + QL_EPSILON)
                return {false, 0, 0, 0, 0, 0, 0, 0};
            const Real zc = 2 * std::sqrt(std::min(tau, tauHat) / tauHat) - 1;
            const Real qv = (*interpY)(zc, true);
            const Real y_t = ymax * std::exp(-std::sqrt(std::max(0.0, qv)));
            const Real dr_ = std::exp(-r * t);
            const Real dq_ = std::exp(-q * t);
            const Real v_ = vol * std::sqrt(t);
            if (v_ < QL_EPSILON || y_t < QL_EPSILON)
                return {false, 0, 0, 0, 0, 0, 0, 0};
            const Real dp_ = std::log(S * dq_ / (y_t * dr_)) / v_ + 0.5 * v_;
            const Real dm_ = dp_ - v_;
            return {true, t, dr_, dq_, v_, y_t, dp_, dm_};
        };

        const Real zYlo = std::sqrt(std::max(0.0, T - tauHat));
        const Real zYhi = std::sqrt(T);
        auto integrateY = [&](const auto& f) -> Real {
            if (!doubleBoundary || tauHat <= QL_EPSILON)
                return 0.0;
#ifdef QL_BOOST_HAS_TANH_SINH
            return TanhSinhIntegral(eps_)(f, zYlo, zYhi);
#else
            return GaussLobattoIntegral(100000, QL_MAX_REAL, 0.1 * eps_)(f, zYlo, zYhi);
#endif
        };

        if (doubleBoundary && tauHat > QL_EPSILON) {
            const Real deltaYAddOn = integrateY([&](Real z) -> Real {
                const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = ySetup(z);
                if (!valid)
                    return 0.0;
                return 2 * z *
                       (-r * K * dr_ * phi(dm_) / (S * v_) - q * dq_ * Phi(-dp_) +
                        q * dq_ * phi(dp_) / v_);
            });
            res.delta -= deltaYAddOn;

            const Real gammaYAddOn = integrateY([&](Real z) -> Real {
                const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = ySetup(z);
                if (!valid)
                    return 0.0;
                return 2 * z *
                       (r * K * dr_ * phi(dm_) * dp_ / (S * S * v_ * v_) -
                        q * dq_ * phi(dp_) * dm_ / (S * v_ * v_));
            });
            res.gamma -= gammaYAddOn;
        }

        // Analytical vega, rho, dividendRho via implicit function theorem.
        // The QdPlus boundary solves eval(B;p) = 0 at each tau.
        // By the IFT: dB/dp = -(deval/dp) / (deval/dB)
        {
            const Real eps_sigma = vol * 1e-6;
            const Real eps_r = std::max(std::abs(r), 1.0) * 1e-6;
            const Real eps_q = std::max(std::abs(q), 1.0) * 1e-6;

            // Build Chebyshev interpolations of dB/dp at each node
            auto makeBoundarySensInterp =
                [&](Real dr_, Real dq_, Real dvol_) -> ext::shared_ptr<ChebyshevInterpolation> {
                const Real bump = (dvol_ != 0.0) ? dvol_ : ((dr_ != 0.0) ? dr_ : dq_);
                return ext::make_shared<ChebyshevInterpolation>(
                    interpolationPoints_,
                    [&, dr_, dq_, dvol_, bump](Real z) -> Real {
                        const Real tau = 0.25 * tauTilde * squared(1 + z);
                        if (tau < QL_EPSILON)
                            return 0.0;

                        const Real b = xmax * std::exp(-std::sqrt(std::max(0.0, (*q_z)(z, true))));

                        const QdPlusBoundaryEvaluator eval0(S, K, r, q, vol, tau, T);
                        const Real evalDeriv = eval0.derivative(b);
                        if (std::abs(evalDeriv) < QL_EPSILON)
                            return 0.0;

                        const QdPlusBoundaryEvaluator evalBump(S, K, r + dr_, q + dq_, vol + dvol_,
                                                               tau, T);
                        const Real dEval = evalBump(b) / bump;

                        return -dEval / evalDeriv;
                    },
                    ChebyshevInterpolation::SecondKind);
            };

            const auto si_sigma = makeBoundarySensInterp(0, 0, eps_sigma);
            const auto si_r = makeBoundarySensInterp(eps_r, 0, 0);
            const auto si_q = makeBoundarySensInterp(0, eps_q, 0);

            // df/dB = 2z/(B*v) * [rK*dr*phi(dm) - qS*dq*phi(dp)]
            auto dfdb = [&](Real z, const detail::QdAddOnSetup& s) -> Real {
                return 2 * z / (s.b_t * s.v) *
                       (r * K * s.dr * phi(s.dm) - q * S * s.dq * phi(s.dp));
            };

            // Lookup dB/dp from interpolation
            auto lookupDBdp = [&](const detail::QdAddOnSetup& s,
                                  const ChebyshevInterpolation& si) -> Real {
                const Real zc = 2 * std::sqrt(std::max(0.0, T - s.t) / tauTilde) - 1;
                return si(zc, true);
            };

            // Vega: df/dsigma|_B + df/dB * dB/dsigma
            const Real vegaAddOn = integrate([&](Real z) -> Real {
                const detail::QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *q_z);
                if (!s.valid)
                    return 0.0;
                const Real partial = (2 * z / vol) * (r * K * s.dr * phi(s.dm) * s.dp -
                                                      q * S * s.dq * phi(s.dp) * s.dm);
                return partial + dfdb(z, s) * lookupDBdp(s, *si_sigma);
            });
            res.vega = bc.vega(T) + vegaAddOn;

            // Rho: df/dr|_B + df/dB * dB/dr
            const Real rhoAddOn = integrate([&](Real z) -> Real {
                const detail::QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *q_z);
                if (!s.valid)
                    return 0.0;
                const Real partial =
                    2 * z *
                    (K * s.dr * Phi(-s.dm) * (1 - r * s.t) -
                     (s.t / s.v) * (r * K * s.dr * phi(s.dm) - q * S * s.dq * phi(s.dp)));
                return partial + dfdb(z, s) * lookupDBdp(s, *si_r);
            });
            res.rho = bc.rho(T) + rhoAddOn;

            // DividendRho: df/dq|_B + df/dB * dB/dq
            const Real divRhoAddOn = integrate([&](Real z) -> Real {
                const detail::QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *q_z);
                if (!s.valid)
                    return 0.0;
                const Real partial =
                    2 * z *
                    (r * K * s.dr * phi(s.dm) * s.t / s.v - S * s.dq * Phi(-s.dp) * (1 - q * s.t) -
                     q * S * s.dq * phi(s.dp) * s.t / s.v);
                return partial + dfdb(z, s) * lookupDBdp(s, *si_q);
            });
            res.dividendRho = bc.dividendRho(T) + divRhoAddOn;

            // --- Y boundary contributions to vega, rho, divRho (double-boundary) ---
            if (doubleBoundary && tauHat > QL_EPSILON && interpY) {
                // Build Y boundary sensitivity interpolations via IFT
                auto makeYSensInterp = [&](Real dr_, Real dq_,
                                           Real dvol_) -> ext::shared_ptr<ChebyshevInterpolation> {
                    const Real bump = (dvol_ != 0.0) ? dvol_ : ((dr_ != 0.0) ? dr_ : dq_);
                    return ext::make_shared<ChebyshevInterpolation>(
                        interpolationPoints_,
                        [&, dr_, dq_, dvol_, bump](Real zz) -> Real {
                            const Real tau = 0.25 * tauHat * squared(1 + zz);
                            if (tau < QL_EPSILON)
                                return 0.0;

                            const Real zc = 2 * std::sqrt(std::min(tau, tauHat) / tauHat) - 1;
                            const Real qv = (*interpY)(zc, true);
                            const Real y = ymax * std::exp(-std::sqrt(std::max(0.0, qv)));

                            const QdPlusBoundaryEvaluator eval0(S, K, r, q, vol, tau, T);
                            const Real evalDeriv = eval0.derivative(y);
                            if (std::abs(evalDeriv) < QL_EPSILON)
                                return 0.0;

                            const QdPlusBoundaryEvaluator evalBump(S, K, r + dr_, q + dq_,
                                                                   vol + dvol_, tau, T);
                            const Real dEval = evalBump(y) / bump;

                            return -dEval / evalDeriv;
                        },
                        ChebyshevInterpolation::SecondKind);
                };

                const auto siY_sigma = makeYSensInterp(0, 0, eps_sigma);
                const auto siY_r = makeYSensInterp(eps_r, 0, 0);
                const auto siY_q = makeYSensInterp(0, eps_q, 0);

                auto lookupDYdp = [&](Real t_val, const ChebyshevInterpolation& si) -> Real {
                    const Real tau = T - t_val;
                    const Real zc =
                        2 * std::sqrt(std::max(0.0, std::min(tau, tauHat)) / tauHat) - 1;
                    return si(zc, true);
                };

                // Subtract Y vega contribution
                const Real vegaYAddOn = integrateY([&](Real z) -> Real {
                    const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = ySetup(z);
                    if (!valid)
                        return 0.0;
                    const Real partial = (2 * z / vol) * (r * K * dr_ * phi(dm_) * dp_ -
                                                          q * S * dq_ * phi(dp_) * dm_);
                    const Real dYds = lookupDYdp(t, *siY_sigma);
                    const Real ddp_ds = -dm_ / vol - dYds / (y_t * v_);
                    const Real ddm_ds = -dp_ / vol - dYds / (y_t * v_);
                    return partial +
                           2 * z *
                               (-r * K * dr_ * phi(dm_) * ddm_ds + q * S * dq_ * phi(dp_) * ddp_ds);
                });
                res.vega -= vegaYAddOn;

                // Subtract Y rho contribution
                const Real rhoYAddOn = integrateY([&](Real z) -> Real {
                    const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = ySetup(z);
                    if (!valid)
                        return 0.0;
                    const Real dYdr = lookupDYdp(t, *siY_r);
                    const Real ddp_dr = t / v_ - dYdr / (y_t * v_);
                    return 2 * z *
                           ((1 - r * t) * K * dr_ * Phi(-dm_) - r * K * dr_ * phi(dm_) * ddp_dr +
                            q * S * dq_ * phi(dp_) * ddp_dr);
                });
                res.rho -= rhoYAddOn;

                // Subtract Y divRho contribution
                const Real divRhoYAddOn = integrateY([&](Real z) -> Real {
                    const auto [valid, t, dr_, dq_, v_, y_t, dp_, dm_] = ySetup(z);
                    if (!valid)
                        return 0.0;
                    const Real dYdq = lookupDYdp(t, *siY_q);
                    const Real ddp_dq = -t / v_ - dYdq / (y_t * v_);
                    return 2 * z *
                           (-r * K * dr_ * phi(dm_) * ddp_dq - (1 - q * t) * S * dq_ * Phi(-dp_) +
                            q * S * dq_ * phi(dp_) * ddp_dq);
                });
                res.dividendRho -= divRhoYAddOn;
            }

            // --- Leibniz boundary correction for vega/rho/divRho ---
            // When tauHat < T, the integration lower limit z0 = sqrt(T-tauHat) depends on
            // sigma, r, q through tauHat. By Leibniz rule: correction =
            // [f_B(z0)-f_Y(z0)]*dtauHat/dp/(2z0)
            if (doubleBoundary && tauTilde < T - QL_EPSILON) {
                const Real z0 = std::sqrt(T - tauHat);
                const Real fB_z0 = aov(z0);

                // Evaluate Y integrand at z0
                Real fY_z0 = 0.0;
                if (interpY) {
                    const Real t0 = z0 * z0;
                    const Real tau0 = T - t0;
                    const Real zc = 2 * std::sqrt(std::min(tau0, tauHat) / tauHat) - 1;
                    const Real qv = (*interpY)(zc, true);
                    const Real y0 = ymax * std::exp(-std::sqrt(std::max(0.0, qv)));
                    const Real dr0 = std::exp(-r * t0);
                    const Real dq0 = std::exp(-q * t0);
                    const Real v0 = vol * std::sqrt(t0);
                    if (v0 >= QL_EPSILON && y0 > QL_EPSILON) {
                        const Real dp0 = std::log(S * dq0 / (y0 * dr0)) / v0 + 0.5 * v0;
                        fY_z0 = 2 * z0 * (r * K * dr0 * Phi(-(dp0 - v0)) - q * S * dq0 * Phi(-dp0));
                    }
                }

                const Real fNet = fB_z0 - fY_z0;
                if (std::abs(fNet) > QL_EPSILON) {
                    const auto ths = detail::computeTauHatSensitivities(r, q, tauHat);
                    const Real leibnizFactor = fNet / (2 * z0);
                    res.vega += leibnizFactor * ths.dTauHat_dSigma;
                    res.rho += leibnizFactor * ths.dTauHat_dR;
                    res.dividendRho += leibnizFactor * ths.dTauHat_dQ;
                }
            }
        }

        // StrikeSensitivity add-on: total d/dK, using B proportional to K
        // dp = [log(S/K) - log(g) + (r-q)t]/v + v/2, so ddp/dK = -1/(Kv)
        const Real strikeSensAddOn = integrate([&](Real z) -> Real {
            const detail::QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *q_z);
            if (!s.valid)
                return 0.0;
            return 2 * z *
                   (r * s.dr * (Phi(-s.dm) + phi(s.dm) / s.v) -
                    q * S * s.dq * phi(s.dp) / (K * s.v));
        });
        res.strikeSensitivity = bc.strikeSensitivity() + strikeSensAddOn;

        // StrikeGamma add-on: d^2f/dK^2
        const Real strikeGammaAddOn = integrate([&](Real z) -> Real {
            const detail::QdAddOnSetup s(z, T, tauTilde, S, r, q, vol, xmax, *q_z);
            if (!s.valid)
                return 0.0;
            return 2 * z *
                   (r * s.dr * phi(s.dm) * s.dp / (K * s.v * s.v) -
                    q * S * s.dq * phi(s.dp) * s.dm / (K * K * s.v * s.v));
        });
        res.strikeGamma = bc.strikeGamma() + strikeGammaAddOn;

        // Theta: Leibniz rule -> f(sqrt(T)) / (2*sqrt(T))
        const Real sqrtT = std::sqrt(T);
        res.theta = bc.theta(S, T) + aov(sqrtT) / (2 * sqrtT);

        return res;
    }
}
