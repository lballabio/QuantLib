/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Kyrylo Protsenko

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
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/coupon.hpp>
#include <ql/experimental/termstructures/overnightindexedfundingratehelper.hpp>
#include <ql/indexes/ibor/aonia.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/australia.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(OvernightIndexedFundingRateHelperTests)

namespace {

    struct FundingQuote {
        Period tenor;
        Spread margin;
    };

    ext::shared_ptr<YieldTermStructure> flatCurve(const Date& referenceDate, Rate rate) {
        return ext::make_shared<FlatForward>(referenceDate, rate, Actual365Fixed(), Continuous);
    }

}

BOOST_AUTO_TEST_CASE(testUsdCofStyleBootstrap) {
    SavedSettings backup;
    Date today(30, September, 2025);
    Settings::instance().evaluationDate() = today;

    Calendar calendar = UnitedStates(UnitedStates::Settlement);
    Handle<YieldTermStructure> projection(flatCurve(today, 0.04));
    auto sofr = ext::make_shared<Sofr>(projection);

    const std::vector<FundingQuote> quotes = {
        {3 * Months, 0.0008750000}, {6 * Months, 0.0020333333}, {9 * Months, 0.0026166667},
        {1 * Years, 0.0032250000},  {2 * Years, 0.0053769087},  {3 * Years, 0.0068085774},
        {4 * Years, 0.0076010466},  {5 * Years, 0.0083935157},  {6 * Years, 0.0083935157},
        {7 * Years, 0.0083935157},  {8 * Years, 0.0083935157},  {9 * Years, 0.0083935157},
        {10 * Years, 0.0083935157}, {12 * Years, 0.0083935157}, {15 * Years, 0.0083935157},
        {20 * Years, 0.0083935157}, {25 * Years, 0.0083935157}, {30 * Years, 0.0083935157},
        {40 * Years, 0.0083935157}};

    std::vector<ext::shared_ptr<RateHelper>> helpers;
    std::vector<ext::shared_ptr<OvernightIndexedFundingRateHelper>> fundingHelpers;
    for (const auto& q : quotes) {
        auto helper = ext::make_shared<OvernightIndexedFundingRateHelper>(
            q.margin, q.tenor, 2, calendar, ModifiedFollowing, false, sofr, 3 * Months, Actual360(),
            2, true);
        helpers.push_back(helper);
        fundingHelpers.push_back(helper);
    }

    auto curve = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(today, helpers,
                                                                            Actual365Fixed());
    curve->discount(helpers.back()->pillarDate());

    Date start = calendar.advance(today, 2, Days, Following);
    for (Size i = 0; i < quotes.size(); ++i) {
        Date maturity = calendar.advance(start, quotes[i].tenor, ModifiedFollowing, false);
        Date expectedPillar = calendar.advance(maturity, 2, Days, Following);
        BOOST_CHECK_EQUAL(fundingHelpers[i]->pillarDate(), expectedPillar);
        Real calculated = fundingHelpers[i]->impliedQuote();
        if (std::fabs(calculated - quotes[i].margin) > 1.0e-11) {
            BOOST_ERROR("failed to reproduce the " << quotes[i].tenor << " funding margin\n"
                                                   << "    expected:   " << quotes[i].margin << "\n"
                                                   << "    calculated: " << calculated);
        }
    }
}

BOOST_AUTO_TEST_CASE(testLaggedCashFlowsAndDatedConstructor) {
    SavedSettings backup;
    Date today(30, September, 2025);
    Settings::instance().evaluationDate() = today;

    Calendar calendar = UnitedStates(UnitedStates::Settlement);
    Handle<YieldTermStructure> projection(flatCurve(today, 0.041));
    auto sofr = ext::make_shared<Sofr>(projection);
    Date start(2, October, 2025);
    Date end = start + 2 * Years;

    auto helper = ext::make_shared<OvernightIndexedFundingRateHelper>(
        0.003, start, end, calendar, ModifiedFollowing, false, sofr, 3 * Months, Actual360(), 2,
        true);
    auto discountCurve = flatCurve(today, 0.047);
    helper->setTermStructure(discountCurve.get());

    auto swap = helper->swap();
    const Leg& overnightLeg = swap->leg(0);
    const Leg& exchanges = swap->leg(1);
    Date expectedInitial = calendar.advance(start, 2, Days, Following);
    Date expectedFinal =
        calendar.advance(calendar.adjust(end, ModifiedFollowing), 2, Days, Following);

    BOOST_CHECK_EQUAL(exchanges.size(), 2);
    BOOST_CHECK_EQUAL(exchanges.front()->date(), expectedInitial);
    BOOST_CHECK_EQUAL(exchanges.back()->date(), expectedFinal);
    BOOST_CHECK_EQUAL(overnightLeg.back()->date(), expectedFinal);
    BOOST_CHECK_EQUAL(helper->maturityDate(), calendar.adjust(end, ModifiedFollowing));
    BOOST_CHECK_EQUAL(helper->pillarDate(), expectedFinal);

    Real overnightNpv = 0.0;
    Real annuity = 0.0;
    for (const auto& cf : overnightLeg) {
        auto coupon = ext::dynamic_pointer_cast<Coupon>(cf);
        BOOST_REQUIRE(coupon);
        DiscountFactor discount = discountCurve->discount(cf->date());
        overnightNpv += cf->amount() * discount;
        annuity += coupon->nominal() * coupon->accrualPeriod() * discount;
    }
    Real expectedMargin = (100.0 * discountCurve->discount(expectedInitial) -
                           100.0 * discountCurve->discount(expectedFinal) - overnightNpv) /
                          annuity;

    BOOST_CHECK_SMALL(helper->impliedQuote() - expectedMargin, 1.0e-13);
}

BOOST_AUTO_TEST_CASE(testSameDayOvernightPillar) {
    SavedSettings backup;
    Date today(15, July, 2026);
    Settings::instance().evaluationDate() = today;
    Settings::instance().includeReferenceDateEvents() = false;

    Calendar calendar = Australia();
    Handle<YieldTermStructure> projection(flatCurve(today, 0.04));
    auto aonia = ext::make_shared<Aonia>(projection);

    Spread margin = 0.0004;
    auto helper = ext::make_shared<OvernightIndexedFundingRateHelper>(
        margin, 1 * Days, 0, calendar, Following, false, aonia, 12 * Months,
        Actual365Fixed(), 0, true);
    std::vector<ext::shared_ptr<RateHelper>> helpers = {helper};
    auto curve = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(
        today, helpers, Actual365Fixed());

    BOOST_CHECK_EQUAL(helper->pillarDate(), Date(16, July, 2026));
    curve->discount(helper->pillarDate());
    BOOST_CHECK_SMALL(helper->impliedQuote() - margin, 1.0e-11);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
