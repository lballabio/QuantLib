
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
#include <ql/Instruments/vanillaoption.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/DayCounters/Actual360.hpp>
#include <ql/PricingEngines/Vanilla/vanillaengines.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <map>

// This makes it easier to use array literals (alas, no std::vector literals)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace QuantLib;

namespace {

    // utilities
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

    void check(std::string greekName,
               Option::Type type,
               double s,
               double strike,
               double q,
               double r,
               Date today,
               Date exDate,
               double v,
               double result,
               double calculated,
               double tol) {
        if (QL_FABS(calculated-result) > tol) {
          CPPUNIT_FAIL(greekName + " of European "
              + OptionTypeFormatter::toString(type) +
              " option :\n"
              "    underlying value: "
              + DoubleFormatter::toString(s) + "\n"
              "    strike:           "
              + DoubleFormatter::toString(strike) +"\n"
              "    dividend yield:   "
              + DoubleFormatter::toString(q) + "\n"
              "    risk-free rate:   "
              + DoubleFormatter::toString(r) + "\n"
              "    reference date:   "
              + DateFormatter::toString(today) + "\n"
              "    maturity:         "
              + DateFormatter::toString(exDate) + "\n"
              "    volatility:       "
              + DoubleFormatter::toString(v) + "\n\n"
              "    tabulated value:  "
              + DoubleFormatter::toString(result) + "\n"
              "    result:  "
              + DoubleFormatter::toString(calculated));
        }
    }



}





// tests


void AmericanOptionTest::testBaroneAdesiWhaleyValues() {

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
    Handle<TermStructure> divCurve = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rfCurve = makeFlatCurve(rRate, dc);
    Date today = Date::todaysDate();
    Handle<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volCurve = makeFlatVolatility(vol, dc);
    Handle<PricingEngine> engine(new BaroneAdesiWhaleyApproximationEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));
        Date exDate = today.plusDays(values[i].t*360);
        Handle<Exercise> exercise(new AmericanExercise(today, exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);
        VanillaOption option(payoff, exercise,
                             RelinkableHandle<Quote>(spot),
                             RelinkableHandle<TermStructure>(divCurve),
                             RelinkableHandle<TermStructure>(rfCurve),
                             RelinkableHandle<BlackVolTermStructure>(volCurve),
                             engine);
        double value = option.NPV();

        if (QL_FABS(value-values[i].result) > values[i].tol) {
          CPPUNIT_FAIL("European "
              + OptionTypeFormatter::toString(values[i].type) +
              " option :\n"
              "    underlying value: "
              + DoubleFormatter::toString(values[i].s) + "\n"
              "    strike:           "
              + DoubleFormatter::toString(values[i].strike) +"\n"
              "    dividend yield:   "
              + DoubleFormatter::toString(values[i].q) + "\n"
              "    risk-free rate:   "
              + DoubleFormatter::toString(values[i].r) + "\n"
              "    reference date:   "
              + DateFormatter::toString(today) + "\n"
              "    maturity:         "
              + DateFormatter::toString(exDate) + "\n"
              "    volatility:       "
              + DoubleFormatter::toString(values[i].v) + "\n\n"
              "    tabulated value:  "
              + DoubleFormatter::toString(values[i].result, 4) + "\n"
              "    result:           "
              + DoubleFormatter::toString(value, 4) + "\n"
              "    error:            "
              + DoubleFormatter::toString(QL_FABS(value-values[i].result), 4) + "\n"
              "    tolerance:        "
              + DoubleFormatter::toString(values[i].tol, 4));
        }
    }

}


void AmericanOptionTest::testBjerksundStenslandValues() {

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
    Handle<TermStructure> divCurve = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rfCurve = makeFlatCurve(rRate, dc);
    Date today = Date::todaysDate();
    Handle<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volCurve = makeFlatVolatility(vol, dc);
    Handle<PricingEngine> engine(new BjerksundStenslandApproximationEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));
        Date exDate = today.plusDays(values[i].t*360);
        Handle<Exercise> exercise(new AmericanExercise(today, exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);
        VanillaOption option(payoff, exercise,
                             RelinkableHandle<Quote>(spot),
                             RelinkableHandle<TermStructure>(divCurve),
                             RelinkableHandle<TermStructure>(rfCurve),
                             RelinkableHandle<BlackVolTermStructure>(volCurve),
                             engine);
        double value = option.NPV();

        if (QL_FABS(value-values[i].result) > values[i].tol) {
          CPPUNIT_FAIL("European "
              + OptionTypeFormatter::toString(values[i].type) +
              " option :\n"
              "    underlying value: "
              + DoubleFormatter::toString(values[i].s) + "\n"
              "    strike:           "
              + DoubleFormatter::toString(values[i].strike) +"\n"
              "    dividend yield:   "
              + DoubleFormatter::toString(values[i].q) + "\n"
              "    risk-free rate:   "
              + DoubleFormatter::toString(values[i].r) + "\n"
              "    reference date:   "
              + DateFormatter::toString(today) + "\n"
              "    maturity:         "
              + DateFormatter::toString(exDate) + "\n"
              "    volatility:       "
              + DoubleFormatter::toString(values[i].v) + "\n\n"
              "    tabulated value:  "
              + DoubleFormatter::toString(values[i].result, 4) + "\n"
              "    result:           "
              + DoubleFormatter::toString(value, 4) + "\n"
              "    error:            "
              + DoubleFormatter::toString(QL_FABS(value-values[i].result), 4) + "\n"
              "    tolerance:        "
              + DoubleFormatter::toString(values[i].tol, 4));
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

