/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2007 StatPro Italia srl

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

#include "digitaloption.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analyticdigitalamericanengine.hpp>
#include <ql/pricingengines/vanilla/mcdigitalengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef REPORT_FAILURE
#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, expected, calculated, error, tolerance, knockin) \
    BOOST_FAIL(exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    spot value:       " << s << "\n" \
               << "    strike:           " << payoff->strike() << "\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance << "\n" \
               << "    knock_in:         " << knockin);

namespace {

    struct DigitalOptionData {
        Option::Type type;
        Real strike;
        Real s;        // spot
        Rate q;        // dividend
        Rate r;        // risk-free rate
        Time t;        // time to maturity
        Volatility v;  // volatility
        Real result;   // expected result
        Real tol;      // tolerance
        bool knockin;  // true if knock-in
    };

}


void DigitalOptionTest::testCashOrNothingEuropeanValues() {

    BOOST_TEST_MESSAGE("Testing European cash-or-nothing digital option...");

    DigitalOptionData values[] = {
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 88
        //        type, strike,  spot,    q,    r,    t,  vol,  value, tol
        { Option::Put,   80.00, 100.0, 0.06, 0.06, 0.75, 0.35, 2.6710, 1e-4, true }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    std::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {

        std::shared_ptr<StrikedTypePayoff> payoff(
            new CashOrNothingPayoff(value.type, value.strike, 10.0));

        Date exDate = today + timeToDays(value.t);
        std::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        std::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));
        std::shared_ptr<PricingEngine> engine(
                                    new AnalyticEuropeanEngine(stochProcess));

        VanillaOption opt(payoff, exercise);
        opt.setPricingEngine(engine);

        Real calculated = opt.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > value.tol) {
            REPORT_FAILURE("value", payoff, exercise, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, value.tol, value.knockin);
        }
    }
}

void DigitalOptionTest::testAssetOrNothingEuropeanValues() {

    BOOST_TEST_MESSAGE("Testing European asset-or-nothing digital option...");

    // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 90
    DigitalOptionData values[] = {
        //        type, strike, spot,    q,    r,    t,  vol,   value, tol
        { Option::Put,   65.00, 70.0, 0.05, 0.07, 0.50, 0.27, 20.2069, 1e-4, true }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    std::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {

        std::shared_ptr<StrikedTypePayoff> payoff(
            new AssetOrNothingPayoff(value.type, value.strike));

        Date exDate = today + timeToDays(value.t);
        std::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        std::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));
        std::shared_ptr<PricingEngine> engine(
                                    new AnalyticEuropeanEngine(stochProcess));

        VanillaOption opt(payoff, exercise);
        opt.setPricingEngine(engine);

        Real calculated = opt.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > value.tol) {
            REPORT_FAILURE("value", payoff, exercise, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, value.tol, value.knockin);
        }
    }
}

void DigitalOptionTest::testGapEuropeanValues() {

    BOOST_TEST_MESSAGE("Testing European gap digital option...");

    // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 88
    DigitalOptionData values[] = {
        //        type, strike, spot,    q,    r,    t,  vol,   value, tol
        { Option::Call,  50.00, 50.0, 0.00, 0.09, 0.50, 0.20, -0.0053, 1e-4, true }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    std::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {

        std::shared_ptr<StrikedTypePayoff> payoff(new GapPayoff(value.type, value.strike, 57.00));

        Date exDate = today + timeToDays(value.t);
        std::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        std::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));
        std::shared_ptr<PricingEngine> engine(
                                    new AnalyticEuropeanEngine(stochProcess));

        VanillaOption opt(payoff, exercise);
        opt.setPricingEngine(engine);

        Real calculated = opt.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > value.tol) {
            REPORT_FAILURE("value", payoff, exercise, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, value.tol, value.knockin);
        }
    }
}

void DigitalOptionTest::testCashAtHitOrNothingAmericanValues() {

    BOOST_TEST_MESSAGE("Testing American cash-(at-hit)-or-nothing "
                       "digital option...");

    DigitalOptionData values[] = {
        //        type, strike,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 1,2
        { Option::Put,  100.00, 105.00, 0.00, 0.10, 0.5, 0.20,  9.7264, 1e-4,  true},
        { Option::Call, 100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 11.6553, 1e-4,  true},

        // the following cases are not taken from a reference paper or book
        // in the money options (guaranteed immediate payoff)
        { Option::Call, 100.00, 105.00, 0.00, 0.10, 0.5, 0.20, 15.0000, 1e-16, true},
        { Option::Put,  100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 15.0000, 1e-16, true},
        // non null dividend (cross-tested with MC simulation)
        { Option::Put,  100.00, 105.00, 0.20, 0.10, 0.5, 0.20, 12.2715, 1e-4,  true},
        { Option::Call, 100.00,  95.00, 0.20, 0.10, 0.5, 0.20,  8.9109, 1e-4,  true},
        { Option::Call, 100.00, 105.00, 0.20, 0.10, 0.5, 0.20, 15.0000, 1e-16, true},
        { Option::Put,  100.00,  95.00, 0.20, 0.10, 0.5, 0.20, 15.0000, 1e-16, true}
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    std::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {

        std::shared_ptr<StrikedTypePayoff> payoff(
            new CashOrNothingPayoff(value.type, value.strike, 15.00));

        Date exDate = today + timeToDays(value.t);
        std::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                    exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        std::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));
        std::shared_ptr<PricingEngine> engine(
                             new AnalyticDigitalAmericanEngine(stochProcess));

        VanillaOption opt(payoff, amExercise);
        opt.setPricingEngine(engine);

        Real calculated = opt.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > value.tol) {
            REPORT_FAILURE("value", payoff, amExercise, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, value.tol, value.knockin);
        }
    }
}

void DigitalOptionTest::testAssetAtHitOrNothingAmericanValues() {

    BOOST_TEST_MESSAGE("Testing American asset-(at-hit)-or-nothing "
                       "digital option...");

    DigitalOptionData values[] = {
        //        type, strike,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 3,4
        { Option::Put,  100.00, 105.00, 0.00, 0.10, 0.5, 0.20, 64.8426, 1e-04, true }, // Haug value is wrong here, Haug VBA code is right
        { Option::Call, 100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 77.7017, 1e-04, true }, // Haug value is wrong here, Haug VBA code is right
        // data from Haug VBA code results
        { Option::Put,  100.00, 105.00, 0.01, 0.10, 0.5, 0.20, 65.7811, 1e-04, true },
        { Option::Call, 100.00,  95.00, 0.01, 0.10, 0.5, 0.20, 76.8858, 1e-04, true },
        // in the money options  (guaranteed immediate payoff = spot)
        { Option::Call, 100.00, 105.00, 0.00, 0.10, 0.5, 0.20,105.0000, 1e-16, true },
        { Option::Put,  100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 95.0000, 1e-16, true },
        { Option::Call, 100.00, 105.00, 0.01, 0.10, 0.5, 0.20,105.0000, 1e-16, true },
        { Option::Put,  100.00,  95.00, 0.01, 0.10, 0.5, 0.20, 95.0000, 1e-16, true }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    std::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.01));
    std::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    std::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {

        std::shared_ptr<StrikedTypePayoff> payoff(
            new AssetOrNothingPayoff(value.type, value.strike));

        Date exDate = today + timeToDays(value.t);
        std::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                    exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        std::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));
        std::shared_ptr<PricingEngine> engine(
                             new AnalyticDigitalAmericanEngine(stochProcess));

        VanillaOption opt(payoff, amExercise);
        opt.setPricingEngine(engine);

        Real calculated = opt.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > value.tol) {
            REPORT_FAILURE("value", payoff, amExercise, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, value.tol, value.knockin);
        }
    }
}

void DigitalOptionTest::testCashAtExpiryOrNothingAmericanValues() {

    BOOST_TEST_MESSAGE("Testing American cash-(at-expiry)-or-nothing "
                       "digital option...");

    DigitalOptionData values[] = {
        //        type, strike,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 5,6,9,10
        { Option::Put,  100.00, 105.00, 0.00, 0.10, 0.5, 0.20,  9.3604, 1e-4, true },
        { Option::Call, 100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 11.2223, 1e-4, true },
        { Option::Put,  100.00, 105.00, 0.00, 0.10, 0.5, 0.20,  4.9081, 1e-4, false },
        { Option::Call, 100.00,  95.00, 0.00, 0.10, 0.5, 0.20,  3.0461, 1e-4, false },
        // in the money options (guaranteed discounted payoff)
        { Option::Call, 100.00, 105.00, 0.00, 0.10, 0.5, 0.20, 15.0000*std::exp(-0.05), 1e-12, true },
        { Option::Put,  100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 15.0000*std::exp(-0.05), 1e-12, true },
        // out of bonds case
        { Option::Call,   2.37,   2.33, 0.07, 0.43,0.19,0.005,  0.0000, 1e-4, false },
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    std::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.01));
    std::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    std::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {

        std::shared_ptr<StrikedTypePayoff> payoff(
            new CashOrNothingPayoff(value.type, value.strike, 15.0));

        Date exDate = today + timeToDays(value.t);
        std::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                    exDate,
                                                                    true));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        std::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));
        std::shared_ptr<PricingEngine> engine;
        if (value.knockin)
            engine.reset(new AnalyticDigitalAmericanEngine(stochProcess));
        else
           engine.reset(new AnalyticDigitalAmericanKOEngine(stochProcess));

        VanillaOption opt(payoff, amExercise);
        opt.setPricingEngine(engine);

        Real calculated = opt.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > value.tol) {
            REPORT_FAILURE("value", payoff, amExercise, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, value.tol, value.knockin);
        }
    }
}

void DigitalOptionTest::testAssetAtExpiryOrNothingAmericanValues() {

    BOOST_TEST_MESSAGE("Testing American asset-(at-expiry)-or-nothing "
                       "digital option...");

    DigitalOptionData values[] = {
        //        type, strike,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 7,8,11,12
        { Option::Put,  100.00, 105.00, 0.00, 0.10, 0.5, 0.20, 64.8426, 1e-04, true },
        { Option::Call, 100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 77.7017, 1e-04, true },
        { Option::Put,  100.00, 105.00, 0.00, 0.10, 0.5, 0.20, 40.1574, 1e-04, false },
        { Option::Call, 100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 17.2983, 1e-04, false },
        // data from Haug VBA code results
        { Option::Put,  100.00, 105.00, 0.01, 0.10, 0.5, 0.20, 65.5291, 1e-04, true },
        { Option::Call, 100.00,  95.00, 0.01, 0.10, 0.5, 0.20, 76.5951, 1e-04, true },
        // in the money options (guaranteed discounted payoff = forward * riskFreeDiscount
        //                                                    = spot * dividendDiscount)
        { Option::Call, 100.00, 105.00, 0.00, 0.10, 0.5, 0.20,105.0000, 1e-12, true },
        { Option::Put,  100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 95.0000, 1e-12, true },
        { Option::Call, 100.00, 105.00, 0.01, 0.10, 0.5, 0.20,105.0000*std::exp(-0.005), 1e-12, true },
        { Option::Put,  100.00,  95.00, 0.01, 0.10, 0.5, 0.20, 95.0000*std::exp(-0.005), 1e-12, true }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    std::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.01));
    std::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    std::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {

        std::shared_ptr<StrikedTypePayoff> payoff(
            new AssetOrNothingPayoff(value.type, value.strike));

        Date exDate = today + timeToDays(value.t);
        std::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                    exDate,
                                                                    true));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        std::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));
        std::shared_ptr<PricingEngine> engine;
        if (value.knockin)
            engine.reset(new AnalyticDigitalAmericanEngine(stochProcess));
        else
           engine.reset(new AnalyticDigitalAmericanKOEngine(stochProcess));

        VanillaOption opt(payoff, amExercise);
        opt.setPricingEngine(engine);

        Real calculated = opt.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > value.tol) {
            REPORT_FAILURE("value", payoff, amExercise, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, value.tol, value.knockin);
        }
    }
}

void DigitalOptionTest::testCashAtHitOrNothingAmericanGreeks() {

    BOOST_TEST_MESSAGE("Testing American cash-(at-hit)-or-nothing "
                       "digital option greeks...");

    SavedSettings backup;

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]  = 5.0e-5;
    tolerance["gamma"]  = 5.0e-5;
    // tolerance["theta"]  = 5.0e-5;
    tolerance["rho"]    = 5.0e-5;
    // tolerance["divRho"] = 5.0e-5;
    // tolerance["vega"]   = 5.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.5, 150.0 };
    Real cashPayoff = 100.0;
    Real underlyings[] = { 100 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Volatility vols[] = { 0.11, 0.5, 1.2 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    // there is no cycling on different residual times
    Date exDate = today + 360;
    std::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
    std::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                exDate,
                                                                false));
    std::shared_ptr<Exercise> exercises[] = { exercise, amExercise };

    std::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

    std::shared_ptr<PricingEngine> euroEngine(
                                    new AnalyticEuropeanEngine(stochProcess));

    std::shared_ptr<PricingEngine> amEngine(
                             new AnalyticDigitalAmericanEngine(stochProcess));

    std::shared_ptr<PricingEngine> engines[] = { euroEngine, amEngine };

    bool knockin=true;
    for (Size j=0; j<LENGTH(engines); j++) {
        for (auto& type : types) {
            for (Real strike : strikes) {
                std::shared_ptr<StrikedTypePayoff> payoff(
                    new CashOrNothingPayoff(type, strike, cashPayoff));

                VanillaOption opt(payoff, exercises[j]);
                opt.setPricingEngine(engines[j]);

                for (Real u : underlyings) {
                    for (Real q : qRates) {
                        for (Real r : rRates) {
                            for (Real v : vols) {
                                // test data
                                spot->setValue(u);
                                qRate->setValue(q);
                                rRate->setValue(r);
                                vol->setValue(v);

                                // theta, dividend rho and vega are not available for
                                // digital option with american exercise. Greeks of
                                // digital options with european payoff are tested
                                // in the europeanoption.cpp test
                                Real value = opt.NPV();
                                calculated["delta"] = opt.delta();
                                calculated["gamma"] = opt.gamma();
                                // calculated["theta"]  = opt.theta();
                                calculated["rho"] = opt.rho();
                                // calculated["divRho"] = opt.dividendRho();
                                // calculated["vega"]   = opt.vega();

                                if (value > 1.0e-6) {
                                    // perturb spot and get delta and gamma
                                    Real du = u * 1.0e-4;
                                    spot->setValue(u + du);
                                    Real value_p = opt.NPV(), delta_p = opt.delta();
                                    spot->setValue(u - du);
                                    Real value_m = opt.NPV(), delta_m = opt.delta();
                                    spot->setValue(u);
                                    expected["delta"] = (value_p - value_m) / (2 * du);
                                    expected["gamma"] = (delta_p - delta_m) / (2 * du);

                                    // perturb rates and get rho and dividend rho
                                    Spread dr = r * 1.0e-4;
                                    rRate->setValue(r + dr);
                                    value_p = opt.NPV();
                                    rRate->setValue(r - dr);
                                    value_m = opt.NPV();
                                    rRate->setValue(r);
                                    expected["rho"] = (value_p - value_m) / (2 * dr);

                                    /*
                                    Spread dq = q*1.0e-4;
                                    qRate->setValue(q+dq);
                                    value_p = opt.NPV();
                                    qRate->setValue(q-dq);
                                    value_m = opt.NPV();
                                    qRate->setValue(q);
                                    expected["divRho"] = (value_p - value_m)/(2*dq);

                                    // perturb volatility and get vega
                                    Volatility dv = v*1.0e-4;
                                    vol->setValue(v+dv);
                                    value_p = opt.NPV();
                                    vol->setValue(v-dv);
                                    value_m = opt.NPV();
                                    vol->setValue(v);
                                    expected["vega"] = (value_p - value_m)/(2*dv);

                                    // perturb date and get theta
                                    Time dT = dc.yearFraction(today-1, today+1);
                                    Settings::instance().setEvaluationDate(today-1);
                                    value_m = opt.NPV();
                                    Settings::instance().setEvaluationDate(today+1);
                                    value_p = opt.NPV();
                                    Settings::instance().setEvaluationDate(today);
                                    expected["theta"] = (value_p - value_m)/dT;
                                    */

                                    // check
                                    std::map<std::string, Real>::iterator it;
                                    for (it = calculated.begin(); it != calculated.end(); ++it) {
                                        std::string greek = it->first;
                                        Real expct = expected[greek], calcl = calculated[greek],
                                             tol = tolerance[greek];
                                        Real error = relativeError(expct, calcl, value);
                                        if (error > tol) {
                                            REPORT_FAILURE(greek, payoff, exercise, u, q, r, today,
                                                           v, expct, calcl, error, tol, knockin);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


void DigitalOptionTest::testMCCashAtHit() {

    BOOST_TEST_MESSAGE("Testing Monte Carlo cash-(at-hit)-or-nothing "
                       "American engine...");

    SavedSettings backup;

    DigitalOptionData values[] = {
        //        type, strike,   spot,    q,    r,   t,  vol,   value, tol
        { Option::Put,  100.00, 105.00, 0.20, 0.10, 0.5, 0.20, 12.2715, 1e-2, true },
        { Option::Call, 100.00,  95.00, 0.20, 0.10, 0.5, 0.20,  8.9109, 1e-2, true }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    std::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    std::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    std::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    std::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    std::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    Size timeStepsPerYear = 90;
    Size maxSamples = 1000000;
    BigNatural seed = 1;

    for (auto& value : values) {

        std::shared_ptr<StrikedTypePayoff> payoff(
            new CashOrNothingPayoff(value.type, value.strike, 15.0));
        Date exDate = today + timeToDays(value.t);
        std::shared_ptr<Exercise> amExercise(
                                         new AmericanExercise(today, exDate));

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        std::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        Size requiredSamples = Size(std::pow(2.0, 14)-1);
        std::shared_ptr<PricingEngine> mcldEngine =
            MakeMCDigitalEngine<LowDiscrepancy>(stochProcess)
            .withStepsPerYear(timeStepsPerYear)
            .withBrownianBridge()
            .withSamples(requiredSamples)
            .withMaxSamples(maxSamples)
            .withSeed(seed);

        VanillaOption opt(payoff, amExercise);
        opt.setPricingEngine(mcldEngine);

        Real calculated = opt.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > value.tol) {
            REPORT_FAILURE("value", payoff, amExercise, value.s, value.q, value.r, today, value.v,
                           value.result, calculated, error, value.tol, value.knockin);
        }
    }
}


test_suite* DigitalOptionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Digital option tests");
    suite->add(QUANTLIB_TEST_CASE(
               &DigitalOptionTest::testCashOrNothingEuropeanValues));
    suite->add(QUANTLIB_TEST_CASE(
               &DigitalOptionTest::testAssetOrNothingEuropeanValues));
    suite->add(QUANTLIB_TEST_CASE(&DigitalOptionTest::testGapEuropeanValues));
    suite->add(QUANTLIB_TEST_CASE(
               &DigitalOptionTest::testCashAtHitOrNothingAmericanValues));
    suite->add(QUANTLIB_TEST_CASE(
               &DigitalOptionTest::testCashAtHitOrNothingAmericanGreeks));
    suite->add(QUANTLIB_TEST_CASE(
               &DigitalOptionTest::testAssetAtHitOrNothingAmericanValues));
    suite->add(QUANTLIB_TEST_CASE(
               &DigitalOptionTest::testCashAtExpiryOrNothingAmericanValues));
    suite->add(QUANTLIB_TEST_CASE(
               &DigitalOptionTest::testAssetAtExpiryOrNothingAmericanValues));
    suite->add(QUANTLIB_TEST_CASE(&DigitalOptionTest::testMCCashAtHit));
    return suite;
}

