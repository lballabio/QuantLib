/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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

#include <ql/methods/montecarlo/genericlsregression.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/math/matrixutilities/svd.hpp>

namespace QuantLib {

    Real genericLongstaffSchwartzRegression(
                std::vector<std::vector<NodeData> >& simulationData,
                std::vector<std::vector<Real> >& basisCoefficients) {

        Size steps = simulationData.size();
        basisCoefficients.resize(steps-1);

        for (Size i=steps-1; i!=0; --i) {

            std::vector<NodeData>& exerciseData = simulationData[i];

            // 1) find the covariance matrix of basis function values and
            //    deflated cash-flows
            Size N = exerciseData.front().values.size();
            std::vector<Real> temp(N+1);
            SequenceStatistics stats(N+1);

            Size j;
            for (j=0; j<exerciseData.size(); ++j) {
                if (exerciseData[j].isValid) {
                    std::copy(exerciseData[j].values.begin(),
                              exerciseData[j].values.end(),
                              temp.begin());
                    temp.back() = exerciseData[j].cumulatedCashFlows
                                - exerciseData[j].controlValue;

                    stats.add(temp);
                }
            }

            std::vector<Real> means = stats.mean();
            Matrix covariance = stats.covariance();

            Matrix C(N,N);
            Array target(N);
            for (Size k=0; k<N; ++k) {
                target[k] = covariance[k][N] + means[k]*means[N];
                for (Size l=0; l<=k; ++l)
                    C[k][l] = C[l][k] = covariance[k][l] + means[k]*means[l];
            }

            // 2) solve for least squares regression
            Array alphas = SVD(C).solveFor(target);
            basisCoefficients[i-1].resize(N);
            std::copy(alphas.begin(), alphas.end(),
                      basisCoefficients[i-1].begin());

            // 3) use exercise strategy to divide paths into exercise and
            //    non-exercise domains
            for (j=0; j<exerciseData.size(); ++j) {
                if (exerciseData[j].isValid) {
                    Real exerciseValue = exerciseData[j].exerciseValue;
                    Real continuationValue =
                        exerciseData[j].cumulatedCashFlows;
                    Real estimatedContinuationValue =
                        std::inner_product(
                                 exerciseData[j].values.begin(),
                                 exerciseData[j].values.end(),
                                 alphas.begin(),
                                 exerciseData[j].controlValue);

                    // for exercise paths, add deflated rebate to
                    // deflated cash-flows at previous time frame;
                    // for non-exercise paths, add deflated cash-flows to
                    // deflated cash-flows at previous time frame
                    Real value = estimatedContinuationValue <= exerciseValue ?
                                 exerciseValue :
                                 continuationValue;

                    simulationData[i-1][j].cumulatedCashFlows += value;
                }
            }
        }

        // the value of the product can now be estimated by averaging
        // over all paths
        Statistics estimate;
        std::vector<NodeData>& estimatedData = simulationData[0];
        for (auto& j : estimatedData)
            estimate.add(j.cumulatedCashFlows);

        return estimate.mean();
    }

}

