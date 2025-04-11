/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2014 Peter Caspers

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

#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/instruments/makeois.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/ibor/estr.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/fedfunds.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/overnightindexedcouponpricer.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/utilities/dataformatters.hpp>

#include <iostream>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::exp;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(OvernightIndexedSwapTests)

struct Datum {
    Integer settlementDays;
    Integer n;
    TimeUnit unit;
    Rate rate;
};

Datum depositData[] = {
    { 0, 1, Days, 1.10 },
    { 1, 1, Days, 1.10 },
    { 2, 1, Weeks, 1.40 },
    { 2, 2, Weeks, 1.50 },
    { 2, 1, Months, 1.70 },
    { 2, 2, Months, 1.90 },
    { 2, 3, Months, 2.05 },
    { 2, 4, Months, 2.08 },
    { 2, 5, Months, 2.11 },
    { 2, 6, Months, 2.13 }
};

Datum estrSwapData[] = {
    { 2,  1, Weeks, 1.245 },
    { 2,  2, Weeks, 1.269 },
    { 2,  3, Weeks, 1.277 },
    { 2,  1, Months, 1.281 },
    { 2,  2, Months, 1.18 },
    { 2,  3, Months, 1.143 },
    { 2,  4, Months, 1.125 },
    { 2,  5, Months, 1.116 },
    { 2,  6, Months, 1.111 },
    { 2,  7, Months, 1.109 },
    { 2,  8, Months, 1.111 },
    { 2,  9, Months, 1.117 },
    { 2, 10, Months, 1.129 },
    { 2, 11, Months, 1.141 },
    { 2, 12, Months, 1.153 },
    { 2, 15, Months, 1.218 },
    { 2, 18, Months, 1.308 },
    { 2, 21, Months, 1.407 },
    { 2,  2,  Years, 1.510 },
    { 2,  3,  Years, 1.916 },
    { 2,  4,  Years, 2.254 },
    { 2,  5,  Years, 2.523 },
    { 2,  6,  Years, 2.746 },
    { 2,  7,  Years, 2.934 },
    { 2,  8,  Years, 3.092 },
    { 2,  9,  Years, 3.231 },
    { 2, 10,  Years, 3.380 },
    { 2, 11,  Years, 3.457 },
    { 2, 12,  Years, 3.544 },
    { 2, 15,  Years, 3.702 },
    { 2, 20,  Years, 3.703 },
    { 2, 25,  Years, 3.541 },
    { 2, 30,  Years, 3.369 }
};

struct CommonVars {
    // global data
    Date today, settlement;
    Swap::Type type;
    Real nominal;
    Calendar calendar;
    Natural settlementDays;

    Period fixedEstrPeriod, floatingEstrPeriod;
    DayCounter fixedEstrDayCount;
    BusinessDayConvention fixedEstrConvention, floatingEstrConvention;
    ext::shared_ptr<Estr> estrIndex;
    RelinkableHandle<YieldTermStructure> estrTermStructure;

    Frequency fixedSwapFrequency;
    DayCounter fixedSwapDayCount;
    BusinessDayConvention fixedSwapConvention;
    ext::shared_ptr<IborIndex> swapIndex;
    RelinkableHandle<YieldTermStructure> swapTermStructure;

    // utilities
    ext::shared_ptr<OvernightIndexedSwap>
    makeSwap(Period length,
             Rate fixedRate,
             Spread spread,
             bool telescopicValueDates,
             Date effectiveDate = Date(),
             Integer paymentLag = 0,
             RateAveraging::Type averagingMethod = RateAveraging::Compound) {
        return MakeOIS(length, estrIndex, fixedRate, 0 * Days)
            .withEffectiveDate(effectiveDate == Date() ? settlement : effectiveDate)
            .withOvernightLegSpread(spread)
            .withNominal(nominal)
            .withPaymentLag(paymentLag)
            .withDiscountingTermStructure(estrTermStructure)
            .withTelescopicValueDates(telescopicValueDates)
            .withAveragingMethod(averagingMethod);
    }

    ext::shared_ptr<OvernightIndexedSwap>
    makeSwapWithLookback(Period length,
                         Rate fixedRate,
                         Integer paymentLag,
                         Natural lookbackDays,
                         Natural lockoutDays,
                         bool applyObservationShift,
                         bool telescopicValueDates) {
        return MakeOIS(length, estrIndex, fixedRate, 0 * Days)
            .withEffectiveDate(settlement)
            .withNominal(nominal)
            .withPaymentLag(paymentLag)
            .withDiscountingTermStructure(estrTermStructure)
            .withLookbackDays(lookbackDays)
            .withLockoutDays(lockoutDays)
            .withObservationShift(applyObservationShift)
            .withTelescopicValueDates(telescopicValueDates);
    }

    CommonVars() {
        type = Swap::Payer;
        settlementDays = 2;
        nominal = 100.0;
        fixedEstrConvention = ModifiedFollowing;
        floatingEstrConvention = ModifiedFollowing;
        fixedEstrPeriod = 1*Years;
        floatingEstrPeriod = 1*Years;
        fixedEstrDayCount = Actual360();
        estrIndex = ext::make_shared<Estr>(estrTermStructure);
        fixedSwapConvention = ModifiedFollowing;
        fixedSwapFrequency = Annual;
        fixedSwapDayCount = Thirty360(Thirty360::BondBasis);
        swapIndex = ext::make_shared<Euribor3M>(swapTermStructure);
        calendar = estrIndex->fixingCalendar();
        today = Date(5, February, 2009);
        Settings::instance().evaluationDate() = today;
        settlement = calendar.advance(today,settlementDays*Days,Following);
        estrTermStructure.linkTo(flatRate(today, 0.05, Actual365Fixed()));
    }
};

void testBootstrap(bool telescopicValueDates,
                   RateAveraging::Type averagingMethod,
                   Real tolerance = 1.0e-8) {

    CommonVars vars;

    Natural paymentLag = 2;

    std::vector<ext::shared_ptr<RateHelper> > estrHelpers;
    Handle<Quote> spread(ext::shared_ptr<Quote>(new SimpleQuote(0.00)));

    auto euribor3m = ext::make_shared<Euribor3M>();
    auto estr = ext::make_shared<Estr>();

    for (auto& i : depositData) {
        Real rate = 0.01 * i.rate;
        ext::shared_ptr<SimpleQuote> simple = ext::make_shared<SimpleQuote>(rate);
        ext::shared_ptr<Quote> quote (simple);
        Period term = i.n * i.unit;
        auto helper = ext::make_shared<DepositRateHelper>(
                Handle<Quote>(quote), term, i.settlementDays, euribor3m->fixingCalendar(),
                euribor3m->businessDayConvention(), euribor3m->endOfMonth(), euribor3m->dayCounter());

        if (term <= 2*Days)
            estrHelpers.push_back(helper);
    }

    for (auto& i : estrSwapData) {
        Real rate = 0.01 * i.rate;
        ext::shared_ptr<SimpleQuote> simple = ext::make_shared<SimpleQuote>(rate);
        ext::shared_ptr<Quote> quote (simple);
        Period term = i.n * i.unit;
        auto helper = ext::make_shared<OISRateHelper>(i.settlementDays,
                                                      term,
                                                      Handle<Quote>(quote),
                                                      estr,
                                                      Handle<YieldTermStructure>(),
                                                      telescopicValueDates,
                                                      paymentLag,
                                                      Following,
                                                      Annual,
                                                      Calendar(),
                                                      0 * Days,
                                                      spread,
                                                      Pillar::LastRelevantDate,
                                                      Date(),
                                                      averagingMethod);
        estrHelpers.push_back(helper);
    }

    auto estrTS = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(vars.today, estrHelpers, Actual365Fixed());

    vars.estrTermStructure.linkTo(estrTS);

    // test curve consistency
    for (auto& i : estrSwapData) {
        Rate expected = i.rate / 100;
        Period term = i.n * i.unit;
        // test telescopic value dates (in bootstrap) against non telescopic value dates (swap here)
        ext::shared_ptr<OvernightIndexedSwap> swap =
            vars.makeSwap(term, 0.0, 0.0, false, Date(), paymentLag, averagingMethod);
        Rate calculated = swap->fairRate();
        Rate error = std::fabs(expected-calculated);

        if (error>tolerance)
            BOOST_ERROR("curve inconsistency:" << std::setprecision(10) <<
                        "\n swap length:     " << term <<
                        "\n quoted rate:     " << expected <<
                        "\n calculated rate: " << calculated <<
                        "\n error:           " << error <<
                        "\n tolerance:       " << tolerance);
    }
}


BOOST_AUTO_TEST_CASE(testFairRate) {

    BOOST_TEST_MESSAGE("Testing Estr-swap calculation of fair fixed rate...");

    CommonVars vars;

    Period lengths[] = { 1*Years, 2*Years, 5*Years, 10*Years, 20*Years };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };

    for (auto& length : lengths) {
        for (Real spread : spreads) {

            ext::shared_ptr<OvernightIndexedSwap> swap = vars.makeSwap(length, 0.0, spread, false);
            ext::shared_ptr<OvernightIndexedSwap> swap2 = vars.makeSwap(length, 0.0, spread, true);
            if(std::fabs(swap->fairRate()-swap2->fairRate()) > 1.0e-10) {
                BOOST_ERROR("fair rates are different:\n"
                            << std::setprecision(2) << "    length: " << length << " \n"
                            << "    floating spread: " << io::rate(spread) << "\n"
                            << std::setprecision(12)
                            << "    fair rate (non telescopic value dates): " << swap->fairRate()
                            << "\n"
                            << "    fair rate (telescopic value dates)    : " << swap2->fairRate());
            }
            swap = vars.makeSwap(length, swap->fairRate(), spread, false);
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("recalculating with implied rate (non telescopic value dates):\n"
                            << std::setprecision(2) << "    length: " << length << " \n"
                            << "    floating spread: " << io::rate(spread) << "\n"
                            << "    swap value: " << swap->NPV());
            }
            swap = vars.makeSwap(length, swap->fairRate(), spread, true);
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("recalculating with implied rate (telescopic value dates):\n"
                            << std::setprecision(2) << "    length: " << length << " \n"
                            << "    floating spread: " << io::rate(spread) << "\n"
                            << "    swap value: " << swap->NPV());
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testFairSpread) {

    BOOST_TEST_MESSAGE("Testing Estr-swap calculation of fair floating spread...");

    CommonVars vars;

    Period lengths[] = { 1*Years, 2*Years, 5*Years, 10*Years, 20*Years };
    Rate rates[] = { 0.04, 0.05, 0.06, 0.07 };

    for (auto& length : lengths) {
        for (Real j : rates) {

            ext::shared_ptr<OvernightIndexedSwap> swap = vars.makeSwap(length, j, 0.0, false);
            ext::shared_ptr<OvernightIndexedSwap> swap2 = vars.makeSwap(length, j, 0.0, true);
            Spread fairSpread = swap->fairSpread();
            Spread fairSpread2 = swap2->fairSpread();
            if(std::fabs(fairSpread-fairSpread2) > 1.0e-10) {
                BOOST_ERROR(
                    "fair spreads are different:\n"
                    << std::setprecision(2) << "\n     length: " << length
                    << "\n fixed rate: " << io::rate(j)
                    << "\nfair spread (non telescopic value dates): " << io::rate(fairSpread)
                    << "\nfair spread (telescopic value dates)    : " << io::rate(fairSpread2));
            }
            swap = vars.makeSwap(length, j, fairSpread, false);
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("\nrecalculating with implied spread (non telescopic value dates):"
                            << std::setprecision(2) << "\n     length: " << length
                            << "\n fixed rate: " << io::rate(j) << "\nfair spread: "
                            << io::rate(fairSpread) << "\n swap value: " << swap->NPV());
            }
            swap = vars.makeSwap(length, j, fairSpread, true);
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("\nrecalculating with implied spread (telescopic value dates):"
                            << std::setprecision(2) << "\n     length: " << length
                            << "\n fixed rate: " << io::rate(j) << "\nfair spread: "
                            << io::rate(fairSpread) << "\n swap value: " << swap->NPV());
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testCachedValue) {

    BOOST_TEST_MESSAGE("Testing Estr-swap calculation against cached value...");

    CommonVars vars;

    Settings::instance().evaluationDate() = vars.today;
    vars.settlement =
        vars.calendar.advance(vars.today,vars.settlementDays,Days);
    Real flat = 0.05;
    vars.estrTermStructure.linkTo(flatRate(vars.settlement,flat,Actual360()));
    Real fixedRate = exp(flat) - 1;
    ext::shared_ptr<OvernightIndexedSwap> swap = vars.makeSwap(1*Years, fixedRate, 0.0,false);
    ext::shared_ptr<OvernightIndexedSwap> swap2 = vars.makeSwap(1*Years, fixedRate, 0.0,true);
    Real cachedNPV   = 0.001730450147;
    Real tolerance = 1.0e-11;
    if (std::fabs(swap->NPV()-cachedNPV) > tolerance)
        BOOST_ERROR("\nfailed to reproduce cached swap value (non telescopic value dates):" <<
                    std::fixed << std::setprecision(12) <<
                    "\ncalculated: " << swap->NPV() <<
                    "\n  expected: " << cachedNPV <<
                    "\n tolerance:" << tolerance);
    if (std::fabs(swap2->NPV()-cachedNPV) > tolerance)
        BOOST_ERROR("\nfailed to reproduce cached swap value (telescopic value dates):" <<
                    std::fixed << std::setprecision(12) <<
                    "\ncalculated: " << swap->NPV() <<
                    "\n  expected: " << cachedNPV <<
                    "\n tolerance:" << tolerance);
}

BOOST_AUTO_TEST_CASE(testBaseBootstrap) {
    BOOST_TEST_MESSAGE("Testing Estr-swap curve building with daily compounded ON rates...");
    testBootstrap(false, RateAveraging::Compound);
}

BOOST_AUTO_TEST_CASE(testBootstrapWithArithmeticAverage) {
    BOOST_TEST_MESSAGE("Testing Estr-swap curve building with arithmetic average ON rates...");
    testBootstrap(false, RateAveraging::Simple);
}

BOOST_AUTO_TEST_CASE(testBootstrapWithTelescopicDates) {
    BOOST_TEST_MESSAGE(
        "Testing Estr-swap curve building with telescopic value dates and DCON rates...");
    testBootstrap(true, RateAveraging::Compound);
}

BOOST_AUTO_TEST_CASE(testBootstrapWithTelescopicDatesAndArithmeticAverage) {
    BOOST_TEST_MESSAGE(
        "Testing Estr-swap curve building with telescopic value dates and AAON rates...");
    // Given that we are using an approximation that omits
    // the required convexity correction, a lower tolerance
    // is needed.
    testBootstrap(true, RateAveraging::Simple, 1.0e-5);
}

BOOST_AUTO_TEST_CASE(testBootstrapWithCustomPricer) {
    BOOST_TEST_MESSAGE("Testing Estr-swap curve building with custom pricer...");

    CommonVars vars;

    Natural paymentLag = 2;
    bool telescopicValueDates = false;
    auto averagingMethod = RateAveraging::Simple;
    auto pricer =
        ext::make_shared<ArithmeticAveragedOvernightIndexedCouponPricer>(0.02, 0.15, true);

    std::vector<ext::shared_ptr<RateHelper> > estrHelpers;
    Handle<Quote> spread(ext::shared_ptr<Quote>(new SimpleQuote(0.00)));

    auto euribor3m = ext::make_shared<Euribor3M>();
    auto estr = ext::make_shared<Estr>();

    for (auto& i : estrSwapData) {
        Real rate = 0.01 * i.rate;
        ext::shared_ptr<SimpleQuote> simple = ext::make_shared<SimpleQuote>(rate);
        ext::shared_ptr<Quote> quote (simple);
        Period term = i.n * i.unit;
        auto helper = ext::make_shared<OISRateHelper>(i.settlementDays,
                                                      term,
                                                      Handle<Quote>(quote),
                                                      estr,
                                                      Handle<YieldTermStructure>(),
                                                      telescopicValueDates,
                                                      paymentLag,
                                                      Following,
                                                      Annual,
                                                      Calendar(),
                                                      0 * Days,
                                                      spread,
                                                      Pillar::LastRelevantDate,
                                                      Date(),
                                                      averagingMethod,
                                                      ext::nullopt,
                                                      ext::nullopt,
                                                      Calendar(),
                                                      Null<Natural>(),
                                                      0,
                                                      false,
                                                      pricer);
        estrHelpers.push_back(helper);
    }

    auto estrTS = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(vars.today, estrHelpers, Actual365Fixed());

    vars.estrTermStructure.linkTo(estrTS);

    // test curve consistency
    for (auto& i : estrSwapData) {
        Rate expected = i.rate / 100;
        Period term = i.n * i.unit;

        ext::shared_ptr<OvernightIndexedSwap> swap =
            vars.makeSwap(term, 0.0, 0.0, false, Date(), paymentLag, averagingMethod);
        setCouponPricer(swap->overnightLeg(), pricer);

        Rate calculated = swap->fairRate();
        Rate error = std::fabs(expected-calculated);
        Real tolerance = 1.0e-8;

        if (error>tolerance)
            BOOST_ERROR("curve inconsistency:" << std::setprecision(10) <<
                        "\n swap length:     " << term <<
                        "\n quoted rate:     " << expected <<
                        "\n calculated rate: " << calculated <<
                        "\n error:           " << error <<
                        "\n tolerance:       " << tolerance);
    }
}


void testBootstrapWithLookback(Natural lookbackDays,
                               Natural lockoutDays,
                               bool applyObservationShift,
                               bool telescopicValueDates,
                               Natural paymentLag) {

    CommonVars vars;

    std::vector<ext::shared_ptr<RateHelper> > estrHelpers;

    auto estr = ext::make_shared<Estr>();
    Handle<Quote> spread(ext::shared_ptr<Quote>(new SimpleQuote(0.00)));


    for (auto& i : estrSwapData) {
        Real rate = 0.01 * i.rate;
        auto quote = ext::make_shared<SimpleQuote>(rate);
        Period term = i.n * i.unit;
        auto helper = ext::make_shared<OISRateHelper>(i.settlementDays,
                                                      term,
                                                      Handle<Quote>(quote),
                                                      estr,
                                                      Handle<YieldTermStructure>(),
                                                      telescopicValueDates,
                                                      paymentLag,
                                                      Following,
                                                      Annual,
                                                      Calendar(),
                                                      0 * Days,
                                                      spread,
                                                      Pillar::LastRelevantDate,
                                                      Date(),
                                                      RateAveraging::Compound,
                                                      ext::nullopt,
                                                      ext::nullopt,
                                                      Calendar(),
                                                      lookbackDays,
                                                      lockoutDays,
                                                      applyObservationShift);
        estrHelpers.push_back(helper);
    }

    auto estrTS = ext::make_shared<PiecewiseYieldCurve<ForwardRate, BackwardFlat>>(vars.today, estrHelpers, Actual365Fixed());

    vars.estrTermStructure.linkTo(estrTS);

    // test curve consistency
    for (auto& i : estrSwapData) {
        Rate expected = i.rate / 100;
        Period term = i.n * i.unit;
        ext::shared_ptr<OvernightIndexedSwap> swap =
            vars.makeSwapWithLookback(term, 0.0, paymentLag, lookbackDays, lockoutDays,
                                      applyObservationShift, telescopicValueDates);
        Rate calculated = swap->fairRate();
        Rate error = std::fabs(expected-calculated);
        Real tolerance = 1e-8;

        if (error>tolerance)
            BOOST_ERROR("curve inconsistency:" << std::setprecision(10) <<
                        "\n swap length:     " << term <<
                        "\n quoted rate:     " << expected <<
                        "\n calculated rate: " << calculated <<
                        "\n error:           " << error <<
                        "\n tolerance:       " << tolerance);
    }
}



BOOST_AUTO_TEST_CASE(testBootstrapWithLookbackDays) {
    BOOST_TEST_MESSAGE("Testing Estr-swap curve building with lookback days...");

    auto lookbackDays = 2;
    auto lockoutDays = 0;
    auto applyObservationShift = false;
    auto paymentLag = 2;

    testBootstrapWithLookback(lookbackDays, lockoutDays, applyObservationShift, false, paymentLag);

    BOOST_CHECK_EXCEPTION(
        testBootstrapWithLookback(lookbackDays, lockoutDays, applyObservationShift, true, paymentLag),
        Error, ExpectedErrorMessage("Telescopic formula cannot be applied"));
}

BOOST_AUTO_TEST_CASE(testBootstrapWithLookbackDaysAndShift) {
    BOOST_TEST_MESSAGE("Testing Estr-swap curve building with lookback days and observation shift...");

    auto lookbackDays = 2;
    auto lockoutDays = 0;
    auto applyObservationShift = true;
    auto paymentLag = 2;

    testBootstrapWithLookback(lookbackDays, lockoutDays, applyObservationShift, false, paymentLag);

    testBootstrapWithLookback(lookbackDays, lockoutDays, applyObservationShift, true, paymentLag);
}

BOOST_AUTO_TEST_CASE(testBootstrapWithLockoutDays) {
    BOOST_TEST_MESSAGE("Testing Estr-swap curve building with lookback and lockout days...");

    auto lookbackDays = 2;
    auto lockoutDays = 2;
    auto applyObservationShift = false;
    auto paymentLag = 0;

    testBootstrapWithLookback(lookbackDays, lockoutDays, applyObservationShift, false, paymentLag);

    BOOST_CHECK_EXCEPTION(
        testBootstrapWithLookback(lookbackDays, lockoutDays, applyObservationShift, true, paymentLag),
        Error, ExpectedErrorMessage("Telescopic formula cannot be applied"));
}

BOOST_AUTO_TEST_CASE(testBootstrapWithLockoutDaysAndShift) {
    BOOST_TEST_MESSAGE("Testing Estr-swap curve building with lookback and lockout days and observation shift...");

    auto lookbackDays = 2;
    auto lockoutDays = 2;
    auto applyObservationShift = true;
    auto paymentLag = 0;

    testBootstrapWithLookback(lookbackDays, lockoutDays, applyObservationShift, false, paymentLag);

    testBootstrapWithLookback(lookbackDays, lockoutDays, applyObservationShift, true, paymentLag);
}


BOOST_AUTO_TEST_CASE(testSeasonedSwaps) {

    BOOST_TEST_MESSAGE("Testing seasoned Estr-swap calculation...");

    CommonVars vars;

    Period lengths[] = { 1*Years, 2*Years, 5*Years, 10*Years, 20*Years };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };

    Date effectiveDate = Date(2, February, 2009);

    vars.estrIndex->addFixing(Date(2,February,2009), 0.0010); // fake fixing values
    vars.estrIndex->addFixing(Date(3,February,2009), 0.0011);
    vars.estrIndex->addFixing(Date(4,February,2009), 0.0012);
    vars.estrIndex->addFixing(Date(5,February,2009), 0.0013);

    for (auto& length : lengths) {
        for (Real spread : spreads) {

            ext::shared_ptr<OvernightIndexedSwap> swap =
                vars.makeSwap(length, 0.0, spread, false, effectiveDate);
            ext::shared_ptr<OvernightIndexedSwap> swap2 =
                vars.makeSwap(length, 0.0, spread, true, effectiveDate);
            if (std::fabs(swap->NPV() - swap2->NPV()) > 1.0e-10) {
                BOOST_ERROR("swap npv is different:\n"
                            << std::setprecision(2) << "    length: " << length << " \n"
                            << "    floating spread: " << io::rate(spread) << "\n"
                            << "    swap value (non telescopic value dates): " << swap->NPV()
                            << "\n    swap value (telescopic value dates    ): " << swap2->NPV());
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testBootstrapRegression) {
    BOOST_TEST_MESSAGE("Testing 1.16 regression with OIS bootstrap...");

    Datum data[] = {
        { 0,  1, Days,   0.0066   },
        { 2,  1, Weeks,  0.006445 },
        { 2,  2, Weeks,  0.006455 },
        { 2,  3, Weeks,  0.00645  },
        { 2,  1, Months, 0.00675  },
        { 2,  2, Months, 0.007    },
        { 2,  3, Months, 0.00724  },
        { 2,  4, Months, 0.007533 },
        { 2,  5, Months, 0.00785  },
        { 2,  6, Months, 0.00814  },
        { 2,  9, Months, 0.00889  },
        { 2,  1, Years,  0.00967  },
        { 2,  2, Years,  0.01221  },
        { 2,  3, Years,  0.01413  },
        { 2,  4, Years,  0.01555  },
        { 2,  5, Years,  0.01672  },
        { 2, 10, Years,  0.02005  },
        { 2, 12, Years,  0.0208   },
        { 2, 15, Years,  0.02152  },
        { 2, 20, Years,  0.02215  },
        { 2, 25, Years,  0.02233  },
        { 2, 30, Years,  0.02234  },
        { 2, 40, Years,  0.02233  }
    };

    Settings::instance().evaluationDate() = Date(21, February, 2017);

    std::vector<ext::shared_ptr<RateHelper> > helpers;
    auto index = ext::make_shared<FedFunds>();
    Handle<Quote> spread(ext::shared_ptr<Quote>(new SimpleQuote(0.00)));

    helpers.push_back(
        ext::make_shared<DepositRateHelper>(data[0].rate,
                                            Period(data[0].n, data[0].unit),
                                            index->fixingDays(),
                                            index->fixingCalendar(),
                                            index->businessDayConvention(),
                                            index->endOfMonth(),
                                            index->dayCounter()));

    for (Size i=1; i<std::size(data); ++i) {
        helpers.push_back(
            ext::make_shared<OISRateHelper>(
                                  data[i].settlementDays,
                                  Period(data[i].n, data[i].unit),
                                  Handle<Quote>(ext::make_shared<SimpleQuote>(data[i].rate)),
                                  index,
                                  Handle<YieldTermStructure>(),
                                  false, 2,
                                  Following, Annual, Calendar(), 0*Days, spread,
                                  // this bootstrap fails with the default LastRelevantDate choice
                                  Pillar::MaturityDate));
    }

    PiecewiseYieldCurve<Discount,LogCubic> curve(0, UnitedStates(UnitedStates::GovernmentBond),
                                                 helpers, Actual365Fixed(), MonotonicLogCubic());

    BOOST_CHECK_NO_THROW(curve.discount(1.0));
}

BOOST_AUTO_TEST_CASE(test131BootstrapRegression) {
    BOOST_TEST_MESSAGE("Testing 1.31 regression with OIS bootstrap...");

    Date today(11, December, 2012);
    Settings::instance().evaluationDate() = today;

    auto estr = ext::make_shared<Estr>();

    std::vector<ext::shared_ptr<RateHelper>> helpers;
    helpers.push_back(ext::make_shared<OISRateHelper>(2, 1 * Weeks, makeQuoteHandle(0.070/100), estr));
    helpers.push_back(ext::make_shared<OISRateHelper>(Date(16, January, 2013), Date(13, February, 2013), makeQuoteHandle(0.046/100), estr));

    auto curve = PiecewiseYieldCurve<ForwardRate,BackwardFlat>(0, TARGET(), helpers, Actual365Fixed());
    BOOST_CHECK_NO_THROW(curve.nodes());
}

BOOST_AUTO_TEST_CASE(testDeprecatedHelper) {
    BOOST_TEST_MESSAGE("Testing deprecated DatedOISRateHelper class...");

    Date today(11, December, 2012);
    Settings::instance().evaluationDate() = today;

    auto estr = ext::make_shared<Estr>();

    std::vector<ext::shared_ptr<RateHelper>> helpers;
    helpers.push_back(ext::make_shared<OISRateHelper>(2, 1 * Weeks, makeQuoteHandle(0.070/100), estr));
    QL_DEPRECATED_DISABLE_WARNING
    helpers.push_back(ext::make_shared<DatedOISRateHelper>(Date(16, January, 2013), Date(13, February, 2013), makeQuoteHandle(0.046/100), estr));
    QL_DEPRECATED_ENABLE_WARNING

    auto curve = ext::make_shared<PiecewiseYieldCurve<ForwardRate,BackwardFlat>>(0, TARGET(), helpers, Actual365Fixed());
    BOOST_CHECK_NO_THROW(curve->nodes());

    estr = ext::make_shared<Estr>(Handle<YieldTermStructure>(curve));
    ext::shared_ptr<OvernightIndexedSwap> swap =
        MakeOIS(Period(), estr, 0.046/100, 0 * Days)
        .withEffectiveDate(Date(16, January, 2013))
        .withTerminationDate(Date(13, February, 2013))
        .withDiscountingTermStructure(Handle<YieldTermStructure>(curve));

    if (std::fabs(swap->NPV()) > 1.0e-10) {
        BOOST_ERROR("npv is not at par:\n"
                    << "    swap value: " << swap->NPV());
    }
}

BOOST_AUTO_TEST_CASE(testConstructorsAndNominals) {
    BOOST_TEST_MESSAGE("Testing different constructors for OIS...");

    CommonVars vars;

    Date spot = vars.calendar.advance(vars.today, 2*Days);
    Real nominal = 100000.0;

    // constant notional, same schedule

    Schedule schedule = MakeSchedule()
        .from(spot)
        .to(vars.calendar.advance(spot, 2*Years))
        .withCalendar(vars.calendar)
        .withFrequency(Annual);

    auto ois_1 = OvernightIndexedSwap(Swap::Payer,
                                      nominal,
                                      schedule,
                                      0.03,
                                      Actual360(),
                                      vars.estrIndex);

    BOOST_CHECK_EQUAL(ois_1.fixedSchedule().tenor(), 1*Years);
    BOOST_CHECK_EQUAL(ois_1.overnightSchedule().tenor(), 1*Years);
    BOOST_CHECK_EQUAL(ois_1.paymentFrequency(), Annual);

    BOOST_CHECK_EQUAL(ois_1.nominal(), nominal);

    BOOST_CHECK_EQUAL(ois_1.nominals().size(), Size(1));
    BOOST_CHECK_EQUAL(ois_1.nominals()[0], nominal);

    BOOST_CHECK_EQUAL(ois_1.fixedNominals().size(), Size(1));
    BOOST_CHECK_EQUAL(ois_1.fixedNominals()[0], nominal);

    BOOST_CHECK_EQUAL(ois_1.overnightNominals().size(), Size(1));
    BOOST_CHECK_EQUAL(ois_1.overnightNominals()[0], nominal);

    // amortizing notionals, same schedule

    auto ois_2 = OvernightIndexedSwap(Swap::Payer,
                                      { nominal, nominal/2 },
                                      schedule,
                                      0.03,
                                      Actual360(),
                                      vars.estrIndex);

    BOOST_CHECK_EQUAL(ois_2.fixedSchedule().tenor(), 1*Years);
    BOOST_CHECK_EQUAL(ois_2.overnightSchedule().tenor(), 1*Years);
    BOOST_CHECK_EQUAL(ois_2.paymentFrequency(), Annual);

    BOOST_CHECK_EXCEPTION(ois_2.nominal(), Error,
                          ExpectedErrorMessage("nominal is not constant"));

    BOOST_CHECK_EQUAL(ois_2.nominals().size(), Size(2));
    BOOST_CHECK_EQUAL(ois_2.nominals()[0], nominal);
    BOOST_CHECK_EQUAL(ois_2.nominals()[1], nominal/2);

    BOOST_CHECK_EQUAL(ois_2.fixedNominals().size(), Size(2));
    BOOST_CHECK_EQUAL(ois_2.fixedNominals()[0], nominal);
    BOOST_CHECK_EQUAL(ois_2.fixedNominals()[1], nominal/2);

    BOOST_CHECK_EQUAL(ois_2.overnightNominals().size(), Size(2));
    BOOST_CHECK_EQUAL(ois_2.overnightNominals()[0], nominal);
    BOOST_CHECK_EQUAL(ois_2.overnightNominals()[1], nominal/2);

    // constant notional, different schedules

    const Schedule& fixedSchedule = schedule;
    Schedule overnightSchedule = MakeSchedule()
        .from(spot)
        .to(vars.calendar.advance(spot, 2*Years))
        .withCalendar(vars.calendar)
        .withFrequency(Semiannual);

    auto ois_3 = OvernightIndexedSwap(Swap::Payer,
                                      nominal,
                                      fixedSchedule,
                                      0.03,
                                      Actual360(),
                                      overnightSchedule,
                                      vars.estrIndex);

    BOOST_CHECK_EQUAL(ois_3.fixedSchedule().tenor(), 1*Years);
    BOOST_CHECK_EQUAL(ois_3.overnightSchedule().tenor(), 6*Months);
    BOOST_CHECK_EQUAL(ois_3.paymentFrequency(), Semiannual);

    BOOST_CHECK_EQUAL(ois_3.nominal(), nominal);

    BOOST_CHECK_EQUAL(ois_3.nominals().size(), Size(1));
    BOOST_CHECK_EQUAL(ois_3.nominals()[0], nominal);

    BOOST_CHECK_EQUAL(ois_3.fixedNominals().size(), Size(1));
    BOOST_CHECK_EQUAL(ois_3.fixedNominals()[0], nominal);

    BOOST_CHECK_EQUAL(ois_3.overnightNominals().size(), Size(1));
    BOOST_CHECK_EQUAL(ois_3.overnightNominals()[0], nominal);

    // amortizing notionals, different schedules

    auto ois_4 = OvernightIndexedSwap(Swap::Payer,
                                      { nominal, nominal/2 },
                                      fixedSchedule,
                                      0.03,
                                      Actual360(),
                                      { nominal, nominal, nominal/2, nominal/2 },
                                      overnightSchedule,
                                      vars.estrIndex);

    BOOST_CHECK_EQUAL(ois_4.fixedSchedule().tenor(), 1*Years);
    BOOST_CHECK_EQUAL(ois_4.overnightSchedule().tenor(), 6*Months);
    BOOST_CHECK_EQUAL(ois_4.paymentFrequency(), Semiannual);

    BOOST_CHECK_EXCEPTION(ois_4.nominal(), Error,
                          ExpectedErrorMessage("nominal is not constant"));

    BOOST_CHECK_EXCEPTION(ois_4.nominals(), Error,
                          ExpectedErrorMessage("different nominals"));

    BOOST_CHECK_EQUAL(ois_4.fixedNominals().size(), Size(2));
    BOOST_CHECK_EQUAL(ois_4.fixedNominals()[0], nominal);
    BOOST_CHECK_EQUAL(ois_4.fixedNominals()[1], nominal/2);

    BOOST_CHECK_EQUAL(ois_4.overnightNominals().size(), Size(4));
    BOOST_CHECK_EQUAL(ois_4.overnightNominals()[0], nominal);
    BOOST_CHECK_EQUAL(ois_4.overnightNominals()[1], nominal);
    BOOST_CHECK_EQUAL(ois_4.overnightNominals()[2], nominal/2);
    BOOST_CHECK_EQUAL(ois_4.overnightNominals()[3], nominal/2);
}

BOOST_AUTO_TEST_CASE(testNotifications) {
    BOOST_TEST_MESSAGE("Testing cash-flow notifications for OIS...");

    CommonVars vars;

    Date spot = vars.calendar.advance(vars.today, 2*Days);
    Real nominal = 100000.0;

    Schedule schedule = MakeSchedule()
        .from(spot)
        .to(vars.calendar.advance(spot, 2*Years))
        .withCalendar(vars.calendar)
        .withFrequency(Annual);

    RelinkableHandle<YieldTermStructure> forecast_handle;
    forecast_handle.linkTo(flatRate(0.02, Actual360()));

    RelinkableHandle<YieldTermStructure> discount_handle;
    discount_handle.linkTo(flatRate(0.02, Actual360()));

    auto index = ext::make_shared<Estr>(forecast_handle);

    auto ois = ext::make_shared<OvernightIndexedSwap>(Swap::Payer,
                                                      nominal,
                                                      schedule,
                                                      0.03,
                                                      Actual360(),
                                                      index);
    ois->setPricingEngine(ext::make_shared<DiscountingSwapEngine>(discount_handle));
    ois->NPV();

    Flag flag;
    flag.registerWith(ois);
    flag.lower();

    forecast_handle.linkTo(flatRate(0.03, Actual360()));

    if (!flag.isUp())
        BOOST_FAIL("OIS was not notified of curve change");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
