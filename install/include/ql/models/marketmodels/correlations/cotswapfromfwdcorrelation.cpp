/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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

#include <ql/models/marketmodels/correlations/cotswapfromfwdcorrelation.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/math/matrixutilities/getcovariance.hpp>

namespace QuantLib {

    CotSwapFromFwdCorrelation::CotSwapFromFwdCorrelation(
            const ext::shared_ptr<PiecewiseConstantCorrelation>& fwdCorr,
            const CurveState& curveState,
            Spread displacement)
    : fwdCorr_(fwdCorr),
      numberOfRates_(fwdCorr->numberOfRates()),
      swapCorrMatrices_(fwdCorr->correlations().size())
    {
        QL_REQUIRE(numberOfRates_==curveState.numberOfRates(),
                   "mismatch between number of rates in fwdCorr (" <<
                   numberOfRates_ << ") and curveState (" <<
                   curveState.numberOfRates() << ")");

        Matrix zed = SwapForwardMappings::coterminalSwapZedMatrix(
                                                curveState, displacement);
        Matrix zedT = transpose(zed);
        const std::vector<Matrix>& fwdCorrMatrices = fwdCorr->correlations();
        for (Size k = 0; k<fwdCorrMatrices.size(); ++k) {
            swapCorrMatrices_[k] = CovarianceDecomposition(
                zed * fwdCorrMatrices[k] * zedT).correlationMatrix();
            // zero expired rates' correlation coefficients
            const std::vector<Time>& rateTimes = curveState.rateTimes();
            const std::vector<Time>& corrTimes = fwdCorr_->times();
            for (Size i=0; i<numberOfRates_; ++i)
                for (Size j=0; j<=i; ++j)
                    if (corrTimes[k]>rateTimes[j])
                        swapCorrMatrices_[k][i][j] =
                            swapCorrMatrices_[k][j][i] = 0.0;
        }
    }

    const std::vector<Time>& CotSwapFromFwdCorrelation::times() const {
        return fwdCorr_->times();
    }

    const std::vector<Time>& CotSwapFromFwdCorrelation::rateTimes() const {
        return fwdCorr_->rateTimes();
    }

    Size CotSwapFromFwdCorrelation::numberOfRates() const {
        return numberOfRates_;
    }

    const std::vector<Matrix>&
    CotSwapFromFwdCorrelation::correlations() const {
        return swapCorrMatrices_;
    }

}
