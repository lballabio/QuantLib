
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

#include "asianoptions.hpp"
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/asianoption.hpp>
#include <ql/PricingEngines/Asian/asianengines.hpp>
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

    std::string averageTypeToString(Average::Type averageType) {

        if (averageType == Average::Geometric)
            return "Geometric Averaging";
        else if (averageType == Average::Arithmetic)
            return "Arithmetic Averaging";
        else
            throw Error("unknown averaging");
    }

    void asianOptionTestFailed(
        std::string greekName,
        Average::Type averageType,
        double runningProduct,
        Size pastFixings,
        std::vector<Date> fixingDates,
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
            " asian option with "
            + averageTypeToString(averageType) + " and "
            + payoffTypeToString(payoff) + ":\n"
            "    running product:  "
            + DoubleFormatter::toString(runningProduct) + "\n"
            "    past fixings:     "
            + IntegerFormatter::toString(pastFixings) + "\n"
            "    future fixings:   "
            + IntegerFormatter::toString(fixingDates.size()) + "\n"
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
            + (tolerance==Null<double>() ? "" :
            "    tolerance:        " + DoubleFormatter::toString(tolerance)));
    }

    struct AsianOptionData {
        Average::Type averageType;
        double runningProduct;
        Size pastFixings;
        Size futureFixings;
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

void AsianOptionTest::testGeometricDiscreteAverage() {

    // data from "Implementing Derivatives Model",
    // Clewlow, Strickland, p.118-123
    AsianOptionData values[] = {
        {Average::Geometric, 1.0, 0, 10, Option::Call, 100.0, 100.0, 0.03, 0.06, 1.0, 0.20, 5.3425606635, 1e-10}
    };

    Date today = Date::todaysDate();
    std::vector<Date> fixingDates(values[0].futureFixings);
    Size dt = Size(values[0].t*360/values[0].futureFixings+0.5);
    fixingDates[0]=today.plusDays(dt);
    for (Size i=1; i<values[0].futureFixings; i++)
        fixingDates[i]=fixingDates[i-1].plusDays(dt);

    DayCounter dc = Actual360();

    Handle<SimpleQuote> underlying(new SimpleQuote(values[0].s));

    Handle<SimpleQuote> qRate(new SimpleQuote(values[0].q));
    Handle<TermStructure> divCurve(new FlatForward(today,today,
        RelinkableHandle<Quote>(qRate), dc));

    Handle<SimpleQuote> rRate(new SimpleQuote(values[0].r));
    Handle<TermStructure> rfCurve(new FlatForward(today,today,
        RelinkableHandle<Quote>(rRate), dc));

    Handle<SimpleQuote> volatility(new SimpleQuote(values[0].v));
    Handle<BlackVolTermStructure> volCurve(new BlackConstantVol(today,
        RelinkableHandle<Quote>(volatility), dc));

    Handle<BlackScholesStochasticProcess> stochProcess(new
        BlackScholesStochasticProcess(
            RelinkableHandle<Quote>(underlying),
            RelinkableHandle<TermStructure>(divCurve),
            RelinkableHandle<TermStructure>(rfCurve),
            RelinkableHandle<BlackVolTermStructure>(volCurve)));

    Handle<StrikedTypePayoff> payoff(new
        PlainVanillaPayoff(values[0].type, values[0].strike));

    Date exDate = today.plusDays(360);
    Handle<Exercise> exercise(new EuropeanExercise(exDate));

    Handle<PricingEngine> engine(new AnalyticDiscreteAveragingAsianEngine);

    DiscreteAveragingAsianOption pricer(
        values[0].averageType,
        values[0].runningProduct,
        values[0].pastFixings,
        fixingDates,
        stochProcess,
        payoff,
        exercise,
        engine);

    if (QL_FABS(pricer.NPV()-values[0].result) > values[0].tol)
        CPPUNIT_FAIL(
            "Batch 1, case 1:\n"
            "    calculated value: "
            + DoubleFormatter::toString(pricer.NPV(),10) + "\n"
            "    expected:         "
            + DoubleFormatter::toString(values[0].result,10));
}


CppUnit::Test* AsianOptionTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Asian option tests");
    tests->addTest(new CppUnit::TestCaller<AsianOptionTest>
                   ("Testing discrete-averaging geometric Asian options",
                    &AsianOptionTest::testGeometricDiscreteAverage));
    return tests;
}

