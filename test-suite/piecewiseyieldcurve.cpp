
/*
 Copyright (C) 2005 StatPro Italia srl

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

#include "piecewiseyieldcurve.hpp"
#include "utilities.hpp"
#include <ql/TermStructures/piecewiseyieldcurve.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/loglinearinterpolation.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct Datum {
        Integer n;
        TimeUnit units;
        Rate rate;
    };

    Datum depositData[] = {
        { 1, Weeks,  4.559 },
        { 1, Months, 4.581 },
        { 2, Months, 4.573 },
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
    BusinessDayConvention depoConvention;
    DayCounter depoDayCounter;
    BusinessDayConvention fixedLegConvention, floatingLegConvention;
    Frequency fixedLegFrequency;
    DayCounter fixedLegDayCounter;
    Frequency floatingLegFrequency;

    Size deposits, swaps;
    std::vector<boost::shared_ptr<SimpleQuote> > rates;
    std::vector<boost::shared_ptr<RateHelper> > instruments;
    boost::shared_ptr<YieldTermStructure> termStructure;

    void setup() {

        // data
        calendar = TARGET();
        settlementDays = 2;
        fixingDays = 2;
        today = calendar.adjust(Date::todaysDate());
        Settings::instance().setEvaluationDate(today);
        settlement = calendar.advance(today,settlementDays,Days);
        depoConvention = ModifiedFollowing;
        depoDayCounter = Actual360();
        fixedLegConvention = Unadjusted;
        floatingLegConvention = ModifiedFollowing;
        fixedLegFrequency = Annual;
        fixedLegDayCounter = Thirty360();
        floatingLegFrequency = Semiannual;

        deposits = LENGTH(depositData);
        swaps = LENGTH(swapData);

        // market elements
        rates = std::vector<boost::shared_ptr<SimpleQuote> >(deposits+swaps);
        Size i;
        for (i=0; i<deposits; i++) {
            rates[i] = boost::shared_ptr<SimpleQuote>(
                                    new SimpleQuote(depositData[i].rate/100));
        }
        for (i=0; i<swaps; i++) {
            rates[i+deposits] = boost::shared_ptr<SimpleQuote>(
                                       new SimpleQuote(swapData[i].rate/100));
        }

        // rate helpers
        instruments =
            std::vector<boost::shared_ptr<RateHelper> >(deposits+swaps);
        for (i=0; i<deposits; i++) {
            Handle<Quote> r(rates[i]);
            instruments[i] = boost::shared_ptr<RateHelper>(
              new DepositRateHelper(r, depositData[i].n, depositData[i].units,
                                    settlementDays, calendar,
                                    depoConvention, depoDayCounter));
        }
        for (i=0; i<swaps; i++) {
            Handle<Quote> r(rates[i+deposits]);
            instruments[i+deposits] = boost::shared_ptr<RateHelper>(
                new SwapRateHelper(r, swapData[i].n, swapData[i].units,
                                   settlementDays, calendar,
                                   fixedLegFrequency, fixedLegConvention,
                                   fixedLegDayCounter, floatingLegFrequency,
                                   floatingLegConvention));
        }
    }

    void teardown() {
        Settings::instance().setEvaluationDate(Date());
    }

}

namespace {

    template <class T, class I>
    void testCurveConsistency(const T&, const I&) {

        termStructure = boost::shared_ptr<YieldTermStructure>(
                          new PiecewiseYieldCurve<T,I>(settlement,instruments,
                                                       Actual360()));

        Handle<YieldTermStructure> euriborHandle;
        euriborHandle.linkTo(termStructure);

        Size i;
        // check deposits
        for (i=0; i<deposits; i++) {
            Euribor index(depositData[i].n,depositData[i].units,euriborHandle);
            Rate expectedRate  = depositData[i].rate/100,
                 estimatedRate = index.fixing(today);
            if (std::fabs(expectedRate-estimatedRate) > 1.0e-9) {
                BOOST_FAIL(
                    depositData[i].n << " "
                    << (depositData[i].units == Weeks ? "week(s)" : "month(s)")
                    << " deposit:"
                    << std::setprecision(8)
                    << "\n    estimated rate: " << io::rate(estimatedRate)
                    << "\n    expected rate:  " << io::rate(expectedRate));
            }
        }

        // check swaps
        boost::shared_ptr<Xibor> index(new Euribor(12/floatingLegFrequency,
                                                   Months, euriborHandle));
        for (i=0; i<swaps; i++) {
            Date maturity = calendar.advance(settlement,
                                             swapData[i].n,swapData[i].units,
                                             floatingLegConvention);
            Schedule fixedSchedule(calendar,settlement,maturity,
                                   fixedLegFrequency,fixedLegConvention);
            Schedule floatSchedule(calendar,settlement,maturity,
                                   floatingLegFrequency,floatingLegConvention);
            SimpleSwap swap(true,100.0,
                            fixedSchedule,0.0,fixedLegDayCounter,
                            floatSchedule,index,fixingDays,0.0,
                            euriborHandle);
            Rate expectedRate = swapData[i].rate/100,
                 estimatedRate = swap.fairRate();
            if (std::fabs(expectedRate-estimatedRate) > 1.0e-9) {
                BOOST_FAIL(swapData[i].n << " year(s) swap:\n"
                           << std::setprecision(8)
                           << "    estimated rate: "
                           << io::rate(estimatedRate) << "\n"
                           << "    expected rate:  "
                           << io::rate(expectedRate));
            }
        }
    }

}

void PiecewiseYieldCurveTest::testLogLinearDiscountConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-log-linear discount curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(Discount(), LogLinear());

    QL_TEST_TEARDOWN
}

void PiecewiseYieldCurveTest::testLinearDiscountConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-linear discount curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(Discount(), Linear());

    QL_TEST_TEARDOWN
}

void PiecewiseYieldCurveTest::testLogLinearZeroConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-log-linear zero-yield curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(ZeroYield(), LogLinear());

    QL_TEST_TEARDOWN
}

void PiecewiseYieldCurveTest::testLinearZeroConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-linear zero-yield curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(ZeroYield(), Linear());

    QL_TEST_TEARDOWN
}

void PiecewiseYieldCurveTest::testObservability() {

    BOOST_MESSAGE("Testing observability of piecewise yield curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    termStructure = boost::shared_ptr<YieldTermStructure>(
       new PiecewiseYieldCurve<Discount,LogLinear>(settlementDays, calendar,
                                                   instruments, Actual360()));

    Flag f;
    f.registerWith(termStructure);

    for (Size i=0; i<deposits+swaps; i++) {
        f.lower();
        rates[i]->setValue(rates[i]->value()*1.01);
        if (!f.isUp())
            BOOST_FAIL("Observer was not notified of underlying rate change");
    }

    f.lower();
    Settings::instance().setEvaluationDate(calendar.advance(today,1,Months));
    if (!f.isUp())
            BOOST_FAIL("Observer was not notified of date change");

    QL_TEST_TEARDOWN
}


test_suite* PiecewiseYieldCurveTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Piecewise yield curve tests");
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLogLinearDiscountConsistency));
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLinearDiscountConsistency));
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLogLinearZeroConsistency));
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLinearZeroConsistency));
    suite->add(BOOST_TEST_CASE(&PiecewiseYieldCurveTest::testObservability));
    return suite;
}

