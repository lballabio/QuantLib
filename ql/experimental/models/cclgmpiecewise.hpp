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

/*! \file cclgmpiecewise.hpp
    \brief cross currency parametrization using piecewise alpha, constant kappa,
           piecewise fx sigma, constant correlation
*/

#ifndef quantlib_cclgm_piecewise_hpp
#define quantlib_cclgm_piecewise_hpp

#include <ql/experimental/models/cclgmparametrization.hpp>
#include <ql/experimental/models/lgmpiecewisealphaconstantkappa.hpp>
#include <ql/experimental/models/lgmfxpiecewisesigma.hpp>
#include <ql/experimental/math/piecewiseintegral.hpp>

#include <ql/math/matrix.hpp>

namespace QuantLib {

namespace detail {

class CcLgmPiecewise
    : public CcLgmParametrization<CcLgmPiecewise, LgmFxPiecewiseSigma,
                                  LgmPiecewiseAlphaConstantKappa> {
  public:
    CcLgmPiecewise(
        const std::vector<boost::shared_ptr<
            LgmFxParametrization<LgmFxPiecewiseSigma> > > &fxParametrizations,
        const std::vector<boost::shared_ptr<LgmParametrization<
            LgmPiecewiseAlphaConstantKappa> > > &lgmParametrizations,
        const Matrix &correlation);

    //! interface (required)
    const Real rho_alpha_alpha_impl(const Size i, const Size j) const;
    const Real rho_alpha_sigma_impl(const Size i, const Size j) const;
    const Real rho_sigma_sigma_impl(const Size i, const Size j) const;

  private:
    const Matrix &correlation_;
    const Size n_;
};

//! interface (required)
inline const Real CcLgmPiecewise::rho_alpha_alpha_impl(const Size i,
                                                       const Size j) const {
    return correlation_[n_ + i][n_ + j];
}

inline const Real CcLgmPiecewise::rho_alpha_sigma_impl(const Size i,
                                                       const Size j) const {
    return correlation_[n_ + i][j];
}

inline const Real CcLgmPiecewise::rho_sigma_sigma_impl(const Size i,
                                                       const Size j) const {
    return correlation_[i][j];
}

/*! TODO implement closed form solutions for the interface methods
  replacing the standard implementation which uses numerical integration */

/* ... */

} // namespace detail

} // namespace QuantLib

#endif
