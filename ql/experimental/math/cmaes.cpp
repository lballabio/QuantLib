/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Colin Alberts

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

#include <ql/experimental/math/cmaes.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/matrixutilities/symmetricschurdecomposition.hpp>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>
#include <vector>

namespace QuantLib {

    Cmaes::Cmaes(Configuration configuration)
    : configuration_(std::move(configuration)), rng_(configuration_.seed) {}

    EndCriteria::Type Cmaes::minimize(Problem& P, const EndCriteria& endCriteria) {
        EndCriteria::Type ecType{EndCriteria::None};
        P.reset();

        Size n{P.currentValue().size()};
        QL_REQUIRE(n >= 1, "CMA-ES needs at least one dimension");

        // strategy parameters and learning rates
        // \lambda = population size = offspring per generation
        Size lambda{configuration_.populationSize != 0
                        ? configuration_.populationSize
                        : Size(4 + std::floor(3.0 * std::log(Real(n))))};
        QL_REQUIRE(lambda >= 2, "CMA-ES population size must be at least 2");
        
        // \mu = number of parents that recombine
        Size mu{lambda / 2};

        // log-rank recombination weights, normalized to sum to one
        Array weights(mu);
        for (Size i{0}; i < mu; ++i)
            weights[i] = std::log((lambda + 1.0) / 2.0) - std::log(Real(i + 1));
        weights /= std::accumulate(weights.begin(), weights.end(), Real(0.0));

        // \mu_{eff} = variance-effective selection mass = how many samples effectively contribute
        Real muEff{1.0 / DotProduct(weights, weights)};

        // c_\sigma = learning rate for the step-size path p_\sigma
        Real cSigma{(muEff + 2.0) / (n + muEff + 5.0)};

        // d_\sigma = damping that slows the multiplicative step-size (sigma) update
        Real dSigma{1.0 + 2.0 * std::max(0.0, std::sqrt((muEff - 1.0) / (n + 1.0)) - 1.0)
                        + cSigma};

        // c_c = learning rate for the covariance path p_c
        Real cc{(4.0 + muEff / n) / (n + 4.0 + 2.0 * muEff / n)};

        // c_1 = learning rate of the rank-one covariance update (from p_c)
        Real c1{2.0 / (std::pow(n + 1.3, 2) + muEff)};

        // c_\mu = learning rate of the rank-\mu covariance update (from the selected steps)
        Real cMu{std::min(1.0 - c1,
                          2.0 * (muEff - 2.0 + 1.0 / muEff) / (std::pow(n + 2.0, 2) + muEff))};

        // \chi_n = E||N(0,I)||, the expected length of a standard normal vector
        Real chiN{std::sqrt(Real(n)) * (1.0 - 1.0 / (4.0 * n) + 1.0 / (21.0 * n * n))};

        // m = mean of the sampling distribution (current estimate of the optimum)
        Array mean(configuration_.initialMean.empty() ? P.currentValue()
                                                       : configuration_.initialMean);
        QL_REQUIRE(mean.size() == n, "initial mean size does not match problem dimension");

        // \sigma = overall step size (global scale of the sampling distribution)
        Real sigma{configuration_.sigma};

        // covariance C = B diag(D)^2 B^T describes the distribution's shape:
        // B's columns are the eigenvectors of C and D the square-roots of its
        // eigenvalues (both refreshed lazily from C below). Start at the identity.
        Matrix C(n, n, 0.0);
        Matrix B(n, n, 0.0);
        for (Size i{0}; i < n; ++i) {
            C[i][i] = 1.0;
            B[i][i] = 1.0;
        }
        Array D(n, 1.0);

        Array pSigma(n, 0.0);            // p_\sigma = step-size evolution path
        Array pCov(n, 0.0);              // p_c = covariance evolution path

        // box bounds lo/up: from the configuration if set, else the Constraint
        Array lo(configuration_.lowerBound.empty()
                     ? P.constraint().lowerBound(P.currentValue())
                     : configuration_.lowerBound);
        Array up(configuration_.upperBound.empty()
                     ? P.constraint().upperBound(P.currentValue())
                     : configuration_.upperBound);
        QL_REQUIRE(lo.size() == n && up.size() == n,
                   "bound size does not match problem dimension");

        // best sample (point and its cost) seen so far -- the returned result
        Array bestX(mean);
        Real bestF{QL_MAX_REAL};

        InverseCumulativeNormal icn;     // turns uniform draws into standard normals

        // generations between eigen-decomposition refreshes of B/D (amortises the
        // O(n^3) decomposition so the per-sample cost stays O(n^2))
        Size eigenEvery{std::max<Size>(1, Size(1.0 / (10.0 * n * (c1 + cMu))))};

        const Size maxResample{10};      // feasibility resample attempts before clamping
        const Real gamma{1.0};           // out-of-bounds penalty weight
        const Real degenerateScale{1e-12}; // stop once sigma*max(D) falls below this

        // per-generation offspring: candidate points x, their underlying
        // N(0,C) steps y = (x - m)/sigma, costs f, and a cost-sorted index list
        std::vector<Array> xSamples(lambda, Array(n));
        std::vector<Array> ySamples(lambda, Array(n));
        Array fSamples(lambda);
        std::vector<Size> order(lambda);

        Size g{0};                       // generation counter
        Size statState{0};               // consecutive near-stationary generations
        Real fOld{QL_MAX_REAL};          // previous best cost (for the stopping test)

        while (!endCriteria.checkMaxIterations(g, ecType)) {

            // refresh B and D from C lazily, and on generation 0; symmetrize
            // first to shed accumulated round-off asymmetry
            if (g % eigenEvery == 0) {
                for (Size i{0}; i < n; ++i)
                    for (Size j{0}; j < i; ++j) {
                        Real avg{0.5 * (C[i][j] + C[j][i])};
                        C[i][j] = C[j][i] = avg;
                    }

                SymmetricSchurDecomposition schur(C);
                B = schur.eigenvectors();

                Array ev(schur.eigenvalues());
                for (Size i{0}; i < n; ++i)
                    ev[i] = std::max(ev[i], QL_EPSILON);

                D = Sqrt(ev);
            }

            for (Size k{0}; k < lambda; ++k) {
                Array z(n);   // z ~ N(0,I)
                Array y(n);   // mutation step y = B(D*z) ~ N(0,C)
                Array x(n);   // candidate point m + sigma*y

                bool feasible{false};
                for (Size attempt{0}; attempt < maxResample && !feasible; ++attempt) {
                    for (Size i{0}; i < n; ++i)
                        z[i] = icn(rng_.next().value);

                    y = B * (D * z);             // element-wise D*z
                    x = mean + sigma * y;

                    feasible = P.constraint().test(x);
                    for (Size i{0}; i < n && feasible; ++i)
                        feasible = (x[i] >= lo[i] && x[i] <= up[i]);
                }

                // keep the original (unclamped) y for the path/covariance updates
                ySamples[k] = y;

                if (feasible) {
                    xSamples[k] = x;
                    fSamples[k] = P.value(x);
                } else {
                    Array xClamped(n);
                    for (Size i{0}; i < n; ++i)
                        xClamped[i] = std::min(std::max(x[i], lo[i]), up[i]);

                    Array excess(x - xClamped);
                    Real penalty{gamma * DotProduct(excess, excess)};

                    xSamples[k] = xClamped;
                    fSamples[k] = P.value(xClamped) + penalty;
                }
            }

            std::iota(order.begin(), order.end(), Size(0));
            std::sort(order.begin(), order.end(),
                      [&fSamples](Size a, Size b) { return fSamples[a] < fSamples[b]; });

            // yw = weighted mean of the best-mu mutation steps (drives the mean move)
            Array yw(n, 0.0);
            for (Size i{0}; i < mu; ++i)
                yw += weights[i] * ySamples[order[i]];

            mean = mean + sigma * yw;

            if (fSamples[order[0]] < bestF) {
                bestF = fSamples[order[0]];
                bestX = xSamples[order[0]];
            }

            // whiten the mean step: CinvHalf_yw = C^{-1/2} yw = B D^{-1} B^T yw,
            // built without forming or inverting a matrix
            Array t(transpose(B) * yw);
            for (Size i{0}; i < n; ++i)
                t[i] /= D[i];
            Array CinvHalf_yw(B * t);

            pSigma = (1.0 - cSigma) * pSigma
                   + std::sqrt(cSigma * (2.0 - cSigma) * muEff) * CinvHalf_yw;

            Real pSigmaNorm{Norm2(pSigma)};

            // Heaviside stall guard: freeze pCov while pSigma is anomalously long
            Real hSigmaLHS{pSigmaNorm / std::sqrt(1.0 - std::pow(1.0 - cSigma, 2.0 * (g + 1)))};
            Real hSigma{hSigmaLHS < (1.4 + 2.0 / (n + 1.0)) * chiN ? 1.0 : 0.0};

            pCov = (1.0 - cc) * pCov
                 + hSigma * std::sqrt(cc * (2.0 - cc) * muEff) * yw;

            // deltaH = variance restored to C when the p_c update is stalled (h_\sigma = 0)
            Real deltaH{(1.0 - hSigma) * cc * (2.0 - cc)};

            // covariance update: rank-one (pCov) plus rank-mu (selected y's)
            Matrix rankMu(n, n, 0.0);
            for (Size i{0}; i < mu; ++i)
                rankMu += weights[i] * outerProduct(ySamples[order[i]], ySamples[order[i]]);

            C = (1.0 - c1 - cMu) * C
              + c1 * (outerProduct(pCov, pCov) + deltaH * C)
              + cMu * rankMu;

            sigma = sigma * std::exp((cSigma / dSigma) * (pSigmaNorm / chiN - 1.0));

            if (sigma * (*std::max_element(D.begin(), D.end())) < degenerateScale) {
                ecType = EndCriteria::StationaryFunctionValue;
                break;
            }

            if (endCriteria.checkStationaryFunctionValue(fOld, bestF, statState, ecType))
                break;

            fOld = bestF;
            ++g;
        }

        P.setCurrentValue(bestX);
        P.setFunctionValue(bestF);
        return ecType;
    }

}
