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
#include "utilities.hpp"
#include <ql/cashflows/coupon.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/currencies/all.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/experimental/fx/discountingmtmcrosscurrencybasisswapengine.hpp>
#include <ql/experimental/fx/fxresetcashflows.hpp>
#include <ql/experimental/fx/mtmcrosscurrencybasisswap.hpp>
#include <ql/experimental/termstructures/crosscurrencyratehelpers.hpp>
#include <ql/indexes/ibor/estr.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/jpylibor.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/instruments/constnotionalcrosscurrencybasisswap.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/pricingengines/swap/discountingconstnotionalcrosscurrencyswapengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/schedule.hpp>
#include <cmath>
#include <iomanip>
#include <vector>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(MtMCrossCurrencyBasisSwapTest)

namespace {

    Handle<YieldTermStructure> flatCurve(const Date& ref, Rate r) {
        return Handle<YieldTermStructure>(
            ext::make_shared<FlatForward>(ref, r, Actual360()));
    }

}

BOOST_AUTO_TEST_CASE(testFairFxQuoteSpreadRepricesToZero) {
    BOOST_TEST_MESSAGE(
        "Testing that a MtM cross-currency basis swap reprices to zero at its fair spread...");

    SavedSettings backup;
    Date today(11, Sep, 2018);
    Settings::instance().evaluationDate() = today;

    // USD is the domestic (NPV) currency, EUR the foreign one.
    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.02);
    Handle<YieldTermStructure> eurCurve = flatCurve(today, 0.01);

    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);
    auto eurIndex = ext::make_shared<Euribor>(3 * Months, eurCurve);

    TARGET cal;
    Date start = cal.advance(today, 2 * Days);
    Date end = start + 5 * Years;
    Schedule sch(start, end, 3 * Months, cal, ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Backward, false);

    Real usdNominal = 10000000.0;
    Rate spotFx = 1.10; // USD per EUR
    Real eurNominal = usdNominal / spotFx;

    auto fxSpot = makeQuoteHandle(spotFx);
    auto engine = ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, fxSpot);

    // Base USD (constant notional), quote EUR (resettable leg).
    auto makeSwap = [&](Spread eurSpread) {
        auto swap = ext::make_shared<MtMCrossCurrencyBasisSwap>(
            MtMCrossCurrencyBasisSwap::PayFxBaseCurrency,
            usdNominal, USDCurrency(), sch, usdIndex, 0.0, 1.0, eurNominal, EURCurrency(), sch,
            eurIndex, eurSpread, 1.0, /*isFxBaseCurrencyLegResettable=*/false);
        swap->setPricingEngine(engine);
        return swap;
    };

    auto swap = makeSwap(0.0);
    BOOST_CHECK_MESSAGE(swap->resettingLegIndex() == 1, "quote (EUR) leg should be resettable");

    Spread fair = swap->fairFxQuoteSpread();
    BOOST_REQUIRE_MESSAGE(fair != Null<Spread>(), "fair FX-quote spread not available");

    // Re-pricing at the fair spread must give a (numerically) zero NPV.
    Real npvAtFair = makeSwap(fair)->NPV();
    Real tol = 1.0e-2;
    if (std::fabs(npvAtFair) > tol)
        BOOST_ERROR("MtM swap NPV at fair FX-quote spread is not zero:"
                    << "\n    fair spread: " << std::setprecision(12) << fair
                    << "\n    NPV:         " << std::setprecision(12) << npvAtFair);
}

BOOST_AUTO_TEST_CASE(testRepricesToParOffHelperBootstrappedCurve) {
    BOOST_TEST_MESSAGE("Testing that the MtM xccy basis swap reprices to par off a curve "
                       "bootstrapped from MtM xccy rate helpers, for all flag combinations...");

    SavedSettings backup;

    Natural fixingDays = 2;
    Calendar cal = TARGET();
    BusinessDayConvention conv = Following;
    DayCounter dc = Actual365Fixed();
    bool endOfMonth = false;
    Natural fxResetFixingDays = 2;

    Date today = cal.adjust(Date(6, September, 2013));
    Settings::instance().evaluationDate() = today;
    Date settlementDt = cal.advance(today, fixingDays, Days);

    // EUR is the FX base currency, USD the quote currency; the index forecast
    // curves double as the collateral (discount) curve of their currency.
    Handle<YieldTermStructure> eurForecast(ext::make_shared<FlatForward>(today, 0.007, dc));
    Handle<YieldTermStructure> usdForecast(ext::make_shared<FlatForward>(today, 0.015, dc));
    auto eurIndex = ext::make_shared<Euribor3M>(eurForecast);
    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdForecast);

    // (tenor, basis in bp): a representative subset of the Moreni-Pallavicini quotes.
    std::vector<std::pair<Period, Spread> > basisData = {{1 * Years, -14.5},
                                                         {2 * Years, -20.5},
                                                         {5 * Years, -26.5},
                                                         {10 * Years, -26.25},
                                                         {30 * Years, -20.50}};

    Real bp = 1.0e-4;
    Real tolerance = 1.0e-8;

    for (bool isFxBaseCurrencyCollateralCurrency : {false, true})
        for (bool isBasisOnFxBaseCurrencyLeg : {false, true})
            for (bool isFxBaseCurrencyLegResettable : {false, true}) {

                Handle<YieldTermStructure> collateral =
                    isFxBaseCurrencyCollateralCurrency ? eurForecast : usdForecast;

                std::vector<ext::shared_ptr<RateHelper> > helpers;
                helpers.reserve(basisData.size());
                for (const auto& q : basisData)
                    helpers.push_back(ext::make_shared<MtMCrossCurrencyBasisSwapRateHelper>(
                        makeQuoteHandle(q.second * bp), q.first, fixingDays, cal, conv, endOfMonth,
                        eurIndex, usdIndex, collateral, isFxBaseCurrencyCollateralCurrency,
                        isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable, std::nullopt, 0,
                        std::nullopt, fxResetFixingDays, cal));

                ext::shared_ptr<YieldTermStructure> bootstrapped(
                    new PiecewiseYieldCurve<Discount, LogLinear>(today, helpers, dc));
                bootstrapped->enableExtrapolation();
                Handle<YieldTermStructure> resetting(bootstrapped);

                // The collateral currency keeps its own curve; the other currency is
                // discounted on the bootstrapped curve.
                Handle<YieldTermStructure> eurDiscount =
                    isFxBaseCurrencyCollateralCurrency ? eurForecast : resetting;
                Handle<YieldTermStructure> usdDiscount =
                    isFxBaseCurrencyCollateralCurrency ? resetting : usdForecast;

                // Unit notionals and spot = 1, matching the helper's par convention.
                auto engine = ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
                    USDCurrency(), usdDiscount, EURCurrency(), eurDiscount, makeQuoteHandle(1.0));

                for (const auto& q : basisData) {
                    Spread basis = q.second * bp;
                    Spread baseLegBasis = isBasisOnFxBaseCurrencyLeg ? basis : Spread(0.0);
                    Spread quoteLegBasis = isBasisOnFxBaseCurrencyLeg ? Spread(0.0) : basis;

                    Schedule eurSch = MakeSchedule()
                                          .from(settlementDt)
                                          .to(settlementDt + q.first)
                                          .withTenor(eurIndex->tenor())
                                          .withCalendar(cal)
                                          .withConvention(conv)
                                          .endOfMonth(endOfMonth)
                                          .backwards();
                    Schedule usdSch = MakeSchedule()
                                          .from(settlementDt)
                                          .to(settlementDt + q.first)
                                          .withTenor(usdIndex->tenor())
                                          .withCalendar(cal)
                                          .withConvention(conv)
                                          .endOfMonth(endOfMonth)
                                          .backwards();

                    auto swap = ext::make_shared<MtMCrossCurrencyBasisSwap>(
                        MtMCrossCurrencyBasisSwap::PayFxBaseCurrency, 1.0, EURCurrency(),
                        eurSch, eurIndex, baseLegBasis, 1.0, 1.0, USDCurrency(), usdSch, usdIndex,
                        quoteLegBasis, 1.0, isFxBaseCurrencyLegResettable, fxResetFixingDays, cal);
                    swap->setPricingEngine(engine);

                    Real npv = swap->NPV();
                    if (std::fabs(npv) > tolerance)
                        BOOST_ERROR(
                            "MtM xccy basis swap does not reprice to par off the bootstrapped curve\n"
                            << std::setprecision(5) << "    NPV:     " << npv << "\n"
                            << "    tenor:   " << q.first << "\n"
                            << "    basis:   " << q.second << "\n"
                            << "    collateral in base ccy:  " << isFxBaseCurrencyCollateralCurrency
                            << "\n"
                            << "    basis on base ccy leg:   " << isBasisOnFxBaseCurrencyLeg << "\n"
                            << "    base ccy leg resettable: " << isFxBaseCurrencyLegResettable
                            << "\n");
                }

                // The helpers expose their underlying par swaps, priced on the
                // helpers' internal handles: after bootstrapping, the fair
                // spread of the basis leg must reproduce the helper quote.
                for (Size i = 0; i < helpers.size(); ++i) {
                    auto helper =
                        ext::dynamic_pointer_cast<MtMCrossCurrencyBasisSwapRateHelper>(helpers[i]);
                    BOOST_REQUIRE(helper != nullptr && helper->swap() != nullptr);
                    BOOST_CHECK_EQUAL(helper->fxResetFixingDays(), fxResetFixingDays);
                    BOOST_CHECK_EQUAL(helper->fxResetFixingCalendar(), cal);
                    auto underlying = helper->swap();
                    underlying->deepUpdate();
                    bool foundFxResetCoupon = false;
                    for (const auto& cf : underlying->resettingLeg()) {
                        if (auto coupon = ext::dynamic_pointer_cast<FxResetCoupon>(cf)) {
                            foundFxResetCoupon = true;
                            Date expectedFixing = cal.advance(
                                coupon->fxResetValueDate(),
                                -static_cast<Integer>(fxResetFixingDays), Days);
                            BOOST_CHECK_EQUAL(coupon->fxResetDate(), expectedFixing);
                            break;
                        }
                    }
                    BOOST_CHECK(foundFxResetCoupon);
                    Spread fair = isBasisOnFxBaseCurrencyLeg ? underlying->fairFxBaseSpread() :
                                                               underlying->fairFxQuoteSpread();
                    Spread expected = basisData[i].second * bp;
                    if (std::fabs(fair - expected) > tolerance)
                        BOOST_ERROR("the exposed helper swap does not reproduce the helper quote\n"
                                    << std::setprecision(5) << "    fair basis: " << fair / bp
                                    << " bp\n"
                                    << "    quote:      " << basisData[i].second << " bp\n"
                                    << "    collateral in base ccy:  "
                                    << isFxBaseCurrencyCollateralCurrency << "\n"
                                    << "    basis on base ccy leg:   " << isBasisOnFxBaseCurrencyLeg
                                    << "\n"
                                    << "    base ccy leg resettable: "
                                    << isFxBaseCurrencyLegResettable << "\n");
                }
            }
}

BOOST_AUTO_TEST_CASE(testFxResetObservationDatesAndProjection) {
    BOOST_TEST_MESSAGE("Testing FX-reset fixing dates and value-date projection...");

    SavedSettings backup;
    Date today(1, July, 2024);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> eurCurve = flatCurve(today, 0.01);
    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.05);
    auto eurIndex = ext::make_shared<Euribor>(3 * Months, eurCurve);
    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);

    Date start(8, July, 2024);
    Schedule sch(start, start + 6 * Months, 3 * Months, TARGET(), Following, Following,
                 DateGeneration::Forward, false);
    Calendar fxCalendar = JointCalendar(TARGET(), UnitedStates(UnitedStates::Settlement),
                                        JoinHolidays);
    FxResetConvention fxResetConvention(2, fxCalendar);

    Real eurNominal = 10000000.0;
    Real spotFx = 1.10;
    auto spotQuote = ext::make_shared<SimpleQuote>(spotFx);
    Handle<Quote> spotFxHandle(spotQuote);
    auto swap = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::PayFxBaseCurrency, eurNominal, EURCurrency(), sch,
        eurIndex, 0.0, 1.0, eurNominal * spotFx, USDCurrency(), sch, usdIndex, 0.0, 1.0,
        /*isFxBaseCurrencyLegResettable=*/false, fxResetConvention.fixingDays(), fxResetConvention.fixingCalendar());
    BOOST_CHECK(!swap->fxResetFixingCalendar().empty());
    swap->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, spotFxHandle));
    Real automaticallyDatedNpv = swap->NPV();

    ext::shared_ptr<FxResetCoupon> firstCoupon;
    ext::shared_ptr<FxResetNotionalExchange> firstExchange;
    for (const auto& cf : swap->resettingLeg()) {
        if (!firstExchange)
            firstExchange = ext::dynamic_pointer_cast<FxResetNotionalExchange>(cf);
        if (auto coupon = ext::dynamic_pointer_cast<FxResetCoupon>(cf)) {
            firstCoupon = coupon;
            break;
        }
    }
    BOOST_REQUIRE(firstCoupon != nullptr);
    BOOST_REQUIRE(firstExchange != nullptr);
    BOOST_CHECK_EQUAL(firstCoupon->fxResetDate(), Date(3, July, 2024));
    BOOST_CHECK_EQUAL(firstCoupon->fxResetValueDate(), start);
    Date spotFxSettleDate = fxResetConvention.valueDate(today);
    BOOST_CHECK_EQUAL(spotFxSettleDate, Date(3, July, 2024));
    BOOST_CHECK_EQUAL(fxResetConvention.valueDate(firstCoupon->fxResetDate()), start);
    BOOST_CHECK(!firstExchange->previousReset());
    BOOST_REQUIRE(firstExchange->currentReset());
    BOOST_CHECK_EQUAL(firstExchange->currentReset()->fixingDate(), firstCoupon->fxResetDate());     // NOLINT(bugprone-unchecked-optional-access)
    BOOST_CHECK_EQUAL(firstExchange->currentReset()->valueDate(), firstCoupon->fxResetValueDate()); // NOLINT(bugprone-unchecked-optional-access)
    BOOST_CHECK(!firstCoupon->fxResetPricer());
    BOOST_CHECK(!firstExchange->fxResetPricer());

    auto fxResetPricer = ext::make_shared<DiscountingFxResetPricer>(
        EURCurrency(), USDCurrency(), eurCurve, usdCurve, spotFxHandle, true,
        spotFxSettleDate);

    Real expectedForward = spotFx * usdCurve->discount(spotFxSettleDate) /
                           eurCurve->discount(spotFxSettleDate) *
                           eurCurve->discount(start) / usdCurve->discount(start);
    std::vector<Real> fxResetRates = swap->fxResetRates();
    std::vector<Real> fxResetNotionals = swap->fxResetNotionals();
    BOOST_REQUIRE_EQUAL(fxResetRates.size(), fxResetNotionals.size());

    Size resetNo = 0;
    for (const auto& cf : swap->resettingLeg()) {
        if (auto coupon = ext::dynamic_pointer_cast<FxResetCoupon>(cf)) {
            BOOST_REQUIRE(resetNo < fxResetRates.size());
            Real expectedRate = fxResetPricer->fxRate(coupon->fxReset());
            QL_CHECK_CLOSE(fxResetRates[resetNo], expectedRate, 1.0e-10);
            QL_CHECK_CLOSE(fxResetNotionals[resetNo], eurNominal * expectedRate, 1.0e-10);
            ++resetNo;
        }
    }
    BOOST_CHECK_EQUAL(resetNo, fxResetRates.size());
    QL_CHECK_CLOSE(fxResetRates.front(), expectedForward, 1.0e-10);

    spotQuote->setValue(1.20);
    std::vector<Real> bumpedFxResetRates = swap->fxResetRates();
    std::vector<Real> bumpedFxResetNotionals = swap->fxResetNotionals();
    QL_CHECK_CLOSE(bumpedFxResetRates.front(), fxResetRates.front() * 1.20 / spotFx,
                      1.0e-10);
    QL_CHECK_CLOSE(bumpedFxResetNotionals.front(),
                      fxResetNotionals.front() * 1.20 / spotFx, 1.0e-10);
    spotQuote->setValue(spotFx);

    setFxResetPricer(swap->resettingLeg(), fxResetPricer);
    QL_CHECK_CLOSE(firstCoupon->nominal(), eurNominal * expectedForward, 1.0e-10);

    // Omitting the engine's explicit spot settlement date must derive the same
    // date from the swap's reset convention.
    swap->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, makeQuoteHandle(spotFx),
        std::nullopt, Date(), Date(), spotFxSettleDate));
    QL_CHECK_SMALL(swap->NPV() - automaticallyDatedNpv, 1.0e-10 * eurNominal);

    // The equivalent reference-date FX quote must give the same result when an
    // explicit reference-date settlement overrides the convention.
    Real referenceDateFx = spotFx * usdCurve->discount(spotFxSettleDate) /
                           eurCurve->discount(spotFxSettleDate);
    swap->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, makeQuoteHandle(referenceDateFx),
        std::nullopt, Date(), Date(), today));
    QL_CHECK_SMALL(swap->NPV() - automaticallyDatedNpv, 1.0e-10 * eurNominal);
}

BOOST_AUTO_TEST_CASE(testResetFixingStateUsesEvaluationDate) {
    BOOST_TEST_MESSAGE(
        "Testing that FX-reset fixing state is determined from the evaluation date...");

    SavedSettings backup;
    Date today(1, July, 2024);
    Settings::instance().evaluationDate() = today;

    TARGET calendar;
    Date spotDate = calendar.advance(today, 2 * Days);
    Handle<YieldTermStructure> eurCurve = flatCurve(spotDate, 0.01);
    Handle<YieldTermStructure> usdCurve = flatCurve(spotDate, 0.05);
    Real spotFx = 1.10;
    DiscountingFxResetPricer pricer(EURCurrency(), USDCurrency(), eurCurve, usdCurve,
                                    makeQuoteHandle(spotFx), true, spotDate);
    FxReset reset(today, spotDate);

    // Although today's fixing precedes the curves' spot-date reference, it is
    // forecast when no fixing has been published yet.
    QL_CHECK_CLOSE(pricer.fxRate(reset), spotFx, 1.0e-12);

    Real publishedFixing = 1.25;
    ExchangeRateManager::instance().add(
        ExchangeRate(EURCurrency(), USDCurrency(), publishedFixing), today, today);
    QL_CHECK_CLOSE(pricer.fxRate(reset), publishedFixing, 1.0e-12);

    ExchangeRateManager::instance().clear();
    Settings::instance().enforcesTodaysHistoricFixings() = true;
    BOOST_CHECK_THROW(pricer.fxRate(reset), Error);
}

BOOST_AUTO_TEST_CASE(testFxResetCouponUpdates) {
    BOOST_TEST_MESSAGE(
        "Testing FX-reset coupon notifications and deep updates...");

    SavedSettings backup;
    Date today(1, July, 2024);
    Settings::instance().evaluationDate() = today;

    RelinkableHandle<YieldTermStructure> forecastCurve;
    forecastCurve.linkTo(*flatCurve(today, 0.01));
    auto index = ext::make_shared<Euribor>(3 * Months, forecastCurve);
    TARGET calendar;
    Date start = calendar.advance(today, 2 * Days);
    Schedule schedule(start, start + 3 * Months, 3 * Months, calendar, Following,
                      Following, DateGeneration::Forward, false);
    Leg leg = IborLeg(schedule, index).withNotionals(1.0);
    auto underlying = ext::dynamic_pointer_cast<FloatingRateCoupon>(leg.front());
    BOOST_REQUIRE(underlying != nullptr);

    auto coupon = ext::make_shared<FxResetCoupon>(underlying, 1000000.0,
                                                   FxReset(start, start));
    auto spotQuote = ext::make_shared<SimpleQuote>(1.10);
    auto fxResetPricer = ext::make_shared<DiscountingFxResetPricer>(
        USDCurrency(), EURCurrency(), forecastCurve, forecastCurve,
        Handle<Quote>(spotQuote), false);
    coupon->setFxResetPricer(fxResetPricer);
    coupon->forwardFirstNotificationOnly();

    // Make the wrapper calculated, then verify that both pricer and underlying
    // coupon changes are forwarded even in first-notification-only mode.
    coupon->amount();
    Flag flag;
    flag.registerWith(coupon);
    spotQuote->setValue(1.11);
    BOOST_CHECK(flag.isUp());

    flag.lower();
    coupon->amount();
    forecastCurve.linkTo(*flatCurve(today, 0.02));
    BOOST_CHECK(flag.isUp());

    coupon->rate();
    underlying->rate();
    BOOST_CHECK(coupon->isCalculated());
    BOOST_CHECK(underlying->isCalculated());
    coupon->deepUpdate();
    BOOST_CHECK(!coupon->isCalculated());
    BOOST_CHECK(!underlying->isCalculated());
}

BOOST_AUTO_TEST_CASE(testKnownFxResetBeforeAccrualStart) {
    BOOST_TEST_MESSAGE("Testing that an FX reset fixed before accrual start uses its fixing...");

    SavedSettings backup;
    Date today(5, July, 2024);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> eurCurve = flatCurve(today, 0.01);
    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.05);
    auto eurIndex = ext::make_shared<Euribor>(3 * Months, eurCurve);
    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);

    Date start(8, July, 2024);
    Schedule sch(start, start + 6 * Months, 3 * Months, TARGET(), Following, Following,
                 DateGeneration::Forward, false);
    Calendar fxCalendar = JointCalendar(TARGET(), UnitedStates(UnitedStates::Settlement),
                                        JoinHolidays);
    FxResetConvention fxResetConvention(2, fxCalendar);
    Date fxFixingDate(3, July, 2024);
    Real fixedFx = 1.25;
    ExchangeRateManager::instance().add(ExchangeRate(EURCurrency(), USDCurrency(), fixedFx),
                                        fxFixingDate, fxFixingDate);

    Date eurFixingDate = eurIndex->fixingDate(start);
    if (eurFixingDate < today)
        eurIndex->addFixing(eurFixingDate, 0.01);
    Date usdFixingDate = usdIndex->fixingDate(start);
    if (usdFixingDate < today)
        usdIndex->addFixing(usdFixingDate, 0.05);

    Real eurNominal = 10000000.0;
    auto swap = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::PayFxBaseCurrency, eurNominal, EURCurrency(), sch,
        eurIndex, 0.0, 1.0, eurNominal * 1.10, USDCurrency(), sch, usdIndex, 0.0, 1.0,
        /*isFxBaseCurrencyLegResettable=*/false, fxResetConvention.fixingDays(), fxResetConvention.fixingCalendar());
    swap->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, makeQuoteHandle(1.10)));
    BOOST_CHECK_NO_THROW(swap->NPV());

    ext::shared_ptr<FxResetCoupon> firstCoupon;
    for (const auto& cf : swap->resettingLeg()) {
        if (auto coupon = ext::dynamic_pointer_cast<FxResetCoupon>(cf)) {
            firstCoupon = coupon;
            break;
        }
    }
    BOOST_REQUIRE(firstCoupon != nullptr);
    BOOST_CHECK_EQUAL(firstCoupon->fxResetDate(), fxFixingDate);
    BOOST_CHECK_EQUAL(firstCoupon->fxResetValueDate(), start);
    setFxResetPricer(
        swap->resettingLeg(),
        ext::make_shared<DiscountingFxResetPricer>(
            EURCurrency(), USDCurrency(), eurCurve, usdCurve, makeQuoteHandle(1.10), true));
    QL_CHECK_CLOSE(firstCoupon->nominal(), eurNominal * fixedFx, 1.0e-10);
}

BOOST_AUTO_TEST_CASE(testResettableLegCashFlowsMatchLegResults) {
    BOOST_TEST_MESSAGE("Testing that enumerating the resettable leg's cash flows reproduces "
                       "the leg NPV and the FX-reset notionals...");

    SavedSettings backup;
    Date today(11, Sep, 2018);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.02);
    Handle<YieldTermStructure> eurCurve = flatCurve(today, 0.01);
    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);
    auto eurIndex = ext::make_shared<Euribor>(3 * Months, eurCurve);

    TARGET cal;
    Date start = cal.advance(today, 2 * Days);
    Date end = start + 5 * Years;
    Schedule sch(start, end, 3 * Months, cal, ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Backward, false);

    Real usdNominal = 10000000.0;
    Rate spotFx = 1.10; // USD per EUR

    // Base USD (constant notional), quote EUR (resettable leg).
    auto swap = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::PayFxBaseCurrency, usdNominal, USDCurrency(), sch,
        usdIndex, 0.0, 1.0, usdNominal / spotFx, EURCurrency(), sch, eurIndex, 10.0e-4, 1.0,
        /*isFxBaseCurrencyLegResettable=*/false);
    BOOST_CHECK(swap->fxResetFixingCalendar().empty());
    swap->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, makeQuoteHandle(spotFx)));

    Size resettingLegNo = swap->resettingLegIndex();
    Real legNpv = swap->inCcyLegNPV(resettingLegNo); // triggers the calculation
    setFxResetPricer(
        swap->leg(resettingLegNo),
        ext::make_shared<DiscountingFxResetPricer>(
            USDCurrency(), EURCurrency(), usdCurve, eurCurve, makeQuoteHandle(spotFx), false));

    // With the same pricer assigned explicitly, enumerating and discounting the
    // live leg's cash flows must reproduce the engine's in-currency leg NPV.
    Real npv = 0.0;
    Size couponCount = 0, exchangeCount = 0;
    for (const auto& cf : swap->leg(resettingLegNo)) {
        npv += cf->amount() * eurCurve->discount(cf->date());
        if (auto coupon = ext::dynamic_pointer_cast<FxResetCoupon>(cf)) {
            ++couponCount;
            // The nominal is the constant-leg notional converted at the
            // forward FX rate (EUR per USD) of the period's reset date.
            Date reset = coupon->fxResetDate();
            Real expected = usdNominal * (1.0 / spotFx) * usdCurve->discount(reset) /
                            eurCurve->discount(reset);
            QL_CHECK_CLOSE(coupon->nominal(), expected, 1.0e-8);
        } else if (ext::dynamic_pointer_cast<FxResetNotionalExchange>(cf)) {
            ++exchangeCount;
        } else {
            BOOST_ERROR("unexpected cash-flow type on the resettable leg");
        }
    }

    BOOST_CHECK_EQUAL(exchangeCount, couponCount + 1);
    QL_CHECK_SMALL(npv - legNpv, 1.0e-8 * usdNominal);
}

BOOST_AUTO_TEST_CASE(testSameDayResetUsesSpot) {
    BOOST_TEST_MESSAGE("Testing that a reset on the curve reference date uses spot FX...");

    SavedSettings backup;
    Date today(11, Sep, 2018);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.02);
    Handle<YieldTermStructure> eurCurve = flatCurve(today, 0.01);
    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);
    auto eurIndex = ext::make_shared<Euribor>(3 * Months, eurCurve);

    TARGET cal;
    Schedule sch(today, today + 3 * Months, 3 * Months, cal, ModifiedFollowing,
                 ModifiedFollowing, DateGeneration::Forward, false);
    Real usdNominal = 10000000.0;
    Rate spotFx = 1.10;
    auto swap = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::PayFxBaseCurrency, usdNominal, USDCurrency(), sch,
        usdIndex, 0.0, 1.0, usdNominal / spotFx, EURCurrency(), sch, eurIndex, 0.0, 1.0,
        /*isFxBaseCurrencyLegResettable=*/false);

    for (Size legNo = 0; legNo < 2; ++legNo)
        for (const auto& cf : swap->leg(legNo))
            if (auto frc = ext::dynamic_pointer_cast<FloatingRateCoupon>(cf)) {
                Date fixingDate = frc->fixingDate();
                if (fixingDate < today)
                    frc->index()->addFixing(fixingDate, legNo == 0 ? 0.02 : 0.01, true);
            }

    swap->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, makeQuoteHandle(spotFx)));
    BOOST_CHECK_NO_THROW(swap->NPV());
}

BOOST_AUTO_TEST_CASE(testFxSettlementAndNpvDateConsistency) {
    BOOST_TEST_MESSAGE("Testing FX-settlement carry, NPV-date conversion, and scalar discount...");

    SavedSettings backup;
    Date today(11, Sep, 2018);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.03);
    Handle<YieldTermStructure> eurCurve = flatCurve(today, 0.01);
    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);
    auto eurIndex = ext::make_shared<Euribor>(3 * Months, eurCurve);

    TARGET cal;
    Date start = cal.advance(today, 2 * Days);
    Date end = start + 2 * Years;
    Schedule sch(start, end, 3 * Months, cal, ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Forward, false);
    Real usdNominal = 10000000.0;
    Rate referenceSpot = 1.10;

    auto makeSwap = [&]() {
        return ext::make_shared<MtMCrossCurrencyBasisSwap>(
            MtMCrossCurrencyBasisSwap::PayFxBaseCurrency, usdNominal, USDCurrency(), sch,
            usdIndex, 0.0, 1.0, usdNominal / referenceSpot, EURCurrency(), sch, eurIndex,
            15.0e-4, 1.0, /*isFxBaseCurrencyLegResettable=*/false);
    };

    auto referenceSwap = makeSwap();
    referenceSwap->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, makeQuoteHandle(referenceSpot)));
    Real referenceNpv = referenceSwap->NPV();

    Date fxSettlementDate = cal.advance(today, 6 * Months);
    Rate settlementFx = referenceSpot * eurCurve->discount(fxSettlementDate) /
                        usdCurve->discount(fxSettlementDate);
    auto settlementSwap = makeSwap();
    settlementSwap->setPricingEngine(
        ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
            USDCurrency(), usdCurve, EURCurrency(), eurCurve, makeQuoteHandle(settlementFx),
            std::nullopt, Date(), Date(), fxSettlementDate));

    Real tolerance = 1.0e-10 * usdNominal;
    QL_CHECK_SMALL(settlementSwap->NPV() - referenceNpv, tolerance);

    Date npvDate = cal.advance(today, 9 * Months);
    auto forwardNpvSwap = makeSwap();
    forwardNpvSwap->setPricingEngine(
        ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
            USDCurrency(), usdCurve, EURCurrency(), eurCurve, makeQuoteHandle(referenceSpot),
            std::nullopt, Date(), npvDate));
    DiscountFactor domesticNpvDateDiscount = usdCurve->discount(npvDate);
    QL_CHECK_SMALL(forwardNpvSwap->NPV() * domesticNpvDateDiscount - referenceNpv,
                      tolerance);
    QL_CHECK_CLOSE(forwardNpvSwap->npvDateDiscount(), domesticNpvDateDiscount, 1.0e-10);
}

BOOST_AUTO_TEST_CASE(testSeasonedResetPeriodNeedsExchangeRate) {
    BOOST_TEST_MESSAGE(
        "Testing that an already-started MtM reset period requires a stored exchange rate...");

    SavedSettings backup;
    Date today(11, Sep, 2018);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.02);
    Handle<YieldTermStructure> eurCurve = flatCurve(today, 0.01);

    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);
    auto eurIndex = ext::make_shared<Euribor>(3 * Months, eurCurve);

    TARGET cal;
    Schedule sch(Date(11, Aug, 2018), Date(11, Feb, 2019), 3 * Months, cal,
                 ModifiedFollowing, ModifiedFollowing, DateGeneration::Forward, false);

    Real usdNominal = 10000000.0;
    Rate spotFx = 1.10;
    Real eurNominal = usdNominal / spotFx;

    auto engine = ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, makeQuoteHandle(spotFx));

    auto swap = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::PayFxBaseCurrency,
        usdNominal, USDCurrency(), sch, usdIndex, 0.0, 1.0, eurNominal, EURCurrency(), sch,
        eurIndex, 0.0, 1.0, /*isFxBaseCurrencyLegResettable=*/true);
    swap->setPricingEngine(engine);

    BOOST_CHECK_THROW(swap->NPV(), Error);
}

BOOST_AUTO_TEST_CASE(testSeasonedTriangulatedResetMatchesConstantNotional) {
    BOOST_TEST_MESSAGE("Testing that an in-progress MtM reset obtained by triangulation prices "
                       "identically to the equivalent constant-notional swap built from the "
                       "realised reset notional...");

    SavedSettings backup;
    Date today(11, Sep, 2018);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.02);
    Handle<YieldTermStructure> eurCurve = flatCurve(today, 0.01);

    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);
    auto eurIndex = ext::make_shared<Euribor>(3 * Months, eurCurve);

    TARGET cal;
    // A single 3M period that started a month ago: the only period is in progress,
    // so its reset notional is fully determined by the realised FX fixing and the
    // swap is economically a constant-notional swap with that EUR notional.
    Date start = cal.advance(today, -1 * Months);
    Date end = start + 3 * Months;
    Schedule sch(start, end, 3 * Months, cal, ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Forward, false);

    Real usdNominal = 10000000.0;
    Rate spotFx = 1.10;            // USD per EUR
    Spread eurBasis = 25 * 1.0e-4; // basis on the resettable EUR leg
    Rate realisedFx = 0.92;        // realised EUR per USD at the past reset (deliberately != 1/spotFx)

    auto mtm = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::PayFxBaseCurrency, usdNominal, USDCurrency(), sch, usdIndex,
        0.0, 1.0, usdNominal / spotFx, EURCurrency(), sch, eurIndex, eurBasis, 1.0,
        /*isFxBaseCurrencyLegResettable=*/false);

    // Seed the realised rate fixings (both legs) and the realised FX reset.
    for (Size legNo = 0; legNo < 2; ++legNo)
        for (const auto& cf : mtm->leg(legNo))
            if (auto frc = ext::dynamic_pointer_cast<FloatingRateCoupon>(cf)) {
                Date fd = frc->fixingDate();
                if (fd <= today)
                    frc->index()->addFixing(fd, legNo == 0 ? 0.021 : 0.012, true);
            }
    for (const auto& cf : mtm->resettingLeg())
        if (auto cpn = ext::dynamic_pointer_cast<Coupon>(cf)) {
            Date reset = cpn->accrualStartDate();
            if (reset <= today) {
                Rate usdGbp = 0.80;
                ExchangeRateManager::instance().add(
                    ExchangeRate(USDCurrency(), GBPCurrency(), usdGbp), reset, reset);
                ExchangeRateManager::instance().add(
                    ExchangeRate(GBPCurrency(), EURCurrency(), realisedFx / usdGbp), reset,
                    reset);
            }
        }

    auto spot = makeQuoteHandle(spotFx);
    mtm->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, spot));

    // Reference: constant-notional swap whose EUR notional is the realised reset notional (usdNominal * realisedFx)
    Real realisedEurNotional = usdNominal * realisedFx;
    auto ref = ext::make_shared<ConstNotionalCrossCurrencyBasisSwap>(
        usdNominal, USDCurrency(), sch, usdIndex, 0.0, 1.0, realisedEurNotional, EURCurrency(), sch,
        eurIndex, eurBasis, 1.0);
    ref->setPricingEngine(ext::make_shared<DiscountingConstNotionalCrossCurrencySwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, spot));

    Real tol = 1.0e-6 * usdNominal;
    Real diff = std::fabs(mtm->NPV() - ref->NPV());
    if (diff > tol)
        BOOST_ERROR("seasoned MtM reset does not match the equivalent constant-notional swap\n"
                    << std::setprecision(12) << "    MtM NPV:        " << mtm->NPV() << "\n"
                    << "    const-not. NPV: " << ref->NPV() << "\n"
                    << "    difference:     " << diff << "\n");
}

BOOST_AUTO_TEST_CASE(testSeasonedEurUsdMarketExchangeRate) {
    BOOST_TEST_MESSAGE("Testing a seasoned EURUSD MtM swap (exchange rate stored as USD per EUR, "
                       "the market convention) reprices to the equivalent constant-notional "
                       "swap...");

    SavedSettings backup;
    Date today(11, Sep, 2018);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> eurCurve = flatCurve(today, 0.01);
    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.02);
    auto eurIndex = ext::make_shared<Euribor>(3 * Months, eurCurve);
    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);

    TARGET cal;
    // a single in-progress 3M period (started a month ago)
    Date start = cal.advance(today, -1 * Months);
    Date end = start + 3 * Months;
    Schedule sch(start, end, 3 * Months, cal, ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Forward, false);

    Real eurNotional = 10000000.0;
    Spread basis = 20 * 1.0e-4; // on the EUR (base, constant) leg
    Rate marketFx = 1.10;       // EURUSD market convention: USD per EUR

    // base = EUR (constant), quote = USD (resettable); reset notional = EUR notional * USD per EUR.
    Real realizedUsdNotional = eurNotional * marketFx;
    auto mtm = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::PayFxBaseCurrency, eurNotional, EURCurrency(), sch,
        eurIndex, basis, 1.0, realizedUsdNotional, USDCurrency(), sch, usdIndex, 0.0, 1.0,
        /*isFxBaseCurrencyLegResettable=*/false);

    for (Size legNo = 0; legNo < 2; ++legNo)
        for (const auto& cf : mtm->leg(legNo))
            if (auto frc = ext::dynamic_pointer_cast<FloatingRateCoupon>(cf)) {
                Date fd = frc->fixingDate();
                if (fd <= today)
                    frc->index()->addFixing(fd, legNo == 0 ? 0.011 : 0.021, true);
            }
    for (const auto& cf : mtm->resettingLeg())
        if (auto cpn = ext::dynamic_pointer_cast<Coupon>(cf)) {
            Date reset = cpn->accrualStartDate();
            if (reset <= today)
                ExchangeRateManager::instance().add(
                    ExchangeRate(EURCurrency(), USDCurrency(), marketFx), reset, reset);
        }

    auto spot = makeQuoteHandle(marketFx); // USD per EUR = domestic(USD) per foreign(EUR)
    mtm->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, spot));

    auto ref = ext::make_shared<ConstNotionalCrossCurrencyBasisSwap>(
        eurNotional, EURCurrency(), sch, eurIndex, basis, 1.0, realizedUsdNotional, USDCurrency(),
        sch, usdIndex, 0.0, 1.0);
    ref->setPricingEngine(ext::make_shared<DiscountingConstNotionalCrossCurrencySwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, spot));

    Real tol = 1.0e-6 * realizedUsdNotional;
    Real diff = std::fabs(mtm->NPV() - ref->NPV());
    if (diff > tol)
        BOOST_ERROR("seasoned EURUSD MtM does not match the constant-notional reference\n"
                    << std::setprecision(12) << "    MtM NPV: " << mtm->NPV() << "\n"
                    << "    ref NPV: " << ref->NPV() << "\n"
                    << "    diff:    " << diff << "\n");
}

BOOST_AUTO_TEST_CASE(testSeasonedUsdJpyMarketExchangeRate) {
    BOOST_TEST_MESSAGE("Testing a seasoned USDJPY MtM swap (exchange rate stored as JPY per USD, "
                       "the market convention) reprices to the equivalent constant-notional "
                       "swap...");

    SavedSettings backup;
    Date today(11, Sep, 2018);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.02);
    Handle<YieldTermStructure> jpyCurve = flatCurve(today, 0.001);
    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);
    auto jpyIndex = ext::make_shared<JPYLibor>(3 * Months, jpyCurve);

    TARGET cal;
    Date start = cal.advance(today, -1 * Months);
    Date end = start + 3 * Months;
    Schedule sch(start, end, 3 * Months, cal, ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Forward, false);

    Real jpyNotional = 1500000000.0;
    Spread basis = 15 * 1.0e-4; // on the JPY (quote, constant) leg
    Rate marketFx = 150.0;      // USDJPY market convention: JPY per USD

    // base = USD (resettable), quote = JPY (constant); reset notional = JPY notional * USD per JPY.
    Real realizedUsdNotional = jpyNotional / marketFx;
    auto mtm = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::PayFxBaseCurrency, realizedUsdNotional, USDCurrency(), sch,
        usdIndex, 0.0, 1.0, jpyNotional, JPYCurrency(), sch, jpyIndex, basis, 1.0,
        /*isFxBaseCurrencyLegResettable=*/true);

    for (Size legNo = 0; legNo < 2; ++legNo)
        for (const auto& cf : mtm->leg(legNo))
            if (auto frc = ext::dynamic_pointer_cast<FloatingRateCoupon>(cf)) {
                Date fd = frc->fixingDate();
                if (fd <= today)
                    frc->index()->addFixing(fd, legNo == 0 ? 0.021 : 0.0005, true);
            }
    for (const auto& cf : mtm->resettingLeg())
        if (auto cpn = ext::dynamic_pointer_cast<Coupon>(cf)) {
            Date reset = cpn->accrualStartDate();
            if (reset <= today)
                ExchangeRateManager::instance().add(
                    ExchangeRate(USDCurrency(), JPYCurrency(), marketFx), reset, reset);
        }

    auto spot = makeQuoteHandle(marketFx); // JPY per USD = domestic(JPY) per foreign(USD)
    mtm->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        JPYCurrency(), jpyCurve, USDCurrency(), usdCurve, spot));

    auto ref = ext::make_shared<ConstNotionalCrossCurrencyBasisSwap>(
        realizedUsdNotional, USDCurrency(), sch, usdIndex, 0.0, 1.0, jpyNotional, JPYCurrency(),
        sch, jpyIndex, basis, 1.0);
    ref->setPricingEngine(ext::make_shared<DiscountingConstNotionalCrossCurrencySwapEngine>(
        JPYCurrency(), jpyCurve, USDCurrency(), usdCurve, spot));

    Real tol = 1.0e-6 * jpyNotional;
    Real diff = std::fabs(mtm->NPV() - ref->NPV());
    if (diff > tol)
        BOOST_ERROR("seasoned USDJPY MtM does not match the constant-notional reference\n"
                    << std::setprecision(12) << "    MtM NPV: " << mtm->NPV() << "\n"
                    << "    ref NPV: " << ref->NPV() << "\n"
                    << "    diff:    " << diff << "\n");
}

BOOST_AUTO_TEST_CASE(testSeasonedOvernightLegsMatchConstantNotional) {
    BOOST_TEST_MESSAGE("Testing that a seasoned MtM swap on overnight indices reprices to the "
                       "equivalent constant-notional swap and accrues like its underlying "
                       "overnight coupon...");

    SavedSettings backup;
    Date today(11, Sep, 2018);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.02);
    Handle<YieldTermStructure> eurCurve = flatCurve(today, 0.01);
    auto usdIndex = ext::make_shared<Sofr>(usdCurve);
    auto eurIndex = ext::make_shared<Estr>(eurCurve);

    TARGET cal;
    // a single in-progress 3M period (started a month ago)
    Date start = cal.advance(today, -1 * Months);
    Date end = start + 3 * Months;
    Schedule sch(start, end, 3 * Months, cal, ModifiedFollowing, ModifiedFollowing,
                 DateGeneration::Forward, false);

    Real usdNominal = 10000000.0;
    Rate spotFx = 1.10;            // USD per EUR
    Spread eurBasis = 25 * 1.0e-4; // basis on the resettable EUR leg
    Rate realisedFx = 0.92;        // realised EUR per USD at the past reset

    // Seed the realised overnight fixings up to today.
    for (const auto& index : {ext::shared_ptr<OvernightIndex>(usdIndex),
                              ext::shared_ptr<OvernightIndex>(eurIndex)}) {
        Rate fixing = index == usdIndex ? 0.021 : 0.012;
        for (Date d = start; d < today; ++d)
            if (index->fixingCalendar().isBusinessDay(d))
                index->addFixing(d, fixing, true);
    }
    ExchangeRateManager::instance().add(ExchangeRate(USDCurrency(), EURCurrency(), realisedFx),
                                        start, start);

    auto spot = makeQuoteHandle(spotFx);
    auto mtm = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::PayFxBaseCurrency, usdNominal, USDCurrency(), sch,
        usdIndex, 0.0, 1.0, usdNominal / spotFx, EURCurrency(), sch, eurIndex, eurBasis, 1.0,
        /*isFxBaseCurrencyLegResettable=*/false);
    mtm->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, spot));

    Real realisedEurNotional = usdNominal * realisedFx;
    auto ref = ext::make_shared<ConstNotionalCrossCurrencyBasisSwap>(
        usdNominal, USDCurrency(), sch, usdIndex, 0.0, 1.0, realisedEurNotional, EURCurrency(),
        sch, eurIndex, eurBasis, 1.0);
    ref->setPricingEngine(ext::make_shared<DiscountingConstNotionalCrossCurrencySwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, spot));

    Real tol = 1.0e-6 * usdNominal;
    QL_CHECK_SMALL(mtm->NPV() - ref->NPV(), tol);

    // The FX-resetting coupon must accrue like the reference constant-notional
    // coupon built on the realised reset notional: only the overnight fixings
    // realised so far compound into the accrued amount.
    ext::shared_ptr<FxResetCoupon> mtmCoupon;
    for (const auto& cf : mtm->resettingLeg())
        if (auto c = ext::dynamic_pointer_cast<FxResetCoupon>(cf))
            mtmCoupon = c;
    ext::shared_ptr<Coupon> refCoupon;
    for (const auto& cf : ref->leg(1))
        if (auto c = ext::dynamic_pointer_cast<Coupon>(cf))
            refCoupon = c;
    BOOST_REQUIRE(mtmCoupon != nullptr && refCoupon != nullptr);
    setFxResetPricer(
        mtm->resettingLeg(),
        ext::make_shared<DiscountingFxResetPricer>(
            USDCurrency(), EURCurrency(), usdCurve, eurCurve, spot, false));
    QL_CHECK_CLOSE(mtmCoupon->nominal(), refCoupon->nominal(), 1.0e-8);
    QL_CHECK_CLOSE(mtmCoupon->accruedAmount(today), refCoupon->accruedAmount(today), 1.0e-8);
    QL_CHECK_CLOSE(mtmCoupon->amount(), refCoupon->amount(), 1.0e-8);
}

BOOST_AUTO_TEST_CASE(testResetExchangePaymentDates) {
    BOOST_TEST_MESSAGE("Testing initial, interim and final reset-exchange payment dates...");

    SavedSettings backup;
    Date today(11, Sep, 2018);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> usdCurve = flatCurve(today, 0.02);
    Handle<YieldTermStructure> eurCurve = flatCurve(today, 0.01);
    auto usdIndex = ext::make_shared<USDLibor>(3 * Months, usdCurve);
    auto eurIndex = ext::make_shared<Euribor>(3 * Months, eurCurve);

    TARGET cal;
    // Unadjusted schedule with period-end dates falling on weekends: the
    // coupons are lagged, while principal exchanges remain on the effective
    // and maturity dates adjusted with the explicit payment convention.
    Date start(15, Sep, 2018); // a Saturday
    Schedule sch(start, start + 1 * Years, 3 * Months, cal, Unadjusted, Unadjusted,
                 DateGeneration::Forward, false);

    Real usdNominal = 10000000.0;
    Rate spotFx = 1.10;
    auto swap = ext::make_shared<MtMCrossCurrencyBasisSwap>(
        MtMCrossCurrencyBasisSwap::PayFxBaseCurrency, usdNominal, USDCurrency(), sch,
        usdIndex, 0.0, 1.0, usdNominal / spotFx, EURCurrency(), sch, eurIndex, 0.0, 1.0,
        /*isFxBaseCurrencyLegResettable=*/false, /*fxResetFixingDays=*/0, /*fxResetFixingCalendar=*/Calendar(),
        /*fxBasePaymentLag=*/2, /*fxQuotePaymentLag=*/2,
        /*fxBasePaymentConvention=*/Preceding,
        /*fxQuotePaymentConvention=*/Preceding);
    swap->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
        USDCurrency(), usdCurve, EURCurrency(), eurCurve, makeQuoteHandle(spotFx)));
    BOOST_CHECK_NO_THROW(swap->NPV());

    std::vector<ext::shared_ptr<FxResetNotionalExchange> > exchanges;
    std::vector<ext::shared_ptr<Coupon> > coupons;
    for (const auto& cf : swap->leg(swap->resettingLegIndex())) {
        if (auto exchange = ext::dynamic_pointer_cast<FxResetNotionalExchange>(cf)) {
            exchanges.push_back(exchange);
        } else if (auto coupon = ext::dynamic_pointer_cast<Coupon>(cf)) {
            coupons.push_back(coupon);
        }
    }

    BOOST_REQUIRE_EQUAL(exchanges.size(), coupons.size() + 1);
    BOOST_CHECK_EQUAL(exchanges.front()->date(), cal.adjust(start, Preceding));
    BOOST_CHECK_EQUAL(exchanges.back()->date(), cal.adjust(start + 1 * Years, Preceding));

    for (Size i = 1; i + 1 < exchanges.size(); ++i) {
        BOOST_REQUIRE(exchanges[i]->currentReset());
        BOOST_CHECK_EQUAL(exchanges[i]->date(), exchanges[i]->currentReset()->valueDate()); // NOLINT(bugprone-unchecked-optional-access)
    }

    const Leg& constantLeg = swap->leg(swap->constantLegIndex());
    std::vector<ext::shared_ptr<SimpleCashFlow> > constantNotionalExchanges;
    for (const auto& cf : constantLeg)
        if (auto exchange = ext::dynamic_pointer_cast<SimpleCashFlow>(cf))
            constantNotionalExchanges.push_back(exchange);
    BOOST_REQUIRE_EQUAL(constantNotionalExchanges.size(), 2U);
    // Both legs exchange notionals on the same business dates, even when the
    // unadjusted schedule dates fall on a weekend.
    BOOST_CHECK_EQUAL(constantNotionalExchanges.front()->date(), exchanges.front()->date());
    BOOST_CHECK_EQUAL(constantNotionalExchanges.back()->date(), exchanges.back()->date());
    BOOST_CHECK_NE(coupons.back()->date(), exchanges.back()->date());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
