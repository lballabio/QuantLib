/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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
#include <ql/instruments/holderextensibleoption.hpp>
#include <ql/instruments/writerextensibleoption.hpp>
#include <ql/pricingengines/exotic/analyticholderextensibleoptionengine.hpp>
#include <ql/pricingengines/exotic/analyticwriterextensibleoptionengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(ExtensibleOptionsTests)

BOOST_AUTO_TEST_CASE(testAnalyticHolderExtensibleOptionEngine) {
    BOOST_TEST_MESSAGE(
        "Testing analytic engine for holder-extensible option...");

    Option::Type type = Option::Call;
    Real strike1 = 100.0;
    Real strike2 = 105.0;
    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();
    Date exDate1 = today + 180;
    Date exDate2 = today + 270;
    Real premium = 1.0;

    ext::shared_ptr<SimpleQuote> spot = ext::make_shared<SimpleQuote>(100.0);
    ext::shared_ptr<SimpleQuote> qRate = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<SimpleQuote> rRate = ext::make_shared<SimpleQuote>(0.08);
    ext::shared_ptr<SimpleQuote> vol = ext::make_shared<SimpleQuote>(0.25);

    ext::shared_ptr<StrikedTypePayoff> payoff =
        ext::make_shared<PlainVanillaPayoff>(type, strike1);
    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(exDate1);

    HolderExtensibleOption option(type, premium,
                                  exDate2, strike2,
                                  payoff, exercise);

    Handle<Quote> underlying(spot);
    Handle<YieldTermStructure> dividendTS(flatRate(today, qRate, dc));
    Handle<YieldTermStructure> riskFreeTS(flatRate(today, rRate, dc));
    Handle<BlackVolTermStructure> blackVolTS(flatVol(today, vol, dc));

    const ext::shared_ptr<BlackScholesMertonProcess> process =
        ext::make_shared<BlackScholesMertonProcess>(underlying,
                                                      dividendTS,
                                                      riskFreeTS,
                                                      blackVolTS);

    option.setPricingEngine(
           ext::make_shared<AnalyticHolderExtensibleOptionEngine>(process));

    Real calculated = option.NPV();
    Real expected = 9.4233;
    Real error = std::fabs(calculated-expected);
    Real tolerance = 1e-4;
    if (error > tolerance)
        BOOST_ERROR("Failed to reproduce holder-extensible option value"
                    << "\n    expected:   " << expected
                    << "\n    calculated: " << calculated
                    << "\n    error:      " << error);
}

BOOST_AUTO_TEST_CASE(testAnalyticWriterExtensibleOptionEngine) {
    BOOST_TEST_MESSAGE("Testing analytic engine for writer-extensible option...");

    // What we need for the option (tests):
    Option::Type type = Option::Call;
    Real strike1 = 90.0;
    Real strike2 = 82.0;
    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();
    Date exDate1 = today + 180;
    Date exDate2 = today + 270;

    ext::shared_ptr<SimpleQuote> spot = ext::make_shared<SimpleQuote>(80.0);
    ext::shared_ptr<SimpleQuote> qRate = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<YieldTermStructure> dividendTS =
        flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate = ext::make_shared<SimpleQuote>(0.10);
    ext::shared_ptr<YieldTermStructure> riskFreeTS =
        flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol = ext::make_shared<SimpleQuote>(0.30);
    ext::shared_ptr<BlackVolTermStructure> blackVolTS =
        flatVol(today, vol, dc);

    // B&S process (needed for the engine):
    const ext::shared_ptr<GeneralizedBlackScholesProcess> process =
        ext::make_shared<GeneralizedBlackScholesProcess>(
                    Handle<Quote>(spot),
                    Handle<YieldTermStructure>(dividendTS),
                    Handle<YieldTermStructure>(riskFreeTS),
                    Handle<BlackVolTermStructure>(blackVolTS));

    // The engine:
    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<AnalyticWriterExtensibleOptionEngine>(process);

    // Create the arguments:
    ext::shared_ptr<PlainVanillaPayoff> payoff1 =
        ext::make_shared<PlainVanillaPayoff>(type, strike1);
    ext::shared_ptr<Exercise> exercise1 =
        ext::make_shared<EuropeanExercise>(exDate1);
    ext::shared_ptr<PlainVanillaPayoff> payoff2 =
        ext::make_shared<PlainVanillaPayoff>(type, strike2);
    ext::shared_ptr<Exercise> exercise2 =
        ext::make_shared<EuropeanExercise>(exDate2);

    // Create the option by calling the constructor:
    WriterExtensibleOption option(payoff1, exercise1,
                                  payoff2, exercise2);

    //Set the engine of our option:
    option.setPricingEngine(engine);

    //Compare the calculated NPV value to the theoretical value:
    Real calculated = option.NPV();
    Real expected = 6.8238;
    Real error = std::fabs(calculated-expected);
    Real tolerance = 1e-4;
    if (error > tolerance)
        BOOST_ERROR("Failed to reproduce writer-extensible option value"
                    << "\n    expected:   " << expected
                    << "\n    calculated: " << calculated
                    << "\n    error:      " << error);
}

BOOST_AUTO_TEST_CASE(testHolderExtensibleOptionCriticalPrices) {
    BOOST_TEST_MESSAGE(
        "Testing holder-extensible options whose extension region is empty or unbounded...");

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();
    Date exDate1 = today + 180;
    Date exDate2 = today + 270;

    ext::shared_ptr<SimpleQuote> spot = ext::make_shared<SimpleQuote>(100.0);
    ext::shared_ptr<SimpleQuote> qRate = ext::make_shared<SimpleQuote>(0.0);
    Handle<YieldTermStructure> dividendTS(flatRate(today, qRate, dc));
    Handle<YieldTermStructure> riskFreeTS(flatRate(today, 0.08, dc));
    Handle<BlackVolTermStructure> blackVolTS(flatVol(today, 0.25, dc));

    const ext::shared_ptr<BlackScholesMertonProcess> process =
        ext::make_shared<BlackScholesMertonProcess>(Handle<Quote>(spot), dividendTS,
                                                    riskFreeTS, blackVolTS);
    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<AnalyticHolderExtensibleOptionEngine>(process);

    struct Case {
        Option::Type type;
        Real spot, strike1, strike2, premium, dividendYield;
        Real expected;
    };

    // expected values from numerical integration at the first expiry
    Case cases[] = {
        {Option::Put, 100.0, 100.0, 95.0, 1.0, 0.0, 5.20095543317},
        {Option::Put, 100.0, 100.0, 105.0, 1.0, 0.0, 7.2042184187},
        {Option::Put, 80.0, 100.0, 95.0, 1.0, 0.0, 17.3209767623},
        {Option::Call, 100.0, 100.0, 120.0, 1.0, 0.0, 9.0411753344},
        {Option::Call, 130.0, 100.0, 105.0, 1.0, 0.0, 34.3706760702},
        {Option::Call, 100.0, 100.0, 100.0, 1.0, 0.03, 9.3728040611},
        {Option::Call, 120.0, 100.0, 100.0, 1.0, 0.03, 24.0816774969},
    };

    Real tolerance = 1e-4;
    for (const auto& c : cases) {
        spot->setValue(c.spot);
        qRate->setValue(c.dividendYield);
        HolderExtensibleOption option(c.type, c.premium, exDate2, c.strike2,
                                      ext::make_shared<PlainVanillaPayoff>(c.type, c.strike1),
                                      ext::make_shared<EuropeanExercise>(exDate1));
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - c.expected);
        if (error > tolerance)
            BOOST_ERROR("Failed to reproduce holder-extensible option value"
                        << "\n    type:       " << c.type
                        << "\n    spot:       " << c.spot
                        << "\n    strikes:    " << c.strike1 << ", " << c.strike2
                        << "\n    premium:    " << c.premium
                        << "\n    dividend:   " << c.dividendYield
                        << "\n    expected:   " << c.expected
                        << "\n    calculated: " << calculated
                        << "\n    error:      " << error);
    }
}

BOOST_AUTO_TEST_CASE(testHolderExtensibleOptionNegativeDividendYield) {
    BOOST_TEST_MESSAGE("Testing holder-extensible options with a negative dividend yield...");

    DayCounter dc = Actual365Fixed();
    Date today = Settings::instance().evaluationDate();

    auto spot = ext::make_shared<SimpleQuote>(0.0);
    auto qRate = ext::make_shared<SimpleQuote>(0.0);
    auto rRate = ext::make_shared<SimpleQuote>(0.0);
    auto vol = ext::make_shared<SimpleQuote>(0.0);

    auto process = ext::make_shared<BlackScholesMertonProcess>(
        Handle<Quote>(spot), Handle<YieldTermStructure>(flatRate(today, qRate, dc)),
        Handle<YieldTermStructure>(flatRate(today, rRate, dc)),
        Handle<BlackVolTermStructure>(flatVol(today, vol, dc)));
    auto engine = ext::make_shared<AnalyticHolderExtensibleOptionEngine>(process);

    struct Case {
        const char* description;
        Option::Type type;
        Real spot, strike1, strike2, premium, volatility, riskFreeRate, dividendYield;
        Integer days1, days2;
        Real expected;
        const char* expectedError = nullptr;
    };

    /* Reference prices: discounted expectation at the first expiry of
       max(exercise, extension - premium, 0), by numerical integration in
       mpmath. With a negative yield, the extension value of a call minus its
       exercise value is convex in the spot but does not decrease for large
       spots. The cases below include no zero and two zeros. */
    Case cases[] = {
        // exercise never beats extension: no zero
        {"no zero", Option::Call, 118.0, 119.08, 117.523, 2.30157, 0.0864588, 0.00138423,
         -0.00971198, 185, 424, 3.513400085302},
        {"no zero", Option::Call, 118.0, 117.852, 118.726, 1.59195, 0.179851, 0.00232909,
         -0.0246219, 545, 870, 15.845257803666},
        // two zeros, the first strike beyond the minimum: extension dominates exercise
        {"two zeros, strike beyond the minimum", Option::Call, 100.0, 100.0, 100.0, 36.0, 0.2, 0.02,
         -0.3, 182, 547, 25.363356673445},
        // two zeros before and after an exercise region: an additional boundary is required
        {"two zeros, strike before the minimum", Option::Call, 118.0, 119.08, 117.523, 2.30157,
         0.0864588, 0.00138423, -0.005, 185, 424, 3.1862655186463,
         "holder-extensible call with two roots is not supported"},
        // the premium is below X1 - X2*exp(-r*(T2-t1)), so there is no zero
        {"low premium", Option::Call, 100.0, 100.0, 95.0, 1.0, 0.25, 0.08, -0.02, 180, 270,
         14.590398720627},
        {"low premium, zero yield", Option::Call, 100.0, 100.0, 95.0, 1.0, 0.25, 0.08, 0.0, 180,
         270, 13.49148769505},
        {"put", Option::Put, 100.0, 100.0, 105.0, 1.0, 0.25, 0.08, -0.01, 180, 270,
         6.8762827333313},
        {"extension only at high spots", Option::Call, 100.0, 100.0, 100.0, 40.0,
         0.2, 0.02, -0.3, 182, 547, 0.0,
         "holder-extensible call extension only at high spots is not supported"},
        // Saturated deltas must not produce infinite Newton steps.
        {"zero call derivative", Option::Call, 100.0, 100.0, 100.0, 1.0, 100.0, 0.0, 0.0,
         365, 730, 0.0, "critical price: zero or non-finite derivative"},
        {"zero put derivative", Option::Put, 100.0, 100.0, 100.0, 1.0, 100.0, 0.0, 0.0,
         365, 730, 0.0, "critical price: zero or non-finite derivative"},
    };

    Real tolerance = 1e-4;
    for (const auto& c : cases) {
        spot->setValue(c.spot);
        qRate->setValue(c.dividendYield);
        rRate->setValue(c.riskFreeRate);
        vol->setValue(c.volatility);

        HolderExtensibleOption option(c.type, c.premium, today + c.days2, c.strike2,
                                      ext::make_shared<PlainVanillaPayoff>(c.type, c.strike1),
                                      ext::make_shared<EuropeanExercise>(today + c.days1));
        option.setPricingEngine(engine);

        BOOST_TEST_CONTEXT(c.description << ", dividend yield " << c.dividendYield) {
            if (c.expectedError)
                BOOST_CHECK_EXCEPTION(option.NPV(), Error,
                                      ExpectedErrorMessage(c.expectedError));
            else
                QL_CHECK_SMALL(option.NPV() - c.expected, tolerance);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
