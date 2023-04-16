/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Mark Joshi

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

#ifndef quantlib_fra_time_dep_corr_struct_hpp
#define quantlib_fra_time_dep_corr_struct_hpp

#include <ql/models/marketmodels/piecewiseconstantcorrelation.hpp>
#include <vector>

namespace QuantLib {

    class CurveState;

    class CotSwapFromFwdCorrelation : public PiecewiseConstantCorrelation {
      public:
        CotSwapFromFwdCorrelation(
            const std::shared_ptr<PiecewiseConstantCorrelation>& fwdCorr,
            const CurveState& curveState,
            Spread displacement);
        const std::vector<Time>& times() const override;
        const std::vector<Time>& rateTimes() const override;
        const std::vector<Matrix>& correlations() const override;
        Size numberOfRates() const override;

      private:
        std::shared_ptr<PiecewiseConstantCorrelation> fwdCorr_;
        Size numberOfRates_;
        std::vector<Matrix> swapCorrMatrices_;
    };

}

#endif
