
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "digitaloption.hpp"
#include "utilities.hpp"
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/vanillaoption.hpp>
#include <ql/PricingEngines/Vanilla/analyticeuropeanengine.hpp>
#include <ql/PricingEngines/Vanilla/analyticdigitalamericanengine.hpp>
#include <ql/PricingEngines/Vanilla/mcdigitalengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, expected, calculated, error, tolerance) \
    BOOST_FAIL(exerciseTypeToString(exercise) + " " \
               + OptionTypeFormatter::toString(payoff->optionType()) + \
               " option with " \
               + payoffTypeToString(payoff) + " payoff:\n" \
               "    spot value: " \
               + DecimalFormatter::toString(s) + "\n" \
               "    strike:           " \
               + DecimalFormatter::toString(payoff->strike()) +"\n" \
               "    dividend yield:   " \
               + RateFormatter::toString(q) + "\n" \
               "    risk-free rate:   " \
               + RateFormatter::toString(r) + "\n" \
               "    reference date:   " \
               + DateFormatter::toString(today) + "\n" \
               "    maturity:         " \
               + DateFormatter::toString(exercise->lastDate()) + "\n" \
               "    volatility:       " \
               + VolatilityFormatter::toString(v) + "\n\n" \
               "    expected   " + greekName + ": " \
               + DecimalFormatter::toString(expected) + "\n" \
               "    calculated " + greekName + ": " \
               + DecimalFormatter::toString(calculated) + "\n" \
               "    error:            " \
               + DecimalFormatter::toString(error) + "\n" \
               "    tolerance:        " \
               + DecimalFormatter::toString(tolerance));

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
    };

}


void DigitalOptionTest::testCashOrNothingEuropeanValues() {

    BOOST_MESSAGE("Testing European cash-or-nothing digital option...");

    DigitalOptionData values[] = {
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 88
        //        type, strike,  spot,    q,    r,    t,  vol,  value, tol
        { Option::Put,   80.00, 100.0, 0.06, 0.06, 0.75, 0.35, 2.6710, 1e-4 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(new AnalyticEuropeanEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new CashOrNothingPayoff(
            values[i].type, values[i].strike, 10.0));

        Date exDate = today.plusDays(Integer(values[i].t*360+0.5));
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(Handle<Quote>(spot),
                                Handle<YieldTermStructure>(qTS),
                                Handle<YieldTermStructure>(rTS),
                                Handle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, exercise, engine);

        Real calculated = opt.NPV();
        Real error = QL_FABS(calculated-values[i].result);
        if (error > values[i].tol) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s, values[i].q,
                           values[i].r, today, values[i].v, values[i].result,
                           calculated, error, values[i].tol);
        }
    }
}

void DigitalOptionTest::testAssetOrNothingEuropeanValues() {

    BOOST_MESSAGE("Testing European asset-or-nothing digital option...");

    // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 90
    DigitalOptionData values[] = {
        //        type, strike, spot,    q,    r,    t,  vol,   value, tol
        { Option::Put,   65.00, 70.0, 0.05, 0.07, 0.50, 0.27, 20.2069, 1e-4 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(new AnalyticEuropeanEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new AssetOrNothingPayoff(
            values[i].type, values[i].strike));

        Date exDate = today.plusDays(Integer(values[i].t*360+0.5));
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(Handle<Quote>(spot),
                                Handle<YieldTermStructure>(qTS),
                                Handle<YieldTermStructure>(rTS),
                                Handle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, exercise, engine);

        Real calculated = opt.NPV();
        Real error = QL_FABS(calculated-values[i].result);
        if (error > values[i].tol) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s, values[i].q,
                           values[i].r, today, values[i].v, values[i].result,
                           calculated, error, values[i].tol);
        }
    }
}

void DigitalOptionTest::testGapEuropeanValues() {

    BOOST_MESSAGE("Testing European gap digital option...");

    // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 88
    DigitalOptionData values[] = {
        //        type, strike, spot,    q,    r,    t,  vol,   value, tol
        { Option::Call,  50.00, 50.0, 0.00, 0.09, 0.50, 0.20, -0.0053, 1e-4 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(new AnalyticEuropeanEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new GapPayoff(
            values[i].type, values[i].strike, 57.00));

        Date exDate = today.plusDays(Integer(values[i].t*360+0.5));
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(Handle<Quote>(spot),
                                Handle<YieldTermStructure>(qTS),
                                Handle<YieldTermStructure>(rTS),
                                Handle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, exercise, engine);

        Real calculated = opt.NPV();
        Real error = QL_FABS(calculated-values[i].result);
        if (error > values[i].tol) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s, values[i].q,
                           values[i].r, today, values[i].v, values[i].result,
                           calculated, error, values[i].tol);
        }
    }
}

void DigitalOptionTest::testCashAtHitOrNothingAmericanValues() {

    BOOST_MESSAGE("Testing American cash-(at-hit)-or-nothing "
                  "digital option...");

    DigitalOptionData values[] = {
        //        type, strike,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 1,2
        { Option::Put,  100.00, 105.00, 0.00, 0.10, 0.5, 0.20,  9.7264, 1e-4 },
        { Option::Call, 100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 11.6553, 1e-4 },

        // the following cases are not taken from a reference paper or book
        // in the money options (guaranteed immediate payoff)
        { Option::Call, 100.00, 105.00, 0.00, 0.10, 0.5, 0.20, 15.0000, 1e-16},
        { Option::Put,  100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 15.0000, 1e-16},
        // non null dividend (cross-tested with MC simulation)
        { Option::Put,  100.00, 105.00, 0.20, 0.10, 0.5, 0.20, 12.2715, 1e-4 },
        { Option::Call, 100.00,  95.00, 0.20, 0.10, 0.5, 0.20,  8.9109, 1e-4 },
        { Option::Call, 100.00, 105.00, 0.20, 0.10, 0.5, 0.20, 15.0000, 1e-16},
        { Option::Put,  100.00,  95.00, 0.20, 0.10, 0.5, 0.20, 15.0000, 1e-16}
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(new AnalyticDigitalAmericanEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new CashOrNothingPayoff(
            values[i].type, values[i].strike, 15.00));

        Date exDate = today.plusDays(Integer(values[i].t*360+0.5));
        boost::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                    exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(Handle<Quote>(spot),
                                Handle<YieldTermStructure>(qTS),
                                Handle<YieldTermStructure>(rTS),
                                Handle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, amExercise,
                          engine);

        Real calculated = opt.NPV();
        Real error = QL_FABS(calculated-values[i].result);
        if (error > values[i].tol) {
            REPORT_FAILURE("value", payoff, amExercise, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           values[i].result, calculated, error, values[i].tol);
        }
    }
}

void DigitalOptionTest::testAssetAtHitOrNothingAmericanValues() {

    BOOST_MESSAGE("Testing American asset-(at-hit)-or-nothing "
                  "digital option...");

    DigitalOptionData values[] = {
        //        type, strike,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 3,4
        { Option::Put,  100.00, 105.00, 0.00, 0.10, 0.5, 0.20, 64.8426, 1e-04 }, // Haug value is wrong here, Haug VBA code is right
        { Option::Call, 100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 77.7017, 1e-04 }, // Haug value is wrong here, Haug VBA code is right
        // data from Haug VBA code results
        { Option::Put,  100.00, 105.00, 0.01, 0.10, 0.5, 0.20, 65.7811, 1e-04 },
        { Option::Call, 100.00,  95.00, 0.01, 0.10, 0.5, 0.20, 76.8858, 1e-04 },
        // in the money options  (guaranteed immediate payoff = spot)
        { Option::Call, 100.00, 105.00, 0.00, 0.10, 0.5, 0.20,105.0000, 1e-16 },
        { Option::Put,  100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 95.0000, 1e-16 },
        { Option::Call, 100.00, 105.00, 0.01, 0.10, 0.5, 0.20,105.0000, 1e-16 },
        { Option::Put,  100.00,  95.00, 0.01, 0.10, 0.5, 0.20, 95.0000, 1e-16 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.01));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(new AnalyticDigitalAmericanEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new AssetOrNothingPayoff(
            values[i].type, values[i].strike));

        Date exDate = today.plusDays(Integer(values[i].t*360+0.5));
        boost::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                    exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(Handle<Quote>(spot),
                                Handle<YieldTermStructure>(qTS),
                                Handle<YieldTermStructure>(rTS),
                                Handle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, amExercise,
                          engine);

        Real calculated = opt.NPV();
        Real error = QL_FABS(calculated-values[i].result);
        if (error > values[i].tol) {
            REPORT_FAILURE("value", payoff, amExercise, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           values[i].result, calculated, error, values[i].tol);
        }
    }
}

void DigitalOptionTest::testCashAtExpiryOrNothingAmericanValues() {

    BOOST_MESSAGE("Testing American cash-(at-expiry)-or-nothing "
                  "digital option...");

    DigitalOptionData values[] = {
        //        type, strike,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 1,2
        { Option::Put,  100.00, 105.00, 0.00, 0.10, 0.5, 0.20,  9.3604, 1e-4 },
        { Option::Call, 100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 11.2223, 1e-4 },
        // in the money options (guaranteed discounted payoff)
        { Option::Call, 100.00, 105.00, 0.00, 0.10, 0.5, 0.20, 15.0000*QL_EXP(-0.05), 1e-16 },
        { Option::Put,  100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 15.0000*QL_EXP(-0.05), 1e-16 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.01));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(new AnalyticDigitalAmericanEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new CashOrNothingPayoff(
            values[i].type, values[i].strike, 15.0));

        Date exDate = today.plusDays(Integer(values[i].t*360+0.5));
        boost::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                    exDate,
                                                                    true));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(Handle<Quote>(spot),
                                Handle<YieldTermStructure>(qTS),
                                Handle<YieldTermStructure>(rTS),
                                Handle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, amExercise,
                          engine);

        Real calculated = opt.NPV();
        Real error = QL_FABS(calculated-values[i].result);
        if (error > values[i].tol) {
            REPORT_FAILURE("value", payoff, amExercise, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           values[i].result, calculated, error, values[i].tol);
        }
    }
}

void DigitalOptionTest::testAssetAtExpiryOrNothingAmericanValues() {

    BOOST_MESSAGE("Testing American asset-(at-expiry)-or-nothing "
                  "digital option...");

    DigitalOptionData values[] = {
        //        type, strike,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 3,4
        { Option::Put,  100.00, 105.00, 0.00, 0.10, 0.5, 0.20, 64.8426, 1e-04 },
        { Option::Call, 100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 77.7017, 1e-04 },
        // data from Haug VBA code results
        { Option::Put,  100.00, 105.00, 0.01, 0.10, 0.5, 0.20, 65.5291, 1e-04 },
        { Option::Call, 100.00,  95.00, 0.01, 0.10, 0.5, 0.20, 76.5951, 1e-04 },
        // in the money options (guaranteed discounted payoff = forward * riskFreeDiscount
        //                                                    = spot * dividendDiscount)
        { Option::Call, 100.00, 105.00, 0.00, 0.10, 0.5, 0.20,105.0000, 1e-16 },
        { Option::Put,  100.00,  95.00, 0.00, 0.10, 0.5, 0.20, 95.0000, 1e-16 },
        { Option::Call, 100.00, 105.00, 0.01, 0.10, 0.5, 0.20,105.0000*QL_EXP(-0.005), 1e-16 },
        { Option::Put,  100.00,  95.00, 0.01, 0.10, 0.5, 0.20, 95.0000*QL_EXP(-0.005), 1e-16 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.01));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.25));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(new AnalyticDigitalAmericanEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new AssetOrNothingPayoff(
            values[i].type, values[i].strike));

        Date exDate = today.plusDays(Integer(values[i].t*360+0.5));
        boost::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                    exDate,
                                                                    true));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(Handle<Quote>(spot),
                                Handle<YieldTermStructure>(qTS),
                                Handle<YieldTermStructure>(rTS),
                                Handle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, amExercise,
                          engine);

        Real calculated = opt.NPV();
        Real error = QL_FABS(calculated-values[i].result);
        if (error > values[i].tol) {
            REPORT_FAILURE("value", payoff, amExercise, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           values[i].result, calculated, error, values[i].tol);
        }
    }
}

void DigitalOptionTest::testCashAtHitOrNothingAmericanGreeks() {

    BOOST_MESSAGE("Testing American cash-(at-hit)-or-nothing "
                  "digital option greeks...");

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]  = 5.0e-5;
    tolerance["gamma"]  = 5.0e-5;
    tolerance["theta"]  = 5.0e-5;
    tolerance["rho"]    = 5.0e-5;
    tolerance["divRho"] = 5.0e-5;
    tolerance["vega"]   = 5.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.5, 150.0 };
    Real cashPayoff = 100.0;
    Real underlyings[] = { 100 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Volatility vols[] = { 0.11, 0.5, 1.2 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    // there is no cycling on different residual times
    Date exDate = today.plusDays(360);
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
    boost::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                exDate,
                                                                false));
    boost::shared_ptr<Exercise> exercises[] = { exercise, amExercise };

    boost::shared_ptr<PricingEngine> euroEngine(new AnalyticEuropeanEngine());

    boost::shared_ptr<PricingEngine> amEngine(
                                         new AnalyticDigitalAmericanEngine());

    boost::shared_ptr<PricingEngine> engines[] = { euroEngine, amEngine };

    for (Size j=0; j<LENGTH(engines); j++) {
      for (Size i1=0; i1<LENGTH(types); i1++) {
        for (Size i6=0; i6<LENGTH(strikes); i6++) {
          boost::shared_ptr<StrikedTypePayoff> payoff(
                            new CashOrNothingPayoff(types[i1],
                                                    strikes[i6], cashPayoff));

          boost::shared_ptr<BlackScholesProcess> stochProcess(new
                BlackScholesProcess(Handle<Quote>(spot), qTS, rTS, volTS));

          VanillaOption opt(stochProcess, payoff, exercises[j], engines[j]);

          for (Size i2=0; i2<LENGTH(underlyings); i2++) {
            for (Size i4=0; i4<LENGTH(qRates); i4++) {
              for (Size i3=0; i3<LENGTH(rRates); i3++) {
                for (Size i7=0; i7<LENGTH(vols); i7++) {
                  // test data
                  Real u = underlyings[i2];
                  Rate q = qRates[i4];
                  Rate r = rRates[i3];
                  Volatility v = vols[i7];
                  spot->setValue(u);
                  qRate->setValue(q);
                  rRate->setValue(r);
                  vol->setValue(v);

                  // just delta and rho are available for digital option with
                  // american exercise. Greeks of digital options with european
                  // payoff are tested in the europeanoption.cpp test
                  Real value = opt.NPV();
                  calculated["delta"]  = opt.delta();
                  calculated["gamma"]  = opt.gamma();
                  //calculated["theta"]  = opt.theta();
                  calculated["rho"]    = opt.rho();
                  //calculated["divRho"] = opt.dividendRho();
                  //calculated["vega"]   = opt.vega();

                  if (value > 1.0e-6) {
                      // perturb spot and get delta and gamma
                      Real du = u*1.0e-4;
                      spot->setValue(u+du);
                      Real value_p = opt.NPV(),
                           delta_p = opt.delta();
                      spot->setValue(u-du);
                      Real value_m = opt.NPV(),
                           delta_m = opt.delta();
                      spot->setValue(u);
                      expected["delta"] = (value_p - value_m)/(2*du);
                      expected["gamma"] = (delta_p - delta_m)/(2*du);

                      // perturb rates and get rho and dividend rho
                      Spread dr = r*1.0e-4;
                      rRate->setValue(r+dr);
                      value_p = opt.NPV();
                      rRate->setValue(r-dr);
                      value_m = opt.NPV();
                      rRate->setValue(r);
                      expected["rho"] = (value_p - value_m)/(2*dr);

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
                      Time dT = 1.0/360;
                      qTS.linkTo(flatRate(today-1,qRate,dc));
                      rTS.linkTo(flatRate(today-1,rRate,dc));
                      volTS.linkTo(flatVol(today-1,vol,dc));
                      value_m = opt.NPV();
                      qTS.linkTo(flatRate(today+1,qRate,dc));
                      rTS.linkTo(flatRate(today+1,rRate,dc));
                      volTS.linkTo(flatVol(today+1,vol,dc));
                      value_p = opt.NPV();
                      qTS.linkTo(flatRate(today,qRate,dc));
                      rTS.linkTo(flatRate(today,rRate,dc));
                      volTS.linkTo(flatVol(today,vol,dc));
                      expected["theta"] = (value_p - value_m)/(2*dT);

                      // check
                      std::map<std::string,Real>::iterator it;
                      for (it = calculated.begin();
                           it != calculated.end(); ++it) {
                          std::string greek = it->first;
                          Real expct = expected  [greek],
                               calcl = calculated[greek],
                               tol   = tolerance [greek];
                          Real error = relativeError(expct,calcl,value);
                          if (error > tol) {
                              REPORT_FAILURE(greek, payoff, exercise,
                                             u, q, r, today, v,
                                             expct, calcl, error, tol);
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

    BOOST_MESSAGE("Testing Monte Carlo cash-(at-hit)-or-nothing "
                  "American engine...");

    DigitalOptionData values[] = {
        //        type, strike,   spot,    q,    r,   t,  vol,   value, tol
        { Option::Put,  100.00, 105.00, 0.20, 0.10, 0.5, 0.20, 12.2715, 5e-3 },
        { Option::Call, 100.00,  95.00, 0.20, 0.10, 0.5, 0.20,  8.9109, 5e-3 }
//        { Option::Call, 100.00, 105.00, 0.20, 0.10, 0.5, 0.20, 15.0000, 1e-16},
//        { Option::Put,  100.00,  95.00, 0.20, 0.10, 0.5, 0.20, 15.0000, 1e-16}
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    Size maxTimeStepsPerYear = 90;
    bool brownianBridge, antitheticVariate;
    bool controlVariate = false;
    Size maxSamples = 1000000;
    BigNatural seed = 1;

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new CashOrNothingPayoff(
            values[i].type, values[i].strike, 15.0));

        Date exDate = today.plusDays(Integer(values[i].t*360+0.5));
        boost::shared_ptr<Exercise> amExercise(
                                         new AmericanExercise(today, exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(Handle<Quote>(spot),
                                Handle<YieldTermStructure>(qTS),
                                Handle<YieldTermStructure>(rTS),
                                Handle<BlackVolTermStructure>(volTS)));

        antitheticVariate = true;
        brownianBridge = false;
        boost::shared_ptr<PricingEngine> mcEngine(new
            MCDigitalEngine<PseudoRandom>(maxTimeStepsPerYear, brownianBridge,
                                          antitheticVariate, controlVariate,
                                          Null<Size>(), values[i].tol,
                                          maxSamples, seed));

        antitheticVariate = false;
        brownianBridge = true;
        Size requiredSamples = Size(QL_POW(2.0, 14)-1);
        boost::shared_ptr<PricingEngine> mcldEngine(new
            MCDigitalEngine<LowDiscrepancy>(maxTimeStepsPerYear,
                                            brownianBridge,
                                            antitheticVariate, controlVariate,
                                            requiredSamples, Null<Real>(),
                                            maxSamples, seed));



        VanillaOption opt(stochProcess, payoff, amExercise,
                          mcEngine);

        Real calculated = opt.NPV();
/*
        std::cout << "\n MC:   " << DecimalFormatter::toString(calculated);

        opt.setPricingEngine(mcldEngine);
        Real calculatedLD = opt.NPV();
        std::cout << "\n MCLD: " << DecimalFormatter::toString(calculatedLD) << " with samples: " << requiredSamples;

        boost::shared_ptr<PricingEngine> amEngine(new AnalyticDigitalAmericanEngine());
        opt.setPricingEngine(amEngine);
        Real calcAnalytic = opt.NPV();
        std::cout << "\n anal: " << DecimalFormatter::toString(calcAnalytic);
*/
        Real error = relativeError(calculated, values[i].result, values[i].result);
        if (error > 2.0*values[i].tol) {
            REPORT_FAILURE("value", payoff, amExercise, values[i].s,
                           values[i].q, values[i].r, today, values[i].v,
                           values[i].result, calculated, error, values[i].tol);
        }
    }
}


test_suite* DigitalOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Digital option tests");
    suite->add(BOOST_TEST_CASE(
               &DigitalOptionTest::testCashOrNothingEuropeanValues));
    suite->add(BOOST_TEST_CASE(
               &DigitalOptionTest::testAssetOrNothingEuropeanValues));
    suite->add(BOOST_TEST_CASE(&DigitalOptionTest::testGapEuropeanValues));
    suite->add(BOOST_TEST_CASE(
               &DigitalOptionTest::testCashAtHitOrNothingAmericanValues));
    suite->add(BOOST_TEST_CASE(
               &DigitalOptionTest::testAssetAtHitOrNothingAmericanValues));
    suite->add(BOOST_TEST_CASE(
               &DigitalOptionTest::testCashAtExpiryOrNothingAmericanValues));
    suite->add(BOOST_TEST_CASE(
               &DigitalOptionTest::testAssetAtExpiryOrNothingAmericanValues));
    suite->add(BOOST_TEST_CASE(&DigitalOptionTest::testMCCashAtHit));
    return suite;
}

