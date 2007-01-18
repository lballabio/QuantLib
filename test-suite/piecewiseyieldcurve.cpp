/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 StatPro Italia srl

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
#include <ql/Indexes/indexmanager.hpp>
#include <ql/Instruments/forwardrateagreement.hpp>
#include <ql/Instruments/makevanillaswap.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/loglinearinterpolation.hpp>
#include <ql/Math/cubicspline.hpp>
#ifdef QL_DISABLE_DEPRECATED
#include <ql/Quotes/simplequote.hpp>
#else
#include <ql/quote.hpp>
#endif
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

Datum fraData[] = {
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
Integer settlementDays;
Date today, settlement;
BusinessDayConvention fixedLegConvention;
Frequency fixedLegFrequency;
DayCounter fixedLegDayCounter;
Integer bondSettlementDays;
DayCounter bondDayCounter;
BusinessDayConvention bondConvention;
Real bondRedemption;

Size deposits, fras, swaps, bonds;
std::vector<boost::shared_ptr<SimpleQuote> > rates, fraRates, prices;
std::vector<boost::shared_ptr<RateHelper> > instruments, fraHelpers,
                                            bondHelpers;
boost::shared_ptr<YieldTermStructure> termStructure;

void setup() {

    // data
    calendar = TARGET();
    settlementDays = 2;
    today = calendar.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today;
    settlement = calendar.advance(today,settlementDays,Days);
    fixedLegConvention = Unadjusted;
    fixedLegFrequency = Annual;
    fixedLegDayCounter = Thirty360();
    bondSettlementDays = 3;
    bondDayCounter = ActualActual();
    bondConvention = Following;
    bondRedemption = 100.0;

    deposits = LENGTH(depositData);
    fras = LENGTH(fraData);
    swaps = LENGTH(swapData);
    bonds = LENGTH(bondData);

    // market elements
    rates = std::vector<boost::shared_ptr<SimpleQuote> >(deposits+swaps);
    fraRates = std::vector<boost::shared_ptr<SimpleQuote> >(fras);
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
    for (i=0; i<fras; i++) {
        fraRates[i] = boost::shared_ptr<SimpleQuote>(
                                        new SimpleQuote(fraData[i].rate/100));
    }
    for (i=0; i<bonds; i++) {
        prices[i] = boost::shared_ptr<SimpleQuote>(
                                          new SimpleQuote(bondData[i].price));
    }

    // rate helpers
    instruments =
        std::vector<boost::shared_ptr<RateHelper> >(deposits+swaps);
    fraHelpers =
        std::vector<boost::shared_ptr<RateHelper> >(fras);
    bondHelpers =
        std::vector<boost::shared_ptr<RateHelper> >(bonds);

    boost::shared_ptr<IborIndex> euribor(new Euribor6M);
    for (i=0; i<deposits; i++) {
        Handle<Quote> r(rates[i]);
        instruments[i] = boost::shared_ptr<RateHelper>(
              new DepositRateHelper(r, depositData[i].n*depositData[i].units,
                                    settlementDays, calendar,
                                    euribor->businessDayConvention(),
                                    euribor->dayCounter()));
    }
    for (i=0; i<swaps; i++) {
        Handle<Quote> r(rates[i+deposits]);
        instruments[i+deposits] = boost::shared_ptr<RateHelper>(
                new SwapRateHelper(r, swapData[i].n*swapData[i].units,
                                   settlementDays, calendar,
                                   fixedLegFrequency, fixedLegConvention,
                                   fixedLegDayCounter, euribor));
    }
    Euribor3M euribor3m;
    for (i=0; i<fras; i++) {
        Handle<Quote> r(fraRates[i]);
        fraHelpers[i] = boost::shared_ptr<RateHelper>(
              new FraRateHelper(r, fraData[i].n, fraData[i].n + 3,
                                euribor3m.fixingDays(),
                                euribor3m.calendar(),
                                euribor3m.businessDayConvention(),
                                euribor3m.dayCounter()));
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
    IndexManager::instance().clearHistories();
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
        Euribor index(depositData[i].n*depositData[i].units,curveHandle);
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
    boost::shared_ptr<IborIndex> euribor6m(new Euribor6M(curveHandle));
    for (i=0; i<swaps; i++) {
        Period tenor = swapData[i].n*swapData[i].units;

        VanillaSwap swap = MakeVanillaSwap(tenor, euribor6m, 0.0)
            .withEffectiveDate(settlement)
            .withFixedLegDayCount(fixedLegDayCounter)
            .withFixedLegTenor(Period(fixedLegFrequency))
            .withFixedLegConvention(fixedLegConvention)
            .withFixedLegTerminationDateConvention(fixedLegConvention);

        Rate expectedRate = swapData[i].rate/100,
             estimatedRate = swap.fairRate();
        Rate approximateRate = curveHandle->parRate(swapData[i].n,
            settlement, fixedLegFrequency);
        #ifdef QL_PATCH_BORLAND
        Real tolerance = 1.0e-5;
        #else
        Real tolerance = 1.0e-9;
        #endif
        Spread error = std::fabs(expectedRate-estimatedRate);
        if (error > tolerance) {
            BOOST_ERROR(swapData[i].n << " year(s) swap:\n"
                    << std::setprecision(8)
                    << "\n approximate rate: " << io::rate(approximateRate)
                    << "\n expected rate:    " << io::rate(expectedRate)
                    << "\n error:            " << io::rate(error)
                    << "\n tolerance:        " << io::rate(tolerance));
        }
        Real tolerance2 = 0.00025;
        error = std::fabs(approximateRate-estimatedRate);
        if (error > tolerance2) {
            BOOST_ERROR(swapData[i].n << " year(s) swap:"
                    << std::setprecision(3)
                    << "\n approximate rate: " << io::rate(approximateRate)
                    << "\n expected rate:    " << io::rate(expectedRate)
                    << "\n error:            " << io::rate(error)
                    << "\n tolerance:        " << io::rate(tolerance2));
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

        FixedCouponBond bond(100.0, issue, issue, maturity, bondSettlementDays,
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
                       << estimatedPrice << "\n"
                       << "    expected price:  "
                       << expectedPrice);
        }
    }

    // check FRA
    termStructure = boost::shared_ptr<YieldTermStructure>(
                          new PiecewiseYieldCurve<T,I>(settlement,fraHelpers,
                                                       Actual360(), 1.0e-12,
                                                       interpolator));
    curveHandle.linkTo(termStructure);

    boost::shared_ptr<IborIndex> euribor3m(new Euribor3M(curveHandle));
    for (i=0; i<fras; i++) {
        Date start = calendar.advance(settlement,
                                      fraData[i].n, fraData[i].units,
                                      euribor3m->businessDayConvention());
        Date end = calendar.advance(start, 3, Months,
                                    euribor3m->businessDayConvention());

        ForwardRateAgreement fra(start, end, Position::Long,
                                 fraData[i].rate/100, 100.0,
                                 euribor3m, curveHandle);
        Rate expectedRate = fraData[i].rate/100,
             estimatedRate = fra.forwardRate();
        Real tolerance = 1.0e-9;
        if (std::fabs(expectedRate-estimatedRate) > tolerance) {
            BOOST_ERROR(io::ordinal(i) << " bond:\n"
                       << std::setprecision(8)
                       << "    estimated rate: "
                       << io::rate(estimatedRate) << "\n"
                       << "    expected rate:  "
                       << io::rate(expectedRate));
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


void PiecewiseYieldCurveTest::testLiborFixing() {

    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_MSVC70)

    BOOST_MESSAGE(
        "Testing use of today's LIBOR fixings in swap curve...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    std::vector<boost::shared_ptr<RateHelper> > swapHelpers(swaps);
    boost::shared_ptr<IborIndex> euribor6m(new Euribor6M);
    Size i;

    for (i=0; i<swaps; i++) {
        Handle<Quote> r(rates[i+deposits]);
        swapHelpers[i] = boost::shared_ptr<RateHelper>(
                new SwapRateHelper(r, Period(swapData[i].n, swapData[i].units),
                                   settlementDays, calendar,
                                   fixedLegFrequency, fixedLegConvention,
                                   fixedLegDayCounter, euribor6m));
    }

    termStructure = boost::shared_ptr<YieldTermStructure>(
           new PiecewiseYieldCurve<Discount,LogLinear>(settlement,swapHelpers,
                                                       Actual360(), 1.0e-12));

    Handle<YieldTermStructure> curveHandle;
    curveHandle.linkTo(termStructure);

    boost::shared_ptr<IborIndex> index(new Euribor6M(curveHandle));
    for (i=0; i<swaps; i++) {
        Period tenor = swapData[i].n*swapData[i].units;

        VanillaSwap swap = MakeVanillaSwap(tenor, index, 0.0)
            .withEffectiveDate(settlement)
            .withFixedLegDayCount(fixedLegDayCounter)
            .withFixedLegTenor(Period(fixedLegFrequency))
            .withFixedLegConvention(fixedLegConvention)
            .withFixedLegTerminationDateConvention(fixedLegConvention);

        Rate expectedRate = swapData[i].rate/100,
             estimatedRate = swap.fairRate();
        #ifdef QL_PATCH_BORLAND
        Real tolerance = 1.0e-5;
        #else
        Real tolerance = 1.0e-9;
        #endif
        if (std::fabs(expectedRate-estimatedRate) > tolerance) {
            BOOST_ERROR("before LIBOR fixing:\n"
                        << swapData[i].n << " year(s) swap:\n"
                        << std::setprecision(8)
                        << "    estimated rate: "
                        << io::rate(estimatedRate) << "\n"
                        << "    expected rate:  "
                        << io::rate(expectedRate));
        }
    }

    Flag f;
    f.registerWith(termStructure);
    f.lower();

    index->addFixing(today, 0.0425);

    if (!f.isUp())
        BOOST_ERROR("Observer was not notified of rate fixing");

    for (i=0; i<swaps; i++) {
        Period tenor = swapData[i].n*swapData[i].units;

        VanillaSwap swap = MakeVanillaSwap(tenor, index, 0.0)
            .withEffectiveDate(settlement)
            .withFixedLegDayCount(fixedLegDayCounter)
            .withFixedLegTenor(Period(fixedLegFrequency))
            .withFixedLegConvention(fixedLegConvention)
            .withFixedLegTerminationDateConvention(fixedLegConvention);

        Rate expectedRate = swapData[i].rate/100,
             estimatedRate = swap.fairRate();
        #ifdef QL_PATCH_BORLAND
        Real tolerance = 1.0e-5;
        #else
        Real tolerance = 1.0e-9;
        #endif
        if (std::fabs(expectedRate-estimatedRate) > tolerance) {
            BOOST_ERROR("after LIBOR fixing:\n"
                        << swapData[i].n << " year(s) swap:\n"
                        << std::setprecision(8)
                        << "    estimated rate: "
                        << io::rate(estimatedRate) << "\n"
                        << "    expected rate:  "
                        << io::rate(expectedRate));
        }
    }

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
    suite->add(BOOST_TEST_CASE(&PiecewiseYieldCurveTest::testLiborFixing));
    #endif
    return suite;
}

