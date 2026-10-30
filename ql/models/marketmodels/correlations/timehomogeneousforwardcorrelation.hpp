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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_time_homogeneous_forward_correlation_hpp
#define quantlib_time_homogeneous_forward_correlation_hpp

#include <ql/models/marketmodels/piecewiseconstantcorrelation.hpp>
#include <ql/math/matrix.hpp>
#include <vector>

namespace QuantLib {

    class TimeHomogeneousForwardCorrelation :
                                        public PiecewiseConstantCorrelation {
      public:
        TimeHomogeneousForwardCorrelation(const Matrix& fwdCorrelation,
                                          const std::vector<Time>& rateTimes);
        const std::vector<Time>& times() const override;
        const std::vector<Time>& rateTimes() const override;
        const std::vector<Matrix>& correlations() const override;
        Size numberOfRates() const override;
        static std::vector<Matrix> evolvedMatrices(
                                        const Matrix& fwdCorrelation);
    private:
        Size numberOfRates_;
        Matrix fwdCorrelation_;
        std::vector<Time> rateTimes_, times_;
        std::vector<Matrix> correlations_;
    };

}

#endif
