/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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
#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/math/matrixutilities/choleskydecomposition.hpp>
#include "ql/pricingengines/basket/denglizhouspreadengine.hpp"

#include <iostream>

namespace QuantLib {

    DengLiZhouSpreadEngine::DengLiZhouSpreadEngine(
        std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
        Matrix rho)
    : processes_(std::move(processes)),
      rho_(std::move(rho)) {

        QL_REQUIRE(!processes_.empty(), "No Black-Scholes process is given.");
        QL_REQUIRE(processes_.size() == rho_.size1() && rho_.size1() == rho_.size2(),
            "process and correlation matrix must have the same size.");

        for (Size i=0; i < processes_.size(); ++i)
            registerWith(processes_[i]);
    }

    void DengLiZhouSpreadEngine::calculate() const {
        const ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not an European exercise");

        const ext::shared_ptr<AverageBasketPayoff> avgPayoff =
            ext::dynamic_pointer_cast<AverageBasketPayoff>(arguments_.payoff);
        QL_REQUIRE(avgPayoff, " average basket payoff expected");

        const ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(avgPayoff->basePayoff());
        QL_REQUIRE(payoff, "non-plain vanilla payoff given");

        const Real strike = payoff->strike();
        const Option::Type optionType = payoff->optionType();

        const Date maturityDate = exercise->lastDate();
        const Time T = processes_[0]->time(maturityDate);

        const auto extractProcesses =
            [this](const std::function<Real(const decltype(processes_)::value_type&)>& f)
                -> Array {

                Array x(processes_.size());
                std::transform(processes_.begin(), processes_.end(), x.begin(), f);

                return x;
            };

        const Array dr = extractProcesses(
            [maturityDate](const auto& p) -> DiscountFactor {
                return p->riskFreeRate()->discount(maturityDate);
            }
        );

        QL_REQUIRE(
            std::equal(
                dr.begin()+1, dr.end(), dr.begin(),
                std::pointer_to_binary_function<Real, Real, bool>(close_enough)
            ),
            "interest rates need to be the same for all underlyings"
        );

        const Array s = extractProcesses([](const auto& p) -> Real { return p->x0(); });

        const Array dq = extractProcesses(
            [maturityDate](const auto& p) -> DiscountFactor {
                return p->dividendYield()->discount(maturityDate);
            }
        );
        const Array v = extractProcesses(
            [maturityDate](const auto& p) -> Volatility {
                return p->blackVolatility()->blackVariance(maturityDate, p->x0());
            }
        );

        results_.value =
            DengLiZhouSpreadEngine::calculate_vanilla_call(Log(s), dr, dq, v, rho_, strike);
    }

    Real DengLiZhouSpreadEngine::calculate_vanilla_call(
        const Array& x, const Array& dr, const Array& dq,
        const Array& v, const Matrix& rho, Real K) {

        const Array mu = x + Log(dq/dr[0]) - 0.5 * v;
        const Array nu = Sqrt(v);

        const Real R = std::accumulate(
            mu.begin()+1, mu.end(), Real(0),
            [](Real a, Real b) -> Real { return a + std::exp(b); }
        );

        const Size N = x.size()-1;

        Matrix sig11(N, N);
        for (Size i=0; i < N; ++i)
            std::copy(rho.row_begin(i+1)+1, rho.row_end(i+1), sig11.row_begin(i));
        const Array sig10(rho.row_begin(0)+1, rho.row_end(0));

        const Matrix sqSig11 = pseudoSqrt(sig11, SalvagingAlgorithm::Principal);

        const Array sig11Inv10 = CholeskySolveFor(CholeskyDecomposition(sig11), sig10);

        const Real sqSig_xy = std::sqrt(1.0 - DotProduct(sig10, sig11Inv10));

        const Real a = -0.5/sqSig_xy;
        Matrix E(N, N);
        for (Size i=1; i <= N; ++i)
            for (Size j=i; j <= N; ++j)
                E(i-1, j-1) = E(j-1, i-1) =
                     a*(((i==j)? squared(nu[j])*std::exp(mu[j])/(nu[0]*(R+K)) : 0.0)
                        -nu[i]*nu[j]*std::exp(mu[i]+mu[j])/(nu[0]*squared(R + K)) );

        const Matrix F = sqSig11*E*sqSig11;

        Real trF(0), trF2(0);
        for (Size i=0; i < N; ++i) {
            trF += F[i][i];
            trF2 += squared(F[i][i]) +
                 2.0*std::accumulate(
                     F.row_begin(i)+i+1, F.row_end(i), Real(0),
                     [](Real a, Real b) -> Real {return a+b*b;}
                 );
        }

        const Real c = -(std::log(R + K) - mu[0])/(nu[0]*sqSig_xy);

        const Array d = (sig11Inv10
            - Exp(Array(mu.begin()+1, mu.end()))*Array(nu.begin()+1,nu.end())/(nu[0]*(R+K)))/sqSig_xy;

        const Array Esig10 = E*sig10;
        const Matrix Esig11 = E*sig11;
        const Array sig11d = sig11*d;

        Array C(N+2);
        C[0] = c + trF + nu[0]*sqSig_xy + nu[0]*DotProduct(sig10, d)
             + squared(nu[0])*DotProduct(sig10, Esig10);
        C[N+1] = c + trF;

        for (Size k=1; k < N+1; ++k)
            C[k] = c + trF + nu[k]*sig11d[k-1] + squared(nu[k])
                * std::inner_product(sig11.row_begin(k-1), sig11.row_end(k-1),
                                     Esig11.column_begin(k-1), 0.0);

        std::vector<Array> D(N+2);
        D[0] = sqSig11*(d + 2*nu[0]*Esig10);
        D[N+1] = sqSig11*d;
        for (Size k=1; k < N+1; ++k)
            D[k] = sqSig11*(d + 2*nu[k]*Array(Esig11.column_begin(k-1), Esig11.column_end(k-1)));

        std::cout << std::setprecision(16);
        for (Size i=0; i < N+2; ++i)
            std::cout << D[i] << std::endl;

        return 1.0;
    }
}
