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

/*! \file lgmpiecewisealphaconstantkappa.hpp
    \brief piecewise alpha, constant kappa
*/

#ifndef quantlib_lgm_piecewisealphaconstantkappa_hpp
#define quantlib_lgm_piecewisealphaconstantkappa_hpp

#include <ql/experimental/models/lgmparametrization.hpp>
#include <ql/math/array.hpp>
#include <vector>

namespace QuantLib {

namespace detail {

class LgmPiecewiseAlphaConstantKappa
    : public LgmParametrization<LgmPiecewiseAlphaConstantKappa> {
  public:
    // kappa is given as an one dimensional array to ease the linking
    // in calibrated model
    LgmPiecewiseAlphaConstantKappa(const Array &times, const Array &alphas,
                                   const Array &kappa);

    const Real zetaImpl(const Time t) const;
    const Real alphaImpl(const Time t) const;
    const Real HImpl(const Time t) const;
    const Real HprimeImpl(const Time t) const;
    const Real Hprime2Impl(const Time t) const;
    const void updateImpl() const;

    const Array &timesImpl() const { return times_; }

  private:
    const Array &times_, &alphas_, &kappa_;
    mutable std::vector<Real> zetas_;
};

// inline definitions

inline const void LgmPiecewiseAlphaConstantKappa::updateImpl() const {
    Real sum = 0.0;
    zetas_.resize(times_.size());
    for (Size i = 0; i < times_.size(); ++i) {
        sum += alphas_[i] * alphas_[i] *
               (times_[i] - (i == 0 ? 0.0 : times_[i - 1]));
        zetas_[i] = sum;
    }
}

inline const Real
LgmPiecewiseAlphaConstantKappa::alphaImpl(const Time t) const {
    if (t < 0.0)
        return 0.0;
    return std::fabs(alphas_[std::min<Size>(
        std::upper_bound(times_.begin(), times_.end(), t) - times_.begin(),
        alphas_.size() - 1)]);
}

inline const Real LgmPiecewiseAlphaConstantKappa::zetaImpl(const Time t) const {
    if (t < 0.0)
        return 0.0;
    Size i = std::upper_bound(times_.begin(), times_.end(), t) - times_.begin();
    Real res = 0.0;
    if (i >= 1)
        res += zetas_[std::min(i - 1, zetas_.size() - 1)];
    Real a = alphas_[std::min(i, alphas_.size() - 1)];
    res += a * a * (t - (i == 0 ? 0.0 : times_[i - 1]));
    return res;
}

inline const Real LgmPiecewiseAlphaConstantKappa::HImpl(const Time t) const {
    return std::fabs(kappa_[0]) < 1E-4 ? t : (1.0 - std::exp(-kappa_[0] * t)) /
                                                kappa_[0];
}

inline const Real
LgmPiecewiseAlphaConstantKappa::HprimeImpl(const Time t) const {
    return std::exp(-kappa_[0] * t);
}

inline const Real
LgmPiecewiseAlphaConstantKappa::Hprime2Impl(const Time t) const {
    return -kappa_[0] * std::exp(-kappa_[0] * t);
}

} // namespace detail

} // namespace QuantLib

#endif
