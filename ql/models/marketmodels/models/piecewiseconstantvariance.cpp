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


#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/errors.hpp>
#include <numeric>

namespace QuantLib {

    Real PiecewiseConstantVariance::variance(Size i) const {
        QL_REQUIRE(i<variances().size(),
                   "invalid step index");
        return variances()[i];
    }

    Volatility PiecewiseConstantVariance::volatility(Size i) const {
        QL_REQUIRE(i<volatilities().size(),
                   "invalid step index");
        return volatilities()[i];
    }

    Real PiecewiseConstantVariance::totalVariance(Size i) const {
        QL_REQUIRE(i<variances().size(),
                   "invalid step index");
        return std::accumulate(variances().begin(),
                               variances().begin()+i+1, 0.0);
    }

    Volatility PiecewiseConstantVariance::totalVolatility(Size i) const {
        return std::sqrt(totalVariance(i)/rateTimes()[i]);
    }

}
