
/*
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

#include "barrieroption.hpp"
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
#include <ql/Instruments/barrieroption.hpp>
#include <ql/PricingEngines/Barrier/barrierengines.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

// This makes it easier to use array literals (alas, no std::vector literals)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace QuantLib;

namespace {

    Handle<TermStructure> makeFlatCurve(const Handle<Quote>& forward) {
        Date today = Date::todaysDate();
        Calendar calendar = NullCalendar();
        //Date reference = calendar.advance(today,2,Days);        
        Date reference = today;
        return Handle<TermStructure>(
            new FlatForward(today,reference,
                            RelinkableHandle<Quote>(forward),
                            SimpleDayCounter()));
    }

    Handle<BlackVolTermStructure> makeFlatVolatility(
                                     const Handle<Quote>& volatility) {
        Date today = Date::todaysDate();
        Calendar calendar = NullCalendar();
        //Date reference = calendar.advance(today,2,Days);        
        Date reference = today;
        return Handle<BlackVolTermStructure>(
            new BlackConstantVol(reference,
                                 RelinkableHandle<Quote>(volatility),
                                 SimpleDayCounter()));
    }


    struct BarrierOptionData {
        Barrier::Type type;
        double volatility;
        double strike;
        double barrier;
        double callValue;
        double putValue;
    };

}

void BarrierOptionTest::testHaugValues() {

    double maxErrorAllowed = 1.0e-4;
    double maxStraddleErrorAllowed = 5.0e-2;

    double underlyingPrice = 100.0;
    double rebate = 3.0;
    Rate r = 0.08;
    Rate q = 0.04;
    

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
       pag. 72 */
    BarrierOptionData values[] = {
        { Barrier::DownOut, 0.25,    90,      95,  9.0246,  2.2798 },
        { Barrier::DownOut, 0.25,   100,      95,  6.7924,  2.2947 },
        { Barrier::DownOut, 0.25,   110,      95,  4.8759,  2.6252 },
        { Barrier::DownOut, 0.25,    90,     100,  3.0000,  3.0000 },
        { Barrier::DownOut, 0.25,   100,     100,  3.0000,  3.0000 },
        { Barrier::DownOut, 0.25,   110,     100,  3.0000,  3.0000 },
        { Barrier::UpOut,   0.25,    90,     105,  2.6789,  3.7760 },
        { Barrier::UpOut,   0.25,   100,     105,  2.3580,  5.4932 },
        { Barrier::UpOut,   0.25,   110,     105,  2.3453,  7.5187 },

        { Barrier::DownIn,  0.25,    90,      95,  7.7627,  2.9586 },
        { Barrier::DownIn,  0.25,   100,      95,  4.0109,  6.5677 },
        { Barrier::DownIn,  0.25,   110,      95,  2.0576, 11.9752 },
        { Barrier::DownIn,  0.25,    90,     100, 13.8333,  2.2845 },
        { Barrier::DownIn,  0.25,   100,     100,  7.8494,  5.9085 },
        { Barrier::DownIn,  0.25,   110,     100,  3.9795, 11.6465 },
        { Barrier::UpIn,    0.25,    90,     105, 14.1112,  1.4653 },
        { Barrier::UpIn,    0.25,   100,     105,  8.4482,  3.3721 },
        { Barrier::UpIn,    0.25,   110,     105,  4.5910,  7.0846 },

        { Barrier::DownOut, 0.30,    90,      95,  8.8334,  2.4170 },
        { Barrier::DownOut, 0.30,   100,      95,  7.0285,  2.4258 },
        { Barrier::DownOut, 0.30,   110,      95,  5.4137,  2.6246 },
        { Barrier::DownOut, 0.30,    90,     100,  3.0000,  3.0000 },
        { Barrier::DownOut, 0.30,   100,     100,  3.0000,  3.0000 },
        { Barrier::DownOut, 0.30,   110,     100,  3.0000,  3.0000 },
        { Barrier::UpOut,   0.30,    90,     105,  2.6341,  4.2293 },
        { Barrier::UpOut,   0.30,   100,     105,  2.4389,  5.8032 },
        { Barrier::UpOut,   0.30,   110,     105,  2.4315,  7.5649 },

        { Barrier::DownIn,  0.30,    90,      95,  9.0093,  3.8769 },
        { Barrier::DownIn,  0.30,   100,      95,  5.1370,  7.7989 },
        { Barrier::DownIn,  0.30,   110,      95,  2.8517, 13.3078 },
        { Barrier::DownIn,  0.30,    90,     100, 14.8816,  3.3328 },
        { Barrier::DownIn,  0.30,   100,     100,  9.2045,  7.2636 },
        { Barrier::DownIn,  0.30,   110,     100,  5.3043, 12.9713 },
        { Barrier::UpIn,    0.30,    90,     105, 15.2098,  2.0658 },
        { Barrier::UpIn,    0.30,   100,     105,  9.7278,  4.4226 },
        { Barrier::UpIn,    0.30,   110,     105,  5.8350,  8.3686 }
    };


    Handle<SimpleQuote> underlying(new SimpleQuote(underlyingPrice));
    Handle<SimpleQuote> qH_SME(new SimpleQuote(q));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);
    Handle<SimpleQuote> rH_SME(new SimpleQuote(r));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);

    Handle<SimpleQuote> volatility(new SimpleQuote(0.25));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatility);

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,6,Months);
    EuropeanExercise exercise(exDate);

    for (Size i=0; i<LENGTH(values); i++) {
        volatility->setValue(values[i].volatility);

        Handle<StrikedTypePayoff> callPayoff(new
            PlainVanillaPayoff(Option::Call, values[i].strike));

        BarrierOption barrierCallOption(
                values[i].type, 
                values[i].barrier, 
                rebate, 
                callPayoff, 
                RelinkableHandle<Quote>(underlying), 
                RelinkableHandle<TermStructure>(qTS), 
                RelinkableHandle<TermStructure>(rTS),
                exercise, 
                RelinkableHandle<BlackVolTermStructure>(volTS));
        double calculated = barrierCallOption.NPV();
        double expected = values[i].callValue;
        if (QL_FABS(calculated-expected) > maxErrorAllowed) {
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Call option:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }

        Handle<StrikedTypePayoff> putPayoff(new
            PlainVanillaPayoff(Option::Put, values[i].strike));

        BarrierOption barrierPutOption(
                values[i].type, 
                values[i].barrier, 
                rebate, 
                putPayoff, 
                RelinkableHandle<Quote>(underlying), 
                RelinkableHandle<TermStructure>(qTS), 
                RelinkableHandle<TermStructure>(rTS),
                exercise, 
                RelinkableHandle<BlackVolTermStructure>(volTS));
        calculated = barrierPutOption.NPV();
        expected = values[i].putValue;
        if (QL_FABS(calculated-expected) > maxErrorAllowed) {
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Put option:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }

        Handle<StrikedTypePayoff> straddlePayoff(new
            PlainVanillaPayoff(Option::Straddle, values[i].strike));

        BarrierOption barrierStraddleOption(
                values[i].type, 
                values[i].barrier, 
                rebate, 
                straddlePayoff, 
                RelinkableHandle<Quote>(underlying), 
                RelinkableHandle<TermStructure>(qTS), 
                RelinkableHandle<TermStructure>(rTS),
                exercise, 
                RelinkableHandle<BlackVolTermStructure>(volTS));
        calculated = barrierStraddleOption.NPV();
        expected = values[i].callValue+values[i].putValue;
        if (QL_FABS(calculated-expected) > maxStraddleErrorAllowed) {
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Straddle option:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }
    }
}

void BarrierOptionTest::testBabsiriValues() {

    double maxErrorAllowed = 1.0e-5;
    double maxMCErrorAllowed = 1.0e-1;

    double underlyingPrice = 100.0;
    double rebate = 0.0;
    Rate r = 0.05;
    Rate q = 0.02;

    Size timeSteps = 1;
    bool antitheticVariate = false;
    bool controlVariate = false;
    Size requiredSamples = 10000;
    double requiredTolerance = 0.02;
    Size maxSamples = 1000000; 
    bool isBiased = false;

    /*
        Data from 
        "Simulating Path-Dependent Options: A New Approach"
          - M. El Babsiri and G. Noel
            Journal of Derivatives; Winter 1998; 6, 2
    */
    BarrierOptionData values[] = {
        { Barrier::DownIn, 0.10,   100,       90,   0.07187,  0.0 },
        { Barrier::DownIn, 0.15,   100,       90,   0.60638,  0.0 },
        { Barrier::DownIn, 0.20,   100,       90,   1.64005,  0.0 },
        { Barrier::DownIn, 0.25,   100,       90,   2.98495,  0.0 },
        { Barrier::DownIn, 0.30,   100,       90,   4.50952,  0.0 },
        { Barrier::UpIn,   0.10,   100,      110,   4.79148,  0.0 },
        { Barrier::UpIn,   0.15,   100,      110,   7.08268,  0.0 },
        { Barrier::UpIn,   0.20,   100,      110,   9.11008,  0.0 },
        { Barrier::UpIn,   0.25,   100,      110,  11.06148,  0.0 },
        { Barrier::UpIn,   0.30,   100,      110,  12.98351,  0.0 }
    };

    Handle<SimpleQuote> underlying(new SimpleQuote(underlyingPrice));

    Handle<SimpleQuote> qH_SME(new SimpleQuote(q));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);

    Handle<SimpleQuote> rH_SME(new SimpleQuote(r));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);

    Handle<SimpleQuote> volatility(new SimpleQuote(0.10));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatility);

    Handle<PricingEngine> engine(new AnalyticBarrierEngine);
    Handle<PricingEngine> mcEngine(
        new MCBarrierEngine<PseudoRandom>(timeSteps, antitheticVariate,
                                          controlVariate, requiredSamples,
                                          requiredTolerance, maxSamples, 
                                          isBiased, 5));

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,1,Years);
    EuropeanExercise exercise(exDate);

    for (Size i=0; i<LENGTH(values); i++) {
        volatility->setValue(values[i].volatility);

        Handle<StrikedTypePayoff> callPayoff(new
            PlainVanillaPayoff(Option::Call, values[i].strike));

        // analytic
        BarrierOption barrierCallOption(
                values[i].type, 
                values[i].barrier, 
                rebate, 
                callPayoff, 
                RelinkableHandle<Quote>(underlying), 
                RelinkableHandle<TermStructure>(qTS), 
                RelinkableHandle<TermStructure>(rTS),
                exercise, 
                RelinkableHandle<BlackVolTermStructure>(volTS), 
                engine);
        double calculated = barrierCallOption.NPV();
        double expected = values[i].callValue;
        if (QL_FABS(calculated-expected) > maxErrorAllowed) {
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Barrier call option:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }

        barrierCallOption.setPricingEngine(mcEngine);
        calculated = barrierCallOption.NPV();
        if (QL_FABS(calculated-expected) > maxMCErrorAllowed) {
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Barrier call option MC:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }

    }
}

void BarrierOptionTest::testBeagleholeValues() {

    double maxErrorAllowed = 1.0e-3;
    double maxMCErrorAllowed = 1.5e-1;

    double underlyingPrice = 50.0;
    double rebate = 0.0;
    Rate r = QL_LOG (1.1);
    Rate q = 0.00;

    Size timeSteps = 1;
    bool antitheticVariate = false;
    bool controlVariate = false;
    Size requiredSamples = 10000;
    double requiredTolerance = 0.02;
    Size maxSamples = 1000000; 
    bool isBiased = false;

    /*
        Data from 
        "Going to Extreme: Correcting Simulation Bias in Exotic 
         Option Valuation"
          - D.R. Beaglehole, P.H. Dybvig and G. Zhou
            Financial Analysts Journal; Jan / Feb 1997; 53, 1
    */
    BarrierOptionData values[] = {
        { Barrier::DownOut, 0.50,   50,      45,  5.477,  0.0 }
    };

    Handle<SimpleQuote> underlying(new SimpleQuote(underlyingPrice));

    Handle<SimpleQuote> qH_SME(new SimpleQuote(q));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);

    Handle<SimpleQuote> rH_SME(new SimpleQuote(r));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);

    Handle<SimpleQuote> volatility(new SimpleQuote(0.10));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatility);

    Handle<PricingEngine> engine(new AnalyticBarrierEngine);
    Handle<PricingEngine> mcEngine(
        new MCBarrierEngine<PseudoRandom>(timeSteps, antitheticVariate, 
                                          controlVariate, requiredSamples,
                                          requiredTolerance, maxSamples, 
                                          isBiased, 10));

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,1,Years);
    EuropeanExercise exercise(exDate);

    for (Size i=0; i<LENGTH(values); i++) {
        volatility->setValue(values[i].volatility);

        Handle<StrikedTypePayoff> callPayoff(new
            PlainVanillaPayoff(Option::Call, values[i].strike));

        // analytic
        BarrierOption barrierCallOption(
                values[i].type, 
                values[i].barrier, 
                rebate, 
                callPayoff, 
                RelinkableHandle<Quote>(underlying), 
                RelinkableHandle<TermStructure>(qTS), 
                RelinkableHandle<TermStructure>(rTS),
                exercise, 
                RelinkableHandle<BlackVolTermStructure>(volTS), 
                engine);
        double calculated = barrierCallOption.NPV();
        double expected = values[i].callValue;
        if (QL_FABS(calculated-expected) > maxErrorAllowed) {
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Barrier call option:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }

        barrierCallOption.setPricingEngine(mcEngine);
        calculated = barrierCallOption.NPV();
        if (QL_FABS(calculated-expected) > maxMCErrorAllowed) {
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Barrier call option MC:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }
    }
}

CppUnit::Test* BarrierOptionTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Barrier option tests");
    tests->addTest(new CppUnit::TestCaller<BarrierOptionTest>
                   ("Testing barrier options against Haug's values",
                    &BarrierOptionTest::testHaugValues));
    tests->addTest(new CppUnit::TestCaller<BarrierOptionTest>
                   ("Testing barrier options against Babsiri's values",
                    &BarrierOptionTest::testBabsiriValues));
    tests->addTest(new CppUnit::TestCaller<BarrierOptionTest>
                   ("Testing barrier options against Beaglehole's values",
                   &BarrierOptionTest::testBeagleholeValues));
    return tests;
}

