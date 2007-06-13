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
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/utilities/disposable.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/math/matrixutilities/getcovariance.hpp>

namespace QuantLib {

    CotSwapFromFwdCorrelation::CotSwapFromFwdCorrelation(
            const Matrix& fraCorrelation,
            const CurveState& curveState,
            Spread displacement,
            const EvolutionDescription& evolution)
    : fraCorrelationMatrix_(evolution.numberOfRates()),
      numberOfRates_(evolution.numberOfRates()),
      evolution_(evolution) {

        Size nbRates = evolution.numberOfRates();
        QL_REQUIRE(nbRates==curveState.numberOfRates(),
                   "mismatch between number of rates in evolution (" <<
                   nbRates << ") and curveState (" <<
                   curveState.numberOfRates() << ")");
        QL_REQUIRE(nbRates==fraCorrelation.rows(),
                   "mismatch between number of rates (" << nbRates <<
                   ") and fraCorrelation rows (" <<
                   fraCorrelation.rows() << ")");
        QL_REQUIRE(nbRates==fraCorrelation.columns(),
                   "mismatch between number of rates (" << nbRates <<
                   ") and fraCorrelation columns (" <<
                   fraCorrelation.columns() << ")");
        QL_REQUIRE(fraCorrelation.rows()==fraCorrelation.columns(),
                   "correlation matrix is not square: " <<
                   fraCorrelation.rows() << " rows and " <<
                   fraCorrelation.columns() << " columns");

        Matrix zed = SwapForwardMappings::coterminalSwapZedMatrix(
                                                curveState, displacement);
        Matrix zedT = transpose(zed);
        Matrix cotSwapCorrelation = CovarianceDecomposition(
                zed * fraCorrelation * zedT).correlationMatrix();
        correlations_ = std::vector<Matrix>(evolution.numberOfRates(),
                                            cotSwapCorrelation);
    }

    const std::vector<Time>& CotSwapFromFwdCorrelation::times() const {
        return evolution_.evolutionTimes();
    }

    Size CotSwapFromFwdCorrelation::numberOfRates() const {
        return numberOfRates_;
    }

    const std::vector<Matrix>&
    CotSwapFromFwdCorrelation::correlations() const {
        return correlations_;
    }

}
