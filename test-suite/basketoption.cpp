
/*
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
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
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
                                        DayCounter dc = SimpleDayCounter()) {
        Date today = Date::todaysDate();
        return Handle<TermStructure>(new
            FlatForward(today, today, RelinkableHandle<Quote>(forward), dc));
    }

    Handle<BlackVolTermStructure> makeFlatVolatility(const Handle<Quote>& vol,
                                                     DayCounter dc = SimpleDayCounter()) {
        Date today = Date::todaysDate();
        return Handle<BlackVolTermStructure>(new
            BlackConstantVol(today, RelinkableHandle<Quote>(vol), dc));
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
    
    struct BasketOptionData {        
        Option::Type type;
        BasketOption::BasketType basketType;
        double s1_0;
        double s2_0;
        double rho;
        double vol_S1;
        double vol_S2;
        double strike;
        double intRate;
        int expiry; // months
        double value;        
    };
    
}

void BasketOptionTest::testValues() {

    double maxErrorAllowed = 1.0e-3;    

    double underlyingPrice = 100.0;    
    Rate r = 0.05;    
    // cannot handle dividends....
    Rate q = 0.00;    

    /*
        Data from Excel spreadsheet
        http://www.maths.ox.ac.uk/~firth/computing/excel.shtml
            {option type, basket type, s1_0, s2_0, rho, vol_S1, vol_S2, 
                    strike, intRate, expiry, value}        
        */
    BasketOptionData values[] = {
        // call on the min            
        {Option::Call, BasketOption::Min, 100.0, 100.0, 0.9, 0.3, 0.3, 100, 0.05, 12, 10.89772},        
        {Option::Call, BasketOption::Min, 100.0, 100.0, 0.7, 0.3, 0.3, 100, 0.05, 12, 8.482995},
        {Option::Call, BasketOption::Min, 100.0, 100.0, 0.5, 0.3, 0.3, 100, 0.05, 12, 6.843896},
        {Option::Call, BasketOption::Min, 100.0, 100.0, 0.3, 0.3, 0.3, 100, 0.05, 12, 5.530978},
        {Option::Call, BasketOption::Min, 100.0, 100.0, 0.1, 0.3, 0.3, 100, 0.05, 12, 4.413080},
        {Option::Call, BasketOption::Min, 100.0, 100.0, 0.0, 0.5, 0.7, 100, 0.05, 12, 4.981078},
        {Option::Call, BasketOption::Min, 100.0, 100.0, 0.0, 0.5, 0.3, 100, 0.05, 12, 4.158957},
        {Option::Call, BasketOption::Min, 100.0, 100.0, 0.0, 0.5, 0.1, 100, 0.05, 12, 2.597341},
        {Option::Call, BasketOption::Min, 100.0, 100.0, 0.5, 0.5, 0.1, 100, 0.05, 12, 4.029683},
        // call on the max
        {Option::Call, BasketOption::Max, 100.0, 100.0, 0.9, 0.3, 0.3, 100, 0.05, 12, 17.56477},        
        {Option::Call, BasketOption::Max, 100.0, 100.0, 0.7, 0.3, 0.3, 100, 0.05, 12, 19.97949},
        {Option::Call, BasketOption::Max, 100.0, 100.0, 0.5, 0.3, 0.3, 100, 0.05, 12, 21.61859},
        {Option::Call, BasketOption::Max, 100.0, 100.0, 0.3, 0.3, 0.3, 100, 0.05, 12, 22.93151},
        {Option::Call, BasketOption::Max, 100.0, 100.0, 0.1, 0.3, 0.3, 100, 0.05, 12, 24.04941},
        {Option::Call, BasketOption::Max, 80.0,  100.0, 0.3, 0.3, 0.3, 100, 0.05, 12, 16.5077},
        {Option::Call, BasketOption::Max, 80.0,  80.0,  0.3, 0.3, 0.3, 100, 0.05, 12, 8.049205},
        {Option::Call, BasketOption::Max, 80.0,  120.0, 0.3, 0.3, 0.3, 100, 0.05, 12, 30.14043},
        {Option::Call, BasketOption::Max, 120.0, 120.0, 0.3, 0.3, 0.3, 100, 0.05, 12, 42.88935},
        // put on the min
        {Option::Put, BasketOption::Min, 100.0, 100.0, 0.9, 0.3, 0.3, 100, 0.05, 12, 11.36904},
        {Option::Put, BasketOption::Min, 100.0, 100.0, 0.7, 0.3, 0.3, 100, 0.05, 12, 12.85571},
        {Option::Put, BasketOption::Min, 100.0, 100.0, 0.5, 0.3, 0.3, 100, 0.05, 12, 13.89038},
        {Option::Put, BasketOption::Min, 100.0, 100.0, 0.3, 0.3, 0.3, 100, 0.05, 12, 14.74097},
        {Option::Put, BasketOption::Min, 100.0, 100.0, 0.1, 0.3, 0.3, 100, 0.05, 12, 15.4854},
        {Option::Put, BasketOption::Min, 100.0, 100.0, 0.1, 0.3, 0.3, 100, 0.05, 6, 13.93915},
        {Option::Put, BasketOption::Min, 100.0, 100.0, 0.1, 0.3, 0.3, 100, 0.05, 3, 12.60395},
        {Option::Put, BasketOption::Min, 100.0, 100.0, 0.1, 0.3, 0.3, 100, 0.05, 24, 17.00579},
        // put on the max
        {Option::Put, BasketOption::Max, 100.0, 100.0, 0.9, 0.3, 0.3, 100, 0.05, 12, 7.339334},
        {Option::Put, BasketOption::Max, 100.0, 100.0, 0.7, 0.3, 0.3, 100, 0.05, 12, 5.852663},
        {Option::Put, BasketOption::Max, 100.0, 100.0, 0.5, 0.3, 0.3, 100, 0.05, 12, 4.817994},
        {Option::Put, BasketOption::Max, 100.0, 100.0, 0.3, 0.3, 0.3, 100, 0.05, 12, 3.967405},
        {Option::Put, BasketOption::Max, 100.0, 100.0, 0.1, 0.3, 0.3, 100, 0.05, 12, 3.222970}
    };

    Handle<SimpleQuote> underlying1(new SimpleQuote(underlyingPrice));
    Handle<SimpleQuote> underlying2(new SimpleQuote(underlyingPrice));

    // cannot handle dividends
    Handle<SimpleQuote> qH_SME(new SimpleQuote(q));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);

    Handle<SimpleQuote> rH_SME(new SimpleQuote(r));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);

    Handle<SimpleQuote> volatility1(new SimpleQuote(0.10));
    Handle<BlackVolTermStructure> volTS1 = makeFlatVolatility(volatility1);
    Handle<SimpleQuote> volatility2(new SimpleQuote(0.10));
    Handle<BlackVolTermStructure> volTS2 = makeFlatVolatility(volatility2);


    Handle<PricingEngine> engine(new StulzEngine);
    
    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();

    for (Size i=0; i<LENGTH(values); i++) {
        Date exDate = calendar.advance(today,values[i].expiry,Months);
        Handle<Exercise> exercise(new EuropeanExercise(exDate));

        rH_SME->setValue(values[i].intRate);

        underlying1->setValue(values[i].s1_0);
        underlying2->setValue(values[i].s2_0);

        volatility1->setValue(values[i].vol_S1);
        volatility2->setValue(values[i].vol_S2);

        Handle<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(values[i].type, values[i].strike));

        BasketOption::BasketType basketType = values[i].basketType;

        Handle<BlackScholesStochasticProcess> stochProcess1(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(underlying1),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS1)));

        Handle<BlackScholesStochasticProcess> stochProcess2(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(underlying2),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS2)));

        std::vector<Handle<BlackScholesStochasticProcess> > procs = 
            std::vector<Handle<BlackScholesStochasticProcess> >();
        procs.push_back(stochProcess1);
        procs.push_back(stochProcess2);

        double rho = values[i].rho;

        // analytic
        BasketOption basketOption(
            basketType,
            procs,
            payoff,
            exercise,
            rho,
            engine
            );
                
        double calculated = basketOption.NPV();
        double expected = values[i].value;
        if (QL_FABS(calculated-expected) > maxErrorAllowed) {
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Basket option:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
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

