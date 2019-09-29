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

#include "overnightindexedswap.hpp"
#include "utilities.hpp"

#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/instruments/makeois.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/ibor/eonia.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/utilities/dataformatters.hpp>

#include <iostream>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::exp;

using ext::shared_ptr;

typedef PiecewiseYieldCurve<Discount,LogLinear> PiecewiseFlatForward;

namespace {

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

    Datum eoniaSwapData[] = {
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
        OvernightIndexedSwap::Type type;
        Real nominal;
        Calendar calendar;
        Natural settlementDays;

        Period fixedEoniaPeriod, floatingEoniaPeriod;
        DayCounter fixedEoniaDayCount;
        BusinessDayConvention fixedEoniaConvention, floatingEoniaConvention;
        shared_ptr<Eonia> eoniaIndex;
        RelinkableHandle<YieldTermStructure> eoniaTermStructure;

        Frequency fixedSwapFrequency;
        DayCounter fixedSwapDayCount;
        BusinessDayConvention fixedSwapConvention;
        shared_ptr<IborIndex> swapIndex;
        RelinkableHandle<YieldTermStructure> swapTermStructure;

        // cleanup
        SavedSettings backup;

        // utilities
        shared_ptr<OvernightIndexedSwap> makeSwap(Period length,
                                                  Rate fixedRate,
                                                  Spread spread,
                                                  bool telescopicValueDates,
                                                  Date effectiveDate = Null<Date>(),
                                                  Natural paymentLag = 0) {
            return MakeOIS(length, eoniaIndex, fixedRate)
                .withEffectiveDate(effectiveDate == Null<Date>() ? settlement : effectiveDate)
                .withOvernightLegSpread(spread)
                .withNominal(nominal)
                .withPaymentLag(paymentLag)
                .withDiscountingTermStructure(eoniaTermStructure)
                .withTelescopicValueDates(telescopicValueDates);
        }

        CommonVars() {
            type = OvernightIndexedSwap::Payer;
            settlementDays = 2;
            nominal = 100.0;
            fixedEoniaConvention = ModifiedFollowing;
            floatingEoniaConvention = ModifiedFollowing;
            fixedEoniaPeriod = 1*Years;
            floatingEoniaPeriod = 1*Years;
            fixedEoniaDayCount = Actual360();
            eoniaIndex = ext::make_shared<Eonia>(eoniaTermStructure);
            fixedSwapConvention = ModifiedFollowing;
            fixedSwapFrequency = Annual;
            fixedSwapDayCount = Thirty360();
            swapIndex = shared_ptr<IborIndex>(new Euribor3M(swapTermStructure));
            calendar = eoniaIndex->fixingCalendar();
            today = Date(5, February, 2009);
            //today = calendar.adjust(Date::todaysDate());
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today,settlementDays*Days,Following);
            eoniaTermStructure.linkTo(flatRate(today, 0.05,
                                               Actual365Fixed()));
        }
    };

}


void OvernightIndexedSwapTest::testFairRate() {

    BOOST_TEST_MESSAGE("Testing Eonia-swap calculation of fair fixed rate...");

    CommonVars vars;

    Period lengths[] = { 1*Years, 2*Years, 5*Years, 10*Years, 20*Years };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(spreads); j++) {

            shared_ptr<OvernightIndexedSwap> swap =
                vars.makeSwap(lengths[i],0.0,spreads[j],false);
            shared_ptr<OvernightIndexedSwap> swap2 =
                vars.makeSwap(lengths[i],0.0,spreads[j],true);
            if(std::fabs(swap->fairRate()-swap2->fairRate()) > 1.0e-10) {
                BOOST_ERROR("fair rates are different:\n"
                            << std::setprecision(2)
                            << "    length: " << lengths[i] << " \n"
                            << "    floating spread: "
                            << io::rate(spreads[j]) << "\n"
                            << std::setprecision(12)
                            << "    fair rate (non telescopic value dates): " << swap->fairRate() << "\n"
                            << "    fair rate (telescopic value dates)    : " << swap2->fairRate());
            }
            swap = vars.makeSwap(lengths[i],swap->fairRate(),spreads[j],false);
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("recalculating with implied rate (non telescopic value dates):\n"
                            << std::setprecision(2)
                            << "    length: " << lengths[i] << " \n"
                            << "    floating spread: "
                            << io::rate(spreads[j]) << "\n"
                            << "    swap value: " << swap->NPV());
            }
            swap = vars.makeSwap(lengths[i],swap->fairRate(),spreads[j],true);
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("recalculating with implied rate (telescopic value dates):\n"
                            << std::setprecision(2)
                            << "    length: " << lengths[i] << " \n"
                            << "    floating spread: "
                            << io::rate(spreads[j]) << "\n"
                            << "    swap value: " << swap->NPV());
            }
        }
    }
}


void OvernightIndexedSwapTest::testFairSpread() {

    BOOST_TEST_MESSAGE("Testing Eonia-swap calculation of "
                       "fair floating spread...");

    CommonVars vars;

    Period lengths[] = { 1*Years, 2*Years, 5*Years, 10*Years, 20*Years };
    Rate rates[] = { 0.04, 0.05, 0.06, 0.07 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(rates); j++) {

            shared_ptr<OvernightIndexedSwap> swap =
                vars.makeSwap(lengths[i], rates[j], 0.0,false);
            shared_ptr<OvernightIndexedSwap> swap2 =
                vars.makeSwap(lengths[i], rates[j], 0.0,true);
            Spread fairSpread = swap->fairSpread();
            Spread fairSpread2 = swap2->fairSpread();
            if(std::fabs(fairSpread-fairSpread2) > 1.0e-10) {
                BOOST_ERROR("fair spreads are different:\n" <<
                            std::setprecision(2) <<
                            "\n     length: " << lengths[i] <<
                            "\n fixed rate: " << io::rate(rates[j]) <<
                            "\nfair spread (non telescopic value dates): " << io::rate(fairSpread) <<
                            "\nfair spread (telescopic value dates)    : " << io::rate(fairSpread2));

            }
            swap = vars.makeSwap(lengths[i], rates[j], fairSpread,false);
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("\nrecalculating with implied spread (non telescopic value dates):" <<
                            std::setprecision(2) <<
                            "\n     length: " << lengths[i] <<
                            "\n fixed rate: " << io::rate(rates[j]) <<
                            "\nfair spread: " << io::rate(fairSpread) <<
                            "\n swap value: " << swap->NPV());
            }
            swap = vars.makeSwap(lengths[i], rates[j], fairSpread,true);
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("\nrecalculating with implied spread (telescopic value dates):" <<
                            std::setprecision(2) <<
                            "\n     length: " << lengths[i] <<
                            "\n fixed rate: " << io::rate(rates[j]) <<
                            "\nfair spread: " << io::rate(fairSpread) <<
                            "\n swap value: " << swap->NPV());
            }
        }
    }

}

void OvernightIndexedSwapTest::testCachedValue() {

    BOOST_TEST_MESSAGE("Testing Eonia-swap calculation against cached value...");

    CommonVars vars;

    Settings::instance().evaluationDate() = vars.today;
    vars.settlement =
        vars.calendar.advance(vars.today,vars.settlementDays,Days);
    Real flat = 0.05;
    vars.eoniaTermStructure.linkTo(flatRate(vars.settlement,flat,Actual360()));
    Real fixedRate = exp(flat) - 1;
    shared_ptr<OvernightIndexedSwap> swap = vars.makeSwap(1*Years, fixedRate, 0.0,false);
    shared_ptr<OvernightIndexedSwap> swap2 = vars.makeSwap(1*Years, fixedRate, 0.0,true);
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

namespace {
void testBootstrap(bool telescopicValueDates) {

    CommonVars vars;

    Natural paymentLag = 2;

    std::vector<shared_ptr<RateHelper> > eoniaHelpers;

    shared_ptr<IborIndex> euribor3m(new Euribor3M);
    shared_ptr<Eonia> eonia(new Eonia);

    for (Size i = 0; i < LENGTH(depositData); i++) {
        Real rate = 0.01 * depositData[i].rate;
        shared_ptr<SimpleQuote> simple = ext::make_shared<SimpleQuote>(rate);
        shared_ptr<Quote> quote (simple);
        Period term = depositData[i].n * depositData[i].unit;
        shared_ptr<RateHelper> helper(new
                    DepositRateHelper(Handle<Quote>(quote),
                                      term,
                                      depositData[i].settlementDays,
                                      euribor3m->fixingCalendar(),
                                      euribor3m->businessDayConvention(),
                                      euribor3m->endOfMonth(),
                                      euribor3m->dayCounter()));

        if (term <= 2*Days)
            eoniaHelpers.push_back(helper);
    }

    for (Size i = 0; i < LENGTH(eoniaSwapData); i++) {
        Real rate = 0.01 * eoniaSwapData[i].rate;
        shared_ptr<SimpleQuote> simple = ext::make_shared<SimpleQuote>(rate);
        shared_ptr<Quote> quote (simple);
        Period term = eoniaSwapData[i].n * eoniaSwapData[i].unit;
        shared_ptr<RateHelper> helper(new
                     OISRateHelper(eoniaSwapData[i].settlementDays,
                                   term,
                                   Handle<Quote>(quote),
                                   eonia,
                                   Handle<YieldTermStructure>(),
                                   telescopicValueDates,
                                   paymentLag));
        eoniaHelpers.push_back(helper);
    }

    shared_ptr<PiecewiseFlatForward> eoniaTS(
        new PiecewiseFlatForward (vars.today, eoniaHelpers, Actual365Fixed()));

    vars.eoniaTermStructure.linkTo(eoniaTS);

    // test curve consistency
    Real tolerance = 1.0e-8;
    for (Size i = 0; i < LENGTH(eoniaSwapData); i++) {
        Rate expected = eoniaSwapData[i].rate/100;
        Period term = eoniaSwapData[i].n * eoniaSwapData[i].unit;
        // test telescopic value dates (in bootstrap) against non telescopic value dates (swap here)
        shared_ptr<OvernightIndexedSwap> swap = vars.makeSwap(term, 0.0, 0.0, false,
                                                              Null<Date>(), paymentLag);
        Rate calculated = swap->fairRate();
        Rate error = std::fabs(expected-calculated);

        if (error>tolerance)
            BOOST_FAIL("curve inconsistency:" << std::setprecision(10) <<
                        "\n swap length:     " << term <<
                        "\n quoted rate:     " << expected <<
                        "\n calculated rate: " << calculated <<
                        "\n error:           " << error <<
                        "\n tolerance:       " << tolerance);
    }
} // testBootstrap(telescopicValueDates)
} // anonymous namespace

void OvernightIndexedSwapTest::testBootstrap() {
    BOOST_TEST_MESSAGE("Testing Eonia-swap curve building...");
    ::testBootstrap(false);
}

void OvernightIndexedSwapTest::testBootstrapWithTelescopicDates() {
    BOOST_TEST_MESSAGE(
        "Testing Eonia-swap curve building with telescopic value dates...");
    ::testBootstrap(true);
}

void OvernightIndexedSwapTest::testSeasonedSwaps() {

    BOOST_TEST_MESSAGE("Testing seasoned Eonia-swap calculation...");

    CommonVars vars;

    Period lengths[] = { 1*Years, 2*Years, 5*Years, 10*Years, 20*Years };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };

    Date effectiveDate = Date(2, February, 2009);

    vars.eoniaIndex->addFixing(Date(2,February,2009), 0.0010); // fake fixing values
    vars.eoniaIndex->addFixing(Date(3,February,2009), 0.0011);
    vars.eoniaIndex->addFixing(Date(4,February,2009), 0.0012);
    vars.eoniaIndex->addFixing(Date(5,February,2009), 0.0013);

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(spreads); j++) {

            shared_ptr<OvernightIndexedSwap> swap =
                vars.makeSwap(lengths[i],0.0,spreads[j],false,effectiveDate);
            shared_ptr<OvernightIndexedSwap> swap2 =
                vars.makeSwap(lengths[i],0.0,spreads[j],true,effectiveDate);
            if (std::fabs(swap->NPV() - swap2->NPV()) > 1.0e-10) {
                BOOST_ERROR("swap npv is different:\n"
                            << std::setprecision(2)
                            << "    length: " << lengths[i] << " \n"
                            << "    floating spread: "
                            << io::rate(spreads[j]) << "\n"
                            << "    swap value (non telescopic value dates): " << swap->NPV()
                            << "\n    swap value (telescopic value dates    ): " << swap2->NPV());
            }
        }
    }
}


test_suite* OvernightIndexedSwapTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Overnight-indexed swap tests");
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testFairRate));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testFairSpread));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testCachedValue));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testBootstrap));
    suite->add(QUANTLIB_TEST_CASE(
        &OvernightIndexedSwapTest::testBootstrapWithTelescopicDates));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testSeasonedSwaps));
    return suite;
}
