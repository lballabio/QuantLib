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
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/interpolations/chebyshevinterpolation.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/qrplusamericanengine.hpp>
#include <ql/math/integrals/tanhsinhintegral.hpp>
#ifndef QL_BOOST_HAS_TANH_SINH
#include <ql/math/integrals/gausslobattointegral.hpp>
#endif

#include <iostream>

namespace QuantLib {

    class QrPlusBoundaryEvaluator {
      public:
        QrPlusBoundaryEvaluator(
            Real S, Real strike,
            Rate rf, Rate dy, Volatility vol, Time t, Time T)
        : tau(t),
          K(strike),
          sigma(vol),
          sigma2(sigma*sigma),
          v(sigma*std::sqrt(tau)),
          r(rf),
          q(dy),
          dr(std::exp(-r*tau)),
          dq(std::exp(-q*tau)),
          omega(2*(r-q)/sigma2),
          lambda(0.5*(-(omega-1)
                - std::sqrt(squared(omega - 1) + 8*r/(sigma2 * (1-dr))))),
          lambdaPrime(2*r/(sigma2*squared(1-dr)
                  *std::sqrt(squared(omega-1) + 8*r/(sigma2*(1-dr))))),
          alpha(2*dr*r/(sigma2*(2*lambda+omega - 1))),
          beta(alpha*(1/(1-dr)+lambdaPrime/(2*lambda+omega-1)) - lambda),
          xMin(0.5*(strike + S)*1e3*QL_EPSILON),
          xMax(calcxMax(strike, r, q)),
          nrEvaluations(0),
          sc(Null<Real>()) {
        }

        Real operator()(Real S) const {
            ++nrEvaluations;
            S = std::max(xMin, S);
            if (S != sc)
                preCalculate(S);

            if (close_enough(K-S, npv)) {
                return (1-dq*Phi_dp)*S + alpha*theta/(dr*r);
            }
            else {
                const Real c0 = -beta - lambda + alpha*theta/(dr*r*(K-S-npv));
                return (1-dq*Phi_dp)*S + (lambda+c0)*(K-S-npv);
            }
        }
        Real derivative(Real S) const {
            S = std::max(xMin, S);
            if (S != sc)
                preCalculate(S);

            return 1 - dq*Phi_dp + dq/v*phi_dp + beta*(1-dq*Phi_dp)
                    + alpha/(r*dr)*charm;
        }
        Real fprime2(Real S) const {
            S = std::max(xMin, S);
            if (S != sc)
                preCalculate(S);
            const Real gamma = phi_dp*dq/(v*S);
            const Real colour = gamma*(q + (r-q)*dp/v + (1-dp*dm)/(2*tau));

            return dq*(phi_dp/(S*v) - phi_dp*dp/(S*v*v))
                    + beta*gamma + alpha/(r*dr)*colour;
        }

        Real xmin() const { return xMin; }
        Real xmax() const { return xMax; }
        Size evaluations() const { return nrEvaluations; }

        static Real calcxMax(Real K, Rate r, Rate q) {
            return K*std::min(1.0, r/ ((q != 0.0)? q : QL_EPSILON));
        }

      private:
        void preCalculate(Real S) const {
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
        const DiscountFactor dr, dq;
        const Real omega, lambda, lambdaPrime, alpha, beta, xMin, xMax;
        mutable Size nrEvaluations;
        mutable Real sc, dp, dm, Phi_dp, Phi_dm, phi_dp;
        mutable Real npv, theta, charm;
    };


    class QrPlusAddOnValue {
      public:
        QrPlusAddOnValue(Time T,
                         Real S, Real K, Rate r, Rate q, Volatility vol,
                         const Real xmax,
                         const ChebyshevInterpolation& q_z)
        : T_(T), S_(S), K_(K), xmax_(xmax),
          r_(r), q_(q), vol_(vol), q_z_(q_z) {}

        Real operator()(Real z) const {
            const Real t = z*z;
            const Real q = q_z_(2*std::sqrt(std::max(0.0, T_-t)/T_) - 1);
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

       private:
         const Time T_;
         const Real S_, K_, xmax_;
         const Rate r_, q_;
         const Volatility vol_;
         const ChebyshevInterpolation& q_z_;
         const CumulativeNormalDistribution Phi_;
    };


    QrPlusAmericanEngine::QrPlusAmericanEngine(
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
        Size interpolationPoints,
        QrPlusAmericanEngine::SolverType solverType,
        Real eps, Size maxIter)
    : process_(std::move(process)),
      interpolationPoints_(interpolationPoints),
      solverType_(solverType),
      eps_(eps),
      maxIter_((maxIter == Null<Size>()) ? (
          (solverType == Newton
           || solverType == Brent || solverType== Ridder)? 100 : 10)
          : maxIter ) {

        registerWith(process);
    }

    template <class Solver>
    Real QrPlusAmericanEngine::buildInSolver(
        const QrPlusBoundaryEvaluator& eval,
        Solver solver, Real S, Real strike, Size maxIter) const {

        solver.setMaxEvaluations(maxIter);
        solver.setLowerBound(eval.xmin());

        Real guess = 0.5*(eval.xmax() + S);
        if (guess >= eval.xmax())
            guess = std::nextafter(eval.xmax(), -1);
        else if (guess <= eval.xmin())
            guess = std::nextafter(eval.xmin(), QL_MAX_REAL);

        return solver.solve(eval, eps_, guess, eval.xmin(), eval.xmax());
    }

    std::pair<Size, Real> QrPlusAmericanEngine::putExerciseBoundary(
        const PutOptionParam& param, Time tau) const {

        const Real S = param.S;
        const Real K = param.K;

        if (tau < QL_EPSILON)
            return std::pair<Size, Real>(
                Size(0), QrPlusBoundaryEvaluator::calcxMax(K, param.r, param.q));

        const QrPlusBoundaryEvaluator eval(
            S, K, param.r, param.q, param.vol, tau, param.T);

        const Real xmin = eval.xmin();

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
                x = 0.5*(eval.xmax() + S);
                Real xOld, fx;
                do {
                    xOld = x;
                    fx = eval(x);
                    const Real fPrime = eval.derivative(x);
                    const Real lf = fx*eval.fprime2(x)/(fPrime*fPrime);
                    Real step = (solverType_ == Halley)
                        ? 1/(1 - 0.5*lf)*fx/fPrime
                        : (1 + 0.5*lf/(1-lf))*fx/fPrime;

                    while ( (step > xmin) && (x - step < xmin) )
                        step *= 0.25;

                    x = std::max(xmin, x - step);
                }
                while (   std::fabs(x-xOld)/S  > eps_
                       && eval.evaluations() < maxIter_);

                // fallback
                if (std::fabs(x-xOld)/S > eps_ && !close(std::fabs(fx), 0.0)) {
                    x = buildInSolver(eval, QuantLib::Brent(), S, K, 10*maxIter_);
                }
            }
            break;
          default:
            QL_FAIL("unknown solver type");
        }

        return std::pair<Size, Real>(eval.evaluations(), x);
    }

    Real QrPlusAmericanEngine::calculate_put(
        Real S, Real K, Rate r, Rate q, Volatility vol, Time T) const {

        if (close(S, 0.0))
            return std::max(K, K*std::exp(-r*T));

        const Real europeanValue = std::max(
            0.0,
            BlackCalculator(
                Option::Put, K,
                S*std::exp((r-q)*T),
                vol*std::sqrt(T), std::exp(-r*T)).value()
        );

        if (r <= 0.0 && r <= q)
            return europeanValue;

        if (close(vol, 0.0)) {
            const auto intrinsic = [&](Real t) {
                return std::max(0.0, K*std::exp(-r*t)-S*std::exp(-q*t));
            };
            const Real npv0 = intrinsic(0.0);
            const Real npvT = intrinsic(T);
            const Real extremT
                = close_enough(r, q)? QL_MAX_REAL : std::log(r*K/(q*S))/(r-q);

             if (extremT > 0.0 && extremT < T)
                return std::max(npv0, std::max(npvT, intrinsic(extremT)));
            else
                return std::max(npv0, npvT);
        }

        QL_REQUIRE(r >= 0 && q >= 0,
            "positiive interest rates and dividend yields are required");

        const Real xmax = QrPlusBoundaryEvaluator::calcxMax(K, r, q);

        const ChebyshevInterpolation q_z(
            interpolationPoints_,
            [&, this](Real z) {
                const Real x_sq = 0.25*T*squared(1+z);
                const PutOptionParam param = {S, K, r, q, vol, T};
                return squared(
                    std::log(this->putExerciseBoundary(param, x_sq).second/xmax));
            }
        );

        const QrPlusAddOnValue aov(T, S, K, r, q, vol, xmax, q_z);

#ifdef QL_BOOST_HAS_TANH_SINH
        const Real addOn = TanhSinhIntegral(eps_)(aov, 0.0, std::sqrt(T));
#else
        const Real addOn = GaussLobattoIntegral(100*maxIter_, QL_MAX_REAL, eps_)
                (aov, 0.0, std::sqrt(T));
#endif

        QL_REQUIRE(addOn > -10*eps_,
            "negative early exercise value " << addOn);

        return europeanValue + std::max(0.0, addOn);
    }

    void QrPlusAmericanEngine::calculate() const {
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
            results_.value = calculate_put(S, K, r, q, vol, T);
        else if (payoff->optionType() == Option::Call)
            results_.value = calculate_put(K, S, q, r, vol, T);
        else
            QL_FAIL("unknown option type");
    }

}
