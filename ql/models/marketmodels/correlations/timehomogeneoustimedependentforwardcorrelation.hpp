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

#ifndef quantlib_time_homogeneous_time_independent_forward_correlation_hpp
#define quantlib_time_homogeneous_time_independent_forward_correlation_hpp

#include <ql/models/marketmodels/piecewiseconstantcorrelation.hpp>
#include <ql/models/marketmodels/correlations/correlations.hpp>
#include <ql/math/matrix.hpp>
#include <vector>

namespace QuantLib {

    class TimeHomogeneousTimeDependentForwardCorrelation : 
        public PiecewiseConstantCorrelation {
      public:
        TimeHomogeneousTimeDependentForwardCorrelation(
                                    const std::vector<Time>& rateTimes,
                                    Real longTermCorr,
                                    Real beta,
                                    Real gamma);
        const std::vector<Time>& times() const;
        const std::vector<Matrix>& correlations() const;
        Size numberOfRates() const;
    private:
        Size numberOfRates_;
        Real longTermCorr_, beta_, gamma_;
        std::vector<Time> rateTimes_, times_;
        std::vector<Matrix> correlations_;
    };

}

#endif
