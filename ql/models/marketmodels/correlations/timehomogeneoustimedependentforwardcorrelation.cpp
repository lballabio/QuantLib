/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti

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

#include <ql/models/marketmodels/correlations/timehomogeneoustimedependentforwardcorrelation.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>

namespace QuantLib {

    TimeHomogeneousTimeDependentForwardCorrelation::
        TimeHomogeneousTimeDependentForwardCorrelation(
                                    const std::vector<Time>& rateTimes,
                                    Real longTermCorr,
                                    Real beta,
                                    Real gamma)
    : numberOfRates_(rateTimes.empty() ? 0 : rateTimes.size()-1),
      longTermCorr_(longTermCorr), beta_(beta), gamma_(gamma),
      rateTimes_(rateTimes),
      times_(numberOfRates_), // ---
      correlations_(times_.size(), Matrix(numberOfRates_,
                                          numberOfRates_,
                                          0.0)) {
        // checks
        QL_REQUIRE(numberOfRates_>1,
                   "Rate times must contain at least two values");

        std::copy(rateTimes.begin(), rateTimes.end()-1, times_.begin()); // ---
        checkIncreasingTimes(times_);

        for (Size k=0; k<times_.size(); ++k) {
            Time time = 0.5*(times_[k+1]+times_[k]);    // rate taus ?
            correlations_[k] = exponentialCorrelationsTimeDependent(
                rateTimes_, longTermCorr_, beta_, gamma_, time);
        }
    }

    const std::vector<Time>&
    TimeHomogeneousTimeDependentForwardCorrelation::times() const {
        return times_;
    }

    const std::vector<Matrix>&
    TimeHomogeneousTimeDependentForwardCorrelation::correlations() const {
        return correlations_;
    }

    Size
    TimeHomogeneousTimeDependentForwardCorrelation::numberOfRates() const {
        return numberOfRates_;
    }

}
