
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
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/vanillaoption.hpp>
#include <ql/PricingEngines/Vanilla/analyticeuropeanengine.hpp>
#include <ql/PricingEngines/Vanilla/analyticamericanengine.hpp>
#include <ql/PricingEngines/Vanilla/mcdigitalengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <map>

// This makes it easier to use array literals (alas, no std::vector literals)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace QuantLib;

namespace {

    Handle<TermStructure> makeFlatCurve(const Handle<Quote>& forward,
        DayCounter dc) {
        Date today = Date::todaysDate();
        return Handle<TermStructure>(new
            FlatForward(today, today, RelinkableHandle<Quote>(forward), dc));
    }

    Handle<BlackVolTermStructure> makeFlatVolatility(
        const Handle<Quote>& volatility, DayCounter dc) {
        Date today = Date::todaysDate();
        return Handle<BlackVolTermStructure>(new
            BlackConstantVol(today, RelinkableHandle<Quote>(volatility), dc));
    }

    std::string payoffTypeToString(const Handle<Payoff>& payoff) {

        // PlainVanillaPayoff?
        Handle<PlainVanillaPayoff> pv;
        #if defined(HAVE_BOOST)
        pv = boost::dynamic_pointer_cast<PlainVanillaPayoff>(payoff);
        #else
        try {
            pv = payoff;
        } catch (...) {}
        #endif
        if (!IsNull(pv)) {
            // ok, the payoff is PlainVanillaPayoff
            return "PlainVanillaPayoff";
        }

        // CashOrNothingPayoff?
        Handle<CashOrNothingPayoff> coo;
        #if defined(HAVE_BOOST)
        coo = boost::dynamic_pointer_cast<CashOrNothingPayoff>(payoff);
        #else
        try {
            coo = payoff;
        } catch (...) {}
        #endif
        if (!IsNull(coo)) {
            // ok, the payoff is CashOrNothingPayoff
            return "Cash ("
                + DoubleFormatter::toString(coo->cashPayoff())
                + ") or Nothing Payoff";
        }

        // AssetOrNothingPayoff?
        Handle<AssetOrNothingPayoff> aoo;
        #if defined(HAVE_BOOST)
        aoo = boost::dynamic_pointer_cast<AssetOrNothingPayoff>(payoff);
        #else
        try {
            aoo = payoff;
        } catch (...) {}
        #endif
        if (!IsNull(aoo)) {
            // ok, the payoff is AssetOrNothingPayoff
            return "AssetOrNothingPayoff";
        }

        // GapPayoff?
        Handle<GapPayoff> gap;
        #if defined(HAVE_BOOST)
        gap = boost::dynamic_pointer_cast<GapPayoff>(payoff);
        #else
        try {
            gap = payoff;
        } catch (...) {}
        #endif
        if (!IsNull(gap)) {
            // ok, the payoff is GapPayoff
            return "Gap ("
                + DoubleFormatter::toString(gap->strikePayoff())
                + " strike) Payoff";
        }

        // SuperSharePayoff?
        Handle<SuperSharePayoff> ss;
        #if defined(HAVE_BOOST)
        ss = boost::dynamic_pointer_cast<SuperSharePayoff>(payoff);
        #else
        try {
            ss = payoff;
        } catch (...) {}
        #endif
        if (!IsNull(ss)) {
            // ok, the payoff is SuperSharePayoff
            return "SuperSharePayoff";
        }

        throw Error("payoffTypeToString : unknown payoff type");
    }


    std::string exerciseTypeToString(const Handle<Exercise>& exercise) {

        // EuropeanExercise?
        Handle<EuropeanExercise> european;
        #if defined(HAVE_BOOST)
        european = boost::dynamic_pointer_cast<EuropeanExercise>(exercise);
        #else
        try {
            european = exercise;
        } catch (...) {}
        #endif
        if (!IsNull(european)) {
            return "European";
        }

        // AmericanExercise?
        Handle<AmericanExercise> american;
        #if defined(HAVE_BOOST)
        american = boost::dynamic_pointer_cast<AmericanExercise>(exercise);
        #else
        try {
            american = exercise;
        } catch (...) {}
        #endif
        if (!IsNull(american)) {
            return "American";
        }

        // BermudanExercise?
        Handle<BermudanExercise> bermudan;
        #if defined(HAVE_BOOST)
        bermudan = boost::dynamic_pointer_cast<BermudanExercise>(exercise);
        #else
        try {
            bermudan = exercise;
        } catch (...) {}
        #endif
        if (!IsNull(bermudan)) {
            return "Bermudan";
        }

        throw Error("exerciseTypeToString : unknown exercise type");
    }

    void vanillaOptionTestFailed(std::string greekName,
               const Handle<StrikedTypePayoff>& payoff,
               const Handle<Exercise>& exercise,
               double s,
               double q,
               double r,
               Date today,
               double v,
               double expected,
               double calculated,
               double tolerance = Null<double>()) {
      CPPUNIT_FAIL(exerciseTypeToString(exercise) + " "
          + OptionTypeFormatter::toString(payoff->optionType()) +
          " option with "
          + payoffTypeToString(payoff) + ":\n"
          "    underlying value: "
          + DoubleFormatter::toString(s) + "\n"
          "    strike:           "
          + DoubleFormatter::toString(payoff->strike()) +"\n"
          "    dividend yield:   "
          + DoubleFormatter::toString(q) + "\n"
          "    risk-free rate:   "
          + DoubleFormatter::toString(r) + "\n"
          "    reference date:   "
          + DateFormatter::toString(today) + "\n"
          "    maturity:         "
          + DateFormatter::toString(exercise->lastDate()) + "\n"
          "    volatility:       "
          + DoubleFormatter::toString(v) + "\n\n"
          "    expected   " + greekName + ": "
          + DoubleFormatter::toString(expected) + "\n"
          "    calculated " + greekName + ": "
          + DoubleFormatter::toString(calculated) + "\n"
          "    error:            "
          + DoubleFormatter::toString(QL_FABS(expected-calculated)) + "\n"
          + (tolerance==Null<double>() ? std::string("") :
          "    tolerance:        " + DoubleFormatter::toString(tolerance)));
    }

    struct DigitalOptionData {
        Option::Type type;
        double strike;
        double extraParameter;
        double s;      // spot
        Rate q;        // dividend
        Rate r;        // risk-free rate
        Time t;        // time to maturity
        double v;      // volatility
        double result; // expected result
        double tol;    // tolerance
    };

    double relativeError(double x1, double x2, double reference) {
        if (reference != 0.0)
            // relative error
            return QL_FABS((x1-x2)/reference);
        else
            // absolute error
            return QL_FABS(x1-x2);
    }

}

void DigitalOptionTest::testCashOrNothingEuropeanValues() {

    DigitalOptionData values[] = {
        //        type, strike, payoff,  spot,            q,            r,    t,  vol,     value, tol
        // unknow source
        { Option::Call, 110.00, 100.00, 100.0,     0.029559,     0.086178, 1.00, 0.20, 35.283179, 1e-4 },
        { Option::Call, 110.00, 100.00, 100.0, QL_LOG(1.03), QL_LOG(1.09), 1.00, 0.20, 35.283179, 1e-4 },
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 88
        { Option::Put,   80.00,  10.00, 100.0,         0.06,         0.06, 0.75, 0.35,    2.6710, 1e-4 }
    };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot(new SimpleQuote(0.0));
    Handle<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);
    Handle<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol, dc);
    Handle<PricingEngine> engine(new AnalyticEuropeanEngine);

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new CashOrNothingPayoff(
            values[i].type, values[i].strike, values[i].extraParameter));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        Handle<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, exercise, engine);

        double calculated = opt.NPV();
        if (QL_FABS(calculated-values[i].result) > values[i].tol) {
            vanillaOptionTestFailed("value", payoff, exercise, values[i].s, values[i].q,
                values[i].r, today, values[i].v, values[i].result, calculated,
                values[i].tol);
        }
    }
}

void DigitalOptionTest::testAssetOrNothingEuropeanValues() {

    // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 90
    DigitalOptionData values[] = {
        //        type, strike, unused,  spot,    q,    r,    t,  vol,   value, tol
        { Option::Put,   65.00,   0.00,  70.0, 0.05, 0.07, 0.50, 0.27, 20.2069, 1e-4 }
    };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot(new SimpleQuote(0.0));
    Handle<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);
    Handle<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol, dc);
    Handle<PricingEngine> engine(new AnalyticEuropeanEngine);

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new AssetOrNothingPayoff(
            values[i].type, values[i].strike));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        Handle<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, exercise, engine);

        double calculated = opt.NPV();
        if (QL_FABS(calculated-values[i].result) > values[i].tol) {
            vanillaOptionTestFailed("value", payoff, exercise, values[i].s, values[i].q,
                values[i].r, today, values[i].v, values[i].result, calculated,
                values[i].tol);
        }
    }
}

void DigitalOptionTest::testGapEuropeanValues() {

    // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 88
    DigitalOptionData values[] = {
        //        type, strike, strikePayoff,  spot,    q,    r,    t,  vol,   value, tol
        { Option::Call,  50.00,        57.00,  50.0, 0.00, 0.09, 0.50, 0.20, -0.0053, 1e-4 }
    };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot(new SimpleQuote(0.0));
    Handle<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);
    Handle<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol, dc);
    Handle<PricingEngine> engine(new AnalyticEuropeanEngine);

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new GapPayoff(
            values[i].type, values[i].strike, values[i].extraParameter));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        Handle<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, exercise, engine);

        double calculated = opt.NPV();
        if (QL_FABS(calculated-values[i].result) > values[i].tol) {
            vanillaOptionTestFailed("value", payoff, exercise, values[i].s, values[i].q,
                values[i].r, today, values[i].v, values[i].result, calculated,
                values[i].tol);
        }
    }
}

void DigitalOptionTest::testCashAtHitOrNothingAmericanValues() {

    DigitalOptionData values[] = {
        //        type, strike, payoff,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 1,2
        { Option::Put,  100.00,  15.00, 105.00, 0.00, 0.10, 0.5, 0.20,  9.7264, 1e-4 },
        { Option::Call, 100.00,  15.00,  95.00, 0.00, 0.10, 0.5, 0.20, 11.6553, 1e-4 },
        // in the money options (guaranteed immediate payoff)
        { Option::Call, 100.00,  15.00, 105.00, 0.00, 0.10, 0.5, 0.20, 15.0000, 1e-16 },
        { Option::Put,  100.00,  15.00,  95.00, 0.00, 0.10, 0.5, 0.20, 15.0000, 1e-16 }
        // unknown source - wrong values
//        { Option::Call, 100.50, 100.00, 100.00, 0.04, 0.01, 1.0, 0.11, 94.8825, 1e-4 },
//        { Option::Call, 100.50, 100.00, 100.00, 0.00, 0.01, 1.0, 0.11, 96.5042, 1e-4 },
//        { Option::Call, 120.00, 100.00, 100.00, 0.04, 0.01, 1.0, 0.11,  5.5676, 1e-4 },
//        { Option::Call, 100.50, 100.00, 100.00, 0.04, 0.01, 1.0, 0.20, 97.3989, 1e-4 },
//        { Option::Call, 100.50, 100.00, 100.00, 0.04, 0.10, 1.0, 0.11, 97.9405, 1e-4 },
//        { Option::Call, 100.50, 100.00, 100.00, 0.04, 0.01, 2.0, 0.11, 95.8913, 1e-4 },
//        { Option::Put,   99.50, 100.00, 100.00, 0.04, 0.01, 1.0, 0.11, 97.7331, 1e-4 },
//        { Option::Put,   99.50, 100.00, 100.00, 0.00, 0.01, 1.0, 0.11, 96.1715, 1e-4 },
//        { Option::Put,   80.00, 100.00, 100.00, 0.04, 0.01, 1.0, 0.11,  8.1172, 1e-4 },
//        { Option::Put,   99.50, 100.00, 100.00, 0.04, 0.01, 1.0, 0.20, 98.6140, 1e-4 },
//        { Option::Put,   99.50, 100.00, 100.00, 0.04, 0.10, 1.0, 0.11, 93.6491, 1e-4 },
//        { Option::Put,   99.50, 100.00, 100.00, 0.04, 0.01, 2.0, 0.11, 98.7776, 1e-4 }
    };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot(new SimpleQuote(100.0));
    Handle<SimpleQuote> qRate(new SimpleQuote(0.04));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.01));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);
    Handle<SimpleQuote> vol(new SimpleQuote(0.25));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol, dc);
    Handle<PricingEngine> engine(new AnalyticAmericanEngine);

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new CashOrNothingPayoff(
            values[i].type, values[i].strike, values[i].extraParameter));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> amExercise(new AmericanExercise(today, exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        Handle<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, amExercise,
                          engine);

        double calculated = opt.NPV();
        if (QL_FABS(calculated-values[i].result) > values[i].tol) {
            vanillaOptionTestFailed("value", payoff, amExercise, values[i].s, values[i].q,
                values[i].r, today, values[i].v, values[i].result, calculated,
                values[i].tol);
        }
    }
}

void DigitalOptionTest::testAssetAtHitOrNothingAmericanValues() {

    DigitalOptionData values[] = {
        //        type, strike, payoff,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 3,4
        { Option::Put,  100.00,  15.00, 105.00, 0.00, 0.10, 0.5, 0.20, 64.8426, 1e-04 }, // Haug value is wrong here, Haug VBA code is right
        { Option::Call, 100.00,  15.00,  95.00, 0.00, 0.10, 0.5, 0.20, 77.7017, 1e-04 }, // Haug value is wrong here, Haug VBA code is right
        // data from Haug VBA code results
//        { Option::Put,  100.00,  15.00, 105.00, 0.01, 0.10, 0.5, 0.20, 65.7811, 1e-04 },
//        { Option::Call, 100.00,  15.00,  95.00, 0.01, 0.10, 0.5, 0.20, 76.8858, 1e-04 },
        // in the money options  (guaranteed immediate payoff = spot)
        { Option::Call, 100.00,  15.00, 105.00, 0.00, 0.10, 0.5, 0.20,105.0000, 1e-16 },
        { Option::Put,  100.00,  15.00,  95.00, 0.00, 0.10, 0.5, 0.20, 95.0000, 1e-16 },
        { Option::Call, 100.00,  15.00, 105.00, 0.01, 0.10, 0.5, 0.20,105.0000, 1e-16 },
        { Option::Put,  100.00,  15.00,  95.00, 0.01, 0.10, 0.5, 0.20, 95.0000, 1e-16 }
    };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot(new SimpleQuote(100.0));
    Handle<SimpleQuote> qRate(new SimpleQuote(0.04));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.01));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);
    Handle<SimpleQuote> vol(new SimpleQuote(0.25));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol, dc);
    Handle<PricingEngine> engine(new AnalyticAmericanEngine);

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new AssetOrNothingPayoff(
            values[i].type, values[i].strike));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> amExercise(new AmericanExercise(today, exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        Handle<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, amExercise,
                          engine);

        double calculated = opt.NPV();
        if (QL_FABS(calculated-values[i].result) > values[i].tol) {
            vanillaOptionTestFailed("value", payoff, amExercise, values[i].s, values[i].q,
                values[i].r, today, values[i].v, values[i].result, calculated,
                values[i].tol);
        }
    }
}

void DigitalOptionTest::testCashAtExpiryOrNothingAmericanValues() {

    DigitalOptionData values[] = {
        //        type, strike, payoff,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 1,2
        { Option::Put,  100.00,  15.00, 105.00, 0.00, 0.10, 0.5, 0.20,  9.3604, 1e-4 },
        { Option::Call, 100.00,  15.00,  95.00, 0.00, 0.10, 0.5, 0.20, 11.2223, 1e-4 },
        // in the money options (guaranteed discounted payoff)
        { Option::Call, 100.00,  15.00, 105.00, 0.00, 0.10, 0.5, 0.20, 15.0000*QL_EXP(-0.05), 1e-16 },
        { Option::Put,  100.00,  15.00,  95.00, 0.00, 0.10, 0.5, 0.20, 15.0000*QL_EXP(-0.05), 1e-16 }
    };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot(new SimpleQuote(100.0));
    Handle<SimpleQuote> qRate(new SimpleQuote(0.04));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.01));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);
    Handle<SimpleQuote> vol(new SimpleQuote(0.25));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol, dc);
    Handle<PricingEngine> engine(new AnalyticAmericanEngine);

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new CashOrNothingPayoff(
            values[i].type, values[i].strike, values[i].extraParameter));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> amExercise(new AmericanExercise(today, exDate, true));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        Handle<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, amExercise,
                          engine);

        double calculated = opt.NPV();
        if (QL_FABS(calculated-values[i].result) > values[i].tol) {
            vanillaOptionTestFailed("value", payoff, amExercise, values[i].s, values[i].q,
                values[i].r, today, values[i].v, values[i].result, calculated,
                values[i].tol);
        }
    }
}

void DigitalOptionTest::testAssetAtExpiryOrNothingAmericanValues() {

    DigitalOptionData values[] = {
        //        type, strike, payoff,   spot,    q,    r,   t,  vol,   value, tol
        // "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 - pag 95, case 3,4
        { Option::Put,  100.00,  15.00, 105.00, 0.00, 0.10, 0.5, 0.20, 64.8426, 1e-04 },
        { Option::Call, 100.00,  15.00,  95.00, 0.00, 0.10, 0.5, 0.20, 77.7017, 1e-04 },
        // data from Haug VBA code results
        { Option::Put,  100.00,  15.00, 105.00, 0.01, 0.10, 0.5, 0.20, 65.5291, 1e-04 },
        { Option::Call, 100.00,  15.00,  95.00, 0.01, 0.10, 0.5, 0.20, 76.5951, 1e-04 },
        // in the money options (guaranteed discounted payoff = forward * riskFreeDiscount
        //                                                    = spot * dividendDiscount)
        { Option::Call, 100.00,  15.00, 105.00, 0.00, 0.10, 0.5, 0.20,105.0000, 1e-16 },
        { Option::Put,  100.00,  15.00,  95.00, 0.00, 0.10, 0.5, 0.20, 95.0000, 1e-16 },
        { Option::Call, 100.00,  15.00, 105.00, 0.01, 0.10, 0.5, 0.20,105.0000*QL_EXP(-0.005), 1e-16 },
        { Option::Put,  100.00,  15.00,  95.00, 0.01, 0.10, 0.5, 0.20, 95.0000*QL_EXP(-0.005), 1e-16 }
    };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot(new SimpleQuote(100.0));
    Handle<SimpleQuote> qRate(new SimpleQuote(0.04));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.01));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);
    Handle<SimpleQuote> vol(new SimpleQuote(0.25));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol, dc);
    Handle<PricingEngine> engine(new AnalyticAmericanEngine);

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new AssetOrNothingPayoff(
            values[i].type, values[i].strike));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> amExercise(new AmericanExercise(today, exDate, true));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        Handle<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        VanillaOption opt(stochProcess, payoff, amExercise,
                          engine);

        double calculated = opt.NPV();
        if (QL_FABS(calculated-values[i].result) > values[i].tol) {
            vanillaOptionTestFailed("value", payoff, amExercise, values[i].s, values[i].q,
                values[i].r, today, values[i].v, values[i].result, calculated,
                values[i].tol);
        }
    }
}

void DigitalOptionTest::testCashAtHitOrNothingAmericanGreeks() {

    std::map<std::string,double> calculated, expected, tolerance;
    tolerance["delta"]  = 5.0e-5;
    tolerance["gamma"]  = 5.0e-5;
    tolerance["theta"]  = 5.0e-5;
    tolerance["rho"]    = 5.0e-5;
    tolerance["divRho"] = 5.0e-5;
    tolerance["vega"]   = 5.0e-5;

    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    double strikes[] = { 50.0, 99.5, 100.5, 150.0 };
    double cashPayoff = 100.0;
    double underlyings[] = { 100 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    double vols[] = { 0.11, 0.5, 1.2 };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot(new SimpleQuote(0.0));
    Handle<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);
    Handle<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol, dc);

    Date today = Date::todaysDate();

    // there is no cycling on different residual times
    Date exDate = today.plusDays(360);
    Handle<Exercise> exercise(new EuropeanExercise(exDate));
    Handle<Exercise> amExercise(new AmericanExercise(today, exDate, false));
    Handle<Exercise> exercises[] = { exercise, amExercise };
    // time-shifted exercise dates
    Date exDateP = exDate.plusDays(1),
         exDateM = exDate.plusDays(-1);
    Time dT = Actual360().yearFraction(today, exDateP) -
              Actual360().yearFraction(today, exDateM);
    Handle<Exercise> exerciseP(new EuropeanExercise(exDateP));
    Handle<Exercise> amExerciseP(new AmericanExercise(today, exDateP, false));
    Handle<Exercise> exercisesP[] = { exerciseP, amExerciseP };
    Handle<Exercise> exerciseM(new EuropeanExercise(exDateM));
    Handle<Exercise> amExerciseM(new AmericanExercise(today, exDateM, false));
    Handle<Exercise> exercisesM[] = { exerciseP, amExerciseM };

    Handle<PricingEngine> euroEngine = Handle<PricingEngine>(
        new AnalyticEuropeanEngine());

    Handle<PricingEngine> amEngine = Handle<PricingEngine>(
        new AnalyticAmericanEngine());

    Handle<PricingEngine> engines[] = { euroEngine, amEngine };

    for (Size j=0; j<LENGTH(engines); j++) {
      for (Size i1=0; i1<LENGTH(types); i1++) {
        for (Size i6=0; i6<LENGTH(strikes); i6++) {
            Handle<StrikedTypePayoff> payoff(new CashOrNothingPayoff(types[i1],
              strikes[i6], cashPayoff));

            Handle<BlackScholesStochasticProcess> stochProcess(new
                BlackScholesStochasticProcess(
                    RelinkableHandle<Quote>(spot),
                    RelinkableHandle<TermStructure>(qTS),
                    RelinkableHandle<TermStructure>(rTS),
                    RelinkableHandle<BlackVolTermStructure>(volTS)));

            // reference option
            VanillaOption opt(stochProcess, payoff, exercises[j],
                              engines[j]);
            // reference option with shifted exercise date
            VanillaOption optP(stochProcess, payoff, exercisesP[j],
                               engines[j]);
            // reference option with shifted exercise date
            VanillaOption optM(stochProcess, payoff, exercisesM[j],
                               engines[j]);
          for (Size i2=0; i2<LENGTH(underlyings); i2++) {
            for (Size i4=0; i4<LENGTH(qRates); i4++) {
              for (Size i3=0; i3<LENGTH(rRates); i3++) {
                for (Size i7=0; i7<LENGTH(vols); i7++) {
                  // test data
                  double u = underlyings[i2];
                  Rate q = qRates[i4];
                  Rate r = rRates[i3];
                  double v = vols[i7];
                  spot->setValue(u);
                  qRate->setValue(q);
                  rRate->setValue(r);
                  vol->setValue(v);

                  // just delta and rho are available for digital option with
                  // american exercise. Greeks of digital options with european
                  // payoff are tested in the europeanoption.cpp test
                  double value         = opt.NPV();
                  calculated["delta"]  = opt.delta();
                  calculated["gamma"]  = opt.gamma();
                  //calculated["theta"]  = opt.theta();
                  calculated["rho"]    = opt.rho();
                  //calculated["divRho"] = opt.dividendRho();
                  //calculated["vega"]   = opt.vega();

                  if (value > 1.0e-6) {
                      // perturb spot and get delta and gamma
                      double du = u*1.0e-4;
                      spot->setValue(u+du);
                      double value_p = opt.NPV(),
                             delta_p = opt.delta();
                      spot->setValue(u-du);
                      double value_m = opt.NPV(),
                             delta_m = opt.delta();
                      spot->setValue(u);
                      expected["delta"] = (value_p - value_m)/(2*du);
                      expected["gamma"] = (delta_p - delta_m)/(2*du);

                      // perturb rates and get rho and dividend rho
                      double dr = r*1.0e-4;
                      rRate->setValue(r+dr);
                      value_p = opt.NPV();
                      rRate->setValue(r-dr);
                      value_m = opt.NPV();
                      rRate->setValue(r);
                      expected["rho"] = (value_p - value_m)/(2*dr);

                      double dq = q*1.0e-4;
                      qRate->setValue(q+dq);
                      value_p = opt.NPV();
                      qRate->setValue(q-dq);
                      value_m = opt.NPV();
                      qRate->setValue(q);
                      expected["divRho"] = (value_p - value_m)/(2*dq);

                      // perturb volatility and get vega
                      double dv = v*1.0e-4;
                      vol->setValue(v+dv);
                      value_p = opt.NPV();
                      vol->setValue(v-dv);
                      value_m = opt.NPV();
                      vol->setValue(v);
                      expected["vega"] = (value_p - value_m)/(2*dv);

                      // get theta from time-shifted options
                      expected["theta"] = (optM.NPV() - optP.NPV())/dT;

                      // check
                      std::map<std::string,double>::iterator it;
                      for (it = calculated.begin(); it != calculated.end(); ++it) {
                          std::string greek = it->first;
                          double expct = expected  [greek],
                                 calcl = calculated[greek],
                                 tol   = tolerance [greek];
                          if (relativeError(expct,calcl,value) > tol) {
                              vanillaOptionTestFailed(greek, payoff, exercise, u, q, r, today,
                                  v, expct, calcl, tol);
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


// should be refactored as testEngineConsistency in europeanoption.cpp
void DigitalOptionTest::testEngineConsistency() {

    double cashPayoff = 100.0;
    Option::Type types[] = { Option::Call };
    double underlyings[] = { 100 };
//    Rate rRates[] = { 0.01, 0.05, 0.15 };
//    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01 };
    Rate qRates[] = { 0.06 };
//    double strikes[] = { 100.5, 150 };
    double strikes[] = { 101.0, 102.0 };
    double volatilities[] = { 0.10};
//    double volatilities[] = { 0.11, 0.5, 1.2 };

    Handle<SimpleQuote> spot(new SimpleQuote(underlyings[0]));

    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, Actual360());
    Handle<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, Actual360());
    Handle<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol,
        Actual360());

    Date today = Date::todaysDate();
    Date exDate = today.plusDays(360);
    Handle<Exercise> exercise(new EuropeanExercise(exDate));
    Handle<Exercise> amExercise(new AmericanExercise(today, exDate));
    Handle<Exercise> exercises[] = {amExercise};


    Size maxTimeStepsPerYear = 10;
//    Size maxTimeStepsPerYear = 365;
    bool antitheticVariate = false;
    bool controlVariate = false;
    Size requiredSamples = Size(QL_POW(2.0, 10)-1);
    double requiredAccuracy = 0.05;
//    double requiredAccuracy = 0.005;
    Size maxSamples = 1000000;
    long seed = 1;

    Handle<PricingEngine> mcEngine = Handle<PricingEngine>(new
        MCDigitalEngine<PseudoRandom>(maxTimeStepsPerYear,
                                      antitheticVariate, controlVariate,
                                      requiredSamples, requiredAccuracy,
                                      maxSamples, seed));

    Handle<PricingEngine> mcldEngine = Handle<PricingEngine>(new
        MCDigitalEngine<LowDiscrepancy>(maxTimeStepsPerYear,
                                        antitheticVariate, controlVariate,
                                        requiredSamples, Null<double>(),
                                        Null<int>()));

    Handle<PricingEngine> euroEngine = Handle<PricingEngine>(new
        AnalyticEuropeanEngine());

    Handle<PricingEngine> amEngine = Handle<PricingEngine>(new
        AnalyticAmericanEngine());

    //Handle<PricingEngine> engines[] = {euroEngine, amEngine};
    Handle<PricingEngine> engines[] = {amEngine};


//    double testTolerance = requiredAccuracy*2;
    double testTolerance = 1.0e-1;
    double calcAnalytic, calcMC;
    for (Size j=0; j<LENGTH(engines); j++) {
      for (Size i1=0; i1<LENGTH(types); i1++) {
        for (Size i6=0; i6<LENGTH(strikes); i6++) {
          for (Size i2=0; i2<LENGTH(underlyings); i2++) {
            for (Size i3=0; i3<LENGTH(rRates); i3++) {
              for (Size i4=0; i4<LENGTH(qRates); i4++) {
                for (Size i7=0; i7<LENGTH(volatilities); i7++) {

                  Option::Type type = types[i1];
                  double strike = strikes[i6];
                  Handle<StrikedTypePayoff> payoff(new CashOrNothingPayoff(
                      type, strike, cashPayoff));

                  double u = underlyings[i2];
                  spot->setValue(u);

                  Rate r = rRates[i3];
                  rRate->setValue(r);

                  Rate q = qRates[i4];
                  qRate->setValue(q);

                  double v = volatilities[i7];
                  vol->setValue(v);

                  Handle<BlackScholesStochasticProcess> stochProcess(new
                    BlackScholesStochasticProcess(
                        RelinkableHandle<Quote>(spot),
                        RelinkableHandle<TermStructure>(qTS),
                        RelinkableHandle<TermStructure>(rTS),
                        RelinkableHandle<BlackVolTermStructure>(volTS)));

                  // reference option
                  VanillaOption opt(stochProcess,
                      payoff,
                      exercises[j],
                      engines[j]);
                  calcAnalytic = opt.NPV();

//                  std::cout << "\n" << type << ", strike: " << strike;
//                  std::cout << ", u: " << u << ", r: " << r;
//                  std::cout << ", q: " << q << ", v: " << v;
//                  std::cout << ", "<< DoubleFormatter::toString(calcAnalytic);

//                  opt.setPricingEngine(mcldEngine);
//                  calcMC = opt.NPV();
                  opt.setPricingEngine(mcEngine);
                  calcMC = opt.NPV();

//                  std::cout << ", " << DoubleFormatter::toString(calcMC);

                  if (relativeError(calcAnalytic,calcMC,u) > testTolerance) {
                      vanillaOptionTestFailed("value", payoff, exercises[j], u,
                          q, r, today,
                          v, calcAnalytic, calcMC, testTolerance);
                  }
                }
              }
            }
          }
        }
      }
    }
}


CppUnit::Test* DigitalOptionTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Digital option tests");

    // European values
    tests->addTest(new CppUnit::TestCaller<DigitalOptionTest>
                   ("Testing European cash-or-nothing digital option",
                    &DigitalOptionTest::testCashOrNothingEuropeanValues));
    tests->addTest(new CppUnit::TestCaller<DigitalOptionTest>
                   ("Testing European asset-or-nothing digital option",
                    &DigitalOptionTest::testAssetOrNothingEuropeanValues));
    tests->addTest(new CppUnit::TestCaller<DigitalOptionTest>
                   ("Testing European gap digital option",
                    &DigitalOptionTest::testGapEuropeanValues));

    // American at-hit values
    tests->addTest(new CppUnit::TestCaller<DigitalOptionTest>
                   ("Testing American cash-(at-hit)-or-nothing digital option",
                    &DigitalOptionTest::testCashAtHitOrNothingAmericanValues));
    tests->addTest(new CppUnit::TestCaller<DigitalOptionTest>
                   ("Testing American asset-(at-hit)-or-nothing digital option",
                    &DigitalOptionTest::testAssetAtHitOrNothingAmericanValues));

    // American at-expiry values
    tests->addTest(new CppUnit::TestCaller<DigitalOptionTest>
                   ("Testing American cash-(at-expiry)-or-nothing digital option",
                    &DigitalOptionTest::testCashAtExpiryOrNothingAmericanValues));
    tests->addTest(new CppUnit::TestCaller<DigitalOptionTest>
                   ("Testing American asset-(at-expiry)-or-nothing digital option",
                    &DigitalOptionTest::testAssetAtExpiryOrNothingAmericanValues));


    // European greeks
    // tested in europeanoption.cpp test

    // American at-hit greeks
//    tests->addTest(new CppUnit::TestCaller<DigitalOptionTest>
//                   ("Testing American cash-(at-hit)-or-nothing digital option greeks",
//                    &DigitalOptionTest::testCashAtHitOrNothingAmericanGreeks));

    // American at-expiry greeks
    // missing for the time being

    // test of the MC engine for American options
    tests->addTest(new CppUnit::TestCaller<DigitalOptionTest>
                   ("Testing Monte Carlo pricing engine for digital options",
                    &DigitalOptionTest::testEngineConsistency));

    return tests;
}

