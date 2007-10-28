/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007 StatPro Italia srl

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

#include "piecewiseyieldcurve.hpp"
#include "utilities.hpp"
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/yield/bondhelpers.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/indexmanager.hpp>
#include <ql/instruments/forwardrateagreement.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/math/interpolations/cubicspline.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
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
Natural settlementDays;
Date today, settlement;
BusinessDayConvention fixedLegConvention;
Frequency fixedLegFrequency;
DayCounter fixedLegDayCounter;
Natural bondSettlementDays;
DayCounter bondDayCounter;
BusinessDayConvention bondConvention;
Real bondRedemption;

Size deposits, fras, swaps, bonds;
std::vector<boost::shared_ptr<SimpleQuote> > rates, fraRates, prices;
std::vector<boost::shared_ptr<RateHelper> > instruments, fraHelpers,
                                            bondHelpers;
std::vector<Schedule> schedules;
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
    for (Size i=0; i<deposits; i++) {
        rates[i] = boost::shared_ptr<SimpleQuote>(
                                    new SimpleQuote(depositData[i].rate/100));
    }
    for (Size i=0; i<swaps; i++) {
        rates[i+deposits] = boost::shared_ptr<SimpleQuote>(
                                       new SimpleQuote(swapData[i].rate/100));
    }
    for (Size i=0; i<fras; i++) {
        fraRates[i] = boost::shared_ptr<SimpleQuote>(
                                        new SimpleQuote(fraData[i].rate/100));
    }
    for (Size i=0; i<bonds; i++) {
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
    schedules = std::vector<Schedule>(bonds);

    boost::shared_ptr<IborIndex> euribor(new Euribor6M);
    for (Size i=0; i<deposits; i++) {
        Handle<Quote> r(rates[i]);
        instruments[i] = boost::shared_ptr<RateHelper>(
              new DepositRateHelper(r, depositData[i].n*depositData[i].units,
                                    settlementDays, calendar,
                                    euribor->businessDayConvention(),
                                    euribor->endOfMonth(),
                                    euribor->fixingDays(),
                                    euribor->dayCounter()));
    }
    for (Size i=0; i<swaps; i++) {
        Handle<Quote> r(rates[i+deposits]);
        instruments[i+deposits] = boost::shared_ptr<RateHelper>(
                new SwapRateHelper(r, swapData[i].n*swapData[i].units,
                                   settlementDays, calendar,
                                   fixedLegFrequency, fixedLegConvention,
                                   fixedLegDayCounter, euribor));
    }
    Euribor3M euribor3m;
    for (Size i=0; i<fras; i++) {
        Handle<Quote> r(fraRates[i]);
        fraHelpers[i] = boost::shared_ptr<RateHelper>(new
            FraRateHelper(r, fraData[i].n, fraData[i].n + 3,
                                euribor3m.fixingDays(),
                                euribor3m.fixingCalendar(),
                                euribor3m.businessDayConvention(),
                                euribor3m.endOfMonth(),
                                euribor3m.fixingDays(),
                                euribor3m.dayCounter()));
    }
    for (Size i=0; i<bonds; i++) {
        Handle<Quote> p(prices[i]);
        Date maturity =
            calendar.advance(today, bondData[i].n, bondData[i].units);
        Date issue = calendar.advance(maturity, -bondData[i].length, Years);
        std::vector<Rate> coupons(1, bondData[i].coupon/100.0);
        schedules[i] = Schedule(issue, maturity,
                                Period(bondData[i].frequency),
                                calendar,
                                bondConvention, bondConvention,
                                true, false);
        bondHelpers[i] = boost::shared_ptr<RateHelper>(new
            FixedRateBondHelper(p,
                                  bondSettlementDays,
                                  schedules[i],
                                  coupons, bondDayCounter,
                                  bondConvention,
                                  bondRedemption, issue));
    }
}

template <class T, class I>
void testCurveConsistency(const T&, const I& interpolator) {

    termStructure = boost::shared_ptr<YieldTermStructure>(
                          new PiecewiseYieldCurve<T,I>(settlement,instruments,
                                                       Actual360(), 1.0e-12,
                                                       interpolator));

    RelinkableHandle<YieldTermStructure> curveHandle;
    curveHandle.linkTo(termStructure);

    // check deposits
    for (Size i=0; i<deposits; i++) {
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
    for (Size i=0; i<swaps; i++) {
        Period tenor = swapData[i].n*swapData[i].units;

        VanillaSwap swap = MakeVanillaSwap(tenor, euribor6m, 0.0)
            .withEffectiveDate(settlement)
            .withFixedLegDayCount(fixedLegDayCounter)
            .withFixedLegTenor(Period(fixedLegFrequency))
            .withFixedLegConvention(fixedLegConvention)
            .withFixedLegTerminationDateConvention(fixedLegConvention);

        Rate expectedRate = swapData[i].rate/100,
             estimatedRate = swap.fairRate();
        Real tolerance = 1.0e-9;
        Spread error = std::fabs(expectedRate-estimatedRate);
        if (error > tolerance) {
            BOOST_ERROR(swapData[i].n << " year(s) swap:\n"
                    << std::setprecision(8)
                    << "\n estimated rate: " << io::rate(estimatedRate)
                    << "\n expected rate:  " << io::rate(expectedRate)
                    << "\n error:          " << io::rate(error)
                    << "\n tolerance:      " << io::rate(tolerance));
        }
    }

    // check bonds
    termStructure = boost::shared_ptr<YieldTermStructure>(
                          new PiecewiseYieldCurve<T,I>(settlement,bondHelpers,
                                                       Actual360(), 1.0e-12,
                                                       interpolator));
    curveHandle.linkTo(termStructure);

    for (Size i=0; i<bonds; i++) {
        Date maturity =
            calendar.advance(today, bondData[i].n, bondData[i].units);
        Date issue = calendar.advance(maturity, -bondData[i].length, Years);
        std::vector<Rate> coupons(1, bondData[i].coupon/100.0);

        FixedRateBond bond(bondSettlementDays, 100.0, schedules[i],
                           coupons, bondDayCounter, bondConvention,
                           bondRedemption, issue);

        boost::shared_ptr<PricingEngine> bondEngine(
                                      new DiscountingBondEngine(curveHandle));
        bond.setPricingEngine(bondEngine);

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
    for (Size i=0; i<fras; i++) {
        Date start = calendar.advance(settlement,
                                      fraData[i].n, fraData[i].units,
                                      euribor3m->businessDayConvention(),
                                      euribor3m->endOfMonth());
        Date end = calendar.advance(start, 3, Months,
                                    euribor3m->businessDayConvention(),
                                    euribor3m->endOfMonth());

        ForwardRateAgreement fra(start, end, Position::Long,
                                 fraData[i].rate/100, 100.0,
                                 euribor3m, curveHandle);
        Rate expectedRate = fraData[i].rate/100,
             estimatedRate = fra.forwardRate();
        Real tolerance = 1.0e-9;
        if (std::fabs(expectedRate-estimatedRate) > tolerance) {
            BOOST_ERROR(io::ordinal(i) << " FRA:\n"
                       << std::setprecision(8)
                       << "    estimated rate: "
                       << io::rate(estimatedRate) << "\n"
                       << "    expected rate:  "
                       << io::rate(expectedRate));
        }
    }

}

QL_END_TEST_LOCALS(PiecewiseYieldCurveTest)


void PiecewiseYieldCurveTest::testLogLinearDiscountConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-log-linear discount curve...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    setup();

    testCurveConsistency(Discount(), LogLinear());
}

void PiecewiseYieldCurveTest::testLinearDiscountConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-linear discount curve...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    setup();

    testCurveConsistency(Discount(), Linear());
}

void PiecewiseYieldCurveTest::testLogLinearZeroConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-log-linear zero-yield curve...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    setup();

    testCurveConsistency(ZeroYield(), LogLinear());
}

void PiecewiseYieldCurveTest::testLinearZeroConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-linear zero-yield curve...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    setup();

    testCurveConsistency(ZeroYield(), Linear());
}

void PiecewiseYieldCurveTest::testSplineZeroConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-spline zero-yield curve...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    setup();

    testCurveConsistency(ZeroYield(),
                         CubicSpline(CubicSplineInterpolation::SecondDerivative,0.0,
                               CubicSplineInterpolation::SecondDerivative,0.0,
                               true));
}

void PiecewiseYieldCurveTest::testLinearForwardConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-linear forward-rate curve...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    setup();

    testCurveConsistency(ForwardRate(), Linear());
}

void PiecewiseYieldCurveTest::testFlatForwardConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-flat forward-rate curve...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    setup();

    testCurveConsistency(ForwardRate(), BackwardFlat());
}

void PiecewiseYieldCurveTest::testSplineForwardConsistency() {

    BOOST_MESSAGE(
        "Testing consistency of piecewise-spline forward-rate curve...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    setup();

    testCurveConsistency(ForwardRate(),
                         CubicSpline(CubicSplineInterpolation::SecondDerivative,0.0,
                               CubicSplineInterpolation::SecondDerivative,0.0,
                               true));
}

void PiecewiseYieldCurveTest::testObservability() {

    BOOST_MESSAGE("Testing observability of piecewise yield curve...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    setup();

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
}


void PiecewiseYieldCurveTest::testLiborFixing() {

    BOOST_MESSAGE(
        "Testing use of today's LIBOR fixings in swap curve...");

    SavedSettings backup;
    IndexHistoryCleaner cleaner;

    setup();

    std::vector<boost::shared_ptr<RateHelper> > swapHelpers(swaps);
    boost::shared_ptr<IborIndex> euribor6m(new Euribor6M);

    for (Size i=0; i<swaps; i++) {
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

    Handle<YieldTermStructure> curveHandle =
        Handle<YieldTermStructure>(termStructure);

    boost::shared_ptr<IborIndex> index(new Euribor6M(curveHandle));
    for (Size i=0; i<swaps; i++) {
        Period tenor = swapData[i].n*swapData[i].units;

        VanillaSwap swap = MakeVanillaSwap(tenor, index, 0.0)
            .withEffectiveDate(settlement)
            .withFixedLegDayCount(fixedLegDayCounter)
            .withFixedLegTenor(Period(fixedLegFrequency))
            .withFixedLegConvention(fixedLegConvention)
            .withFixedLegTerminationDateConvention(fixedLegConvention);

        Rate expectedRate = swapData[i].rate/100,
             estimatedRate = swap.fairRate();
        Real tolerance = 1.0e-9;
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

    for (Size i=0; i<swaps; i++) {
        Period tenor = swapData[i].n*swapData[i].units;

        VanillaSwap swap = MakeVanillaSwap(tenor, index, 0.0)
            .withEffectiveDate(settlement)
            .withFixedLegDayCount(fixedLegDayCounter)
            .withFixedLegTenor(Period(fixedLegFrequency))
            .withFixedLegConvention(fixedLegConvention)
            .withFixedLegTerminationDateConvention(fixedLegConvention);

        Rate expectedRate = swapData[i].rate/100,
             estimatedRate = swap.fairRate();
        Real tolerance = 1.0e-9;
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
}


test_suite* PiecewiseYieldCurveTest::suite() {

    test_suite* suite = BOOST_TEST_SUITE("Piecewise yield curve tests");
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
    suite->add(BOOST_TEST_CASE(
                 &PiecewiseYieldCurveTest::testSplineForwardConsistency));
    suite->add(BOOST_TEST_CASE(&PiecewiseYieldCurveTest::testObservability));
    suite->add(BOOST_TEST_CASE(&PiecewiseYieldCurveTest::testLiborFixing));
    return suite;
}
