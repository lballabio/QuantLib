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
#include "utilities.hpp"
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/currencies/america.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/experimental/fx/makemtmcrosscurrencybasisswap.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/instruments/makeconstnotionalcrosscurrencybasisswap.hpp>
#include <ql/instruments/makeconstnotionalcrosscurrencyfixedvsfloatingswap.hpp>
#include <ql/instruments/makeconstnotionalcrosscurrencyswap.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/pricingengines/swap/discountingconstnotionalcrosscurrencyswapengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(MakeCrossCurrencySwapTests)

namespace {

    struct CommonVars {
        Date today;
        // discounting and forecasting curves are kept distinct so that the
        // legs are not trivially par and the cross-currency basis is non-zero
        Handle<YieldTermStructure> eurCurve, usdCurve;
        Handle<YieldTermStructure> eurForecast, usdForecast;
        ext::shared_ptr<IborIndex> eurIndex, usdIndex;
        Handle<Quote> spotFX; // USD per EUR

        CommonVars() {
            today = Date(15, Jun, 2023);
            Settings::instance().evaluationDate() = today;

            eurCurve = Handle<YieldTermStructure>(
                ext::make_shared<FlatForward>(today, 0.0280, Actual360()));
            usdCurve = Handle<YieldTermStructure>(
                ext::make_shared<FlatForward>(today, 0.0475, Actual360()));
            eurForecast = Handle<YieldTermStructure>(
                ext::make_shared<FlatForward>(today, 0.0310, Actual360()));
            usdForecast = Handle<YieldTermStructure>(
                ext::make_shared<FlatForward>(today, 0.0530, Actual360()));

            eurIndex = ext::make_shared<Euribor>(3 * Months, eurForecast);
            usdIndex = ext::make_shared<USDLibor>(3 * Months, usdForecast);

            spotFX = Handle<Quote>(ext::make_shared<SimpleQuote>(1.10));
        }
    };

}

BOOST_AUTO_TEST_CASE(testConstNotionalBasisSwapBuilder) {
    BOOST_TEST_MESSAGE("Testing MakeConstNotionalCrossCurrencyBasisSwap builder...");

    CommonVars vars;

    ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> swap =
        MakeConstNotionalCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex, 10000000.0)
            .withFxSpot(vars.spotFX)
            .withPayDiscountCurve(vars.eurCurve)
            .withRecDiscountCurve(vars.usdCurve);

    BOOST_CHECK_EQUAL(swap->payCurrency(), EURCurrency());
    BOOST_CHECK_EQUAL(swap->recCurrency(), USDCurrency());
    BOOST_CHECK_CLOSE(swap->recNominal(), 10000000.0 * 1.10, 1.0e-8);

    Spread fairPaySpread = swap->fairPaySpread();
    Spread fairRecSpread = swap->fairRecSpread();
    BOOST_CHECK(fairPaySpread != Null<Spread>());
    BOOST_CHECK(fairRecSpread != Null<Spread>());

    // Rebuilding the swap at the fair pay spread should reprice it to par.
    ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> parSwap =
        MakeConstNotionalCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex, 10000000.0)
            .withFxSpot(vars.spotFX)
            .withPaySpread(fairPaySpread)
            .withPayDiscountCurve(vars.eurCurve)
            .withRecDiscountCurve(vars.usdCurve);

    if (std::fabs(parSwap->NPV()) > 1.0)
        BOOST_ERROR("Failed to reproduce par NPV using the fair pay spread:\n"
                    << "    NPV: " << parSwap->NPV());
}

BOOST_AUTO_TEST_CASE(testConstNotionalFixedVsFloatingBuilder) {
    BOOST_TEST_MESSAGE(
        "Testing MakeConstNotionalCrossCurrencyFixedVsFloatingSwap builder...");

    CommonVars vars;

    // Fixed rate left unspecified: the builder should solve for the fair rate.
    ext::shared_ptr<ConstNotionalCrossCurrencyFixedVsFloatingSwap> swap =
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap(5 * Years, EURCurrency(), vars.usdIndex,
                                                           Null<Rate>(), 10000000.0)
            .withFxSpot(vars.spotFX)
            .withFixedDiscountCurve(vars.eurCurve)
            .withFloatDiscountCurve(vars.usdCurve);

    BOOST_CHECK(swap->fixedRate() != Null<Rate>());

    if (std::fabs(swap->NPV()) > 1.0)
        BOOST_ERROR("Failed to solve for the at-market fixed rate:\n"
                    << "    NPV: " << swap->NPV());

    // Explicit fixed rate: fairRate()/fairSpread() should still be available.
    ext::shared_ptr<ConstNotionalCrossCurrencyFixedVsFloatingSwap> offMarketSwap =
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap(5 * Years, EURCurrency(), vars.usdIndex,
                                                           0.02, 10000000.0)
            .withFxSpot(vars.spotFX)
            .withFixedDiscountCurve(vars.eurCurve)
            .withFloatDiscountCurve(vars.usdCurve);

    BOOST_CHECK(offMarketSwap->fairRate() != Null<Rate>());
    BOOST_CHECK(offMarketSwap->fairSpread() != Null<Spread>());
}

BOOST_AUTO_TEST_CASE(testConstNotionalCrossCurrencySwapGenericBuilder) {
    BOOST_TEST_MESSAGE("Testing MakeConstNotionalCrossCurrencySwap generic builder...");

    CommonVars vars;

    Date start = TARGET().advance(vars.today, 2 * Days);
    Schedule schedule(start, start + 2 * Years, 3 * Months, TARGET(), ModifiedFollowing,
                      ModifiedFollowing, DateGeneration::Backward, false);

    Leg payLeg = IborLeg(schedule, vars.eurIndex).withNotionals(1000000.0);
    Leg recLeg = IborLeg(schedule, vars.usdIndex).withNotionals(1100000.0);

    ext::shared_ptr<ConstNotionalCrossCurrencySwap> swap =
        MakeConstNotionalCrossCurrencySwap(payLeg, EURCurrency(), recLeg, USDCurrency())
            .withFxSpot(vars.spotFX)
            .withPayDiscountCurve(vars.eurCurve)
            .withRecDiscountCurve(vars.usdCurve);

    BOOST_CHECK_NO_THROW(swap->NPV());
    BOOST_CHECK_EQUAL(swap->legCurrency(0), EURCurrency());
    BOOST_CHECK_EQUAL(swap->legCurrency(1), USDCurrency());
}

BOOST_AUTO_TEST_CASE(testMtMBasisSwapBuilder) {
    BOOST_TEST_MESSAGE("Testing MakeMtMCrossCurrencyBasisSwap builder...");

    CommonVars vars;

    ext::shared_ptr<MtMCrossCurrencyBasisSwap> swap =
        MakeMtMCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex,
                                      /*isFxBaseCurrencyLegResettable=*/true, 10000000.0)
            .withFxSpot(vars.spotFX)
            .withFxBaseDiscountCurve(vars.eurCurve)
            .withFxQuoteDiscountCurve(vars.usdCurve);

    Spread fairFxBaseSpread = swap->fairFxBaseSpread();
    Spread fairFxQuoteSpread = swap->fairFxQuoteSpread();
    BOOST_CHECK(fairFxBaseSpread != Null<Spread>());
    BOOST_CHECK(fairFxQuoteSpread != Null<Spread>());

    ext::shared_ptr<MtMCrossCurrencyBasisSwap> parSwap =
        MakeMtMCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex, true, 10000000.0)
            .withFxSpot(vars.spotFX)
            .withFxBaseSpread(fairFxBaseSpread)
            .withFxBaseDiscountCurve(vars.eurCurve)
            .withFxQuoteDiscountCurve(vars.usdCurve);

    if (std::fabs(parSwap->NPV()) > 1.0)
        BOOST_ERROR("Failed to reproduce par NPV using the fair FX-base spread:\n"
                    << "    NPV: " << parSwap->NPV());
}

BOOST_AUTO_TEST_CASE(testAutoAttachedEngineMatchesManualEngine) {
    BOOST_TEST_MESSAGE("Testing that the auto-attached engine matches a hand-wired one...");

    CommonVars vars;
    const Real nominal = 10000000.0;

    ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> autoSwap =
        MakeConstNotionalCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex, nominal)
            .withPaySpread(0.0025)
            .withFxSpot(vars.spotFX)
            .withPayDiscountCurve(vars.eurCurve)
            .withRecDiscountCurve(vars.usdCurve);

    // same trade, with the engine wired up by hand
    ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> manualSwap =
        MakeConstNotionalCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex, nominal)
            .withPaySpread(0.0025)
            .withFxSpot(vars.spotFX);
    manualSwap->setPricingEngine(ext::make_shared<DiscountingConstNotionalCrossCurrencySwapEngine>(
        USDCurrency(), vars.usdCurve, EURCurrency(), vars.eurCurve, vars.spotFX));

    if (std::fabs(autoSwap->NPV() - manualSwap->NPV()) > 1.0e-6)
        BOOST_ERROR("The engine built by the helper class does not match a manual one:\n"
                    << "    auto:   " << autoSwap->NPV() << "\n"
                    << "    manual: " << manualSwap->NPV());

    // the NPV is expressed in the receive currency, so the pay leg must be
    // converted with the spot quoted as receive currency per pay currency
    Real expected =
        autoSwap->inCcyLegNPV(1) + autoSwap->inCcyLegNPV(0) * vars.spotFX->value();
    if (std::fabs(autoSwap->NPV() - expected) > 1.0e-6)
        BOOST_ERROR("The FX spot is not applied in the documented direction:\n"
                    << "    NPV:                            " << autoSwap->NPV() << "\n"
                    << "    recLegNPV + payLegNPV * fxSpot: " << expected);
}

BOOST_AUTO_TEST_CASE(testNotionalDerivationFromFxSpot) {
    BOOST_TEST_MESSAGE("Testing notional derivation from the FX spot quote...");

    CommonVars vars;
    const Real nominal = 10000000.0;
    const Real spot = vars.spotFX->value();

    // spot is quoted as receive currency per pay currency
    ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> basisSwap =
        MakeConstNotionalCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex, nominal)
            .withFxSpot(vars.spotFX)
            .withPayDiscountCurve(vars.eurCurve)
            .withRecDiscountCurve(vars.usdCurve);
    BOOST_CHECK_CLOSE(basisSwap->recNominal(), nominal * spot, 1.0e-8);

    // spot is quoted as floating currency per fixed currency, so the fixed
    // notional is the floating one divided by the spot
    ext::shared_ptr<ConstNotionalCrossCurrencyFixedVsFloatingSwap> fixedVsFloating =
        MakeConstNotionalCrossCurrencyFixedVsFloatingSwap(5 * Years, EURCurrency(), vars.usdIndex,
                                                          0.02, nominal)
            .withFxSpot(vars.spotFX)
            .withFixedDiscountCurve(vars.eurCurve)
            .withFloatDiscountCurve(vars.usdCurve);
    BOOST_CHECK_CLOSE(fixedVsFloating->fixedNominal(), nominal / spot, 1.0e-8);

    // spot is quoted as fx-quote currency per fx-base currency
    ext::shared_ptr<MtMCrossCurrencyBasisSwap> mtmSwap =
        MakeMtMCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex, true, nominal)
            .withFxSpot(vars.spotFX)
            .withFxBaseDiscountCurve(vars.eurCurve)
            .withFxQuoteDiscountCurve(vars.usdCurve);
    BOOST_CHECK_CLOSE(mtmSwap->fxQuoteNominal(), nominal * spot, 1.0e-8);

    // an explicitly given notional is never overridden
    ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> explicitNotional =
        MakeConstNotionalCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex, nominal)
            .withFxSpot(vars.spotFX)
            .withRecNominal(12000000.0)
            .withPayDiscountCurve(vars.eurCurve)
            .withRecDiscountCurve(vars.usdCurve);
    BOOST_CHECK_CLOSE(explicitNotional->recNominal(), 12000000.0, 1.0e-8);
}

BOOST_AUTO_TEST_CASE(testMissingFxSpotIsRejected) {
    BOOST_TEST_MESSAGE("Testing that a missing FX spot quote is reported...");

    CommonVars vars;
    const Real nominal = 10000000.0;

    // Without a spot quote the helper classes cannot convert the notional or
    // build the engine; assuming a unit FX rate would silently misprice the
    // swap, so an exception is expected instead.
    BOOST_CHECK_THROW(ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> swap =
                          MakeConstNotionalCrossCurrencyBasisSwap(5 * Years, vars.eurIndex,
                                                                   vars.usdIndex, nominal)
                              .withPayDiscountCurve(vars.eurCurve)
                              .withRecDiscountCurve(vars.usdCurve),
                      Error);

    BOOST_CHECK_THROW(ext::shared_ptr<ConstNotionalCrossCurrencyFixedVsFloatingSwap> swap =
                          MakeConstNotionalCrossCurrencyFixedVsFloatingSwap(
                              5 * Years, EURCurrency(), vars.usdIndex, 0.02, nominal)
                              .withFixedDiscountCurve(vars.eurCurve)
                              .withFloatDiscountCurve(vars.usdCurve),
                      Error);

    BOOST_CHECK_THROW(ext::shared_ptr<MtMCrossCurrencyBasisSwap> swap =
                          MakeMtMCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex,
                                                        true, nominal)
                              .withFxBaseDiscountCurve(vars.eurCurve)
                              .withFxQuoteDiscountCurve(vars.usdCurve),
                      Error);

    Date start = TARGET().advance(vars.today, 2 * Days);
    Schedule schedule = MakeSchedule().from(start).to(start + 2 * Years).withTenor(3 * Months);
    Leg payLeg = IborLeg(schedule, vars.eurIndex).withNotionals(nominal);
    Leg recLeg = IborLeg(schedule, vars.usdIndex).withNotionals(nominal * 1.10);
    BOOST_CHECK_THROW(ext::shared_ptr<ConstNotionalCrossCurrencySwap> swap =
                          MakeConstNotionalCrossCurrencySwap(payLeg, EURCurrency(), recLeg,
                                                              USDCurrency())
                              .withPayDiscountCurve(vars.eurCurve)
                              .withRecDiscountCurve(vars.usdCurve),
                      Error);

    // Legs in the same currency, on the other hand, need no FX spot at all.
    ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> singleCurrency =
        MakeConstNotionalCrossCurrencyBasisSwap(2 * Years, vars.eurIndex, vars.eurIndex, nominal)
            .withRecSpread(0.0010)
            .withPayDiscountCurve(vars.eurCurve)
            .withRecDiscountCurve(vars.eurCurve);
    BOOST_CHECK_NO_THROW(singleCurrency->NPV());

    // An explicitly supplied notional and engine also make the spot redundant.
    BOOST_CHECK_NO_THROW(ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> swap =
                             MakeConstNotionalCrossCurrencyBasisSwap(5 * Years, vars.eurIndex,
                                                                      vars.usdIndex, nominal)
                                 .withRecNominal(nominal * 1.10)
                                 .withPricingEngine(
                                     ext::make_shared<
                                         DiscountingConstNotionalCrossCurrencySwapEngine>(
                                         USDCurrency(), vars.usdCurve, EURCurrency(), vars.eurCurve,
                                         vars.spotFX)));
}

BOOST_AUTO_TEST_CASE(testIncompleteDiscountCurvesAreRejected) {
    BOOST_TEST_MESSAGE("Testing that a single discount curve is reported...");

    CommonVars vars;
    const Real nominal = 10000000.0;

    // Supplying one curve out of two cannot be anything but a mistake, and
    // would otherwise only surface as a null-engine error at pricing time.
    BOOST_CHECK_THROW(ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> swap =
                          MakeConstNotionalCrossCurrencyBasisSwap(5 * Years, vars.eurIndex,
                                                                   vars.usdIndex, nominal)
                              .withFxSpot(vars.spotFX)
                              .withPayDiscountCurve(vars.eurCurve),
                      Error);

    BOOST_CHECK_THROW(ext::shared_ptr<ConstNotionalCrossCurrencyFixedVsFloatingSwap> swap =
                          MakeConstNotionalCrossCurrencyFixedVsFloatingSwap(
                              5 * Years, EURCurrency(), vars.usdIndex, 0.02, nominal)
                              .withFxSpot(vars.spotFX)
                              .withFloatDiscountCurve(vars.usdCurve),
                      Error);

    BOOST_CHECK_THROW(ext::shared_ptr<MtMCrossCurrencyBasisSwap> swap =
                          MakeMtMCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex,
                                                        true, nominal)
                              .withFxSpot(vars.spotFX)
                              .withFxBaseDiscountCurve(vars.eurCurve),
                      Error);

    // Giving no curve at all remains legitimate: the user may want to attach
    // an engine to the instrument afterwards.
    ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> engineLess =
        MakeConstNotionalCrossCurrencyBasisSwap(5 * Years, vars.eurIndex, vars.usdIndex, nominal)
            .withFxSpot(vars.spotFX);
    engineLess->setPricingEngine(ext::make_shared<DiscountingConstNotionalCrossCurrencySwapEngine>(
        USDCurrency(), vars.usdCurve, EURCurrency(), vars.eurCurve, vars.spotFX));
    BOOST_CHECK_NO_THROW(engineLess->NPV());
}

BOOST_AUTO_TEST_CASE(testDefaultPaymentConventions) {
    BOOST_TEST_MESSAGE("Testing that default payment conventions follow the index...");

    CommonVars vars;
    const Real nominal = 10000000.0;

    // 30 September 2023 is a Saturday: with a Following convention the
    // payment would roll into October, as it would not with the
    // ModifiedFollowing convention of the underlying index.
    Date monthEnd(30, Sep, 2023);

    ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> swap =
        MakeConstNotionalCrossCurrencyBasisSwap(1 * Years, vars.eurIndex, vars.eurIndex, nominal)
            .withEffectiveDate(Date(30, Jun, 2023))
            .withEndOfMonth()
            .withPayDiscountCurve(vars.eurCurve)
            .withRecDiscountCurve(vars.eurCurve);

    ext::shared_ptr<VanillaSwap> vanillaSwap = MakeVanillaSwap(1 * Years, vars.eurIndex, 0.03)
                                                   .withEffectiveDate(Date(30, Jun, 2023))
                                                   .withFloatingLegEndOfMonth()
                                                   .withDiscountingTermStructure(vars.eurCurve);

    std::vector<Date> builderDates, vanillaDates;
    for (const auto& cf : swap->leg(0))
        if (ext::dynamic_pointer_cast<FloatingRateCoupon>(cf) != nullptr)
            builderDates.push_back(cf->date());
    for (const auto& cf : vanillaSwap->floatingLeg())
        vanillaDates.push_back(cf->date());

    if (builderDates != vanillaDates)
        BOOST_ERROR("Floating-leg payment dates differ from the MakeVanillaSwap equivalent");

    bool paidInMonthEnd = false;
    for (const Date& d : builderDates)
        if (d.month() == Sep && d.year() == 2023)
            paidInMonthEnd = true;
    if (!paidInMonthEnd)
        BOOST_ERROR("A month-end payment was rolled into the following month; the default "
                    "payment convention should follow the index ("
                    << monthEnd << " expected to be adjusted backwards)");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
