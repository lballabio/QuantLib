/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/exoticoptions/partialtimebarrieroption.hpp>
#include <ql/experimental/exoticoptions/analyticpartialtimebarrieroptionengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(PartialTimeBarrierOptionTests)

struct TestCase {
    Real underlying;
    Real strike;
    Integer days;
    Real result;
};


BOOST_AUTO_TEST_CASE(testAnalyticEngine) {
    BOOST_TEST_MESSAGE(
        "Testing analytic engine for partial-time barrier option...");

    Date today = Settings::instance().evaluationDate();

    Option::Type type = Option::Call;
    DayCounter dc = Actual360();
    Date maturity = today + 360;
    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturity);
    Real barrier = 100.0;
    Real rebate = 0.0;

    auto spot = ext::make_shared<SimpleQuote>();
    auto qRate = ext::make_shared<SimpleQuote>(0.0);
    auto rRate = ext::make_shared<SimpleQuote>(0.1);
    auto vol = ext::make_shared<SimpleQuote>(0.25);

    Handle<Quote> underlying(spot);
    Handle<YieldTermStructure> dividendTS(flatRate(today, qRate, dc));
    Handle<YieldTermStructure> riskFreeTS(flatRate(today, rRate, dc));
    Handle<BlackVolTermStructure> blackVolTS(flatVol(today, vol, dc));

    const auto process =
        ext::make_shared<BlackScholesMertonProcess>(underlying,
                                                      dividendTS,
                                                      riskFreeTS,
                                                      blackVolTS);
    auto engine =
        ext::make_shared<AnalyticPartialTimeBarrierOptionEngine<Option::Call>>(process);

    TestCase cases[] = {
        {  95.0,  90.0,   1,  0.0393 },
        {  95.0, 110.0,   1,  0.0000 },
        { 105.0,  90.0,   1,  9.8751 },
        { 105.0, 110.0,   1,  6.2303 },

        {  95.0,  90.0,  90,  6.2747 },
        {  95.0, 110.0,  90,  3.7352 },
        { 105.0,  90.0,  90, 15.6324 },
        { 105.0, 110.0,  90,  9.6812 },

        {  95.0,  90.0, 180, 10.3345 },
        {  95.0, 110.0, 180,  5.8712 },
        { 105.0,  90.0, 180, 19.2896 },
        { 105.0, 110.0, 180, 11.6055 },

        {  95.0,  90.0, 270, 13.4342 },
        {  95.0, 110.0, 270,  7.1270 },
        { 105.0,  90.0, 270, 22.0753 },
        { 105.0, 110.0, 270, 12.7342 },

        {  95.0,  90.0, 359, 16.8576 },
        {  95.0, 110.0, 359,  7.5763 },
        { 105.0,  90.0, 359, 25.1488 },
        { 105.0, 110.0, 359, 13.1376 }
    };

    for (auto& i : cases) {
        Date coverEventDate = today + i.days;
        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::make_shared<PlainVanillaPayoff>(type, i.strike);
        PartialTimeBarrierOption option(PartialBarrier::DownOut,
                                        PartialBarrier::EndB1,
                                        barrier, rebate,
                                        coverEventDate,
                                        payoff, exercise);
        option.setPricingEngine(engine);

        spot->setValue(i.underlying);
        Real calculated = option.NPV();
        Real expected = i.result;
        Real error = std::fabs(calculated-expected);
        Real tolerance = 1e-4;
        if (error > tolerance)
            BOOST_ERROR("Failed to reproduce partial-time barrier option value"
                        << "\n    expected:   " << expected
                        << "\n    calculated: " << calculated
                        << "\n    error:      " << error);
    }
}

BOOST_AUTO_TEST_CASE(testAnalyticEnginePutOption) {
    BOOST_TEST_MESSAGE(
        "Testing analytic engine for partial-time put barrier option...");

    Date today = Settings::instance().evaluationDate();

    Option::Type type = Option::Put;
    DayCounter dc = Actual360();
    Date maturity = today + 360;
    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturity);
    Real barrier = 100.0;
    Real rebate = 0.0;

    auto spot = ext::make_shared<SimpleQuote>();
    auto qRate = ext::make_shared<SimpleQuote>(0.0);
    auto rRate = ext::make_shared<SimpleQuote>(0.1);
    auto vol = ext::make_shared<SimpleQuote>(0.25);

    Handle<Quote> underlying(spot);
    Handle<YieldTermStructure> dividendTS(flatRate(today, qRate, dc));
    Handle<YieldTermStructure> riskFreeTS(flatRate(today, rRate, dc));
    Handle<BlackVolTermStructure> blackVolTS(flatVol(today, vol, dc));

    const auto process =
        ext::make_shared<BlackScholesMertonProcess>(underlying,
                                                      dividendTS,
                                                      riskFreeTS,
                                                      blackVolTS);
    auto engine =
        ext::make_shared<AnalyticPartialTimeBarrierOptionEngine<Option::Put>>(process);

    TestCase cases[] = {
        {  95.0,  90.0,   1,  1.5551 },
        {  95.0,  95.0,   1,  2.0589 },
        {  90.0,  95.0,   1,  4.4512 },
        {  99.0,  90.0,   1,  0.3404 },

        {  95.0,  90.0,   90,  2.4181 },
        {  95.0,  95.0,   90,  3.2257 },
        {  90.0,  95.0,   90,  5.0624 },
        {  99.0,  90.0,   90,  1.5992 },

        {  95.0,  90.0,   180,  3.0021 },
        {  95.0,  95.0,   180,  4.0617 },
        {  90.0,  95.0,   180,  5.7960 },
        {  99.0,  90.0,   180,  2.1903 },

        {  95.0,  90.0,   270,  3.4194 },
        {  95.0,  95.0,   270,  4.7362 },
        {  90.0,  95.0,   270,  6.4370 },
        {  99.0,  90.0,   270,  2.6025 },

        {  95.0,  90.0,   359,  3.5965 },
        {  95.0,  95.0,   359,  5.1865 },
        {  90.0,  95.0,   359,  6.8782 },
        {  99.0,  90.0,   359,  2.7759 }
    };

    for (auto& i : cases) {
        Date coverEventDate = today + i.days;
        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::make_shared<PlainVanillaPayoff>(type, i.strike);
        PartialTimeBarrierOption option(PartialBarrier::UpOut,
                                        PartialBarrier::EndB1,
                                        barrier, rebate,
                                        coverEventDate,
                                        payoff, exercise);
        option.setPricingEngine(engine);

        spot->setValue(i.underlying);
        Real calculated = option.NPV();
        Real expected = i.result;
        Real error = std::fabs(calculated-expected);
        Real tolerance = 1e-4;
        if (error > tolerance)
            BOOST_ERROR("Failed to reproduce partial-time put barrier option value"
                        << "\n    expected:   " << expected
                        << "\n    calculated: " << calculated
                        << "\n    error:      " << error);
    }
}

BOOST_AUTO_TEST_CASE(testPutCallSymmetry) {
    BOOST_TEST_MESSAGE(
        "Testing put-call symmetry for the partial-time barrier option...");

    Date today = Settings::instance().evaluationDate();

    struct PutCallSymmetryTestCase {
        Real callStrike;
        Real callBarrier;
        Barrier::Type callType;
        Real putStrike;
        Real putBarrier;
        Integer days;
        Barrier::Type putType;
    };

    PutCallSymmetryTestCase cases[] = {
        { 105.2631, 95.2380, PartialBarrier::DownOut, 95.0, 105.0, 1, PartialBarrier::UpOut },
        { 105.2631, 95.2380, PartialBarrier::DownOut, 95.0, 105.0, 90, PartialBarrier::UpOut },
        { 105.2631, 95.2380, PartialBarrier::DownOut, 95.0, 105.0, 180, PartialBarrier::UpOut },
        { 105.2631, 95.2380, PartialBarrier::DownOut, 95.0, 105.0, 270, PartialBarrier::UpOut },
        { 105.2631, 95.2380, PartialBarrier::DownOut, 95.0, 105.0, 359, PartialBarrier::UpOut },

        { 110.0, 120.0, PartialBarrier::UpOut, 90.9090, 83.3333, 1, PartialBarrier::DownOut },
        { 110.0, 120.0, PartialBarrier::UpOut, 90.9090, 83.3333, 90, PartialBarrier::DownOut },
        { 110.0, 120.0, PartialBarrier::UpOut, 90.9090, 83.3333, 180, PartialBarrier::DownOut },
        { 110.0, 120.0, PartialBarrier::UpOut, 90.9090, 83.3333, 270, PartialBarrier::DownOut },
        { 110.0, 120.0, PartialBarrier::UpOut, 90.9090, 83.3333, 359, PartialBarrier::DownOut },
    };

    DayCounter dc = Actual360();
    Date maturity = today + 360;
    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturity);
    Real r = 0.01;
    Real rebate = 0.0;
    Real spotPrice = 100.0;

    auto spot = ext::make_shared<SimpleQuote>();
    auto qRate = ext::make_shared<SimpleQuote>(0.0);
    auto rRate = ext::make_shared<SimpleQuote>(r);
    auto vol = ext::make_shared<SimpleQuote>(0.25);

    Handle<Quote> underlying(spot);
    Handle<YieldTermStructure> dividendTSCall(flatRate(today, qRate, dc));
    Handle<YieldTermStructure> riskFreeTSCall(flatRate(today, rRate, dc));
    Handle<YieldTermStructure> dividendTSPut(flatRate(today, rRate, dc));
    Handle<YieldTermStructure> riskFreeTSPut(flatRate(today, qRate, dc));
    Handle<BlackVolTermStructure> blackVolTS(flatVol(today, vol, dc));

    const auto callProcess =
        ext::make_shared<BlackScholesMertonProcess>(underlying,
                                                      dividendTSCall,
                                                      riskFreeTSCall,
                                                      blackVolTS);
    const auto putProcess =
        ext::make_shared<BlackScholesMertonProcess>(underlying,
                                                      dividendTSPut,
                                                      riskFreeTSPut,
                                                      blackVolTS);
    auto callEngine =
        ext::make_shared<AnalyticPartialTimeBarrierOptionEngine<Option::Call>>(callProcess);
    auto putEngine =
        ext::make_shared<AnalyticPartialTimeBarrierOptionEngine<Option::Put>>(putProcess);

    for (auto& i : cases) {
        Date coverEventDate = today + i.days;
        ext::shared_ptr<StrikedTypePayoff> putPayoff =
            ext::make_shared<PlainVanillaPayoff>(Option::Put, i.putStrike);
        ext::shared_ptr<StrikedTypePayoff> callPayoff =
            ext::make_shared<PlainVanillaPayoff>(Option::Call, i.callStrike);
        PartialTimeBarrierOption putOption(i.putType,
                                        PartialBarrier::EndB1,
                                        i.putBarrier, rebate,
                                        coverEventDate,
                                        putPayoff, exercise);
        putOption.setPricingEngine(putEngine);
        PartialTimeBarrierOption callOption(i.callType,
                                        PartialBarrier::EndB1,
                                        i.callBarrier, rebate,
                                        coverEventDate,
                                        callPayoff, exercise);
        callOption.setPricingEngine(callEngine);

        spot->setValue(spotPrice);
        Real putValue = putOption.NPV();
        Real callValue = callOption.NPV();
        Real callAmount = (i.putStrike / spotPrice);
        Real error = std::fabs(putValue - callAmount * callValue);
        Real tolerance = 1e-4;
        if (error > tolerance)
            BOOST_ERROR("Failed to reproduce the put-call symmetry for the partial-time barrier options "
                        << "\n    error:      " << error);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
