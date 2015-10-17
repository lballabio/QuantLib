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

/*! \file lgmfxpiecewisesigma.hpp
    \brief fx piecewise sigma
*/

#ifndef lgm_fx_piecewisesigma_hpp
#define lgm_fx_piecewisesigma_hpp

#include <ql/experimental/models/lgmfxparametrization.hpp>
#include <ql/math/array.hpp>

namespace QuantLib {

namespace detail {

class LgmFxPiecewiseSigma : public LgmFxParametrization<LgmFxPiecewiseSigma> {
  public:
    LgmFxPiecewiseSigma(const Array &times, const Array &sigmas);

    const void updateImpl() const;
    const Real sigmaImpl(const Time t) const;
    const Real varianceImpl(const Time t) const;
    const Real stdDeviationImpl(const Time t) const;

    const Array &timesImpl() const { return times_; }

  private:
    const Array &times_, &sigmas_;
    mutable std::vector<Real> variances_;
};

// inline definitions

inline const void LgmFxPiecewiseSigma::updateImpl() const {
    Real sum = 0.0;
    variances_.resize(times_.size());
    for (Size i = 0; i < times_.size(); ++i) {
        sum += sigmas_[i] * sigmas_[i] *
               (times_[i] - (i == 0 ? 0.0 : times_[i - 1]));
        variances_[i] = sum;
    }
}

inline const Real LgmFxPiecewiseSigma::sigmaImpl(const Time t) const {
    if (t < 0.0)
        return 0.0;
    return std::fabs(sigmas_[std::min<Size>(
        std::upper_bound(times_.begin(), times_.end(), t) - times_.begin(),
        sigmas_.size() - 1)]);
}

inline const Real LgmFxPiecewiseSigma::varianceImpl(const Time t) const {
    if (t < 0.0)
        return 0.0;
    Size i = std::upper_bound(times_.begin(), times_.end(), t) - times_.begin();
    Real res = 0.0;
    if (i >= 1)
        res += variances_[std::min(i - 1, variances_.size() - 1)];
    Real s = sigmas_[std::min(i, sigmas_.size() - 1)];
    res += s * s * (t - (i == 0 ? 0.0 : times_[i - 1]));
    return res;
}
} // namespace detail
} // namespace QuantLib

#endif
