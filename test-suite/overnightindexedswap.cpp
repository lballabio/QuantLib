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
#include <ql/indexes/ibor/fedfunds.hpp>
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

namespace overnight_indexed_swap_test {

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
        Swap::Type type;
        Real nominal;
        Calendar calendar;
        Natural settlementDays;

        Period fixedEoniaPeriod, floatingEoniaPeriod;
        DayCounter fixedEoniaDayCount;
        BusinessDayConvention fixedEoniaConvention, floatingEoniaConvention;
        ext::shared_ptr<Eonia> eoniaIndex;
        RelinkableHandle<YieldTermStructure> eoniaTermStructure;

        Frequency fixedSwapFrequency;
        DayCounter fixedSwapDayCount;
        BusinessDayConvention fixedSwapConvention;
        ext::shared_ptr<IborIndex> swapIndex;
        RelinkableHandle<YieldTermStructure> swapTermStructure;

        // cleanup
        SavedSettings backup;

        // utilities
        ext::shared_ptr<OvernightIndexedSwap>
        makeSwap(Period length,
                 Rate fixedRate,
                 Spread spread,
                 bool telescopicValueDates,
                 Date effectiveDate = Null<Date>(),
                 Natural paymentLag = 0,
                 RateAveraging::Type averagingMethod = RateAveraging::Compound) {
            return MakeOIS(length, eoniaIndex, fixedRate, 0 * Days)
                .withEffectiveDate(effectiveDate == Null<Date>() ? settlement : effectiveDate)
                .withOvernightLegSpread(spread)
                .withNominal(nominal)
                .withPaymentLag(paymentLag)
                .withDiscountingTermStructure(eoniaTermStructure)
                .withTelescopicValueDates(telescopicValueDates)
                .withAveragingMethod(averagingMethod);
        }

        CommonVars() {
            type = Swap::Payer;
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
            fixedSwapDayCount = Thirty360(Thirty360::BondBasis);
            swapIndex = ext::shared_ptr<IborIndex>(new Euribor3M(swapTermStructure));
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

    using namespace overnight_indexed_swap_test;

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


void OvernightIndexedSwapTest::testFairSpread() {

    BOOST_TEST_MESSAGE("Testing Eonia-swap calculation of "
                       "fair floating spread...");

    using namespace overnight_indexed_swap_test;

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

void OvernightIndexedSwapTest::testCachedValue() {

    BOOST_TEST_MESSAGE("Testing Eonia-swap calculation against cached value...");

    using namespace overnight_indexed_swap_test;

    CommonVars vars;

    Settings::instance().evaluationDate() = vars.today;
    vars.settlement =
        vars.calendar.advance(vars.today,vars.settlementDays,Days);
    Real flat = 0.05;
    vars.eoniaTermStructure.linkTo(flatRate(vars.settlement,flat,Actual360()));
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

namespace overnight_indexed_swap_test {
    void testBootstrap(bool telescopicValueDates,
                       RateAveraging::Type averagingMethod,
                       Real tolerance = 1.0e-8) {

    CommonVars vars;

    Natural paymentLag = 2;

    std::vector<ext::shared_ptr<RateHelper> > eoniaHelpers;

    ext::shared_ptr<IborIndex> euribor3m(new Euribor3M);
    ext::shared_ptr<Eonia> eonia(new Eonia);

    for (auto& i : depositData) {
        Real rate = 0.01 * i.rate;
        ext::shared_ptr<SimpleQuote> simple = ext::make_shared<SimpleQuote>(rate);
        ext::shared_ptr<Quote> quote (simple);
        Period term = i.n * i.unit;
        ext::shared_ptr<RateHelper> helper(new DepositRateHelper(
            Handle<Quote>(quote), term, i.settlementDays, euribor3m->fixingCalendar(),
            euribor3m->businessDayConvention(), euribor3m->endOfMonth(), euribor3m->dayCounter()));

        if (term <= 2*Days)
            eoniaHelpers.push_back(helper);
    }

    for (auto& i : eoniaSwapData) {
        Real rate = 0.01 * i.rate;
        ext::shared_ptr<SimpleQuote> simple = ext::make_shared<SimpleQuote>(rate);
        ext::shared_ptr<Quote> quote (simple);
        Period term = i.n * i.unit;
        ext::shared_ptr<RateHelper> helper(new
                     OISRateHelper(i.settlementDays,
                                   term,
                                   Handle<Quote>(quote),
                                   eonia,
                                   Handle<YieldTermStructure>(),
                                   telescopicValueDates,
                                   paymentLag, 
                                   Following, 
                                   Annual, 
                                   Calendar(), 
                                   0 * Days, 
                                   0.0, 
                                   Pillar::LastRelevantDate, 
                                   Date(), 
                                   averagingMethod));
        eoniaHelpers.push_back(helper);
    }

    auto eoniaTS = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(vars.today, eoniaHelpers, Actual365Fixed());

    vars.eoniaTermStructure.linkTo(eoniaTS);

    // test curve consistency
    for (auto& i : eoniaSwapData) {
        Rate expected = i.rate / 100;
        Period term = i.n * i.unit;
        // test telescopic value dates (in bootstrap) against non telescopic value dates (swap here)
        ext::shared_ptr<OvernightIndexedSwap> swap =
            vars.makeSwap(term, 0.0, 0.0, false, Null<Date>(), paymentLag, averagingMethod);
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
    BOOST_TEST_MESSAGE("Testing Eonia-swap curve building with daily compounded ON rates...");
    overnight_indexed_swap_test::testBootstrap(false, RateAveraging::Compound);
}

void OvernightIndexedSwapTest::testBootstrapWithArithmeticAverage() {
    BOOST_TEST_MESSAGE("Testing Eonia-swap curve building with arithmetic average ON rates...");
    overnight_indexed_swap_test::testBootstrap(false, RateAveraging::Simple);
}

void OvernightIndexedSwapTest::testBootstrapWithTelescopicDates() {
    BOOST_TEST_MESSAGE(
        "Testing Eonia-swap curve building with telescopic value dates and DCON rates...");
    overnight_indexed_swap_test::testBootstrap(true, RateAveraging::Compound);
}

void OvernightIndexedSwapTest::testBootstrapWithTelescopicDatesAndArithmeticAverage() {
    BOOST_TEST_MESSAGE(
        "Testing Eonia-swap curve building with telescopic value dates and AAON rates...");
    // Given that we are using an approximation that omits
    // the required convexity correction, a lower tolerance
    // is needed.
    overnight_indexed_swap_test::testBootstrap(true, RateAveraging::Simple, 1.0e-5);
}

void OvernightIndexedSwapTest::testSeasonedSwaps() {

    BOOST_TEST_MESSAGE("Testing seasoned Eonia-swap calculation...");

    using namespace overnight_indexed_swap_test;

    CommonVars vars;

    Period lengths[] = { 1*Years, 2*Years, 5*Years, 10*Years, 20*Years };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };

    Date effectiveDate = Date(2, February, 2009);

    vars.eoniaIndex->addFixing(Date(2,February,2009), 0.0010); // fake fixing values
    vars.eoniaIndex->addFixing(Date(3,February,2009), 0.0011);
    vars.eoniaIndex->addFixing(Date(4,February,2009), 0.0012);
    vars.eoniaIndex->addFixing(Date(5,February,2009), 0.0013);

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


void OvernightIndexedSwapTest::testBootstrapRegression() {
    BOOST_TEST_MESSAGE("Testing 1.16 regression with OIS bootstrap...");

    using namespace overnight_indexed_swap_test;

    SavedSettings backup;

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
    ext::shared_ptr<FedFunds> index(new FedFunds);

    helpers.push_back(
        ext::make_shared<DepositRateHelper>(data[0].rate,
                                            Period(data[0].n, data[0].unit),
                                            index->fixingDays(),
                                            index->fixingCalendar(),
                                            index->businessDayConvention(),
                                            index->endOfMonth(),
                                            index->dayCounter()));

    for (Size i=1; i<LENGTH(data); ++i) {
        helpers.push_back(
            ext::shared_ptr<RateHelper>(
                new OISRateHelper(data[i].settlementDays,
                                  Period(data[i].n, data[i].unit),
                                  Handle<Quote>(ext::make_shared<SimpleQuote>(data[i].rate)),
                                  index,
                                  Handle<YieldTermStructure>(),
                                  false, 2,
                                  Following, Annual, Calendar(), 0*Days, 0.0,
                                  // this bootstrap fails with the default LastRelevantDate choice
                                  Pillar::MaturityDate)));
    }

    PiecewiseYieldCurve<Discount,LogCubic> curve(0, UnitedStates(UnitedStates::GovernmentBond),
                                                 helpers, Actual365Fixed(), MonotonicLogCubic());

    BOOST_CHECK_NO_THROW(curve.discount(1.0));
}


test_suite* OvernightIndexedSwapTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Overnight-indexed swap tests");
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testFairRate));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testFairSpread));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testCachedValue));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testBootstrap));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testBootstrapWithArithmeticAverage));
    suite->add(QUANTLIB_TEST_CASE(
        &OvernightIndexedSwapTest::testBootstrapWithTelescopicDates));
    suite->add(QUANTLIB_TEST_CASE(
        &OvernightIndexedSwapTest::testBootstrapWithTelescopicDatesAndArithmeticAverage));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testSeasonedSwaps));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedSwapTest::testBootstrapRegression));
    return suite;
}
