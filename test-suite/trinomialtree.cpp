/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Richard Amaya

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/methods/lattices/trinomialtree.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/timegrid.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(TrinomialTreeTests)

BOOST_AUTO_TEST_CASE(testSmallMandatoryGapDoesNotExplode) {
    BOOST_TEST_MESSAGE(
        "Testing trinomial tree node count stays bounded with a "
        "small mandatory time gap...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;

    // Flat yield curve at 5%
    Handle<YieldTermStructure> termStructure(
        flatRate(today, 0.05, Actual365Fixed()));

    // Hull-White process: a=0.1, sigma=0.01
    auto model = ext::make_shared<HullWhite>(termStructure, 0.1, 0.01);
    ext::shared_ptr<StochasticProcess1D> process =
        model->dynamics()->process();

    // TimeGrid with one very short mandatory-time gap (1ms after t=1.0).
    // Without the global dx floor this causes node count to explode to
    // several hundred at the tiny-dt step; with the floor it stays bounded.
    std::vector<Time> mandatoryTimes = {1.0, 1.0 + 1.0e-3, 2.0, 3.0};
    TimeGrid grid(mandatoryTimes.begin(), mandatoryTimes.end());

    TrinomialTree tree(process, grid);

    Size maxNodes = 0;
    for (Size i = 0; i < grid.size(); ++i)
        maxNodes = std::max(maxNodes, tree.size(i));

    // The trinomial tree node count grows by at most one j-index per
    // side per step, so after nSteps steps maxNodes <= 2*nSteps + 1.
    // Without the floor, the tiny-dt step would force dx down enough
    // that the surrounding steps' integer offsets land far from j=0,
    // producing hundreds of nodes.  With the floor, this bound holds.
    Size nSteps = grid.size() - 1;
    Size expectedBound = 2*nSteps + 1;
    BOOST_CHECK_MESSAGE(maxNodes <= expectedBound,
        "trinomial tree exceeded derived bound: max node count "
        << maxNodes << " > 2*nSteps+1 = " << expectedBound
        << " (dx-floor fix may be broken)");

    // The grid contains a step short enough to activate the dx floor.
    // Per trinomialtree.cpp, the floored regime accepts slightly
    // negative probabilities as the documented cost of the pathology
    // fix (first two moments still match exactly).  Asserting `p in
    // [0, 1]` here would couple this regression test to the specific
    // HW parameters where |e| happens to stay small; bounded node
    // count is the actual invariant being validated.
}

BOOST_AUTO_TEST_CASE(testFloorThresholdBoundary) {
    BOOST_TEST_MESSAGE(
        "Testing trinomial tree behaviour at the floor activation "
        "threshold (dt_i / dt_max = 0.01)...");

    Date today(15, January, 2026);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> termStructure(
        flatRate(today, 0.05, Actual365Fixed()));
    auto model = ext::make_shared<HullWhite>(termStructure, 0.1, 0.01);
    ext::shared_ptr<StochasticProcess1D> process =
        model->dynamics()->process();

    // Helper: build a tree on a grid with one custom-ratio short step
    // and report:
    //  - maxNodes (peak j-range across the grid)
    //  - nSteps (so the bounded-node test can derive its own bound)
    //  - shortStepDx (the dx assigned to the short step, possibly floored)
    //  - shortStepNaturalDx (v*sqrt(3) for the short step, the unfloored
    //    value).
    // The two dx values let the boundary test positively assert the
    // floor's on/off state on each side of the threshold, catching
    // drift in either direction.
    struct Probe {
        Size maxNodes; Size nSteps;
        Real shortStepDx; Real shortStepNaturalDx;
    };
    auto runProbe = [&](Time gapRatio) -> Probe {
        Time dtMax = 1.0;
        std::vector<Time> times = {1.0, 1.0 + gapRatio*dtMax, 2.0, 3.0};
        TimeGrid grid(times.begin(), times.end());

        TrinomialTree tree(process, grid);

        // Locate the short step by scanning for the smallest dt: the
        // TimeGrid constructor prepends t=0 to the mandatory-time list
        // we pass, so the index of the short step is one greater than
        // its position in `times` above.  Scanning instead of
        // hard-coding the index keeps this test independent of that
        // detail and of any future reordering of `times`.  In the
        // constructor, dx_ starts at length 1 and step i's dx is
        // pushed into dx_[i+1], so the short step's dx lives at
        // tree.dx(shortIdx + 1).  Compute the reference natural dx in
        // the same operation order as production (sqrt(v2) * sqrt(3.0),
        // not sqrt(3.0 * v2)) so BOOST_CHECK_EQUAL below holds
        // bit-for-bit across compilers and optimisation levels rather
        // than relying on the two forms happening to agree in the
        // last bit.
        Size shortIdx = 0;
        for (Size i = 1; i < grid.size() - 1; ++i)
            if (grid.dt(i) < grid.dt(shortIdx))
                shortIdx = i;
        Time shortDt = grid.dt(shortIdx);
        Real shortV2 = process->variance(grid[shortIdx], 0.0, shortDt);
        Real shortNatural = std::sqrt(shortV2) * std::sqrt(3.0);

        Size maxNodes = 0;
        for (Size i = 0; i < grid.size(); ++i)
            maxNodes = std::max(maxNodes, tree.size(i));
        return {maxNodes, grid.size() - 1,
                tree.dx(shortIdx + 1), shortNatural};
    };

    // Probes are placed immediately on either side of the activation
    // threshold (0.01) so the test exercises the gating boundary
    // itself; loose multipliers (e.g. 0.005 / 0.02) would still pass
    // for any threshold in a wide range.
    constexpr Time kBelow = 0.0099;
    constexpr Time kAbove = 0.0101;

    // Just below threshold: floor active.  Node count must stay
    // bounded (each step extends the j-range by at most 1 on each
    // side) AND the short step's dx must be strictly larger than its
    // natural value, positively confirming the floor activated here.
    // Without the dx check, a `kFloorThreshold` that drifted *down*
    // could leave kBelow unfloored without this test noticing.
    auto belowResult = runProbe(kBelow);
    Size belowBound = 2*belowResult.nSteps + 1;
    BOOST_CHECK_MESSAGE(belowResult.maxNodes <= belowBound,
        "below-threshold node count " << belowResult.maxNodes
        << " exceeded derived bound 2*nSteps+1 = " << belowBound);
    BOOST_CHECK_MESSAGE(belowResult.shortStepDx > belowResult.shortStepNaturalDx,
        "below-threshold floor did not activate: dx="
        << belowResult.shortStepDx
        << " natural=" << belowResult.shortStepNaturalDx
        << " (kFloorThreshold may have drifted down)");

    // Just above threshold: floor inactive.  The constructor assigns
    // dx exactly equal to v*sqrt(3) in this regime, so strict equality
    // holds.  The check positively confirms the floor is OFF, catching
    // a `kFloorThreshold` that drifted *up*.  Any regression in the
    // unfloored-regime QL_ENSURE would also throw out of runProbe and
    // fail the test as an unhandled exception.
    auto aboveResult = runProbe(kAbove);
    BOOST_CHECK_EQUAL(aboveResult.shortStepDx, aboveResult.shortStepNaturalDx);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
