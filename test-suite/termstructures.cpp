
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

#include "termstructures.hpp"
#include "utilities.hpp"
#include <ql/TermStructures/piecewiseflatforward.hpp>
#include <ql/TermStructures/impliedtermstructure.hpp>
#include <ql/TermStructures/forwardspreadedtermstructure.hpp>
#include <ql/TermStructures/zerospreadedtermstructure.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

namespace {

    // global data

    Calendar calendar_;
    int settlementDays_;
    boost::shared_ptr<TermStructure> termStructure_;

    // utilities

    struct Datum {
        int n;
        TimeUnit units;
        double rate;
    };

}

void TermStructureTest::setUp() {
    calendar_ = TARGET();
    settlementDays_ = 2;
    Date today = calendar_.roll(Date::todaysDate());
    Date settlement = calendar_.advance(today,settlementDays_,Days);
    Datum depositData[] = {
        { 1, Months, 4.581 },
        { 2, Months, 4.573 },
        { 3, Months, 4.557 },
        { 6, Months, 4.496 },
        { 9, Months, 4.490 }
    };
    Datum swapData[] = {
        {  1, Years, 4.54 },
        {  5, Years, 4.99 },
        { 10, Years, 5.47 },
        { 20, Years, 5.89 },
        { 30, Years, 5.96 }
    };
    Size deposits = LENGTH(depositData),
         swaps = LENGTH(swapData);

    std::vector<boost::shared_ptr<RateHelper> > instruments(deposits+swaps);
    Size i;
    for (i=0; i<deposits; i++) {
        instruments[i] = boost::shared_ptr<RateHelper>(
            new DepositRateHelper(depositData[i].rate/100,
                                  depositData[i].n, depositData[i].units,
                                  settlementDays_, calendar_,
                                  ModifiedFollowing, Actual360()));
    }
    for (i=0; i<swaps; i++) {
        instruments[i+deposits] = boost::shared_ptr<RateHelper>(
            new SwapRateHelper(swapData[i].rate/100,
                               swapData[i].n, swapData[i].units,
                               settlementDays_, calendar_,
                               ModifiedFollowing, 1, false,
                               Thirty360(), 2));
    }
    termStructure_ = boost::shared_ptr<TermStructure>(
        new PiecewiseFlatForward(today,settlement,instruments,Actual360()));
}

void TermStructureTest::testImplied() {
    double tolerance = 1.0e-10;
    Date newToday = termStructure_->todaysDate().plusYears(3);
    Date newSettlement = calendar_.advance(newToday,settlementDays_,Days);
    Date testDate = newSettlement.plusYears(5);
    boost::shared_ptr<TermStructure> implied(
        new ImpliedTermStructure(
            RelinkableHandle<TermStructure>(termStructure_),
            newToday, newSettlement));
    double baseDiscount = termStructure_->discount(newSettlement);
    double discount = termStructure_->discount(testDate);
    double impliedDiscount = implied->discount(testDate);
    if (QL_FABS(discount - baseDiscount*impliedDiscount) > tolerance)
        CPPUNIT_FAIL(
            "unable to reproduce discount from implied curve\n"
            "    calculated: "
            + DoubleFormatter::toString(baseDiscount*impliedDiscount,10) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(discount,10));
}

void TermStructureTest::testImpliedObs() {
    Date newToday = termStructure_->todaysDate().plusYears(3);
    Date newSettlement = calendar_.advance(newToday,settlementDays_,Days);
    RelinkableHandle<TermStructure> h;
    boost::shared_ptr<TermStructure> implied(
        new ImpliedTermStructure(h, newToday, newSettlement));
    Flag flag;
    flag.registerWith(implied);
    h.linkTo(termStructure_);
    if (!flag.isUp())
        CPPUNIT_FAIL("Observer was not notified of term structure change");
}

void TermStructureTest::testFSpreaded() {
    double tolerance = 1.0e-10;
    boost::shared_ptr<Quote> me(new SimpleQuote(0.01));
    RelinkableHandle<Quote> mh(me);
    boost::shared_ptr<TermStructure> spreaded(
        new ForwardSpreadedTermStructure(
            RelinkableHandle<TermStructure>(termStructure_),mh));
    Date testDate = termStructure_->referenceDate().plusYears(5);
    Rate forward = termStructure_->instantaneousForward(testDate);
    Rate spreadedForward = spreaded->instantaneousForward(testDate);
    if (QL_FABS(forward - (spreadedForward-me->value())) > tolerance)
        CPPUNIT_FAIL(
            "unable to reproduce forward from spreaded curve\n"
            "    calculated: "
            + DoubleFormatter::toString(spreadedForward-me->value(),10) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(forward,10));
}

void TermStructureTest::testFSpreadedObs() {
    boost::shared_ptr<SimpleQuote> me(new SimpleQuote(0.01));
    RelinkableHandle<Quote> mh(me);
    RelinkableHandle<TermStructure> h;
    boost::shared_ptr<TermStructure> spreaded(
        new ForwardSpreadedTermStructure(h,mh));
    Flag flag;
    flag.registerWith(spreaded);
    h.linkTo(termStructure_);
    if (!flag.isUp())
        CPPUNIT_FAIL("Observer was not notified of term structure change");
    flag.lower();
    me->setValue(0.005);
    if (!flag.isUp())
        CPPUNIT_FAIL("Observer was not notified of spread change");
}

void TermStructureTest::testZSpreaded() {
    double tolerance = 1.0e-10;
    boost::shared_ptr<Quote> me(new SimpleQuote(0.01));
    RelinkableHandle<Quote> mh(me);
    boost::shared_ptr<TermStructure> spreaded(
        new ZeroSpreadedTermStructure(
            RelinkableHandle<TermStructure>(termStructure_),mh));
    Date testDate = termStructure_->referenceDate().plusYears(5);
    Rate zero = termStructure_->zeroYield(testDate);
    Rate spreadedZero = spreaded->zeroYield(testDate);
    if (QL_FABS(zero - (spreadedZero-me->value())) > tolerance)
        CPPUNIT_FAIL(
            "unable to reproduce zero yield from spreaded curve\n"
            "    calculated: "
            + DoubleFormatter::toString(spreadedZero-me->value(),10) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(zero,10));
}

void TermStructureTest::testZSpreadedObs() {
    boost::shared_ptr<SimpleQuote> me(new SimpleQuote(0.01));
    RelinkableHandle<Quote> mh(me);
    RelinkableHandle<TermStructure> h;
    boost::shared_ptr<TermStructure> spreaded(
        new ZeroSpreadedTermStructure(h,mh));
    Flag flag;
    flag.registerWith(spreaded);
    h.linkTo(termStructure_);
    if (!flag.isUp())
        CPPUNIT_FAIL("Observer was not notified of term structure change");
    flag.lower();
    me->setValue(0.005);
    if (!flag.isUp())
        CPPUNIT_FAIL("Observer was not notified of spread change");
}

CppUnit::Test* TermStructureTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Term structure tests");
    tests->addTest(new CppUnit::TestCaller<TermStructureTest>
                   ("Testing consistency of implied term structure",
                    &TermStructureTest::testImplied));
    tests->addTest(new CppUnit::TestCaller<TermStructureTest>
                   ("Testing observability of implied term structure",
                    &TermStructureTest::testImpliedObs));
    tests->addTest(new CppUnit::TestCaller<TermStructureTest>
                   ("Testing consistency of forward-spreaded term structure",
                    &TermStructureTest::testFSpreaded));
    tests->addTest(new CppUnit::TestCaller<TermStructureTest>
                   ("Testing observability of forward-spreaded term structure",
                    &TermStructureTest::testFSpreadedObs));
    tests->addTest(new CppUnit::TestCaller<TermStructureTest>
                   ("Testing consistency of zero-spreaded term structure",
                    &TermStructureTest::testZSpreaded));
    tests->addTest(new CppUnit::TestCaller<TermStructureTest>
                   ("Testing observability of zero-spreaded term structure",
                    &TermStructureTest::testZSpreadedObs));
    return tests;
}

