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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/marketmodels/piecewiseconstantcorrelations/timehomogeneousforwardcorrelation.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>

namespace QuantLib {

    TimeHomogeneousForwardCorrelation::TimeHomogeneousForwardCorrelation(
                        const Matrix& fwdCorrelation,
                        const std::vector<Time>& rateTimes)
    : fwdCorrelation_(fwdCorrelation),
      rateTimes_(rateTimes),
      times_(rateTimes.begin(), rateTimes.end()-1),
      numberOfRates_(rateTimes.size()-1),
      correlations_(rateTimes.size()-1, Matrix(numberOfRates_,
                                               numberOfRates_,
                                               0.0)) {

        QL_REQUIRE(numberOfRates_==fwdCorrelation.rows(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and fwdCorrelation rows (" << fwdCorrelation.rows() << ")");
        QL_REQUIRE(numberOfRates_==fwdCorrelation.columns(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and fwdCorrelation columns (" << fwdCorrelation.columns() << ")");

        for (Size k=0; k<correlations_.size(); ++k) {
            // proper diagonal values
            for (Size i=0; i<numberOfRates_; ++i)
                correlations_[k][i][i] = 1.0;
            // copy only time hogeneous values
            for (Size i=k; i<numberOfRates_; ++i) {
                for (Size j=k; j<i; ++j) {
                    correlations_[k][i][j] = correlations_[k][j][i] = 
                        fwdCorrelation_[i-k][j-k];
                }
            }
        }


    }

    const std::vector<Time>& TimeHomogeneousForwardCorrelation::times() const {
        return times_;
    }

    const std::vector<Matrix>&
    TimeHomogeneousForwardCorrelation::correlations() const {
        return correlations_;
    }

    Size TimeHomogeneousForwardCorrelation::numberOfRates() const {
        return numberOfRates_;
    }

}
