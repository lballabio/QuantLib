/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2012 StatPro Italia srl

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

#include "cashflows.hpp"
#include "utilities.hpp"
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/settings.hpp>

using namespace QuantLib;
using namespace boost;
using namespace boost::unit_test_framework;

void CashFlowsTest::testSettings() {

    BOOST_TEST_MESSAGE("Testing cash-flow settings...");

    SavedSettings backup;

    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    // cash flows at T+0, T+1, T+2
    std::vector<shared_ptr<CashFlow> > leg;
    for (Integer i=0; i<3; ++i)
        leg.push_back(shared_ptr<CashFlow>(new SimpleCashFlow(1.0, today+i)));


    #define CHECK_INCLUSION(n, days, expected) \
    if ((!leg[n]->hasOccurred(today+days)) != expected) { \
        BOOST_ERROR("cashflow at T+" << n << " " \
                    << (expected ? "not" : "") << "included" \
                    << " at T+" << days); \
    }

    // case 1: don't include reference-date payments, no override at
    //         today's date

    Settings::instance().includeReferenceDateEvents() = false;
    Settings::instance().includeTodaysCashFlows() = none;

    CHECK_INCLUSION(0, 0, false);
    CHECK_INCLUSION(0, 1, false);

    CHECK_INCLUSION(1, 0, true);
    CHECK_INCLUSION(1, 1, false);
    CHECK_INCLUSION(1, 2, false);

    CHECK_INCLUSION(2, 1, true);
    CHECK_INCLUSION(2, 2, false);
    CHECK_INCLUSION(2, 3, false);

    // case 2: same, but with explicit setting at today's date

    Settings::instance().includeReferenceDateEvents() = false;
    Settings::instance().includeTodaysCashFlows() = false;

    CHECK_INCLUSION(0, 0, false);
    CHECK_INCLUSION(0, 1, false);

    CHECK_INCLUSION(1, 0, true);
    CHECK_INCLUSION(1, 1, false);
    CHECK_INCLUSION(1, 2, false);

    CHECK_INCLUSION(2, 1, true);
    CHECK_INCLUSION(2, 2, false);
    CHECK_INCLUSION(2, 3, false);

    // case 3: do include reference-date payments, no override at
    //         today's date

    Settings::instance().includeReferenceDateEvents() = true;
    Settings::instance().includeTodaysCashFlows() = none;

    CHECK_INCLUSION(0, 0, true);
    CHECK_INCLUSION(0, 1, false);

    CHECK_INCLUSION(1, 0, true);
    CHECK_INCLUSION(1, 1, true);
    CHECK_INCLUSION(1, 2, false);

    CHECK_INCLUSION(2, 1, true);
    CHECK_INCLUSION(2, 2, true);
    CHECK_INCLUSION(2, 3, false);

    // case 4: do include reference-date payments, explicit (and same)
    //         setting at today's date

    Settings::instance().includeReferenceDateEvents() = true;
    Settings::instance().includeTodaysCashFlows() = true;

    CHECK_INCLUSION(0, 0, true);
    CHECK_INCLUSION(0, 1, false);

    CHECK_INCLUSION(1, 0, true);
    CHECK_INCLUSION(1, 1, true);
    CHECK_INCLUSION(1, 2, false);

    CHECK_INCLUSION(2, 1, true);
    CHECK_INCLUSION(2, 2, true);
    CHECK_INCLUSION(2, 3, false);

    // case 5: do include reference-date payments, override at
    //         today's date

    Settings::instance().includeReferenceDateEvents() = true;
    Settings::instance().includeTodaysCashFlows() = false;

    CHECK_INCLUSION(0, 0, false);
    CHECK_INCLUSION(0, 1, false);

    CHECK_INCLUSION(1, 0, true);
    CHECK_INCLUSION(1, 1, true);
    CHECK_INCLUSION(1, 2, false);

    CHECK_INCLUSION(2, 1, true);
    CHECK_INCLUSION(2, 2, true);
    CHECK_INCLUSION(2, 3, false);


    // no discount to make calculations easier
    InterestRate no_discount(0.0, Actual365Fixed(), Continuous, Annual);

    #define CHECK_NPV(includeRef, expected)                             \
    do {                                                            \
        Real NPV = CashFlows::npv(leg, no_discount, includeRef, today); \
        if (std::fabs(NPV - expected) > 1e-6) {                         \
            BOOST_ERROR("NPV mismatch:\n"                               \
                        << "    calculated: " << NPV << "\n"            \
                        << "    expected: " << expected);               \
        }                                                               \
    } while (false);

    // no override
    Settings::instance().includeTodaysCashFlows() = none;

    CHECK_NPV(false, 2.0);
    CHECK_NPV(true, 3.0);
    
    // override
    Settings::instance().includeTodaysCashFlows() = false;
    
    CHECK_NPV(false, 2.0);
    CHECK_NPV(true, 2.0);

}

void CashFlowsTest::testAccessViolation() {
    BOOST_TEST_MESSAGE("Testing dynamic cast of coupon in Black pricer...");

    SavedSettings backup;

    Date todaysDate(7, April, 2010);
    Date settlementDate(9, April, 2010);
    Settings::instance().evaluationDate() = todaysDate;
    Calendar calendar = TARGET();

    Handle<YieldTermStructure> rhTermStructure(
        flatRate(settlementDate, 0.04875825, Actual365Fixed()));

    Volatility volatility = 0.10;
    Handle<OptionletVolatilityStructure> vol;
    vol = Handle<OptionletVolatilityStructure>(
             boost::shared_ptr<OptionletVolatilityStructure>(
                 new ConstantOptionletVolatility(
                             2,
                             calendar,
                             ModifiedFollowing,
                             volatility,
                             Actual365Fixed())));

    boost::shared_ptr<IborIndex> index3m (new USDLibor(3*Months,
                                                       rhTermStructure));

    Date payDate(20, December, 2013);
    Date startDate(20, September, 2013);
    Date endDate(20, December, 2013);
    Rate spread = 0.0115;
    boost::shared_ptr<IborCouponPricer> pricer(new BlackIborCouponPricer(vol));
    boost::shared_ptr<FloatingRateCoupon> coupon(
        new FloatingRateCoupon(payDate,100, startDate, endDate, 2,
                               index3m, 1.0 , spread / 100));
    coupon->setPricer(pricer);

    try {
        // this caused an access violation in version 1.0
        coupon->amount();
    } catch (Error&) {
        // ok; proper exception thrown
    }
}

void CashFlowsTest::testDefaultSettlementDate() {
    BOOST_TEST_MESSAGE("Testing default evaluation date in cashflows methods...");
    Date today = Settings::instance().evaluationDate();
    Schedule schedule =
        MakeSchedule()
        .from(today-2*Months).to(today+4*Months)
        .withFrequency(Semiannual)
        .withCalendar(TARGET())
        .withConvention(Unadjusted)
        .backwards();

    Leg leg = FixedRateLeg(schedule)
              .withNotionals(100.0)
              .withCouponRates(0.03, Actual360())
              .withPaymentCalendar(TARGET())
              .withPaymentAdjustment(Following);

    Time accruedPeriod = CashFlows::accruedPeriod(leg, false);
    if (accruedPeriod == 0.0)
        BOOST_ERROR("null accrued period with default settlement date");

    BigInteger accruedDays = CashFlows::accruedDays(leg, false);
    if (accruedDays == 0)
        BOOST_ERROR("no accrued days with default settlement date");

    Real accruedAmount = CashFlows::accruedAmount(leg, false);
    if (accruedAmount == 0.0)
        BOOST_ERROR("null accrued amount with default settlement date");
}

void CashFlowsTest::testNullFixingDays() {
    BOOST_TEST_MESSAGE("Testing ibor leg construction with null fixing days...");
    Date today = Settings::instance().evaluationDate();
    Schedule schedule =
        MakeSchedule()
        .from(today-2*Months).to(today+4*Months)
        .withFrequency(Semiannual)
        .withCalendar(TARGET())
        .withConvention(Following)
        .backwards();

    boost::shared_ptr<IborIndex> index(new USDLibor(6*Months));
    Leg leg = IborLeg(schedule, index)
        .withNotionals(100.0)
        // this can happen with default values, and caused an
        // exception when the null was not managed properly
        .withFixingDays(Null<Natural>());
}

test_suite* CashFlowsTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Cash flows tests");
    suite->add(QUANTLIB_TEST_CASE(&CashFlowsTest::testSettings));
    suite->add(QUANTLIB_TEST_CASE(&CashFlowsTest::testAccessViolation));
    suite->add(QUANTLIB_TEST_CASE(&CashFlowsTest::testDefaultSettlementDate));
    #ifndef QL_USE_INDEXED_COUPON
    suite->add(QUANTLIB_TEST_CASE(&CashFlowsTest::testNullFixingDays));
    #endif
    return suite;
}

