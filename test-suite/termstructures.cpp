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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/termstructures/yield/compositezeroyieldstructure.hpp>
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

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(TermStructureTests)

struct Datum {
    Integer n;
    TimeUnit units;
    Rate rate;
};

struct CommonVars {
    // common data
    Calendar calendar;
    Natural settlementDays;
    ext::shared_ptr<YieldTermStructure> termStructure;
    ext::shared_ptr<YieldTermStructure> dummyTermStructure;

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
        Size deposits = std::size(depositData),
            swaps = std::size(swapData);

        std::vector<ext::shared_ptr<RateHelper> > instruments(deposits+swaps);
        for (Size i=0; i<deposits; i++) {
            instruments[i] = ext::shared_ptr<RateHelper>(new
                    DepositRateHelper(depositData[i].rate/100,
                                      depositData[i].n*depositData[i].units,
                                      settlementDays, calendar,
                                      ModifiedFollowing, true,
                                      Actual360()));
        }
        ext::shared_ptr<IborIndex> index(new IborIndex("dummy",
                                                       6*Months,
                                                       settlementDays,
                                                       Currency(),
                                                       calendar,
                                                       ModifiedFollowing,
                                                       false,
                                                       Actual360()));
        for (Size i=0; i<swaps; ++i) {
            instruments[i+deposits] = ext::shared_ptr<RateHelper>(new
                    SwapRateHelper(swapData[i].rate/100,
                                   swapData[i].n*swapData[i].units,
                                   calendar,
                                   Annual, Unadjusted, Thirty360(Thirty360::BondBasis),
                                   index));
        }
        termStructure = ext::shared_ptr<YieldTermStructure>(new
                PiecewiseYieldCurve<Discount,LogLinear>(settlement,
                                                        instruments, Actual360()));
        dummyTermStructure = ext::shared_ptr<YieldTermStructure>(new
                PiecewiseYieldCurve<Discount,LogLinear>(settlement,
                                                        instruments, Actual360()));
    }
};

Real sub(Real x, Real y) { return x - y; }


BOOST_AUTO_TEST_CASE(testReferenceChange) {

    BOOST_TEST_MESSAGE("Testing term structure against evaluation date change...");

    CommonVars vars;

    ext::shared_ptr<SimpleQuote> flatRate (new SimpleQuote);
    Handle<Quote> flatRateHandle(flatRate);
    vars.termStructure = ext::shared_ptr<YieldTermStructure>(
                          new FlatForward(vars.settlementDays, NullCalendar(),
                                          flatRateHandle, Actual360()));
    Date today = Settings::instance().evaluationDate();
    flatRate->setValue(.03);
    Integer days[] = { 10, 30, 60, 120, 360, 720 };
    Size i;

    std::vector<DiscountFactor> expected(std::size(days));
    for (i=0; i<std::size(days); i++)
        expected[i] = vars.termStructure->discount(today+days[i]);

    Settings::instance().evaluationDate() = today+30;
    std::vector<DiscountFactor> calculated(std::size(days));
    for (i=0; i<std::size(days); i++)
        calculated[i] = vars.termStructure->discount(today+30+days[i]);

    for (i=0; i<std::size(days); i++) {
        if (!close(expected[i],calculated[i]))
            BOOST_ERROR("\n  Discount at " << days[i] << " days:\n"
                        << std::setprecision(12)
                        << "    before date change: " << expected[i] << "\n"
                        << "    after date change:  " << calculated[i]);
    }
}

BOOST_AUTO_TEST_CASE(testImplied) {

    BOOST_TEST_MESSAGE("Testing consistency of implied term structure...");

    CommonVars vars;

    Real tolerance = 1.0e-10;
    Date today = Settings::instance().evaluationDate();
    Date newToday = today + 3*Years;
    Date newSettlement = vars.calendar.advance(newToday,
                                               vars.settlementDays,Days);
    Date testDate = newSettlement + 5*Years;
    ext::shared_ptr<YieldTermStructure> implied(
        new ImpliedTermStructure(Handle<YieldTermStructure>(vars.termStructure),
                                 newSettlement));
    DiscountFactor baseDiscount = vars.termStructure->discount(newSettlement);
    DiscountFactor discount = vars.termStructure->discount(testDate);
    DiscountFactor impliedDiscount = implied->discount(testDate);
    if (std::fabs(discount - baseDiscount*impliedDiscount) > tolerance)
        BOOST_ERROR(
            "unable to reproduce discount from implied curve\n"
            << std::fixed << std::setprecision(10)
            << "    calculated: " << baseDiscount*impliedDiscount << "\n"
            << "    expected:   " << discount);
}

BOOST_AUTO_TEST_CASE(testImpliedObs) {

    BOOST_TEST_MESSAGE("Testing observability of implied term structure...");

    CommonVars vars;

    Date today = Settings::instance().evaluationDate();
    Date newToday = today + 3*Years;
    Date newSettlement = vars.calendar.advance(newToday,
                                               vars.settlementDays,Days);
    RelinkableHandle<YieldTermStructure> h;
    ext::shared_ptr<YieldTermStructure> implied(
                                  new ImpliedTermStructure(h, newSettlement));
    Flag flag;
    flag.registerWith(implied);
    h.linkTo(vars.termStructure);
    if (!flag.isUp())
        BOOST_ERROR("Observer was not notified of term structure change");
}

BOOST_AUTO_TEST_CASE(testFSpreaded) {

    BOOST_TEST_MESSAGE("Testing consistency of forward-spreaded term structure...");

    CommonVars vars;

    Real tolerance = 1.0e-10;
    ext::shared_ptr<Quote> me(new SimpleQuote(0.01));
    Handle<Quote> mh(me);
    ext::shared_ptr<YieldTermStructure> spreaded(
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

BOOST_AUTO_TEST_CASE(testFSpreadedObs) {

    BOOST_TEST_MESSAGE("Testing observability of forward-spreaded "
                       "term structure...");

    CommonVars vars;

    ext::shared_ptr<SimpleQuote> me(new SimpleQuote(0.01));
    Handle<Quote> mh(me);
    RelinkableHandle<YieldTermStructure> h; //(vars.dummyTermStructure);
    ext::shared_ptr<YieldTermStructure> spreaded(
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

BOOST_AUTO_TEST_CASE(testZSpreaded) {

    BOOST_TEST_MESSAGE("Testing consistency of zero-spreaded term structure...");

    CommonVars vars;

    Real tolerance = 1.0e-10;
    ext::shared_ptr<Quote> me(new SimpleQuote(0.01));
    Handle<Quote> mh(me);
    ext::shared_ptr<YieldTermStructure> spreaded(
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

BOOST_AUTO_TEST_CASE(testZSpreadedObs) {

    BOOST_TEST_MESSAGE("Testing observability of zero-spreaded term structure...");

    CommonVars vars;

    ext::shared_ptr<SimpleQuote> me(new SimpleQuote(0.01));
    Handle<Quote> mh(me);
    RelinkableHandle<YieldTermStructure> h(vars.dummyTermStructure);

    ext::shared_ptr<YieldTermStructure> spreaded(
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

BOOST_AUTO_TEST_CASE(testCreateWithNullUnderlying) {
    BOOST_TEST_MESSAGE(
        "Testing that a zero-spreaded curve can be created with "
        "a null underlying curve...");

    CommonVars vars;

    Handle<Quote> spread(ext::shared_ptr<Quote>(new SimpleQuote(0.01)));
    RelinkableHandle<YieldTermStructure> underlying;
    // this shouldn't throw
    ext::shared_ptr<YieldTermStructure> spreaded(
        new ZeroSpreadedTermStructure(underlying,spread));
    // if we do this, the curve can work.
    underlying.linkTo(vars.termStructure);
    // check that we can use it
    spreaded->referenceDate();
}

BOOST_AUTO_TEST_CASE(testLinkToNullUnderlying) {
    BOOST_TEST_MESSAGE(
        "Testing that an underlying curve can be relinked to "
        "a null underlying curve...");

    CommonVars vars;

    Handle<Quote> spread(ext::shared_ptr<Quote>(new SimpleQuote(0.01)));
    RelinkableHandle<YieldTermStructure> underlying(vars.termStructure);
    ext::shared_ptr<YieldTermStructure> spreaded(
        new ZeroSpreadedTermStructure(underlying,spread));
    // check that we can use it
    spreaded->referenceDate();
    // if we do this, the curve can't work anymore. But it shouldn't
    // throw as long as we don't try to use it.
    underlying.linkTo(ext::shared_ptr<YieldTermStructure>());
}

BOOST_AUTO_TEST_CASE(testCompositeZeroYieldStructures) {
    BOOST_TEST_MESSAGE(
        "Testing composite zero yield structures...");

    Settings::instance().evaluationDate() = Date(10, Nov, 2017);

    // First curve
    std::vector<Date> dates = {Date(10, Nov, 2017), Date(13, Nov, 2017), Date(12, Feb, 2018),
                               Date(10, May, 2018), Date(10, Aug, 2018), Date(12, Nov, 2018),
                               Date(21, Dec, 2018), Date(15, Jan, 2020), Date(31, Mar, 2021),
                               Date(28, Feb, 2023), Date(21, Dec, 2026), Date(31, Jan, 2030),
                               Date(28, Feb, 2031), Date(31, Mar, 2036), Date(28, Feb, 2041),
                               Date(28, Feb, 2048), Date(31, Dec, 2141)};

    std::vector<Rate> rates = {0.0655823213132524, 0.0655823213132524, 0.0699455024156877,
                               0.0799107139233497, 0.0813931951022577, 0.0841615820666691,
                               0.0501297919004145, 0.0823483583439658, 0.0860720030924466,
                               0.0922887604375688, 0.10588902278996,   0.117021968693922,
                               0.109824660896137,  0.109231572878364,  0.119218123236241,
                               0.128647300167664,  0.0506086995288751};

    ext::shared_ptr<YieldTermStructure> termStructure1 = ext::shared_ptr<YieldTermStructure>(
        new ForwardCurve(dates, rates, Actual365Fixed(), NullCalendar()));

    // Second curve
    dates = {Date(10, Nov, 2017), Date(13, Nov, 2017), Date(11, Dec, 2017), Date(12, Feb, 2018),
             Date(10, May, 2018), Date(31, Jan, 2022), Date(7, Dec, 2023),  Date(31, Jan, 2025),
             Date(31, Mar, 2028), Date(7, Dec, 2033),  Date(1, Feb, 2038),  Date(2, Apr, 2046),
             Date(2, Jan, 2051),  Date(31, Dec, 2141)};

    rates = {0.056656806197189,  0.056656806197189,  0.0419541633454473, 0.0286681050019797,
             0.0148840226959593, 0.0246680238374363, 0.0255349067810599, 0.0298907184711927,
             0.0263943927922053, 0.0291924526539802, 0.0270049276163556, 0.028775807327614,
             0.0293567711641792, 0.010518655099659};

    ext::shared_ptr<YieldTermStructure> termStructure2 = ext::shared_ptr<YieldTermStructure>(
        new ForwardCurve(dates, rates, Actual365Fixed(), NullCalendar()));

    typedef Real(*binary_f)(Real, Real);

    ext::shared_ptr<YieldTermStructure> compoundCurve = ext::shared_ptr<YieldTermStructure>(
        new CompositeZeroYieldStructure<binary_f>(Handle<YieldTermStructure>(termStructure1), Handle<YieldTermStructure>(termStructure2), sub));

    // Expected values
    dates = {Date(10, Nov, 2017), Date(15, Dec, 2017), Date(15, Jun, 2018), Date(15, Sep, 2029),
             Date(15, Sep, 2038), Date(15, Mar, 2046), Date(15, Dec, 2141)};

    rates = {0.00892551511527986, 0.0278755322562788, 0.0512001768603456, 0.0729941474263546,
             0.0778333309498459,  0.0828451659139004, 0.0503573807521742};

    Real tolerance = 1.0e-10;
    for (Size i = 0; i < dates.size(); ++i) {
        Rate actual = compoundCurve->zeroRate(dates[i], Actual365Fixed(), Continuous).rate();
        Rate expected = rates[i];

        if (std::fabs(actual - expected) > tolerance)
            BOOST_ERROR(
                "unable to reproduce zero yield rate from composite input curve\n"
                << std::fixed << std::setprecision(10)
                << "    calculated: " << actual << "\n"
                << "    expected:   " << expected);
    }
}

BOOST_AUTO_TEST_CASE(testNullTimeToReference) {
    BOOST_TEST_MESSAGE("Testing zero-rate calculation for null time-to-reference...");

    Rate rate = 0.02;
    auto dayCount = Thirty360(Thirty360::BondBasis);
    auto curve = FlatForward(Date(30, August, 2023), rate, dayCount);

    // the time between August 30th and 31st is null for the 30/360 day count convention
    Rate expected = rate;
    Rate calculated = curve.zeroRate(Date(31, August, 2023), dayCount, Continuous);
    Real tolerance = 1.0e-10;

    if (std::fabs(calculated - expected) > tolerance)
        BOOST_ERROR("unable to reproduce zero yield rate from curve\n"
                    << std::fixed << std::setprecision(10)
                    << "    calculated: " << calculated << "\n"
                    << "    expected:   " << expected);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
