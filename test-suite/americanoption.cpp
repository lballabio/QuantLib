
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
        DayCounter dc = Actual365()) {
        Date today = Date::todaysDate();
        Date reference = today;
        return Handle<TermStructure>(
            new FlatForward(today,reference,
                            RelinkableHandle<Quote>(forward),
                            dc));
    }

    Handle<BlackVolTermStructure> makeFlatVolatility(
                                     const Handle<Quote>& volatility,
                                     DayCounter dc = Actual365()) {
        Date today = Date::todaysDate();
        Date reference = today;
        return Handle<BlackVolTermStructure>(
            new BlackConstantVol(reference,
                                 RelinkableHandle<Quote>(volatility),
                                 dc));
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


void AmericanOptionTest::testBAWValues() {

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
    */
    VanillaOptionData values[] = {
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.15,  0.0205, 1e-4 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8734, 1e-4 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.15,  9.9413, 1e-4 },
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.25,  0.3150, 1e-4 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1217, 1e-4 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.25, 10.3556, 1e-4 },
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.10, 0.35,  0.9474, 1e-4 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3693, 1e-4 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.10, 0.35, 11.1381, 1e-4 },
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.15,  0.8069, 1e-4 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0232, 1e-4 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.15, 10.5769, 1e-4 },
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.25,  2.7026, 1e-4 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.6997, 1e-4 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.25, 12.7857, 1e-4 },
        { Option::Call, 100.00,  90.00, 0.10, 0.10, 0.50, 0.35,  4.9329, 1e-4 },
        { Option::Call, 100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.3679, 1e-4 },
        { Option::Call, 100.00, 110.00, 0.10, 0.10, 0.50, 0.35, 15.3086, 1e-4 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.15,  9.9210, 1e-4 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.15,  1.8734, 1e-4 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.15,  0.0408, 1e-4 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.25, 10.2155, 1e-4 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.25,  3.1217, 1e-4 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.25,  0.4551, 1e-4 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.10, 0.35, 10.8479, 1e-4 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.10, 0.35,  4.3693, 1e-4 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.10, 0.35,  1.2376, 1e-4 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.15, 10.3192, 1e-4 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.15,  4.0232, 1e-4 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.15,  1.0646, 1e-4 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.25, 12.2149, 1e-4 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.25,  6.6997, 1e-4 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.25,  3.2734, 1e-4 },
        { Option::Put,  100.00,  90.00, 0.10, 0.10, 0.50, 0.35, 14.4452, 1e-4 },
        { Option::Put,  100.00, 100.00, 0.10, 0.10, 0.50, 0.35,  9.3679, 1e-4 },
        { Option::Put,  100.00, 110.00, 0.10, 0.10, 0.50, 0.35,  5.7963, 1e-4 }
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
              + DoubleFormatter::toString(values[i].result) + "\n"
              "    result:  "
              + DoubleFormatter::toString(value));
        }
    }

}


CppUnit::Test* AmericanOptionTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("American option tests");

    tests->addTest(new CppUnit::TestCaller<AmericanOptionTest>
        ("Testing Barone-Adesi and Whiley approximation for American options",
        &AmericanOptionTest::testBAWValues));
    return tests;
}

