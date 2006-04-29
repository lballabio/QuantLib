/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "piecewiseyieldcurve.hpp"
#include "utilities.hpp"
#include <ql/TermStructures/piecewiseyieldcurve.hpp>
#include <ql/TermStructures/ratehelpers.hpp>
#include <ql/TermStructures/bondhelpers.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/DayCounters/actualactual.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/loglinearinterpolation.hpp>
#include <ql/Math/cubicspline.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(PiecewiseYieldCurveTest)

struct Datum {
    Integer n;
    TimeUnit units;
    Rate rate;
};

struct BondDatum {
    Integer n;
    TimeUnit units;
    Integer length;
    Frequency frequency;
    Rate coupon;
    Real price;
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

BondDatum bondData[] = {
    {  6, Months, 5, Semiannual, 4.75, 101.320 },
    {  1, Years,  3, Semiannual, 2.75, 100.590 },
    {  2, Years,  5, Semiannual, 5.00, 105.650 },
    {  5, Years, 11, Semiannual, 5.50, 113.610 },
    { 10, Years, 11, Semiannual, 3.75, 104.070 }
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
Integer bondSettlementDays;
DayCounter bondDayCounter;
BusinessDayConvention bondConvention;
Real bondRedemption;

Size deposits, swaps, bonds;
std::vector<boost::shared_ptr<SimpleQuote> > rates, prices;
std::vector<boost::shared_ptr<RateHelper> > instruments, bondHelpers;
boost::shared_ptr<YieldTermStructure> termStructure;

void setup() {

    // data
    calendar = TARGET();
    settlementDays = 2;
    fixingDays = 2;
    today = calendar.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today;
    settlement = calendar.advance(today,settlementDays,Days);
    depoConvention = ModifiedFollowing;
    depoDayCounter = Actual360();
    fixedLegConvention = Unadjusted;
    floatingLegConvention = ModifiedFollowing;
    fixedLegFrequency = Annual;
    fixedLegDayCounter = Thirty360();
    floatingLegFrequency = Semiannual;
    bondSettlementDays = 3;
    bondDayCounter = ActualActual();
    bondConvention = Following;
    bondRedemption = 100.0;

    deposits = LENGTH(depositData);
    swaps = LENGTH(swapData);
    bonds = LENGTH(bondData);

    // market elements
    rates = std::vector<boost::shared_ptr<SimpleQuote> >(deposits+swaps);
    prices = std::vector<boost::shared_ptr<SimpleQuote> >(bonds);
    Size i;
    for (i=0; i<deposits; i++) {
        rates[i] = boost::shared_ptr<SimpleQuote>(
                                    new SimpleQuote(depositData[i].rate/100));
    }
    for (i=0; i<swaps; i++) {
        rates[i+deposits] = boost::shared_ptr<SimpleQuote>(
                                       new SimpleQuote(swapData[i].rate/100));
    }
    for (i=0; i<bonds; i++) {
        prices[i] = boost::shared_ptr<SimpleQuote>(
                                    new SimpleQuote(bondData[i].price));
    }

    // rate helpers
    instruments =
        std::vector<boost::shared_ptr<RateHelper> >(deposits+swaps);
    bondHelpers =
        std::vector<boost::shared_ptr<RateHelper> >(bonds);
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
                                   floatingLegConvention, Actual360()));
    }
    for (i=0; i<bonds; i++) {
        Handle<Quote> p(prices[i]);
        Date maturity =
            calendar.advance(today, bondData[i].n, bondData[i].units);
        Date issue = calendar.advance(maturity, -bondData[i].length, Years);
        std::vector<Rate> coupons(1, bondData[i].coupon/100.0);
        bondHelpers[i] = boost::shared_ptr<RateHelper>(
                          new FixedCouponBondHelper(p, issue, issue, maturity,
                                                    bondSettlementDays,
                                                    coupons,
                                                    bondData[i].frequency,
                                                    calendar, bondDayCounter,
                                                    bondConvention,
                                                    bondConvention,
                                                    bondRedemption));
    }
}

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

#if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)

template <class T, class I>
void testCurveConsistency(const T&, const I& interpolator) {

    termStructure = boost::shared_ptr<YieldTermStructure>(
                          new PiecewiseYieldCurve<T,I>(settlement,instruments,
                                                       Actual360(), 1.0e-12,
                                                       interpolator));

    Handle<YieldTermStructure> curveHandle;
    curveHandle.linkTo(termStructure);

    Size i;
    // check deposits
    for (i=0; i<deposits; i++) {
        Euribor index(depositData[i].n,depositData[i].units,curveHandle);
        Rate expectedRate  = depositData[i].rate/100,
             estimatedRate = index.fixing(today);
        if (std::fabs(expectedRate-estimatedRate) > 1.0e-9) {
            BOOST_ERROR(
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
                                               Months, curveHandle));
    for (i=0; i<swaps; i++) {
        Date maturity = settlement + swapData[i].n*swapData[i].units;
        Schedule fixedSchedule(calendar,settlement,maturity,
                               fixedLegFrequency,fixedLegConvention);
        Schedule floatSchedule(calendar,settlement,maturity,
                               floatingLegFrequency,floatingLegConvention);
        VanillaSwap swap(true,100.0,
                         fixedSchedule,0.0,fixedLegDayCounter,
                         floatSchedule,index,fixingDays,0.0,
                         Actual360(),curveHandle);
        Rate expectedRate = swapData[i].rate/100,
             estimatedRate = swap.fairRate();
        #ifdef QL_PATCH_BORLAND
        Real tolerance = 1.0e-5;
        #else
        Real tolerance = 1.0e-9;
        #endif
        if (std::fabs(expectedRate-estimatedRate) > tolerance) {
            BOOST_ERROR(swapData[i].n << " year(s) swap:\n"
                       << std::setprecision(8)
                       << "    estimated rate: "
                       << io::rate(estimatedRate) << "\n"
                       << "    expected rate:  "
                       << io::rate(expectedRate));
        }
    }

    // check bonds
    termStructure = boost::shared_ptr<YieldTermStructure>(
                          new PiecewiseYieldCurve<T,I>(settlement,bondHelpers,
                                                       Actual360(), 1.0e-12,
                                                       interpolator));
    curveHandle.linkTo(termStructure);

    for (i=0; i<bonds; i++) {
        Date maturity =
            calendar.advance(today, bondData[i].n, bondData[i].units);
        Date issue = calendar.advance(maturity, -bondData[i].length, Years);
        std::vector<Rate> coupons(1, bondData[i].coupon/100.0);

        FixedCouponBond bond(issue, issue, maturity, bondSettlementDays,
                             coupons, bondData[i].frequency,
                             calendar, bondDayCounter,
                             bondConvention, bondConvention,
                             bondRedemption, curveHandle);
        Real expectedPrice = bondData[i].price,
             estimatedPrice = bond.cleanPrice();
        Real tolerance = 1.0e-9;
        if (std::fabs(expectedPrice-estimatedPrice) > tolerance) {
            BOOST_ERROR(io::ordinal(i) << " bond:\n"
                       << std::setprecision(8)
                       << "    estimated price: "
                       << io::rate(estimatedPrice) << "\n"
                       << "    expected price:  "
                       << io::rate(expectedPrice));
        }
    }
}

#endif

QL_END_TEST_LOCALS(PiecewiseYieldCurveTest)


void PiecewiseYieldCurveTest::testLogLinearDiscountConsistency() {

    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)

    BOOST_MESSAGE(
        "Testing consistency of piecewise-log-linear discount curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(Discount(), LogLinear());

    QL_TEST_TEARDOWN

    #endif
}

void PiecewiseYieldCurveTest::testLinearDiscountConsistency() {

    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)

    BOOST_MESSAGE(
        "Testing consistency of piecewise-linear discount curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(Discount(), Linear());

    QL_TEST_TEARDOWN

    #endif
}

void PiecewiseYieldCurveTest::testLogLinearZeroConsistency() {

    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)

    BOOST_MESSAGE(
        "Testing consistency of piecewise-log-linear zero-yield curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(ZeroYield(), LogLinear());

    QL_TEST_TEARDOWN

    #endif
}

void PiecewiseYieldCurveTest::testLinearZeroConsistency() {

    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)

    BOOST_MESSAGE(
        "Testing consistency of piecewise-linear zero-yield curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(ZeroYield(), Linear());

    QL_TEST_TEARDOWN

    #endif
}

void PiecewiseYieldCurveTest::testSplineZeroConsistency() {

    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)

    BOOST_MESSAGE(
        "Testing consistency of piecewise-spline zero-yield curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(ZeroYield(),
                         Cubic(CubicSpline::SecondDerivative,0.0,
                               CubicSpline::SecondDerivative,0.0,
                               true));

    QL_TEST_TEARDOWN

    #endif
}

void PiecewiseYieldCurveTest::testLinearForwardConsistency() {

    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)

    BOOST_MESSAGE(
        "Testing consistency of piecewise-linear forward-rate curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(ForwardRate(), Linear());

    QL_TEST_TEARDOWN

    #endif
}

void PiecewiseYieldCurveTest::testFlatForwardConsistency() {

    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)

    BOOST_MESSAGE(
        "Testing consistency of piecewise-flat forward-rate curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(ForwardRate(), BackwardFlat());

    QL_TEST_TEARDOWN

    #endif
}

void PiecewiseYieldCurveTest::testSplineForwardConsistency() {

    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)

    BOOST_MESSAGE(
        "Testing consistency of piecewise-spline forward-rate curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    testCurveConsistency(ForwardRate(),
                         Cubic(CubicSpline::SecondDerivative,0.0,
                               CubicSpline::SecondDerivative,0.0,
                               true));

    QL_TEST_TEARDOWN

    #endif
}

void PiecewiseYieldCurveTest::testObservability() {

    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)

    BOOST_MESSAGE("Testing observability of piecewise yield curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    termStructure = boost::shared_ptr<YieldTermStructure>(
       new PiecewiseYieldCurve<Discount,LogLinear>(settlementDays, calendar,
                                                   instruments, Actual360()));
    Flag f;
    f.registerWith(termStructure);

    for (Size i=0; i<deposits+swaps; i++) {
        Time testTime = Actual360().yearFraction(settlement,
                                                 instruments[i]->latestDate());
        DiscountFactor discount = termStructure->discount(testTime);
        f.lower();
        rates[i]->setValue(rates[i]->value()*1.01);
        if (!f.isUp())
            BOOST_FAIL("Observer was not notified of underlying rate change");
        if (termStructure->discount(testTime,true) == discount)
            BOOST_FAIL("rate change did not trigger recalculation");
        rates[i]->setValue(rates[i]->value()/1.01);
    }

    f.lower();
    Settings::instance().evaluationDate() = calendar.advance(today,15,Days);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of date change");

    QL_TEST_TEARDOWN

    #endif
}


test_suite* PiecewiseYieldCurveTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Piecewise yield curve tests");
    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLogLinearDiscountConsistency));
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLinearDiscountConsistency));
    #if !defined(QL_USE_INDEXED_COUPON)
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLogLinearZeroConsistency));
    #endif
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLinearZeroConsistency));
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testSplineZeroConsistency));
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLinearForwardConsistency));
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testFlatForwardConsistency));
    // alas, doesn't bootstrap
    // suite->add(BOOST_TEST_CASE(
    //              &PiecewiseYieldCurveTest::testSplineForwardConsistency));
    suite->add(BOOST_TEST_CASE(&PiecewiseYieldCurveTest::testObservability));
    #endif
    return suite;
}

