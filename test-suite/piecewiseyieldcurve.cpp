/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007, 2008, 2009, 2017 StatPro Italia srl

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
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/termstructures/globalbootstrap.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/yield/bondhelpers.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/imm.hpp>
#include <ql/time/asx.hpp>
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

namespace piecewise_yield_curve_test {

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

    Datum immFutData[] = {
        { 1, Months, 4.581 },
        { 2, Months, 4.573 },
        { 3, Months, 4.557 }
    };

    Datum asxFutData[] = {
        { 1, Months, 4.581 },
        { 2, Months, 4.573 },
        { 3, Months, 4.557 }
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

        Size deposits, fras, immFuts, asxFuts, swaps, bonds, bmas;
        std::vector<ext::shared_ptr<SimpleQuote> > rates, fraRates,
                                                     immFutPrices, asxFutPrices,
                                                     prices, fractions;
        std::vector<ext::shared_ptr<RateHelper> > instruments, fraHelpers,
                                                    immFutHelpers, asxFutHelpers,
                                                    bondHelpers, bmaHelpers;
        std::vector<Schedule> schedules;
        ext::shared_ptr<YieldTermStructure> termStructure;

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
            immFuts = LENGTH(immFutData);
            asxFuts = LENGTH(asxFutData);
            swaps = LENGTH(swapData);
            bonds = LENGTH(bondData);
            bmas = LENGTH(bmaData);

            // market elements
            rates =
                std::vector<ext::shared_ptr<SimpleQuote> >(deposits+swaps);
            fraRates = std::vector<ext::shared_ptr<SimpleQuote> >(fras);
            immFutPrices = std::vector<ext::shared_ptr<SimpleQuote> >(immFuts);
            asxFutPrices = std::vector<ext::shared_ptr<SimpleQuote> >(asxFuts);
            prices = std::vector<ext::shared_ptr<SimpleQuote> >(bonds);
            fractions = std::vector<ext::shared_ptr<SimpleQuote> >(bmas);
            for (Size i=0; i<deposits; i++) {
                rates[i] = ext::make_shared<SimpleQuote>(
                                    depositData[i].rate/100);
            }
            for (Size i=0; i<swaps; i++) {
                rates[i+deposits] = ext::make_shared<SimpleQuote>(
                                       swapData[i].rate/100);
            }
            for (Size i=0; i<fras; i++) {
                fraRates[i] = ext::make_shared<SimpleQuote>(
                                        fraData[i].rate/100);
            }
            for (Size i = 0; i<bonds; i++) {
                prices[i] = ext::make_shared<SimpleQuote>(
                                          bondData[i].price);
            }
            for (Size i = 0; i<immFuts; i++) {
                immFutPrices[i] = ext::make_shared<SimpleQuote>(
                    100.0 - immFutData[i].rate);
            }
            for (Size i = 0; i<asxFuts; i++) {
                asxFutPrices[i] = ext::make_shared<SimpleQuote>(
                    100.0 - asxFutData[i].rate);
            }
            for (Size i = 0; i<bmas; i++) {
                fractions[i] = ext::make_shared<SimpleQuote>(
                                        bmaData[i].rate/100);
            }

            // rate helpers
            instruments =
                std::vector<ext::shared_ptr<RateHelper> >(deposits+swaps);
            fraHelpers = std::vector<ext::shared_ptr<RateHelper> >(fras);
            immFutHelpers = std::vector<ext::shared_ptr<RateHelper> >(immFuts);
            asxFutHelpers = std::vector<ext::shared_ptr<RateHelper> >();
            bondHelpers = std::vector<ext::shared_ptr<RateHelper> >(bonds);
            schedules = std::vector<Schedule>(bonds);
            bmaHelpers = std::vector<ext::shared_ptr<RateHelper> >(bmas);

            ext::shared_ptr<IborIndex> euribor6m(new Euribor6M);
            for (Size i=0; i<deposits; i++) {
                Handle<Quote> r(rates[i]);
                instruments[i] = ext::shared_ptr<RateHelper>(new
                    DepositRateHelper(r,
                                      ext::make_shared<Euribor>(
                                          depositData[i].n*depositData[i].units)));
            }
            for (Size i=0; i<swaps; i++) {
                Handle<Quote> r(rates[i+deposits]);
                instruments[i+deposits] = ext::shared_ptr<RateHelper>(new
                    SwapRateHelper(r, swapData[i].n*swapData[i].units,
                                   calendar,
                                   fixedLegFrequency, fixedLegConvention,
                                   fixedLegDayCounter, euribor6m));
            }


#ifdef QL_USE_INDEXED_COUPON
            bool useIndexedFra = false;
#else
            bool useIndexedFra = true;
#endif

            ext::shared_ptr<IborIndex> euribor3m(new Euribor3M());
            for (Size i=0; i<fras; i++) {
                Handle<Quote> r(fraRates[i]);
                fraHelpers[i] = ext::shared_ptr<RateHelper>(new
                    FraRateHelper(r, fraData[i].n, fraData[i].n + 3,
                                  euribor3m->fixingDays(),
                                  euribor3m->fixingCalendar(),
                                  euribor3m->businessDayConvention(),
                                  euribor3m->endOfMonth(),
                                  euribor3m->dayCounter(),
                                  Pillar::LastRelevantDate,
                                  Date(),
                                  useIndexedFra));
            }
            Date immDate = Date();
            for (Size i = 0; i<immFuts; i++) {
                Handle<Quote> r(immFutPrices[i]);
                immDate = IMM::nextDate(immDate, false);
                // if the fixing is before the evaluation date, we
                // just jump forward by one future maturity
                if (euribor3m->fixingDate(immDate) <
                    Settings::instance().evaluationDate())
                    immDate = IMM::nextDate(immDate, false);
                immFutHelpers[i] = ext::shared_ptr<RateHelper>(new
                    FuturesRateHelper(r, immDate, euribor3m, Handle<Quote>(),
                                      Futures::IMM));
            }
            Date asxDate = Date();
            for (Size i = 0; i<asxFuts; i++) {
                Handle<Quote> r(asxFutPrices[i]);
                asxDate = ASX::nextDate(asxDate, false);
                // if the fixing is before the evaluation date, we
                // just jump forward by one future maturity
                if (euribor3m->fixingDate(asxDate) <
                    Settings::instance().evaluationDate())
                    asxDate = ASX::nextDate(asxDate, false);
                if (euribor3m->fixingCalendar().isBusinessDay(asxDate))
                    asxFutHelpers.push_back(ext::shared_ptr<RateHelper>(new
                        FuturesRateHelper(r, asxDate, euribor3m,
                                          Handle<Quote>(), Futures::ASX)));
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
                bondHelpers[i] = ext::shared_ptr<RateHelper>(new
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

        vars.termStructure = ext::shared_ptr<YieldTermStructure>(new
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
        ext::shared_ptr<IborIndex> euribor6m(new Euribor6M(curveHandle));
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
        vars.termStructure = ext::shared_ptr<YieldTermStructure>(new
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

            ext::shared_ptr<PricingEngine> bondEngine(
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
        vars.termStructure = ext::shared_ptr<YieldTermStructure>(new
            PiecewiseYieldCurve<T,I>(vars.settlement, vars.fraHelpers,
                                     Actual360(),
                                     interpolator));
        curveHandle.linkTo(vars.termStructure);

#ifdef QL_USE_INDEXED_COUPON
        bool useIndexedFra = false;
#else
        bool useIndexedFra = true;
#endif

        ext::shared_ptr<IborIndex> euribor3m(new Euribor3M(curveHandle));
        for (Size i=0; i<vars.fras; i++) {
            Date start =
                vars.calendar.advance(vars.settlement,
                                      fraData[i].n,
                                      fraData[i].units,
                                      euribor3m->businessDayConvention(),
                                      euribor3m->endOfMonth());
            BOOST_REQUIRE(fraData[i].units == Months);
            Date end = vars.calendar.advance(vars.settlement, 3 + fraData[i].n, Months,
                                             euribor3m->businessDayConvention(),
                                             euribor3m->endOfMonth());

            ForwardRateAgreement fra(start, end, Position::Long,
                                     fraData[i].rate/100, 100.0,
                                     euribor3m, curveHandle,
                                     useIndexedFra);
            Rate expectedRate = fraData[i].rate/100,
                 estimatedRate = fra.forwardRate();
            if (std::fabs(expectedRate-estimatedRate) > tolerance) {
                BOOST_ERROR(io::ordinal(i+1) << " FRA failure:" <<
                            std::setprecision(8) <<
                            "\n  estimated rate: " << io::rate(estimatedRate) <<
                            "\n  expected rate:  " << io::rate(expectedRate));
            }
        }

        // check immFuts
        vars.termStructure = ext::shared_ptr<YieldTermStructure>(new
            PiecewiseYieldCurve<T, I>(vars.settlement, vars.immFutHelpers,
            Actual360(),
            interpolator));
        curveHandle.linkTo(vars.termStructure);

        Date immStart = Date();
        for (Size i = 0; i<vars.immFuts; i++) {
            immStart = IMM::nextDate(immStart, false);
            // if the fixing is before the evaluation date, we
            // just jump forward by one future maturity
            if (euribor3m->fixingDate(immStart) <
                Settings::instance().evaluationDate())
                immStart = IMM::nextDate(immStart, false);
            Date end = vars.calendar.advance(immStart, 3, Months,
                euribor3m->businessDayConvention(),
                euribor3m->endOfMonth());

            ForwardRateAgreement immFut(immStart, end, Position::Long,
                immFutData[i].rate / 100, 100.0,
                euribor3m, curveHandle);
            Rate expectedRate = immFutData[i].rate / 100,
                estimatedRate = immFut.forwardRate();
            if (std::fabs(expectedRate - estimatedRate) > tolerance) {
                BOOST_ERROR(io::ordinal(i + 1) << " IMM futures failure:" <<
                    std::setprecision(8) <<
                    "\n  estimated rate: " << io::rate(estimatedRate) <<
                    "\n  expected rate:  " << io::rate(expectedRate));
            }
        }

        // check asxFuts
        vars.termStructure = ext::shared_ptr<YieldTermStructure>(new
            PiecewiseYieldCurve<T, I>(vars.settlement, vars.asxFutHelpers,
            Actual360(),
            interpolator));
        curveHandle.linkTo(vars.termStructure);

        Date asxStart = Date();
        for (Size i = 0; i<vars.asxFuts; i++) {
            asxStart = ASX::nextDate(asxStart, false);
            // if the fixing is before the evaluation date, we
            // just jump forward by one future maturity
            if (euribor3m->fixingDate(asxStart) <
                Settings::instance().evaluationDate())
                asxStart = ASX::nextDate(asxStart, false);
            if (euribor3m->fixingCalendar().isHoliday(asxStart))
                continue;
            Date end = vars.calendar.advance(asxStart, 3, Months,
                euribor3m->businessDayConvention(),
                euribor3m->endOfMonth());

            ForwardRateAgreement asxFut(asxStart, end, Position::Long,
                asxFutData[i].rate / 100, 100.0,
                euribor3m, curveHandle);
            Rate expectedRate = asxFutData[i].rate / 100,
                estimatedRate = asxFut.forwardRate();
            if (std::fabs(expectedRate - estimatedRate) > tolerance) {
                BOOST_ERROR(io::ordinal(i + 1) << " ASX futures failure:" <<
                    std::setprecision(8) <<
                    "\n  estimated rate: " << io::rate(estimatedRate) <<
                    "\n  expected rate:  " << io::rate(expectedRate));
            }
        }

    // end checks
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
            ext::shared_ptr<YieldTermStructure>(
                        new FlatForward(vars.settlement, 0.04, Actual360())));

        ext::shared_ptr<BMAIndex> bmaIndex(new BMAIndex);
        ext::shared_ptr<IborIndex> liborIndex(
                                        new USDLibor(3*Months,riskFreeCurve));
        for (Size i=0; i<vars.bmas; ++i) {
            Handle<Quote> f(vars.fractions[i]);
            vars.bmaHelpers[i] = ext::shared_ptr<RateHelper>(
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

        vars.termStructure = ext::shared_ptr<YieldTermStructure>(new
            PiecewiseYieldCurve<T,I,B>(vars.today, vars.bmaHelpers,
                                       Actual360(),
                                       interpolator));

        RelinkableHandle<YieldTermStructure> curveHandle;
        curveHandle.linkTo(vars.termStructure);

        // check BMA swaps
        ext::shared_ptr<BMAIndex> bma(new BMAIndex(curveHandle));
        ext::shared_ptr<IborIndex> libor3m(new USDLibor(3*Months,
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
            swap.setPricingEngine(ext::shared_ptr<PricingEngine>(
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

    using namespace piecewise_yield_curve_test;

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

    using namespace piecewise_yield_curve_test;

    CommonVars vars;

    testCurveConsistency<Discount,LogLinear,IterativeBootstrap>(vars);
    testBMACurveConsistency<Discount,LogLinear,IterativeBootstrap>(vars);
}

void PiecewiseYieldCurveTest::testLinearDiscountConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-linear discount curve...");

    using namespace piecewise_yield_curve_test;

    CommonVars vars;

    testCurveConsistency<Discount,Linear,IterativeBootstrap>(vars);
    testBMACurveConsistency<Discount,Linear,IterativeBootstrap>(vars);
}

void PiecewiseYieldCurveTest::testLinearZeroConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-linear zero-yield curve...");

    using namespace piecewise_yield_curve_test;

    CommonVars vars;

    testCurveConsistency<ZeroYield,Linear,IterativeBootstrap>(vars);
    testBMACurveConsistency<ZeroYield,Linear,IterativeBootstrap>(vars);
}

void PiecewiseYieldCurveTest::testSplineZeroConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-cubic zero-yield curve...");

    using namespace piecewise_yield_curve_test;

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

    using namespace piecewise_yield_curve_test;

    CommonVars vars;

    testCurveConsistency<ForwardRate,Linear,IterativeBootstrap>(vars);
    testBMACurveConsistency<ForwardRate,Linear,IterativeBootstrap>(vars);
}

void PiecewiseYieldCurveTest::testFlatForwardConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-flat forward-rate curve...");

    using namespace piecewise_yield_curve_test;

    CommonVars vars;

    testCurveConsistency<ForwardRate,BackwardFlat,IterativeBootstrap>(vars);
    testBMACurveConsistency<ForwardRate,BackwardFlat,IterativeBootstrap>(vars);
}

void PiecewiseYieldCurveTest::testSplineForwardConsistency() {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-cubic forward-rate curve...");

    using namespace piecewise_yield_curve_test;

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

    using namespace piecewise_yield_curve_test;

    CommonVars vars;
    testCurveConsistency<ForwardRate,ConvexMonotone,IterativeBootstrap>(vars);

    testBMACurveConsistency<ForwardRate,ConvexMonotone,
                            IterativeBootstrap>(vars);
}


void PiecewiseYieldCurveTest::testLocalBootstrapConsistency() {
    BOOST_TEST_MESSAGE(
        "Testing consistency of local-bootstrap algorithm...");

    using namespace piecewise_yield_curve_test;

    CommonVars vars;
    testCurveConsistency<ForwardRate,ConvexMonotone,LocalBootstrap>(
                                              vars, ConvexMonotone(), 1.0e-6);
    testBMACurveConsistency<ForwardRate,ConvexMonotone,LocalBootstrap>(
                                              vars, ConvexMonotone(), 1.0e-7);
}


void PiecewiseYieldCurveTest::testObservability() {

    BOOST_TEST_MESSAGE("Testing observability of piecewise yield curve...");

    using namespace piecewise_yield_curve_test;

    CommonVars vars;

    vars.termStructure = ext::shared_ptr<YieldTermStructure>(
       new PiecewiseYieldCurve<Discount,LogLinear>(vars.settlementDays,
                                                   vars.calendar,
                                                   vars.instruments,
                                                   Actual360()));
    Flag f;
    f.registerWith(vars.termStructure);

    for (Size i=0; i<vars.deposits+vars.swaps; i++) {
        Time testTime =
            Actual360().yearFraction(vars.settlement,
                                     vars.instruments[i]->pillarDate());
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

    using namespace piecewise_yield_curve_test;

    CommonVars vars;

    std::vector<ext::shared_ptr<RateHelper> > swapHelpers(vars.swaps);
    ext::shared_ptr<IborIndex> euribor6m(new Euribor6M);

    for (Size i=0; i<vars.swaps; i++) {
        Handle<Quote> r(vars.rates[i+vars.deposits]);
        swapHelpers[i] = ext::shared_ptr<RateHelper>(new
            SwapRateHelper(r, Period(swapData[i].n, swapData[i].units),
                           vars.calendar,
                           vars.fixedLegFrequency, vars.fixedLegConvention,
                           vars.fixedLegDayCounter, euribor6m));
    }

    vars.termStructure = ext::shared_ptr<YieldTermStructure>(new
        PiecewiseYieldCurve<Discount,LogLinear>(vars.settlement,
                                                swapHelpers,
                                                Actual360()));

    Handle<YieldTermStructure> curveHandle =
        Handle<YieldTermStructure>(vars.termStructure);

    ext::shared_ptr<IborIndex> index(new Euribor6M(curveHandle));
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

    using namespace piecewise_yield_curve_test;

    CommonVars vars;

    vars.today = Date(4, October, 2007);
    Settings::instance().evaluationDate() = vars.today;

    vars.calendar = Japan();
    vars.settlement =
        vars.calendar.advance(vars.today,vars.settlementDays,Days);

    // market elements
    vars.rates = std::vector<ext::shared_ptr<SimpleQuote> >(vars.swaps);
    for (Size i=0; i<vars.swaps; i++) {
        vars.rates[i] = ext::make_shared<SimpleQuote>(
                                       swapData[i].rate/100);
    }

    // rate helpers
    vars.instruments = std::vector<ext::shared_ptr<RateHelper> >(vars.swaps);

    ext::shared_ptr<IborIndex> index(new JPYLibor(6*Months));
    for (Size i=0; i<vars.swaps; i++) {
        Handle<Quote> r(vars.rates[i]);
        vars.instruments[i] = ext::shared_ptr<RateHelper>(
           new SwapRateHelper(r, swapData[i].n*swapData[i].units,
                              vars.calendar,
                              vars.fixedLegFrequency, vars.fixedLegConvention,
                              vars.fixedLegDayCounter, index));
    }

    vars.termStructure = ext::shared_ptr<YieldTermStructure>(
        new PiecewiseYieldCurve<Discount,LogLinear>(
                                       vars.settlement, vars.instruments,
                                       Actual360()));

    RelinkableHandle<YieldTermStructure> curveHandle;
    curveHandle.linkTo(vars.termStructure);

    // check swaps
    ext::shared_ptr<IborIndex> jpylibor6m(new JPYLibor(6*Months,curveHandle));
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

namespace piecewise_yield_curve_test {

    template <class T, class I>
    void testCurveCopy(CommonVars& vars,
                       const I& interpolator = I()) {

        PiecewiseYieldCurve<T,I> curve(vars.settlement, vars.instruments,
                                       Actual360(),
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

    using namespace piecewise_yield_curve_test;

    CommonVars vars;
    testCurveCopy<Discount,LogLinear>(vars);
}

void PiecewiseYieldCurveTest::testForwardCopy() {
    BOOST_TEST_MESSAGE("Testing copying of forward-rate curve...");

    using namespace piecewise_yield_curve_test;

    CommonVars vars;
    testCurveCopy<ForwardRate,BackwardFlat>(vars);
}

void PiecewiseYieldCurveTest::testZeroCopy() {
    BOOST_TEST_MESSAGE("Testing copying of zero-rate curve...");

    using namespace piecewise_yield_curve_test;

    CommonVars vars;
    testCurveCopy<ZeroYield,Linear>(vars);
}

void PiecewiseYieldCurveTest::testSwapRateHelperLastRelevantDate() {
    BOOST_TEST_MESSAGE("Testing SwapRateHelper last relevant date...");

    SavedSettings backup;
    Settings::instance().evaluationDate() = Date(22, Dec, 2016);
    Date today = Settings::instance().evaluationDate();

    Handle<YieldTermStructure> flat3m(
        ext::make_shared<FlatForward>(today, Handle<Quote>(ext::make_shared<SimpleQuote>(0.02)), Actual365Fixed()));
    ext::shared_ptr<IborIndex> usdLibor3m = ext::make_shared<USDLibor>(3 * Months, flat3m);

    // note that the calendar should be US+UK here actually, but technically it should also work with
    // the US calendar only
    ext::shared_ptr<RateHelper> helper = ext::make_shared<SwapRateHelper>(
        0.02, 50 * Years, UnitedStates(), Semiannual, ModifiedFollowing, Thirty360(), usdLibor3m);

    PiecewiseYieldCurve<Discount, LogLinear> curve(today, std::vector<ext::shared_ptr<RateHelper> >(1, helper),
                                                   Actual365Fixed());
    BOOST_CHECK_NO_THROW(curve.discount(1.0));
}

void PiecewiseYieldCurveTest::testBadPreviousCurve() {
    BOOST_TEST_MESSAGE("Testing bootstrap starting from bad guess...");

    using namespace piecewise_yield_curve_test;

    SavedSettings backup;

    Datum data[] = {
        {  1, Weeks,  -0.003488 },
        {  2, Weeks,  -0.0033 },
        {  6, Months, -0.00339 },
        {  2, Years,  -0.00336 },
        {  8, Years,   0.00302 },
        { 50, Years,   0.01185 }
    };

    std::vector<ext::shared_ptr<RateHelper> > helpers;
    ext::shared_ptr<Euribor> euribor1m(new Euribor1M);
    for (Size i=0; i<LENGTH(data); ++i) {
        helpers.push_back(
           ext::make_shared<SwapRateHelper>(data[i].rate,
                                              Period(data[i].n, data[i].units),
                                              TARGET(), Monthly, Unadjusted,
                                              Thirty360(), euribor1m));
    }

    Date today = Date(12, October, 2017);
    Date test_date = Date(16, December, 2016);

    Settings::instance().evaluationDate() = today;

    ext::shared_ptr<YieldTermStructure> curve =
        ext::make_shared<PiecewiseYieldCurve<ForwardRate, BackwardFlat> >(
                                            test_date, helpers, Actual360());

    // force bootstrap on today's date, so we have a previous curve...
    curve->discount(1.0);

    // ...then move to a date where the previous curve is a bad guess.
    Settings::instance().evaluationDate() = test_date;

    RelinkableHandle<YieldTermStructure> h;
    h.linkTo(curve);

    ext::shared_ptr<Euribor1M> index = ext::make_shared<Euribor1M>(h);
    for (Size i=0; i<LENGTH(data); i++) {
        Period tenor = data[i].n*data[i].units;

        VanillaSwap swap = MakeVanillaSwap(tenor, index, 0.0)
            .withFixedLegDayCount(Thirty360())
            .withFixedLegTenor(Period(1, Months))
            .withFixedLegConvention(Unadjusted);
        swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(h));

        Rate expectedRate = data[i].rate,
             estimatedRate = swap.fairRate();
        Spread error = std::fabs(expectedRate-estimatedRate);
        Real tolerance = 1.0e-9;
        if (error > tolerance) {
            BOOST_ERROR(tenor << " swap:\n"
                        << std::setprecision(8)
                        << "\n estimated rate: " << io::rate(estimatedRate)
                        << "\n expected rate:  " << io::rate(expectedRate)
                        << "\n error:          " << io::rate(error)
                        << "\n tolerance:      " << io::rate(tolerance));
        }
    }
}

void PiecewiseYieldCurveTest::testConstructionWithExplicitBootstrap() {

    BOOST_TEST_MESSAGE("Testing that construction with an explicit bootstrap succeeds...");

    using namespace piecewise_yield_curve_test;

    CommonVars vars;

    // With an explicit IterativeBootstrap object
    typedef PiecewiseYieldCurve<ForwardRate, Linear, IterativeBootstrap> PwLinearForward;
    ext::shared_ptr<YieldTermStructure> yts = 
        ext::make_shared<PwLinearForward>(
            vars.settlement, vars.instruments, Actual360(), Linear(),
            PwLinearForward::bootstrap_type());

    // Check anything to show that the construction succeeded
    BOOST_CHECK_NO_THROW(yts->discount(1.0, true));

    // With an explicit LocalBootstrap object
    typedef PiecewiseYieldCurve<ForwardRate, ConvexMonotone, LocalBootstrap> PwCmForward;
    yts = ext::make_shared<PwCmForward>(
        vars.settlement, vars.instruments, Actual360(), ConvexMonotone(), 
        PwCmForward::bootstrap_type());

    BOOST_CHECK_NO_THROW(yts->discount(1.0, true));
}

void PiecewiseYieldCurveTest::testLargeRates() {
    BOOST_TEST_MESSAGE("Testing bootstrap with large input rates...");

    using namespace piecewise_yield_curve_test;

    SavedSettings backup;

    Datum data[] = {
        {  1, Weeks,  2.418633 },
        {  2, Weeks,  1.361540 },
        {  3, Weeks,  1.195362 },
        {  1, Months, 0.829009 }
    };

    std::vector<ext::shared_ptr<RateHelper> > helpers;
    for (Size i=0; i<LENGTH(data); ++i) {
        helpers.push_back(
           ext::make_shared<DepositRateHelper>(data[i].rate,
                                               Period(data[i].n, data[i].units),
                                               0, WeekendsOnly(), Following,
                                               false, Actual360()));
    }

    Date today = Date(12, October, 2017);

    Settings::instance().evaluationDate() = today;

    Real accuracy = Null<Real>(); // use the default
    Real minValue = Null<Real>(); // use the default
    Real maxValue = 3.0;          // override

    typedef PiecewiseYieldCurve<ForwardRate, BackwardFlat> PiecewiseCurve;
    ext::shared_ptr<YieldTermStructure> curve =
        ext::make_shared<PiecewiseCurve>(
                                  today, helpers, Actual360(), BackwardFlat(),
                                  PiecewiseCurve::bootstrap_type(accuracy, minValue, maxValue));

    // force bootstrap and check it worked
    curve->discount(0.01);
    BOOST_CHECK_NO_THROW(curve->discount(0.01));
}

namespace piecewise_yield_curve_test {
    // helper classes for testGlobalBootstrap() below:

    // functor returning the additional error terms for the cost function
    struct additionalErrors {
        explicit additionalErrors(
            const std::vector<ext::shared_ptr<BootstrapHelper<YieldTermStructure> > >&
                additionalHelpers)
        : additionalHelpers(additionalHelpers) {}
        std::vector<ext::shared_ptr<BootstrapHelper<YieldTermStructure> > > additionalHelpers;
        Array operator()() {
            Array errors(5);
            Real a = additionalHelpers[0]->impliedQuote();
            Real b = additionalHelpers[6]->impliedQuote();
            for (Size k = 0; k < 5; ++k) {
                errors[k] = (5.0 - k) / 6.0 * a + (1.0 + k) / 6.0 * b -
                            additionalHelpers[1 + k]->impliedQuote();
            }
            return errors;
        }
    };

    // functor returning additional dates used in the bootstrap
    struct additionalDates {
        std::vector<Date> operator()() {
            Date settl = TARGET().advance(Settings::instance().evaluationDate(), 2 * Days);
            std::vector<Date> dates;
            for (Size i = 0; i < 5; ++i)
                dates.push_back(TARGET().advance(settl, (1 + i) * Months));
            return dates;
        }
    };
}

void PiecewiseYieldCurveTest::testGlobalBootstrap() {

    BOOST_TEST_MESSAGE("Testing global bootstrap...");

    using namespace piecewise_yield_curve_test;

    SavedSettings backup;

    Date today(26, Sep, 2019);
    Settings::instance().evaluationDate() = today;

    // Here we compare zero rates from bbg curve S45 (EUR-EURIBOR-6M) with QL. We assume the
    // following settings in bbg's SWDF screen (which are sort of the "factory" settings):
    // Curve Defaults                            : Pay = Mid, Rec = Mid
    // Interpolation Method                      : Piecewise linear (Simple-comp)
    // Enable OIS Discount/Dual Curve Stripping  : not enabled
    // We get a quite good match (1/100 bp), yet above numerical accuracy that might have to do
    // with details of bbg's "special FRA treatment" which is not fully disclosed

    // bbg maturity date
    Date bbgDate[] = {
        Date(31, Mar, 2020), Date(30, Apr, 2020), Date(29, May, 2020), Date(30, Jun, 2020),
        Date(31, Jul, 2020), Date(31, Aug, 2020), Date(30, Sep, 2020), Date(30, Oct, 2020),
        Date(30, Nov, 2020), Date(31, Dec, 2020), Date(29, Jan, 2021), Date(26, Feb, 2021),
        Date(31, Mar, 2021), Date(30, Sep, 2021), Date(30, Sep, 2022), Date(29, Sep, 2023),
        Date(30, Sep, 2024), Date(30, Sep, 2025), Date(30, Sep, 2026), Date(30, Sep, 2027),
        Date(29, Sep, 2028), Date(28, Sep, 2029), Date(30, Sep, 2030), Date(30, Sep, 2031),
        Date(29, Sep, 2034), Date(30, Sep, 2039), Date(30, Sep, 2044), Date(30, Sep, 2049),
        Date(30, Sep, 2054), Date(30, Sep, 2059), Date(30, Sep, 2064), Date(30, Sep, 2069)};

    // bbg market rate
    Real bbgMktRate[] = {-0.373,   -0.388,   -0.402,   -0.418,   -0.431,  -0.441,   -0.45,
                         -0.457,   -0.463,   -0.469,   -0.461,   -0.463,  -0.479,   -0.4511,
                         -0.45418, -0.439,   -0.4124,  -0.37703, -0.3335, -0.28168, -0.22725,
                         -0.1745,  -0.12425, -0.07746, 0.0385,   0.1435,  0.17525,  0.17275,
                         0.1515,   0.1225,   0.095,    0.0644};

    // bbg zero rate
    Real bbgZeroRate[] = {-0.373,   -0.38058, -0.38718, -0.39353, -0.407,   -0.41274, -0.41107,
                          -0.41542, -0.41951, -0.42329, -0.42658, -0.42959, -0.43297, -0.45103,
                          -0.4541,  -0.43905, -0.41266, -0.3775,  -0.33434, -0.2828,  -0.2285,
                          -0.17582, -0.1254,  -0.0783,  0.03913,  0.14646,  0.17874,  0.17556,
                          0.1531,   0.12299,  0.0948,   0.06383};

    // build ql helpers
    std::vector<ext::shared_ptr<RateHelper> > helpers;
    ext::shared_ptr<IborIndex> index = ext::make_shared<Euribor>(6 * Months);

    helpers.push_back(ext::make_shared<DepositRateHelper>(
        bbgMktRate[0] / 100.0, 6 * Months, 2, TARGET(), ModifiedFollowing, true, Actual360()));

    for (Size i = 0; i < 12; ++i) {
        helpers.push_back(
            ext::make_shared<FraRateHelper>(bbgMktRate[1 + i] / 100.0, (i + 1) * Months, index));
    }

    Size swapTenors[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15, 20, 25, 30, 35, 40, 45, 50};
    for (Size i = 0; i < 19; ++i) {
        helpers.push_back(ext::make_shared<SwapRateHelper>(bbgMktRate[13 + i] / 100.0,
                                                           swapTenors[i] * Years, TARGET(), Annual,
                                                           ModifiedFollowing, Thirty360(), index));
    }

    // global bootstrap constraints (bbg 'special serial FRA treatment')
    std::vector<ext::shared_ptr<BootstrapHelper<YieldTermStructure> > > additionalHelpers;

    // set up the additional rate helpers we need in the cost function
    for (Size i = 0; i < 7; ++i) {
        additionalHelpers.push_back(
            ext::make_shared<FraRateHelper>(-0.004, (12 + i) * Months, index));
    }

    // build curve with additional dates and constraints using a global bootstrapper
    typedef PiecewiseYieldCurve<SimpleZeroYield, Linear, GlobalBootstrap> bbgCurve;
    ext::shared_ptr<bbgCurve> curve = ext::make_shared<bbgCurve>(
        2, TARGET(), helpers, Actual365Fixed(), std::vector<Handle<Quote> >(), std::vector<Date>(),
        Linear(),
        bbgCurve::bootstrap_type(additionalHelpers, additionalDates(),
                                 additionalErrors(additionalHelpers), 1.0e-12));
    curve->enableExtrapolation();

    // check ql vs bbg curve pillar dates
    for (Size i = 0; i < LENGTH(bbgDate); ++i) {
        BOOST_CHECK_EQUAL(bbgDate[i], helpers[i]->pillarDate());
    }

    // check ql vs bbg zero rates
    for (Size i = 0; i < LENGTH(bbgZeroRate); ++i) {
        // account for the way bbg displays zero rates
        DayCounter dc;
        Compounding comp;
        if (i < 13) {
            dc = Actual360();
            comp = Simple;
        } else {
            dc = Thirty360();
            comp = SimpleThenCompounded;
        }
        // 0.01 basis points tolerance
        BOOST_CHECK_SMALL(std::fabs(bbgZeroRate[i] / 100.0 -
                                    curve->zeroRate(bbgDate[i], dc, comp, Annual).rate()),
                          1E-6);
    }
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

    suite->add(QUANTLIB_TEST_CASE(
               &PiecewiseYieldCurveTest::testSwapRateHelperLastRelevantDate));

    if (IborCoupon::usingAtParCoupons()) {
        // This regression test didn't work with indexed coupons anyway.
        suite->add(QUANTLIB_TEST_CASE(
               &PiecewiseYieldCurveTest::testBadPreviousCurve));
    }

    suite->add(QUANTLIB_TEST_CASE(&PiecewiseYieldCurveTest::testConstructionWithExplicitBootstrap));
    suite->add(QUANTLIB_TEST_CASE(&PiecewiseYieldCurveTest::testLargeRates));

#ifndef QL_USE_INDEXED_COUPON
    suite->add(QUANTLIB_TEST_CASE(&PiecewiseYieldCurveTest::testGlobalBootstrap));
#endif

    return suite;
}
