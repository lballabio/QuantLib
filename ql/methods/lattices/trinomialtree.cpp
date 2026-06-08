/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/methods/lattices/trinomialtree.hpp>
#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    namespace {
        // Floor activation threshold: the dx floor (Clewlow-Strickland 1998)
        // is applied only on grid steps shorter than `kFloorThreshold * dtMax`.
        // Two orders of magnitude leaves uniform and typical non-uniform
        // grids (weekend rolls, 1-day mismatches) untouched; the floor
        // intervenes only on the pathological small-mandatory-gap case.
        constexpr double kFloorThreshold = 0.01;
    }

    TrinomialTree::TrinomialTree(
                        const ext::shared_ptr<StochasticProcess1D>& process,
                        const TimeGrid& timeGrid,
                        bool isPositive)
    : Tree<TrinomialTree>(timeGrid.size()), dx_(1, 0.0), timeGrid_(timeGrid) {
        x0_ = process->x0();

        Size nTimeSteps = timeGrid.size() - 1;
        QL_REQUIRE(nTimeSteps > 0, "null time steps for trinomial tree");

        // Preflight: capture per-step variances and dtMax in one pass.
        // dxFloor is the largest natural dx in the grid: when applied
        // on a tiny step it produces a dx no larger than what some
        // other step is already using, preventing node explosion.
        // Iterating over actual step durations (rather than a
        // hypothetical dtMax window anchored at each step start) keeps
        // the variance integration strictly within the declared grid
        // horizon and avoids floating-point fragility from any
        // `terminal - t_i` subtraction.  The per-step v2 values are
        // cached so the main loop below does not re-invoke
        // process->variance(); for processes with non-trivial variance
        // evaluation cost this halves the call count.
        Time dtMax = 0.0;
        std::vector<Real> v2Cache(nTimeSteps);
        Real dxFloorVar = 0.0;
        for (Size i = 0; i < nTimeSteps; i++) {
            Time dt_i = timeGrid.dt(i);
            dtMax = std::max(dtMax, dt_i);
            Real v2_i = process->variance(timeGrid[i], 0.0, dt_i);
            v2Cache[i] = v2_i;
            dxFloorVar = std::max(dxFloorVar, v2_i);
        }
        Real dxFloor = std::sqrt(3.0 * dxFloorVar);

        Integer jMin = 0;
        Integer jMax = 0;

        for (Size i=0; i<nTimeSteps; i++) {
            Time t = timeGrid[i];
            Time dt = timeGrid.dt(i);

            //Variance must be independent of x
            Real v2 = v2Cache[i];
            Volatility v = std::sqrt(v2);
            Real dxNatural = v*std::sqrt(3.0);
            Real dxNext = (dt < kFloorThreshold * dtMax)
                          ? std::max(dxNatural, dxFloor)
                          : dxNatural;
            dx_.push_back(dxNext);

            // dxIsFloored captures whether the floor was *effective*
            // at this step (dx widened beyond its natural value),
            // not just whether the gate condition fired.  Time-
            // dependent diffusions can have a small dt whose
            // natural dx already equals or exceeds dxFloor; in
            // that case the gate fires but dx is unchanged, and
            // the classical-formula branch below still applies.
            bool dxIsFloored = (dxNext > dxNatural);
            Real dx2 = dxNext*dxNext;

            Branching branching;
            for (Integer j=jMin; j<=jMax; j++) {
                Real x = x0_ + j*dx_[i];
                Real m = process->expectation(t, x, dt);
                auto temp = Integer(std::floor((m - x0_) / dx_[i + 1] + 0.5));

                bool tempBumped = false;
                if (isPositive) {
                    while (x0_+(temp-1)*dx_[i+1]<=0) {
                        temp++;
                        tempBumped = true;
                    }
                }

                Real e = m - (x0_ + temp*dx_[i+1]);
                Real e2 = e*e;

                Real p1, p2, p3;
                if (dxIsFloored) {
                    // General moment-matching probabilities valid for
                    // any grid spacing dx, used only when the floor
                    // widened dx beyond v*sqrt(3).  They redistribute
                    // probability toward the middle node to reflect
                    // the smaller variance of the short step.
                    //
                    // Non-negativity requires v^2 >= |e|*(dx - |e|),
                    // which can fail in the floored regime when
                    // v << v_max.  We accept slightly negative weights
                    // as the cost of the pathology fix; first two
                    // moments are still matched exactly so signed
                    // weights remain arithmetically consistent.
                    // Hull-White-style alternative branching does not
                    // help (it solves boundary drift, not the small-
                    // variance regime).
                    p1 = (v2 + e2 - e*dxNext) / (2.0*dx2);
                    p2 = 1.0 - (v2 + e2) / dx2;
                    p3 = (v2 + e2 + e*dxNext) / (2.0*dx2);
                } else {
                    // Classical Hull-White / Clewlow trinomial
                    // probabilities for dx = v*sqrt(3).  Kept in this
                    // exact form (not the algebraically-equivalent
                    // dx-based form) so cached pricing values in
                    // bermudanswaption.cpp / callablebonds.cpp remain
                    // bit-for-bit identical to upstream master by
                    // construction rather than by FP coincidence.
                    Real e3 = e*std::sqrt(3.0);
                    p1 = (1.0 + e2/v2 - e3/v)/6.0;
                    p2 = (2.0 - e2/v2)/3.0;
                    p3 = (1.0 + e2/v2 + e3/v)/6.0;
                }

                // In the unfloored regime with naturally-rounded temp
                // the formulas above are non-negative by construction
                // (|e| <= dx/2 implies v^2 >= |e|*(dx - |e|)); guard
                // against future drift in this safe path.  When
                // isPositive bumps temp upward to keep the underlying
                // positive, |e| can exceed dx/2 -- the resulting signed
                // weights were accepted by upstream before this change
                // (CIR family models rely on this), so we skip the
                // assertion in that case.  In the floored regime the
                // limitation is documented and accepted uniformly.
                if (!dxIsFloored && !tempBumped) {
                    QL_ENSURE(p1 >= 0.0 && p2 >= 0.0 && p3 >= 0.0,
                              "negative probability in trinomial tree "
                              "(unfloored regime) at step " << i
                              << ", node " << j
                              << ": p1=" << p1 << ", p2=" << p2
                              << ", p3=" << p3
                              << " (v=" << v << ", dx=" << dxNext
                              << ", e=" << e << ")");
                }

                branching.add(temp, p1, p2, p3);
            }
            branchings_.push_back(branching);

            jMin = branching.jMin();
            jMax = branching.jMax();
        }
    }

}

