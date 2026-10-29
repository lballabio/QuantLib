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
    \brief ODE solutions of the sticky-delta cost equation for the total vol smile.
*/

#ifndef quantlib_fx_sticky_delta_solver_hpp
#define quantlib_fx_sticky_delta_solver_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! Total vol smile of the sticky-delta cost model
    /*! The smile is a function of log-moneyness \f$ x = \ln(F/K) \f$ held
        at nodes, with the total vol \f$ w = \sigma \sqrt{\tau} \f$ and its
        slope \f$ \beta = \partial w / \partial \ln K = -w'(x) \f$ at each.
        Between nodes the total vol is the cubic Hermite interpolant of the
        node values and slopes, so it is \f$ C^1 \f$; beyond the nodes it is
        extrapolated linearly with the slope at the last node.  The costs
        are \f$ [C_\gamma, C_\theta, C_\eta, C_\omega] \f$, of which only
        the ratios matter.

        Derived classes solve the cost equation for the nodes in their
        constructors, so smiles are immutable.
    */
    class FxStickyDeltaSmile {
      public:
        virtual ~FxStickyDeltaSmile() = default;

        //! Total vol \f$ w = \sigma \sqrt{\tau} \f$ at log-moneyness \f$ x = \ln(F/K) \f$.
        Real totalVol(Real x) const;
        //! Slope \f$ \beta = \partial w / \partial \ln K \f$ at log-moneyness \f$ x \f$.
        Real slope(Real x) const;
        //! Convexity \f$ \kappa \f$ of the smile dynamics at log-moneyness \f$ x \f$
        /*! The change in total vol at a fixed strike is
            \f$ dw = d\tilde w - \beta\, dF/F + \kappa\, (dF/F)^2 \f$.
        */
        virtual Real convexity(Real x) const = 0;

        const std::vector<Real>& costs() const { return costs_; }
        //! Whether \c costs give this smile, i.e. are a positive multiple of costs().
        /*! Ratios are compared to a relative tolerance of \f$ 10^{-12} \f$:
            rescaling the costs changes them by a few ulps, and costs closer
            than that give smiles far closer than the solvers' accuracy.
        */
        bool hasCosts(const std::vector<Real>& costs) const;
        bool premiumAdjust() const { return premiumAdjust_; }

        //! \name Solved range
        /*! Beyond it the smile is extrapolated linearly. */
        //@{
        Real minLogMoneyness() const { return x_.front(); }
        Real maxLogMoneyness() const { return x_.back(); }
        //@}

        //! The cost equation without convexity
        /*! \f[
                F = C_\gamma (1 + 2 d_- \beta + \beta^2 d_+ d_-) + 2 w C_\theta
                    - 2 C_\eta (d_* + \beta d_+ d_-) + d_+ d_- C_\omega
            \f]
            with \f$ d_\pm = x / w \pm w / 2 \f$ and \f$ d_* = d_+ \f$ with
            premium adjustment, \f$ d_- \f$ otherwise.
        */
        Real costEquation(Real x, Real w, Real beta) const;

      protected:
        FxStickyDeltaSmile(std::vector<Real> costs, bool premiumAdjust);
        //! Sets the nodes, ascending in \f$ x \f$.
        void setNodes(std::vector<Real> x, std::vector<Real> w, std::vector<Real> beta);

        std::vector<Real> costs_;
        bool premiumAdjust_;

      private:
        std::vector<Real> x_, w_, beta_;
    };


    //! Smile calibration ODE solver for the sticky-delta cost model
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
        depends on \f$ \beta \f$. Every smooth smile passes through this
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

        The solver is immutable: the smile is integrated in the constructor.
        Its dynamics have no convexity.
    */
    class FxStickyDeltaSmileSolver : public FxStickyDeltaSmile {
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

        Real convexity(Real) const override { return 0.0; }

        //! \name Singular point (\f$ d_- = 0 \f$)
        //@{
        Real singularLogMoneyness() const { return xs_; }
        Real singularTotalVol() const { return ws_; }
        Real singularSlope() const { return bs_; }
        //@}

        //! Root of the quadratic in \f$ \beta \f$ at \f$ (x, w) \f$ closest to \c betaRef.
        /*! NaN if the quadratic has no real root. */
        Real quadraticRoot(Real x, Real w, Real betaRef) const;

      private:
        void coefficients(Real x, Real w, Real& a, Real& b, Real& c) const;
        //! Nodes outward from the singular point up to \c xEnd, excluding the singular point.
        void integrate(Integer side, Real xEnd,
                       std::vector<Real>& x, std::vector<Real>& w, std::vector<Real>& beta) const;

        Real h_, accuracy_;
        Real xs_, ws_, bs_;
    };


    //! Sticky-delta cost smile with the convexity of exact sticky moneyness
    /*! If the smile is a fixed function of \f$ x = \ln(F/K) \f$, the change
        in total vol at a fixed strike is, with \f$ r = dF/F \f$ and
        \f$ \ln(1 + r) = r - r^2/2 \f$,
        \f[
            dw = d\tilde w - \beta r + \kappa r^2, \qquad \kappa = \frac{\beta + w''}{2},
        \f]
        so the convexity is given by the smile itself.  It only adds
        \f$ 2 w \kappa \f$ to the \f$ C_\gamma \f$ term of the cost
        equation, which then involves \f$ w'' \f$:
        \f[
            w'' = -\frac{F(x, w, \beta)}{w C_\gamma} - \beta, \qquad \beta = -w',
        \f]
        with \f$ F \f$ the cost equation without convexity (see
        costEquation()), and so \f$ \kappa = -F / (2 w C_\gamma) \f$.  There
        is no singular point any more, but the coefficient of \f$ w'' \f$ is
        small, of order \f$ w \f$, so the problem is singularly perturbed:
        it has fast modes on a scale of order \f$ w \f$ that grow in one
        direction and decay in the other.  Shooting from a point (or
        iterating on \f$ \kappa \f$) is therefore unstable, and the smile is
        found as a boundary value problem on \f$ [x_{min}, x_{max}] \f$ with
        \f$ w'' = 0 \f$ at both ends, i.e. no convexity beyond the range.
        The fast modes confine the effect of the end conditions to layers
        of width of order \f$ w \f$ at the ends.

        The equation is discretised on equally spaced nodes by central
        differences and solved by Newton's method, whose Jacobian is
        tridiagonal, starting from the smile without convexity
        (FxStickyDeltaSmileSolver) on the same range.  The end conditions
        use one-sided differences.  The solution is computed with \f$ n \f$
        and \f$ 2n - 1 \f$ nodes and Richardson-extrapolated on the coarse
        nodes, which makes it fourth-order accurate in the node spacing; the
        slopes at the nodes are fourth-order differences of the result.
    */
    class FxStickyDeltaConvexSmileSolver : public FxStickyDeltaSmile {
      public:
        /*! \param costs          \f$ [C_\gamma, C_\theta, C_\eta, C_\omega] \f$;
                                  only their ratios matter.
            \param premiumAdjust  whether the costs refer to premium-adjusted deltas.
            \param xMin, xMax     log-moneyness range of the boundary value problem.
            \param nodes          number of coarse nodes, at least 5.
            \param tolerance      Newton stops when no node moves by more than
                                  this, relative to the largest total vol, or
                                  when the steps stop shrinking at rounding
                                  level (below \f$ 10^{-10} \f$ relative).
        */
        FxStickyDeltaConvexSmileSolver(std::vector<Real> costs,
                                       bool premiumAdjust,
                                       Real xMin,
                                       Real xMax,
                                       Size nodes = 401,
                                       Real tolerance = 1.0e-13);

        //! \f$ \kappa = -F / (2 w C_\gamma) \f$ within the range, \f$ \beta / 2 \f$ beyond it.
        Real convexity(Real x) const override;

      private:
        // total vols at equally spaced nodes on [xMin, xMax] solving the
        // discretised problem, from the given guess
        std::vector<Real> solve(Real xMin, Real xMax, std::vector<Real> w, Real tolerance) const;
    };

}

#endif
