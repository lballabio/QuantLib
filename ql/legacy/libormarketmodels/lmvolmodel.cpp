/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/legacy/libormarketmodels/lmlinexpvolmodel.hpp>

namespace QuantLib {

    LmVolatilityModel::LmVolatilityModel(Size size, Size nArguments)
    : size_(size),
      arguments_(nArguments) {
    }

    Size LmVolatilityModel::size() const {
        return size_;
    }

    Volatility LmVolatilityModel::volatility(
        Size i, Time t, const Array& x) const {
        // inefficient implementation, please overload in derived classes
        return volatility(t, x)[i];
    }

    Real LmVolatilityModel::integratedVariance(Size, Size, Time,
                                               const Array&) const {
        QL_FAIL("integratedVariance() method is not supported");
    }

    std::vector<Parameter> & LmVolatilityModel::params() {
        return arguments_;
    }

    void LmVolatilityModel::setParams(
        const std::vector<Parameter> & arguments) {
        arguments_ = arguments;
        generateArguments();
    }

}

