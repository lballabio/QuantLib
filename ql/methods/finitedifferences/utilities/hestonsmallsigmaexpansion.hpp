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

namespace QuantLib {

namespace detail {

    /*! As the Heston vol-of-vol sigma -> 0, the running affine Riccati
        coefficients (D, C) of the Heston characteristic function

            chF(z, tau) = exp(v0*D + C)

        over a time interval of length tau, with terminal/boundary value
        Dprev for D, develop a removable singularity: every closed-form
        expression for D and C carries explicit v0/sigma^2 and
        kappa*theta/sigma^2 prefactors multiplying differences that
        themselves vanish like O(sigma^2), so naive floating point
        evaluation loses precision catastrophically (and can fail outright)
        as sigma becomes very small.

        hestonSmallSigmaStep() returns the O(sigma^2) Taylor expansion of
        (D, C) in sigma instead, which has no such cancellation and stays
        accurate (and defined) all the way down to sigma == 0.

        Dprev == 0 is the single-segment case: it reproduces (to machine
        precision) AnalyticHestonEngine::chF's existing small-sigma branch,
        which this expansion generalizes to an arbitrary complex terminal
        value Dprev. That generalization is what lets a piecewise-constant
        (multi-segment) Heston model apply the same small-sigma treatment
        per segment, carrying the running D coefficient from later segments
        into earlier ones exactly as AnalyticPTDHestonEngine::lnChF and
        AnalyticPTDHestonEngine::Fj_Helper already do for the full-precision
        formula.

        The expansion was derived by symbolic (computer-algebra) Taylor
        series expansion of the exact Riccati step in sigma around sigma=0,
        and cross-checked both against high-precision (multi-digit)
        numerical evaluation of the exact formula and against
        AnalyticHestonEngine::chF's existing branch (Dprev=0 case); see the
        regression tests in hestonmodel.cpp for small-vol-of-vol
        convergence to the corresponding Black-Scholes price.

        Used by COSHestonEngine::chF, AnalyticPTDHestonEngine::lnChF,
        AnalyticPTDHestonEngine::Fj_Helper, and HestonRNDCalculator's
        characteristic function (the v0 term there is already a stable,
        algebraically different representation and does not need this
        branch).

        The implementation is expressed purely in (bounded, in (0,1])
        powers of exp(-kappa*tau), never forming exp(+kappa*tau) or
        exp(+2*kappa*tau) as an intermediate quantity -- D and C are
        themselves bounded as kappa*tau -> infinity (at fixed kappa, theta,
        rho, z, Dprev), so this is not a change in what is computed, only in
        how; forming the positive exponential directly, as a more naive
        transcription of the Taylor coefficients would, instead overflows
        for long-dated, fast-mean-reverting inputs (kappa*tau beyond
        roughly 350 in double precision) well before the true value does.

        Known limitation (pre-existing, not introduced here): the exact
        (non-small-sigma) formula at each of this function's call sites is
        evaluated by direct subtraction/division rather than with the
        Andersen-Lake cancellation-reduction technique that
        AnalyticHestonEngine::lnChF's own exact branch already uses, so its
        own accuracy for sigma a few orders of magnitude above the
        sigma > 1e-6 threshold (not sigma -> 0, which is this function's
        job) can be visibly worse than this expansion's near-machine-
        precision accuracy just below the threshold. That mismatch predates
        this expansion -- it is a property of the untouched exact formula at
        each call site, not of hestonSmallSigmaStep -- and porting the
        Andersen-Lake technique into those call sites is a separate,
        independent improvement left for future work.
    */
    void hestonSmallSigmaStep(
        Real kappa, Real theta, Real sigma, Real rho, Time tau,
        const std::complex<Real>& z, const std::complex<Real>& Dprev,
        std::complex<Real>& D, std::complex<Real>& C);

}

}

#endif
