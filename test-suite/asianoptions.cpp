
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
#include <ql/Instruments/asianoption.hpp>
#include <ql/PricingEngines/Asian/asianengines.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/Daycounters/actual360.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <map>

// This makes it easier to use array literals (alas, no std::vector literals)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace QuantLib;


void AsianOptionTest::testGeometricDiscreteAverage() {

    // data from "Implementing Derivatives Model",
    // Clewlow, Strickland, p.118-123
    Average::Type averageType = Average::Geometric;
    Option::Type type = Option::Call;

    Handle<SimpleQuote> underlying(new SimpleQuote(100.0));

    double strike = 100.0;
    double runningProduct = 1.0;
    Size pastFixings = 0;

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    DayCounter dc = Actual360();
    Date reference = calendar.advance(today,2,Days);

    std::vector<Date> fixingDates(10);
    fixingDates[0]=reference+36;
    for (Size i=1; i<10; i++) {
        fixingDates[i]=fixingDates[i-1]+36;
    }

    Handle<SimpleQuote> qRate(new SimpleQuote(  0.03));
    Handle<TermStructure> divCurve(new FlatForward(today,reference,
        RelinkableHandle<Quote>(qRate), dc));

    Handle<SimpleQuote> rRate(new SimpleQuote(  0.06));
    Handle<TermStructure> rfCurve(new FlatForward(today,reference,
        RelinkableHandle<Quote>(rRate), dc));

    Handle<PricingEngine> engine(new AnalyticDiscreteAveragingAsianEngine);
            
    Date exDate = reference+360;
    EuropeanExercise exercise(exDate);

    Handle<SimpleQuote> volatility(new SimpleQuote(  0.2));
    Handle<BlackVolTermStructure> volCurve(new BlackConstantVol(reference,
        RelinkableHandle<Quote>(volatility), dc));

    DiscreteAveragingAsianOption pricer(averageType, type,
        RelinkableHandle<Quote>(underlying), strike,
        runningProduct, pastFixings, fixingDates,
        RelinkableHandle<TermStructure>(divCurve),
        RelinkableHandle<TermStructure>(rfCurve),
        exercise, RelinkableHandle<BlackVolTermStructure>(volCurve),
        engine);

    double storedValue = 5.3425606635;
    if (QL_FABS(pricer.NPV()-storedValue) > 1.0e-10)
        CPPUNIT_FAIL(
            "Batch 1, case 1:\n"
            "    calculated value: "
            + DoubleFormatter::toString(pricer.NPV(),10) + "\n"
            "    expected:         "
            + DoubleFormatter::toString(storedValue,10));
}


CppUnit::Test* AsianOptionTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Asian option tests");
    tests->addTest(new CppUnit::TestCaller<AsianOptionTest>
                   ("Testing Discrete Averaging Geometric Asian options",
                    &AsianOptionTest::testGeometricDiscreteAverage));
    return tests;
}

