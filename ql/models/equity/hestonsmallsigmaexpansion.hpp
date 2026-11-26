/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yang Ye

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

/*! \file hestonsmallsigmaexpansion.hpp
    \brief small-vol-of-vol Taylor expansion of one Heston Riccati step
*/

#ifndef quantlib_heston_small_sigma_expansion_hpp
#define quantlib_heston_small_sigma_expansion_hpp

#include <ql/types.hpp>
#include <complex>
#include <utility>

namespace QuantLib {

namespace detail {

    /*! As the Heston vol-of-vol sigma -> 0, the running affine Riccati
        coefficients (D, C) of the Heston characteristic function

            chF(z, tau) = exp(v0*D + C)

        over a time interval of length tau, with terminal/boundary value
        Dprev for D, develop a removable singularity: the usual closed-form
        expressions for D and C carry 1/sigma^2 and kappa*theta/sigma^2
        prefactors multiplying differences that
        themselves vanish like O(sigma^2), so naive floating point
        evaluation loses precision catastrophically (and can fail outright)
        as sigma becomes very small.

        hestonSmallSigmaStep() returns the pair (D, C) given by the O(sigma^2)
        Taylor expansion in sigma instead, which has no such cancellation.
        It is used when sigma is small (and kappa is not); at sigma == 0 with
        kappa == 0 the expansion itself divides by kappa^3, so callers keep
        the closed-form branch for tiny kappa.

        Dprev == 0 is the single-segment case, used by
        AnalyticHestonEngine::chF and COSHestonEngine::chF.
        A nonzero complex Dprev lets a piecewise-constant (multi-segment)
        Heston model apply the same treatment per segment, carrying the
        running D coefficient from later segments into earlier ones as
        AnalyticPTDHestonEngine::lnChF and
        AnalyticPTDHestonEngine::Fj_Helper do for the full formula.

        The expansion is the symbolic Taylor series in sigma around sigma=0
        of the exact Riccati step; see the regression tests in
        hestonmodel.cpp.

        Used by AnalyticHestonEngine::chF, COSHestonEngine::chF,
        AnalyticPTDHestonEngine::lnChF,
        AnalyticPTDHestonEngine::Fj_Helper, and HestonRNDCalculator's
        characteristic function (whose v0 term uses a stable, algebraically
        different representation and does not need this branch).

        The implementation uses only powers of exp(-kappa*tau), which lie in
        (0,1], and never forms exp(+kappa*tau) or exp(+2*kappa*tau): D stays
        bounded and C grows at most linearly in tau as kappa*tau -> infinity,
        whereas exp(+2*kappa*tau) overflows for kappa*tau beyond roughly 350
        in double precision.

        \param kappa, theta, sigma, rho  Heston parameters of the segment
        \param tau    length of the segment
        \param z      argument of the characteristic function
        \param Dprev  value of D at the end of the segment
        \return the pair (D, C)
    */
    std::pair<std::complex<Real>, std::complex<Real>> hestonSmallSigmaStep(
        Real kappa, Real theta, Real sigma, Real rho, Time tau,
        const std::complex<Real>& z, const std::complex<Real>& Dprev);

}

}

#endif
