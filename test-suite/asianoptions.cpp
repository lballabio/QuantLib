
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
#include "utilities.hpp"
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/asianoption.hpp>
#include <ql/PricingEngines/Asian/analyticasianengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <map>

using namespace QuantLib;

namespace {

    std::string averageTypeToString(Average::Type averageType) {

        if (averageType == Average::Geometric)
            return "Geometric Averaging";
        else if (averageType == Average::Arithmetic)
            return "Arithmetic Averaging";
        else
            QL_FAIL("unknown averaging");
    }

    void asianOptionTestFailed(std::string greekName,
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
                               DayCounter dc,
                               double v,
                               double expected,
                               double calculated,
                               double tolerance = Null<double>()) {

        Time t = dc.yearFraction(today, exercise->lastDate());

        CPPUNIT_FAIL(exerciseTypeToString(exercise) + " "
            " asian option with "
            + averageTypeToString(averageType) + " and "
            + payoffTypeToString(payoff) + " payoff:\n"
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
            "    time to expiry:   "
            + DoubleFormatter::toString(t) + "\n"
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

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    Handle<SimpleQuote> spot(new SimpleQuote(0.0));
    Handle<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qRate, dc);
    Handle<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rRate, dc);
    Handle<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(vol, dc);

    Handle<BlackScholesStochasticProcess> stochProcess(new
        BlackScholesStochasticProcess(
            RelinkableHandle<Quote>(spot),
            RelinkableHandle<TermStructure>(qTS),
            RelinkableHandle<TermStructure>(rTS),
            RelinkableHandle<BlackVolTermStructure>(volTS)));

    Handle<PricingEngine> engine(new AnalyticDiscreteAveragingAsianEngine);

    for (Size i=0; i<LENGTH(values); i++) {

        Handle<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today.plusDays(int(values[i].t*360+0.5));
        Handle<Exercise> exercise(new EuropeanExercise(exDate));

        std::vector<Date> fixingDates(values[i].futureFixings);
        Size dt = Size(values[i].t*360/values[i].futureFixings+0.5);
        fixingDates[0]=today.plusDays(dt);
        for (Size j=1; j<values[i].futureFixings; j++)
            fixingDates[j]=fixingDates[j-1].plusDays(dt);

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        DiscreteAveragingAsianOption pricer(
            values[i].averageType,
            values[i].runningProduct,
            values[i].pastFixings,
            fixingDates,
            stochProcess,
            payoff,
            exercise,
            engine);

        double calculated = pricer.NPV();
        if (QL_FABS(calculated-values[i].result) > values[i].tol) {
            asianOptionTestFailed("value",
                                  values[i].averageType,
                                  values[i].runningProduct,
                                  values[i].pastFixings,
                                  fixingDates,
                                  payoff, exercise,
                                  values[i].s, values[i].q, values[i].r,
                                  today, dc,
                                  values[i].v,
                                  values[i].result, calculated,
                                  values[i].tol);
        }
    }
}


CppUnit::Test* AsianOptionTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Asian option tests");
    tests->addTest(new CppUnit::TestCaller<AsianOptionTest>
                   ("Testing discrete-averaging geometric Asian options",
                    &AsianOptionTest::testGeometricDiscreteAverage));
    return tests;
}

