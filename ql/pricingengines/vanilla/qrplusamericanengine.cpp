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
#include <ql/pricingengines/vanilla/qrplusamericanengine.hpp>

namespace QuantLib {


    class QrPlusBoundaryEvaluator {
      public:
        QrPlusBoundaryEvaluator(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process,
            Time t, Time T, Real strike)
        : tau(t),
          K(strike),
          sigma(process->blackVolatility()->blackVol(T, strike)),
          sigma2(sigma*sigma),
          v(sigma*std::sqrt(tau)),
          r(-std::log(process->riskFreeRate()->discount(T))/T),
          q(-std::log(process->dividendYield()->discount(T))/T),
          dr(std::exp(-r*tau)),
          dq(std::exp(-q*tau)),
          omega(2*(r-q)/sigma2),
          lambda(0.5*(-(omega-1)
                - std::sqrt(squared(omega - 1) + 8*r/(sigma2 * (1-dr))))),
          lambdaPrime(2*r/(sigma2*squared(1-dr)
                  *std::sqrt(squared(omega-1) + 8*r/(sigma2*(1-dr))))),
          alpha(2*dr*r/(sigma2*(2*lambda+omega - 1))),
          beta(alpha*(1/(1-dr)+lambdaPrime/(2*lambda+omega-1)) - lambda),
          xMin(0.5*(strike + process->x0())*1e3*QL_EPSILON),
          xMax(strike*std::min(1.0, r/ ((q != 0.0)? q : QL_EPSILON))),
          nrEvaluations(0),
          sc(Null<Real>()) {
        }

        Real operator()(Real S) const {
            ++nrEvaluations;
            S = std::max(xMin, S);
            if (S != sc)
                preCalculate(S);
            const Real c0 = -beta - lambda + alpha*theta/(dr*r*(K-S-npv));

            return (1-dq*Phi_dp)*S + (lambda+c0)*(K-S-npv);
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

    QrPlusAmericanEngine::QrPlusAmericanEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
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
    }

    template <class Solver>
    Real QrPlusAmericanEngine::buildInSolver(
        const QrPlusBoundaryEvaluator& eval,
        Solver solver, Real strike, Size maxIter) const {

        solver.setMaxEvaluations(maxIter);
        solver.setLowerBound(eval.xmin());

        Real guess = 0.5*(eval.xmax() + process_->x0());
        if (guess >= eval.xmax())
            guess = std::nextafter(eval.xmax(), -1);
        else if (guess <= eval.xmin())
            guess = std::nextafter(eval.xmin(), QL_MAX_REAL);

        return solver.solve(eval, eps_, guess, eval.xmin(), eval.xmax());
    }

    std::pair<Size, Real> QrPlusAmericanEngine::exerciseBoundary(
        Time tau, Time T, Real strike) const {

        const QrPlusBoundaryEvaluator eval(process_, tau, T, strike);

        if (tau < QL_EPSILON)
            return std::pair<Size, Real>(Size(0), eval.xmax());

        const Real xmin = eval.xmin();
        const Real x0 = process_->x0();

        Real x;
        switch (solverType_) {
          case Brent:
            x = buildInSolver(eval, QuantLib::Brent(), strike, maxIter_);
            break;
          case Newton:
            x = buildInSolver(eval, QuantLib::Newton(), strike, maxIter_);
            break;
          case Ridder:
            x = buildInSolver(eval, QuantLib::Ridder(), strike, maxIter_);
            break;
          case Halley:
          case SuperHalley:
            {
                x = 0.5*(eval.xmax() + x0);
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
                while (   std::fabs(x-xOld)/x0  > eps_
                       && eval.evaluations() < maxIter_);

                // fallback
                if (std::fabs(x-xOld)/x0 > eps_ && !close(std::fabs(fx), 0.0)) {
                    x = buildInSolver(eval, QuantLib::Brent(), strike, 10*maxIter_);
                }
            }
            break;
          default:
            QL_FAIL("unknown solver type");
        }

        return std::pair<Size, Real>(eval.evaluations(), x);
    }

    void QrPlusAmericanEngine::calculate() const {
        QL_REQUIRE(arguments_.exercise->type() == Exercise::American,
                   "not an American option");

        const auto payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        const Real spot = process_->x0();
        QL_REQUIRE(spot > 0.0, "negative underlying given");

        const Time T = process_->time(arguments_.exercise->lastDate());
        const Real K = payoff->strike();
        const Real xmax = QrPlusBoundaryEvaluator(process_, T, T, K).xmax();

        const auto interp = ChebyshevInterpolation(
            interpolationPoints_,
            [&T, &K, &xmax, this](Real x) {
                return squared(
                    std::log(exerciseBoundary(x*x, T, K).second / xmax));
            }
        );

    }
}
