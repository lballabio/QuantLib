/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/marketmodels/models/expcorrflatvol.hpp>
#include <ql/math/pseudosqrt.hpp>
#include <ql/math/linearinterpolation.hpp>

namespace QuantLib {

    ExpCorrFlatVol::ExpCorrFlatVol(
            Real longTermCorr,
            Real beta,
            const std::vector<Volatility>& volatilities,
            const EvolutionDescription& evolution,
            Size numberOfFactors,
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
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and rate times");
        QL_REQUIRE(numberOfRates_==displacements.size(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and displacements (" << displacements.size() << ")");
        QL_REQUIRE(numberOfRates_==volatilities.size(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and volatilities (" << volatilities.size() << ")");
        QL_REQUIRE(numberOfRates_<=numberOfFactors_*numberOfSteps_,
                   "number of rates (" << numberOfRates_ <<
                   ") greater than number of factors (" << numberOfFactors_
                   << ") times number of steps (" << numberOfSteps_ << ")");

        std::vector<Volatility> stdDev(numberOfRates_);

        Time effStartTime;
        Real correlation;
        const Matrix& effectiveStopTime = evolution.effectiveStopTime();
        for (Size k=0; k<numberOfSteps_; ++k) {
            Size i;
            for (i=0; i<numberOfRates_; ++i) {
                effStartTime = (k>0 ? effectiveStopTime[k-1][i] : 0.0);
                stdDev[i] = volatilities[i] *
                    std::sqrt(effectiveStopTime[k][i]-effStartTime);
            }

            for (i=0; i<numberOfRates_; ++i) {
                for (Size j=i; j<numberOfRates_; ++j) {
                     correlation = longTermCorr + (1.0-longTermCorr) *
                         std::exp(-beta*std::fabs(rateTimes[i]-rateTimes[j]));
                     covariance_[k][i][j] =  covariance_[k][j][i] =
                         stdDev[j] * correlation * stdDev[i];
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


    ExpCorrFlatVolFactory::ExpCorrFlatVolFactory(
                                 Real longTermCorr,
                                 Real beta,
                                 const std::vector<Time>& times,
                                 const std::vector<Volatility>& vols,
                                 const Handle<YieldTermStructure>& yieldCurve,
                                 Spread displacement)
    : longTermCorr_(longTermCorr), beta_(beta), times_(times),
      vols_(vols), yieldCurve_(yieldCurve), displacement_(displacement) {
        volatility_ = LinearInterpolation(times_.begin(),times_.end(),
                                          vols_.begin());
        volatility_.update();
        registerWith(yieldCurve_);
    }

    boost::shared_ptr<MarketModel>
    ExpCorrFlatVolFactory::create(const EvolutionDescription& evolution,
                                  Size numberOfFactors) const {
        const std::vector<Time>& rateTimes = evolution.rateTimes();
        Size numberOfRates = rateTimes.size()-1;

        std::vector<Rate> initialRates(numberOfRates);
        for (Size i=0; i<numberOfRates; ++i)
            initialRates[i] = yieldCurve_->forwardRate(rateTimes[i],
                                                       rateTimes[i+1],
                                                       Simple);

        std::vector<Volatility> displacedVolatilities(numberOfRates);
        for (Size i=0; i<numberOfRates; ++i) {
            Volatility vol = // to be changes
                volatility_(rateTimes[i]);
            displacedVolatilities[i] =
                initialRates[i]*vol/(initialRates[i]+displacement_);
        }

        std::vector<Spread> displacements(numberOfRates, displacement_);

        return boost::shared_ptr<MarketModel>(
                                      new ExpCorrFlatVol(longTermCorr_, beta_,
                                                         displacedVolatilities,
                                                         evolution,
                                                         numberOfFactors,
                                                         initialRates,
                                                         displacements));
    }

    void ExpCorrFlatVolFactory::update() {
        notifyObservers();
    }

}

