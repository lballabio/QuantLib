
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#include "americanoption.hpp"
#include "utilities.hpp"

#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/vanillaoption.hpp>
#include <ql/PricingEngines/Vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/PricingEngines/Vanilla/bjerksundstenslandengine.hpp>
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

    Handle<BlackVolTermStructure> makeFlatVolatility(const Handle<Quote>& vol,
                                                     DayCounter dc) {
        Date today = Date::todaysDate();
        return Handle<BlackVolTermStructure>(new
            BlackConstantVol(today, RelinkableHandle<Quote>(vol), dc));
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
                                 DayCounter dc,
                                 double v,
                                 double expected,
                                 double calculated,
                                 double error,
                                 double tolerance) {

        Time t = dc.yearFraction(today, exercise->lastDate());

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
            "    time to expiry:   "
            + DoubleFormatter::toString(t) + "\n"
            "    volatility:       "
            + DoubleFormatter::toString(v) + "\n\n"
            "    expected   " + greekName + ": "
            + DoubleFormatter::toString(expected) + "\n"
            "    calculated " + greekName + ": "
            + DoubleFormatter::toString(calculated) + "\n"
            "    error:            "
            + DoubleFormatter::toString(error) + "\n"
            "    tolerance:        "
            + DoubleFormatter::toString(tolerance));
    }

    struct VanillaOptionData {
        Option::Type type;
        double strike;
        double s;      // spot
        double q;      // dividend
        double r;      // risk-free rate
        Time t;        // time to maturity
        double v;      // volatility
        double result; // result
        double tol;    // tolerance
    };

}



// tests


void AmericanOptionTest::testBaroneAdesiWhaleyValues() {

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
       pag 24

       The following values were replicated only up to the second digit
       by the VB code provided by Haug, which was used as base for the
       C++ implementation

    */
    VanillaOptionData values[] = {
        //        type, strike,   spot,    q,    r,    t,  vol,   value, tol
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.15,  0.0206, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8771, 1e-2 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.15, 10.0089, 1e-2 },
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.25,  0.3159, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1280, 1e-2 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.25, 10.3919, 1e-2 },
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.35,  0.9495, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3777, 1e-2 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.35, 11.1679, 1e-2 },
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.15,  0.8208, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0842, 1e-2 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.15, 10.8087, 1e-2 },
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.25,  2.7437, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.8015, 1e-2 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.25, 13.0170, 1e-2 },
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.35,  5.0063, 1e-2 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.5106, 1e-2 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.35, 15.5689, 1e-2 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.15, 10.0000, 1e-2 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8770, 1e-2 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.15,  0.0410, 1e-2 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.25, 10.2533, 1e-2 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1277, 1e-2 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.25,  0.4562, 1e-2 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.35, 10.8787, 1e-2 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3777, 1e-2 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.35,  1.2402, 1e-2 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.15, 10.5595, 1e-2 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0842, 1e-2 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.15,  1.0822, 1e-2 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.25, 12.4419, 1e-2 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.8014, 1e-2 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.25,  3.3226, 1e-2 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.35, 14.6945, 1e-2 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.5104, 1e-2 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.35,  5.8823, 1e-2 }
    };



    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot(new SimpleQuote(0.0));
    Handle<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);
    Handle<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol, dc);
    Handle<PricingEngine> engine(new BaroneAdesiWhaleyApproximationEngine);

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> exercise(new AmericanExercise(today, exDate));

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

        VanillaOption option(stochProcess, payoff, exercise,
                             engine);

        double calculated = option.NPV();
        double error = QL_FABS(calculated-values[i].result);
        if (error > values[i].tol) {
            vanillaOptionTestFailed("value", payoff, exercise, values[i].s, values[i].q,
                values[i].r, today, dc, values[i].v, values[i].result, calculated,
                error, values[i].tol);
        }
    }

}


void AmericanOptionTest::testBjerksundStenslandValues() {

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
       pag 27
    */
    VanillaOptionData values[] = {
          //      type, strike,   spot,    q,    r,    t,  vol,   value, tol
        { Option::Call,  40.00,  42.00, 0.08, 0.04, 0.75, 0.35,  5.2704, 1e-4 }
    };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot(new SimpleQuote(0.0));
    Handle<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);
    Date today = Date::todaysDate();
    Handle<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol, dc);
    Handle<PricingEngine> engine(new BjerksundStenslandApproximationEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> exercise(new AmericanExercise(today, exDate));

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

        VanillaOption option(stochProcess, payoff, exercise,
                             engine);

        double calculated = option.NPV();
        double error = QL_FABS(calculated-values[i].result);
        if (error > values[i].tol) {
            vanillaOptionTestFailed("value", payoff, exercise, values[i].s, values[i].q,
                values[i].r, today, dc, values[i].v, values[i].result, calculated,
                error, values[i].tol);
        }
    }

}


CppUnit::Test* AmericanOptionTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("American option tests");

    tests->addTest(new CppUnit::TestCaller<AmericanOptionTest>
        ("Testing Bjerksund and Stensland approximation for American options",
        &AmericanOptionTest::testBjerksundStenslandValues));
    tests->addTest(new CppUnit::TestCaller<AmericanOptionTest>
        ("Testing Barone-Adesi and Whiley approximation for American options",
        &AmericanOptionTest::testBaroneAdesiWhaleyValues));

    return tests;
}

