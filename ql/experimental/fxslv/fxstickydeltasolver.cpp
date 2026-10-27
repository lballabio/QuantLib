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
#include <ql/math/ode/adaptiverungekutta.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/utilities/null.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

namespace QuantLib {

    FxStickyDeltaSmileSolver::FxStickyDeltaSmileSolver(std::vector<Real> costs,
                                                       bool premiumAdjust,
                                                       Real xMin,
                                                       Real xMax,
                                                       Real stepFraction,
                                                       Real accuracy)
    : costs_(std::move(costs)), premiumAdjust_(premiumAdjust), accuracy_(accuracy) {
        QL_REQUIRE(costs_.size() == 4, "four costs required, " << costs_.size() << " given");
        QL_REQUIRE(costs_[0] > 0.0, "gamma cost must be positive, got " << costs_[0]);
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

        // slope at the singular point: u = 1 + w * beta solves the monic cubic
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

        x_.assign(xl.rbegin(), xl.rend());
        w_.assign(wl.rbegin(), wl.rend());
        beta_.assign(bl.rbegin(), bl.rend());
        x_.push_back(xs_);
        w_.push_back(ws_);
        beta_.push_back(bs_);
        x_.insert(x_.end(), xr.begin(), xr.end());
        w_.insert(w_.end(), wr.begin(), wr.end());
        beta_.insert(beta_.end(), br.begin(), br.end());
    }

    bool FxStickyDeltaSmileSolver::hasCosts(const std::vector<Real>& costs) const {
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
            if (!std::isfinite(bn))
                break;

            x.push_back(xn);
            w.push_back(wn);
            beta.push_back(bn);
            xi = xn;
            wi = wn;
            bi = bn;
        }
    }

    Real FxStickyDeltaSmileSolver::totalVol(Real x) const {
        if (x <= x_.front() || x >= x_.back()) {
            // linear extrapolation beyond the integrated range
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

    Real FxStickyDeltaSmileSolver::slope(Real x) const {
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

}
