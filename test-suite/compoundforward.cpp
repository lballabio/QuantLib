
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
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/Instruments/simpleswap.hpp>
#include <ql/Indexes/zarlibor.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct Datum {
        Integer n;
        TimeUnit units;
        Rate rate;
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
    Integer settlementDays, fixingDays;
    Date today, settlement;
    BusinessDayConvention convention;
    DayCounter dayCounter;
    Frequency frequency;

    Size deposits, swaps;
    std::vector<Rate> rates;
    std::vector<Date> dates;
    boost::shared_ptr<CompoundForward> termStructure;


    void setup() {

        // data
        calendar = Johannesburg();
        settlementDays = 0;
        fixingDays = 0;
        today = calendar.adjust(Date::todaysDate());
        Settings::instance().setEvaluationDate(today);
        settlement = calendar.advance(today,settlementDays,Days);
        convention = ModifiedFollowing;
        dayCounter = Actual365Fixed();
        frequency = Semiannual;

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
                                        convention);
        }
        for (i=0; i<swaps; i++) {
            rates[i+deposits] = swapData[i].rate/100;
            dates[i+deposits] = calendar.advance(settlement,
                                                 Period(swapData[i].n,
                                                        swapData[i].units),
                                                 convention);
        }

        termStructure = boost::shared_ptr<CompoundForward>(
                             new CompoundForward(settlement,dates,rates,
                                                 calendar,convention,
                                                 frequency,dayCounter));
    }

    void teardown() {
        Settings::instance().setEvaluationDate(Date());
    }

}

void CompoundForwardTest::testSuppliedRates() {

    BOOST_MESSAGE("Testing consistency of compound-forward curve "
                  "with supplied rates...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Handle<YieldTermStructure> liborHandle;
    liborHandle.linkTo(termStructure);

    Size i;
    // check swaps against original
    boost::shared_ptr<Xibor> index(
                               new ZARLibor(12/frequency,Months,liborHandle));
    for (i=0; i<swaps; i++) {
        Date maturity = calendar.advance(settlement,
                                         swapData[i].n,swapData[i].units,
                                         convention);
        Schedule schedule(calendar,settlement,maturity,
                          frequency,convention);
        SimpleSwap swap(true,100.0,
                        schedule,0.0,dayCounter,
                        schedule,index,fixingDays,0.0,
                        liborHandle);
        Rate expectedRate = swapData[i].rate/100,
             estimatedRate = swap.fairRate();
        if (std::fabs(expectedRate-estimatedRate) > 1.0e-9) {
            BOOST_FAIL(
                IntegerFormatter::toString(swapData[i].n) + " year(s) swap:\n"
                "    estimated rate: "
                + RateFormatter::toString(estimatedRate,8) + "\n"
                "    expected rate:  "
                + RateFormatter::toString(expectedRate,8));
        }
    }

    QL_TEST_TEARDOWN
}

void CompoundForwardTest::testConvertedRates() {

    BOOST_MESSAGE("Testing consistency of compound-forward curve "
                  "with converted rates...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Handle<YieldTermStructure> liborHandle;
    liborHandle.linkTo(termStructure);

    Size i;
    frequency = Quarterly;
    // check swaps against quarterly rates
    boost::shared_ptr<Xibor> index(
                               new ZARLibor(12/frequency,Months,liborHandle));
    for (i=0; i<swaps; i++) {
        Date maturity = calendar.advance(settlement,
                                         swapData[i].n,swapData[i].units,
                                         convention);
        Schedule schedule(calendar,settlement,maturity,
                          frequency,convention);
        SimpleSwap swap(true,100.0,
                        schedule,0.0,dayCounter,
                        schedule,index,fixingDays,0.0,
                        liborHandle);
        DayCounter tsdc  = termStructure->dayCounter();
        Rate expectedRate = termStructure->compoundForward(swap.maturity(),
                                                           frequency);
        Rate estimatedRate = swap.fairRate();
        if (std::fabs(expectedRate-estimatedRate) > 1.0e-9) {
            BOOST_FAIL(
                IntegerFormatter::toString(swapData[i].n) + " year(s) swap:\n"
                "    estimated rate: "
                + RateFormatter::toString(estimatedRate,8) + "\n"
                "    compound rate:  "
                + RateFormatter::toString(expectedRate,8));
        }
    }

    QL_TEST_TEARDOWN
}


test_suite* CompoundForwardTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Compound forward tests");
    suite->add(BOOST_TEST_CASE(&CompoundForwardTest::testSuppliedRates));
    suite->add(BOOST_TEST_CASE(&CompoundForwardTest::testConvertedRates));
    return suite;
}

