/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007, 2008, 2009 StatPro Italia srl

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
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/indexes/ibor/jpylibor.hpp>
#include <ql/indexes/bmaindex.hpp>
#include <ql/indexes/indexmanager.hpp>
#include <ql/instruments/forwardrateagreement.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/convexmonotoneinterpolation.hpp>
#include <ql/math/comparison.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

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

    Datum bmaData[] = {
        {  1, Years, 67.56 },
        {  2, Years, 68.00 },
        {  3, Years, 68.25 },
        {  4, Years, 68.50 },
        {  5, Years, 68.81 },
        {  7, Years, 69.50 },
        { 10, Years, 70.44 },
        { 15, Years, 71.69 },
        { 20, Years, 72.69 },
        { 30, Years, 73.81 }
    };

    struct CommonVars {
        // global variables
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
        Frequency bmaFrequency;
        BusinessDayConvention bmaConvention;
        DayCounter bmaDayCounter;

        Size deposits, fras, swaps, bonds, bmas;
        std::vector<boost::shared_ptr<SimpleQuote> > rates, fraRates,
                                                     prices, fractions;
        std::vector<boost::shared_ptr<RateHelper> > instruments, fraHelpers,
                                                    bondHelpers, bmaHelpers;
        std::vector<Schedule> schedules;
        boost::shared_ptr<YieldTermStructure> termStructure;

        // cleanup
        SavedSettings backup;
        IndexHistoryCleaner cleaner;

        // setup
        CommonVars() {
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
            bmaFrequency = Quarterly;
            bmaConvention = Following;
            bmaDayCounter = ActualActual();

            deposits = LENGTH(depositData);
            fras = LENGTH(fraData);
            swaps = LENGTH(swapData);
            bonds = LENGTH(bondData);
            bmas = LENGTH(bmaData);

            // market elements
            rates =
                std::vector<boost::shared_ptr<SimpleQuote> >(deposits+swaps);
            fraRates = std::vector<boost::shared_ptr<SimpleQuote> >(fras);
            prices = std::vector<boost::shared_ptr<SimpleQuote> >(bonds);
            fractions = std::vector<boost::shared_ptr<SimpleQuote> >(bmas);
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
            for (Size i=0; i<bmas; i++) {
                fractions[i] = boost::shared_ptr<SimpleQuote>(
                                        new SimpleQuote(bmaData[i].rate/100));
            }

            // rate helpers
            instruments =
                std::vector<boost::shared_ptr<RateHelper> >(deposits+swaps);
            fraHelpers = std::vector<boost::shared_ptr<RateHelper> >(fras);
            bondHelpers = std::vector<boost::shared_ptr<RateHelper> >(bonds);
            schedules = std::vector<Schedule>(bonds);
            bmaHelpers = std::vector<boost::shared_ptr<RateHelper> >(bmas);

            boost::shared_ptr<IborIndex> euribor6m(new Euribor6M);
            for (Size i=0; i<deposits; i++) {
                Handle<Quote> r(rates[i]);
                instruments[i] = boost::shared_ptr<RateHelper>(new
                    DepositRateHelper(r, depositData[i].n*depositData[i].units,
                                      euribor6m->fixingDays(), calendar,
                                      euribor6m->businessDayConvention(),
                                      euribor6m->endOfMonth(),
                                      euribor6m->dayCounter()));
            }
            for (Size i=0; i<swaps; i++) {
                Handle<Quote> r(rates[i+deposits]);
                instruments[i+deposits] = boost::shared_ptr<RateHelper>(new
                    SwapRateHelper(r, swapData[i].n*swapData[i].units,
                                   calendar,
                                   fixedLegFrequency, fixedLegConvention,
                                   fixedLegDayCounter, euribor6m));
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
                                  euribor3m.dayCounter()));
            }

            for (Size i=0; i<bonds; i++) {
                Handle<Quote> p(prices[i]);
                Date maturity =
                    calendar.advance(today, bondData[i].n, bondData[i].units);
                Date issue =
                    calendar.advance(maturity, -bondData[i].length, Years);
                std::vector<Rate> coupons(1, bondData[i].coupon/100.0);
                schedules[i] = Schedule(issue, maturity,
                                        Period(bondData[i].frequency),
                                        calendar,
                                        bondConvention, bondConvention,
                                        DateGeneration::Backward, false);
                bondHelpers[i] = boost::shared_ptr<RateHelper>(new
                    FixedRateBondHelper(p,
                                        bondSettlementDays,
                                        bondRedemption, schedules[i],
                                        coupons, bondDayCounter,
                                        bondConvention,
                                        bondRedemption, issue));
            }
        }
    };


    template <class T, class I, template<class C> class B>
    void testCurveConsistency(CommonVars& vars,
                              const I& interpolator = I(),
                              Real tolerance = 1.0e-9) {

        vars.termStructure = boost::shared_ptr<YieldTermStructure>(new
            PiecewiseYieldCurve<T,I,B>(vars.settlement, vars.instruments,
                                       Actual360(),
                                       interpolator));

        RelinkableHandle<YieldTermStructure> curveHandle;
        curveHandle.linkTo(vars.termStructure);

        // check deposits
        for (Size i=0; i<vars.deposits; i++) {
            Euribor index(depositData[i].n*depositData[i].units,curveHandle);
            Rate expectedRate  = depositData[i].rate/100,
                 estimatedRate = index.fixing(vars.today);
            if (std::fabs(expectedRate-estimatedRate) > tolerance) {
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
        for (Size i=0; i<vars.swaps; i++) {
            Period tenor = swapData[i].n*swapData[i].units;

            VanillaSwap swap = MakeVanillaSwap(tenor, euribor6m, 0.0)
                .withEffectiveDate(vars.settlement)
                .withFixedLegDayCount(vars.fixedLegDayCounter)
                .withFixedLegTenor(Period(vars.fixedLegFrequency))
                .withFixedLegConvention(vars.fixedLegConvention)
                .withFixedLegTerminationDateConvention(vars.fixedLegConvention);

            Rate expectedRate = swapData[i].rate/100,
                 estimatedRate = swap.fairRate();
            Spread error = std::fabs(expectedRate-estimatedRate);
            if (error > tolerance) {
                BOOST_ERROR(
                    swapData[i].n << " year(s) swap:\n"
                    << std::setprecision(8)
                    << "\n estimated rate: " << io::rate(estimatedRate)
                    << "\n expected rate:  " << io::rate(expectedRate)
                    << "\n error:          " << io::rate(error)
                    << "\n tolerance:      " << io::rate(tolerance));
            }
        }

        // check bonds
        vars.termStructure = boost::shared_ptr<YieldTermStructure>(new
            PiecewiseYieldCurve<T,I,B>(vars.settlement, vars.bondHelpers,
                                       Actual360(),
                                       interpolator));
        curveHandle.linkTo(vars.termStructure);

        for (Size i=0; i<vars.bonds; i++) {
            Date maturity = vars.calendar.advance(vars.today,
                                                  bondData[i].n,
                                                  bondData[i].units);
            Date issue = vars.calendar.advance(maturity,
                                               -bondData[i].length,
                                               Years);
            std::vector<Rate> coupons(1, bondData[i].coupon/100.0);

            FixedRateBond bond(vars.bondSettlementDays, 100.0,
                               vars.schedules[i], coupons,
                               vars.bondDayCounter, vars.bondConvention,
                               vars.bondRedemption, issue);

            boost::shared_ptr<PricingEngine> bondEngine(
                                      new DiscountingBondEngine(curveHandle));
            bond.setPricingEngine(bondEngine);

            Real expectedPrice = bondData[i].price,
                 estimatedPrice = bond.cleanPrice();
            Real error = std::fabs(expectedPrice-estimatedPrice);
            if (error > tolerance) {
                BOOST_ERROR(io::ordinal(i+1) << " bond failure:" <<
                            std::setprecision(8) <<
                            "\n  estimated price: " << estimatedPrice <<
                            "\n  expected price:  " << expectedPrice <<
                            "\n  error:           " << error);
            }
        }

        // check FRA
        vars.termStructure = boost::shared_ptr<YieldTermStructure>(new
            PiecewiseYieldCurve<T,I>(vars.settlement, vars.fraHelpers,
                                     Actual360(),
                                     interpolator));
        curveHandle.linkTo(vars.termStructure);

        boost::shared_ptr<IborIndex> euribor3m(new Euribor3M(curveHandle));
        for (Size i=0; i<vars.fras; i++) {
            Date start =
                vars.calendar.advance(vars.settlement,
                                      fraData[i].n,
                                      fraData[i].units,
                                      euribor3m->businessDayConvention(),
                                      euribor3m->endOfMonth());
            Date end = vars.calendar.advance(start, 3, Months,
                                             euribor3m->businessDayConvention(),
                                             euribor3m->endOfMonth());

            ForwardRateAgreement fra(start, end, Position::Long,
                                     fraData[i].rate/100, 100.0,
                                     euribor3m, curveHandle);
            Rate expectedRate = fraData[i].rate/100,
                 estimatedRate = fra.forwardRate();
            if (std::fabs(expectedRate-estimatedRate) > tolerance) {
                BOOST_ERROR(io::ordinal(i+1) << " FRA failure:" <<
                            std::setprecision(8) <<
                            "\n  estimated rate: " << io::rate(estimatedRate) <<
                            "\n  expected rate:  " << io::rate(expectedRate));
            }
        }
    }


    template <class T, class I, template<class C> class B>
    void testBMACurveConsistency(CommonVars& vars,
                                 const I& interpolator = I(),
                                 Real tolerance = 1.0e-9) {

        // re-adjust settlement
        vars.calendar = JointCalendar(BMAIndex().fixingCalendar(),
                                      USDLibor(3*Months).fixingCalendar(),
                                      JoinHolidays);
        vars.today = vars.calendar.adjust(Date::todaysDate());
        Settings::instance().evaluationDate() = vars.today;
        vars.settlement =
            vars.calendar.advance(vars.today,vars.settlementDays,Days);


        Handle<YieldTermStructure> riskFreeCurve(
            boost::shared_ptr<YieldTermStructure>(
                        new FlatForward(vars.settlement, 0.04, Actual360())));

        boost::shared_ptr<BMAIndex> bmaIndex(new BMAIndex);
        boost::shared_ptr<IborIndex> liborIndex(
                                        new USDLibor(3*Months,riskFreeCurve));
        for (Size i=0; i<vars.bmas; ++i) {
            Handle<Quote> f(vars.fractions[i]);
            vars.bmaHelpers[i] = boost::shared_ptr<RateHelper>(
                      new BMASwapRateHelper(f, bmaData[i].n*bmaData[i].units,
                                            vars.settlementDays,
                                            vars.calendar,
                                            Period(vars.bmaFrequency),
                                            vars.bmaConvention,
                                            vars.bmaDayCounter,
                                            bmaIndex,
                                            liborIndex));
        }

        Weekday w = vars.today.weekday();
        Date lastWednesday =
            (w >= 4) ? vars.today - (w - 4) : vars.today + (4 - w - 7);
        Date lastFixing = bmaIndex->fixingCalendar().adjust(lastWednesday);
        bmaIndex->addFixing(lastFixing, 0.03);

        vars.termStructure = boost::shared_ptr<YieldTermStructure>(new
            PiecewiseYieldCurve<T,I,B>(vars.settlement, vars.bmaHelpers,
                                       Actual360(),
                                       1.0e-12,
                                       interpolator));

        RelinkableHandle<YieldTermStructure> curveHandle;
        curveHandle.linkTo(vars.termStructure);

        // check BMA swaps
        boost::shared_ptr<BMAIndex> bma(new BMAIndex(curveHandle));
        boost::shared_ptr<IborIndex> libor3m(new USDLibor(3*Months,
                                                          riskFreeCurve));
        for (Size i=0; i<vars.bmas; i++) {
            Period tenor = bmaData[i].n*bmaData[i].units;

            Schedule bmaSchedule =
                MakeSchedule().from(vars.settlement)
                              .to(vars.settlement+tenor)
                              .withFrequency(vars.bmaFrequency)
                              .withCalendar(bma->fixingCalendar())
                              .withConvention(vars.bmaConvention)
                              .backwards();
            Schedule liborSchedule =
                MakeSchedule().from(vars.settlement)
                              .to(vars.settlement+tenor)
                              .withTenor(libor3m->tenor())
                              .withCalendar(libor3m->fixingCalendar())
                              .withConvention(libor3m->businessDayConvention())
                              .endOfMonth(libor3m->endOfMonth())
                              .backwards();


            BMASwap swap(BMASwap::Payer, 100.0,
                         liborSchedule, 0.75, 0.0,
                         libor3m, libor3m->dayCounter(),
                         bmaSchedule, bma, vars.bmaDayCounter);
            swap.setPricingEngine(boost::shared_ptr<PricingEngine>(
              new DiscountingSwapEngine(libor3m->forwardingTermStructure())));

            Real expectedFraction = bmaData[i].rate/100,
                 estimatedFraction = swap.fairLiborFraction();
            Real error = std::fabs(expectedFraction-estimatedFraction);
            if (error > tolerance) {
                BOOST_ERROR(bmaData[i].n << " year(s) BMA swap:\n"
                            << std::setprecision(8)
                            << "\n estimated libor fraction: " << estimatedFraction
                            << "\n expected libor fraction:  " << expectedFraction
                            << "\n error:          " << error
                            << "\n tolerance:      " << tolerance);
            }
        }
    }

}


void PiecewiseYieldCurveTest::testLogCubicDiscountConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-log-cubic discount curve...");

    CommonVars vars;

    testCurveConsistency<Discount,LogCubic,IterativeBootstrap>(
        vars,
        LogCubic(CubicInterpolation::Spline, true,
                 CubicInterpolation::SecondDerivative, 0.0,
                 CubicInterpolation::SecondDerivative, 0.0));
    testBMACurveConsistency<Discount,LogCubic,IterativeBootstrap>(
        vars,
        LogCubic(CubicInterpolation::Spline, true,
                 CubicInterpolation::SecondDerivative, 0.0,
                 CubicInterpolation::SecondDerivative, 0.0));
}

void PiecewiseYieldCurveTest::testLogLinearDiscountConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-log-linear discount curve...");

    CommonVars vars;

    testCurveConsistency<Discount,LogLinear,IterativeBootstrap>(vars);
    testBMACurveConsistency<Discount,LogLinear,IterativeBootstrap>(vars);
}

void PiecewiseYieldCurveTest::testLinearDiscountConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-linear discount curve...");

    CommonVars vars;

    testCurveConsistency<Discount,Linear,IterativeBootstrap>(vars);
    testBMACurveConsistency<Discount,Linear,IterativeBootstrap>(vars);
}

void PiecewiseYieldCurveTest::testLogLinearZeroConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-log-linear zero-yield curve...");

    CommonVars vars;

    testCurveConsistency<ZeroYield,LogLinear,IterativeBootstrap>(vars);
    testBMACurveConsistency<ZeroYield,LogLinear,IterativeBootstrap>(vars);
}

void PiecewiseYieldCurveTest::testLinearZeroConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-linear zero-yield curve...");

    CommonVars vars;

    testCurveConsistency<ZeroYield,Linear,IterativeBootstrap>(vars);
    testBMACurveConsistency<ZeroYield,Linear,IterativeBootstrap>(vars);
}

void PiecewiseYieldCurveTest::testSplineZeroConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-cubic zero-yield curve...");

    CommonVars vars;

    testCurveConsistency<ZeroYield,Cubic,IterativeBootstrap>(
                   vars,
                   Cubic(CubicInterpolation::Spline, true,
                         CubicInterpolation::SecondDerivative, 0.0,
                         CubicInterpolation::SecondDerivative, 0.0));
    testBMACurveConsistency<ZeroYield,Cubic,IterativeBootstrap>(
                   vars,
                   Cubic(CubicInterpolation::Spline, true,
                         CubicInterpolation::SecondDerivative, 0.0,
                         CubicInterpolation::SecondDerivative, 0.0));
}

void PiecewiseYieldCurveTest::testLinearForwardConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-linear forward-rate curve...");

    CommonVars vars;

    testCurveConsistency<ForwardRate,Linear,IterativeBootstrap>(vars);
    testBMACurveConsistency<ForwardRate,Linear,IterativeBootstrap>(vars);
}

void PiecewiseYieldCurveTest::testFlatForwardConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-flat forward-rate curve...");

    CommonVars vars;

    testCurveConsistency<ForwardRate,BackwardFlat,IterativeBootstrap>(vars);
    testBMACurveConsistency<ForwardRate,BackwardFlat,IterativeBootstrap>(vars);
}

void PiecewiseYieldCurveTest::testSplineForwardConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-cubic forward-rate curve...");

    CommonVars vars;

    testCurveConsistency<ForwardRate,Cubic,IterativeBootstrap>(
                   vars,
                   Cubic(CubicInterpolation::Spline, true,
                         CubicInterpolation::SecondDerivative, 0.0,
                         CubicInterpolation::SecondDerivative, 0.0));
    testBMACurveConsistency<ForwardRate,Cubic,IterativeBootstrap>(
                   vars,
                   Cubic(CubicInterpolation::Spline, true,
                         CubicInterpolation::SecondDerivative, 0.0,
                         CubicInterpolation::SecondDerivative, 0.0));
}

void PiecewiseYieldCurveTest::testConvexMonotoneForwardConsistency() {
    BOOST_TEST_MESSAGE(
        "Testing consistency of convex monotone forward-rate curve...");

    CommonVars vars;
    testCurveConsistency<ForwardRate,ConvexMonotone,IterativeBootstrap>(vars);

    testBMACurveConsistency<ForwardRate,ConvexMonotone,
                            IterativeBootstrap>(vars);
}


void PiecewiseYieldCurveTest::testLocalBootstrapConsistency() {
    BOOST_TEST_MESSAGE(
        "Testing consistency of local-bootstrap algorithm...");

    CommonVars vars;
    testCurveConsistency<ForwardRate,ConvexMonotone,LocalBootstrap>(
                                              vars, ConvexMonotone(), 1.0e-7);
    testBMACurveConsistency<ForwardRate,ConvexMonotone,LocalBootstrap>(
                                              vars, ConvexMonotone(), 1.0e-7);
}


void PiecewiseYieldCurveTest::testObservability() {

    BOOST_TEST_MESSAGE("Testing observability of piecewise yield curve...");

    CommonVars vars;

    vars.termStructure = boost::shared_ptr<YieldTermStructure>(
       new PiecewiseYieldCurve<Discount,LogLinear>(vars.settlementDays,
                                                   vars.calendar,
                                                   vars.instruments,
                                                   Actual360()));
    Flag f;
    f.registerWith(vars.termStructure);

    for (Size i=0; i<vars.deposits+vars.swaps; i++) {
        Time testTime =
            Actual360().yearFraction(vars.settlement,
                                     vars.instruments[i]->latestDate());
        DiscountFactor discount = vars.termStructure->discount(testTime);
        f.lower();
        vars.rates[i]->setValue(vars.rates[i]->value()*1.01);
        if (!f.isUp())
            BOOST_FAIL("Observer was not notified of underlying rate change");
        if (vars.termStructure->discount(testTime,true) == discount)
            BOOST_FAIL("rate change did not trigger recalculation");
        vars.rates[i]->setValue(vars.rates[i]->value()/1.01);
    }

    vars.termStructure->maxDate();
    f.lower();
    Settings::instance().evaluationDate() =
        vars.calendar.advance(vars.today,15,Days);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of date change");

    f.lower();
    Settings::instance().evaluationDate() = vars.today;
    if (f.isUp())
        BOOST_FAIL("Observer was notified of date change"
                   " without an intervening recalculation");
}


void PiecewiseYieldCurveTest::testLiborFixing() {

    BOOST_TEST_MESSAGE(
        "Testing use of today's LIBOR fixings in swap curve...");

    CommonVars vars;

    std::vector<boost::shared_ptr<RateHelper> > swapHelpers(vars.swaps);
    boost::shared_ptr<IborIndex> euribor6m(new Euribor6M);

    for (Size i=0; i<vars.swaps; i++) {
        Handle<Quote> r(vars.rates[i+vars.deposits]);
        swapHelpers[i] = boost::shared_ptr<RateHelper>(new
            SwapRateHelper(r, Period(swapData[i].n, swapData[i].units),
                           vars.calendar,
                           vars.fixedLegFrequency, vars.fixedLegConvention,
                           vars.fixedLegDayCounter, euribor6m));
    }

    vars.termStructure = boost::shared_ptr<YieldTermStructure>(new
        PiecewiseYieldCurve<Discount,LogLinear>(vars.settlement,
                                                swapHelpers,
                                                Actual360()));

    Handle<YieldTermStructure> curveHandle =
        Handle<YieldTermStructure>(vars.termStructure);

    boost::shared_ptr<IborIndex> index(new Euribor6M(curveHandle));
    for (Size i=0; i<vars.swaps; i++) {
        Period tenor = swapData[i].n*swapData[i].units;

        VanillaSwap swap = MakeVanillaSwap(tenor, index, 0.0)
            .withEffectiveDate(vars.settlement)
            .withFixedLegDayCount(vars.fixedLegDayCounter)
            .withFixedLegTenor(Period(vars.fixedLegFrequency))
            .withFixedLegConvention(vars.fixedLegConvention)
            .withFixedLegTerminationDateConvention(vars.fixedLegConvention);

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
    f.registerWith(vars.termStructure);
    f.lower();

    index->addFixing(vars.today, 0.0425);

    if (!f.isUp())
        BOOST_ERROR("Observer was not notified of rate fixing");

    for (Size i=0; i<vars.swaps; i++) {
        Period tenor = swapData[i].n*swapData[i].units;

        VanillaSwap swap = MakeVanillaSwap(tenor, index, 0.0)
            .withEffectiveDate(vars.settlement)
            .withFixedLegDayCount(vars.fixedLegDayCounter)
            .withFixedLegTenor(Period(vars.fixedLegFrequency))
            .withFixedLegConvention(vars.fixedLegConvention)
            .withFixedLegTerminationDateConvention(vars.fixedLegConvention);

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

void PiecewiseYieldCurveTest::testJpyLibor() {
    BOOST_TEST_MESSAGE(
        "Testing bootstrap over JPY LIBOR swaps...");

    CommonVars vars;

    vars.today = Date(4, October, 2007);
    Settings::instance().evaluationDate() = vars.today;

    vars.calendar = Japan();
    vars.settlement =
        vars.calendar.advance(vars.today,vars.settlementDays,Days);

    // market elements
    vars.rates = std::vector<boost::shared_ptr<SimpleQuote> >(vars.swaps);
    for (Size i=0; i<vars.swaps; i++) {
        vars.rates[i] = boost::shared_ptr<SimpleQuote>(
                                       new SimpleQuote(swapData[i].rate/100));
    }

    // rate helpers
    vars.instruments = std::vector<boost::shared_ptr<RateHelper> >(vars.swaps);

    boost::shared_ptr<IborIndex> index(new JPYLibor(6*Months));
    for (Size i=0; i<vars.swaps; i++) {
        Handle<Quote> r(vars.rates[i]);
        vars.instruments[i] = boost::shared_ptr<RateHelper>(
           new SwapRateHelper(r, swapData[i].n*swapData[i].units,
                              vars.calendar,
                              vars.fixedLegFrequency, vars.fixedLegConvention,
                              vars.fixedLegDayCounter, index));
    }

    vars.termStructure = boost::shared_ptr<YieldTermStructure>(
        new PiecewiseYieldCurve<Discount,LogLinear>(
                                       vars.settlement, vars.instruments,
                                       Actual360(),
                                       1.0e-12));

    RelinkableHandle<YieldTermStructure> curveHandle;
    curveHandle.linkTo(vars.termStructure);

    // check swaps
    boost::shared_ptr<IborIndex> jpylibor6m(new JPYLibor(6*Months,curveHandle));
    for (Size i=0; i<vars.swaps; i++) {
        Period tenor = swapData[i].n*swapData[i].units;

        VanillaSwap swap = MakeVanillaSwap(tenor, jpylibor6m, 0.0)
            .withEffectiveDate(vars.settlement)
            .withFixedLegDayCount(vars.fixedLegDayCounter)
            .withFixedLegTenor(Period(vars.fixedLegFrequency))
            .withFixedLegConvention(vars.fixedLegConvention)
            .withFixedLegTerminationDateConvention(vars.fixedLegConvention)
            .withFixedLegCalendar(vars.calendar)
            .withFloatingLegCalendar(vars.calendar);

        Rate expectedRate = swapData[i].rate/100,
             estimatedRate = swap.fairRate();
        Spread error = std::fabs(expectedRate-estimatedRate);
        Real tolerance = 1.0e-9;

        if (error > tolerance) {
            BOOST_ERROR(swapData[i].n << " year(s) swap:\n"
                        << std::setprecision(8)
                        << "\n estimated rate: " << io::rate(estimatedRate)
                        << "\n expected rate:  " << io::rate(expectedRate)
                        << "\n error:          " << io::rate(error)
                        << "\n tolerance:      " << io::rate(tolerance));
        }
    }
}

namespace {

    template <class T, class I>
    void testCurveCopy(CommonVars& vars,
                       const I& interpolator = I()) {

        PiecewiseYieldCurve<T,I> curve(vars.settlement, vars.instruments,
                                       Actual360(),
                                       1.0e-12,
                                       interpolator);
        // necessary to trigger bootstrap
        curve.recalculate();

        typedef typename T::template curve<I>::type base_curve;

        base_curve copiedCurve = curve;

        // the two curves should be the same.
        Time t = 2.718;
        Rate r1 = curve.zeroRate(t, Continuous);
        Rate r2 = copiedCurve.zeroRate(t, Continuous);
        if (!close(r1, r2)) {
            BOOST_ERROR("failed to link original and copied curve");
        }

        for (Size i=0; i<vars.rates.size(); ++i) {
            vars.rates[i]->setValue(vars.rates[i]->value() + 0.001);
        }

        // now the original curve should have changed; the copied
        // curve should not.
        Rate r3 = curve.zeroRate(t, Continuous);
        Rate r4 = copiedCurve.zeroRate(t, Continuous);
        if (close(r1, r3)) {
            BOOST_ERROR("failed to modify original curve");
        }
        if (!close(r2,r4)) {
            BOOST_ERROR(
                    "failed to break link between original and copied curve");
        }
    }

}


void PiecewiseYieldCurveTest::testDiscountCopy() {
    BOOST_TEST_MESSAGE("Testing copying of discount curve...");

    CommonVars vars;
    testCurveCopy<Discount,LogLinear>(vars);
}

void PiecewiseYieldCurveTest::testForwardCopy() {
    BOOST_TEST_MESSAGE("Testing copying of forward-rate curve...");

    CommonVars vars;
    testCurveCopy<ForwardRate,BackwardFlat>(vars);
}

void PiecewiseYieldCurveTest::testZeroCopy() {
    BOOST_TEST_MESSAGE("Testing copying of zero-rate curve...");

    CommonVars vars;
    testCurveCopy<ZeroYield,Linear>(vars);
}




test_suite* PiecewiseYieldCurveTest::suite() {

    test_suite* suite = BOOST_TEST_SUITE("Piecewise yield curve tests");

    // unstable
    //suite->add(QUANTLIB_TEST_CASE(
    //             &PiecewiseYieldCurveTest::testLogCubicDiscountConsistency));
    suite->add(QUANTLIB_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLogLinearDiscountConsistency));
    suite->add(QUANTLIB_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLinearDiscountConsistency));

// why?
#if !defined(QL_USE_INDEXED_COUPON)
  // if rates can be negative it makes no sense to interpolate loglinearly
  #if !defined(QL_NEGATIVE_RATES)
    suite->add(QUANTLIB_TEST_CASE(
                     &PiecewiseYieldCurveTest::testLogLinearZeroConsistency));
  #endif
#endif
    suite->add(QUANTLIB_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLinearZeroConsistency));
    suite->add(QUANTLIB_TEST_CASE(
                 &PiecewiseYieldCurveTest::testSplineZeroConsistency));

    suite->add(QUANTLIB_TEST_CASE(
                 &PiecewiseYieldCurveTest::testLinearForwardConsistency));
    suite->add(QUANTLIB_TEST_CASE(
                 &PiecewiseYieldCurveTest::testFlatForwardConsistency));
    // unstable
    //suite->add(QUANTLIB_TEST_CASE(
    //             &PiecewiseYieldCurveTest::testSplineForwardConsistency));

    suite->add(QUANTLIB_TEST_CASE(
             &PiecewiseYieldCurveTest::testConvexMonotoneForwardConsistency));
    suite->add(QUANTLIB_TEST_CASE(
             &PiecewiseYieldCurveTest::testLocalBootstrapConsistency));

    suite->add(QUANTLIB_TEST_CASE(&PiecewiseYieldCurveTest::testObservability));
    suite->add(QUANTLIB_TEST_CASE(&PiecewiseYieldCurveTest::testLiborFixing));

    suite->add(QUANTLIB_TEST_CASE(&PiecewiseYieldCurveTest::testJpyLibor));

    suite->add(QUANTLIB_TEST_CASE(&PiecewiseYieldCurveTest::testDiscountCopy));
    suite->add(QUANTLIB_TEST_CASE(&PiecewiseYieldCurveTest::testForwardCopy));
    suite->add(QUANTLIB_TEST_CASE(&PiecewiseYieldCurveTest::testZeroCopy));

    return suite;
}
