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

/*! \file fxrootbracketing.hpp
    \brief Root finding for monotonic functions by walking from a start point
*/

#ifndef quantlib_fx_root_bracketing_hpp
#define quantlib_fx_root_bracketing_hpp

#include <ql/errors.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <cmath>
#include <utility>

namespace QuantLib::detail {

    //! Whether a function rises or falls with its argument.
    enum class Slope { Increasing, Decreasing };

    //! Brackets the root of a monotonic function, walking from a start.
    /*! The sign of \f$ f(x_0) \f$ and the slope give the side the root
        is on; the walk goes that way in steps starting at \c step and
        growing by half each time, until the sign changes.  Only points
        between \f$ x_0 \f$ and the root are visited, so the function
        need not be defined beyond the root.  Returns an interval whose
        ends have opposite signs, or \f$ (x_0, x_0) \f$ if \f$ x_0 \f$ is
        a root.  Fails if the function is not finite at a visited point,
        or no sign change is found within \c maxSteps steps.
    */
    template <class F>
    std::pair<Real, Real> bracketRoot(const F& f, Real x0, Real step, Slope slope,
                                      Size maxSteps = 100) {
        QL_REQUIRE(step > 0.0, "positive step required: " << step);
        Real f0 = f(x0);
        QL_REQUIRE(std::isfinite(f0), "function not finite at the start point " << x0);
        if (f0 == 0.0)
            return {x0, x0};

        // a decreasing function that is positive has its root above
        const bool up = (f0 > 0.0) == (slope == Slope::Decreasing);
        Real x = x0;
        for (Size i = 0; i < maxSteps; ++i) {
            const Real next = up ? x + step : x - step;
            const Real fNext = f(next);
            QL_REQUIRE(std::isfinite(fNext), "function not finite at " << next
                                                 << " while bracketing a root from " << x0);
            if ((fNext > 0.0) != (f0 > 0.0) || fNext == 0.0)
                return up ? std::make_pair(x, next) : std::make_pair(next, x);
            x = next;
            step *= 1.5;
        }
        QL_FAIL("no root found within " << maxSteps << " steps from " << x0
                                        << "; the function may not be monotonic");
    }

    //! Root of a monotonic function, bracketed by bracketRoot() and refined by Brent.
    template <class F>
    Real findRoot(const F& f, Real x0, Real step, Slope slope, Real accuracy,
                  Size maxSteps = 100) {
        const auto [lo, hi] = bracketRoot(f, x0, step, slope, maxSteps);
        if (lo == hi)
            return lo;
        Brent solver;
        solver.setMaxEvaluations(1000);
        return solver.solve(f, accuracy, 0.5 * (lo + hi), lo, hi);
    }

}

#endif
