
/*
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

#include "piecewiseflatforward.hpp"
#include <ql/TermStructures/piecewiseflatforward.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/Indexes/euribor.hpp>

using namespace QuantLib;
using namespace QuantLib::TermStructures;
using namespace QuantLib::Calendars;
using namespace QuantLib::DayCounters;
using namespace QuantLib::Indexes;
using namespace QuantLib::Instruments;

#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

namespace {

    struct Datum {
        int n;
        TimeUnit units;
        double rate;
    };

}

void PiecewiseFlatForwardTest::runTest() {

    Calendar calendar = TARGET();
    int settlementDays = 2, fixingDays = 2;
    Date today = calendar.roll(Date::todaysDate());
    Date settlement = calendar.advance(today,settlementDays,Days);
    RelinkableHandle<TermStructure> euriborHandle;

    // deposits
    RollingConvention rollingConvention = ModifiedFollowing;
    DayCounter dayCounter = Actual360();
    Datum depositData[] = {
        { 1, Weeks,  4.559 },
        { 1, Months, 4.581 },
        { 2, Months, 4.573 },
        { 3, Months, 4.557 },
        { 6, Months, 4.496 },
        { 9, Months, 4.490 }
    };
    // swaps
    RollingConvention swapRollingConvention = ModifiedFollowing;
    int fixedFrequency = 1;
    bool fixedIsAdjusted = false;
    DayCounter fixedDayCounter = Thirty360();
    int floatingFrequency = 2;
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

    Size deposits = LENGTH(depositData),
         swaps = LENGTH(swapData);

    std::vector<Handle<RateHelper> > instruments(deposits+swaps);
    Size i;
    for (i=0; i<deposits; i++) {
        instruments[i] = Handle<RateHelper>(
            new DepositRateHelper(depositData[i].rate/100,
                                  depositData[i].n, depositData[i].units,
                                  settlementDays, calendar,
                                  rollingConvention, dayCounter));
    }
    for (i=0; i<swaps; i++) {
        instruments[i+deposits] = Handle<RateHelper>(
            new SwapRateHelper(swapData[i].rate/100,
                               swapData[i].n, swapData[i].units,
                               settlementDays, calendar,
                               swapRollingConvention,
                               fixedFrequency, fixedIsAdjusted,
                               fixedDayCounter, floatingFrequency));
    }

    // instantiate curve
    Handle<TermStructure> termStructure(
        new PiecewiseFlatForward(today,settlement,instruments,Actual360()));
    euriborHandle.linkTo(termStructure);

    // check deposits
    for (i=0; i<deposits; i++) {
        Euribor index(depositData[i].n,depositData[i].units,euriborHandle);
        double expectedRate  = depositData[i].rate/100,
               estimatedRate = index.fixing(today);
        if (QL_FABS(expectedRate-estimatedRate) > 1.0e-9) {
            CPPUNIT_FAIL(
                IntegerFormatter::toString(depositData[i].n) + " "
                + (depositData[i].units == Weeks ? std::string("week(s)") :
                                                   std::string("month(s)")) +
                " deposit: \n"
                "    estimated rate: "
                + RateFormatter::toString(estimatedRate,8) + "\n"
                "    expected rate:  "
                + RateFormatter::toString(expectedRate,8));
        }
    }

    // check swaps
    Handle<Xibor> index(new Euribor(12/floatingFrequency,Months,
                                    euriborHandle));
    for (i=0; i<swaps; i++) {
        SimpleSwap swap(true,settlement,swapData[i].n,swapData[i].units,
                        calendar,swapRollingConvention,100.0,
                        fixedFrequency,0.0,fixedIsAdjusted,
                        fixedDayCounter,floatingFrequency,index,
                        fixingDays,0.0,euriborHandle);
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



