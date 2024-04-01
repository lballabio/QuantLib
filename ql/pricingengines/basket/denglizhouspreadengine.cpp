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
                for (Size i=0; i < x.size(); ++i)
                    x[i] = f(processes_[i]);

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
            DengLiZhouSpreadEngine::calculate_vanilla_call(s, dr, dq, v, rho_, strike);
    }

    Real DengLiZhouSpreadEngine::calculate_vanilla_call(
        const Array& s, const Array& dr, const Array& dq,
        const Array& v, const Matrix& rho, Real K) {

        const Array mu = Log(s*dq/dr[0]) - 0.5 * v;
        const Array nu = Sqrt(v);

        const Real R = std::accumulate(
            mu.begin()+1, mu.end(), Real(0),
            [](Real a, Real b) -> Real { return a + std::exp(b); }
        );

        const Size N = s.size()-1;

        Matrix sig11(N, N);
        for (Size i=0; i < N; ++i)
            std::copy(rho.row_begin(i+1)+1, rho.row_end(i+1), sig11.row_begin(i));

        const Matrix c = CholeskyDecomposition(sig11, true);


        const Matrix sq_sig11 = pseudoSqrt(sig11, SalvagingAlgorithm::Principal);

        //const Real a = -0.5/std::sqrt( sig11)
        Matrix E(N, N);
        for (Size i=1; i <= N; ++i)
            for (Size j=i; j <= N; ++j)
                E(i-1, j-1) = E(j-1, i-1) =
                    - nu[i]*nu[j]*std::exp(mu[i]+mu[j])/(nu[0]*squared(R + K))
                    + ((i==j)? squared(nu[j])*std::exp(mu[j])/(nu[0]*(R+K)) : 0.0);

        std::cout << std::setprecision(16) << E << std::endl
                << sq_sig11 << std::endl;
        return 1.0;
    }
}
