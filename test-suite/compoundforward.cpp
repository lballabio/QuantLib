
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

#include "compoundforward.hpp"
#include "utilities.hpp"
#include <ql/TermStructures/compoundforward.hpp>
#include <ql/DayCounters/actual365.hpp>
#include <ql/Instruments/simpleswap.hpp>
#include <ql/Indexes/zarlibor.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

namespace {

    struct Datum {
        int n;
        TimeUnit units;
        double rate;
    };

    Datum depositData[] = {
        { 3, Months, 4.557 },
        { 6, Months, 4.496 },
        { 9, Months, 4.490 }
    };

    Datum swapData[] = {
        {  1, Years, 4.54 },
        {  2, Years, 4.63 },
        {  3, Years, 4.75 },
        {  4, Years, 4.86 },
        {  5, Years, 4.99 },
        {  6, Years, 5.11 },
        {  7, Years, 5.23 },
        {  8, Years, 5.33 },
        {  9, Years, 5.41 },
        { 10, Years, 5.47 },
        { 12, Years, 5.60 },
        { 15, Years, 5.75 },
        { 20, Years, 5.89 },
        { 25, Years, 5.95 },
        { 30, Years, 5.96 }
    };

    // test-global variables

    Calendar calendar;
    int settlementDays, fixingDays;
    Date today, settlement;
    RollingConvention rollingConvention;
    DayCounter dayCounter;
    int frequency;

    Size deposits, swaps;
    std::vector<Rate> rates;
    std::vector<Date> dates;
    Handle<TermStructure> termStructure;
}

void CompoundForwardTest::setUp() {

    // data
    calendar = Johannesburg();
    settlementDays = 0;
    fixingDays = 0;
    today = calendar.roll(Date::todaysDate());
    // just for fun
    settlement = calendar.advance(today,settlementDays,Days);
    rollingConvention = ModifiedFollowing;
    dayCounter = Actual365();
    frequency = 2;

    deposits = LENGTH(depositData);
    swaps = LENGTH(swapData);

    // market elements
    rates = std::vector<Rate>(deposits+swaps);
    dates = std::vector<Date>(deposits+swaps);
    Size i;
    for (i=0; i<deposits; i++) {
        rates[i] = depositData[i].rate/100;
	dates[i] = calendar.advance(settlement,
				    Period(depositData[i].n,
					   depositData[i].units),
				    rollingConvention);
    }
    for (i=0; i<swaps; i++) {
        rates[i+deposits] = swapData[i].rate/100;
	dates[i+deposits] = calendar.advance(settlement,
					     Period(swapData[i].n,
						    swapData[i].units),
					     rollingConvention);
    }

    termStructure = Handle<TermStructure>(
	new CompoundForward(today,settlement,dates,rates,
			    calendar,rollingConvention,frequency,dayCounter));
}

void CompoundForwardTest::testSuppliedRates() {

    RelinkableHandle<TermStructure> liborHandle;
    liborHandle.linkTo(termStructure);

    Size i;
    // check swaps against original
    Handle<Xibor> index(new ZARLibor(12/frequency,Months,liborHandle));
    for (i=0; i<swaps; i++) {
        SimpleSwap swap(true,settlement,swapData[i].n,swapData[i].units,
                        calendar,rollingConvention,100.0,
                        frequency,0.0,true,
                        dayCounter,frequency,index,
                        fixingDays,0.0,liborHandle);
        double expectedRate = swapData[i].rate/100,
               estimatedRate = swap.fairRate();
        if (QL_FABS(expectedRate-estimatedRate) > 1.0e-9) {
            CPPUNIT_FAIL(
                IntegerFormatter::toString(swapData[i].n) + " year(s) swap:\n"
                "    estimated rate: "
                + RateFormatter::toString(estimatedRate,8) + "\n"
                "    expected rate:  "
                + RateFormatter::toString(expectedRate,8));
        }
    }
}

void CompoundForwardTest::testConvertedRates() {

    RelinkableHandle<TermStructure> liborHandle;
    liborHandle.linkTo(termStructure);

    Size i;
    frequency = 4;
    // check swaps against quarterly rates
    Handle<Xibor> index(new ZARLibor(12/frequency,Months,liborHandle));
    for (i=0; i<swaps; i++) {
        SimpleSwap swap(true,settlement,swapData[i].n,swapData[i].units,
                        calendar,rollingConvention,100.0,
                        frequency,0.0,true,
                        dayCounter,frequency,index,
                        fixingDays,0.0,liborHandle);
        double expectedRate = termStructure->compoundForward(swap.maturity(),
							     frequency),
               estimatedRate = swap.fairRate();
        if (QL_FABS(expectedRate-estimatedRate) > 1.0e-9) {
            CPPUNIT_FAIL(
                IntegerFormatter::toString(swapData[i].n) + " year(s) swap:\n"
                "    estimated rate: "
                + RateFormatter::toString(estimatedRate,8) + "\n"
                "    compound rate:  "
                + RateFormatter::toString(expectedRate,8));
        }
    }
}

CppUnit::Test* CompoundForwardTest::suite() {
    CppUnit::TestSuite* tests = 
        new CppUnit::TestSuite("Compound forward tests");
    tests->addTest(new CppUnit::TestCaller<CompoundForwardTest>
                   ("Testing consistency of compound-forward curve "
                    "with supplied rates",
                    &CompoundForwardTest::testSuppliedRates));
    tests->addTest(new CppUnit::TestCaller<CompoundForwardTest>
                   ("Testing consistency of compound-forward curve "
                    "with converted rates",
                    &CompoundForwardTest::testConvertedRates));
    return tests;
}

