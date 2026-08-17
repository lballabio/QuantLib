/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 StatPro Italia srl
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
#include "utilities.hpp"
#include <ql/experimental/termstructures/basisswapratehelpers.hpp>
#include <ql/indexes/ibor/fedfunds.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/instruments/swap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/unitedstates.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(BasisSwapRateHelpersTests)

struct BasisSwapQuote {
    Integer n;
    TimeUnit units;
    Spread basis;
};

void testIborIborBootstrap(bool bootstrapBaseCurve, Integer paymentLag = 0) {
    std::vector<BasisSwapQuote> quotes = {
        { 1, Years,  0.0010 },
        { 2, Years,  0.0012 },
        { 3, Years,  0.0015 },
        { 5, Years,  0.0015 },
        { 8, Years,  0.0018 },
        { 10, Years, 0.0020 },
        { 15, Years, 0.0021 },
        { 20, Years, 0.0021 },
    };

    auto settlementDays = 2;
    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    auto convention = Following;
    auto endOfMonth = false;

    Handle<YieldTermStructure> knownForecastCurve(flatRate(0.01, Actual365Fixed()));
    Handle<YieldTermStructure> discountCurve(flatRate(0.005, Actual365Fixed()));

    ext::shared_ptr<IborIndex> baseIndex, otherIndex;

    if (bootstrapBaseCurve) {
        baseIndex = ext::make_shared<USDLibor>(3 * Months);
        otherIndex = ext::make_shared<USDLibor>(6 * Months, knownForecastCurve);
    } else {
        baseIndex = ext::make_shared<USDLibor>(3 * Months, knownForecastCurve);
        otherIndex = ext::make_shared<USDLibor>(6 * Months);
    }

    std::vector<ext::shared_ptr<RateHelper>> helpers;
    for (auto q : quotes) {
        auto h = ext::make_shared<IborIborBasisSwapRateHelper>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(q.basis)),
                Period(q.n, q.units), settlementDays, calendar, convention, endOfMonth,
                baseIndex, otherIndex, discountCurve, bootstrapBaseCurve, std::nullopt,
                DateGeneration::Backward, paymentLag);
        helpers.push_back(h);
    }

    auto bootstrappedCurve = ext::make_shared<PiecewiseYieldCurve<ZeroYield, Linear>>
        (0, calendar, helpers, Actual365Fixed());

    Date today = Settings::instance().evaluationDate();
    Date spot = calendar.advance(today, settlementDays, Days);

    if (bootstrapBaseCurve) {
        baseIndex = ext::make_shared<USDLibor>(3 * Months, Handle<YieldTermStructure>(bootstrappedCurve));
        otherIndex = ext::make_shared<USDLibor>(6 * Months, knownForecastCurve);
    } else {
        baseIndex = ext::make_shared<USDLibor>(3 * Months, knownForecastCurve);
        otherIndex = ext::make_shared<USDLibor>(6 * Months, Handle<YieldTermStructure>(bootstrappedCurve));
    }

    for (auto q : quotes) {
        // create swaps and check they're fair
        Date maturity = spot + Period(q.n, q.units);

        Schedule s1 =
            MakeSchedule()
            .from(spot).to(maturity)
            .withTenor(baseIndex->tenor())
            .withCalendar(calendar)
            .withConvention(convention)
            .withRule(DateGeneration::Backward);
        Leg leg1 = IborLeg(s1, baseIndex)
            .withSpreads(q.basis)
            .withNotionals(100.0)
            .withPaymentLag(paymentLag);

        Schedule s2 =
            MakeSchedule()
            .from(spot).to(maturity)
            .withTenor(otherIndex->tenor())
            .withCalendar(calendar)
            .withConvention(convention)
            .withRule(DateGeneration::Backward);
        Leg leg2 = IborLeg(s2, otherIndex)
            .withNotionals(100.0)
            .withPaymentLag(paymentLag);

        Swap swap(leg1, leg2);
        swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(discountCurve));

        Real NPV = swap.NPV();
        Real tolerance = 1e-8;
        if (std::fabs(NPV) > tolerance) {
            BOOST_ERROR("Failed to price fair " << q.n << "-year(s) swap:"
                        << "\n    calculated: " << NPV);
        }
    }
}

void testOvernightIborBootstrap(bool externalDiscountCurve,
                                bool bootstrapBaseCurve = false,
                                Integer paymentLag = 0,
                                bool linkDiscountCurveAfterConstruction = false,
                                RateAveraging::Type averagingMethod = RateAveraging::Compound,
                                bool telescopicValueDates = false) {
    std::vector<BasisSwapQuote> quotes = {
        { 1, Years,  0.0010 },
        { 2, Years,  0.0012 },
        { 3, Years,  0.0015 },
        { 5, Years,  0.0015 },
        { 8, Years,  0.0018 },
        { 10, Years, 0.0020 },
        { 15, Years, 0.0021 },
        { 20, Years, 0.0021 },
    };

    auto settlementDays = 2;
    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    auto convention = Following;
    auto endOfMonth = false;

    Handle<YieldTermStructure> knownForecastCurve(flatRate(0.01, Actual365Fixed()));

    RelinkableHandle<YieldTermStructure> discountCurve;
    if (externalDiscountCurve && !linkDiscountCurveAfterConstruction)
        discountCurve.linkTo(flatRate(0.005, Actual365Fixed()));

    ext::shared_ptr<OvernightIndex> baseIndex;
    ext::shared_ptr<IborIndex> otherIndex;

    if (bootstrapBaseCurve) {
        baseIndex = ext::make_shared<Sofr>();
        otherIndex = ext::make_shared<USDLibor>(6 * Months, knownForecastCurve);
    } else {
        baseIndex = ext::make_shared<Sofr>(knownForecastCurve);
        otherIndex = ext::make_shared<USDLibor>(6 * Months);
    }

    std::vector<ext::shared_ptr<RateHelper>> helpers;
    for (auto q : quotes) {
        auto h = ext::make_shared<OvernightIborBasisSwapRateHelper>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(q.basis)),
                Period(q.n, q.units), settlementDays, calendar, convention, endOfMonth,
                baseIndex, otherIndex, discountCurve, bootstrapBaseCurve, paymentLag,
                std::nullopt, std::nullopt, DateGeneration::Backward, averagingMethod,
                telescopicValueDates);
        helpers.push_back(h);
    }

    auto bootstrappedCurve = ext::make_shared<PiecewiseYieldCurve<ZeroYield, Linear>>
        (0, calendar, helpers, Actual365Fixed());

    if (linkDiscountCurveAfterConstruction) {
        bootstrappedCurve->discount(helpers.back()->pillarDate());
        discountCurve.linkTo(flatRate(0.005, Actual365Fixed()));
    }

    Date today = Settings::instance().evaluationDate();
    Date spot = calendar.advance(today, settlementDays, Days);

    if (bootstrapBaseCurve) {
        baseIndex = ext::make_shared<Sofr>(Handle<YieldTermStructure>(bootstrappedCurve));
        otherIndex = ext::make_shared<USDLibor>(6 * Months, knownForecastCurve);
    } else {
        baseIndex = ext::make_shared<Sofr>(knownForecastCurve);
        otherIndex = ext::make_shared<USDLibor>(6 * Months, Handle<YieldTermStructure>(bootstrappedCurve));
    }

    for (auto q : quotes) {
        // create swaps and check they're fair
        Date maturity = spot + Period(q.n, q.units);

        Schedule s =
            MakeSchedule()
            .from(spot).to(maturity)
            .withTenor(otherIndex->tenor())
            .withCalendar(calendar)
            .withConvention(convention)
            .withRule(DateGeneration::Backward);

        Leg leg1 = OvernightLeg(s, baseIndex)
            .withSpreads(q.basis)
            .withNotionals(100.0)
            .withPaymentLag(paymentLag)
            .withTelescopicValueDates(
                telescopicValueDates && averagingMethod == RateAveraging::Compound)
            .withAveragingMethod(averagingMethod);
        Leg leg2 = IborLeg(s, otherIndex)
            .withNotionals(100.0)
            .withPaymentLag(paymentLag);

        Swap swap(leg1, leg2);
        if (externalDiscountCurve) {
            swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(discountCurve));
        } else {
            swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(
                                                                          Handle<YieldTermStructure>(bootstrappedCurve)));
        }

        Real NPV = swap.NPV();
        Real tolerance = 1e-8;
        if (std::fabs(NPV) > tolerance) {
            BOOST_ERROR("Failed to price fair " << q.n << "-year(s) swap:"
                        << "\n    calculated: " << NPV);
        }
    }
}

void testOvernightOvernightBootstrap(bool externalDiscountCurve,
                                     bool bootstrapBaseCurve,
                                     bool linkDiscountCurveAfterConstruction = false) {
    std::vector<BasisSwapQuote> quotes = {
        { 1, Years, 0.0008 },
        { 2, Years, 0.0010 },
        { 3, Years, 0.0011 },
        { 5, Years, 0.0013 },
        { 10, Years, 0.0015 },
    };

    auto settlementDays = 2;
    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    auto convention = Following;
    auto endOfMonth = false;
    auto paymentLag = 2;
    auto paymentFrequency = Quarterly;
    auto baseAveragingMethod = RateAveraging::Simple;
    auto otherAveragingMethod = RateAveraging::Compound;

    Handle<YieldTermStructure> knownForecastCurve(flatRate(0.01, Actual365Fixed()));

    RelinkableHandle<YieldTermStructure> discountCurve;
    if (externalDiscountCurve && !linkDiscountCurveAfterConstruction)
        discountCurve.linkTo(flatRate(0.005, Actual365Fixed()));

    ext::shared_ptr<OvernightIndex> baseIndex, otherIndex;
    if (bootstrapBaseCurve) {
        baseIndex = ext::make_shared<FedFunds>();
        otherIndex = ext::make_shared<Sofr>(knownForecastCurve);
    } else {
        baseIndex = ext::make_shared<FedFunds>(knownForecastCurve);
        otherIndex = ext::make_shared<Sofr>();
    }

    std::vector<ext::shared_ptr<OvernightOvernightBasisSwapRateHelper>> basisHelpers;
    std::vector<ext::shared_ptr<RateHelper>> helpers;
    for (auto q : quotes) {
        auto h = ext::make_shared<OvernightOvernightBasisSwapRateHelper>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(q.basis)),
            Period(q.n, q.units), settlementDays, calendar, convention, endOfMonth,
            baseIndex, otherIndex, discountCurve, bootstrapBaseCurve, paymentLag,
            paymentFrequency, baseAveragingMethod, otherAveragingMethod);
        basisHelpers.push_back(h);
        helpers.push_back(h);
    }

    auto firstBaseCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
        basisHelpers.front()->swap()->leg(0).front());
    auto firstOtherCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
        basisHelpers.front()->swap()->leg(1).front());
    BOOST_REQUIRE(firstBaseCoupon);
    BOOST_REQUIRE(firstOtherCoupon);
    BOOST_CHECK_EQUAL(firstBaseCoupon->averagingMethod(), baseAveragingMethod);
    BOOST_CHECK_EQUAL(firstOtherCoupon->averagingMethod(), otherAveragingMethod);
    BOOST_CHECK_EQUAL(basisHelpers.front()->swap()->leg(0).size(), 4);
    BOOST_CHECK_EQUAL(basisHelpers.front()->swap()->leg(1).size(), 4);

    auto bootstrappedCurve = ext::make_shared<PiecewiseYieldCurve<ZeroYield, Linear>>(
        0, calendar, helpers, Actual365Fixed());
    Handle<YieldTermStructure> bootstrappedCurveHandle(bootstrappedCurve);

    if (linkDiscountCurveAfterConstruction) {
        bootstrappedCurve->discount(basisHelpers.back()->pillarDate());
        discountCurve.linkTo(flatRate(0.005, Actual365Fixed()));
    }

    if (bootstrapBaseCurve) {
        baseIndex = ext::make_shared<FedFunds>(bootstrappedCurveHandle);
        otherIndex = ext::make_shared<Sofr>(knownForecastCurve);
    } else {
        baseIndex = ext::make_shared<FedFunds>(knownForecastCurve);
        otherIndex = ext::make_shared<Sofr>(bootstrappedCurveHandle);
    }

    Date today = Settings::instance().evaluationDate();
    Date spot = calendar.advance(today, settlementDays, Days);

    for (Size i = 0; i < quotes.size(); ++i) {
        const auto& q = quotes[i];
        Date maturity = calendar.advance(spot, q.n, q.units, convention);

        Schedule schedule =
            MakeSchedule().from(spot).to(maturity)
            .withFrequency(paymentFrequency)
            .withCalendar(calendar)
            .withConvention(convention)
            .withRule(DateGeneration::Forward);

        Leg baseLeg = OvernightLeg(schedule, baseIndex)
            .withNotionals(100.0)
            .withSpreads(q.basis)
            .withPaymentLag(paymentLag)
            .withAveragingMethod(baseAveragingMethod);
        Leg otherLeg = OvernightLeg(schedule, otherIndex)
            .withNotionals(100.0)
            .withPaymentLag(paymentLag)
            .withAveragingMethod(otherAveragingMethod);

        Swap swap(baseLeg, otherLeg);
        swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(
            externalDiscountCurve ? discountCurve : bootstrappedCurveHandle));

        Real tolerance = 1e-8;
        BOOST_CHECK_SMALL(swap.NPV(), tolerance);
        BOOST_CHECK_SMALL(basisHelpers[i]->impliedQuote() - q.basis, tolerance);
    }
}


BOOST_AUTO_TEST_CASE(testIborIborBaseCurveBootstrap) {
    BOOST_TEST_MESSAGE("Testing IBOR-IBOR basis-swap rate helpers (base curve bootstrap)...");

    testIborIborBootstrap(true);
}

BOOST_AUTO_TEST_CASE(testIborIborOtherCurveBootstrap) {
    BOOST_TEST_MESSAGE("Testing IBOR-IBOR basis-swap rate helpers (other curve bootstrap)...");

    testIborIborBootstrap(false);
}

BOOST_AUTO_TEST_CASE(testIborIborBootstrapWithPaymentLag) {
    BOOST_TEST_MESSAGE("Testing IBOR-IBOR basis-swap rate helpers with payment lag...");

    testIborIborBootstrap(true, 2);
    testIborIborBootstrap(false, 2);
}

BOOST_AUTO_TEST_CASE(testBasisSwapRateHelpersRejectNullIndexes) {
    BOOST_TEST_MESSAGE("Testing basis-swap rate helpers with null indexes...");

    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    Handle<YieldTermStructure> curve(flatRate(0.01, Actual365Fixed()));
    Handle<Quote> basis(ext::make_shared<SimpleQuote>(0.0010));
    auto overnightIndex = ext::make_shared<Sofr>(curve);
    auto iborIndex = ext::make_shared<USDLibor>(3 * Months, curve);
    ext::shared_ptr<OvernightIndex> nullOvernightIndex;
    ext::shared_ptr<IborIndex> nullIborIndex;

    auto makeIborIborHelper = [&](const ext::shared_ptr<IborIndex>& baseIndex,
                                  const ext::shared_ptr<IborIndex>& otherIndex) {
        return ext::make_shared<IborIborBasisSwapRateHelper>(
            basis, 2 * Years, 2, calendar, Following, false, baseIndex, otherIndex,
            curve, false);
    };
    auto makeOvernightIborHelper = [&](const ext::shared_ptr<OvernightIndex>& baseIndex,
                                       const ext::shared_ptr<IborIndex>& otherIndex) {
        return ext::make_shared<OvernightIborBasisSwapRateHelper>(
            basis, 2 * Years, 2, calendar, Following, false, baseIndex, otherIndex,
            curve, false);
    };

    BOOST_CHECK_THROW(makeIborIborHelper(nullIborIndex, iborIndex), Error);
    BOOST_CHECK_THROW(makeIborIborHelper(iborIndex, nullIborIndex), Error);
    BOOST_CHECK_THROW(makeOvernightIborHelper(nullOvernightIndex, iborIndex), Error);
    BOOST_CHECK_THROW(makeOvernightIborHelper(overnightIndex, nullIborIndex), Error);
}

BOOST_AUTO_TEST_CASE(testIborIborIndexedCouponOverride) {
    BOOST_TEST_MESSAGE("Testing the indexed-coupon override for IBOR-IBOR basis-swap rate helpers...");

    struct IborCouponSettingsRestorer {
        bool initiallyUsingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();
        ~IborCouponSettingsRestorer() {
            if (initiallyUsingAtParCoupons)
                IborCoupon::Settings::instance().createAtParCoupons();
            else
                IborCoupon::Settings::instance().createIndexedCoupons();
        }
    } settingsRestorer;

    // spot is the last UK business day of the month, so the end-of-month rule
    // of the index makes the two dates below differ; other dates may not
    Settings::instance().evaluationDate() = Date(27, January, 2010);
    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    Handle<YieldTermStructure> curve(flatRate(0.01, Actual365Fixed()));
    auto baseIndex = ext::make_shared<USDLibor>(3 * Months, curve);
    auto otherIndex = ext::make_shared<USDLibor>(6 * Months, curve);
    Handle<Quote> basis(ext::make_shared<SimpleQuote>(0.0010));

    // each helper is built against the opposite global setting, so the checks
    // below can only pass if the override wins
    IborCoupon::Settings::instance().createAtParCoupons();
    IborIborBasisSwapRateHelper indexedHelper(basis, 2 * Years, 2, calendar, Following, false,
                                               baseIndex, otherIndex, curve, false, true);

    IborCoupon::Settings::instance().createIndexedCoupons();
    IborIborBasisSwapRateHelper atParHelper(basis, 2 * Years, 2, calendar, Following, false,
                                            baseIndex, otherIndex, curve, false, false);

    for (Size i = 0; i < 2; ++i) {
        auto indexedCoupon =
            ext::dynamic_pointer_cast<IborCoupon>(indexedHelper.swap()->leg(i).front());
        auto atParCoupon = ext::dynamic_pointer_cast<IborCoupon>(atParHelper.swap()->leg(i).front());
        BOOST_REQUIRE(indexedCoupon);
        BOOST_REQUIRE(atParCoupon);
        BOOST_CHECK_EQUAL(indexedCoupon->fixingEndDate(), indexedCoupon->fixingMaturityDate());
        BOOST_CHECK_NE(atParCoupon->fixingEndDate(), atParCoupon->fixingMaturityDate());
    }
}

BOOST_AUTO_TEST_CASE(testOvernightIborDifferentPaymentFrequencies) {
    BOOST_TEST_MESSAGE(
        "Testing overnight-IBOR basis-swap rate helpers with different payment frequencies...");

    Settings::instance().evaluationDate() = Date(27, January, 2010);
    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    Handle<YieldTermStructure> curve(flatRate(0.01, Actual365Fixed()));
    auto overnightIndex = ext::make_shared<Sofr>(curve);
    auto iborIndex = ext::make_shared<USDLibor>(3 * Months, curve);

    // the overnight leg pays annually, the ibor leg at the tenor of its index
    OvernightIborBasisSwapRateHelper helper(
        Handle<Quote>(ext::make_shared<SimpleQuote>(0.0010)), 2 * Years, 2, calendar, Following,
        false, overnightIndex, iborIndex, curve, false, 0, Annual);
    OvernightIborBasisSwapRateHelper zeroCouponHelper(
        Handle<Quote>(ext::make_shared<SimpleQuote>(0.0010)), 2 * Years, 2, calendar, Following,
        false, overnightIndex, iborIndex, curve, false, 0, NoFrequency);

    BOOST_CHECK_EQUAL(helper.swap()->leg(0).size(), 2);
    BOOST_CHECK_EQUAL(helper.swap()->leg(1).size(), 8);
    BOOST_CHECK_EQUAL(zeroCouponHelper.swap()->leg(0).size(), 1);
    BOOST_CHECK_EQUAL(zeroCouponHelper.swap()->leg(1).size(), 8);
}

BOOST_AUTO_TEST_CASE(testOvernightIborIndexedCouponOverride) {
    BOOST_TEST_MESSAGE(
        "Testing the indexed-coupon override for overnight-IBOR basis-swap rate helpers...");

    struct IborCouponSettingsRestorer {
        bool initiallyUsingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();
        ~IborCouponSettingsRestorer() {
            if (initiallyUsingAtParCoupons)
                IborCoupon::Settings::instance().createAtParCoupons();
            else
                IborCoupon::Settings::instance().createIndexedCoupons();
        }
    } settingsRestorer;

    // see testIborIborIndexedCouponOverride for the choice of date
    Settings::instance().evaluationDate() = Date(27, January, 2010);
    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    Handle<YieldTermStructure> curve(flatRate(0.01, Actual365Fixed()));
    auto overnightIndex = ext::make_shared<Sofr>(curve);
    auto iborIndex = ext::make_shared<USDLibor>(3 * Months, curve);
    Handle<Quote> basis(ext::make_shared<SimpleQuote>(0.0010));

    IborCoupon::Settings::instance().createAtParCoupons();
    OvernightIborBasisSwapRateHelper indexedHelper(basis, 2 * Years, 2, calendar, Following, false,
                                                   overnightIndex, iborIndex, curve, false, 0,
                                                   Annual, true);

    IborCoupon::Settings::instance().createIndexedCoupons();
    OvernightIborBasisSwapRateHelper atParHelper(basis, 2 * Years, 2, calendar, Following, false,
                                                 overnightIndex, iborIndex, curve, false, 0, Annual,
                                                 false);

    auto indexedCoupon =
        ext::dynamic_pointer_cast<IborCoupon>(indexedHelper.swap()->leg(1).front());
    auto atParCoupon = ext::dynamic_pointer_cast<IborCoupon>(atParHelper.swap()->leg(1).front());
    BOOST_REQUIRE(indexedCoupon);
    BOOST_REQUIRE(atParCoupon);
    BOOST_CHECK_EQUAL(indexedCoupon->fixingEndDate(), indexedCoupon->fixingMaturityDate());
    BOOST_CHECK_NE(atParCoupon->fixingEndDate(), atParCoupon->fixingMaturityDate());
}

BOOST_AUTO_TEST_CASE(testOvernightIborDateGenerationRule) {
    BOOST_TEST_MESSAGE(
        "Testing the date-generation rule of overnight-IBOR basis-swap rate helpers...");

    Settings::instance().evaluationDate() = Date(27, January, 2010);
    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    Handle<YieldTermStructure> curve(flatRate(0.01, Actual365Fixed()));
    auto overnightIndex = ext::make_shared<Sofr>(curve);
    // an annual index, so that both legs have the same stub
    auto iborIndex = ext::make_shared<USDLibor>(1 * Years, curve);
    Handle<Quote> basis(ext::make_shared<SimpleQuote>(0.0010));

    Date spot = calendar.advance(Settings::instance().evaluationDate(), 2, Days);

    // 18M against an annual frequency leaves a six-month stub
    OvernightIborBasisSwapRateHelper backwardHelper(
        basis, 18 * Months, 2, calendar, Following, false, overnightIndex, iborIndex, curve, false,
        0, Annual, std::nullopt, DateGeneration::Backward);

    OvernightIborBasisSwapRateHelper forwardHelper(
        basis, 18 * Months, 2, calendar, Following, false, overnightIndex, iborIndex, curve, false,
        0, Annual, std::nullopt, DateGeneration::Forward);

    auto firstAccrualEnd = [](const OvernightIborBasisSwapRateHelper& h, Size leg) {
        return ext::dynamic_pointer_cast<Coupon>(h.swap()->leg(leg).front())->accrualEndDate();
    };

    for (Size i = 0; i < 2; ++i) {
        BOOST_CHECK_EQUAL(backwardHelper.swap()->leg(i).size(), 2);
        BOOST_CHECK_EQUAL(forwardHelper.swap()->leg(i).size(), 2);
        BOOST_CHECK_EQUAL(firstAccrualEnd(backwardHelper, i),
                          calendar.adjust(spot + 6 * Months, Following));
        BOOST_CHECK_EQUAL(firstAccrualEnd(forwardHelper, i),
                          calendar.adjust(spot + 1 * Years, Following));
    }

    Date expectedMaturity = calendar.adjust(spot + 18 * Months, Following);
    BOOST_CHECK_EQUAL(backwardHelper.maturityDate(), expectedMaturity);
    BOOST_CHECK_EQUAL(forwardHelper.maturityDate(), expectedMaturity);
}

BOOST_AUTO_TEST_CASE(testOvernightIborBootstrapWithoutDiscountCurve) {
    BOOST_TEST_MESSAGE("Testing overnight-IBOR basis-swap rate helpers...");

    testOvernightIborBootstrap(false);
}

BOOST_AUTO_TEST_CASE(testOvernightIborBootstrapWithDiscountCurve) {
    BOOST_TEST_MESSAGE("Testing overnight-IBOR basis-swap rate helpers with external discount curve...");

    testOvernightIborBootstrap(true);
}

BOOST_AUTO_TEST_CASE(testOvernightIborBootstrapWithLateLinkedDiscountCurve) {
    BOOST_TEST_MESSAGE("Testing overnight-IBOR basis-swap rate helpers with a "
                       "late-linked discount curve...");

    testOvernightIborBootstrap(true, false, 0, true);
}

BOOST_AUTO_TEST_CASE(testOvernightIborBaseCurveBootstrapWithoutDiscountCurve) {
    BOOST_TEST_MESSAGE("Testing overnight-IBOR basis-swap rate helpers (base curve bootstrap)...");

    testOvernightIborBootstrap(false, true);
}

BOOST_AUTO_TEST_CASE(testOvernightIborBaseCurveBootstrapWithDiscountCurve) {
    BOOST_TEST_MESSAGE("Testing overnight-IBOR basis-swap rate helpers "
                       "(base curve bootstrap with external discount curve)...");

    testOvernightIborBootstrap(true, true);
}

BOOST_AUTO_TEST_CASE(testOvernightIborBootstrapWithPaymentLag) {
    BOOST_TEST_MESSAGE("Testing overnight-IBOR basis-swap rate helpers with payment lag...");

    testOvernightIborBootstrap(false, false, 2);
    testOvernightIborBootstrap(true, false, 2);
    testOvernightIborBootstrap(false, true, 2);
    testOvernightIborBootstrap(true, true, 2);
}

BOOST_AUTO_TEST_CASE(testOvernightIborAveragingAndTelescopicValueDates) {
    BOOST_TEST_MESSAGE(
        "Testing averaging and telescopic value dates for overnight-IBOR helpers...");

    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    Handle<YieldTermStructure> curve(flatRate(0.01, Actual365Fixed()));
    auto overnightIndex = ext::make_shared<Sofr>(curve);
    auto iborIndex = ext::make_shared<USDLibor>(3 * Months, curve);
    Handle<Quote> basis(ext::make_shared<SimpleQuote>(0.0010));

    auto makeHelper = [&](RateAveraging::Type averagingMethod, bool telescopicValueDates) {
        return ext::make_shared<OvernightIborBasisSwapRateHelper>(
            basis, 2 * Years, 2, calendar, Following, false, overnightIndex,
            iborIndex, curve, false, 0, Annual, std::nullopt,
            DateGeneration::Backward, averagingMethod, telescopicValueDates);
    };

    auto fullSimpleHelper = makeHelper(RateAveraging::Simple, false);
    auto telescopicSimpleHelper = makeHelper(RateAveraging::Simple, true);
    auto fullCompoundedHelper = makeHelper(RateAveraging::Compound, false);
    auto telescopicCompoundedHelper = makeHelper(RateAveraging::Compound, true);

    auto overnightCoupon = [](const OvernightIborBasisSwapRateHelper& helper) {
        return ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
            helper.swap()->leg(0).front());
    };
    auto fullSimpleCoupon = overnightCoupon(*fullSimpleHelper);
    auto telescopicSimpleCoupon = overnightCoupon(*telescopicSimpleHelper);
    auto fullCompoundedCoupon = overnightCoupon(*fullCompoundedHelper);
    auto telescopicCompoundedCoupon = overnightCoupon(*telescopicCompoundedHelper);
    BOOST_REQUIRE(fullSimpleCoupon);
    BOOST_REQUIRE(telescopicSimpleCoupon);
    BOOST_REQUIRE(fullCompoundedCoupon);
    BOOST_REQUIRE(telescopicCompoundedCoupon);

    BOOST_CHECK_EQUAL(fullSimpleCoupon->averagingMethod(), RateAveraging::Simple);
    BOOST_CHECK_EQUAL(fullCompoundedCoupon->averagingMethod(), RateAveraging::Compound);
    BOOST_CHECK_EQUAL(telescopicSimpleCoupon->valueDates().size(),
                      fullSimpleCoupon->valueDates().size());
    BOOST_CHECK_LT(telescopicCompoundedCoupon->valueDates().size(),
                   fullCompoundedCoupon->valueDates().size());

    auto lastBaseCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
        telescopicCompoundedHelper->swap()->leg(0).back());
    auto lastOtherCoupon = ext::dynamic_pointer_cast<IborCoupon>(
        telescopicCompoundedHelper->swap()->leg(1).back());
    BOOST_REQUIRE(lastBaseCoupon);
    BOOST_REQUIRE(lastOtherCoupon);
    Date lastBaseFixingEndDate = overnightIndex->maturityDate(
        overnightIndex->valueDate(lastBaseCoupon->fixingDate()));
    Date lastPaymentDate = std::max(
        telescopicCompoundedHelper->swap()->leg(0).back()->date(),
        telescopicCompoundedHelper->swap()->leg(1).back()->date());
    Date expectedLatestRelevantDate =
        std::max({telescopicCompoundedHelper->maturityDate(), lastPaymentDate,
                  lastBaseFixingEndDate, lastOtherCoupon->fixingEndDate()});
    BOOST_CHECK_EQUAL(telescopicCompoundedHelper->latestRelevantDate(),
                      expectedLatestRelevantDate);

    testOvernightIborBootstrap(true, false, 0, false, RateAveraging::Simple, true);
}

BOOST_AUTO_TEST_CASE(testOvernightOvernightOtherCurveBootstrap) {
    BOOST_TEST_MESSAGE("Testing overnight-overnight basis-swap rate helpers "
                       "(other curve bootstrap)...");

    testOvernightOvernightBootstrap(false, false);
    testOvernightOvernightBootstrap(true, false);
}

BOOST_AUTO_TEST_CASE(testOvernightOvernightBaseCurveBootstrap) {
    BOOST_TEST_MESSAGE("Testing overnight-overnight basis-swap rate helpers "
                       "(base curve bootstrap)...");

    testOvernightOvernightBootstrap(false, true);
    testOvernightOvernightBootstrap(true, true);
}

BOOST_AUTO_TEST_CASE(testOvernightOvernightBootstrapWithLateLinkedDiscountCurve) {
    BOOST_TEST_MESSAGE("Testing overnight-overnight basis-swap rate helpers with a "
                       "late-linked discount curve...");

    testOvernightOvernightBootstrap(true, false, true);
}

BOOST_AUTO_TEST_CASE(testOvernightOvernightDateGenerationRule) {
    BOOST_TEST_MESSAGE(
        "Testing the date-generation rule of overnight-overnight basis-swap helpers...");

    Settings::instance().evaluationDate() = Date(27, January, 2010);
    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    Handle<YieldTermStructure> curve(flatRate(0.01, Actual365Fixed()));
    auto baseIndex = ext::make_shared<FedFunds>(curve);
    auto otherIndex = ext::make_shared<Sofr>(curve);
    Handle<Quote> basis(ext::make_shared<SimpleQuote>(0.0010));

    OvernightOvernightBasisSwapRateHelper backwardHelper(
        basis, 18 * Months, 2, calendar, Following, false, baseIndex, otherIndex,
        curve, false, 0, Annual, RateAveraging::Compound, RateAveraging::Compound,
        false, DateGeneration::Backward);
    OvernightOvernightBasisSwapRateHelper defaultHelper(
        basis, 18 * Months, 2, calendar, Following, false, baseIndex, otherIndex,
        curve);
    OvernightOvernightBasisSwapRateHelper forwardHelper(
        basis, 18 * Months, 2, calendar, Following, false, baseIndex, otherIndex,
        curve, false, 0, Annual, RateAveraging::Compound, RateAveraging::Compound,
        false, DateGeneration::Forward);

    auto firstAccrualEnd = [](const OvernightOvernightBasisSwapRateHelper& helper,
                              Size leg) {
        return ext::dynamic_pointer_cast<Coupon>(helper.swap()->leg(leg).front())
            ->accrualEndDate();
    };
    Date spot = calendar.advance(Settings::instance().evaluationDate(), 2, Days);
    for (Size i = 0; i < 2; ++i) {
        BOOST_CHECK_EQUAL(backwardHelper.swap()->leg(i).size(), 2);
        BOOST_CHECK_EQUAL(defaultHelper.swap()->leg(i).size(), 2);
        BOOST_CHECK_EQUAL(forwardHelper.swap()->leg(i).size(), 2);
        BOOST_CHECK_EQUAL(firstAccrualEnd(backwardHelper, i),
                          calendar.adjust(spot + 6 * Months, Following));
        BOOST_CHECK_EQUAL(firstAccrualEnd(defaultHelper, i),
                          firstAccrualEnd(backwardHelper, i));
        BOOST_CHECK_EQUAL(firstAccrualEnd(forwardHelper, i),
                          calendar.adjust(spot + 1 * Years, Following));
    }

    Date expectedMaturity = calendar.adjust(spot + 18 * Months, Following);
    BOOST_CHECK_EQUAL(backwardHelper.maturityDate(), expectedMaturity);
    BOOST_CHECK_EQUAL(defaultHelper.maturityDate(), expectedMaturity);
    BOOST_CHECK_EQUAL(forwardHelper.maturityDate(), expectedMaturity);
}

BOOST_AUTO_TEST_CASE(testOvernightOvernightNoFrequency) {
    BOOST_TEST_MESSAGE(
        "Testing zero-coupon overnight-overnight basis-swap helpers...");

    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    Handle<YieldTermStructure> curve(flatRate(0.01, Actual365Fixed()));
    auto baseIndex = ext::make_shared<FedFunds>(curve);
    auto otherIndex = ext::make_shared<Sofr>(curve);
    Handle<Quote> basis(ext::make_shared<SimpleQuote>(0.0010));

    OvernightOvernightBasisSwapRateHelper helper(
        basis, 5 * Years, 2, calendar, Following, false, baseIndex, otherIndex,
        curve, false, 0, NoFrequency);

    BOOST_CHECK_EQUAL(helper.swap()->leg(0).size(), 1);
    BOOST_CHECK_EQUAL(helper.swap()->leg(1).size(), 1);
    auto baseCoupon = ext::dynamic_pointer_cast<Coupon>(helper.swap()->leg(0).front());
    auto otherCoupon = ext::dynamic_pointer_cast<Coupon>(helper.swap()->leg(1).front());
    BOOST_REQUIRE(baseCoupon);
    BOOST_REQUIRE(otherCoupon);
    BOOST_CHECK_EQUAL(baseCoupon->accrualStartDate(), helper.earliestDate());
    BOOST_CHECK_EQUAL(otherCoupon->accrualStartDate(), helper.earliestDate());
    BOOST_CHECK_EQUAL(baseCoupon->accrualEndDate(), helper.maturityDate());
    BOOST_CHECK_EQUAL(otherCoupon->accrualEndDate(), helper.maturityDate());
}

BOOST_AUTO_TEST_CASE(testOvernightOvernightTelescopicValueDatesWithSimpleAveraging) {
    BOOST_TEST_MESSAGE("Testing telescopic value dates with simple overnight averaging...");

    auto calendar = UnitedStates(UnitedStates::GovernmentBond);
    Handle<YieldTermStructure> forecastCurve(flatRate(0.01, Actual365Fixed()));
    auto baseIndex = ext::make_shared<FedFunds>(forecastCurve);
    auto otherIndex = ext::make_shared<Sofr>();

    auto makeHelper = [&](bool telescopicValueDates) {
        return ext::make_shared<OvernightOvernightBasisSwapRateHelper>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(0.0010)), 1 * Years, 2,
            calendar, Following, false, baseIndex, otherIndex,
            Handle<YieldTermStructure>(), false, 0, Annual, RateAveraging::Simple,
            RateAveraging::Compound, telescopicValueDates);
    };

    auto fullScheduleHelper = makeHelper(false);
    auto telescopicHelper = makeHelper(true);

    auto fullSimpleCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
        fullScheduleHelper->swap()->leg(0).front());
    auto telescopicSimpleCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
        telescopicHelper->swap()->leg(0).front());
    auto fullCompoundedCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
        fullScheduleHelper->swap()->leg(1).front());
    auto telescopicCompoundedCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
        telescopicHelper->swap()->leg(1).front());

    BOOST_REQUIRE(fullSimpleCoupon);
    BOOST_REQUIRE(telescopicSimpleCoupon);
    BOOST_REQUIRE(fullCompoundedCoupon);
    BOOST_REQUIRE(telescopicCompoundedCoupon);
    BOOST_CHECK_EQUAL(telescopicSimpleCoupon->valueDates().size(),
                      fullSimpleCoupon->valueDates().size());
    BOOST_CHECK_LT(telescopicCompoundedCoupon->valueDates().size(),
                   fullCompoundedCoupon->valueDates().size());
    BOOST_CHECK_SMALL(telescopicSimpleCoupon->rate() - fullSimpleCoupon->rate(), 1.0e-14);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
