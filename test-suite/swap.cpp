/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl

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

#include "swap.hpp"
#include "utilities.hpp"
#include <ql/instruments/vanillaswap.hpp>
#include <ql/yieldtermstructures/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/voltermstructures/interestrate/caplet/capletconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/currencies/europe.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(SwapTest)

// global data

Date today_, settlement_;
VanillaSwap::Type type_;
Real nominal_;
Calendar calendar_;
BusinessDayConvention fixedConvention_, floatingConvention_;
Frequency fixedFrequency_, floatingFrequency_;
DayCounter fixedDayCount_;
boost::shared_ptr<IborIndex> index_;
Natural settlementDays_;
RelinkableHandle<YieldTermStructure> termStructure_;

// utilities

boost::shared_ptr<VanillaSwap> makeSwap(Integer length, Rate fixedRate,
                                        Spread floatingSpread) {
    Date maturity = calendar_.advance(settlement_,length,Years,
                                      floatingConvention_);
    Schedule fixedSchedule(settlement_,maturity,Period(fixedFrequency_),
                           calendar_,fixedConvention_,fixedConvention_,
                           false,false);
    Schedule floatSchedule(settlement_,maturity,Period(floatingFrequency_),
                           calendar_,floatingConvention_,floatingConvention_,
                           false,false);
    return boost::shared_ptr<VanillaSwap>(new
        VanillaSwap(type_, nominal_,
                    fixedSchedule, fixedRate, fixedDayCount_,
                    floatSchedule, index_, floatingSpread,
                    index_->dayCounter(), termStructure_));
}

void setup() {
    type_ = VanillaSwap::Payer;
    settlementDays_ = 2;
    nominal_ = 100.0;
    fixedConvention_ = Unadjusted;
    floatingConvention_ = ModifiedFollowing;
    fixedFrequency_ = Annual;
    floatingFrequency_ = Semiannual;
    fixedDayCount_ = Thirty360();
    index_ = boost::shared_ptr<IborIndex>(new
        Euribor(Period(floatingFrequency_), termStructure_));
    calendar_ = index_->fixingCalendar();
    today_ = calendar_.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today_;
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(flatRate(settlement_,0.05,Actual365Fixed()));
}

QL_END_TEST_LOCALS(SwapTest)


void SwapTest::testFairRate() {

    BOOST_MESSAGE("Testing vanilla-swap calculation of fair fixed rate...");

    SavedSettings backup;

    setup();

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(spreads); j++) {

            boost::shared_ptr<VanillaSwap> swap =
                makeSwap(lengths[i],0.0,spreads[j]);
            swap = makeSwap(lengths[i],swap->fairRate(),spreads[j]);
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("recalculating with implied rate:\n"
                            << std::setprecision(2)
                            << "    length: " << lengths[i] << " years\n"
                            << "    floating spread: "
                            << io::rate(spreads[j]) << "\n"
                            << "    swap value: " << swap->NPV());
            }
        }
    }
}

void SwapTest::testFairSpread() {

    BOOST_MESSAGE("Testing vanilla-swap calculation of "
                  "fair floating spread...");

    SavedSettings backup;

    setup();

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Rate rates[] = { 0.04, 0.05, 0.06, 0.07 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(rates); j++) {

            boost::shared_ptr<VanillaSwap> swap =
                makeSwap(lengths[i],rates[j],0.0);
            swap = makeSwap(lengths[i],rates[j],swap->fairSpread());
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("recalculating with implied spread:\n"
                            << std::setprecision(2)
                            << "    length: " << lengths[i] << " years\n"
                            << "    fixed rate: " << io::rate(rates[j]) << "\n"
                            << "    swap value: " << swap->NPV());
            }
        }
    }
}

void SwapTest::testRateDependency() {

    BOOST_MESSAGE("Testing vanilla-swap dependency on fixed rate...");

    SavedSettings backup;

    setup();

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };
    Rate rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(spreads); j++) {
            // store the results for different rates...
            std::vector<Real> swap_values;
            for (Size k=0; k<LENGTH(rates); k++) {
                boost::shared_ptr<VanillaSwap> swap =
                    makeSwap(lengths[i],rates[k],spreads[j]);
                swap_values.push_back(swap->NPV());
            }
            // and check that they go the right way
            std::vector<Real>::iterator it =
                std::adjacent_find(swap_values.begin(),swap_values.end(),
                                   std::less<Real>());
            if (it != swap_values.end()) {
                Size n = it - swap_values.begin();
                BOOST_ERROR(
                    "NPV is increasing with the fixed rate in a swap: \n"
                    << "    length: " << lengths[i] << " years\n"
                    << "    value:  " << swap_values[n]
                    << " paying fixed rate: " << io::rate(rates[n]) << "\n"
                    << "    value:  " << swap_values[n+1]
                    << " paying fixed rate: " << io::rate(rates[n+1]));
            }
        }
    }
}

void SwapTest::testSpreadDependency() {

    BOOST_MESSAGE("Testing vanilla-swap dependency on floating spread...");

    SavedSettings backup;

    setup();

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Rate rates[] = { 0.04, 0.05, 0.06, 0.07 };
    Spread spreads[] = { -0.01, -0.002, -0.001, 0.0, 0.001, 0.002, 0.01 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(rates); j++) {
            // store the results for different spreads...
            std::vector<Real> swap_values;
            for (Size k=0; k<LENGTH(spreads); k++) {
                boost::shared_ptr<VanillaSwap> swap =
                    makeSwap(lengths[i],rates[j],spreads[k]);
                swap_values.push_back(swap->NPV());
            }
            // and check that they go the right way
            std::vector<Real>::iterator it =
                std::adjacent_find(swap_values.begin(),swap_values.end(),
                                   std::greater<Real>());
            if (it != swap_values.end()) {
                Size n = it - swap_values.begin();
                BOOST_ERROR(
                    "NPV is decreasing with the floating spread in a swap: \n"
                    << "    length: " << lengths[i] << " years\n"
                    << "    value:  " << swap_values[n]
                    << " receiving spread: " << io::rate(spreads[n]) << "\n"
                    << "    value:  " << swap_values[n+1]
                    << " receiving spread: " << io::rate(spreads[n+1]));
            }
        }
    }
}

void SwapTest::testInArrears() {

    BOOST_MESSAGE("Testing in-arrears swap calculation...");

    SavedSettings backup;

    setup();

    /* See Hull, 4th ed., page 550
       Note: the calculation in the book is wrong (work out the
       adjustment and you'll get 0.05 + 0.000115 T1)
    */

    Date maturity = today_ + 5*Years;
    Calendar calendar = NullCalendar();
    Schedule schedule(today_, maturity,Period(Annual),calendar,
                      Following,Following,false,false);
    DayCounter dayCounter = SimpleDayCounter();
    std::vector<Real> nominals(1, 100000000.0);
    boost::shared_ptr<IborIndex> index(new IborIndex("dummy", 1*Years, 0,
                                             EURCurrency(), calendar,
                                             Following, false, dayCounter,
                                             termStructure_));
    Rate oneYear = 0.05;
    Rate r = std::log(1.0+oneYear);
    termStructure_.linkTo(flatRate(today_,r,dayCounter));


    std::vector<Rate> coupons(1, oneYear);
    Leg fixedLeg = FixedRateLeg(nominals, schedule, coupons,
                                dayCounter, Following);

    std::vector<Real> gearings;
    std::vector<Rate> spreads;
    Natural fixingDays = 0;

    Volatility capletVolatility = 0.22;
    Handle<OptionletVolatilityStructure> vol(
        boost::shared_ptr<OptionletVolatilityStructure>(new
            CapletConstantVolatility(today_, capletVolatility, dayCounter)));
    boost::shared_ptr<IborCouponPricer> pricer(new
        BlackIborCouponPricer(vol));

    Leg floatingLeg = IborLeg(nominals,
                              schedule,
                              index,
                              dayCounter,
                              Following,
                              std::vector<Natural>(1,fixingDays),
                              gearings, spreads,
                              std::vector<Rate>(), std::vector<Rate>(),
                              true);
    setCouponPricer(floatingLeg, pricer);

    Swap swap(termStructure_,floatingLeg,fixedLeg);

    Decimal storedValue = -144813.0;
    Real tolerance = 1.0;

    if (std::fabs(swap.NPV()-storedValue) > tolerance)
        BOOST_ERROR("Wrong NPV calculation:\n"
                    << "    expected:   " << storedValue << "\n"
                    << "    calculated: " << swap.NPV());
}

void SwapTest::testCachedValue() {

    BOOST_MESSAGE("Testing vanilla-swap calculation against cached value...");

    SavedSettings backup;

    setup();

    today_ = Date(17,June,2002);
    Settings::instance().evaluationDate() = today_;
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(flatRate(settlement_,0.05,Actual365Fixed()));

    boost::shared_ptr<VanillaSwap> swap = makeSwap(10, 0.06, 0.001);
    #ifndef QL_USE_INDEXED_COUPON
    Real cachedNPV   = -5.872863313209;
    #else
    Real cachedNPV   = -5.872342992212;
    #endif

    if (std::fabs(swap->NPV()-cachedNPV) > 1.0e-11)
        BOOST_ERROR("failed to reproduce cached swap value:\n"
                    << QL_FIXED << std::setprecision(12)
                    << "    calculated: " << swap->NPV() << "\n"
                    << "    expected:   " << cachedNPV);
}


test_suite* SwapTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Swap tests");
    suite->add(BOOST_TEST_CASE(&SwapTest::testFairRate));
    suite->add(BOOST_TEST_CASE(&SwapTest::testFairSpread));
    suite->add(BOOST_TEST_CASE(&SwapTest::testRateDependency));
    suite->add(BOOST_TEST_CASE(&SwapTest::testSpreadDependency));
    suite->add(BOOST_TEST_CASE(&SwapTest::testInArrears));
    suite->add(BOOST_TEST_CASE(&SwapTest::testCachedValue));
    return suite;
}

