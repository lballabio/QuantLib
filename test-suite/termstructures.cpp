/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "termstructures.hpp"
#include "utilities.hpp"
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/impliedtermstructure.hpp>
#include <ql/termstructures/yield/forwardspreadedtermstructure.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/math/comparison.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/currency.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct Datum {
        Integer n;
        TimeUnit units;
        Rate rate;
    };

    struct CommonVars {
        // common data
        Calendar calendar;
        Natural settlementDays;
        boost::shared_ptr<YieldTermStructure> termStructure;
        boost::shared_ptr<YieldTermStructure> dummyTermStructure;

        // cleanup
        SavedSettings backup;

        // setup
        CommonVars() {
            calendar = TARGET();
            settlementDays = 2;
            Date today = calendar.adjust(Date::todaysDate());
            Settings::instance().evaluationDate() = today;
            Date settlement = calendar.advance(today,settlementDays,Days);
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

            std::vector<boost::shared_ptr<RateHelper> > instruments(
                                                              deposits+swaps);
            for (Size i=0; i<deposits; i++) {
                instruments[i] = boost::shared_ptr<RateHelper>(new
                    DepositRateHelper(depositData[i].rate/100,
                                      depositData[i].n*depositData[i].units,
                                      settlementDays, calendar,
                                      ModifiedFollowing, true,
                                      Actual360()));
            }
            boost::shared_ptr<IborIndex> index(new IborIndex("dummy",
                                                             6*Months,
                                                             settlementDays,
                                                             Currency(),
                                                             calendar,
                                                             ModifiedFollowing,
                                                             false,
                                                             Actual360()));
            for (Size i=0; i<swaps; ++i) {
                instruments[i+deposits] = boost::shared_ptr<RateHelper>(new
                    SwapRateHelper(swapData[i].rate/100,
                                   swapData[i].n*swapData[i].units,
                                   calendar,
                                   Annual, Unadjusted, Thirty360(),
                                   index));
            }
            termStructure = boost::shared_ptr<YieldTermStructure>(new
                PiecewiseYieldCurve<Discount,LogLinear>(settlement,
                                                        instruments, Actual360()));
            dummyTermStructure = boost::shared_ptr<YieldTermStructure>(new
                PiecewiseYieldCurve<Discount,LogLinear>(settlement,
                                                        instruments, Actual360()));
        }
    };

}


void TermStructureTest::testReferenceChange() {

    BOOST_TEST_MESSAGE("Testing term structure against evaluation date change...");

    CommonVars vars;

    boost::shared_ptr<SimpleQuote> flatRate (new SimpleQuote);
    Handle<Quote> flatRateHandle(flatRate);
    vars.termStructure = boost::shared_ptr<YieldTermStructure>(
                          new FlatForward(vars.settlementDays, NullCalendar(),
                                          flatRateHandle, Actual360()));
    Date today = Settings::instance().evaluationDate();
    flatRate->setValue(.03);
    Integer days[] = { 10, 30, 60, 120, 360, 720 };
    Size i;

    std::vector<DiscountFactor> expected(LENGTH(days));
    for (i=0; i<LENGTH(days); i++)
        expected[i] = vars.termStructure->discount(today+days[i]);

    Settings::instance().evaluationDate() = today+30;
    std::vector<DiscountFactor> calculated(LENGTH(days));
    for (i=0; i<LENGTH(days); i++)
        calculated[i] = vars.termStructure->discount(today+30+days[i]);

    for (i=0; i<LENGTH(days); i++) {
        if (!close(expected[i],calculated[i]))
            BOOST_ERROR("\n  Discount at " << days[i] << " days:\n"
                        << std::setprecision(12)
                        << "    before date change: " << expected[i] << "\n"
                        << "    after date change:  " << calculated[i]);
    }
}


void TermStructureTest::testImplied() {

    BOOST_TEST_MESSAGE("Testing consistency of implied term structure...");

    CommonVars vars;

    Real tolerance = 1.0e-10;
    Date today = Settings::instance().evaluationDate();
    Date newToday = today + 3*Years;
    Date newSettlement = vars.calendar.advance(newToday,
                                               vars.settlementDays,Days);
    Date testDate = newSettlement + 5*Years;
    boost::shared_ptr<YieldTermStructure> implied(
        new ImpliedTermStructure(Handle<YieldTermStructure>(vars.termStructure),
                                 newSettlement));
    DiscountFactor baseDiscount = vars.termStructure->discount(newSettlement);
    DiscountFactor discount = vars.termStructure->discount(testDate);
    DiscountFactor impliedDiscount = implied->discount(testDate);
    if (std::fabs(discount - baseDiscount*impliedDiscount) > tolerance)
        BOOST_ERROR(
            "unable to reproduce discount from implied curve\n"
            << QL_FIXED << std::setprecision(10)
            << "    calculated: " << baseDiscount*impliedDiscount << "\n"
            << "    expected:   " << discount);
}

void TermStructureTest::testImpliedObs() {

    BOOST_TEST_MESSAGE("Testing observability of implied term structure...");

    CommonVars vars;

    Date today = Settings::instance().evaluationDate();
    Date newToday = today + 3*Years;
    Date newSettlement = vars.calendar.advance(newToday,
                                               vars.settlementDays,Days);
    RelinkableHandle<YieldTermStructure> h;
    boost::shared_ptr<YieldTermStructure> implied(
                                  new ImpliedTermStructure(h, newSettlement));
    Flag flag;
    flag.registerWith(implied);
    h.linkTo(vars.termStructure);
    if (!flag.isUp())
        BOOST_ERROR("Observer was not notified of term structure change");
}

void TermStructureTest::testFSpreaded() {

    BOOST_TEST_MESSAGE("Testing consistency of forward-spreaded term structure...");

    CommonVars vars;

    Real tolerance = 1.0e-10;
    boost::shared_ptr<Quote> me(new SimpleQuote(0.01));
    Handle<Quote> mh(me);
    boost::shared_ptr<YieldTermStructure> spreaded(
        new ForwardSpreadedTermStructure(
            Handle<YieldTermStructure>(vars.termStructure),mh));
    Date testDate = vars.termStructure->referenceDate() + 5*Years;
    DayCounter tsdc  = vars.termStructure->dayCounter();
    DayCounter sprdc = spreaded->dayCounter();
    Rate forward = vars.termStructure->forwardRate(testDate, testDate, tsdc,
                                                   Continuous, NoFrequency);
    Rate spreadedForward = spreaded->forwardRate(testDate, testDate, sprdc,
                                                 Continuous, NoFrequency);
    if (std::fabs(forward - (spreadedForward-me->value())) > tolerance)
        BOOST_ERROR(
            "unable to reproduce forward from spreaded curve\n"
            << std::setprecision(10)
            << "    calculated: "
            << io::rate(spreadedForward-me->value()) << "\n"
            << "    expected:   " << io::rate(forward));
}

void TermStructureTest::testFSpreadedObs() {

    BOOST_TEST_MESSAGE("Testing observability of forward-spreaded "
                       "term structure...");

    CommonVars vars;

    boost::shared_ptr<SimpleQuote> me(new SimpleQuote(0.01));
    Handle<Quote> mh(me);
    RelinkableHandle<YieldTermStructure> h; //(vars.dummyTermStructure);
    boost::shared_ptr<YieldTermStructure> spreaded(
        new ForwardSpreadedTermStructure(h,mh));
    Flag flag;
    flag.registerWith(spreaded);
    h.linkTo(vars.termStructure);
    if (!flag.isUp())
        BOOST_ERROR("Observer was not notified of term structure change");
    flag.lower();
    me->setValue(0.005);
    if (!flag.isUp())
        BOOST_ERROR("Observer was not notified of spread change");
}

void TermStructureTest::testZSpreaded() {

    BOOST_TEST_MESSAGE("Testing consistency of zero-spreaded term structure...");

    CommonVars vars;

    Real tolerance = 1.0e-10;
    boost::shared_ptr<Quote> me(new SimpleQuote(0.01));
    Handle<Quote> mh(me);
    boost::shared_ptr<YieldTermStructure> spreaded(
        new ZeroSpreadedTermStructure(
            Handle<YieldTermStructure>(vars.termStructure),mh));
    Date testDate = vars.termStructure->referenceDate() + 5*Years;
    DayCounter rfdc  = vars.termStructure->dayCounter();
    Rate zero = vars.termStructure->zeroRate(testDate, rfdc,
                                             Continuous, NoFrequency);
    Rate spreadedZero = spreaded->zeroRate(testDate, rfdc,
                                           Continuous, NoFrequency);
    if (std::fabs(zero - (spreadedZero-me->value())) > tolerance)
        BOOST_ERROR(
            "unable to reproduce zero yield from spreaded curve\n"
            << std::setprecision(10)
            << "    calculated: " << io::rate(spreadedZero-me->value()) << "\n"
            << "    expected:   " << io::rate(zero));
}

void TermStructureTest::testZSpreadedObs() {

    BOOST_TEST_MESSAGE("Testing observability of zero-spreaded term structure...");

    CommonVars vars;

    boost::shared_ptr<SimpleQuote> me(new SimpleQuote(0.01));
    Handle<Quote> mh(me);
    RelinkableHandle<YieldTermStructure> h(vars.dummyTermStructure);

    boost::shared_ptr<YieldTermStructure> spreaded(
        new ZeroSpreadedTermStructure(h,mh));
    Flag flag;
    flag.registerWith(spreaded);
    h.linkTo(vars.termStructure);
    if (!flag.isUp())
        BOOST_ERROR("Observer was not notified of term structure change");
    flag.lower();
    me->setValue(0.005);
    if (!flag.isUp())
        BOOST_ERROR("Observer was not notified of spread change");
}

void TermStructureTest::testLinkToNullUnderlying() {
    BOOST_TEST_MESSAGE(
        "Testing that an underlying curve can be relinked to "
        "a null underlying curve...");

    CommonVars vars;

    Handle<Quote> spread(boost::shared_ptr<Quote>(new SimpleQuote(0.01)));
    RelinkableHandle<YieldTermStructure> underlying(vars.termStructure);
    boost::shared_ptr<YieldTermStructure> spreaded(
        new ZeroSpreadedTermStructure(underlying,spread));
    // check that we can use it
    vars.termStructure->referenceDate();
    // if we do this, the curve can't work anymore. But it shouldn't
    // throw as long as we don't try to use it.
    underlying.linkTo(boost::shared_ptr<YieldTermStructure>());
}

test_suite* TermStructureTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Term structure tests");
    suite->add(QUANTLIB_TEST_CASE(&TermStructureTest::testReferenceChange));
    suite->add(QUANTLIB_TEST_CASE(&TermStructureTest::testImplied));
    suite->add(QUANTLIB_TEST_CASE(&TermStructureTest::testImpliedObs));
    suite->add(QUANTLIB_TEST_CASE(&TermStructureTest::testFSpreaded));
    suite->add(QUANTLIB_TEST_CASE(&TermStructureTest::testFSpreadedObs));
    suite->add(QUANTLIB_TEST_CASE(&TermStructureTest::testZSpreaded));
    suite->add(QUANTLIB_TEST_CASE(&TermStructureTest::testZSpreadedObs));
    suite->add(QUANTLIB_TEST_CASE(
                             &TermStructureTest::testLinkToNullUnderlying));
    return suite;
}

