/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

/*! \file fxstickydeltasolver.hpp
    \brief ODE solution of the sticky-delta cost equation for the total vol smile.
*/

#ifndef quantlib_fx_sticky_delta_solver_hpp
#define quantlib_fx_sticky_delta_solver_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! Total vol smile implied by the sticky-delta cost equation
    /*! With total vol \f$ w(x) \f$ at log-moneyness \f$ x = \ln(F/K) \f$
        and local slope \f$ \beta = \partial w / \partial \ln K = -w'(x) \f$,
        the sticky-delta cost equation multiplied by \f$ w^2 \f$ is a
        quadratic in \f$ \beta \f$,
        \f[
            f(x, w, \beta) = a \beta^2 + b \beta + c = 0,
        \f]
        i.e. an implicit first-order ODE for \f$ w(x) \f$.  The costs are
        \f$ [C_\gamma, C_\theta, C_\eta, C_\omega] \f$.

        At \f$ d_- = 0 \f$, i.e. \f$ x^* = w^{*2}/2 \f$, total vanna and
        volga vanish, so \f$ a = b = 0 \f$ and the equation no longer
        depends on \f$ \beta \f$.  Every smooth smile passes through this
        singular point, where
        \f[
            w^* = -\frac{C_\gamma}{2 C_\theta} \;\mbox{(no premium adjustment)},
            \qquad
            w^* = \frac{C_\gamma}{2 (C_\eta - C_\theta)} \;\mbox{(premium adjusted)},
        \f]
        and \f$ u = 1 + w^* \beta^* \f$ solves
        \f$ C_\gamma u^3 - 2 C_\eta w^* u^2 + C_\omega w^{*2} u - C_\gamma = 0 \f$
        (one positive root if \f$ C_\eta^2 < \frac{3}{4} C_\gamma C_\omega \f$;
        otherwise the one closest to a flat smile is taken).

        The smile is integrated outwards from the singular point in both
        directions, between equally spaced nodes, with AdaptiveRungeKutta.
        In each interval the root of the quadratic closest to the slope at
        the starting node is taken.  Between nodes the total vol is the
        cubic Hermite interpolant of the node values and their exact
        slopes, so it is \f$ C^1 \f$.

        If the quadratic has no real root the smile cannot be continued;
        the side is then cut at the last node, and beyond the integrated
        range the total vol is extrapolated linearly with the slope at the
        last node.

        The solver is immutable: the smile is integrated in the
        constructor.
    */
    class FxStickyDeltaSmileSolver {
      public:
        /*! \param costs          \f$ [C_\gamma, C_\theta, C_\eta, C_\omega] \f$;
                                  only their ratios matter.
            \param premiumAdjust  whether the costs refer to premium-adjusted deltas.
            \param xMin, xMax     log-moneyness range to integrate; it is
                                  extended to contain the singular point.
            \param stepFraction   node spacing as a fraction of \f$ w^* \f$.
            \param accuracy       tolerance of the Runge-Kutta integration.
        */
        FxStickyDeltaSmileSolver(std::vector<Real> costs,
                                 bool premiumAdjust,
                                 Real xMin,
                                 Real xMax,
                                 Real stepFraction = 0.1,
                                 Real accuracy = 1.0e-8);

        //! Total vol \f$ w = \sigma \sqrt{\tau} \f$ at log-moneyness \f$ x = \ln(F/K) \f$.
        Real totalVol(Real x) const;
        //! Slope \f$ \beta = \partial w / \partial \ln K \f$ at log-moneyness \f$ x \f$.
        Real slope(Real x) const;

        const std::vector<Real>& costs() const { return costs_; }
        bool premiumAdjust() const { return premiumAdjust_; }

        //! \name Singular point (\f$ d_- = 0 \f$)
        //@{
        Real singularLogMoneyness() const { return xs_; }
        Real singularTotalVol() const { return ws_; }
        Real singularSlope() const { return bs_; }
        //@}

        //! \name Integrated range
        /*! Shorter than requested on a side where the smile could not be continued. */
        //@{
        Real minLogMoneyness() const { return x_.front(); }
        Real maxLogMoneyness() const { return x_.back(); }
        //@}

        //! Root of the quadratic in \f$ \beta \f$ at \f$ (x, w) \f$ closest to \c betaRef.
        /*! NaN if the quadratic has no real root. */
        Real quadraticRoot(Real x, Real w, Real betaRef) const;

      private:
        void coefficients(Real x, Real w, Real& a, Real& b, Real& c) const;
        //! Nodes outward from the singular point up to \c xEnd, excluding the singular point.
        void integrate(Integer side, Real xEnd,
                       std::vector<Real>& x, std::vector<Real>& w, std::vector<Real>& beta) const;

        std::vector<Real> costs_;
        bool premiumAdjust_;
        Real h_, accuracy_;
        Real xs_, ws_, bs_;
        std::vector<Real> x_, w_, beta_;  // nodes, ascending in x
    };

}

#endif
