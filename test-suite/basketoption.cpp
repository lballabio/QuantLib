
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004 Neil Firth
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

#include "basketoption.hpp"
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/basketoption.hpp>
#include <ql/PricingEngines/Basket/stulzengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

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

    std::string basketTypeToString(BasketOption::BasketType basketType) {

        switch (basketType) {
        case BasketOption::Min:
            return "Basket::Min";
        case BasketOption::Max:
            return "Basket::Max";
        }

        throw Error("basketTypeToString : unknown basket option type");
    }

    void basketOptionTestFailed(std::string greekName,
                                BasketOption::BasketType basketType,
                                const Handle<StrikedTypePayoff>& payoff,
                                const Handle<Exercise>& exercise,
                                double s1,
                                double s2,
                                double q1,
                                double q2,
                                double r,
                                Date today,
                                DayCounter dc,
                                double v1,
                                double v2,
                                double rho,
                                double expected,
                                double calculated,
                                double error,
                                double tolerance) {

        Time t = dc.yearFraction(today, exercise->lastDate());

        CPPUNIT_FAIL(exerciseTypeToString(exercise) + " "
            + OptionTypeFormatter::toString(payoff->optionType()) +
            " option on "
            + basketTypeToString(basketType) +
            " with "
            + payoffTypeToString(payoff) + ":\n"
            "1st underlying value: "
            + DoubleFormatter::toString(s1) + "\n"
            "2nd underlying value: "
            + DoubleFormatter::toString(s2) + "\n"
            "              strike: "
            + DoubleFormatter::toString(payoff->strike()) +"\n"
            "  1st dividend yield: "
            + DoubleFormatter::toString(q1) + "\n"
            "  2nd dividend yield: "
            + DoubleFormatter::toString(q2) + "\n"
            "      risk-free rate: "
            + DoubleFormatter::toString(r) + "\n"
            "      reference date: "
            + DateFormatter::toString(today) + "\n"
            "            maturity: "
            + DateFormatter::toString(exercise->lastDate()) + "\n"
            "      time to expiry: "
            + DoubleFormatter::toString(t) + "\n"
            "1st asset volatility: "
            + DoubleFormatter::toString(v1) + "\n"
            "2nd asset volatility: "
            + DoubleFormatter::toString(v2) + "\n"
            "         correlation: "
            + DoubleFormatter::toString(rho) + "\n\n"
            "    expected   " + greekName + ": "
            + DoubleFormatter::toString(expected) + "\n"
            "    calculated " + greekName + ": "
            + DoubleFormatter::toString(calculated) + "\n"
            "    error:            "
            + DoubleFormatter::toString(error) + "\n"
            "    tolerance:        "
            + DoubleFormatter::toString(tolerance));
    }

    struct BasketOptionData {
        BasketOption::BasketType basketType;
        Option::Type type;
        double strike;
        double s1;
        double s2;
        double q1;
        double q2;
        double r;
        Time t; // years
        double v1;
        double v2;
        double rho;
        double result;
        double tol;
    };

}

void BasketOptionTest::testValues() {

    /*
        Data from:
        Excel spreadsheet www.maths.ox.ac.uk/~firth/computing/excel.shtml
        and
        "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag 56-58
    */
    BasketOptionData values[] = {
        //      basketType,   optionType, strike,    s1,    s2,   q1,   q2,    r,    t,   v1,   v2,  rho, result, tol
        // data from http://www.maths.ox.ac.uk/~firth/computing/excel.shtml
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 10.898, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70,  8.483, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50,  6.844, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  5.531, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10,  4.413, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.70, 0.00,  4.981, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.30, 0.00,  4.159, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.10, 0.00,  2.597, 1.0e-3},
        {BasketOption::Min, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.10, 0.50,  4.030, 1.0e-3},

        {BasketOption::Max, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 17.565, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70, 19.980, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50, 21.619, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 22.932, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10, 24.049, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0,  80.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 16.508, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0,  80.0,  80.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  8.049, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0,  80.0, 120.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 30.141, 1.0e-3},
        {BasketOption::Max, Option::Call,  100.0, 120.0, 120.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 42.889, 1.0e-3},

        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 11.369, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70, 12.856, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50, 13.890, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 14.741, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10, 15.485, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 0.50, 0.30, 0.30, 0.10, 11.893, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 0.25, 0.30, 0.30, 0.10,  8.881, 1.0e-3},
        {BasketOption::Min,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 2.00, 0.30, 0.30, 0.10, 19.268, 1.0e-3},

        {BasketOption::Max,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90,  7.339, 1.0e-3},
        {BasketOption::Max,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70,  5.853, 1.0e-3},
        {BasketOption::Max,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50,  4.818, 1.0e-3},
        {BasketOption::Max,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  3.967, 1.0e-3},
        {BasketOption::Max,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10,  3.223, 1.0e-3},

        //      basketType,   optionType, strike,    s1,    s2,   q1,   q2,    r,    t,   v1,   v2,  rho,  result, tol
        // data from "Option pricing formulas" VB code + spreadsheet 
        {BasketOption::Min, Option::Call,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  4.8177, 1.0e-4},
        {BasketOption::Max, Option::Call,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63, 11.6323, 1.0e-4},
        {BasketOption::Min,  Option::Put,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  2.0376, 1.0e-4},
        {BasketOption::Max,  Option::Put,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  0.5731, 1.0e-4},
        {BasketOption::Min, Option::Call,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  2.9340, 1.0e-4},
        {BasketOption::Min,  Option::Put,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  3.5224, 1.0e-4},
        // data from "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag 58
        {BasketOption::Max, Option::Call,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  8.0701, 1.0e-4},
        {BasketOption::Max,  Option::Put,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  1.2181, 1.0e-4}
    };

    DayCounter dc = Actual360();
    Handle<SimpleQuote> spot1(new SimpleQuote(0.0));
    Handle<SimpleQuote> spot2(new SimpleQuote(0.0));

    Handle<SimpleQuote> qRate1(new SimpleQuote(0.0));
    Handle<TermStructure> qTS1 = makeFlatCurve(qRate1, dc);
    Handle<SimpleQuote> qRate2(new SimpleQuote(0.0));
    Handle<TermStructure> qTS2 = makeFlatCurve(qRate2, dc);

    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);

    Handle<SimpleQuote> vol1(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS1 = makeFlatVolatility(vol1, dc);
    Handle<SimpleQuote> vol2(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS2 = makeFlatVolatility(vol2, dc);


    Handle<PricingEngine> engine(new StulzEngine);

    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<PlainVanillaPayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> exercise(new EuropeanExercise(exDate));

        spot1 ->setValue(values[i].s1);
        spot2 ->setValue(values[i].s2);
        qRate1->setValue(values[i].q1);
        qRate2->setValue(values[i].q2);
        rRate ->setValue(values[i].r );
        vol1  ->setValue(values[i].v1);
        vol2  ->setValue(values[i].v2);

        Handle<BlackScholesStochasticProcess> stochProcess1(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot1),
                RelinkableHandle<TermStructure>(qTS1),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS1)));

        Handle<BlackScholesStochasticProcess> stochProcess2(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(spot2),
                RelinkableHandle<TermStructure>(qTS2),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS2)));

        std::vector<Handle<BlackScholesStochasticProcess> > procs =
            std::vector<Handle<BlackScholesStochasticProcess> >();
        procs.push_back(stochProcess1);
        procs.push_back(stochProcess2);


        BasketOption basketOption(values[i].basketType, procs, payoff, 
                                  exercise, values[i].rho, engine);

        double calculated = basketOption.NPV();
        double expected = values[i].result;
        double error = QL_FABS(calculated-expected);
        if (error > values[i].tol) {
            basketOptionTestFailed("value",
                values[i].basketType, payoff, exercise, values[i].s1, values[i].s2,
                values[i].q1, values[i].q2, values[i].r,
                today, dc, values[i].v1, values[i].v2, values[i].rho,
                values[i].result, calculated, error, values[i].tol);
        }

    }
}

CppUnit::Test* BasketOptionTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Basket option tests");
    tests->addTest(new CppUnit::TestCaller<BasketOptionTest>
                   ("Testing basket options against correct values",
                    &BasketOptionTest::testValues));

    return tests;
}

