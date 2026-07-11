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

        const Size n{P.currentValue().size()};
        QL_REQUIRE(n >= 1, "CMA-ES needs at least one dimension");

        const Size lambda{configuration_.populationSize != 0
                              ? configuration_.populationSize
                              : Size(4 + std::floor(3.0 * std::log(Real(n))))};
        QL_REQUIRE(lambda >= 2, "CMA-ES population size must be at least 2");

        const Size mu{lambda / 2};

        Array weights(mu);
        for (Size i{0}; i < mu; ++i)
            weights[i] = std::log((lambda + 1.0) / 2.0) - std::log(Real(i + 1));
        weights /= std::accumulate(weights.begin(), weights.end(), Real(0.0));

        const Real muEff{1.0 / DotProduct(weights, weights)};
        const Real cSigma{(muEff + 2.0) / (n + muEff + 5.0)};
        const Real dSigma{1.0 + 2.0 * std::max(0.0, std::sqrt((muEff - 1.0) / (n + 1.0)) - 1.0)
                              + cSigma};
        const Real cc{(4.0 + muEff / n) / (n + 4.0 + 2.0 * muEff / n)};
        const Real c1{2.0 / (std::pow(n + 1.3, 2) + muEff)};
        const Real cMu{std::min(1.0 - c1,
                                2.0 * (muEff - 2.0 + 1.0 / muEff) / (std::pow(n + 2.0, 2) + muEff))};
        const Real chiN{std::sqrt(Real(n)) * (1.0 - 1.0 / (4.0 * n) + 1.0 / (21.0 * n * n))};

        const Real pSigmaGain{std::sqrt(cSigma * (2.0 - cSigma) * muEff)};
        const Real pCovGain{std::sqrt(cc * (2.0 - cc) * muEff)};

        Array mean(configuration_.initialMean.empty() ? P.currentValue()
                                                       : configuration_.initialMean);
        QL_REQUIRE(mean.size() == n, "initial mean size does not match problem dimension");

        Real sigma{configuration_.sigma};

        // C = B diag(D)^2 * B^T, initialised to the identity
        Matrix C(n, n, 0.0);
        for (Size i{0}; i < n; ++i)
            C[i][i] = 1.0;
        Matrix B(C);
        Array D(n, 1.0);

        Array pSigma(n, 0.0);
        Array pCov(n, 0.0);

        // Box bounds: configuration overrides the constraint
        const Array lo(configuration_.lowerBound.empty()
                           ? P.constraint().lowerBound(P.currentValue())
                           : configuration_.lowerBound);
        const Array up(configuration_.upperBound.empty()
                           ? P.constraint().upperBound(P.currentValue())
                           : configuration_.upperBound);
        QL_REQUIRE(lo.size() == n && up.size() == n,
                   "bound size does not match problem dimension");

        Array bestX(mean);
        Real bestF{QL_MAX_REAL};

        const InverseCumulativeNormal icn;

        // B/D refresh cadence, amortising the decomposition
        const Size eigenEvery{std::max<Size>(1, Size(1.0 / (10.0 * n * (c1 + cMu))))};

        const Size maxResample{10};      // resample attempts before clamping
        const Real gamma{1.0};           // out-of-bounds penalty weight
        const Real degenerateScale{1e-12};

        std::vector<Array> xSamples(lambda, Array(n));
        std::vector<Array> ySamples(lambda, Array(n));
        Array fSamples(lambda);
        std::vector<Size> order(lambda);

        Size g{0};
        Size statState{0};               // consecutive near-stationary generations
        Real fOld{QL_MAX_REAL};

        while (!endCriteria.checkMaxIterations(g, ecType)) {

            // Symmetrize C to shed round-off before decomposing
            if (g % eigenEvery == 0) {
                for (Size i{0}; i < n; ++i)
                    for (Size j{0}; j < i; ++j) {
                        const Real avg{0.5 * (C[i][j] + C[j][i])};
                        C[i][j] = C[j][i] = avg;
                    }

                SymmetricSchurDecomposition schur(C);
                B = schur.eigenvectors();

                // floor the eigenvalues to keep D positive-definite
                const Array& ev = schur.eigenvalues();
                std::transform(ev.begin(), ev.end(), D.begin(),
                               [](Real e) { return std::sqrt(std::max(e, QL_EPSILON)); });
            }

            // per-sample scratch, reused across offspring
            Array z(n);
            Array y(n);
            Array x(n);
            Array xClamped(n);

            for (Size k{0}; k < lambda; ++k) {
                bool feasible{false};
                for (Size attempt{0}; attempt < maxResample && !feasible; ++attempt) {
                    std::generate(z.begin(), z.end(),
                                  [&] { return icn(rng_.next().value); });

                    y = B * (D * z);             // element-wise D*z
                    x = mean + sigma * y;

                    feasible = P.constraint().test(x);
                    for (Size i{0}; i < n && feasible; ++i)
                        feasible = (x[i] >= lo[i] && x[i] <= up[i]);
                }

                // unclamped y (not the clamped x) feeds the updates
                ySamples[k] = y;

                if (feasible) {
                    xSamples[k] = x;
                    fSamples[k] = P.value(x);
                } else {
                    for (Size i{0}; i < n; ++i)
                        xClamped[i] = std::clamp(x[i], lo[i], up[i]);

                    const Array excess(x - xClamped);
                    const Real penalty{gamma * DotProduct(excess, excess)};

                    xSamples[k] = xClamped;
                    fSamples[k] = P.value(xClamped) + penalty;
                }
            }

            std::iota(order.begin(), order.end(), Size(0));
            std::sort(order.begin(), order.end(),
                      [&fSamples](Size a, Size b) { return fSamples[a] < fSamples[b]; });

            Array yw(n, 0.0);
            for (Size i{0}; i < mu; ++i)
                yw += weights[i] * ySamples[order[i]];

            mean = mean + sigma * yw;

            if (fSamples[order[0]] < bestF) {
                bestF = fSamples[order[0]];
                bestX = xSamples[order[0]];
            }

            // cInvSqrtYw = C^{-1/2} yw, without forming or inverting a matrix
            Array t(transpose(B) * yw);
            t /= D;
            const Array cInvSqrtYw(B * t);

            pSigma = (1.0 - cSigma) * pSigma
                   + pSigmaGain * cInvSqrtYw;

            const Real pSigmaNorm{Norm2(pSigma)};

            const Real hSigmaLHS{pSigmaNorm / std::sqrt(1.0 - std::pow(1.0 - cSigma, 2.0 * (g + 1)))};
            const Real hSigma{hSigmaLHS < (1.4 + 2.0 / (n + 1.0)) * chiN ? 1.0 : 0.0};

            pCov = (1.0 - cc) * pCov
                 + hSigma * pCovGain * yw;

            const Real deltaH{(1.0 - hSigma) * cc * (2.0 - cc)};

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
