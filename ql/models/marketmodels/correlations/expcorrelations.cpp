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

#include <ql/models/marketmodels/correlations/expcorrelations.hpp>
#include <ql/models/marketmodels/correlations/timehomogeneousforwardcorrelation.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/math/comparison.hpp>

namespace QuantLib {

    Disposable<Matrix> exponentialCorrelations(
                                        const std::vector<Time>& rateTimes,
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
        Matrix correlations(nbRows, nbRows);
        for (Size i=0; i<nbRows; ++i) {
            correlations[i][i] = 1.0;
            for (Size j=0; j<i; ++j)
                if (time<rateTimes[j]) {
                    // correlation is defined only between 
                    // stochastic rates...
                    correlations[i][j] = correlations[j][i] =
                        longTermCorr + (1.0-longTermCorr) *
                        std::exp(-beta*std::fabs(
                            std::pow(rateTimes[i]-time, gamma) -
                            std::pow(rateTimes[j]-time, gamma)
                            )
                        );
                } else {
                    // ...so, if rates have already fixed 
                    // we put correlation to zero.
                    correlations[i][j] = correlations[j][i] = 0.0;
                }
        }
        return correlations;
    }


    ExponentialForwardCorrelation::ExponentialForwardCorrelation(
                                    const std::vector<Time>& rateTimes,
                                    Real longTermCorr,
                                    Real beta,
                                    Real gamma,
                                    const std::vector<Time>& times)
    : numberOfRates_(rateTimes.empty() ? 0 : rateTimes.size()-1),
      longTermCorr_(longTermCorr), beta_(beta), gamma_(gamma),
      rateTimes_(rateTimes),
      times_(times) {

        QL_REQUIRE(numberOfRates_>1,
                   "Rate times must contain at least two values");

        checkIncreasingTimes(rateTimes_);

        if (close(gamma,1.0)) {

            if (times_ == std::vector<Time>())
                std::copy(rateTimes.begin(), rateTimes.end()-2,
                          times_.begin());
            else {
                // times should be a subset of rateTimes
                std::vector<Time> temp(rateTimes_.begin(), rateTimes_.end()-2);
                QL_REQUIRE(times_==temp, "times!=rateTimes");
            }
            Matrix c = exponentialCorrelations(
                rateTimes_, longTermCorr_, beta_, 1.0, 0.0);
            correlations_ = TimeHomogeneousForwardCorrelation::
                                            evolvedMatrices(c, rateTimes_);
        } else {
            if (times_ == std::vector<Time>())
                std::copy(rateTimes.begin(), rateTimes.end()-1,
                          times_.begin());
            else {
                checkIncreasingTimes(times_);
                QL_REQUIRE(times_.back()<=rateTimes_[numberOfRates_],
                           "last corr time after rateTimes_[numberOfRates_]");
            }

            for (Size k=0; k<times_.size(); ++k) {
                Time time = 0.5*(times_[k+1]+times_[k]);
                correlations_[k] = exponentialCorrelations(
                    rateTimes_, longTermCorr_, beta_, gamma_, time);
            }
        }
    }

    const std::vector<Time>&
    ExponentialForwardCorrelation::times() const {
        return times_;
    }

    const std::vector<Matrix>&
    ExponentialForwardCorrelation::correlations() const {
        return correlations_;
    }

    Size ExponentialForwardCorrelation::numberOfRates() const {
        return numberOfRates_;
    }

}
