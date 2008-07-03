/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2007 Katiuscia Manzoni

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

#include <ql/models/marketmodels/correlations/timehomogeneousforwardcorrelation.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>

namespace QuantLib {

    TimeHomogeneousForwardCorrelation::TimeHomogeneousForwardCorrelation(
                        const Matrix& fwdCorrelation,
                        const std::vector<Time>& rateTimes)
    : numberOfRates_(rateTimes.empty() ? 0 : rateTimes.size()-1),
      fwdCorrelation_(fwdCorrelation),
      rateTimes_(rateTimes),
      times_(numberOfRates_) {

        checkIncreasingTimes(rateTimes);
        QL_REQUIRE(numberOfRates_>=1,
                   "Rate times must contain at least two values");
        QL_REQUIRE(numberOfRates_==fwdCorrelation.rows(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and fwdCorrelation rows (" << fwdCorrelation.rows() << ")");
        QL_REQUIRE(numberOfRates_==fwdCorrelation.columns(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and fwdCorrelation columns (" << fwdCorrelation.columns() << ")");

        std::copy(rateTimes.begin(), rateTimes.end()-1, times_.begin());
        correlations_ = evolvedMatrices(fwdCorrelation_);
    }

    std::vector<Matrix> TimeHomogeneousForwardCorrelation::evolvedMatrices(
                                    const Matrix& fwdCorrelation) {
        Size numberOfRates = fwdCorrelation.rows();
        std::vector<Matrix> correlations(numberOfRates, Matrix(numberOfRates,
                                                               numberOfRates,
                                                               0.0));
        for (Size k=0; k<correlations.size(); ++k) {
            // proper diagonal values
            for (Size i=k; i<numberOfRates; ++i)
                correlations[k][i][i] = 1.0;
            // copy only time homogeneous values
            for (Size i=k; i<numberOfRates; ++i) {
                for (Size j=k; j<i; ++j) {
                    correlations[k][i][j] = correlations[k][j][i] =
                        fwdCorrelation[i-k][j-k];
                }
            }
        }
        return correlations;
    }

    const std::vector<Time>&
    TimeHomogeneousForwardCorrelation::times() const {
        return times_;
    }

    const std::vector<Time>&
    TimeHomogeneousForwardCorrelation::rateTimes() const {
        return rateTimes_;
    }

    const std::vector<Matrix>&
    TimeHomogeneousForwardCorrelation::correlations() const {
        return correlations_;
    }

    Size TimeHomogeneousForwardCorrelation::numberOfRates() const {
        return numberOfRates_;
    }

}
