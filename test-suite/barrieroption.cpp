
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "barrieroption.hpp"
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
#include <ql/Instruments/barrieroption.hpp>
#include <ql/PricingEngines/barrierengines.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
//#include <map>

// This makes it easier to use array literals (alas, no std::vector literals)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace QuantLib;
using namespace QuantLib::PricingEngines;
using namespace QuantLib::Instruments;
using namespace QuantLib::TermStructures;
using namespace QuantLib::VolTermStructures;
using namespace QuantLib::DayCounters;
using namespace QuantLib::Calendars;
using namespace QuantLib::MonteCarlo;

namespace {

    Handle<TermStructure> makeFlatCurve(const Handle<MarketElement>& forward) {
        Date today = Date::todaysDate();
        Calendar calendar = NullCalendar();
        //Date reference = calendar.advance(today,2,Days);        
        Date reference = today;        
        return Handle<TermStructure>(
            new FlatForward(today,reference,
                            RelinkableHandle<MarketElement>(forward),
                            SimpleDayCounter()));
    }

    Handle<BlackVolTermStructure> makeFlatVolatility(
                                     const Handle<MarketElement>& volatility) {
        Date today = Date::todaysDate();
        Calendar calendar = NullCalendar();
        //Date reference = calendar.advance(today,2,Days);        
        Date reference = today;        
        return Handle<BlackVolTermStructure>(
            new BlackConstantVol(reference,
                                 RelinkableHandle<MarketElement>(volatility),
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
    double maxMCErrorAllowed = 1.0e-1;
    double maxStraddleErrorAllowed = 5.0e-2;

    double underlyingPrice = 100.0;
    double rebate = 3.0;
    Rate r = 0.08;
    Rate q = 0.04;
    
    Size timeSteps = 5;
    bool antitheticVariate = false;
    bool controlVariate = false;
    Size requiredSamples = 10000;     
    double requiredTolerance = 0.02;
    Size maxSamples = 1000000; 
    bool isBiased = false;                    

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
       pag. 72 */
    BarrierOptionData values[] = {
        { Barrier::Type::DownOut, 0.25,    90,      95,  9.0246,  2.2798 },
        { Barrier::Type::DownOut, 0.25,   100,      95,  6.7924,  2.2947 },
        { Barrier::Type::DownOut, 0.25,   110,      95,  4.8759,  2.6252 },
//        { Barrier::Type::DownOut, 0.25,    90,     100,  3.0000,  3.0000 },
//        { Barrier::Type::DownOut, 0.25,   100,     100,  3.0000,  3.0000 },
//        { Barrier::Type::DownOut, 0.25,   110,     100,  3.0000,  3.0000 },
        { Barrier::Type::UpOut,   0.25,    90,     105,  2.6789,  3.7760 },
        { Barrier::Type::UpOut,   0.25,   100,     105,  2.3580,  5.4932 },
        { Barrier::Type::UpOut,   0.25,   110,     105,  2.3453,  7.5187 },

        { Barrier::Type::DownIn,  0.25,    90,      95,  7.7627,  2.9586 },
        { Barrier::Type::DownIn,  0.25,   100,      95,  4.0109,  6.5677 },
        { Barrier::Type::DownIn,  0.25,   110,      95,  2.0576, 11.9752 },
//        { Barrier::Type::DownIn,  0.25,    90,     100, 13.8333,  2.2845 },
//        { Barrier::Type::DownIn,  0.25,   100,     100,  7.8494,  5.9085 },
//        { Barrier::Type::DownIn,  0.25,   110,     100,  3.9795, 11.6465 },
        { Barrier::Type::UpIn,    0.25,    90,     105, 14.1112,  1.4653 },
        { Barrier::Type::UpIn,    0.25,   100,     105,  8.4482,  3.3721 },
        { Barrier::Type::UpIn,    0.25,   110,     105,  4.5910,  7.0846 },

        { Barrier::Type::DownOut, 0.30,    90,      95,  8.8334,  2.4170 },
        { Barrier::Type::DownOut, 0.30,   100,      95,  7.0285,  2.4258 },
        { Barrier::Type::DownOut, 0.30,   110,      95,  5.4137,  2.6246 },
//        { Barrier::Type::DownOut, 0.30,    90,     100,  3.0000,  3.0000 },
//        { Barrier::Type::DownOut, 0.30,   100,     100,  3.0000,  3.0000 },
//        { Barrier::Type::DownOut, 0.30,   110,     100,  3.0000,  3.0000 },
        { Barrier::Type::UpOut,   0.30,    90,     105,  2.6341,  4.2293 },
        { Barrier::Type::UpOut,   0.30,   100,     105,  2.4389,  5.8032 },
        { Barrier::Type::UpOut,   0.30,   110,     105,  2.4315,  7.5649 },

        { Barrier::Type::DownIn,  0.30,    90,      95,  9.0093,  3.8769 },
        { Barrier::Type::DownIn,  0.30,   100,      95,  5.1370,  7.7989 },
        { Barrier::Type::DownIn,  0.30,   110,      95,  2.8517, 13.3078 },
//        { Barrier::Type::DownIn,  0.30,    90,     100, 14.8816,  3.3328 },
//        { Barrier::Type::DownIn,  0.30,   100,     100,  9.2045,  7.2636 },
//        { Barrier::Type::DownIn,  0.30,   110,     100,  5.3043, 12.9713 },
        { Barrier::Type::UpIn,    0.30,    90,     105, 15.2098,  2.0658 },
        { Barrier::Type::UpIn,    0.30,   100,     105,  9.7278,  4.4226 },
        { Barrier::Type::UpIn,    0.30,   110,     105,  5.8350,  8.3686 }
    };


    Handle<SimpleMarketElement> underlyingH_SME(new SimpleMarketElement(underlyingPrice));
    Handle<SimpleMarketElement> qH_SME(new SimpleMarketElement(q));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);
    Handle<SimpleMarketElement> rH_SME(new SimpleMarketElement(r));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);

    Handle<SimpleMarketElement> volatilityH_SME(new SimpleMarketElement(0.25));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatilityH_SME);
    
    Handle<MarketElement> underlyingH = underlyingH_SME;
    //Handle<MarketElement> qH = qH_SME;
    //Handle<MarketElement> rH = rH_SME;

    // the date calculations are ignored by DayCounters::HalfYear
    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,6,Months);    
    EuropeanExercise exercise(exDate);
              
    //underlyingH_SME->setValue(underlyingPrice);
    //qH_SME->setValue(q);
    //rH_SME->setValue(r);

    Handle<PricingEngine> engine = Handle<PricingEngine>(new PricingEngines::AnalyticBarrierEngine());
/*
    Handle<PricingEngine> mcEngine = Handle<PricingEngine>(        
        new PricingEngines::MCBarrierEngine<MonteCarlo::PseudoRandom, Math::Statistics>
                    (timeSteps, antitheticVariate, 
                    controlVariate, requiredSamples,                     
                    requiredTolerance, maxSamples, isBiased, 5));
                    //Null<long>()));        

    Handle<PricingEngine> mcPutEngine = Handle<PricingEngine>(        
        new PricingEngines::MCBarrierEngine<MonteCarlo::PseudoRandom, Math::Statistics>
                    (timeSteps, antitheticVariate, 
                    controlVariate, requiredSamples,                     
                    requiredTolerance, maxSamples, isBiased, 45));
                    //Null<long>()));        
*/

    for (Size i=0; i<LENGTH(values); i++) {
        volatilityH_SME->setValue(values[i].volatility);

        Instruments::BarrierOption barrierCallOption = Instruments::BarrierOption(
                values[i].type, 
                values[i].barrier, 
                rebate, 
                Option::Call, 
                RelinkableHandle<MarketElement>(underlyingH), 
                values[i].strike, 
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
                "Call option:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }

        /*
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
*/

        Instruments::BarrierOption barrierPutOption = Instruments::BarrierOption(
                values[i].type, 
                values[i].barrier, 
                rebate, 
                Option::Put, 
                RelinkableHandle<MarketElement>(underlyingH), 
                values[i].strike, 
                RelinkableHandle<TermStructure>(qTS), 
                RelinkableHandle<TermStructure>(rTS),
                exercise, 
                RelinkableHandle<BlackVolTermStructure>(volTS), 
                engine);        
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

        /*
        barrierPutOption.setPricingEngine(mcPutEngine);
        calculated = barrierPutOption.NPV();
        if (QL_FABS(calculated-expected) > maxMCErrorAllowed) {
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Barrier put option MC:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }

*/
        Instruments::BarrierOption barrierStraddleOption = Instruments::BarrierOption(
                values[i].type, 
                values[i].barrier, 
                rebate, 
                Option::Straddle, 
                RelinkableHandle<MarketElement>(underlyingH), 
                values[i].strike, 
                RelinkableHandle<TermStructure>(qTS), 
                RelinkableHandle<TermStructure>(rTS),
                exercise, 
                RelinkableHandle<BlackVolTermStructure>(volTS), 
                engine);        
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
    double maxStraddleErrorAllowed = 5.0e-3;
    
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
        { Barrier::Type::DownIn, 0.10,   100,      90,  0.07187,  0.0 },
        { Barrier::Type::DownIn, 0.15,   100,      90,  0.60638,  0.0 },
        { Barrier::Type::DownIn, 0.20,   100,      90,  1.64005,  0.0 },
        { Barrier::Type::DownIn, 0.25,   100,      90,  2.98495,  0.0 },
        { Barrier::Type::DownIn, 0.30,   100,      90,  4.50952,  0.0 },
        { Barrier::Type::UpIn,   0.10,   100,      110,  4.79148,  0.0 },
        { Barrier::Type::UpIn,   0.15,   100,      110,  7.08268,  0.0 },
        { Barrier::Type::UpIn,   0.20,   100,      110,  9.11008,  0.0 },
        { Barrier::Type::UpIn,   0.25,   100,      110,  11.06148,  0.0 },
        { Barrier::Type::UpIn,   0.30,   100,      110,  12.98351,  0.0 }
    };

    Handle<SimpleMarketElement> underlyingH_SME(new SimpleMarketElement(underlyingPrice));    
    Handle<MarketElement> underlyingH = underlyingH_SME;

    Handle<SimpleMarketElement> qH_SME(new SimpleMarketElement(q));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);
    
    Handle<SimpleMarketElement> rH_SME(new SimpleMarketElement(r));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);

    Handle<SimpleMarketElement> volatilityH_SME(new SimpleMarketElement(0.10));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatilityH_SME);
    
    Handle<PricingEngine> engine = Handle<PricingEngine>(new PricingEngines::AnalyticBarrierEngine());
    Handle<PricingEngine> mcEngine = Handle<PricingEngine>(        
        new PricingEngines::MCBarrierEngine<MonteCarlo::PseudoRandom, Math::Statistics>
                    (timeSteps, antitheticVariate, 
                    controlVariate, requiredSamples,                     
                    requiredTolerance, maxSamples, isBiased, 5));
                    //Null<long>()));        

    // the date calculations are ignored by DayCounters::Year
    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,1,Years);    
    EuropeanExercise exercise(exDate);

    for (Size i=0; i<LENGTH(values); i++) {
        volatilityH_SME->setValue(values[i].volatility);

        // analytic
        Instruments::BarrierOption barrierCallOption = Instruments::BarrierOption(
                values[i].type, 
                values[i].barrier, 
                rebate, 
                Option::Call, 
                RelinkableHandle<MarketElement>(underlyingH), 
                values[i].strike, 
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
    double maxMCErrorAllowed = 1.0e-1;
    double maxStraddleErrorAllowed = 5.0e-3;
    
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
        "Going to Extreme: Correcting Simulation Bias in Exotic Option Valuation"
          - D.R. Beaglehole, P.H. Dybvig and G. Zhou
            Financial Analysts Journal; Jan / Feb 1997; 53, 1
    */
    BarrierOptionData values[] = {
        { Barrier::Type::DownOut, 0.50,   50,      45,  5.477,  0.0 }        
    };

    Handle<SimpleMarketElement> underlyingH_SME(new SimpleMarketElement(underlyingPrice));    
    Handle<MarketElement> underlyingH = underlyingH_SME;

    Handle<SimpleMarketElement> qH_SME(new SimpleMarketElement(q));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);
    
    Handle<SimpleMarketElement> rH_SME(new SimpleMarketElement(r));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);

    Handle<SimpleMarketElement> volatilityH_SME(new SimpleMarketElement(0.10));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatilityH_SME);  
    
    Handle<PricingEngine> engine = Handle<PricingEngine>(new PricingEngines::AnalyticBarrierEngine());
    Handle<PricingEngine> mcEngine = Handle<PricingEngine>(        
        new PricingEngines::MCBarrierEngine<MonteCarlo::PseudoRandom, Math::Statistics>
                    (timeSteps, antitheticVariate, 
                    controlVariate, requiredSamples,                     
                    requiredTolerance, maxSamples, isBiased, 10));
                    //Null<long>()));        

    // the date calculations are ignored by DayCounters::Year
    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,1,Years);    
    EuropeanExercise exercise(exDate);

    for (Size i=0; i<LENGTH(values); i++) {
        volatilityH_SME->setValue(values[i].volatility);

        // analytic
        Instruments::BarrierOption barrierCallOption = Instruments::BarrierOption(
                values[i].type, 
                values[i].barrier, 
                rebate, 
                Option::Call, 
                RelinkableHandle<MarketElement>(underlyingH), 
                values[i].strike, 
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
                   ("Testing Barrier option valuation - Haug values ",
                    &BarrierOptionTest::testHaugValues));
    tests->addTest(new CppUnit::TestCaller<BarrierOptionTest>
                   ("Testing Barrier option valuation - Babsiri values ",
                    &BarrierOptionTest::testBabsiriValues));
   tests->addTest(new CppUnit::TestCaller<BarrierOptionTest>
                   ("Testing Barrier option valuation - Beaglehole values ",
                    &BarrierOptionTest::testBeagleholeValues));

    return tests;
}


