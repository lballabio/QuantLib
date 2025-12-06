/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007, 2008, 2009, 2017 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/experimental/termstructures/basisswapratehelpers.hpp>
#include <ql/indexes/bmaindex.hpp>
#include <ql/indexes/ibor/estr.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/jpylibor.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/instruments/forwardrateagreement.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/math/interpolations/convexmonotoneinterpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/forwardflatinterpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/quotes/futuresconvadjustmentquote.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/globalbootstrap.hpp>
#include <ql/termstructures/globalbootstrapvars.hpp>
#include <ql/termstructures/localbootstrap.hpp>
#include <ql/termstructures/multicurve.hpp>
#include <ql/termstructures/yield/bondhelpers.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/termstructures/yield/piecewisespreadyieldcurve.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>
#include <ql/time/asx.hpp>
#include <ql/time/calendars/canada.hpp>
#include <ql/time/calendars/japan.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/mexico.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/imm.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <iomanip>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using std::map;
using std::vector;
using std::string;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(PiecewiseYieldCurveTests)

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
    std::vector<ext::shared_ptr<RateHelper> > instruments,
        immFutHelpers, asxFutHelpers,
        bondHelpers, bmaHelpers;

    std::vector<ext::shared_ptr<RateHelper> > fraHelpers(bool useIndexedFra) const {
        auto helpers = std::vector<ext::shared_ptr<RateHelper> >(fras);
        auto euribor3m = ext::make_shared<Euribor3M>();
        for (Size i=0; i<fras; i++) {
            Handle<Quote> r(fraRates[i]);
            helpers[i] = ext::make_shared<FraRateHelper>(
                    r, fraData[i].n, fraData[i].n + 3,
                    euribor3m->fixingDays(),
                    euribor3m->fixingCalendar(),
                    euribor3m->businessDayConvention(),
                    euribor3m->endOfMonth(),
                    euribor3m->dayCounter(),
                    Pillar::LastRelevantDate,
                    Date(),
                    useIndexedFra);
        }

        return helpers;
    }

    std::vector<Schedule> schedules;
    ext::shared_ptr<YieldTermStructure> termStructure;

    // setup
    CommonVars(Date evaluationDate = Date()) {
        // data
        calendar = TARGET();
        settlementDays = 2;
        today = calendar.adjust(evaluationDate != Date() ? evaluationDate : Date::todaysDate());
        Settings::instance().evaluationDate() = today;
        settlement = calendar.advance(today,settlementDays,Days);
        fixedLegConvention = Unadjusted;
        fixedLegFrequency = Annual;
        fixedLegDayCounter = Thirty360(Thirty360::BondBasis);
        bondSettlementDays = 3;
        bondDayCounter = ActualActual(ActualActual::ISDA);
        bondConvention = Following;
        bondRedemption = 100.0;
        bmaFrequency = Quarterly;
        bmaConvention = Following;
        bmaDayCounter = ActualActual(ActualActual::ISDA);

        deposits = std::size(depositData);
        fras = std::size(fraData);
        immFuts = std::size(immFutData);
        asxFuts = std::size(asxFutData);
        swaps = std::size(swapData);
        bonds = std::size(bondData);
        bmas = std::size(bmaData);

        // market elements
        rates = std::vector<ext::shared_ptr<SimpleQuote> >(deposits+swaps);
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
        instruments = std::vector<ext::shared_ptr<RateHelper> >(deposits+swaps);
        immFutHelpers = std::vector<ext::shared_ptr<RateHelper> >(immFuts);
        asxFutHelpers = std::vector<ext::shared_ptr<RateHelper> >();
        bondHelpers = std::vector<ext::shared_ptr<RateHelper> >(bonds);
        schedules = std::vector<Schedule>(bonds);
        bmaHelpers = std::vector<ext::shared_ptr<RateHelper> >(bmas);

        auto euribor6m = ext::make_shared<Euribor6M>();
        for (Size i=0; i<deposits; i++) {
            Handle<Quote> r(rates[i]);
            instruments[i] = ext::make_shared<DepositRateHelper>(
                r, ext::make_shared<Euribor>(depositData[i].n*depositData[i].units));
        }
        for (Size i=0; i<swaps; i++) {
            Handle<Quote> r(rates[i+deposits]);
            instruments[i+deposits] = ext::make_shared<SwapRateHelper>(
                                   r, swapData[i].n*swapData[i].units,
                                   calendar,
                                   fixedLegFrequency, fixedLegConvention,
                                   fixedLegDayCounter, euribor6m);
        }

        Date immDate = Date();
        auto euribor3m = ext::make_shared<Euribor3M>();
        for (Size i = 0; i<immFuts; i++) {
            Handle<Quote> r(immFutPrices[i]);
            immDate = IMM::nextDate(immDate, false);
            // if the fixing is before the evaluation date, we
            // just jump forward by one future maturity
            if (euribor3m->fixingDate(immDate) < Settings::instance().evaluationDate())
                immDate = IMM::nextDate(immDate, false);
            immFutHelpers[i] = ext::make_shared<FuturesRateHelper>(
                                      r, immDate, euribor3m, Handle<Quote>(),
                                      Futures::IMM);
        }
        Date asxDate = Date();
        for (Size i = 0; i<asxFuts; i++) {
            Handle<Quote> r(asxFutPrices[i]);
            asxDate = ASX::nextDate(asxDate, false);
            // if the fixing is before the evaluation date, we
            // just jump forward by one future maturity
            if (euribor3m->fixingDate(asxDate) < Settings::instance().evaluationDate())
                asxDate = ASX::nextDate(asxDate, false);
            if (euribor3m->fixingCalendar().isBusinessDay(asxDate))
                asxFutHelpers.push_back(ext::make_shared<FuturesRateHelper>(
                                          r, asxDate, euribor3m,
                                          Handle<Quote>(), Futures::ASX));
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
            bondHelpers[i] = ext::make_shared<FixedRateBondHelper>(
                                        p,
                                        bondSettlementDays,
                                        bondRedemption, schedules[i],
                                        coupons, bondDayCounter,
                                        bondConvention,
                                        bondRedemption, issue);
        }
    }
};


template <class T, class I, template<class C> class B>
void testCurveConsistency(CommonVars& vars,
                          const I& interpolator = I(),
                          Real tolerance = 1.0e-9) {

    vars.termStructure = ext::make_shared<PiecewiseYieldCurve<T,I,B>>(
                                       vars.settlement, vars.instruments,
                                       Actual360(),
                                       interpolator);

    RelinkableHandle<YieldTermStructure> curveHandle;
    curveHandle.linkTo(vars.termStructure);

    // check deposits
    for (Size i=0; i<vars.deposits; i++) {
        Euribor index(depositData[i].n*depositData[i].units,curveHandle);
        Rate expectedRate  = depositData[i].rate/100,
            estimatedRate = index.fixing(vars.today);
        if (std::fabs(expectedRate-estimatedRate) > tolerance) {
            BOOST_ERROR(depositData[i].n << " "
                        << (depositData[i].units == Weeks ? "week(s)" : "month(s)")
                        << " deposit:"
                        << std::setprecision(8)
                        << "\n    estimated rate: " << io::rate(estimatedRate)
                        << "\n    expected rate:  " << io::rate(expectedRate));
        }
    }

    // check swaps
    auto euribor6m = ext::make_shared<Euribor6M>(curveHandle);
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
    vars.termStructure = ext::make_shared<PiecewiseYieldCurve<T,I,B>>(
                                       vars.settlement, vars.bondHelpers,
                                       Actual360(),
                                       interpolator);
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

        auto bondEngine = ext::make_shared<DiscountingBondEngine>(curveHandle);
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

    // check FRA, use indexed

    bool useIndexedFra = true;
    auto euribor3m = ext::make_shared<Euribor3M>(curveHandle);

    vars.termStructure = ext::make_shared<PiecewiseYieldCurve<T,I>>(
                                     vars.settlement, vars.fraHelpers(useIndexedFra),
                                     Actual360(),
                                     interpolator);
    curveHandle.linkTo(vars.termStructure);

    for (Size i=0; i<vars.fras; i++) {
        Date start =
            vars.calendar.advance(vars.settlement,
                                  fraData[i].n,
                                  fraData[i].units,
                                  euribor3m->businessDayConvention(),
                                  euribor3m->endOfMonth());
        BOOST_REQUIRE(fraData[i].units == Months);

        ForwardRateAgreement fra(euribor3m, start, Position::Long,
                                 fraData[i].rate/100, 100.0, curveHandle);
        Rate expectedRate = fraData[i].rate/100,
            estimatedRate = fra.forwardRate();
        if (std::fabs(expectedRate-estimatedRate) > tolerance) {
            BOOST_ERROR(io::ordinal(i+1) << " FRA (indexed) failure:" <<
                        std::setprecision(8) <<
                        "\n  estimated rate: " << io::rate(estimatedRate) <<
                        "\n  expected rate:  " << io::rate(expectedRate));
        }
    }

    // check FRA, don't use indexed

    useIndexedFra = false;

    vars.termStructure = ext::make_shared<PiecewiseYieldCurve<T,I>>(
                                     vars.settlement, vars.fraHelpers(useIndexedFra),
                                     Actual360(),
                                     interpolator);
    curveHandle.linkTo(vars.termStructure);

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
        ForwardRateAgreement fra(euribor3m, start, end, Position::Long,
                                 fraData[i].rate/100, 100.0, curveHandle);
        Rate expectedRate = fraData[i].rate/100,
            estimatedRate = fra.forwardRate();
        if (std::fabs(expectedRate-estimatedRate) > tolerance) {
            BOOST_ERROR(io::ordinal(i+1) << " FRA (at par) failure:" <<
                        std::setprecision(8) <<
                        "\n  estimated rate: " << io::rate(estimatedRate) <<
                        "\n  expected rate:  " << io::rate(expectedRate));
        }
    }

    // check immFuts
    vars.termStructure = ext::make_shared<PiecewiseYieldCurve<T, I>>(
                                      vars.settlement, vars.immFutHelpers,
                                      Actual360(),
                                      interpolator);
    curveHandle.linkTo(vars.termStructure);

    Date immStart = Date();
    for (Size i = 0; i<vars.immFuts; i++) {
        immStart = IMM::nextDate(immStart, false);
        // if the fixing is before the evaluation date, we
        // just jump forward by one future maturity
        if (euribor3m->fixingDate(immStart) < Settings::instance().evaluationDate())
            immStart = IMM::nextDate(immStart, false);

        ForwardRateAgreement immFut(euribor3m, immStart, Position::Long,
                                    immFutData[i].rate / 100, 100.0, curveHandle);
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
    vars.termStructure = ext::make_shared<PiecewiseYieldCurve<T, I>>(
                                      vars.settlement, vars.asxFutHelpers,
                                      Actual360(),
                                      interpolator);
    curveHandle.linkTo(vars.termStructure);

    Date asxStart = Date();
    for (Size i = 0; i<vars.asxFuts; i++) {
        asxStart = ASX::nextDate(asxStart, false);
        // if the fixing is before the evaluation date, we
        // just jump forward by one future maturity
        if (euribor3m->fixingDate(asxStart) < Settings::instance().evaluationDate())
            asxStart = ASX::nextDate(asxStart, false);
        if (euribor3m->fixingCalendar().isHoliday(asxStart))
            continue;

        ForwardRateAgreement asxFut(euribor3m, asxStart, Position::Long,
                                    asxFutData[i].rate / 100, 100.0, curveHandle);
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
            ext::make_shared<FlatForward>(vars.settlement, 0.04, Actual360()));

    auto bmaIndex = ext::make_shared<BMAIndex>();
    auto liborIndex = ext::make_shared<USDLibor>(3*Months,riskFreeCurve);
    for (Size i=0; i<vars.bmas; ++i) {
        Handle<Quote> f(vars.fractions[i]);
        vars.bmaHelpers[i] = ext::make_shared<BMASwapRateHelper>(
                                            f, bmaData[i].n*bmaData[i].units,
                                            vars.settlementDays,
                                            vars.calendar,
                                            Period(vars.bmaFrequency),
                                            vars.bmaConvention,
                                            vars.bmaDayCounter,
                                            bmaIndex,
                                            liborIndex);
    }

    Weekday w = vars.today.weekday();
    Date lastWednesday =
        (w >= 4) ? vars.today - (w - 4) : vars.today + (4 - w - 7);
    Date lastFixing = bmaIndex->fixingCalendar().adjust(lastWednesday);
    bmaIndex->addFixing(lastFixing, 0.03);

    vars.termStructure = ext::make_shared<PiecewiseYieldCurve<T,I,B>>(
                                       vars.today, vars.bmaHelpers,
                                       Actual360(),
                                       interpolator);

    RelinkableHandle<YieldTermStructure> curveHandle;
    curveHandle.linkTo(vars.termStructure);

    // check BMA swaps
    auto bma = ext::make_shared<BMAIndex>(curveHandle);
    auto libor3m = ext::make_shared<USDLibor>(3*Months, riskFreeCurve);
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


        BMASwap swap(Swap::Payer, 100.0,
                     liborSchedule, 0.75, 0.0,
                     libor3m, libor3m->dayCounter(),
                     bmaSchedule, bma, vars.bmaDayCounter);
        swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(
                                        libor3m->forwardingTermStructure()));

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


//Unstable
//BOOST_AUTO_TEST_CASE(testLogCubicDiscountConsistency) {
//
//    BOOST_TEST_MESSAGE(
//        "Testing consistency of piecewise-log-cubic discount curve...");
//
//    using namespace piecewise_yield_curve_test;
//
//    CommonVars vars;
//
//    testCurveConsistency<Discount,LogCubic,IterativeBootstrap>(
//        vars,
//        MonotonicLogCubic());
//    testBMACurveConsistency<Discount,LogCubic,IterativeBootstrap>(
//        vars,
//        MonotonicLogCubic());
//}

BOOST_AUTO_TEST_CASE(testLogLinearDiscountConsistency) {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-log-linear discount curve...");

    CommonVars vars;

    testCurveConsistency<Discount,LogLinear,IterativeBootstrap>(vars);
    testBMACurveConsistency<Discount,LogLinear,IterativeBootstrap>(vars);
}

BOOST_AUTO_TEST_CASE(testLinearDiscountConsistency) {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-linear discount curve...");

    CommonVars vars;

    testCurveConsistency<Discount,Linear,IterativeBootstrap>(vars);
    testBMACurveConsistency<Discount,Linear,IterativeBootstrap>(vars);
}

BOOST_AUTO_TEST_CASE(testLinearZeroConsistency) {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-linear zero-yield curve...");

    CommonVars vars;

    testCurveConsistency<ZeroYield,Linear,IterativeBootstrap>(vars);
    testBMACurveConsistency<ZeroYield,Linear,IterativeBootstrap>(vars);
}

BOOST_AUTO_TEST_CASE(testSplineZeroConsistency) {

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

BOOST_AUTO_TEST_CASE(testLinearForwardConsistency) {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-linear forward-rate curve...");

    CommonVars vars;

    testCurveConsistency<ForwardRate,Linear,IterativeBootstrap>(vars);
    testBMACurveConsistency<ForwardRate,Linear,IterativeBootstrap>(vars);
}

BOOST_AUTO_TEST_CASE(testFlatForwardConsistency) {

    BOOST_TEST_MESSAGE(
        "Testing consistency of piecewise-flat forward-rate curve...");

    CommonVars vars;

    testCurveConsistency<ForwardRate,BackwardFlat,IterativeBootstrap>(vars);
    testBMACurveConsistency<ForwardRate,BackwardFlat,IterativeBootstrap>(vars);
}

//Unstable
//BOOST_AUTO_TEST_CASE(testSplineForwardConsistency) {
//
//    BOOST_TEST_MESSAGE(
//        "Testing consistency of piecewise-cubic forward-rate curve...");
//
//    using namespace piecewise_yield_curve_test;
//
//    CommonVars vars;
//
//    testCurveConsistency<ForwardRate,Cubic,IterativeBootstrap>(
//                   vars,
//                   Cubic(CubicInterpolation::Spline, true,
//                         CubicInterpolation::SecondDerivative, 0.0,
//                         CubicInterpolation::SecondDerivative, 0.0));
//    testBMACurveConsistency<ForwardRate,Cubic,IterativeBootstrap>(
//                   vars,
//                   Cubic(CubicInterpolation::Spline, true,
//                         CubicInterpolation::SecondDerivative, 0.0,
//                         CubicInterpolation::SecondDerivative, 0.0));
//}

BOOST_AUTO_TEST_CASE(testConvexMonotoneForwardConsistency) {
    BOOST_TEST_MESSAGE(
        "Testing consistency of convex monotone forward-rate curve...");

    CommonVars vars;
    testCurveConsistency<ForwardRate,ConvexMonotone,IterativeBootstrap>(vars);

    testBMACurveConsistency<ForwardRate,ConvexMonotone,
                            IterativeBootstrap>(vars);
}

BOOST_AUTO_TEST_CASE(testLocalBootstrapConsistency) {
    BOOST_TEST_MESSAGE(
        "Testing consistency of local-bootstrap algorithm...");

    CommonVars vars;
    testCurveConsistency<ForwardRate,ConvexMonotone,LocalBootstrap>(
                                              vars, ConvexMonotone(), 1.0e-6);
    testBMACurveConsistency<ForwardRate,ConvexMonotone,LocalBootstrap>(
                                              vars, ConvexMonotone(), 1.0e-7);
}

BOOST_AUTO_TEST_CASE(testParFraRegression) {
    BOOST_TEST_MESSAGE("Testing regression for at-par FRA...");

    CommonVars vars(Date(23, February, 2023));

    bool useIndexedFra = false;
    RelinkableHandle<YieldTermStructure> curveHandle;
    auto euribor3m = ext::make_shared<Euribor3M>(curveHandle);

    vars.termStructure = ext::make_shared<PiecewiseYieldCurve<ZeroYield, Linear>>(
        vars.settlement, vars.fraHelpers(useIndexedFra), Actual360());
    curveHandle.linkTo(vars.termStructure);

    for (Size i=0; i<vars.fras; i++) {
        Date start = vars.calendar.advance(vars.settlement,
                                           fraData[i].n,
                                           fraData[i].units,
                                           euribor3m->businessDayConvention(),
                                           euribor3m->endOfMonth());
        BOOST_REQUIRE(fraData[i].units == Months);

        Date end = vars.calendar.advance(vars.settlement, 3 + fraData[i].n, Months,
                                         euribor3m->businessDayConvention(),
                                         euribor3m->endOfMonth());
        ForwardRateAgreement fra(euribor3m, start, end, Position::Long,
                                 fraData[i].rate/100, 100.0, curveHandle);
        Rate expectedRate = fraData[i].rate/100;
        Rate estimatedRate = fra.forwardRate();
        Real tolerance = 1.0e-6;
        if (std::fabs(expectedRate-estimatedRate) > tolerance) {
            BOOST_ERROR(io::ordinal(i+1) << " FRA (at par) failure:" <<
                        std::setprecision(8) <<
                        "\n  estimated rate: " << io::rate(estimatedRate) <<
                        "\n  expected rate:  " << io::rate(expectedRate));
        }
    }
}

BOOST_AUTO_TEST_CASE(testObservability) {

    BOOST_TEST_MESSAGE("Testing observability of piecewise yield curve...");

    CommonVars vars;

    vars.termStructure = ext::make_shared<PiecewiseYieldCurve<Discount,LogLinear>>(
                                                   vars.settlementDays,
                                                   vars.calendar,
                                                   vars.instruments,
                                                   Actual360());

    ext::dynamic_pointer_cast<LazyObject>(vars.termStructure)->forwardFirstNotificationOnly();

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

BOOST_AUTO_TEST_CASE(testLiborFixing) {

    BOOST_TEST_MESSAGE(
        "Testing use of today's LIBOR fixings in swap curve...");

    CommonVars vars;

    std::vector<ext::shared_ptr<RateHelper> > swapHelpers(vars.swaps);
    auto euribor6m = ext::make_shared<Euribor6M>();

    for (Size i=0; i<vars.swaps; i++) {
        Handle<Quote> r(vars.rates[i+vars.deposits]);
        swapHelpers[i] = ext::make_shared<SwapRateHelper>(
                           r, Period(swapData[i].n, swapData[i].units),
                           vars.calendar,
                           vars.fixedLegFrequency, vars.fixedLegConvention,
                           vars.fixedLegDayCounter, euribor6m);
    }

    vars.termStructure =
        ext::make_shared<PiecewiseYieldCurve<Discount,LogLinear>>(
                                                vars.settlement,
                                                swapHelpers,
                                                Actual360());

    auto curveHandle =
        Handle<YieldTermStructure>(vars.termStructure);

    auto index = ext::make_shared<Euribor6M>(curveHandle);
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

BOOST_AUTO_TEST_CASE(testJpyLibor) {
    BOOST_TEST_MESSAGE(
        "Testing bootstrap over JPY LIBOR swaps...");

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

    auto index = ext::make_shared<JPYLibor>(6*Months);
    for (Size i=0; i<vars.swaps; i++) {
        Handle<Quote> r(vars.rates[i]);
        vars.instruments[i] = ext::make_shared<SwapRateHelper>(
                              r, swapData[i].n*swapData[i].units,
                              vars.calendar,
                              vars.fixedLegFrequency, vars.fixedLegConvention,
                              vars.fixedLegDayCounter, index);
    }

    vars.termStructure =
        ext::make_shared<PiecewiseYieldCurve<Discount,LogLinear>>(
                                       vars.settlement, vars.instruments,
                                       Actual360());

    RelinkableHandle<YieldTermStructure> curveHandle;
    curveHandle.linkTo(vars.termStructure);

    // check swaps
    auto jpylibor6m = ext::make_shared<JPYLibor>(6*Months, curveHandle);
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


BOOST_AUTO_TEST_CASE(testCA365Futures) {

    BOOST_TEST_MESSAGE("Testing futures rate helpers with act/365 Canadian day counter...");

    CommonVars vars;

    Settings::instance().evaluationDate() = vars.today;

    auto index =
        ext::make_shared<IborIndex>("foo", 3*Months, 2, Currency(),
                                    Canada(), ModifiedFollowing, true,
                                    Actual365Fixed(Actual365Fixed::Canadian));

    Date immDate = Date();
    for (Size i = 0; i<vars.immFuts; i++) {
        Handle<Quote> r(vars.immFutPrices[i]);
        immDate = IMM::nextDate(immDate, false);
        // if the fixing is before the evaluation date, we
        // just jump forward by one future maturity
        if (index->fixingDate(immDate) < Settings::instance().evaluationDate())
            immDate = IMM::nextDate(immDate, false);
        vars.immFutHelpers[i] =
            ext::make_shared<FuturesRateHelper>(r, immDate, index, Handle<Quote>(), Futures::IMM);
    }

    auto termStructure =
        ext::make_shared<PiecewiseYieldCurve<Discount,LogLinear>>(
            vars.settlement, vars.immFutHelpers, Actual360());

    BOOST_CHECK_NO_THROW(termStructure->nodes());
}


BOOST_AUTO_TEST_CASE(testDefaultInstantiation) {

    BOOST_TEST_MESSAGE("Testing instantiation of curves without passing an interpolator...");

    CommonVars vars;

    // no actual tests at runtime; this tests that all these instantiations compile
    PiecewiseYieldCurve<Discount, Linear> linear(vars.settlement, vars.instruments, Actual360());
    PiecewiseYieldCurve<Discount, LogLinear> log_linear(vars.settlement, vars.instruments, Actual360());
    PiecewiseYieldCurve<Discount, Cubic> cubic(vars.settlement, vars.instruments, Actual360());
    PiecewiseYieldCurve<Discount, DefaultLogCubic> log_cubic(vars.settlement, vars.instruments, Actual360());
    PiecewiseYieldCurve<Discount, MonotonicLogCubic> monotonic_log_cubic(vars.settlement, vars.instruments, Actual360());
    PiecewiseYieldCurve<Discount, KrugerLog> kruger_log_cubic(vars.settlement, vars.instruments, Actual360());
    PiecewiseYieldCurve<ForwardRate, BackwardFlat> backward(vars.settlement, vars.instruments, Actual360());
    PiecewiseYieldCurve<ForwardRate, ForwardFlat> forward(vars.settlement, vars.instruments, Actual360());
    PiecewiseYieldCurve<ForwardRate, ConvexMonotone> convex(vars.settlement, vars.instruments, Actual360());
}

BOOST_AUTO_TEST_CASE(testSwapRateHelperLastRelevantDate) {
    BOOST_TEST_MESSAGE("Testing SwapRateHelper last relevant date...");

    Settings::instance().evaluationDate() = Date(22, Dec, 2016);
    Date today = Settings::instance().evaluationDate();

    Handle<YieldTermStructure> flat3m(
        ext::make_shared<FlatForward>(today, Handle<Quote>(ext::make_shared<SimpleQuote>(0.02)), Actual365Fixed()));
    ext::shared_ptr<IborIndex> usdLibor3m = ext::make_shared<USDLibor>(3 * Months, flat3m);

    // note that the calendar should be US+UK here actually, but technically it should also work with
    // the US calendar only
    ext::shared_ptr<RateHelper> helper = ext::make_shared<SwapRateHelper>(
        0.02, 50 * Years, UnitedStates(UnitedStates::GovernmentBond), Semiannual, ModifiedFollowing,
        Thirty360(Thirty360::BondBasis), usdLibor3m);

    PiecewiseYieldCurve<Discount, LogLinear> curve(today, std::vector<ext::shared_ptr<RateHelper> >(1, helper),
                                                   Actual365Fixed());
    BOOST_CHECK_NO_THROW(curve.discount(1.0));
}

BOOST_AUTO_TEST_CASE(testSwapRateHelperSpotDate) {
    BOOST_TEST_MESSAGE("Testing SwapRateHelper spot date...");

    ext::shared_ptr<IborIndex> usdLibor3m = ext::make_shared<USDLibor>(3 * Months);

    ext::shared_ptr<SwapRateHelper> helper = ext::make_shared<SwapRateHelper>(
        0.02, 5 * Years, UnitedStates(UnitedStates::GovernmentBond), Semiannual, ModifiedFollowing,
        Thirty360(Thirty360::BondBasis), usdLibor3m);

    Settings::instance().evaluationDate() = Date(11, October, 2019);

    // Advancing 2 days on the US calendar would yield October 16th (because October 14th
    // is Columbus day), but the LIBOR spot is calculated advancing on the UK calendar,
    // resulting in October 15th which is also a business day for the US calendar.
    Date expected = Date(15, October, 2019);
    Date calculated = helper->swap()->startDate();
    if (calculated != expected)
        BOOST_ERROR("expected spot date: " << expected << "\n"
                    "calculated:         " << calculated);

    // Settings::instance().evaluationDate() = Date(1, July, 2020);

    // TODO: July 3rd is holiday in the US, but not for LIBOR purposes.  This should probably
    // be considered when building the schedule.
    // expected = Date(3, July, 2020);
    // calculated = helper->swap()->startDate();
    // if (calculated != expected)
    //     BOOST_ERROR("expected spot date: " << expected << "\n"
    //                 "calculated:         " << calculated);
}

// This regression test didn't work with indexed coupons anyway.
BOOST_AUTO_TEST_CASE(testBadPreviousCurve, *precondition(usingAtParCoupons())) {
    BOOST_TEST_MESSAGE("Testing bootstrap starting from bad guess...");

    Datum data[] = {
        {  1, Weeks,  -0.003488 },
        {  2, Weeks,  -0.0033 },
        {  6, Months, -0.00339 },
        {  2, Years,  -0.00336 },
        {  8, Years,   0.00302 },
        { 50, Years,   0.01185 }
    };

    std::vector<ext::shared_ptr<RateHelper> > helpers;
    auto euribor1m = ext::make_shared<Euribor1M>();
    for (auto& i : data) {
        helpers.push_back(ext::make_shared<SwapRateHelper>(
            i.rate, Period(i.n, i.units), TARGET(), Monthly, Unadjusted,
            Thirty360(Thirty360::BondBasis), euribor1m));
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
    for (auto& i : data) {
        Period tenor = i.n * i.units;

        VanillaSwap swap = MakeVanillaSwap(tenor, index, 0.0)
            .withFixedLegDayCount(Thirty360(Thirty360::BondBasis))
            .withFixedLegTenor(Period(1, Months))
            .withFixedLegConvention(Unadjusted);
        swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(h));

        Rate expectedRate = i.rate, estimatedRate = swap.fairRate();
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

BOOST_AUTO_TEST_CASE(testConstructionWithExplicitBootstrap) {

    BOOST_TEST_MESSAGE("Testing that construction with an explicit bootstrap succeeds...");

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

BOOST_AUTO_TEST_CASE(testLargeRates) {
    BOOST_TEST_MESSAGE("Testing bootstrap with large input rates...");

    Datum data[] = {
        {  1, Weeks,  2.418633 },
        {  2, Weeks,  1.361540 },
        {  3, Weeks,  1.195362 },
        {  1, Months, 0.829009 }
    };

    std::vector<ext::shared_ptr<RateHelper> > helpers;
    for (auto& i : data) {
        helpers.push_back(ext::make_shared<DepositRateHelper>(
            i.rate, Period(i.n, i.units), 0, WeekendsOnly(), Following, false, Actual360()));
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

// helper classes for testGlobalBootstrap() below:

// functor returning the additional error terms for the cost function
struct additionalErrors {
    explicit additionalErrors(std::vector<ext::shared_ptr<BootstrapHelper<YieldTermStructure> > > additionalHelpers)
    : additionalHelpers(std::move(additionalHelpers)) {}
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
        Date today = Settings::instance().evaluationDate();
        Calendar cal = TARGET();
        Date settl = cal.advance(today, 2 * Days);
        std::vector<Date> dates;
        dates.reserve(5);
        for (Size i = 0; i < 5; ++i)
            dates.push_back(cal.advance(settl, (1 + i) * Months));
        // Add dates before the referenceDate and not in sorted order.
        // These should be skipped by GlobalBootstrap::initialize().
        dates.insert(dates.begin(), today - 1);
        dates.push_back(today - 2);
        return dates;
    }
};


BOOST_AUTO_TEST_CASE(testGlobalBootstrap, *precondition(usingAtParCoupons())) {

    BOOST_TEST_MESSAGE("Testing global bootstrap...");

    Date today(26, Sep, 2019);
    Settings::instance().evaluationDate() = today;

    // market rates
    Real refMktRate[] = {-0.373,   -0.388,   -0.402,   -0.418,   -0.431,  -0.441,   -0.45,
                         -0.457,   -0.463,   -0.469,   -0.461,   -0.463,  -0.479,   -0.4511,
                         -0.45418, -0.439,   -0.4124,  -0.37703, -0.3335, -0.28168, -0.22725,
                         -0.1745,  -0.12425, -0.07746, 0.0385,   0.1435,  0.17525,  0.17275,
                         0.1515,   0.1225,   0.095,    0.0644};

    // expected outputs
    Date refDate[] = {
        Date(31, Mar, 2020), Date(30, Apr, 2020), Date(29, May, 2020), Date(30, Jun, 2020),
        Date(31, Jul, 2020), Date(31, Aug, 2020), Date(30, Sep, 2020), Date(30, Oct, 2020),
        Date(30, Nov, 2020), Date(31, Dec, 2020), Date(29, Jan, 2021), Date(26, Feb, 2021),
        Date(31, Mar, 2021), Date(30, Sep, 2021), Date(30, Sep, 2022), Date(29, Sep, 2023),
        Date(30, Sep, 2024), Date(30, Sep, 2025), Date(30, Sep, 2026), Date(30, Sep, 2027),
        Date(29, Sep, 2028), Date(28, Sep, 2029), Date(30, Sep, 2030), Date(30, Sep, 2031),
        Date(29, Sep, 2034), Date(30, Sep, 2039), Date(30, Sep, 2044), Date(30, Sep, 2049),
        Date(30, Sep, 2054), Date(30, Sep, 2059), Date(30, Sep, 2064), Date(30, Sep, 2069)};

    Real refZeroRate[] = {-0.00373354, -0.00381005, -0.00387689, -0.00394124, -0.00407706, -0.00413633, -0.00411935,
                          -0.00416370, -0.00420557, -0.00424431, -0.00427824, -0.00430977, -0.00434401, -0.00445243,
                          -0.00448506, -0.00433690, -0.00407401, -0.00372752, -0.00330050, -0.00279139, -0.00225477,
                          -0.00173422, -0.00123688, -0.00077237,  0.00038554,  0.00144248,  0.00175995,  0.00172873,
                           0.00150782,  0.00121145,  0.000933912, 0.000628946};

    // build ql helpers
    std::vector<ext::shared_ptr<RateHelper> > helpers;
    ext::shared_ptr<IborIndex> index = ext::make_shared<Euribor>(6 * Months);

    helpers.push_back(ext::make_shared<DepositRateHelper>(
        refMktRate[0] / 100.0, 6 * Months, 2, TARGET(), ModifiedFollowing, true, Actual360()));

    for (Size i = 0; i < 12; ++i) {
        helpers.push_back(
            ext::make_shared<FraRateHelper>(refMktRate[1 + i] / 100.0, (i + 1) * Months, index));
    }

    Size swapTenors[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15, 20, 25, 30, 35, 40, 45, 50};
    for (Size i = 0; i < 19; ++i) {
        helpers.push_back(ext::make_shared<SwapRateHelper>(
            refMktRate[13 + i] / 100.0, swapTenors[i] * Years, TARGET(), Annual, ModifiedFollowing,
            Thirty360(Thirty360::BondBasis), index));
    }

    // global bootstrap constraints
    std::vector<ext::shared_ptr<BootstrapHelper<YieldTermStructure> > > additionalHelpers;

    // set up the additional rate helpers we need in the cost function
    additionalHelpers.reserve(7);
    for (Size i = 0; i < 7; ++i) {
        additionalHelpers.push_back(
            ext::make_shared<FraRateHelper>(-0.004, (12 + i) * Months, index));
    }

    // build curve with additional dates and constraints using a global bootstrapper
    typedef PiecewiseYieldCurve<SimpleZeroYield, Linear, GlobalBootstrap> Curve;
    ext::shared_ptr<Curve> curve = ext::make_shared<Curve>(
        2, TARGET(), helpers, Actual365Fixed(), std::vector<Handle<Quote> >(), std::vector<Date>(),
        Linear(),
        Curve::bootstrap_type(additionalHelpers, additionalDates(),
                              additionalErrors(additionalHelpers), 1.0e-12));
    curve->enableExtrapolation();

    // check expected pillar dates
    for (Size i = 0; i < std::size(refDate); ++i) {
        BOOST_CHECK_EQUAL(refDate[i], helpers[i]->pillarDate());
    }

    // check expected zero rates
    for (Size i = 0; i < std::size(refZeroRate); ++i) {
        // 0.01 basis points tolerance
        QL_CHECK_SMALL(std::fabs(refZeroRate[i] - curve->zeroRate(refDate[i], Actual360(), Continuous).rate()),
                          1E-6);
    }
}

BOOST_AUTO_TEST_CASE(testGlobalBootstrapPenalty, *precondition(usingAtParCoupons())) {

    Settings::instance().evaluationDate() = Date(26, Sep, 2019);

    // market rates
    Real refMktRate[] = {-0.373,   -0.388,   -0.402,   -0.418,   -0.431,  -0.441,   -0.45,
                         -0.457,   -0.463,   -0.469,   -0.461,   -0.463,  -0.479,   -0.4511,
                         -0.45418, -0.439,   -0.4124,  -0.37703, -0.3335, -0.28168, -0.22725,
                         -0.1745,  -0.12425, -0.07746, 0.0385,   0.1435,  0.17525,  0.17275,
                         0.1515,   0.1225,   0.095,    0.0644};

    // expected outputs
    Date refDate[] = {
        Date(31, Mar, 2020), Date(30, Apr, 2020), Date(29, May, 2020), Date(30, Jun, 2020),
        Date(31, Jul, 2020), Date(31, Aug, 2020), Date(30, Sep, 2020), Date(30, Oct, 2020),
        Date(30, Nov, 2020), Date(31, Dec, 2020), Date(29, Jan, 2021), Date(26, Feb, 2021),
        Date(31, Mar, 2021), Date(30, Sep, 2021), Date(30, Sep, 2022), Date(29, Sep, 2023),
        Date(30, Sep, 2024), Date(30, Sep, 2025), Date(30, Sep, 2026), Date(30, Sep, 2027),
        Date(29, Sep, 2028), Date(28, Sep, 2029), Date(30, Sep, 2030), Date(30, Sep, 2031),
        Date(29, Sep, 2034), Date(30, Sep, 2039), Date(30, Sep, 2044), Date(30, Sep, 2049),
        Date(30, Sep, 2054), Date(30, Sep, 2059), Date(30, Sep, 2064), Date(30, Sep, 2069)};

    Real refZeroRateNP[] = {
        -0.00373354, -0.00386194, -0.00395205, -0.00403303, -0.00408033, -0.00410875, -0.00411935,
        -0.00419161, -0.00424817, -0.00429923, -0.00428029, -0.00429178, -0.00434401, -0.00445243,
        -0.00448506, -0.0043369, -0.00407401, -0.00372752, -0.0033005, -0.00279139, -0.00225477,
        -0.00173422, -0.00123688, -0.00077236, 0.00038550, 0.00144208, 0.00175947, 0.00172834,
        0.00150757, 0.00121131, 0.00093384, 0.00062891};

    Real refZeroRateGP[] = {
        -0.00377892, -0.00386127, -0.00394737, -0.00402914, -0.00409541, -0.00413252, -0.00415463,
        -0.00419484, -0.00424238, -0.00427875, -0.00429712, -0.00431898, -0.00436027, -0.00445297,
        -0.00448502, -0.00433694, -0.00407406, -0.00372755, -0.00330018, -0.00279133, -0.00225491,
        -0.00173429, -0.00123643, -0.00077298, 0.00038547, 0.00144206, 0.00175948, 0.00172834,
        0.00150756, 0.00121135, 0.00093379, 0.00062895};

    // build ql helpers
    std::vector<ext::shared_ptr<RateHelper>> helpers;
    ext::shared_ptr<IborIndex> index = ext::make_shared<Euribor>(6 * Months);

    helpers.push_back(ext::make_shared<DepositRateHelper>(
        refMktRate[0] / 100.0, 6 * Months, 2, TARGET(), ModifiedFollowing, true, Actual360()));

    for (Size i = 0; i < 12; ++i) {
        helpers.push_back(
            ext::make_shared<FraRateHelper>(refMktRate[1 + i] / 100.0, (i + 1) * Months, index));
    }

    Size swapTenors[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15, 20, 25, 30, 35, 40, 45, 50};
    for (Size i = 0; i < std::size(swapTenors); ++i) {
        helpers.push_back(ext::make_shared<SwapRateHelper>(
            refMktRate[13 + i] / 100.0, swapTenors[i] * Years, TARGET(), Annual, ModifiedFollowing,
            Thirty360(Thirty360::BondBasis), index));
    }

    // build the curve without penalties first
    typedef PiecewiseYieldCurve<ForwardRate, BackwardFlat, GlobalBootstrap> Curve;
    auto curve = ext::make_shared<Curve>(
        2, TARGET(), helpers, Actual365Fixed(), std::vector<Handle<Quote>>(), std::vector<Date>(),
        BackwardFlat(),
        Curve::bootstrap_type({}, nullptr, std::function<Array()>(), 1.0e-12));

    // check expected pillar dates
    for (Size i = 0; i < std::size(refDate); ++i) {
        BOOST_CHECK_EQUAL(refDate[i], helpers[i]->pillarDate());
    }

    // check expected zero rates
    for (Size i = 0; i < std::size(refZeroRateNP); ++i) {
        // 0.01 basis points tolerance
        QL_CHECK_SMALL(
            refZeroRateNP[i] - curve->zeroRate(refDate[i], Actual360(), Continuous).rate(),
            1E-6);
    }

    // build the curve with gradient penalties
    auto gradientPenalty = [](const std::vector<Time>& times, const std::vector<Real>& data) {
        Array errors(times.size() - 1);
        for (Size i = 0; i < times.size() - 1; ++i) {
            errors[i] = 0.01 * (data[i+1] - data[i]) / (times[i+1] - times[i]);
        }
        return errors;
    };

    curve = ext::make_shared<Curve>(
        2, TARGET(), helpers, Actual365Fixed(), std::vector<Handle<Quote>>(), std::vector<Date>(),
        BackwardFlat(),
        Curve::bootstrap_type({}, nullptr, gradientPenalty, 1.0e-12));

    // check expected zero rates
    for (Size i = 0; i < std::size(refZeroRateGP); ++i) {
        // 0.01 basis points tolerance
        QL_CHECK_SMALL(
            refZeroRateGP[i] - curve->zeroRate(refDate[i], Actual360(), Continuous).rate(),
            1E-6);
    }
}

BOOST_AUTO_TEST_CASE(testGlobalBootstrapVariables) {

    // Use fixed evaluationDate to make the test stable. It works for any date,
    // but the tolerance varies and also depends on usingAtParCoupons().
    CommonVars vars(Date(25, Sep, 2019));

    // First, build the curve without futures.
    typedef PiecewiseYieldCurve<Discount, LogLinear, GlobalBootstrap> Curve;
    auto curve = ext::make_shared<Curve>(
        vars.settlement, vars.instruments, Actual365Fixed(), LogLinear());

    // Now build the curve with futures adjusted to the same level as swaps.
    // Remove the first swap from helpers so it does not create a pillar.
    auto helpers = vars.instruments;
    auto firstSwap = helpers[vars.deposits];
    helpers.erase(helpers.begin() + vars.deposits);

    auto euribor3m = ext::make_shared<Euribor3M>();
    Date immDate = vars.today;
    // We will optimize vol as an additional variable during bootstrapping.
    auto vol = ext::make_shared<SimpleQuote>();
    auto mr = makeQuoteHandle(0.03);
    for (Size i = 0; i < vars.immFuts; i++) {
        Handle<Quote> r(vars.immFutPrices[i]);
        immDate = IMM::nextDate(immDate);
        // if the fixing is before today, jump forward by one future maturity
        if (euribor3m->fixingDate(immDate) < vars.today)
            immDate = IMM::nextDate(immDate);
        auto convAdj = ext::make_shared<FuturesConvAdjustmentQuote>(
            euribor3m, immDate, r, Handle<Quote>(vol), mr);
        // Pass registerAsObserver = false so that FuturesRateHelpers do not
        // depend on convAdj quotes. Otherwise, the curve will be invalidated
        // every time we change the vol quote during optimization, which will
        // break bootstrapping.
        bool registerAsObserver = false;
        helpers.push_back(ext::make_shared<FuturesRateHelper>(
            r, immDate, euribor3m, Handle<Quote>(convAdj, registerAsObserver),
            Futures::IMM));
    }

    auto penalties = [&]() { return Array{1e4 * firstSwap->quoteError()}; };

    auto curveFutures = ext::make_shared<Curve>(
        vars.settlement, helpers, Actual365Fixed(), LogLinear(),
        Curve::bootstrap_type(
            {firstSwap}, nullptr, penalties, 1e-12, nullptr, nullptr,
            ext::make_shared<SimpleQuoteVariables>(
                std::vector<ext::shared_ptr<SimpleQuote>>{vol},
                std::vector<Real>{1.0}, std::vector<Real>{0.0})));

    // Check that the pillars are not the same.
    BOOST_CHECK(curve->dates() != curveFutures->dates());

    // Check that all deposit and swap rates are the same in both curves.
    for (const auto& helper : vars.instruments) {
        QL_CHECK_CLOSE(curve->discount(helper->pillarDate()),
                          curveFutures->discount(helper->pillarDate()),
                          1e-6);
    }
}

BOOST_AUTO_TEST_CASE(testMultiCurveTwoPiecewiseYieldCurves) {

    BOOST_TEST_MESSAGE("Testing multicurve bootstrap with two piecewise yield curves...");

    CommonVars vars(Date(23, Oct, 2025));

    constexpr auto accuracy = 1E-10;

    Handle<YieldTermStructure> discountCurve(
        ext::make_shared<FlatForward>(vars.settlement, 0.02, Actual360()));

    RelinkableHandle<YieldTermStructure> intcurve3m, intcurve6m;

    auto euribor3m = ext::make_shared<Euribor3M>(intcurve3m);
    auto euribor6m = ext::make_shared<Euribor6M>(intcurve6m);

    std::vector<ext::shared_ptr<RateHelper>> helpers3m, helpers6m;

    Handle<Quote> q(ext::make_shared<SimpleQuote>(0.03));
    Handle<Quote> b(ext::make_shared<SimpleQuote>(0.0020));

    for (Size i = 1; i <= 9; ++i) {
        helpers3m.push_back(ext::make_shared<FraRateHelper>(
            q, i, i + 3, euribor3m->fixingDays(), euribor3m->fixingCalendar(),
            euribor3m->businessDayConvention(), euribor3m->endOfMonth(), euribor3m->dayCounter(),
            Pillar::LastRelevantDate));
    }

    for (Size i = 2; i <= 10; ++i) {
        helpers3m.push_back(ext::make_shared<IborIborBasisSwapRateHelper>(
            b, i * Years, euribor3m->fixingDays(), euribor3m->fixingCalendar(),
            euribor3m->businessDayConvention(), euribor3m->endOfMonth(), euribor3m, euribor6m,
            discountCurve, true));
    }

    for (Size i = 1; i <= 3; ++i) {
        helpers6m.push_back(ext::make_shared<IborIborBasisSwapRateHelper>(
            b, (i * 6) * Months, euribor3m->fixingDays(), euribor3m->fixingCalendar(),
            euribor3m->businessDayConvention(), euribor3m->endOfMonth(), euribor3m, euribor6m,
            discountCurve, false));
    }

    for (Size i = 2; i <= 10; ++i) {
        helpers6m.push_back(ext::make_shared<SwapRateHelper>(
            q, i * Years, euribor6m->fixingCalendar(), Annual, Following,
            Thirty360(Thirty360::BondBasis), euribor6m, Handle<Quote>(), 0 * Days, discountCurve));
    }

    using CurveType = PiecewiseYieldCurve<Discount, LogLinear, GlobalBootstrap>;

    ext::shared_ptr<YieldTermStructure> ptr3m = ext::make_shared<CurveType>(
        vars.today, helpers3m, Actual360(), LogLinear(), GlobalBootstrap<CurveType>(accuracy));
    ext::shared_ptr<YieldTermStructure> ptr6m = ext::make_shared<CurveType>(
        vars.today, helpers6m, Actual360(), LogLinear(), GlobalBootstrap<CurveType>(accuracy));

    auto multiCurve = ext::make_shared<MultiCurve>(accuracy);

    auto curve3m = multiCurve->addBootstrappedCurve(intcurve3m, std::move(ptr3m));
    auto curve6m = multiCurve->addBootstrappedCurve(intcurve6m, std::move(ptr6m));

    // check instrument npvs

    constexpr auto tolerance = 1E-10;

    for (Size i = 1; i <= 9; ++i) {
        Date start = euribor3m->fixingCalendar().advance(
            euribor3m->fixingCalendar().advance(vars.today, euribor3m->fixingDays(), Days), i,
            Months, euribor3m->businessDayConvention(), euribor3m->endOfMonth());
        ForwardRateAgreement fra(euribor3m, start, Position::Long, q->value(), 1.0, curve3m);
        BOOST_CHECK_CLOSE(fra.forwardRate().rate(), q->value(), tolerance);
    }

    for (Size i = 2; i <= 10; ++i) {
        Date start = euribor3m->fixingCalendar().advance(vars.today, euribor3m->fixingDays(), Days);
        Date maturity = euribor3m->fixingCalendar().advance(start, i * Years,
                                                            euribor3m->businessDayConvention());
        Schedule baseSchedule = MakeSchedule()
                                   .from(start)
                                   .to(maturity)
                                   .withTenor(3 * Months)
                                   .withCalendar(euribor3m->fixingCalendar())
                                   .withConvention(euribor3m->businessDayConvention())
                                   .endOfMonth(euribor3m->endOfMonth())
                                   .forwards();
        Schedule otherSchedule = MakeSchedule()
                                     .from(start)
                                     .to(maturity)
                                     .withTenor(6 * Months)
                                     .withCalendar(euribor6m->fixingCalendar())
                                     .withConvention(euribor6m->businessDayConvention())
                                     .endOfMonth(euribor6m->endOfMonth())
                                     .forwards();
        Leg baseLeg = IborLeg(baseSchedule, euribor3m).withSpreads(b->value()).withNotionals(1.0);
        Leg otherLeg = IborLeg(otherSchedule, euribor6m).withNotionals(1.0);
        Swap swap(baseLeg, otherLeg);
        swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(discountCurve));
        BOOST_CHECK_SMALL(swap.NPV(), tolerance);
    }

    for (Size i = 1; i <= 3; ++i) {
        Date start = euribor3m->fixingCalendar().advance(vars.today, euribor3m->fixingDays(), Days);
        Date maturity = euribor3m->fixingCalendar().advance(start, (i * 6) * Months,
                                                            euribor3m->businessDayConvention());
        Schedule baseSchedule = MakeSchedule()
                                   .from(start)
                                   .to(maturity)
                                   .withTenor(3 * Months)
                                   .withCalendar(euribor3m->fixingCalendar())
                                   .withConvention(euribor3m->businessDayConvention())
                                   .endOfMonth(euribor3m->endOfMonth())
                                   .forwards();
        Schedule otherSchedule = MakeSchedule()
                                     .from(start)
                                     .to(maturity)
                                     .withTenor(6 * Months)
                                     .withCalendar(euribor6m->fixingCalendar())
                                     .withConvention(euribor6m->businessDayConvention())
                                     .endOfMonth(euribor6m->endOfMonth())
                                     .forwards();
        Leg baseLeg = IborLeg(baseSchedule, euribor3m).withSpreads(b->value()).withNotionals(1.0);
        Leg otherLeg = IborLeg(otherSchedule, euribor6m).withNotionals(1.0);
        Swap swap(baseLeg, otherLeg);
        swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(discountCurve));
        BOOST_CHECK_SMALL(swap.NPV(), tolerance);
    }

    for (Size i = 2; i <= 10; ++i) {
        VanillaSwap swap = MakeVanillaSwap(i * Years, euribor6m, q->value())
                               .withSettlementDays(euribor6m->fixingDays())
                               .withFixedLegDayCount(Thirty360(Thirty360::BondBasis))
                               .withFixedLegTenor(1 * Years)
                               .withFixedLegConvention(Following)
                               .withFixedLegTerminationDateConvention(Following);
        swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(discountCurve));
        BOOST_CHECK_SMALL(swap.NPV(), tolerance);
    }

}

BOOST_AUTO_TEST_CASE(testMultiCurvePiecewiseYieldCurveAndSpreadedCurve) {

    BOOST_TEST_MESSAGE("Testing multicurve bootstrap with piecewise yield curve and spreaded curve...");

    CommonVars vars(Date(23, Oct, 2025));

    constexpr Real accuracy = 1E-10;

    RelinkableHandle<YieldTermStructure> intcurveois;
    RelinkableHandle<YieldTermStructure> intcurve3m;

    auto euribor3m = ext::make_shared<Euribor3M>(intcurve3m);

    std::vector<ext::shared_ptr<RateHelper>> helpers3m;

    Handle<Quote> q(ext::make_shared<SimpleQuote>(0.03));
    Handle<Quote> b(ext::make_shared<SimpleQuote>(-0.01));

    for (Size i = 1; i <= 10; ++i) {
        helpers3m.push_back(ext::make_shared<SwapRateHelper>(
            q, i * Years, euribor3m->fixingCalendar(), Annual, Following,
            Thirty360(Thirty360::BondBasis), euribor3m, Handle<Quote>(), 0 * Days, intcurveois));
    }

    using CurveType = PiecewiseYieldCurve<Discount, LogLinear, GlobalBootstrap>;

    auto multiCurve = ext::make_shared<MultiCurve>(accuracy);

    ext::shared_ptr<YieldTermStructure> ptr3m = ext::make_shared<CurveType>(
        vars.today, helpers3m, Actual360(), LogLinear(), GlobalBootstrap<CurveType>(accuracy));
    auto curve3m = multiCurve->addBootstrappedCurve(intcurve3m, std::move(ptr3m));

    ext::shared_ptr<YieldTermStructure> ptrois =
        ext::make_shared<ZeroSpreadedTermStructure>(intcurve3m, b);
    auto curveois = multiCurve->addNonBootstrappedCurve(intcurveois, std::move(ptrois));

    // check spread ois 3m

    constexpr Real tolerance = 1E-10;

    BOOST_CHECK_CLOSE(curveois->zeroRate(1.0, Continuous) - curve3m->zeroRate(1.0, Continuous),
                      b->value(), tolerance);

    // check instrument npvs

    for (Size i = 1; i <= 10; ++i) {
        VanillaSwap swap = MakeVanillaSwap(i * Years, euribor3m, q->value())
                               .withSettlementDays(euribor3m->fixingDays())
                               .withFixedLegDayCount(Thirty360(Thirty360::BondBasis))
                               .withFixedLegTenor(1 * Years)
                               .withFixedLegConvention(Following)
                               .withFixedLegTerminationDateConvention(Following);
        swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(curveois));
        BOOST_CHECK_SMALL(swap.NPV(), tolerance);
    }

}

template <template<class C> class Bootstrap>
void testPiecewiseSpreadYieldCurveImpl() {
    // Use fixed evaluationDate to make the test stable. When usingAtParCoupons() == false
    // the dates don't always align between the 3M and 6M indexes, but most of the test is
    // still valid.
    CommonVars vars(Date(23, Sep, 2019));
    Actual365Fixed dc;

    // First, build the base curve. We can use any bootstrapping and interpolation.
    typedef PiecewiseYieldCurve<Discount, LogLinear> BaseCurve;
    Handle<YieldTermStructure> baseCurve(ext::make_shared<BaseCurve>(
        vars.settlement, vars.instruments, dc, LogLinear()));
    baseCurve->enableExtrapolation();

    // Now build the curve with fewer benchmarks as a spread to the base.
    Datum swapData[] = {
        {  1, Years, 4.44 },
        {  3, Years, 4.55 },
        {  6, Years, 4.81 },
        {  9, Years, 5.01 },
        { 15, Years, 5.25 },
        { 30, Years, 5.36 }
    };

    std::vector<ext::shared_ptr<RateHelper>> helpers;
    auto euribor3m = ext::make_shared<Euribor3M>();
    for (const auto& datum : swapData) {
        helpers.push_back(ext::make_shared<SwapRateHelper>(
            datum.rate / 100.0, datum.n * datum.units, vars.calendar,
            vars.fixedLegFrequency, vars.fixedLegConvention, vars.fixedLegDayCounter,
            euribor3m));
    }

    // We rely on LogLinear interpolation to check the curve's shape.
    typedef PiecewiseSpreadYieldCurve<Discount, LogLinear, Bootstrap> Curve;
    auto curve = ext::make_shared<Curve>(baseCurve, helpers, LogLinear());
    curve->enableExtrapolation();
    Handle<YieldTermStructure> curveHandle(curve);

    // Check that we reprice the swaps.
    const Real tolerance = 1.0e-9;
    euribor3m = ext::make_shared<Euribor3M>(curveHandle);
    for (const auto& datum : swapData) {
        VanillaSwap swap = MakeVanillaSwap(datum.n * datum.units, euribor3m, 0.0)
            .withEffectiveDate(vars.settlement)
            .withFixedLegDayCount(vars.fixedLegDayCounter)
            .withFixedLegTenor(Period(vars.fixedLegFrequency))
            .withFixedLegConvention(vars.fixedLegConvention)
            .withFixedLegTerminationDateConvention(vars.fixedLegConvention);

        Rate expectedRate = datum.rate / 100.0,
            estimatedRate = swap.fairRate();
        Spread error = std::fabs(expectedRate - estimatedRate);
        if (error > tolerance) {
            BOOST_ERROR(datum.n << " year(s) swap:\n"
                        << std::setprecision(8)
                        << "\n estimated rate: " << io::rate(estimatedRate)
                        << "\n expected rate:  " << io::rate(expectedRate)
                        << "\n error:          " << io::rate(error)
                        << "\n tolerance:      " << io::rate(tolerance));
        }
    }

    // Check that the curve has shape between pillars.
    auto prev = vars.settlement;
    for (const auto& helper : helpers) {
        Date pillar = helper->pillarDate();
        Rate rate1 = curve->forwardRate(prev, pillar, dc, Continuous).rate();
        Rate rate2 = curve->forwardRate(prev, prev + (pillar - prev) / 2, dc, Continuous).rate();
        BOOST_CHECK_GT(std::fabs(rate1 - rate2), 1e-4);
        prev = pillar;
    }

    // Check that extrapolation preserves constant spread.
    Date maxDate = curve->maxDate();
    BOOST_CHECK_EQUAL(maxDate, baseCurve->maxDate());
    Rate rate1 = curve->forwardRate(maxDate - 1*Years, maxDate, dc, Continuous).rate();
    Rate rate2 = curve->forwardRate(maxDate, maxDate + 1*Years, dc, Continuous).rate();
    Rate baseRate1 = baseCurve->forwardRate(maxDate - 1*Years, maxDate, dc, Continuous).rate();
    Rate baseRate2 = baseCurve->forwardRate(maxDate, maxDate + 1*Years, dc, Continuous).rate();
    QL_CHECK_CLOSE(rate1 - baseRate1, rate2 - baseRate2, 1e-9);

    // Check accessors.
    BOOST_CHECK_EQUAL(curve->dates().size(), helpers.size() + 1);
    BOOST_CHECK_EQUAL(curve->times().size(), helpers.size() + 1);
    BOOST_CHECK_EQUAL(curve->data().size(), helpers.size() + 1);
    const auto nodes = curve->nodes();
    BOOST_CHECK_EQUAL(nodes.size(), helpers.size() + 1);

    BOOST_CHECK_EQUAL(curve->dates()[0], vars.settlement);
    BOOST_CHECK_EQUAL(curve->times()[0], 0.0);
    BOOST_CHECK_EQUAL(curve->data()[0], 1.0);
    BOOST_CHECK(nodes[0] == std::make_pair(vars.settlement, Real(1)));
    for (Size i = 0; i < helpers.size(); ++i) {
        BOOST_CHECK_EQUAL(curve->dates()[i+1], helpers[i]->pillarDate());
        BOOST_CHECK_EQUAL(curve->times()[i+1], curve->timeFromReference(helpers[i]->pillarDate()));
        BOOST_CHECK(nodes[i+1] == std::make_pair(curve->dates()[i+1], curve->data()[i+1]));
    }

    // Check that we can rebuild the curve from raw data.
    auto rawCurve = ext::make_shared<SpreadDiscountCurve>(
        curve->baseCurve(), curve->dates(), curve->data());
    rawCurve->enableExtrapolation();

    const Integer maxSwapYears = (std::end(swapData)-1)->n;
    for (Integer i = 0; i < maxSwapYears + 3; ++i) {
        Date d = vars.settlement + i*Years;
        QL_CHECK_CLOSE(curve->discount(d), rawCurve->discount(d), 1e-9);
    }
}

BOOST_AUTO_TEST_CASE(testPiecewiseSpreadYieldCurve) {

    BOOST_TEST_MESSAGE("Testing PiecewiseSpreadYieldCurve...");

    testPiecewiseSpreadYieldCurveImpl<IterativeBootstrap>();
    testPiecewiseSpreadYieldCurveImpl<GlobalBootstrap>();
}

/* This test attempts to build an ARS collateralised in USD curve as of 25 Sep 2019. Using the default 
   IterativeBootstrap with no retries, the yield curve building fails. Allowing retries, it expands the min and max 
   bounds and passes.
*/
BOOST_AUTO_TEST_CASE(testIterativeBootstrapRetries) {

    BOOST_TEST_MESSAGE("Testing iterative bootstrap with retries...");

    Date asof(25, Sep, 2019);
    Settings::instance().evaluationDate() = asof;
    Actual365Fixed tsDayCounter;

    // USD discount curve built out of FedFunds OIS swaps.
    vector<Date> usdCurveDates = {
        Date(25, Sep, 2019),
        Date(26, Sep, 2019),
        Date(8, Oct, 2019),
        Date(16, Oct, 2019),
        Date(22, Oct, 2019),
        Date(30, Oct, 2019),
        Date(2, Dec, 2019),
        Date(31, Dec, 2019),
        Date(29, Jan, 2020),
        Date(2, Mar, 2020),
        Date(31, Mar, 2020),
        Date(29, Apr, 2020),
        Date(29, May, 2020),
        Date(1, Jul, 2020),
        Date(29, Jul, 2020),
        Date(31, Aug, 2020),
        Date(30, Sep, 2020)
    };

    vector<DiscountFactor> usdCurveDfs = {
        1.000000000,
        0.999940837,
        0.999309357,
        0.998894646,
        0.998574816,
        0.998162528,
        0.996552511,
        0.995197584,
        0.993915264,
        0.992530008,
        0.991329696,
        0.990179606,
        0.989005698,
        0.987751691,
        0.986703371,
        0.985495036,
        0.984413446
    };

    Handle<YieldTermStructure> usdYts(ext::make_shared<InterpolatedDiscountCurve<LogLinear> >(
        usdCurveDates, usdCurveDfs, tsDayCounter));

    // USD/ARS forward points
    Handle<Quote> arsSpot(ext::make_shared<SimpleQuote>(56.881));
    map<Period, Real> arsFwdPoints = {
        {1 * Months, 8.5157},
        {2 * Months, 12.7180},
        {3 * Months, 17.8310},
        {6 * Months, 30.3680},
        {9 * Months, 45.5520},
        {1 * Years, 60.7370}
    };

    // Create the FX swap rate helpers for the ARS in USD curve.
    vector<ext::shared_ptr<RateHelper>> instruments, datedInstruments;
    Calendar calendar = UnitedStates(UnitedStates::GovernmentBond);
    Date startDate = calendar.advance(calendar.adjust(asof), 2*Days);
    for (auto & arsFwdPoint : arsFwdPoints) {
        Handle<Quote> arsFwd(ext::make_shared<SimpleQuote>(arsFwdPoint.second));
        instruments.push_back(ext::make_shared<FxSwapRateHelper>(arsFwd, arsSpot, arsFwdPoint.first, 2,
            calendar, Following, false, true, usdYts));
        datedInstruments.push_back(ext::make_shared<FxSwapRateHelper>(arsFwd, arsSpot, startDate,
            calendar.advance(startDate, arsFwdPoint.first), true, usdYts));
    }

    // Create the ARS in USD curve with the default IterativeBootstrap.
    typedef PiecewiseYieldCurve<Discount, LogLinear, IterativeBootstrap> LLDFCurve;
    auto arsYts = ext::make_shared<LLDFCurve>(asof, instruments, tsDayCounter);

    // USD/ARS spot date. The date on which we check the ARS discount curve.
    Date spotDate(27, Sep, 2019);

    // Check that the ARS in USD curve throws by requesting a discount factor.
    BOOST_CHECK_EXCEPTION(arsYts->discount(spotDate), Error,
        ExpectedErrorMessage("1st iteration: failed at 1st alive instrument"));

    // Create the ARS in USD curve with an IterativeBootstrap allowing for 4 retries.
    IterativeBootstrap<LLDFCurve> ib(Null<Real>(), Null<Real>(), Null<Real>(), 5);
    arsYts = ext::make_shared<LLDFCurve>(asof, instruments, tsDayCounter, ib);
    
    // Check that the ARS in USD curve builds and populate the spot ARS discount factor.
    DiscountFactor spotDfArs = 1.0;
    BOOST_REQUIRE_NO_THROW(spotDfArs = arsYts->discount(spotDate));

    // Additional dates and discount factors used in the final check i.e. that calculated 1Y FX forward equals input.
    Date oneYearFwdDate(28, Sep, 2020);
    DiscountFactor spotDfUsd = usdYts->discount(spotDate);
    DiscountFactor oneYearDfUsd = usdYts->discount(oneYearFwdDate);

    // Given that the ARS in USD curve builds, check that the 1Y USD/ARS forward rate is as expected.
    DiscountFactor oneYearDfArs = arsYts->discount(oneYearFwdDate);
    Real calcFwd = (spotDfArs * arsSpot->value() / oneYearDfArs) / (spotDfUsd / oneYearDfUsd);
    Real expFwd = arsSpot->value() + arsFwdPoints.at(1 * Years);
    QL_CHECK_SMALL(calcFwd - expFwd, 1e-10);

    // Check that datedInstruments give the same result.
    auto datedArsYts = ext::make_shared<LLDFCurve>(asof, datedInstruments, tsDayCounter, ib);
    BOOST_CHECK(arsYts->dates() == datedArsYts->dates());
    for (const auto date : arsYts->dates()) {
        QL_CHECK_CLOSE(arsYts->discount(date), datedArsYts->discount(date), 1e-6);
    }
}

BOOST_AUTO_TEST_CASE(testCustomFuturesHelpers) {

    BOOST_TEST_MESSAGE("Testing futures rate helpers with custom dates...");

    CommonVars vars;

    std::vector<ext::shared_ptr<RateHelper>> helpers;

    Date startDate1 = vars.today + 60;
    Real price1 = 97.0;
    Natural length1 = 2;
    auto convention = ModifiedFollowing;
    bool endOfMonth = true;
    auto dayCounter = Actual360();

    helpers.push_back(ext::make_shared<FuturesRateHelper>(price1, startDate1, length1,
                                                          TARGET(), convention, endOfMonth,
                                                          dayCounter, 0.0, Futures::Custom));

    Date startDate2 = vars.today + 120;
    Date endDate2 = startDate2 + 45;
    Real price2 = 96.5;

    helpers.push_back(ext::make_shared<FuturesRateHelper>(price2, startDate2, endDate2,
                                                          dayCounter, 0.0, Futures::Custom));

    Date startDate3 = vars.today + 180;
    auto index = ext::make_shared<Euribor3M>();
    Real price3 = 96.0;

    helpers.push_back(ext::make_shared<FuturesRateHelper>(price3, startDate3, index,
                                                          0.0, Futures::Custom));

    ext::shared_ptr<YieldTermStructure> curve =
        ext::make_shared<PiecewiseYieldCurve<ForwardRate, BackwardFlat> >(
                                            vars.today, helpers, Actual360());

    Date endDate1 = TARGET().advance(startDate1, length1, Months, convention, endOfMonth);

    Rate calculated = curve->forwardRate(startDate1, endDate1, dayCounter, Simple).rate();
    Rate expected = (100-price1)/100;
    Real error = std::fabs(expected - calculated);
    Real tolerance = 1e-8;
    if (error > tolerance) {
        BOOST_ERROR(" first helper:\n"
                    << std::setprecision(8)
                    << "\n estimated rate: " << io::rate(calculated)
                    << "\n expected rate:  " << io::rate(expected));
    }

    calculated = curve->forwardRate(startDate2, endDate2, dayCounter, Simple).rate();
    expected = (100-price2)/100;
    error = std::fabs(expected - calculated);
    if (error > tolerance) {
        BOOST_ERROR(" second helper:\n"
                    << std::setprecision(8)
                    << "\n estimated rate: " << io::rate(calculated)
                    << "\n expected rate:  " << io::rate(expected));
    }

    Date endDate3 = index->fixingCalendar().advance(startDate3, index->tenor(), index->businessDayConvention());

    calculated = curve->forwardRate(startDate3, endDate3, dayCounter, Simple).rate();
    expected = (100-price3)/100;
    error = std::fabs(expected - calculated);
    if (error > tolerance) {
        BOOST_ERROR(" third helper:\n"
                    << std::setprecision(8)
                    << "\n estimated rate: " << io::rate(calculated)
                    << "\n expected rate:  " << io::rate(expected));
    }
}


BOOST_AUTO_TEST_CASE(testSwapHelpersWithOnceFrequency) {
    BOOST_TEST_MESSAGE("Testing single-coupon swap rate helpers...");

    auto index = ext::make_shared<IborIndex>(
        "TestIndex", 4*Weeks, 1, MXNCurrency(),
        Mexico(), Following, false, Actual360());

    Handle<Quote> r(ext::make_shared<SimpleQuote>(0.02));

    BOOST_CHECK_NO_THROW(SwapRateHelper(r, 4*Weeks, Mexico(), Once, Following, Actual360(), index));

    BOOST_CHECK_NO_THROW(OISRateHelper(2, 4*Weeks, r, ext::make_shared<Estr>(), {}, false, 0, Following, Once));
}


BOOST_AUTO_TEST_CASE(testDepositForDates) {
    BOOST_TEST_MESSAGE("Testing DepositRateHelper with custom fixingDate...");

    CommonVars vars;

    std::vector<ext::shared_ptr<RateHelper>> helpers;
    const Date fixingDate = TARGET().adjust(vars.today);
    for (Size i = 0; i < vars.deposits; i++) {
        Handle<Quote> r(vars.rates[i]);
        helpers.push_back(ext::make_shared<DepositRateHelper>(
            r, fixingDate, ext::make_shared<Euribor>(depositData[i].n*depositData[i].units)));
    }

    auto curve = ext::make_shared<PiecewiseYieldCurve<ZeroYield, Linear>>(
                                       vars.settlement, helpers, Actual365Fixed());
    Handle<YieldTermStructure> h(curve);

    const Real tolerance = 1.0e-9;
    for (Size i = 0; i < vars.deposits; i++) {
        Euribor index(depositData[i].n*depositData[i].units, h);
        Rate expectedRate  = depositData[i].rate/100,
            estimatedRate = index.fixing(vars.today);
        if (std::fabs(expectedRate-estimatedRate) > tolerance) {
            BOOST_ERROR(depositData[i].n << " "
                        << (depositData[i].units == Weeks ? "week(s)" : "month(s)")
                        << " deposit:"
                        << std::setprecision(8)
                        << "\n    estimated rate: " << io::rate(estimatedRate)
                        << "\n    expected rate:  " << io::rate(expectedRate));
        }
    }
}

BOOST_AUTO_TEST_CASE(testFraForDates) {
    BOOST_TEST_MESSAGE("Testing FraRateHelper with custom dates...");

    CommonVars vars;

    std::vector<ext::shared_ptr<RateHelper>> helpers;
    auto euribor6m = ext::make_shared<Euribor6M>();
    for (Size i = 0; i < vars.fras; i++) {
        Handle<Quote> r(vars.fraRates[i]);
        Date startDate =
            vars.calendar.advance(vars.settlement,
                                  fraData[i].n,
                                  fraData[i].units,
                                  euribor6m->businessDayConvention(),
                                  euribor6m->endOfMonth());
        Date endDate =
            vars.calendar.advance(vars.settlement,
                                  fraData[i].n + 3,
                                  fraData[i].units,
                                  euribor6m->businessDayConvention(),
                                  euribor6m->endOfMonth());
        helpers.push_back(ext::make_shared<FraRateHelper>(
            r, startDate, endDate, euribor6m, Pillar::LastRelevantDate, Date(), false));
    }

    auto curve = ext::make_shared<PiecewiseYieldCurve<ZeroYield, Linear>>(
                                       vars.settlement, helpers, Actual365Fixed());
    Handle<YieldTermStructure> h(curve);
    euribor6m = ext::make_shared<Euribor6M>(h);

    const Real tolerance = 1.0e-9;
    for (Size i = 0; i < vars.fras; i++) {
        Date start =
            vars.calendar.advance(vars.settlement,
                                  fraData[i].n,
                                  fraData[i].units,
                                  euribor6m->businessDayConvention(),
                                  euribor6m->endOfMonth());
        Date end =
            vars.calendar.advance(vars.settlement,
                                  fraData[i].n + 3,
                                  fraData[i].units,
                                  euribor6m->businessDayConvention(),
                                  euribor6m->endOfMonth());
        BOOST_REQUIRE(fraData[i].units == Months);

        ForwardRateAgreement fra(euribor6m, start, end, Position::Long,
                                 fraData[i].rate/100, 100.0);
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

BOOST_AUTO_TEST_CASE(testDatedSwapHelpers) {
    BOOST_TEST_MESSAGE("Testing dated swap rate helpers...");

    Date today { 28, October, 2024 };
    Settings::instance().evaluationDate() = today;

    std::tuple<Date, Date, Rate> swapData[] = {
        {{1, November, 2024}, {1, November, 2025}, 4.54 },
        {{15, October, 2024}, {15, October, 2026}, 4.63 },
        {{28, October, 2024}, {1, November, 2029}, 4.99 },
        {{4, November, 2024}, {4, November, 2034}, 5.47 },
        {{11, October, 2024}, {11, October, 2044}, 5.89 }
    };

    auto euribor6m = ext::make_shared<Euribor6M>();
    euribor6m->addFixing({9, October, 2024}, 0.0447);
    euribor6m->addFixing({11, October, 2024}, 0.045);
    euribor6m->addFixing({24, October, 2024}, 0.0442);

    auto calendar = TARGET();
    auto fixedLegFrequency = Annual;
    auto fixedLegConvention = Unadjusted;
    auto fixedLegDayCounter = Thirty360(Thirty360::BondBasis);

    std::vector<ext::shared_ptr<RateHelper>> helpers;
    for (auto [start, end, q] : swapData) {
        Handle<Quote> r(ext::make_shared<SimpleQuote>(q/100));
        helpers.push_back(ext::make_shared<SwapRateHelper>(
                                   r, start, end,
                                   calendar,
                                   fixedLegFrequency, fixedLegConvention,
                                   fixedLegDayCounter, euribor6m));
    }

    auto curve = ext::make_shared<PiecewiseYieldCurve<ZeroYield, Linear>>(
                                       today, helpers, Actual365Fixed());
    Handle<YieldTermStructure> h(curve);
    euribor6m = ext::make_shared<Euribor6M>(h);

    for (auto [start, end, q] : swapData) {
        VanillaSwap swap = MakeVanillaSwap(Period(), euribor6m, 0.0)
            .withEffectiveDate(start)
            .withTerminationDate(end)
            .withFixedLegDayCount(fixedLegDayCounter)
            .withFixedLegTenor(Period(fixedLegFrequency))
            .withFixedLegConvention(fixedLegConvention)
            .withFixedLegTerminationDateConvention(fixedLegConvention);

        Rate expectedRate = q/100,
            estimatedRate = swap.fairRate();
        Spread error = std::fabs(expectedRate-estimatedRate);
        Real tolerance = 1e-9;
        if (error > tolerance) {
            BOOST_ERROR("swap from " << start << " to " << end << ":\n"
                        << std::setprecision(8)
                        << "\n    estimated rate: " << io::rate(estimatedRate)
                        << "\n    expected rate:  " << io::rate(expectedRate)
                        << "\n    error:          " << io::rate(error)
                        << "\n    tolerance:      " << io::rate(tolerance));
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
