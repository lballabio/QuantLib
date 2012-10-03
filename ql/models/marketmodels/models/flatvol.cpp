/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/models/marketmodels/models/flatvol.hpp>
#include <ql/models/marketmodels/piecewiseconstantcorrelation.hpp>
#include <ql/models/marketmodels/correlations/timehomogeneousforwardcorrelation.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/models/marketmodels/correlations/expcorrelations.hpp>

using boost::shared_ptr;
using std::vector;

namespace QuantLib {

    Real flatVolCovariance(Time t1,Time t2,
                           Time T, Time S,
                           Volatility v1, Volatility v2) {
        QL_REQUIRE(t1<=t2,
                   "integrations bounds (" << t1 <<
                   "," << t2 << ") are in reverse order");
        Time cutOff = std::min(S,T);
        if (t1>=cutOff) {
            return 0.0;
        } else {
            cutOff = std::min(t2, cutOff);
            return (cutOff - t1)*v1*v2;
        }
    }

    FlatVol::FlatVol(
            const vector<Volatility>& vols,
            const shared_ptr<PiecewiseConstantCorrelation>& corr,
            const EvolutionDescription& evolution,
            Size numberOfFactors,
            const vector<Rate>& initialRates,
            const vector<Spread>& displacements)
    : numberOfFactors_(numberOfFactors),
      numberOfRates_(initialRates.size()),
      numberOfSteps_(evolution.evolutionTimes().size()),
      initialRates_(initialRates),
      displacements_(displacements),
      evolution_(evolution),
      pseudoRoots_(numberOfSteps_, Matrix(numberOfRates_, numberOfFactors_))
    {
        const vector<Time>& rateTimes = evolution.rateTimes();
        QL_REQUIRE(numberOfRates_==rateTimes.size()-1,
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and rate times");
        QL_REQUIRE(numberOfRates_==displacements.size(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and displacements (" << displacements.size() << ")");
        QL_REQUIRE(numberOfRates_==vols.size(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and vols (" << vols.size() << ")");
        QL_REQUIRE(numberOfRates_<=numberOfFactors_*numberOfSteps_,
                   "number of rates (" << numberOfRates_ <<
                   ") greater than number of factors (" << numberOfFactors_
                   << ") times number of steps (" << numberOfSteps_ << ")");
        QL_REQUIRE(numberOfFactors<=numberOfRates_,
                   "number of factors (" << numberOfFactors <<
                   ") cannot be greater than numberOfRates (" <<
                   numberOfRates_ << ")");
        QL_REQUIRE(numberOfFactors>0,
                   "number of factors (" << numberOfFactors <<
                   ") must be greater than zero");

        Time effStopTime = 0.0;
        const vector<Time>& corrTimes = corr->times();
        const vector<Time>& evolTimes = evolution.evolutionTimes();
        Matrix covariance(numberOfRates_, numberOfRates_);
        for (Size k=0, kk=0; k<numberOfSteps_; ++k) {
            // one covariance per evolution step
            std::fill(covariance.begin(), covariance.end(), 0.0);

            // there might be more than one correlation matrix
            // in a single evolution step
            for (; corrTimes[kk]<evolTimes[k]; ++kk) {
                Time effStartTime = effStopTime;
                effStopTime = corrTimes[kk];
                const Matrix& corrMatrix = corr->correlation(kk);
                for (Size i=0; i<numberOfRates_; ++i) {
                    for (Size j=i; j<numberOfRates_; ++j) {
                        Real cov = flatVolCovariance(effStartTime, effStopTime,
                                                     rateTimes[i], rateTimes[j],
                                                     vols[i], vols[j]);
                        covariance[i][j] += cov * corrMatrix[i][j];
                     }
                }
            }
            // last part in the evolution step
            Time effStartTime = effStopTime;
            effStopTime = evolTimes[k];
            const Matrix& corrMatrix = corr->correlation(kk);
            for (Size i=0; i<numberOfRates_; ++i) {
                for (Size j=i; j<numberOfRates_; ++j) {
                    Real cov = flatVolCovariance(effStartTime, effStopTime,
                                                 rateTimes[i], rateTimes[j],
                                                 vols[i], vols[j]);
                    covariance[i][j] += cov * corrMatrix[i][j];
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
                      << " flat vol wrong number of rows: "
                      << pseudoRoots_[k].rows()
                      << " instead of " << numberOfRates_);
            QL_ENSURE(pseudoRoots_[k].columns()==numberOfFactors,
                      "step " << k
                      << " flat vol wrong number of columns: "
                      << pseudoRoots_[k].columns()
                      << " instead of " << numberOfFactors_);
        }
    }


    FlatVolFactory::FlatVolFactory(
                                Real longTermCorrelation,
                                Real beta,
                                const vector<Time>& times,
                                const vector<Volatility>& vols,
                                const Handle<YieldTermStructure>& yieldCurve,
                                Spread displacement)
    : longTermCorrelation_(longTermCorrelation), beta_(beta),
      times_(times), vols_(vols), yieldCurve_(yieldCurve),
      displacement_(displacement) {
        volatility_ = LinearInterpolation(times_.begin(), times_.end(),
                                          vols_.begin());
        volatility_.update();
        registerWith(yieldCurve_);
    }

    shared_ptr<MarketModel>
    FlatVolFactory::create(const EvolutionDescription& evolution,
                                  Size numberOfFactors) const {
        const vector<Time>& rateTimes = evolution.rateTimes();
        Size numberOfRates = rateTimes.size()-1;

        vector<Rate> initialRates(numberOfRates);
        for (Size i=0; i<numberOfRates; ++i)
            initialRates[i] = yieldCurve_->forwardRate(rateTimes[i],
                                                       rateTimes[i+1],
                                                       Simple);

        vector<Volatility> displacedVolatilities(numberOfRates);
        for (Size i=0; i<numberOfRates; ++i) {
            Volatility vol = // to be changes
                volatility_(rateTimes[i]);
            displacedVolatilities[i] =
                initialRates[i]*vol/(initialRates[i]+displacement_);
        }

        vector<Spread> displacements(numberOfRates, displacement_);

        Matrix correlations = exponentialCorrelations(evolution.rateTimes(),
                                                      longTermCorrelation_,
                                                      beta_);
        shared_ptr<PiecewiseConstantCorrelation> corr(new
            TimeHomogeneousForwardCorrelation(correlations,
                                              rateTimes));
        return shared_ptr<MarketModel>(new
            FlatVol(displacedVolatilities,
                           corr,
                           evolution,
                           numberOfFactors,
                           initialRates,
                           displacements));
    }

    void FlatVolFactory::update() {
        notifyObservers();
    }

}

