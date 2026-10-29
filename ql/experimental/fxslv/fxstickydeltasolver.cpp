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

#include <ql/errors.hpp>
#include <ql/experimental/fxslv/fxstickydeltasolver.hpp>
#include <ql/math/array.hpp>
#include <ql/math/ode/adaptiverungekutta.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/utilities/null.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

namespace QuantLib {

    FxStickyDeltaSmile::FxStickyDeltaSmile(std::vector<Real> costs, bool premiumAdjust)
    : costs_(std::move(costs)), premiumAdjust_(premiumAdjust) {
        QL_REQUIRE(costs_.size() == 4, "four costs required, " << costs_.size() << " given");
        QL_REQUIRE(costs_[0] > 0.0, "gamma cost must be positive, got " << costs_[0]);
    }

    void FxStickyDeltaSmile::setNodes(std::vector<Real> x,
                                      std::vector<Real> w,
                                      std::vector<Real> beta) {
        QL_REQUIRE(!x.empty() && x.size() == w.size() && x.size() == beta.size(),
                   "inconsistent smile nodes");
        x_ = std::move(x);
        w_ = std::move(w);
        beta_ = std::move(beta);
    }

    bool FxStickyDeltaSmile::hasCosts(const std::vector<Real>& costs) const {
        if (costs.size() != costs_.size() || costs[0] <= 0.0)
            return false;
        // c_i / c_0 == costs_i / costs_0, cross-multiplied to avoid dividing
        const Real tolerance = 1.0e-12;
        for (Size i = 1; i < costs.size(); ++i) {
            const Real lhs = costs[i] * costs_[0], rhs = costs_[i] * costs[0];
            if (std::fabs(lhs - rhs) > tolerance * (std::fabs(lhs) + std::fabs(rhs)))
                return false;
        }
        return true;
    }

    Real FxStickyDeltaSmile::costEquation(Real x, Real w, Real beta) const {
        const Real cg = costs_[0], ct = costs_[1], ce = costs_[2], cw = costs_[3];
        const Real dp = x / w + w / 2.0, dm = dp - w;
        const Real ds = premiumAdjust_ ? dp : dm;
        return cg * (1.0 + 2.0 * dm * beta + beta * beta * dp * dm) + 2.0 * w * ct -
               2.0 * ce * (ds + beta * dp * dm) + dp * dm * cw;
    }

    Real FxStickyDeltaSmile::totalVol(Real x) const {
        if (x <= x_.front() || x >= x_.back()) {
            // linear extrapolation beyond the solved range
            const Size i = x <= x_.front() ? 0 : x_.size() - 1;
            const Real w = w_[i] - beta_[i] * (x - x_[i]);
            QL_REQUIRE(w > 0.0, "extrapolated total vol is not positive at log-moneyness " << x);
            return w;
        }

        // cubic Hermite interpolation with dw/dx = -beta at the nodes
        const Size i = std::upper_bound(x_.begin(), x_.end(), x) - x_.begin() - 1;
        const Real dx = x_[i + 1] - x_[i], t = (x - x_[i]) / dx;
        const Real t2 = t * t, t3 = t2 * t;
        return (2.0 * t3 - 3.0 * t2 + 1.0) * w_[i] - (t3 - 2.0 * t2 + t) * dx * beta_[i] +
               (-2.0 * t3 + 3.0 * t2) * w_[i + 1] - (t3 - t2) * dx * beta_[i + 1];
    }

    Real FxStickyDeltaSmile::slope(Real x) const {
        if (x <= x_.front())
            return beta_.front();
        if (x >= x_.back())
            return beta_.back();

        const Size i = std::upper_bound(x_.begin(), x_.end(), x) - x_.begin() - 1;
        const Real dx = x_[i + 1] - x_[i], t = (x - x_[i]) / dx;
        const Real t2 = t * t;
        const Real dwdx = (6.0 * t2 - 6.0 * t) / dx * (w_[i] - w_[i + 1]) -
                          (3.0 * t2 - 4.0 * t + 1.0) * beta_[i] -
                          (3.0 * t2 - 2.0 * t) * beta_[i + 1];
        return -dwdx;
    }


    FxStickyDeltaSmileSolver::FxStickyDeltaSmileSolver(std::vector<Real> costs,
                                                       bool premiumAdjust,
                                                       Real xMin,
                                                       Real xMax,
                                                       Real stepFraction,
                                                       Real accuracy)
    : FxStickyDeltaSmile(std::move(costs), premiumAdjust), accuracy_(accuracy) {
        QL_REQUIRE(stepFraction > 0.0, "step fraction must be positive, got " << stepFraction);
        QL_REQUIRE(accuracy > 0.0, "accuracy must be positive, got " << accuracy);
        QL_REQUIRE(xMin <= xMax, "empty log-moneyness range [" << xMin << ", " << xMax << "]");

        const Real cg = costs_[0], ct = costs_[1], ce = costs_[2], cw = costs_[3];

        // singular point: the cost equation at d- = 0 no longer depends on beta
        const Real den = premiumAdjust_ ? 2.0 * (ce - ct) : -2.0 * ct;
        QL_REQUIRE(den > 0.0, "costs [" << cg << ", " << ct << ", " << ce << ", " << cw
                                         << "] give no positive total vol at d- = 0");
        ws_ = cg / den;
        xs_ = ws_ * ws_ / 2.0;

        // slope at the singular point: u = 1 + w_s * beta_s solves the monic cubic
        // u^3 + p u^2 + q u - 1 = 0, negative at u = 0 so it has a positive root
        const Real p = -2.0 * ce / cg * ws_, q = cw / cg * ws_ * ws_;
        auto g = [p, q](Real u) { return ((u + p) * u + q) * u - 1.0; };

        // bracket the positive roots between the turning points and a root bound
        std::vector<Real> knots = {0.0, 1.0 + std::max({std::fabs(p), std::fabs(q), 1.0})};
        const Real disc = p * p - 3.0 * q;
        if (disc > 0.0) {
            for (Real t : {(-p - std::sqrt(disc)) / 3.0, (-p + std::sqrt(disc)) / 3.0})
                if (t > 0.0 && t < knots.back())
                    knots.push_back(t);
        }
        std::sort(knots.begin(), knots.end());

        Real u = Null<Real>();
        Brent brent;
        for (Size i = 1; i < knots.size(); ++i) {
            if (g(knots[i - 1]) * g(knots[i]) > 0.0)
                continue;
            const Real r = brent.solve(g, 1.0e-14, 0.5 * (knots[i - 1] + knots[i]),
                                       knots[i - 1], knots[i]);
            // several roots only if C_eta^2 >= 3/4 C_gamma C_omega: take the flattest smile
            if (u == Null<Real>() || std::fabs(r - 1.0) < std::fabs(u - 1.0))
                u = r;
        }
        QL_ENSURE(u != Null<Real>(), "no slope found at the singular point");
        bs_ = (u - 1.0) / ws_;

        h_ = stepFraction * ws_;

        std::vector<Real> xl, wl, bl, xr, wr, br;
        integrate(-1, std::min(xMin, xs_), xl, wl, bl);
        integrate(1, std::max(xMax, xs_), xr, wr, br);

        std::vector<Real> x(xl.rbegin(), xl.rend()), w(wl.rbegin(), wl.rend()),
            beta(bl.rbegin(), bl.rend());
        x.push_back(xs_);
        w.push_back(ws_);
        beta.push_back(bs_);
        x.insert(x.end(), xr.begin(), xr.end());
        w.insert(w.end(), wr.begin(), wr.end());
        beta.insert(beta.end(), br.begin(), br.end());
        setNodes(std::move(x), std::move(w), std::move(beta));
    }

    void FxStickyDeltaSmileSolver::coefficients(Real x, Real w, Real& a, Real& b, Real& c) const {
        const Real cg = costs_[0], ct = costs_[1], ce = costs_[2], cw = costs_[3];
        const Real pa = premiumAdjust_ ? 1.0 : -1.0;
        const Real w2 = w * w;
        const Real dpdm = x * x - w2 * w2 / 4.0;  // w^2 d+ d-
        a = cg * dpdm;
        b = 2.0 * cg * w * (x - w2 / 2.0) - 2.0 * ce * dpdm;
        c = (2.0 * ct * w + cg) * w2 - 2.0 * ce * w * (x + pa * w2 / 2.0) + cw * dpdm;
    }

    Real FxStickyDeltaSmileSolver::quadraticRoot(Real x, Real w, Real betaRef) const {
        Real a, b, c;
        coefficients(x, w, a, b, c);

        Real disc = b * b - 4.0 * a * c;
        if (disc < 0.0) {
            // allow for rounding where the two roots merge
            if (disc < -1.0e-10 * (b * b + std::fabs(4.0 * a * c)))
                return std::numeric_limits<Real>::quiet_NaN();
            disc = 0.0;
        }

        // numerically stable roots; the finite one survives a -> 0
        const Real s = -(b + std::copysign(std::sqrt(disc), b)) / 2.0;
        Real best = std::numeric_limits<Real>::quiet_NaN();
        auto consider = [&](Real r) {
            if (std::isnan(best) || std::fabs(r - betaRef) < std::fabs(best - betaRef))
                best = r;
        };
        if (s != 0.0)
            consider(c / s);
        if (a != 0.0)
            consider(s / a);
        return best;
    }

    void FxStickyDeltaSmileSolver::integrate(Integer side,
                                             Real xEnd,
                                             std::vector<Real>& x,
                                             std::vector<Real>& w,
                                             std::vector<Real>& beta) const {
        AdaptiveRungeKutta<Real> rk(accuracy_, h_ / 10.0, 0.0);

        Real xi = xs_, wi = ws_, bi = bs_;
        for (Size n = 1; side * (xi - xEnd) < 0.0; ++n) {
            const Real xn = xs_ + side * Real(n) * h_;

            // dw/dx = -beta; the slope at the starting node picks the branch,
            // and at the singular point the quadratic is 0/0 so use its slope
            const Real betaRef = bi;
            auto rhs = [this, betaRef](Real xx, Real ww) -> Real {
                if (xx == xs_)
                    return -bs_;
                return -quadraticRoot(xx, ww, betaRef);
            };

            Real wn;
            try {
                wn = rk(rhs, wi, xi, xn);
            } catch (Error&) {
                break;  // step size control failed, e.g. where the roots merge
            }
            // a missing root shows up as NaN: AdaptiveRungeKutta passes it through
            if (!std::isfinite(wn) || wn <= 0.0)
                break;
            const Real bn = quadraticRoot(xn, wn, betaRef);
            // beyond Lee's moment bound, |dw^2/dx| = 2 w |beta| <= 2, the wing is
            // arbitrageable; typically the root followed is running away near d+ = 0
            if (!std::isfinite(bn) || wn * std::fabs(bn) > 1.0)
                break;

            x.push_back(xn);
            w.push_back(wn);
            beta.push_back(bn);
            xi = xn;
            wi = wn;
            bi = bn;
        }
    }

    FxStickyDeltaConvexSmileSolver::FxStickyDeltaConvexSmileSolver(std::vector<Real> costs,
                                                                   bool premiumAdjust,
                                                                   Real xMin,
                                                                   Real xMax,
                                                                   Size nodes,
                                                                   Real tolerance)
    : FxStickyDeltaSmile(std::move(costs), premiumAdjust) {
        QL_REQUIRE(xMin < xMax, "empty log-moneyness range [" << xMin << ", " << xMax << "]");
        QL_REQUIRE(nodes >= 5, "at least 5 nodes required, " << nodes << " given");
        QL_REQUIRE(tolerance > 0.0, "tolerance must be positive, got " << tolerance);

        // the smile without convexity is the initial guess
        const FxStickyDeltaSmileSolver guess(costs_, premiumAdjust_, xMin, xMax);
        auto guessOn = [&](Size n) {
            std::vector<Real> w(n);
            for (Size i = 0; i < n; ++i)
                w[i] = guess.totalVol(xMin + (xMax - xMin) * Real(i) / Real(n - 1));
            return w;
        };
        const std::vector<Real> coarse = solve(xMin, xMax, guessOn(nodes), tolerance);
        const std::vector<Real> fine = solve(xMin, xMax, guessOn(2 * nodes - 1), tolerance);

        // Richardson extrapolation of the second-order scheme on the coarse nodes
        const Size n = nodes;
        const Real h = (xMax - xMin) / Real(n - 1);
        std::vector<Real> x(n), w(n), beta(n);
        for (Size i = 0; i < n; ++i) {
            x[i] = xMin + h * Real(i);
            w[i] = (4.0 * fine[2 * i] - coarse[i]) / 3.0;
        }
        // fourth-order differences for the slopes, one-sided near the ends
        for (Size i = 0; i < n; ++i) {
            Real dw;
            if (i == 0)
                dw = -25.0 * w[0] + 48.0 * w[1] - 36.0 * w[2] + 16.0 * w[3] - 3.0 * w[4];
            else if (i == 1)
                dw = -3.0 * w[0] - 10.0 * w[1] + 18.0 * w[2] - 6.0 * w[3] + w[4];
            else if (i == n - 2)
                dw = 3.0 * w[n - 1] + 10.0 * w[n - 2] - 18.0 * w[n - 3] + 6.0 * w[n - 4] -
                     w[n - 5];
            else if (i == n - 1)
                dw = 25.0 * w[n - 1] - 48.0 * w[n - 2] + 36.0 * w[n - 3] - 16.0 * w[n - 4] +
                     3.0 * w[n - 5];
            else
                dw = -w[i + 2] + 8.0 * w[i + 1] - 8.0 * w[i - 1] + w[i - 2];
            beta[i] = -dw / (12.0 * h);
        }
        setNodes(std::move(x), std::move(w), std::move(beta));
    }

    std::vector<Real> FxStickyDeltaConvexSmileSolver::solve(Real xMin,
                                                            Real xMax,
                                                            std::vector<Real> w,
                                                            Real tolerance) const {
        const Size n = w.size();
        const Real h = (xMax - xMin) / Real(n - 1), h2 = h * h;
        const Real cg = costs_[0];

        // w'' = G(x, w, w') with G = -F(x, w, -w') / (w C_g) + w'
        auto G = [&](Real x, Real wi, Real p) { return -costEquation(x, wi, -p) / (wi * cg) + p; };
        auto partials = [&](Real x, Real wi, Real p, Real& gw, Real& gp) {
            const Real ew = 1.0e-6 * wi, ep = 1.0e-6 * std::max(1.0, std::fabs(p));
            gw = (G(x, wi + ew, p) - G(x, wi - ew, p)) / (2.0 * ew);
            gp = (G(x, wi, p + ep) - G(x, wi, p - ep)) / (2.0 * ep);
        };

        Array residual(n), low(n - 1), mid(n), high(n - 1);
        Real previousMove = QL_MAX_REAL;
        for (Size iteration = 0; iteration < 50; ++iteration) {
            Real gw, gp;
            // w'' = 0 at the ends: with the ghost node w_{-1} = 2 w_0 - w_1 the
            // central difference for w' is the one-sided one, still second order
            {
                const Real p = (w[1] - w[0]) / h;
                residual[0] = G(xMin, w[0], p);
                partials(xMin, w[0], p, gw, gp);
                mid[0] = gw - gp / h;
                high[0] = gp / h;
            }
            for (Size i = 1; i < n - 1; ++i) {
                const Real x = xMin + h * Real(i);
                const Real p = (w[i + 1] - w[i - 1]) / (2.0 * h);
                residual[i] = (w[i + 1] - 2.0 * w[i] + w[i - 1]) / h2 - G(x, w[i], p);
                partials(x, w[i], p, gw, gp);
                low[i - 1] = 1.0 / h2 + gp / (2.0 * h);
                mid[i] = -2.0 / h2 - gw;
                high[i] = 1.0 / h2 - gp / (2.0 * h);
            }
            {
                const Real p = (w[n - 1] - w[n - 2]) / h;
                residual[n - 1] = G(xMax, w[n - 1], p);
                partials(xMax, w[n - 1], p, gw, gp);
                low[n - 2] = -gp / h;
                mid[n - 1] = gw + gp / h;
            }
            for (Real r : residual)
                QL_REQUIRE(std::isfinite(r),
                           "sticky-delta convex smile: non-finite residual for costs ["
                               << costs_[0] << ", " << costs_[1] << ", " << costs_[2] << ", "
                               << costs_[3] << "]");

            const Array step = TridiagonalOperator(low, mid, high).solveFor(-residual);

            // halve the step until every total vol stays positive
            Real lambda = 1.0;
            auto positive = [&](Real l) {
                for (Size i = 0; i < n; ++i)
                    if (w[i] + l * step[i] <= 0.0)
                        return false;
                return true;
            };
            while (!positive(lambda)) {
                lambda /= 2.0;
                QL_REQUIRE(lambda > 1.0e-6, "sticky-delta convex smile: Newton step keeps "
                                            "the total vol positive only when vanishing");
            }
            Real largestMove = 0.0, largestVol = 0.0;
            for (Size i = 0; i < n; ++i) {
                w[i] += lambda * step[i];
                largestMove = std::max(largestMove, std::fabs(lambda * step[i]));
                largestVol = std::max(largestVol, w[i]);
            }
            if (largestMove < tolerance * largestVol)
                return w;
            // a tolerance below rounding is never met: accept once the steps
            // stop shrinking at rounding level
            if (largestMove < 1.0e-10 * largestVol && largestMove >= previousMove)
                return w;
            previousMove = largestMove;
        }
        QL_FAIL("sticky-delta convex smile: Newton did not converge for costs ["
                << costs_[0] << ", " << costs_[1] << ", " << costs_[2] << ", " << costs_[3]
                << "]");
    }

    Real FxStickyDeltaConvexSmileSolver::convexity(Real x) const {
        const Real beta = slope(x);
        if (x < minLogMoneyness() || x > maxLogMoneyness())
            return beta / 2.0;  // w'' = 0 beyond the range
        const Real w = totalVol(x);
        return -costEquation(x, w, beta) / (2.0 * w * costs_[0]);
    }

}
