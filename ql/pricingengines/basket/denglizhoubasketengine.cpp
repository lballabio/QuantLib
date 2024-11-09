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
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/math/matrixutilities/choleskydecomposition.hpp>
#include <ql/pricingengines/basket/vectorbsmprocessextractor.hpp>
#include <ql/pricingengines/basket/denglizhoubasketengine.hpp>

namespace QuantLib {

    DengLiZhouBasketEngine::DengLiZhouBasketEngine(
        std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
        Matrix rho)
    : n_(processes.size()),
      processes_(std::move(processes)),
      rho_(std::move(rho)) {

        QL_REQUIRE(n_ > 0, "No Black-Scholes process is given.");
        QL_REQUIRE(n_ == rho_.size1() && rho_.size1() == rho_.size2(),
            "process and correlation matrix must have the same size.");

        std::for_each(processes_.begin(), processes_.end(),
            [this](const auto& p) { registerWith(p); });
    }

    void DengLiZhouBasketEngine::calculate() const {
        const ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not an European exercise");
        const Date maturityDate = exercise->lastDate();

        const ext::shared_ptr<AverageBasketPayoff> avgPayoff =
            (ext::dynamic_pointer_cast<AverageBasketPayoff>(arguments_.payoff) != nullptr)
            ? ext::dynamic_pointer_cast<AverageBasketPayoff>(arguments_.payoff)
            : (ext::dynamic_pointer_cast<SpreadBasketPayoff>(arguments_.payoff) != nullptr)
                ? ext::make_shared<AverageBasketPayoff>(
                    ext::dynamic_pointer_cast<SpreadBasketPayoff>(
                        arguments_.payoff)->basePayoff(),
                    Array({1.0, -1.0})
                  )
                : ext::shared_ptr<AverageBasketPayoff>();

        QL_REQUIRE(avgPayoff, "average or spread basket payoff expected");

        const Array weights = avgPayoff->weights();
        QL_REQUIRE(n_ == weights.size() && n_ > 1,
             "wrong number of weights arguments in payoff");

        const detail::VectorBsmProcessExtractor pExtractor(processes_);
        const Array s = pExtractor.getSpot();
        const Array dq = pExtractor.getDividendYieldDf(maturityDate);
        const Array v = pExtractor.getBlackVariance(maturityDate);
        const DiscountFactor dr0 = pExtractor.getInterestRateDf(maturityDate);

        std::vector< std::tuple<Real, Size, Real, Real, Real> > p;
        p.reserve(n_);

        for (Size i=0; i < n_; ++i)
            p.emplace_back(std::make_tuple(weights[i], i, s[i], dq[i], v[i]));

        const ext::shared_ptr<PlainVanillaPayoff> payoff =
             ext::dynamic_pointer_cast<PlainVanillaPayoff>(avgPayoff->basePayoff());
        QL_REQUIRE(payoff, "non-plain vanilla payoff given");

        Matrix rho;
        if (payoff->strike() < 0.0) {
            p.emplace_back(std::make_tuple(1.0, n_, -payoff->strike(), dr0, 0.0));
            rho = Matrix(n_+1, n_+1);
            for (Size i=0; i < n_; ++i) {
                std::copy(rho_.row_begin(i), rho_.row_end(i), rho.row_begin(i));
                rho[n_][i] = rho[i][n_] = 0.0;
            }
            rho[n_][n_] = 1.0;
        }
        else
            rho = rho_;

        const Real strike = std::max(0.0, payoff->strike());

        // positive weights first
        std::sort(p.begin(), p.end(), std::greater<>());

        const Size M = std::distance(
            p.begin(),
            std::lower_bound(p.begin(), p.end(), Real(0),
                [](const auto& p, const Real& value) -> bool { return std::get<0>(p) > value;}            )
        );

        QL_REQUIRE(M > 0, "at least one positive asset weight must be given");
        QL_REQUIRE(M < p.size(), "at least one negative asset weight must be given");

        const Size N = p.size() - M;

        Matrix nRho(N+1, N+1);
        Array _s(N+1), _dq(N+1), _v(N+1);

        if (M > 1) {
            Array F(M), vol(M);
            for (Size i=0; i < M; ++i) {
                vol[i] = std::sqrt(std::get<4>(p[i]));
                F[i] = std::get<0>(p[i])*std::get<2>(p[i])*std::get<3>(p[i])/dr0;
            }

            const Real S0 = std::accumulate(
                p.begin(), p.begin()+M, Real(0.0),
                [](const Real& value, const auto& p) -> Real {
                    return value + std::get<0>(p)*std::get<2>(p);
            });
            const Real F0 = std::accumulate(F.begin(), F.end(), 0.0);
            const DiscountFactor dq_S0 = F0/S0*dr0;

            Real v_s = 0.0;
            for (Size i=0; i < M; ++i)
                for (Size j=0; j < M; ++j)
                    v_s += vol[i]*vol[j]*F[i]*F[j]
                        *rho[std::get<1>(p[i])][std::get<1>(p[j])];

            v_s /= F0*F0;
            _s[0] = S0; _dq[0] = dq_S0; _v[0] = v_s;

            nRho[0][0] = 1.0;

            for (Size i=0; i < N; ++i) {
                Real rhoHat = 0.0;
                for (Size j=0; j < M; ++j)
                    rhoHat += rho[std::get<1>(p[M+i])][std::get<1>(p[j])]*vol[j]*F[j];

                nRho[i+1][0] = nRho[0][i+1]
                      = std::min(1.0, std::max(-1.0, rhoHat/(std::sqrt(v_s)*F0)));
            }
        }
        else {
            _s[0]  = std::abs(std::get<0>(p[0])*std::get<2>(p[0]));
            _dq[0] = std::get<3>(p[0]);
            _v[0]  = std::get<4>(p[0]);
            for (Size i=0; i < N+1; ++i)
                nRho[0][i] = nRho[i][0] = rho[std::get<1>(p[i])][std::get<1>(p[0])];
        }

        for (Size i=0; i < N; ++i) {
            _s[i+1]  = std::abs(std::get<0>(p[M+i])*std::get<2>(p[M+i]));
            _dq[i+1] = std::get<3>(p[M+i]);
            _v[i+1]  = std::get<4>(p[M+i]);

            const Size idx = std::get<1>(p[M+i]);
            for (Size j=0; j < N; ++j)
                nRho[i+1][j+1] = rho[idx][std::get<1>(p[M+j])];
        }

        const Real callValue
            = DengLiZhouBasketEngine::calculate_vanilla_call(Log(_s), dr0, _dq, _v, nRho, strike);

        if (payoff->optionType() == Option::Call)
            results_.value = std::max(0.0, callValue);
        else {
            const Real fwd = _s[0]*_dq[0] - dr0*strike
                - std::inner_product(_s.begin()+1, _s.end(), _dq.begin()+1, 0.0);
            results_.value = std::max(0.0, callValue - fwd);
        }
    }

    Real DengLiZhouBasketEngine::I(Real u, Real tF2, const Matrix& D, const Matrix& DF, Size i) {
        const Real psi = 1.0/
            (1.0 + std::inner_product(
                 D.row_begin(i), D.row_end(i), D.row_begin(i), 0.0));
        const Real sqrtPsi = std::sqrt(psi);

        const Real n_uSqrtPsi = NormalDistribution()(u*sqrtPsi);
        const Real J_0 = CumulativeNormalDistribution()(u*sqrtPsi);

        const Real vFv = std::inner_product(
            DF.row_begin(i), DF.row_end(i), D.row_begin(i), 0.0);
        const Real J_1 = psi*sqrtPsi*(psi*u*u - 1.0) * vFv * n_uSqrtPsi;

        const Real vFFv = std::inner_product(
            DF.row_begin(i), DF.row_end(i), DF.row_begin(i), 0.0);
        const Real J_2 = u*psi*sqrtPsi*n_uSqrtPsi*(
                2 * tF2
                + vFv*vFv*(squared(squared(psi*u))
                           - 10.0*psi*psi*psi*u*u + 15*psi*psi)
                + vFFv * (4*psi*psi*u*u - 12*psi)
        );

        return J_0 + J_1 - 0.5*J_2;
    }

    Real DengLiZhouBasketEngine::calculate_vanilla_call(
        const Array& x, DiscountFactor dr, const Array& dq,
        const Array& v, const Matrix& rho, Real K) {

        const Array mu = x + Log(dq/dr) - 0.5 * v;
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

        const Real sig_xy = 1.0 - DotProduct(sig10, sig11Inv10);
        QL_REQUIRE(sig_xy > 0.0, "approximation loses validity");
        const Real sqSig_xy = std::sqrt(sig_xy);

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

        Matrix DM(N+2, N);
        for (Size k=0; k < N+2; ++k)
            std::copy(D[k].begin(), D[k].end(), DM.row_begin(k));

        const Matrix DF = DM*F;

        Real npv = dr*std::exp(mu[0] + 0.5*squared(nu[0])) * I(C[0], trF2, DM, DF, 0)
            - K*dr*I(C.back(), trF2, DM, DF, N+1);

        for (Size k=1; k <= N; ++k)
            npv -= dr*std::exp(mu[k] + 0.5*squared(nu[k])) * I(C[k], trF2, DM, DF, k);

        return npv;
    }
}
