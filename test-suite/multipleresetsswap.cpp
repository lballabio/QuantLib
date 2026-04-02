/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2026 Zain Mughal

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
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/instruments/makemultipleresetsswap.hpp>
#include <ql/termstructures/yield/multipleresetsswaphelper.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(MultipleResetsSwapTests)

struct CommonVars {
    Date today;
    Calendar calendar;
    DayCounter dayCount;
    RelinkableHandle<YieldTermStructure> termStructure;
    ext::shared_ptr<IborIndex> euribor3m;

    // Flat 5% curve, 3M Euribor, 2 resets per semiannual coupon.
    CommonVars() {
        calendar = TARGET();
        today = calendar.adjust(Date(15, January, 2024));
        Settings::instance().evaluationDate() = today;
        dayCount = Actual365Fixed();
        termStructure.linkTo(flatRate(today, 0.05, dayCount));
        euribor3m = ext::make_shared<Euribor3M>(termStructure);
        euribor3m->addFixing(Date(11, January, 2024), 0.05);
    }

    ext::shared_ptr<MultipleResetsSwap>
    makeSwap(Rate fixedRate, RateAveraging::Type method = RateAveraging::Compound) {
        return MakeMultipleResetsSwap(2 * Years, euribor3m, 2)
            .withFixedRate(fixedRate)
            .withSettlementDays(0)
            .withNominal(1.0e6)
            .withAveragingMethod(method);
    }
};


BOOST_AUTO_TEST_CASE(testFairRate) {
    BOOST_TEST_MESSAGE("Testing fair rate of multiple-resets swap...");

    CommonVars vars;

    ext::shared_ptr<MultipleResetsSwap> swap = vars.makeSwap(0.06);

    Rate fair = swap->fairRate();
    BOOST_REQUIRE(fair != Null<Rate>());

    // Rebuilding at the fair rate must give zero NPV.
    ext::shared_ptr<MultipleResetsSwap> fairSwap = vars.makeSwap(fair);
    QL_CHECK_SMALL(fairSwap->NPV(), 1.0e-8);

    // Cross-check: fixed-leg NPV + floating-leg NPV equals total NPV.
    Real npvCheck = swap->fixedLegNPV() + swap->floatingLegNPV();
    QL_CHECK_SMALL(npvCheck - swap->NPV(), 1.0e-10);

    // Omitting withFixedRate triggers auto-computation; NPV must be zero.
    ext::shared_ptr<MultipleResetsSwap> autoFair =
        MakeMultipleResetsSwap(2 * Years, vars.euribor3m, 2)
            .withSettlementDays(0)
            .withNominal(1.0e6);
    QL_CHECK_SMALL(autoFair->NPV(), 1.0e-8);
}


BOOST_AUTO_TEST_CASE(testConsistencyWithLeg) {
    BOOST_TEST_MESSAGE("Testing that multiple-resets swap NPV is consistent with legs NPV...");

    CommonVars vars;

    for (auto type : {Swap::Payer, Swap::Receiver}) {
        ext::shared_ptr<MultipleResetsSwap> swap =
            MakeMultipleResetsSwap(2 * Years, vars.euribor3m, 2)
                .withFixedRate(0.05)
                .withSettlementDays(0)
                .withNominal(1.0e6)
                .withType(type);

        Real legSum = swap->fixedLegNPV() + swap->floatingLegNPV();
        QL_CHECK_SMALL(legSum - swap->NPV(), 1.0e-10);
    }
}


BOOST_AUTO_TEST_CASE(testAveragingVsCompounding) {
    BOOST_TEST_MESSAGE("Testing averaging vs compounding in multiple-resets swaps...");

    CommonVars vars;

    Rate fixedRate = 0.05;
    auto swapCompound = vars.makeSwap(fixedRate, RateAveraging::Compound);
    auto swapAverage = vars.makeSwap(fixedRate, RateAveraging::Simple);

    BOOST_CHECK(std::abs(swapCompound->fairRate() - swapAverage->fairRate()) > 1.0e-10);
}


BOOST_AUTO_TEST_CASE(testRateHelper) {
    BOOST_TEST_MESSAGE("Testing bootstrapping using multiple-resets swap helpers...");

    CommonVars vars;

    // Build a flat curve from multiple-resets swap quotes at 1Y, 2Y, 3Y.
    // A flat 5% input should bootstrap to a flat 5% output.
    Rate inputRate = 0.05;
    std::vector<ext::shared_ptr<RateHelper>> helpers;
    for (const auto& tenor : {1 * Years, 2 * Years, 3 * Years}) {
        helpers.push_back(ext::make_shared<MultipleResetsSwapRateHelper>(
            0, tenor, Handle<Quote>(ext::make_shared<SimpleQuote>(inputRate)), vars.euribor3m, 2));
    }

    auto curve = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(vars.today, helpers,
                                                                            vars.dayCount);

    RelinkableHandle<YieldTermStructure> bootstrapped;
    bootstrapped.linkTo(curve);
    auto indexOnCurve = ext::make_shared<Euribor3M>(bootstrapped);

    const Real tolerance = 1.0e-6;
    for (const auto& tenor : {1 * Years, 2 * Years, 3 * Years}) {
        ext::shared_ptr<MultipleResetsSwap> check = MakeMultipleResetsSwap(tenor, indexOnCurve, 2)
                                                        .withFixedRate(0.0)
                                                        .withSettlementDays(0)
                                                        .withNominal(1.0e6)
                                                        .withDiscountingTermStructure(bootstrapped);
        Rate implied = check->fairRate();
        QL_CHECK_SMALL(implied - inputRate, tolerance);
    }
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
