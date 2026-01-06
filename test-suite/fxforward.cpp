/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024

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

    BOOST_CHECK_EQUAL(fwd1.nominal1(), usdNominal);
    BOOST_CHECK_EQUAL(fwd1.nominal2(), sgdNominal);
    BOOST_CHECK(fwd1.currency1() == vars.usd);
    BOOST_CHECK(fwd1.currency2() == vars.sgd);
    BOOST_CHECK_EQUAL(fwd1.maturityDate(), vars.maturityDate);
    BOOST_CHECK_EQUAL(fwd1.payCurrency1(), true);
    BOOST_CHECK_EQUAL(fwd1.isExpired(), false);
}


BOOST_AUTO_TEST_CASE(testFxForwardConstructionWithRate) {
    BOOST_TEST_MESSAGE("Testing FX forward construction with rate...");

    CommonVars vars;

    Real nominal = 1000000.0;
    Real forwardRate = 1.36; // SGD/USD forward rate

    FxForward fwd(nominal, vars.usd, vars.sgd, forwardRate, vars.maturityDate,
                  true); // sell USD

    BOOST_CHECK_EQUAL(fwd.nominal1(), nominal);
    BOOST_CHECK_CLOSE(fwd.nominal2(), nominal * forwardRate, 1.0e-4); // 0.0001% tolerance
    BOOST_CHECK(fwd.currency1() == vars.usd);
    BOOST_CHECK(fwd.currency2() == vars.sgd);
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
        vars.usd, vars.usdCurveHandle, vars.sgd, vars.sgdCurveHandle, vars.spotFxHandle);

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
        vars.usd, vars.usdCurveHandle, vars.sgd, vars.sgdCurveHandle, vars.spotFxHandle);

    fwd.setPricingEngine(engine);

    // Fair forward rate = Spot * (DFforeign / DFdomestic)
    // With USD as currency1 (domestic) and SGD as currency2 (foreign):
    // F = S * (DF_SGD / DF_USD)
    Real spotFx = vars.spotFxHandle->value();
    Real dfUsd = vars.usdCurveHandle->discount(vars.maturityDate);
    Real dfSgd = vars.sgdCurveHandle->discount(vars.maturityDate);
    Real expectedFairRate = spotFx * dfSgd / dfUsd;

    Real calculatedFairRate = fwd.fairForwardRate();

    BOOST_CHECK_CLOSE(calculatedFairRate, expectedFairRate, 1.0e-4); // 0.0001% tolerance

    BOOST_TEST_MESSAGE("Spot FX: " << spotFx);
    BOOST_TEST_MESSAGE("DF USD: " << dfUsd);
    BOOST_TEST_MESSAGE("DF SGD: " << dfSgd);
    BOOST_TEST_MESSAGE("Expected Fair Rate: " << expectedFairRate);
    BOOST_TEST_MESSAGE("Calculated Fair Rate: " << calculatedFairRate);
}


BOOST_AUTO_TEST_CASE(testAtTheMoney) {
    BOOST_TEST_MESSAGE("Testing FX forward at-the-money has zero NPV...");

    CommonVars vars;

    // For an ATM forward, we need NPV = 0
    // NPV = -nominal1 * df1 + nominal2 * df2 / spotFx = 0
    // Solving: nominal2 = nominal1 * df1 * spotFx / df2

    Real spotFx = vars.spotFxHandle->value();
    Real dfUsd = vars.usdCurveHandle->discount(vars.maturityDate);
    Real dfSgd = vars.sgdCurveHandle->discount(vars.maturityDate);

    // The fair forward rate (for reference)
    Real fairForwardRate = spotFx * dfSgd / dfUsd;

    // Create a forward at the ATM strike
    // ATM condition: nominal2 = nominal1 * df1 * spotFx / df2
    Real usdNominal = 1000000.0;
    Real sgdNominal = usdNominal * dfUsd * spotFx / dfSgd;

    FxForward fwd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate, true);

    auto engine = ext::make_shared<DiscountingFxForwardEngine>(
        vars.usd, vars.usdCurveHandle, vars.sgd, vars.sgdCurveHandle, vars.spotFxHandle);

    fwd.setPricingEngine(engine);

    // At-the-money forward should have NPV close to zero
    Real npv = fwd.NPV();

    BOOST_TEST_MESSAGE("Spot FX: " << spotFx);
    BOOST_TEST_MESSAGE("DF USD: " << dfUsd);
    BOOST_TEST_MESSAGE("DF SGD: " << dfSgd);
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

    // Long USD (pay SGD, receive USD) - payCurrency1 = false
    FxForward longUsd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate, false);

    // Short USD (pay USD, receive SGD) - payCurrency1 = true
    FxForward shortUsd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate, true);

    auto engine = ext::make_shared<DiscountingFxForwardEngine>(
        vars.usd, vars.usdCurveHandle, vars.sgd, vars.sgdCurveHandle, vars.spotFxHandle);

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
        vars.usd, vars.usdCurveHandle, vars.sgd, vars.sgdCurveHandle, vars.spotFxHandle);

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

    // Higher USD rates should decrease the PV of paying USD and increase overall NPV
    // Higher SGD rates should decrease the PV of receiving SGD and decrease overall NPV
    // Verify sensitivities are non-zero
    BOOST_CHECK(std::fabs(npvUsdUp - npvBase) > vars.tolerance);
    BOOST_CHECK(std::fabs(npvSgdUp - npvBase) > vars.tolerance);
}


BOOST_AUTO_TEST_CASE(testSpotFxSensitivity) {
    BOOST_TEST_MESSAGE("Testing FX forward sensitivity to spot FX...");

    CommonVars vars;

    Real usdNominal = 1000000.0;
    Real sgdNominal = 1350000.0;

    FxForward fwd(usdNominal, vars.usd, sgdNominal, vars.sgd, vars.maturityDate,
                  true); // pay USD, receive SGD

    auto engine = ext::make_shared<DiscountingFxForwardEngine>(
        vars.usd, vars.usdCurveHandle, vars.sgd, vars.sgdCurveHandle, vars.spotFxHandle);

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
    // When paying USD and receiving SGD:
    // - SGD leg PV in USD terms = SGD_PV / spotFx
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
        vars.usd, vars.usdCurveHandle, vars.sgd, vars.sgdCurveHandle, vars.spotFxHandle);

    fwd.setPricingEngine(engine);

    // Trigger calculation
    fwd.NPV();

    // Check additional results are available
    auto additionalResults = fwd.additionalResults();

    BOOST_CHECK(additionalResults.find("spotFx") != additionalResults.end());
    BOOST_CHECK(additionalResults.find("currency1DiscountFactor") != additionalResults.end());
    BOOST_CHECK(additionalResults.find("currency2DiscountFactor") != additionalResults.end());

    Real spotFx = ext::any_cast<Real>(additionalResults.at("spotFx"));
    Real df1 = ext::any_cast<Real>(additionalResults.at("currency1DiscountFactor"));
    Real df2 = ext::any_cast<Real>(additionalResults.at("currency2DiscountFactor"));

    BOOST_TEST_MESSAGE("Additional Results:");
    BOOST_TEST_MESSAGE("  Spot FX: " << spotFx);
    BOOST_TEST_MESSAGE("  Currency1 DF: " << df1);
    BOOST_TEST_MESSAGE("  Currency2 DF: " << df2);

    BOOST_CHECK_CLOSE(spotFx, 1.35, 1.0e-4); // 0.0001% tolerance
    BOOST_CHECK(df1 > 0.0 && df1 < 1.0);
    BOOST_CHECK(df2 > 0.0 && df2 < 1.0);
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
