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

#include <ql/models/marketmodels/models/timehomogeneousforwardcorrelation.hpp>
#include <ql/math/pseudosqrt.hpp>

namespace QuantLib {

    TimeHomogeneousForwardCorrelation::TimeHomogeneousForwardCorrelation(
                        const Matrix& fwdCorrelation,
                        const std::vector<Time>& rateTimes,
                        Size numberOfFactors)
    : fwdCorrelation_(fwdCorrelation),
      rateTimes_(rateTimes),
      times_(rateTimes.begin(), rateTimes.end()-1),
      numberOfFactors_(numberOfFactors),
      numberOfRates_(rateTimes.size()-1),
      pseudoRoots_(rateTimes.size()-1, Matrix(numberOfRates_,
                                              numberOfFactors_,
                                              0.0)) {

        QL_REQUIRE(numberOfRates_==fwdCorrelation.rows(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and fwdCorrelation rows (" << fwdCorrelation.rows() << ")");
        QL_REQUIRE(numberOfRates_==fwdCorrelation.columns(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and fwdCorrelation columns (" << fwdCorrelation.columns() << ")");
        QL_REQUIRE(numberOfFactors<=fwdCorrelation.rows(),
                   "number of factors (" << numberOfFactors <<
                   ") must be less than correlation rows (" << fwdCorrelation.rows() << ")");

        for (Size i=0; i<pseudoRoots_.size(); ++i) {
            Matrix thisCorrelationMatrix(numberOfRates_-i,numberOfRates_-i);
            for (Size j=0; j < thisCorrelationMatrix.rows(); ++j)
                    for (Size k=0; k < thisCorrelationMatrix.rows(); ++k)
                        thisCorrelationMatrix[j][k] =  fwdCorrelation[j][k];

            Matrix smallPseudo = rankReducedSqrt(thisCorrelationMatrix,
                                                 numberOfFactors, 1.0,
                                                 SalvagingAlgorithm::None);

            for (Size j=0; j<smallPseudo.rows(); ++j) {
                std::copy(smallPseudo.row_begin(j), smallPseudo.row_end(j),
                          pseudoRoots_[i].row_begin(i+j));
            }
            for (Size j=0; j<i; ++j)
                pseudoRoots_[i][j][0] = 1.0;

        }

    }

    const std::vector<Time>& TimeHomogeneousForwardCorrelation::times() const {
        return times_;
    }

    const Matrix& TimeHomogeneousForwardCorrelation::pseudoRoot(Size i) const {
        QL_REQUIRE(i<pseudoRoots_.size(),
                   "index (" << i << ") must be less than pseudoRoots size ("
                   << pseudoRoots_.size() << ")");
        return pseudoRoots_[i];
    }

    Size TimeHomogeneousForwardCorrelation::numberOfFactors() const {
        return numberOfFactors_;
    }

    Size TimeHomogeneousForwardCorrelation::numberOfRates() const {
        return numberOfRates_;
    }

}
