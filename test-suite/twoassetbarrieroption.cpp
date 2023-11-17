/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 StatPro Italia srl

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
#include <ql/experimental/exoticoptions/twoassetbarrieroption.hpp>
#include <ql/experimental/exoticoptions/analytictwoassetbarrierengine.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct OptionData {
        Barrier::Type barrierType;
        Option::Type type;
        Real barrier;
        Real strike;
        Real s1;       // spot
        Rate q1;       // dividend
        Volatility v1; // volatility
        Real s2;
        Rate q2;
        Volatility v2;
        Real correlation;
        Rate r;        // risk-free rate
        Real result;   // result
    };

}

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(TwoAssetBarrierOptionExperimentalTest)

BOOST_AUTO_TEST_CASE(testHaugValues) {

    BOOST_TEST_MESSAGE("Testing two-asset barrier options against Haug's values...");

    OptionData values[] = {
        /* The data below are from
          "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
        */
        { Barrier::DownOut, Option::Call, 95, 90,
          100.0, 0.0, 0.2, 100.0, 0.0, 0.2, 0.5, 0.08, 6.6592 },
        { Barrier::UpOut, Option::Call, 105, 90,
          100.0, 0.0, 0.2, 100.0, 0.0, 0.2, -0.5, 0.08, 4.6670 },
        { Barrier::DownOut, Option::Put, 95, 90,
          100.0, 0.0, 0.2, 100.0, 0.0, 0.2, -0.5, 0.08, 0.6184 },
        { Barrier::UpOut, Option::Put, 105, 100,
          100.0, 0.0, 0.2, 100.0, 0.0, 0.2, 0.0, 0.08, 0.8246 }
    };

    DayCounter dc = Actual360();
    Calendar calendar = TARGET();
    Date today = Date::todaysDate();
    Date maturity = today + 180;
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(maturity));

    ext::shared_ptr<SimpleQuote> r(new SimpleQuote);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, r, dc);

    ext::shared_ptr<SimpleQuote> s1(new SimpleQuote);
    ext::shared_ptr<SimpleQuote> q1(new SimpleQuote);
    ext::shared_ptr<YieldTermStructure> qTS1 = flatRate(today, q1, dc);
    ext::shared_ptr<SimpleQuote> vol1(new SimpleQuote);
    ext::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);

    ext::shared_ptr<BlackScholesMertonProcess> process1(
        new BlackScholesMertonProcess(Handle<Quote>(s1),
                                      Handle<YieldTermStructure>(qTS1),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS1)));

    ext::shared_ptr<SimpleQuote> s2(new SimpleQuote);
    ext::shared_ptr<SimpleQuote> q2(new SimpleQuote);
    ext::shared_ptr<YieldTermStructure> qTS2 = flatRate(today, q2, dc);
    ext::shared_ptr<SimpleQuote> vol2(new SimpleQuote);
    ext::shared_ptr<BlackVolTermStructure> volTS2 = flatVol(today, vol2, dc);

    ext::shared_ptr<BlackScholesMertonProcess> process2(
        new BlackScholesMertonProcess(Handle<Quote>(s2),
                                      Handle<YieldTermStructure>(qTS2),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS2)));

    ext::shared_ptr<SimpleQuote> rho(new SimpleQuote);

    ext::shared_ptr<PricingEngine> engine(
                       new AnalyticTwoAssetBarrierEngine(process1, process2,
                                                         Handle<Quote>(rho)));

    for (auto& value : values) {

        s1->setValue(value.s1);
        q1->setValue(value.q1);
        vol1->setValue(value.v1);

        s2->setValue(value.s2);
        q2->setValue(value.q2);
        vol2->setValue(value.v2);

        rho->setValue(value.correlation);

        r->setValue(value.r);

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, value.strike));

        TwoAssetBarrierOption barrierOption(value.barrierType, value.barrier, payoff, exercise);
        barrierOption.setPricingEngine(engine);

        Real calculated = barrierOption.NPV();
        Real expected = value.result;
        Real error = std::fabs(calculated-expected);
        Real tolerance = 4.0e-3;
        if (error > tolerance) {
            BOOST_ERROR("failed to reproduce expected price"
                        << "\n    expected:   " << expected
                        << "\n    calculated: " << calculated
                        << "\n    tolerance:  " << tolerance
                        << "\n    error:      " << error);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()