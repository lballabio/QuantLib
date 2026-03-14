/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/legacy/libormarketmodels/lmcorrmodel.hpp>

namespace QuantLib {

    LmCorrelationModel::LmCorrelationModel(Size size, Size nArguments)
    : size_(size), arguments_(nArguments) {}

    Size LmCorrelationModel::size() const {
        return size_;
    }

    Size LmCorrelationModel::factors() const {
        return size_;
    }

    bool LmCorrelationModel::isTimeIndependent() const {
        return false;
    }

    Matrix LmCorrelationModel::pseudoSqrt(
        Time t, const Array& x) const {
        return QuantLib::pseudoSqrt(this->correlation(t, x),
                                    SalvagingAlgorithm::Spectral);
    }

    Real LmCorrelationModel::correlation(
        Size i, Size j, Time t, const Array& x) const {
        // inefficient implementation, please overload in derived classes
        return correlation(t, x)[i][j];
    }


    std::vector<Parameter>& LmCorrelationModel::params() {
        return arguments_;
    }

    void LmCorrelationModel::setParams(
        const std::vector<Parameter> & arguments) {
        arguments_ = arguments;
        generateArguments();
    }

}

