/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/experimental/models/cclgmpiecewise.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/matrixutilities/symmetricschurdecomposition.hpp>

namespace QuantLib {

namespace detail {

CcLgmPiecewise::CcLgmPiecewise(
    const std::vector<boost::shared_ptr<
        LgmFxParametrization<LgmFxPiecewiseSigma> > > &fxParametrizations,
    const std::vector<
        boost::shared_ptr<LgmParametrization<LgmPiecewiseAlphaConstantKappa> > >
        &lgmParametrizations,
    const Matrix &correlation)
    : CcLgmParametrization<CcLgmPiecewise, LgmFxPiecewiseSigma,
                           LgmPiecewiseAlphaConstantKappa>(fxParametrizations,
                                                           lgmParametrizations),
      correlation_(correlation), n_(fxParametrizations.size()) {
    QL_REQUIRE(correlation_.rows() == 2 * n_ + 1 &&
                   correlation_.columns() == 2 * n_ + 1,
               "correlation matrix is "
                   << correlation_.rows() << " x " << correlation_.columns()
                   << ", expected " << (2 * n_ + 1) << " x " << (2 * n_ + 1));

    for (Size i = 0; i < correlation_.rows(); ++i) {
        for (Size j = 0; j < i; ++j) {
            QL_REQUIRE(correlation_[i][j] == correlation_[j][i],
                       "correlation matrix is not symmetric");
            QL_REQUIRE(correlation_[i][j] >= -1 && correlation_[i][j] <= 1,
                       "correlation matrix contains elements outside [-1,1]");
        }
        QL_REQUIRE(
            close_enough(correlation_[i][i], 1.0),
            "correlation matrix contains diagonal elements not equal to 1");
    }

    SymmetricSchurDecomposition ssd(correlation_);
    for (Size i = 0; i < ssd.eigenvalues().size(); ++i) {
        QL_REQUIRE(ssd.eigenvalues()[i] >= 0.0,
                   "correlation matrix has negative eigenvalue @"
                       << i << ": " << ssd.eigenvalues()[i]);
    }

    std::vector<Time> allTimes;
    for (Size i = 0; i < fxParametrizations.size(); ++i) {
        allTimes.insert(allTimes.end(), fxParametrizations[i]->times().begin(),
                        fxParametrizations[i]->times().end());
    }
    for (Size i = 0; i < lgmParametrizations.size(); ++i) {
        allTimes.insert(allTimes.end(), lgmParametrizations[i]->times().begin(),
                        lgmParametrizations[i]->times().end());
    }

    boost::shared_ptr<Integrator> simpson =
        boost::make_shared<SimpsonIntegral>(1E-10, 100);

    setIntegrator(
        boost::make_shared<PiecewiseIntegral>(simpson, allTimes, true));
}

} // namespace detail

} // namespace QuantLib
