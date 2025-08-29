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
#include <ql/legacy/libormarketmodels/lmexpcorrmodel.hpp>

namespace QuantLib {

    LmExponentialCorrelationModel::LmExponentialCorrelationModel(Size size,
                                                                 Real rho)
   : LmCorrelationModel(size, 1),
     corrMatrix_(size, size),
     pseudoSqrt_(size, size) {
        arguments_[0] = ConstantParameter(rho, PositiveConstraint());
        LmExponentialCorrelationModel::generateArguments();
    }

    Matrix LmExponentialCorrelationModel::correlation(Time, const Array&) const {

        Matrix tmp(corrMatrix_);
        return tmp;
    }

    Real LmExponentialCorrelationModel::correlation(
                                   Size i, Size j, Time, const Array&) const {
        return corrMatrix_[i][j];
    }

    bool LmExponentialCorrelationModel::isTimeIndependent() const {
        return true;
    }

    Matrix LmExponentialCorrelationModel::pseudoSqrt(Time, const Array&) const {
        Matrix tmp(pseudoSqrt_);
        return tmp;
    }

    void LmExponentialCorrelationModel::generateArguments() {
        const Real rho = arguments_[0](0.0);

        for (Size i=0; i<size_; ++i) {
            for (Size j=i; j<size_; ++j) {
                corrMatrix_[i][j] = corrMatrix_[j][i]
                    = std::exp(-rho*std::fabs(Real(i)-Real(j)));
            }
        }

        pseudoSqrt_ = QuantLib::pseudoSqrt(corrMatrix_,
                                           SalvagingAlgorithm::Spectral);
    }

}

