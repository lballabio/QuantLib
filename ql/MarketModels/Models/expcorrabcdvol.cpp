/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/MarketModels/Models/expcorrabcdvol.hpp>
#include <ql/Math/pseudosqrt.hpp>

namespace QuantLib {

    ExpCorrAbcdVol::ExpCorrAbcdVol(
            Real a,
            Real b,
            Real c,
            Real d,
            const std::vector<Real>& ks,
            Real longTermCorr,
            Real beta,
            const EvolutionDescription& evolution,
            const Size numberOfFactors,
            const std::vector<Rate>& initialRates,
            const std::vector<Spread>& displacements)
    : numberOfFactors_(numberOfFactors),
      numberOfRates_(initialRates.size()),
      numberOfSteps_(evolution.evolutionTimes().size()),
      initialRates_(initialRates),
      displacements_(displacements),
      evolution_(evolution),
      pseudoRoots_(numberOfSteps_, Matrix(numberOfRates_, numberOfFactors_)),
      covariance_(numberOfSteps_, Matrix(numberOfRates_, numberOfRates_)),
      totalCovariance_(numberOfSteps_, Matrix(numberOfRates_, numberOfRates_))
    {
        const std::vector<Time>& rateTimes = evolution.rateTimes();
        QL_REQUIRE(numberOfRates_==rateTimes.size()-1,
                   "initialRates/rateTimes mismatch");
        QL_REQUIRE(numberOfRates_==displacements.size(),
                   "initialRates/displacements mismatch");
        QL_REQUIRE(numberOfRates_==ks.size(),
                   "initialRates/ks mismatch");

        Time effStartTime;
        Real correlation, covar;
        Abcd abcd(a, b, c, d);
        const Matrix& effectiveStopTime = evolution.effectiveStopTime();
        for (Size k=0; k<numberOfSteps_; ++k) {
            for (Size i=0; i<numberOfRates_; ++i) {
                for (Size j=i; j<numberOfRates_; ++j) {
                    correlation = longTermCorr + (1.0-longTermCorr) *
                        std::exp(-beta*std::abs(rateTimes[i]-rateTimes[j]));
                    effStartTime = k>0 ? effectiveStopTime[k-1][i] : 0.0;
                    covar = abcd.covariance(effStartTime,
                                            effectiveStopTime[k][i],
                                            rateTimes[i], rateTimes[j]);
                    covariance_[k][j][i] = covariance_[k][i][j] =
                        ks[i] * ks[j] * covar * correlation ;
                 }
             }

            pseudoRoots_[k] =
                rankReducedSqrt(covariance_[k], numberOfFactors, 1.0,
                                SalvagingAlgorithm::None);

            totalCovariance_[k] = covariance_[k];
            if (k>0)
                totalCovariance_[k] += totalCovariance_[k-1];

            QL_ENSURE(pseudoRoots_[k].rows()==numberOfRates_,
                      "step " << k
                      << " abcd vol wrong number of rows: "
                      << pseudoRoots_[k].rows()
                      << " instead of " << numberOfRates_);
            QL_ENSURE(pseudoRoots_[k].columns()==numberOfFactors,
                      "step " << k
                      << " abcd vol wrong number of columns: "
                      << pseudoRoots_[k].columns()
                      << " instead of " << numberOfFactors);
        }
    }

}
