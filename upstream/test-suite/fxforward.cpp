/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Chirag Desai

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
#include <ql/currencies/america.hpp>
#include <ql/currencies/asia.hpp>
#include <ql/instruments/fxforward.hpp>
#include <ql/pricingengines/forward/discountingfxforwardengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(FxForwardTests)

struct CommonVars {
    // Common data
    Date today;
    Date maturityDate;
    Currency usd;
    Currency sgd;
    RelinkableHandle<YieldTermStructure> usdCurveHandle;
    RelinkableHandle<YieldTermStructure> sgdCurveHandle;
    RelinkableHandle<Quote> spotFxHandle;
    Real tolerance;

    // Setup in constructor
    CommonVars() {
        today = Date(15, March, 2024);
        Settings::instance().evaluationDate() = today;
        maturityDate = today + 6 * Months;

        usd = USDCurrency();
        sgd = SGDCurrency();

        // USD discount rate: 5%
        usdCurveHandle.linkTo(flatRate(today, 0.05, Actual365Fixed()));
        // SGD discount rate: 3.5%
        sgdCurveHandle.linkTo(flatRate(today, 0.035, Actual365Fixed()));

        // Spot FX rate: 1.35 SGD/USD (1 USD = 1.35 SGD)
        spotFxHandle.linkTo(ext::make_shared<SimpleQuote>(1.35));

        tolerance = 1.0e-6;
    }
};


BOOST_AUTO_TEST_CASE(testFxForwardConstruction) {
    BOOST_TEST_MESSAGE("Testing FX forward construction...");

    CommonVars vars;

    // Test constructor with two nominals
    Real usdNominal = 1000000.0;
    Real sgdNominal = 1350000.0;

    FxForward fwd1(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate,
                   true); // pay USD, receive SGD

    BOOST_CHECK_EQUAL(fwd1.sourceNominal(), usdNominal);
    BOOST_CHECK_EQUAL(fwd1.targetNominal(), sgdNominal);
    BOOST_CHECK(fwd1.sourceCurrency() == vars.usd);
    BOOST_CHECK(fwd1.targetCurrency() == vars.sgd);
    BOOST_CHECK_EQUAL(fwd1.maturityDate(), vars.maturityDate);
    BOOST_CHECK_EQUAL(fwd1.paySourceCurrency(), true);
    BOOST_CHECK_EQUAL(fwd1.isExpired(), false);
}


BOOST_AUTO_TEST_CASE(testFxForwardConstructionWithRate) {
    BOOST_TEST_MESSAGE("Testing FX forward construction with rate...");

    CommonVars vars;

    Real nominal = 1000000.0;
    Real forwardRate = 1.36; // SGD/USD forward rate

    FxForward fwd(nominal, vars.usd, vars.sgd, forwardRate, vars.maturityDate,
                  true); // sell USD

    BOOST_CHECK_EQUAL(fwd.sourceNominal(), nominal);
    BOOST_CHECK_CLOSE(fwd.targetNominal(), nominal * forwardRate, 1.0e-4); // 0.0001% tolerance
    BOOST_CHECK(fwd.sourceCurrency() == vars.usd);
    BOOST_CHECK(fwd.targetCurrency() == vars.sgd);
}


BOOST_AUTO_TEST_CASE(testContractedForwardRate) {
    BOOST_TEST_MESSAGE("Testing FX forward contracted rate...");

    CommonVars vars;

    // Test with explicit nominals
    Real usdNominal = 1000000.0;
    Real sgdNominal = 1350000.0;
    Real expectedRate = sgdNominal / usdNominal; // 1.35

    FxForward fwd1(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate, true);

    BOOST_CHECK_CLOSE(fwd1.forwardRate(), expectedRate, 1.0e-10);
    BOOST_TEST_MESSAGE("Contracted rate (from nominals): " << fwd1.forwardRate());

    // Test with rate constructor
    Real inputRate = 1.36;
    FxForward fwd2(usdNominal, vars.usd, vars.sgd, inputRate, vars.maturityDate, true);

    BOOST_CHECK_CLOSE(fwd2.forwardRate(), inputRate, 1.0e-10);
    BOOST_TEST_MESSAGE("Contracted rate (from rate constructor): " << fwd2.forwardRate());

    // Verify contracted rate differs from fair forward rate
    auto engine = ext::make_shared<DiscountingFxForwardEngine>(
        vars.usdCurveHandle, vars.sgdCurveHandle, vars.spotFxHandle);
    fwd1.setPricingEngine(engine);

    Real fairRate = fwd1.fairForwardRate();
    Real contractedRate = fwd1.forwardRate();

    BOOST_TEST_MESSAGE("Contracted rate: " << contractedRate);
    BOOST_TEST_MESSAGE("Fair forward rate: " << fairRate);

    // These should generally be different unless the contract is at fair value
    BOOST_CHECK(contractedRate != fairRate);
}


BOOST_AUTO_TEST_CASE(testFxForwardExpiry) {
    BOOST_TEST_MESSAGE("Testing FX forward expiry...");

    CommonVars vars;

    // Create a forward that has already expired
    Date pastDate = vars.today - 1 * Days;

    FxForward expiredFwd(1000000.0, vars.usd, 1350000.0, vars.sgd, pastDate, true);

    BOOST_CHECK_EQUAL(expiredFwd.isExpired(), true);
}


BOOST_AUTO_TEST_CASE(testDiscountingFxForwardEngine) {
    BOOST_TEST_MESSAGE("Testing discounting FX forward engine...");

    CommonVars vars;

    Real usdNominal = 1000000.0;
    Real sgdNominal = 1350000.0;

    FxForward fwd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate,
                  true); // pay USD, receive SGD

    auto engine = ext::make_shared<DiscountingFxForwardEngine>(
        vars.usdCurveHandle, vars.sgdCurveHandle, vars.spotFxHandle);

    fwd.setPricingEngine(engine);

    // Check that NPV is computed
    Real npv = fwd.NPV();
    BOOST_CHECK(npv != Null<Real>());

    // Check that fair forward rate is computed
    Real fairRate = fwd.fairForwardRate();
    BOOST_CHECK(fairRate > 0.0);

    BOOST_TEST_MESSAGE("NPV: " << npv);
    BOOST_TEST_MESSAGE("Fair Forward Rate: " << fairRate);
}


BOOST_AUTO_TEST_CASE(testFairForwardRate) {
    BOOST_TEST_MESSAGE("Testing fair forward rate calculation...");

    CommonVars vars;

    Real usdNominal = 1000000.0;
    Real sgdNominal = 1350000.0;

    FxForward fwd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate, true);

    auto engine = ext::make_shared<DiscountingFxForwardEngine>(
        vars.usdCurveHandle, vars.sgdCurveHandle, vars.spotFxHandle);

    fwd.setPricingEngine(engine);

    // Fair forward rate = Spot * (DFforeign / DFdomestic)
    // The engine calculates discount factors from settlement date to maturity
    // With USD as source currency (domestic) and SGD as target currency (foreign):
    // F = S * (DF_SGD / DF_USD)
    Date settlementDate = fwd.settlementDate();
    Real spotFx = vars.spotFxHandle->value();
    Real dfUsd = vars.usdCurveHandle->discount(vars.maturityDate) /
                 vars.usdCurveHandle->discount(settlementDate);
    Real dfSgd = vars.sgdCurveHandle->discount(vars.maturityDate) /
                 vars.sgdCurveHandle->discount(settlementDate);
    Real expectedFairRate = spotFx * dfSgd / dfUsd;

    Real calculatedFairRate = fwd.fairForwardRate();

    BOOST_CHECK_CLOSE(calculatedFairRate, expectedFairRate, 1.0e-4); // 0.0001% tolerance

    BOOST_TEST_MESSAGE("Settlement Date: " << settlementDate);
    BOOST_TEST_MESSAGE("Spot FX: " << spotFx);
    BOOST_TEST_MESSAGE("DF USD (settlement to maturity): " << dfUsd);
    BOOST_TEST_MESSAGE("DF SGD (settlement to maturity): " << dfSgd);
    BOOST_TEST_MESSAGE("Expected Fair Rate: " << expectedFairRate);
    BOOST_TEST_MESSAGE("Calculated Fair Rate: " << calculatedFairRate);
}


BOOST_AUTO_TEST_CASE(testAtTheMoney) {
    BOOST_TEST_MESSAGE("Testing FX forward at-the-money has zero NPV...");

    CommonVars vars;

    // For an ATM forward, we need NPV = 0
    // The engine calculates discount factors from settlement date to maturity:
    //   dfSource = DF(maturity) / DF(settlement)
    //   dfTarget = DF(maturity) / DF(settlement)
    // NPV = -sourceNominal * dfSource + targetNominal * dfTarget / spotFx = 0
    // Solving: targetNominal = sourceNominal * dfSource * spotFx / dfTarget

    Real spotFx = vars.spotFxHandle->value();
    
    // We need to use a forward with default settlement (2 days) to compute the correct DFs
    Real usdNominal = 1000000.0;
    
    // Create a temporary forward to get the settlement date
    FxForward tempFwd(usdNominal, vars.usd, usdNominal, vars.sgd, vars.maturityDate, true);
    Date settlementDate = tempFwd.settlementDate();
    
    // Calculate discount factors from settlement to maturity (as the engine does)
    Real dfUsd = vars.usdCurveHandle->discount(vars.maturityDate) / 
                 vars.usdCurveHandle->discount(settlementDate);
    Real dfSgd = vars.sgdCurveHandle->discount(vars.maturityDate) / 
                 vars.sgdCurveHandle->discount(settlementDate);

    // The fair forward rate (for reference)
    Real fairForwardRate = spotFx * dfSgd / dfUsd;

    // Create a forward at the ATM strike
    // ATM condition: targetNominal = sourceNominal * dfSource * spotFx / dfTarget
    Real sgdNominal = usdNominal * dfUsd * spotFx / dfSgd;

    FxForward fwd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate, true);

    auto engine = ext::make_shared<DiscountingFxForwardEngine>(
        vars.usdCurveHandle, vars.sgdCurveHandle, vars.spotFxHandle);

    fwd.setPricingEngine(engine);

    // At-the-money forward should have NPV close to zero
    Real npv = fwd.NPV();

    BOOST_TEST_MESSAGE("Settlement Date: " << settlementDate);
    BOOST_TEST_MESSAGE("Spot FX: " << spotFx);
    BOOST_TEST_MESSAGE("DF USD (settlement to maturity): " << dfUsd);
    BOOST_TEST_MESSAGE("DF SGD (settlement to maturity): " << dfSgd);
    BOOST_TEST_MESSAGE("Fair Forward Rate: " << fairForwardRate);
    BOOST_TEST_MESSAGE("USD Nominal: " << usdNominal);
    BOOST_TEST_MESSAGE("SGD Nominal (ATM): " << sgdNominal);
    BOOST_TEST_MESSAGE("ATM NPV: " << npv);

    BOOST_CHECK_SMALL(npv, 1.0e-4); // NPV should be essentially zero
}


BOOST_AUTO_TEST_CASE(testPositionDirection) {
    BOOST_TEST_MESSAGE("Testing FX forward position direction...");

    CommonVars vars;

    Real usdNominal = 1000000.0;
    Real sgdNominal = 1350000.0;

    // Long USD (pay SGD, receive USD) - paySourceCurrency = false
    FxForward longUsd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate, false);

    // Short USD (pay USD, receive SGD) - paySourceCurrency = true
    FxForward shortUsd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate, true);

    auto engine = ext::make_shared<DiscountingFxForwardEngine>(
        vars.usdCurveHandle, vars.sgdCurveHandle, vars.spotFxHandle);

    longUsd.setPricingEngine(engine);
    shortUsd.setPricingEngine(engine);

    Real npvLong = longUsd.NPV();
    Real npvShort = shortUsd.NPV();

    // Long and short positions should have opposite NPVs
    BOOST_CHECK_CLOSE(npvLong, -npvShort, 1.0e-4); // 0.0001% tolerance

    BOOST_TEST_MESSAGE("Long USD NPV: " << npvLong);
    BOOST_TEST_MESSAGE("Short USD NPV: " << npvShort);
}


BOOST_AUTO_TEST_CASE(testIRCurveSensitivity) {
    BOOST_TEST_MESSAGE("Testing FX forward sensitivity to IR curves...");

    CommonVars vars;

    Real usdNominal = 1000000.0;
    Real sgdNominal = 1350000.0;

    FxForward fwd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate, true);

    auto engine = ext::make_shared<DiscountingFxForwardEngine>(
        vars.usdCurveHandle, vars.sgdCurveHandle, vars.spotFxHandle);

    fwd.setPricingEngine(engine);

    Real npvBase = fwd.NPV();

    // Shift USD curve up by 10bp
    vars.usdCurveHandle.linkTo(flatRate(vars.today, 0.051, Actual365Fixed()));
    Real npvUsdUp = fwd.NPV();

    // Reset USD curve and shift SGD curve up by 10bp
    vars.usdCurveHandle.linkTo(flatRate(vars.today, 0.05, Actual365Fixed()));
    vars.sgdCurveHandle.linkTo(flatRate(vars.today, 0.036, Actual365Fixed()));
    Real npvSgdUp = fwd.NPV();

    BOOST_TEST_MESSAGE("Base NPV: " << npvBase);
    BOOST_TEST_MESSAGE("NPV with USD +10bp: " << npvUsdUp);
    BOOST_TEST_MESSAGE("NPV with SGD +10bp: " << npvSgdUp);

    // When paying USD (source) and receiving SGD (target):
    // - Higher USD rates -> lower DF for USD leg -> less negative PV for paying USD -> NPV
    // increases
    // - Higher SGD rates -> lower DF for SGD leg -> less positive PV for receiving SGD -> NPV
    // decreases
    BOOST_CHECK(npvUsdUp > npvBase);
    BOOST_CHECK(npvSgdUp < npvBase);
}


BOOST_AUTO_TEST_CASE(testSpotFxSensitivity) {
    BOOST_TEST_MESSAGE("Testing FX forward sensitivity to spot FX...");

    CommonVars vars;

    Real usdNominal = 1000000.0;
    Real sgdNominal = 1350000.0;

    FxForward fwd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate,
                  true); // pay USD, receive SGD

    auto engine = ext::make_shared<DiscountingFxForwardEngine>(
        vars.usdCurveHandle, vars.sgdCurveHandle, vars.spotFxHandle);

    fwd.setPricingEngine(engine);

    Real npvBase = fwd.NPV();

    // Increase spot FX (spot = SGD/USD, so higher spot means USD strengthens / SGD weakens)
    vars.spotFxHandle.linkTo(ext::make_shared<SimpleQuote>(1.40));
    Real npvSpotUp = fwd.NPV();

    // Decrease spot FX (USD weakens / SGD strengthens)
    vars.spotFxHandle.linkTo(ext::make_shared<SimpleQuote>(1.30));
    Real npvSpotDown = fwd.NPV();

    BOOST_TEST_MESSAGE("Base NPV (spot=1.35): " << npvBase);
    BOOST_TEST_MESSAGE("NPV with spot=1.40: " << npvSpotUp);
    BOOST_TEST_MESSAGE("NPV with spot=1.30: " << npvSpotDown);

    // Spot FX convention: spotFx = SGD/USD (1 USD = spotFx SGD)
    // When paying USD (source) and receiving SGD (target):
    // - Target leg PV in source currency terms = SGD_PV / spotFx
    // - If spot goes UP (USD strengthens), SGD is worth LESS in USD terms, NPV DECREASES
    // - If spot goes DOWN (USD weakens), SGD is worth MORE in USD terms, NPV INCREASES
    BOOST_CHECK(npvSpotUp < npvBase);
    BOOST_CHECK(npvSpotDown > npvBase);
}


BOOST_AUTO_TEST_CASE(testAdditionalResults) {
    BOOST_TEST_MESSAGE("Testing FX forward additional results...");

    CommonVars vars;

    Real usdNominal = 1000000.0;
    Real sgdNominal = 1350000.0;

    FxForward fwd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate, true);

    auto engine = ext::make_shared<DiscountingFxForwardEngine>(
        vars.usdCurveHandle, vars.sgdCurveHandle, vars.spotFxHandle);

    fwd.setPricingEngine(engine);

    // Trigger calculation
    fwd.NPV();

    // Check additional results are available
    auto additionalResults = fwd.additionalResults();

    BOOST_CHECK(additionalResults.find("spotFx") != additionalResults.end());
    BOOST_CHECK(additionalResults.find("sourceCurrencyDiscountFactor") != additionalResults.end());
    BOOST_CHECK(additionalResults.find("targetCurrencyDiscountFactor") != additionalResults.end());

    Real spotFx = ext::any_cast<Real>(additionalResults.at("spotFx"));
    Real dfSource = ext::any_cast<Real>(additionalResults.at("sourceCurrencyDiscountFactor"));
    Real dfTarget = ext::any_cast<Real>(additionalResults.at("targetCurrencyDiscountFactor"));

    BOOST_TEST_MESSAGE("Additional Results:");
    BOOST_TEST_MESSAGE("  Spot FX: " << spotFx);
    BOOST_TEST_MESSAGE("  Source Currency DF: " << dfSource);
    BOOST_TEST_MESSAGE("  Target Currency DF: " << dfTarget);

    BOOST_CHECK_CLOSE(spotFx, 1.35, 1.0e-4); // 0.0001% tolerance
    BOOST_CHECK(dfSource > 0.0 && dfSource < 1.0);
    BOOST_CHECK(dfTarget > 0.0 && dfTarget < 1.0);
}


BOOST_AUTO_TEST_CASE(testSettlementDays) {
    BOOST_TEST_MESSAGE("Testing FX forward settlement days...");

    CommonVars vars;

    Real usdNominal = 1000000.0;
    Real sgdNominal = 1350000.0;

    // Test different settlement conventions
    // Overnight (O/N): 0 days
    FxForward overnightFwd(usdNominal, vars.usd, sgdNominal, vars.sgd, 
                           vars.maturityDate, true, 0);
    BOOST_CHECK_EQUAL(overnightFwd.settlementDays(), 0);
    BOOST_CHECK_EQUAL(overnightFwd.settlementDate(), vars.today);

    // TomNext (T/N): 1 day
    FxForward tomNextFwd(usdNominal, vars.usd, sgdNominal, vars.sgd, 
                         vars.maturityDate, true, 1);
    BOOST_CHECK_EQUAL(tomNextFwd.settlementDays(), 1);
    BOOST_CHECK_EQUAL(tomNextFwd.settlementDate(), vars.today + 1);

    // Spot (S/N): 2 days (default)
    FxForward spotFwd(usdNominal, vars.usd, sgdNominal, vars.sgd, 
                      vars.maturityDate, true);  // default is 2
    BOOST_CHECK_EQUAL(spotFwd.settlementDays(), 2);
    BOOST_CHECK_EQUAL(spotFwd.settlementDate(), vars.today + 2);

    BOOST_TEST_MESSAGE("Overnight settlement date: " << overnightFwd.settlementDate());
    BOOST_TEST_MESSAGE("TomNext settlement date: " << tomNextFwd.settlementDate());
    BOOST_TEST_MESSAGE("Spot settlement date: " << spotFwd.settlementDate());
}


BOOST_AUTO_TEST_CASE(testSettlementDaysWithCalendar) {
    BOOST_TEST_MESSAGE("Testing FX forward settlement days with calendar...");

    CommonVars vars;

    Real usdNominal = 1000000.0;
    Real sgdNominal = 1350000.0;

    // Use a calendar that skips weekends
    Calendar cal = TARGET();

    // Find a Friday to test weekend skipping
    Date friday(15, March, 2024);  // March 15, 2024 is a Friday
    Settings::instance().evaluationDate() = friday;

    // With 2 settlement days on a Friday, settlement should be Tuesday (skipping weekend)
    FxForward fwd(usdNominal, vars.usd, sgdNominal, vars.sgd, 
                  vars.maturityDate, true, 2, cal);

    Date expectedSettlementDate = cal.advance(friday, 2, Days);
    BOOST_CHECK_EQUAL(fwd.settlementDate(), expectedSettlementDate);

    BOOST_TEST_MESSAGE("Evaluation date (Friday): " << friday);
    BOOST_TEST_MESSAGE("Settlement date (should skip weekend): " << fwd.settlementDate());
    BOOST_TEST_MESSAGE("Expected settlement date: " << expectedSettlementDate);

    // Restore evaluation date
    Settings::instance().evaluationDate() = vars.today;
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
