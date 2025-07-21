/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 François du Vignaud

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

#include <ql/math/comparison.hpp>
#include <ql/models/marketmodels/correlations/expcorrelations.hpp>
#include <ql/models/marketmodels/correlations/timehomogeneousforwardcorrelation.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

    Matrix exponentialCorrelations(const std::vector<Time>& rateTimes,
                                   Real longTermCorr,
                                   Real beta,
                                   Real gamma,
                                   Time time) {
        // preliminary checks
        checkIncreasingTimes(rateTimes);
        QL_REQUIRE(longTermCorr<=1.0 && longTermCorr>=0.0,
                   "Long term correlation (" << longTermCorr <<
                   ") outside [0;1] interval");
        QL_REQUIRE(beta>=0.0,
                   "beta (" << beta <<
                   ") must be greater than zero");
        QL_REQUIRE(gamma<=1.0 && gamma>=0.0,
                   "gamma (" << gamma <<
                   ") outside [0;1] interval");

        // Calculate correlation matrix
        Size nbRows = rateTimes.size()-1;
        Matrix correlations(nbRows, nbRows, 0.0);
        for (Size i=0; i<nbRows; ++i) {
            // correlation is defined only between
            // (alive) stochastic rates...
            if (time<=rateTimes[i]) {
                correlations[i][i] = 1.0;
                for (Size j=0; j<i; ++j) {
                    if (time<=rateTimes[j]) {
                        correlations[i][j] = correlations[j][i] =
                            longTermCorr + (1.0-longTermCorr) *
                            std::exp(-beta*std::fabs(
                                std::pow(rateTimes[i]-time, gamma) -
                                std::pow(rateTimes[j]-time, gamma)
                                )
                            );
                    }
                }
            }
        }
        return correlations;
    }


    ExponentialForwardCorrelation::ExponentialForwardCorrelation(const std::vector<Time>& rateTimes,
                                                                 Real longTermCorr,
                                                                 Real beta,
                                                                 Real gamma,
                                                                 std::vector<Time> times)
    : numberOfRates_(rateTimes.empty() ? 0 : rateTimes.size() - 1), longTermCorr_(longTermCorr),
      beta_(beta), gamma_(gamma), rateTimes_(rateTimes), times_(std::move(times)) {

        QL_REQUIRE(numberOfRates_>1,
                   "Rate times must contain at least two values");

        checkIncreasingTimes(rateTimes_);

        // corrTimes must include all rateTimes but the last
        if (times_.empty())
            times_ = std::vector<Time>(rateTimes_.begin(),
                                       rateTimes_.end()-1);
        else
            checkIncreasingTimes(times_);

        if (close(gamma,1.0)) {
            std::vector<Time> temp(rateTimes_.begin(), rateTimes_.end()-1);
            QL_REQUIRE(times_==temp,
                       "corr times " << io::sequence(times_)
                       << " must be equal to (all) rate times (but the last) "
                       << io::sequence(temp));
            Matrix c = exponentialCorrelations(
                rateTimes_, longTermCorr_, beta_, 1.0, 0.0);
            correlations_ =
                TimeHomogeneousForwardCorrelation::evolvedMatrices(c);
        } else {
            // FIXME should check here that all rateTimes but the last
            // are included in rateTimes
            QL_REQUIRE(times_.back()<=rateTimes_[numberOfRates_],
                       "last corr time " << times_.back() <<
                       "is after next-to-last rate time " <<
                       rateTimes_[numberOfRates_]);
            correlations_.resize(times_.size());
            Time time = times_[0]/2.0;
            correlations_[0] = exponentialCorrelations(
                rateTimes_, longTermCorr_, beta_, gamma_, time);
            for (Size k=1; k<times_.size(); ++k) {
                time = (times_[k]+times_[k-1])/2.0;
                correlations_[k] = exponentialCorrelations(
                    rateTimes_, longTermCorr_, beta_, gamma_, time);
            }
        }
    }

    const std::vector<Time>&
    ExponentialForwardCorrelation::times() const {
        return times_;
    }

    const std::vector<Time>&
    ExponentialForwardCorrelation::rateTimes() const {
        return rateTimes_;
    }

    const std::vector<Matrix>&
    ExponentialForwardCorrelation::correlations() const {
        return correlations_;
    }

    Size ExponentialForwardCorrelation::numberOfRates() const {
        return numberOfRates_;
    }

}
