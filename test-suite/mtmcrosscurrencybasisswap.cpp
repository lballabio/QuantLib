/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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
#include <ql/cashflows/coupon.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/currencies/all.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/experimental/termstructures/crosscurrencyratehelpers.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/jpylibor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/instruments/constnotionalcrosscurrencybasisswap.hpp>
#include <ql/instruments/mtmcrosscurrencybasisswap.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/pricingengines/swap/discountingconstnotionalcrosscurrencyswapengine.hpp>
#include <ql/pricingengines/swap/discountingmtmcrosscurrencybasisswapengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/target.hpp>
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

    struct ExchangeRateManagerCleaner {
        ExchangeRateManagerCleaner() { ExchangeRateManager::instance().clear(); }
        ~ExchangeRateManagerCleaner() { ExchangeRateManager::instance().clear(); }
    };

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
            MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency,
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
                for (const auto& q : basisData)
                    helpers.push_back(ext::make_shared<MtMCrossCurrencyBasisSwapRateHelper>(
                        makeQuoteHandle(q.second * bp), q.first, fixingDays, cal, conv, endOfMonth,
                        eurIndex, usdIndex, collateral, isFxBaseCurrencyCollateralCurrency,
                        isBasisOnFxBaseCurrencyLeg, isFxBaseCurrencyLegResettable));

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
                        MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency, 1.0, EURCurrency(),
                        eurSch, eurIndex, baseLegBasis, 1.0, 1.0, USDCurrency(), usdSch, usdIndex,
                        quoteLegBasis, 1.0, isFxBaseCurrencyLegResettable);
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
            }
}

BOOST_AUTO_TEST_CASE(testSeasonedResetPeriodNeedsExchangeRate) {
    BOOST_TEST_MESSAGE(
        "Testing that an already-started MtM reset period requires a stored exchange rate...");

    SavedSettings backup;
    ExchangeRateManagerCleaner exchangeRateCleaner;
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
        MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency,
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
    ExchangeRateManagerCleaner exchangeRateCleaner;
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
        MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency, usdNominal, USDCurrency(), sch, usdIndex,
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
    for (const auto& cf : mtm->leg(mtm->resettingLegIndex()))
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
    ExchangeRateManagerCleaner exchangeRateCleaner;
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
        MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency, eurNotional, EURCurrency(), sch,
        eurIndex, basis, 1.0, realizedUsdNotional, USDCurrency(), sch, usdIndex, 0.0, 1.0,
        /*isFxBaseCurrencyLegResettable=*/false);

    for (Size legNo = 0; legNo < 2; ++legNo)
        for (const auto& cf : mtm->leg(legNo))
            if (auto frc = ext::dynamic_pointer_cast<FloatingRateCoupon>(cf)) {
                Date fd = frc->fixingDate();
                if (fd <= today)
                    frc->index()->addFixing(fd, legNo == 0 ? 0.011 : 0.021, true);
            }
    for (const auto& cf : mtm->leg(mtm->resettingLegIndex()))
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
    ExchangeRateManagerCleaner exchangeRateCleaner;
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
        MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency, realizedUsdNotional, USDCurrency(), sch,
        usdIndex, 0.0, 1.0, jpyNotional, JPYCurrency(), sch, jpyIndex, basis, 1.0,
        /*isFxBaseCurrencyLegResettable=*/true);

    for (Size legNo = 0; legNo < 2; ++legNo)
        for (const auto& cf : mtm->leg(legNo))
            if (auto frc = ext::dynamic_pointer_cast<FloatingRateCoupon>(cf)) {
                Date fd = frc->fixingDate();
                if (fd <= today)
                    frc->index()->addFixing(fd, legNo == 0 ? 0.021 : 0.0005, true);
            }
    for (const auto& cf : mtm->leg(mtm->resettingLegIndex()))
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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
