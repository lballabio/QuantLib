/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/MonteCarlo/genericlsregression.hpp>
#include <ql/Math/sequencestatistics.hpp>
#include <ql/Math/svd.hpp>

namespace QuantLib {

    Real genericLongstaffSchwartzRegression(
                std::vector<std::vector<LSNodeData> >& simulationData,
                std::vector<std::vector<Real> >& basisCoefficients) {

        Size steps = simulationData.size();
        basisCoefficients.resize(steps-1);

        for (Size i=steps-1; i!=0; --i) {

            std::vector<LSNodeData>& exerciseData = simulationData[i];

            // 1) find the covariance matrix of basis function values and
            //    deflated cash-flows
            Size N = exerciseData.front().basisFunctionValues.size();
            std::vector<Real> temp(N+1);
            SequenceStatistics stats(N+1);
            
            Size j;
            for (j=0; j<exerciseData.size(); ++j) {
                std::copy(exerciseData[j].basisFunctionValues.begin(),
                          exerciseData[j].basisFunctionValues.end(),
                          temp.begin());
                temp.back() = exerciseData[j].cumulatedCashFlows
                            - exerciseData[j].controlValue;

                stats.add(temp);
            }

            Matrix covariance = stats.covariance();
            
            Matrix basisCovariance(N,N);
            for (Size k=0; k<N; ++k)
                std::copy(covariance.row_begin(k),covariance.row_begin(k)+N,
                          basisCovariance.row_begin(k));

            Array target(N);
            std::copy(covariance.row_begin(N),covariance.row_begin(N)+N,
                      target.begin());

            // 2) solve for least squares regression
            Array alphas = SVD(basisCovariance).solveFor(target);
            basisCoefficients[i-1].resize(N);
            std::copy(alphas.begin(), alphas.end(),
                      basisCoefficients[i-1].begin());

            // 3) use exercise strategy to divide paths into exercise and
            //    non-exercise domains
            for (j=0; j<exerciseData.size(); ++j) {

                Real exerciseValue = exerciseData[j].exerciseValue;
                Real continuationValue = exerciseData[j].cumulatedCashFlows;
                Real estimatedContinuationValue =
                    std::inner_product(
                                 exerciseData[j].basisFunctionValues.begin(),
                                 exerciseData[j].basisFunctionValues.end(),
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

        // the value of the product can now be estimated by averaging
        // over all paths
        Statistics estimate;
        std::vector<LSNodeData>& estimatedData = simulationData[0];
        for (Size j=0; j<estimatedData.size(); ++j)
            estimate.add(estimatedData[j].cumulatedCashFlows);

        return estimate.mean();
    }

}

