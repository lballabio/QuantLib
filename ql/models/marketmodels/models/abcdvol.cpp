/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/models/marketmodels/models/abcdvol.hpp>
#include <ql/models/marketmodels/piecewiseconstantcorrelation.hpp>
#include <ql/termstructures/volatility/abcd.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>

namespace QuantLib {

    AbcdVol::AbcdVol(
            Real a,
            Real b,
            Real c,
            Real d,
            const std::vector<Real>& ks,
            const boost::shared_ptr<PiecewiseConstantCorrelation>& corr,
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
      pseudoRoots_(numberOfSteps_, Matrix(numberOfRates_, numberOfFactors_))
    {
        const std::vector<Time>& rateTimes = evolution.rateTimes();
        QL_REQUIRE(numberOfRates_==rateTimes.size()-1,
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and rate times");
        QL_REQUIRE(numberOfRates_==displacements.size(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and displacements (" << displacements.size() << ")");
        QL_REQUIRE(numberOfRates_==ks.size(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and ks (" << ks.size() << ")");
      //  QL_REQUIRE(numberOfRates_<=numberOfFactors_*numberOfSteps_,
      //             "number of rates (" << numberOfRates_ <<
     //              ") greater than number of factors (" << numberOfFactors_
       //            << ") times number of steps (" << numberOfSteps_ << ")");
        QL_REQUIRE(numberOfFactors<=numberOfRates_,
                   "number of factors (" << numberOfFactors <<
                   ") cannot be greater than numberOfRates (" <<
                   numberOfRates_ << ")");
        QL_REQUIRE(numberOfFactors>0,
                   "number of factors (" << numberOfFactors <<
                   ") must be greater than zero");

        AbcdFunction abcd(a, b, c, d);
        Real covar;
        Time effStartTime, effStopTime;
        Real correlation;
        const std::vector<Time>& corrTimes = corr->times();
        const std::vector<Time>& evolTimes = evolution.evolutionTimes();
        for (Size k=0, kk=0; k<numberOfSteps_; ++k) {
            // one covariance per evolution step
            Matrix covariance(numberOfRates_, numberOfRates_, 0.0);

            // there might be more than one correlation matrix
            // in a single evolution step
            Matrix correlations;

            for (; corrTimes[kk]<evolTimes[k]; ++kk) {
                effStartTime = kk==0 ? 0.0 : corrTimes[kk-1];
                effStopTime = corrTimes[kk];
                correlations = corr->correlation(kk);
                for (Size i=0; i<numberOfRates_; ++i) {
                    for (Size j=i; j<numberOfRates_; ++j) {
                        covar = ks[i] * ks[j] * abcd.covariance(effStartTime,
                                                                effStopTime,
                                                                rateTimes[i],
                                                                rateTimes[j]);
                        correlation = correlations[i][j];
                        covariance[i][j] += covar * correlation;
                    }
                }
            }
            // last part in the evolution step
            effStartTime = kk==0 ? 0.0 : corrTimes[kk-1];
            effStopTime = evolTimes[k];
            correlations = corr->correlation(kk);
            for (Size i=0; i<numberOfRates_; ++i) {
                for (Size j=i; j<numberOfRates_; ++j) {
                    covar = ks[i] * ks[j] * abcd.covariance(effStartTime,
                                                            effStopTime,
                                                            rateTimes[i],
                                                            rateTimes[j]);
                    correlation = correlations[i][j];
                    covariance[i][j] += covar * correlation;
                }
            }
            // no more use for the kk-th correlation matrix
            while (kk<corrTimes.size() && corrTimes[kk]<=evolTimes[k])
                ++kk;

            // make it symmetric
            for (Size i=0; i<numberOfRates_; ++i) {
                for (Size j=i+1; j<numberOfRates_; ++j) {
                     covariance[j][i] = covariance[i][j];
                 }
            }

            pseudoRoots_[k] = rankReducedSqrt(covariance,
                                              numberOfFactors, 1.0,
                                              SalvagingAlgorithm::None);

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
