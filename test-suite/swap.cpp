
/*
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

#include "swap.hpp"
#include "utilities.hpp"
#include <ql/Instruments/simpleswap.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/DayCounters/actual365.hpp>
#include <ql/Indexes/euribor.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

namespace {

    // global data

    Date today_, settlement_;
    bool payFixed_;
    double nominal_;
    Calendar calendar_;
    RollingConvention rollingConvention_;
    int fixedFrequency_, floatingFrequency_;
    DayCounter fixedDayCount_;
    bool fixedIsAdjusted_;
    Handle<Xibor> index_;
    int settlementDays_, fixingDays_;
    RelinkableHandle<TermStructure> termStructure_;

    // utilities

    Handle<SimpleSwap> makeSwap(int length, Rate fixedRate,
                                Spread floatingSpread) {
        return Handle<SimpleSwap>(
            new SimpleSwap(payFixed_,settlement_,length,Years,
                           calendar_,rollingConvention_,nominal_,
                           fixedFrequency_,fixedRate,fixedIsAdjusted_,
                           fixedDayCount_,floatingFrequency_,index_,
                           fixingDays_,floatingSpread,termStructure_));
    }

}

// tests

void SimpleSwapTest::setUp() {
    payFixed_ = true;
    settlementDays_ = 2;
    fixingDays_ = 2;
    nominal_ = 100.0;
    rollingConvention_ = ModifiedFollowing;
    fixedFrequency_ = 1;
    floatingFrequency_ = 2;
    fixedDayCount_ = Thirty360();
    fixedIsAdjusted_ = false;
    index_ = Handle<Xibor>(new Euribor(12/floatingFrequency_,Months,
                                       termStructure_));
    calendar_ = index_->calendar();
    today_ = calendar_.roll(Date::todaysDate());
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(
        Handle<TermStructure>(new FlatForward(today_,settlement_,0.05,
                                              Actual365())));
}

void SimpleSwapTest::testFairRate() {

    int lengths[] = { 1, 2, 5, 10, 20 };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(spreads); j++) {

            Handle<SimpleSwap> swap = makeSwap(lengths[i],0.0,spreads[j]);
            swap = makeSwap(lengths[i],swap->fairRate(),spreads[j]);
            if (QL_FABS(swap->NPV()) > 1.0e-10) {
                CPPUNIT_FAIL(
                    "recalculating with implied rate:\n"
                    "    length: " + 
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    floating spread: " +
                    DoubleFormatter::toString(spreads[j]*100,2) + " %\n"
                    "    swap value: " +
                    DoubleFormatter::toString(swap->NPV()));
            }
        }
    }
}

void SimpleSwapTest::testFairSpread() {

    int lengths[] = { 1, 2, 5, 10, 20 };
    Rate rates[] = { 0.04, 0.05, 0.06, 0.07 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(rates); j++) {

            Handle<SimpleSwap> swap = makeSwap(lengths[i],rates[j],0.0);
            swap = makeSwap(lengths[i],rates[j],swap->fairSpread());
            if (QL_FABS(swap->NPV()) > 1.0e-10) {
                CPPUNIT_FAIL(
                    "recalculating with implied spread:\n"
                    "    length: " + 
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    fixed rate: " +
                    DoubleFormatter::toString(rates[j]*100,2) + " %\n"
                    "    swap value: " +
                    DoubleFormatter::toString(swap->NPV()));
            }
        }
    }
}

void SimpleSwapTest::testRateDependency() {

    int lengths[] = { 1, 2, 5, 10, 20 };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };
    Rate rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(spreads); j++) {
            // store the results for different rates...
            std::vector<double> swap_values;
            for (Size k=0; k<LENGTH(rates); k++) {
                Handle<SimpleSwap> swap = 
                    makeSwap(lengths[i],rates[k],spreads[j]);
                swap_values.push_back(swap->NPV());
            }
            // and check that they go the right way
            std::vector<double>::iterator it = 
                std::adjacent_find(swap_values.begin(),swap_values.end(),
                                   std::less<double>());
            if (it != swap_values.end()) {
                int n = it - swap_values.begin();
                CPPUNIT_FAIL(
                    "NPV is increasing with the fixed rate in a swap: \n"
                    "    length: " + 
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    value: " +
                    DoubleFormatter::toString(swap_values[n]) +
                    " paying fixed rate: " +
                    DoubleFormatter::toString(rates[n]*100,2) + " %\n"
                    "    value: " +
                    DoubleFormatter::toString(swap_values[n+1]) +
                    " paying fixed rate: " +
                    DoubleFormatter::toString(rates[n+1]*100,2) + " %");
            }
        }
    }
}

void SimpleSwapTest::testSpreadDependency() {

    int lengths[] = { 1, 2, 5, 10, 20 };
    Rate rates[] = { 0.04, 0.05, 0.06, 0.07 };
    Spread spreads[] = { -0.01, -0.002, -0.001, 0.0, 0.001, 0.002, 0.01 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(rates); j++) {
            // store the results for different spreads...
            std::vector<double> swap_values;
            for (Size k=0; k<LENGTH(spreads); k++) {
                Handle<SimpleSwap> swap = 
                    makeSwap(lengths[i],rates[j],spreads[k]);
                swap_values.push_back(swap->NPV());
            }
            // and check that they go the right way
            std::vector<double>::iterator it = 
                std::adjacent_find(swap_values.begin(),swap_values.end(),
                                   std::greater<double>());
            if (it != swap_values.end()) {
                int n = it - swap_values.begin();
                CPPUNIT_FAIL(
                    "NPV is decreasing with the floating spread in a swap: \n"
                    "    length: " + 
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    value: " +
                    DoubleFormatter::toString(swap_values[n]) +
                    " receiving spread: " +
                    DoubleFormatter::toString(spreads[n]*100,2) + " %\n"
                    "    value: " +
                    DoubleFormatter::toString(swap_values[n+1]) +
                    " receiving spread: " +
                    DoubleFormatter::toString(spreads[n+1]*100,2) + " %");
            }
        }
    }
}

void SimpleSwapTest::testCachedValue() {

    today_ = Date(17,June,2002);
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(
        Handle<TermStructure>(new FlatForward(today_,settlement_,0.05,
                                              Actual365())));

    Handle<SimpleSwap> swap = makeSwap(10, 0.06, 0.001);
    double cachedNPV   = -5.883663676727;
    
    if (QL_FABS(swap->NPV()-cachedNPV) > 1.0e-11)
        CPPUNIT_FAIL(
            "failed to reproduce cached swap value:\n"
            "    calculated: " +
            DoubleFormatter::toString(swap->NPV(),12) + "\n"
            "    expected:   " +
            DoubleFormatter::toString(cachedNPV,12));
}
        
CppUnit::Test* SimpleSwapTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Simple swap tests");
    tests->addTest(new CppUnit::TestCaller<SimpleSwapTest>
                   ("Testing simple swap calculation of fair fixed rate",
                    &SimpleSwapTest::testFairRate));
    tests->addTest(new CppUnit::TestCaller<SimpleSwapTest>
                   ("Testing simple swap calculation of fair floating spread",
                    &SimpleSwapTest::testFairSpread));
    tests->addTest(new CppUnit::TestCaller<SimpleSwapTest>
                   ("Testing simple swap dependency on fixed rate",
                    &SimpleSwapTest::testRateDependency));
    tests->addTest(new CppUnit::TestCaller<SimpleSwapTest>
                   ("Testing simple swap dependency on floating spread",
                    &SimpleSwapTest::testSpreadDependency));
    tests->addTest(new CppUnit::TestCaller<SimpleSwapTest>
                   ("Testing simple swap calculation against cached value",
                    &SimpleSwapTest::testCachedValue));
    return tests;
}

