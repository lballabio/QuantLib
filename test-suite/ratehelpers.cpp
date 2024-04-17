// /* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// /*
//  Copyright (C) 2024 Paul Xi Cao

//  This file is part of QuantLib, a free-software/open-source library
//  for financial quantitative analysts and developers - http://quantlib.org/

//  QuantLib is free software: you can redistribute it and/or modify it
//  under the terms of the QuantLib license.  You should have received a
//  copy of the license along with this program; if not, please email
//  <quantlib-dev@lists.sf.net>. The license is also available online at
//  <http://quantlib.org/license.shtml>.

//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
//  FOR A PARTICULAR PURPOSE.  See the license for more details.
// */

// #include "toplevelfixture.hpp"
// #include "utilities.hpp"
// #include <ql/currencies/america.hpp>
// #include <ql/termstructures/yield/ratehelpers.hpp>
// #include <ql/time/calendars/weekendsonly.hpp>
// #include <ql/time/daycounters/actual360.hpp>

// using namespace QuantLib;
// using namespace boost::unit_test_framework;

// BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

// BOOST_AUTO_TEST_SUITE(RateHelpersTests)


// BOOST_AUTO_TEST_CASE(testSwapWithSwapBuilderArgument) {
//     BOOST_TEST_MESSAGE("Testing SwapRateHelper construction with MakeVanillaSwap...");

//     const Rate rate = 0.05;

//     const MakeVanillaSwap swapBuilder = []() -> MakeVanillaSwap {
//         const Period swapTenor = 3 * Months;
//         const ext::shared_ptr<IborIndex> iborIndex =
//             ext::make_shared<IborIndex>("IsdaIbor", swapTenor, /*settlementDays=*/2, USDCurrency(),
//                                         WeekendsOnly(), ModifiedFollowing, false, Actual360());
//         const Rate fixedRate = 0.07;
//         const Period forwardStart = 2 * Days;
//         return {swapTenor, iborIndex, fixedRate, forwardStart};
//     }();

//     const Handle<Quote> spread(ext::make_shared<SimpleQuote>(100));

//     const Pillar::Choice pillar = Pillar::LastRelevantDate;
//     Date customPillarDate = Date();

//     SwapRateHelper helper(rate, swapBuilder, spread, pillar, customPillarDate);

//     const ext::shared_ptr<VanillaSwap> swapPtr = helper.swap();
//     const VanillaSwap& swap = *swapPtr;

//     swap == swap;

//     swap.
// }

// BOOST_AUTO_TEST_CASE(testSwapWithExplicitArguments) {
//     BOOST_TEST_MESSAGE("Testing SwapRateHelper construction with explicit arguments (aka without "
//                        "MakeVanillaSwap)...");
// }


// BOOST_AUTO_TEST_SUITE_END()

// BOOST_AUTO_TEST_SUITE_END()
