/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters

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

using boost::shared_ptr;

typedef PiecewiseYieldCurve<Discount,LogLinear> PiecewiseFlatForward;

namespace {

    struct Datum {
        Integer settlementDays;
        Integer n;
        TimeUnit unit;
        Rate rate;
    };

    struct FraDatum {
        Integer settlementDays;
        Integer nExpiry;
        Integer nMaturity;
        //        TimeUnit units;
        Rate rate;
    };

    struct SwapDatum {
        Integer settlementDays;
        Integer nIndexUnits;
        TimeUnit indexUnit;
        Integer nTermUnits;
        TimeUnit termUnit;
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

    FraDatum fraData[] = {
        { 2, 3, 6, 1.728 },
        { 2, 6, 9, 1.702 }
     };

    SwapDatum swapData[] = {
        { 2, 3, Months,  1, Years, 1.867 },
        { 2, 3, Months, 15, Months, 1.879 },
        { 2, 3, Months, 18, Months, 1.934 },
        { 2, 3, Months, 21, Months, 2.005 },
        { 2, 3, Months,  2, Years, 2.091 },
        { 2, 3, Months,  3, Years, 2.435 },
        { 2, 3, Months,  4, Years, 2.733 },
        { 2, 3, Months,  5, Years, 2.971 },
        { 2, 3, Months,  6, Years, 3.174 },
        { 2, 3, Months,  7, Years, 3.345 },
        { 2, 3, Months,  8, Years, 3.491 },
        { 2, 3, Months,  9, Years, 3.620 },
        { 2, 3, Months, 10, Years, 3.733 },
        { 2, 3, Months, 12, Years, 3.910 },
        { 2, 3, Months, 15, Years, 4.052 },
        { 2, 3, Months, 20, Years, 4.073 },
        { 2, 3, Months, 25, Years, 3.844 },
        { 2, 3, Months, 30, Years, 3.687 }
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
                                                  Spread spread) {
            return MakeOIS(length, eoniaIndex, fixedRate)
                .withEffectiveDate(settlement)
                .withOvernightLegSpread(spread)
                .withNominal(nominal)
                .withDiscountingTermStructure(eoniaTermStructure);
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
            eoniaIndex = shared_ptr<Eonia>(new Eonia(eoniaTermStructure));
            fixedSwapConvention = ModifiedFollowing;
            fixedSwapFrequency = Annual;
            fixedSwapDayCount = Thirty360();
            swapIndex = shared_ptr<IborIndex>(new Euribor3M(swapTermStructure));
            calendar = eoniaIndex->fixingCalendar();
            today = Date(5, February, 2009);
            //today = calendar.adjust(Date::todaysDate());
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today,settlementDays*Days,Following);
            eoniaTermStructure.linkTo(flatRate(settlement, 0.05,
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
                vars.makeSwap(lengths[i],0.0,spreads[j]);
            swap = vars.makeSwap(lengths[i],swap->fairRate(),spreads[j]);
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("recalculating with implied rate:\n"
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
                vars.makeSwap(lengths[i], rates[j], 0.0);
            Spread fairSpread = swap->fairSpread();
            swap = vars.makeSwap(lengths[i], rates[j], fairSpread);

            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("\nrecalculating with implied spread:" <<
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
    shared_ptr<OvernightIndexedSwap> swap = vars.makeSwap(1*Years, fixedRate, 0.0);
    Real cachedNPV   = 0.001730450147;
    Real tolerance = 1.0e-11;
    if (std::fabs(swap->NPV()-cachedNPV) > tolerance)
        BOOST_ERROR("\nfailed to reproduce cached swap value:" <<
                    QL_FIXED << std::setprecision(12) <<
                    "\ncalculated: " << swap->NPV() <<
                    "\n  expected: " << cachedNPV <<
                    "\n tolerance:" << tolerance);
}


void OvernightIndexedSwapTest::testBootstrap() {

    BOOST_TEST_MESSAGE("Testing Eonia-swap curve building...");

    CommonVars vars;

    std::vector<shared_ptr<RateHelper> > eoniaHelpers;
    std::vector<shared_ptr<RateHelper> > swap3mHelpers;

    shared_ptr<IborIndex> euribor3m(new Euribor3M);
    shared_ptr<Eonia> eonia(new Eonia);

    for (Size i = 0; i < LENGTH(depositData); i++) {
        Real rate = 0.01 * depositData[i].rate;
        shared_ptr<SimpleQuote> simple = shared_ptr<SimpleQuote>(new SimpleQuote(rate));
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
        if (term <= 3*Months)
            swap3mHelpers.push_back(helper);
    }

    for (Size i = 0; i < LENGTH(fraData); i++) {
        Real rate = 0.01 * fraData[i].rate;
        shared_ptr<SimpleQuote> simple = shared_ptr<SimpleQuote>(new SimpleQuote(rate));
        shared_ptr<Quote> quote (simple);
        shared_ptr<RateHelper> helper(new
                               FraRateHelper(Handle<Quote>(quote),
                                             fraData[i].nExpiry,
                                             fraData[i].nMaturity,
                                             fraData[i].settlementDays,
                                             euribor3m->fixingCalendar(),
                                             euribor3m->businessDayConvention(),
                                             euribor3m->endOfMonth(),
                                             euribor3m->dayCounter()));
        swap3mHelpers.push_back(helper);
    }

    for (Size i = 0; i < LENGTH(eoniaSwapData); i++) {
        Real rate = 0.01 * eoniaSwapData[i].rate;
        shared_ptr<SimpleQuote> simple = shared_ptr<SimpleQuote>(new SimpleQuote(rate));
        shared_ptr<Quote> quote (simple);
        Period term = eoniaSwapData[i].n * eoniaSwapData[i].unit;
        shared_ptr<RateHelper> helper(new
                     OISRateHelper(eoniaSwapData[i].settlementDays,
                                   term,
                                   Handle<Quote>(quote),
                                   eonia));
        eoniaHelpers.push_back(helper);
    }

    for (Size i = 0; i < LENGTH(swapData); i++) {
        Real rate = 0.01 * swapData[i].rate;
        shared_ptr<SimpleQuote> simple = shared_ptr<SimpleQuote>(new SimpleQuote(rate));
        shared_ptr<Quote> quote (simple);
        Period tenor = swapData[i].nIndexUnits * swapData[i].indexUnit;
        Period term = swapData[i].nTermUnits * swapData[i].termUnit;
        shared_ptr<RateHelper> helper(new SwapRateHelper(
                               Handle<Quote>(quote),
                               term,
                               vars.calendar,
                               vars.fixedSwapFrequency,
                               vars.fixedSwapConvention,
                               vars.fixedSwapDayCount,
                               euribor3m));
        if (tenor == 3*Months)
            swap3mHelpers.push_back(helper);
    }

    shared_ptr<PiecewiseFlatForward> eoniaTS(new
        PiecewiseFlatForward (vars.today, eoniaHelpers, Actual365Fixed()));

    shared_ptr<PiecewiseFlatForward> swapTS(new
        PiecewiseFlatForward (vars.today, swap3mHelpers, Actual365Fixed()));

    vars.eoniaTermStructure.linkTo(eoniaTS);

    /*
    std::cout.setf (std::ios::fixed, std::ios::floatfield);
    std::cout.setf (std::ios::showpoint);
    */

    // test curve consistency
    Real tolerance = 1.0e-8;
    for (Size i = 0; i < LENGTH(eoniaSwapData); i++) {
        Rate expected = eoniaSwapData[i].rate;
        Period term = eoniaSwapData[i].n * eoniaSwapData[i].unit;
        shared_ptr<OvernightIndexedSwap> swap = vars.makeSwap(term, 0.0, 0.0);
        Rate calculated = 100.0 * swap->fairRate();
        Rate error = std::fabs(expected-calculated);

        if (error>tolerance)
            BOOST_FAIL("curve inconsistency:" << std::setprecision(10) <<
                        "\n swap length:     " << term <<
                        "\n quoted rate:     " << expected <<
                        "\n calculated rate: " << calculated <<
                        "\n error:           " << error <<
                        "\n tolerance:       " << tolerance);
    }

    // zero spread
    /*
    std::cout << "zero spread:" << std::endl;
    std::cout << "years date zero3m/% zero1d/% spread/bp" << std::endl;
    DayCounter dc = Actual365Fixed();
    for (Size i = 1; i <= 10; i++) {
        Date d = vars.today + i*Years;
        Rate zero1d = eoniaTS->zeroRate(d, dc, Continuous, Annual,false).rate();
        Rate zero3m = swapTS->zeroRate(d, dc, Continuous, Annual,false).rate();
        std::cout << std::setw(2) << i << "y  " << io::iso_date(d) << "  "
                  << std::setprecision(3)
                  << zero3m * 100.0 << " "
                  << zero1d * 100.0 << " "
                  << std::setprecision(1)
                  << (zero3m - zero1d) * 10000.0 << std::endl;
    }
    */
}


test_suite* OvernightIndexedSwapTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Overnight-indexed swap tests");
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testFairRate));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testFairSpread));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testCachedValue));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testBootstrap));
    return suite;
}

