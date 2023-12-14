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

namespace {

    struct TestCase {
        Real underlying;
        Real strike;
        Integer days;
        Real result;
    };

}

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

    ext::shared_ptr<SimpleQuote> spot = ext::make_shared<SimpleQuote>();
    ext::shared_ptr<SimpleQuote> qRate = ext::make_shared<SimpleQuote>(0.0);
    ext::shared_ptr<SimpleQuote> rRate = ext::make_shared<SimpleQuote>(0.1);
    ext::shared_ptr<SimpleQuote> vol = ext::make_shared<SimpleQuote>(0.25);

    Handle<Quote> underlying(spot);
    Handle<YieldTermStructure> dividendTS(flatRate(today, qRate, dc));
    Handle<YieldTermStructure> riskFreeTS(flatRate(today, rRate, dc));
    Handle<BlackVolTermStructure> blackVolTS(flatVol(today, vol, dc));

    const ext::shared_ptr<BlackScholesMertonProcess> process =
        ext::make_shared<BlackScholesMertonProcess>(underlying,
                                                      dividendTS,
                                                      riskFreeTS,
                                                      blackVolTS);
    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<AnalyticPartialTimeBarrierOptionEngine>(process);

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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
