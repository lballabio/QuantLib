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

#ifndef quantlib_piecewise_constant_correlation_hpp
#define quantlib_piecewise_constant_correlation_hpp

#include <ql/math/matrix.hpp>
#include <vector>

namespace QuantLib {

    class Matrix;

    // corrTimes must include all rateTimes but the last
    class PiecewiseConstantCorrelation {
      public:
        virtual ~PiecewiseConstantCorrelation() = default;
        virtual const std::vector<Time>& times() const = 0;
        virtual const std::vector<Time>& rateTimes() const = 0;
        virtual const std::vector<Matrix>& correlations() const = 0;
        virtual const Matrix& correlation(Size i) const;
        virtual Size numberOfRates() const = 0;
    };

    inline const Matrix&
    PiecewiseConstantCorrelation::correlation(Size i) const {
        const std::vector<Matrix>& results = correlations();
        QL_REQUIRE(i<results.size(),
                   "index (" << i <<
                   ") must be less than correlations vector size (" <<
                   results.size() << ")");
        return results[i];
    }

}

#endif
