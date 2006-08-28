/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
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

#include <ql/MarketModels/PseudoRoots/expcorrflatvol.hpp>
#include <ql/Math/pseudosqrt.hpp>


namespace QuantLib
{
    ExpCorrFlatVol::ExpCorrFlatVol(
            const Real longTermCorr,
            const Real beta,
            const std::vector<Volatility>& volatilities,
            const EvolutionDescription& evolution,
            const Size numberOfFactors,
            const std::vector<Rate>& initialRates,
            const std::vector<Rate>& displacements)
     :  longTermCorr_(longTermCorr), beta_(beta), 
        volatilities_(volatilities),
        rateTimes_(evolution.rateTimes()),
        evolutionTimes_(evolution.evolutionTimes()),
        numberOfFactors_(numberOfFactors),
        initialRates_(initialRates),
        displacements_(displacements),
        pseudoRoots_(evolution.evolutionTimes().size())
    {
        Size n=volatilities.size();
        QL_REQUIRE(n==rateTimes_.size()-1, "resetTimes/vol mismatch");

        Matrix covariance(n, n);
        std::vector<Volatility> stdDev(n);
      
        Time effStartTime;
        Real correlation;
        for (Size k=0; k<evolutionTimes_.size(); ++k) {
            for (Size i=0; i<n; ++i) {
                effStartTime = (k>0 ? evolution.effectiveStopTime()[k-1][i] : 0.0);
                stdDev[i] = volatilities[i] *
                    std::sqrt(evolution.effectiveStopTime()[k][i]-effStartTime);
            }

            for (Size i=0; i<n; ++i) {
                for (Size j=i; j<n; ++j) {
                     correlation = longTermCorr + (1.0-longTermCorr) * 
                         std::exp(-beta*std::abs(rateTimes_[i]-rateTimes_[j]));
                     covariance[i][j] =  covariance[j][i] = 
                         stdDev[j] * correlation * stdDev[i];
                 }
            }

            pseudoRoots_[k] =
                rankReducedSqrt(covariance, numberOfFactors, 1.1,
                                SalvagingAlgorithm::None);
                //pseudoSqrt(covariance, SalvagingAlgorithm::None);

            QL_ENSURE(pseudoRoots_[k].rows()==n,
                      "step " << k <<
                      " flat vol wrong number of rows: " << pseudoRoots_[k].rows() <<
                      " instead of " << n);
            QL_ENSURE(pseudoRoots_[k].columns()==numberOfFactors,
                      "step " << k <<
                      " flat vol wrong number of columns: " << pseudoRoots_[k].columns() <<
                      " instead of " << numberOfFactors);

        }
       
    }

}
