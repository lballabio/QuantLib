/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

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

#include <ql/legacy/libormarketmodels/lmextlinexpvolmodel.hpp>

namespace QuantLib {

    LmExtLinearExponentialVolModel::LmExtLinearExponentialVolModel(
                                         const std::vector<Time>& fixingTimes,
                                         Real a, Real b, Real c, Real d)
    : LmLinearExponentialVolatilityModel(fixingTimes, a, b, c, d) {

        arguments_.resize(4+size_);
        for (Size i=0; i <size_; ++i) {
            arguments_[i+4] = ConstantParameter(1.0, PositiveConstraint());
        }
    }


    Array LmExtLinearExponentialVolModel::volatility(
                                               Time t, const Array& x) const {
        Array tmp = LmLinearExponentialVolatilityModel::volatility(t, x);
        for (Size i=0; i<size_; ++i) {
            tmp[i]*=arguments_[i+4](0.0);
        }

        return tmp;
    }

    Volatility LmExtLinearExponentialVolModel::volatility(
                                       Size i, Time t, const Array& x) const {
        return arguments_[i+4](0.0)
            *LmLinearExponentialVolatilityModel::volatility(i, t, x);
    }

    Real LmExtLinearExponentialVolModel::integratedVariance(
                               Size i, Size j, Time u, const Array& x) const {
        return arguments_[i+4](0.0)*arguments_[j+4](0.0)
            *LmLinearExponentialVolatilityModel::integratedVariance(i,j,u,x);
    }

}

