
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "swap.hpp"
#include "utilities.hpp"
#include <ql/Instruments/simpleswap.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/DayCounters/actual365.hpp>
#include <ql/Indexes/euribor.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    // global data

    Date today_, settlement_;
    bool payFixed_;
    Real nominal_;
    Calendar calendar_;
    BusinessDayConvention fixedConvention_, floatingConvention_;
    Frequency fixedFrequency_, floatingFrequency_;
    DayCounter fixedDayCount_;
    boost::shared_ptr<Xibor> index_;
    Integer settlementDays_, fixingDays_;
    Handle<TermStructure> termStructure_;

    // utilities

    boost::shared_ptr<SimpleSwap> makeSwap(Integer length, Rate fixedRate,
                                           Spread floatingSpread) {
        Date maturity = calendar_.advance(settlement_,length,Years,
                                          floatingConvention_);
        Schedule fixedSchedule(calendar_,settlement_,maturity,
                               fixedFrequency_,fixedConvention_);
        Schedule floatSchedule(calendar_,settlement_,maturity,
                               floatingFrequency_,floatingConvention_);
        return boost::shared_ptr<SimpleSwap>(
            new SimpleSwap(payFixed_,nominal_,
                           fixedSchedule,fixedRate,fixedDayCount_,
                           floatSchedule,index_,fixingDays_,floatingSpread,
                           termStructure_));
    }

    void initialize() {
        payFixed_ = true;
        settlementDays_ = 2;
        fixingDays_ = 2;
        nominal_ = 100.0;
        fixedConvention_ = Unadjusted;
        floatingConvention_ = ModifiedFollowing;
        fixedFrequency_ = Annual;
        floatingFrequency_ = Semiannual;
        fixedDayCount_ = Thirty360();
        index_ = boost::shared_ptr<Xibor>(
                                     new Euribor(12/floatingFrequency_,Months,
                                                 termStructure_));
        calendar_ = index_->calendar();
        today_ = calendar_.adjust(Date::todaysDate());
        Settings::instance().setEvaluationDate(today_);
        settlement_ = calendar_.advance(today_,settlementDays_,Days);
        termStructure_.linkTo(flatRate(settlement_,0.05,Actual365()));
    }

    void finalize() {
        Settings::instance().setEvaluationDate(Date());
    }

}

void SwapTest::testFairRate() {

    BOOST_MESSAGE("Testing simple swap calculation of fair fixed rate...");

    initialize();

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(spreads); j++) {

            boost::shared_ptr<SimpleSwap> swap =
                makeSwap(lengths[i],0.0,spreads[j]);
            swap = makeSwap(lengths[i],swap->fairRate(),spreads[j]);
            if (QL_FABS(swap->NPV()) > 1.0e-10) {
                BOOST_FAIL(
                    "recalculating with implied rate:\n"
                    "    length: " +
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    floating spread: " +
                    RateFormatter::toString(spreads[j],2) + "\n"
                    "    swap value: " +
                    DecimalFormatter::toString(swap->NPV()));
            }
        }
    }

    finalize();
}

void SwapTest::testFairSpread() {

    BOOST_MESSAGE("Testing simple swap calculation of "
                  "fair floating spread...");

    initialize();

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Rate rates[] = { 0.04, 0.05, 0.06, 0.07 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(rates); j++) {

            boost::shared_ptr<SimpleSwap> swap =
                makeSwap(lengths[i],rates[j],0.0);
            swap = makeSwap(lengths[i],rates[j],swap->fairSpread());
            if (QL_FABS(swap->NPV()) > 1.0e-10) {
                BOOST_FAIL(
                    "recalculating with implied spread:\n"
                    "    length: " +
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    fixed rate: " +
                    RateFormatter::toString(rates[j],2) + "\n"
                    "    swap value: " +
                    DecimalFormatter::toString(swap->NPV()));
            }
        }
    }

    finalize();
}

void SwapTest::testRateDependency() {

    BOOST_MESSAGE("Testing simple swap dependency on fixed rate...");

    initialize();

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };
    Rate rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(spreads); j++) {
            // store the results for different rates...
            std::vector<Real> swap_values;
            for (Size k=0; k<LENGTH(rates); k++) {
                boost::shared_ptr<SimpleSwap> swap =
                    makeSwap(lengths[i],rates[k],spreads[j]);
                swap_values.push_back(swap->NPV());
            }
            // and check that they go the right way
            std::vector<Real>::iterator it =
                std::adjacent_find(swap_values.begin(),swap_values.end(),
                                   std::less<Real>());
            if (it != swap_values.end()) {
                Size n = it - swap_values.begin();
                BOOST_FAIL(
                    "NPV is increasing with the fixed rate in a swap: \n"
                    "    length: " +
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    value: " +
                    DecimalFormatter::toString(swap_values[n]) +
                    " paying fixed rate: " +
                    RateFormatter::toString(rates[n],2) + "\n"
                    "    value: " +
                    DecimalFormatter::toString(swap_values[n+1]) +
                    " paying fixed rate: " +
                    RateFormatter::toString(rates[n+1],2));
            }
        }
    }

    finalize();
}

void SwapTest::testSpreadDependency() {

    BOOST_MESSAGE("Testing simple swap dependency on floating spread...");

    initialize();

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Rate rates[] = { 0.04, 0.05, 0.06, 0.07 };
    Spread spreads[] = { -0.01, -0.002, -0.001, 0.0, 0.001, 0.002, 0.01 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(rates); j++) {
            // store the results for different spreads...
            std::vector<Real> swap_values;
            for (Size k=0; k<LENGTH(spreads); k++) {
                boost::shared_ptr<SimpleSwap> swap =
                    makeSwap(lengths[i],rates[j],spreads[k]);
                swap_values.push_back(swap->NPV());
            }
            // and check that they go the right way
            std::vector<Real>::iterator it =
                std::adjacent_find(swap_values.begin(),swap_values.end(),
                                   std::greater<Real>());
            if (it != swap_values.end()) {
                Size n = it - swap_values.begin();
                BOOST_FAIL(
                    "NPV is decreasing with the floating spread in a swap: \n"
                    "    length: " +
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    value: " +
                    DecimalFormatter::toString(swap_values[n]) +
                    " receiving spread: " +
                    RateFormatter::toString(spreads[n],2) + "\n"
                    "    value: " +
                    DecimalFormatter::toString(swap_values[n+1]) +
                    " receiving spread: " +
                    RateFormatter::toString(spreads[n+1],2));
            }
        }
    }

    finalize();
}

void SwapTest::testCachedValue() {

    BOOST_MESSAGE("Testing simple swap calculation against cached value...");

    initialize();

    today_ = Date(17,June,2002);
    Settings::instance().setEvaluationDate(today_);
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(flatRate(settlement_,0.05,Actual365()));

    boost::shared_ptr<SimpleSwap> swap = makeSwap(10, 0.06, 0.001);
#ifndef QL_USE_INDEXED_COUPON
    Real cachedNPV   = -5.872863313209;
#else
    Real cachedNPV   = -5.872342992212;
#endif

    if (QL_FABS(swap->NPV()-cachedNPV) > 1.0e-11)
        BOOST_FAIL(
            "failed to reproduce cached swap value:\n"
            "    calculated: " +
            DecimalFormatter::toString(swap->NPV(),12) + "\n"
            "    expected:   " +
            DecimalFormatter::toString(cachedNPV,12));

    finalize();
}


test_suite* SwapTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Swap tests");
    suite->add(BOOST_TEST_CASE(&SwapTest::testFairRate));
    suite->add(BOOST_TEST_CASE(&SwapTest::testFairSpread));
    suite->add(BOOST_TEST_CASE(&SwapTest::testRateDependency));
    suite->add(BOOST_TEST_CASE(&SwapTest::testSpreadDependency));
    suite->add(BOOST_TEST_CASE(&SwapTest::testCachedValue));
    return suite;
}

