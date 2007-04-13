/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud

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

#include <ql/marketmodels/models/abcdmarketmodel.hpp>
#include <ql/marketmodels/utilities.hpp>
#include <ql/termstructures/volatilities/abcd.hpp>
#include <ql/math/pseudosqrt.hpp>

namespace QuantLib {

    // to be moved in utilities ...
     bool isIncluded(const std::vector<Time>& subSet,
                     const std::vector<Time>& set) {
        // we assume the vectors to be sorted
        Size j = 0;
        for (Size i = 0; i<subSet.size(); ++i) {
            Time subSetValue = subSet[i];
            while (subSetValue > set[j]){
                ++j;
                if(j == set.size())
                    return false;
            }
            if (subSetValue != set[j])
                    return false;
        }
        return true;
    }

    AbcdMarketModel::AbcdMarketModel(
            Real a,
            Real b,
            Real c,
            Real d,
            const std::vector<Real>& ks,
            const Matrix& correlations,
            const Size numberOfFactors,
            const std::vector<Rate>& initialRates,
            const std::vector<Spread>& displacements,
            const std::vector<Time>& rateTimes,
            const std::vector<Time>& evolutionTimes)
    : pseudoRoots_(evolutionTimes.size()),
      numberOfFactors_(numberOfFactors),
      numberOfRates_(initialRates.size()),
      numberOfSteps_(evolutionTimes.size()),
      initialRates_(initialRates),
      displacements_(displacements)
    {
        // we test if rate times are included in evolution times
        if (isIncluded(rateTimes, evolutionTimes ))
        {
            QL_REQUIRE(numberOfRates_==rateTimes.size()-1,
                       "mismatch between number of rates (" << numberOfRates_ <<
                       ") and rate times");
            QL_REQUIRE(numberOfRates_==displacements.size(),
                       "mismatch between number of rates (" << numberOfRates_ <<
                       ") and displacements (" << displacements.size() << ")");
            QL_REQUIRE(numberOfRates_==ks.size(),
                       "mismatch between number of rates (" << numberOfRates_ <<
                       ") and ks (" << ks.size() << ")");
            QL_REQUIRE(numberOfRates_<=numberOfFactors_*numberOfSteps_,
                       "number of rates (" << numberOfRates_ <<
                       ") greater than number of factors (" << numberOfFactors_
                       << ") times number of steps (" << numberOfSteps_ << ")");
            QL_REQUIRE(numberOfRates_==correlations.rows(),
                       "mismatch between number of rates (" << numberOfRates_ <<
                       ") and correlation rows (" << correlations.rows() << ")");
            QL_REQUIRE(numberOfRates_==correlations.columns(),
                       "mismatch between number of rates (" << numberOfRates_ <<
                       ") and correlation columns (" << correlations.columns() << ")");

            Abcd abcd(a, b, c, d);
            Matrix covariance(numberOfRates_, numberOfRates_);

            Size m = 0;
            // we iterate over evolution times
            for (Size l=0; l<numberOfSteps_; ++l) {
                Time effStartTime = l>0 ? evolutionTimes[l-1] : 0.0;
                Time effStopTime = evolutionTimes[l];
                // we find the corresponding rates time index
                while(rateTimes[m+1] < evolutionTimes[l] && m+1 < numberOfRates_)
                    ++m;
                // we iterate over Rates to fill the covariance matrix
                for (Size i=0; i<numberOfRates_; ++i) {
                    for (Size j=i; j<numberOfRates_; ++j) {
                        Real covar = abcd.covariance(effStartTime,
                                            effStopTime,
                                            rateTimes[i], rateTimes[j]);
                        Real correlation;
                        if (i>=m && j >=m)
                            correlation = correlations[i-m][j-m];
                        else
                            correlation = 0;
                        covariance[j][i] = covariance[i][j] =
                            ks[i] * ks[j] * covar * correlation;
                    }
                }

                pseudoRoots_[l] =
                    rankReducedSqrt(covariance, numberOfFactors, 1.0,
                                     SalvagingAlgorithm::None);

                QL_ENSURE(pseudoRoots_[l].rows()==numberOfRates_,
                          "step " << l
                          << " abcd vol wrong number of rows: "
                          << pseudoRoots_[l].rows()
                          << " instead of " << numberOfRates_);
                // test modified fdv1
                QL_ENSURE(pseudoRoots_[l].columns()<=numberOfFactors,
                          "step " << l
                          << " abcd vol wrong number of columns: "
                          << pseudoRoots_[l].columns()
                          << " instead of " << numberOfFactors);
            }
        }

        else // rate times are not included in evolution times
        {
            // we add rateTimes to evolutionTimes
            std::vector<Time> extendedEvolutionTimes;
            std::vector<std::vector<Time> > timesToMerge(2);
            timesToMerge[0] = rateTimes;
            timesToMerge[1] = evolutionTimes;
            std::vector<std::vector<bool> > isPresent;
            mergeTimes(timesToMerge, extendedEvolutionTimes, isPresent);
            // create a temporary AbcdMarketModel full factors
            AbcdMarketModel abcdMarketModelTemp(
                    a, b, c, d, ks, correlations, numberOfRates_,
                    initialRates, displacements, rateTimes,
                    extendedEvolutionTimes);
            // we iterate over the evolutionTimes and add covariance matrices
            Size i = 0;
            Matrix covariance(numberOfRates_, numberOfRates_,0);
            for (Size l = 0; l < evolutionTimes.size(); ++l) {
                // TODO: improve memory allocation if needed... fdv1
                Matrix Covariance(numberOfRates_, numberOfRates_, 0.0);
                while(extendedEvolutionTimes[i] < evolutionTimes[l]) {
                    const Matrix& pseudoRoot
                        = abcdMarketModelTemp.pseudoRoot(i);
                    Matrix temp = pseudoRoot * transpose(pseudoRoot);
                    covariance += temp;
                }
                pseudoRoots_[l] =
                    rankReducedSqrt(covariance, numberOfFactors, 1.0,
                                     SalvagingAlgorithm::None);
            }
        }
    }

}
