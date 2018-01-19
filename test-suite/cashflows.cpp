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
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/settings.hpp>

#include <boost/make_shared.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using boost::shared_ptr;
using boost::none;

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

    Date::serial_type accruedDays = CashFlows::accruedDays(leg, false);
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

void CashFlowsTest::testIrregularFirstCouponReferenceDatesAtEndOfMonth() {
    BOOST_TEST_MESSAGE("Testing irregular first coupon reference dates with end of month enabled...");
    Schedule schedule =
        MakeSchedule()
        .from(Date(17, January, 2017)).to(Date(28, February, 2018))
        .withFrequency(Semiannual)
        .withConvention(Unadjusted)
        .endOfMonth()
        .backwards();

    Leg leg = FixedRateLeg(schedule)
        .withNotionals(100.0)
        .withCouponRates(0.01, Actual360());

    boost::shared_ptr<Coupon> firstCoupon =
        boost::dynamic_pointer_cast<Coupon>(leg.front());

    if (firstCoupon->referencePeriodStart() != Date(31, August, 2016))
        BOOST_ERROR("Expected reference start date at end of month, "
                    "got " << firstCoupon->referencePeriodStart());
}

void CashFlowsTest::testIrregularLastCouponReferenceDatesAtEndOfMonth() {
    BOOST_TEST_MESSAGE("Testing irregular last coupon reference dates with end of month enabled...");
    Schedule schedule =
            MakeSchedule()
                    .from(Date(17, January, 2017)).to(Date(15, September, 2018))
                    .withNextToLastDate(Date(28, February, 2018))
                    .withFrequency(Semiannual)
                    .withConvention(Unadjusted)
                    .endOfMonth()
                    .backwards();

    Leg leg = FixedRateLeg(schedule)
            .withNotionals(100.0)
            .withCouponRates(0.01, Actual360());

    boost::shared_ptr<Coupon> lastCoupon =
            boost::dynamic_pointer_cast<Coupon>(leg.back());

    if (lastCoupon->referencePeriodEnd() != Date(31, August, 2018))
        BOOST_ERROR("Expected reference end date at end of month, "
                            "got " << lastCoupon->referencePeriodEnd());
}

void CashFlowsTest::testPartialScheduleLegConstruction() {
    BOOST_TEST_MESSAGE("Testing leg construction with partial schedule...");
    // schedule with irregular first and last period
    Schedule schedule = MakeSchedule()
                            .from(Date(15, September, 2017))
                            .to(Date(30, September, 2020))
                            .withNextToLastDate(Date(25, September, 2020))
                            .withFrequency(Semiannual)
                            .backwards();
    // same schedule, date based, with metadata
    Schedule schedule2(schedule.dates(), NullCalendar(), Unadjusted, Unadjusted,
                       6 * Months, boost::none, schedule.endOfMonth(),
                       schedule.isRegular());
    // same schedule, date based, without metadata
    Schedule schedule3(schedule.dates());

    // fixed rate legs based on the three schedule
    Leg leg = FixedRateLeg(schedule).withNotionals(100.0).withCouponRates(
        0.01, ActualActual(ActualActual::ISMA));
    Leg leg2 = FixedRateLeg(schedule2).withNotionals(100.0).withCouponRates(
        0.01, ActualActual(ActualActual::ISMA));
    Leg leg3 = FixedRateLeg(schedule3).withNotionals(100.0).withCouponRates(
        0.01, ActualActual(ActualActual::ISMA));

    // check reference period of first and last coupon in all variants
    // for the first two we expect a 6M reference period, for the
    // third it can not be constructed, so should be equal to the
    // respective schedule period
    boost::shared_ptr<FixedRateCoupon> firstCpn =
        boost::dynamic_pointer_cast<FixedRateCoupon>(leg.front());
    boost::shared_ptr<FixedRateCoupon> lastCpn =
        boost::dynamic_pointer_cast<FixedRateCoupon>(leg.back());
    BOOST_REQUIRE(firstCpn != NULL);
    BOOST_REQUIRE(lastCpn != NULL);
    BOOST_CHECK_EQUAL(firstCpn->referencePeriodStart(), Date(25, Mar, 2017));
    BOOST_CHECK_EQUAL(firstCpn->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpn->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpn->referencePeriodEnd(), Date(25, Mar, 2021));

    boost::shared_ptr<FixedRateCoupon> firstCpn2 =
        boost::dynamic_pointer_cast<FixedRateCoupon>(leg2.front());
    boost::shared_ptr<FixedRateCoupon> lastCpn2 =
        boost::dynamic_pointer_cast<FixedRateCoupon>(leg2.back());
    BOOST_REQUIRE(firstCpn2 != NULL);
    BOOST_REQUIRE(lastCpn2 != NULL);
    BOOST_CHECK_EQUAL(firstCpn2->referencePeriodStart(), Date(25, Mar, 2017));
    BOOST_CHECK_EQUAL(firstCpn2->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpn2->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpn2->referencePeriodEnd(), Date(25, Mar, 2021));

    boost::shared_ptr<FixedRateCoupon> firstCpn3 =
        boost::dynamic_pointer_cast<FixedRateCoupon>(leg3.front());
    boost::shared_ptr<FixedRateCoupon> lastCpn3 =
        boost::dynamic_pointer_cast<FixedRateCoupon>(leg3.back());
    BOOST_REQUIRE(firstCpn3 != NULL);
    BOOST_REQUIRE(lastCpn3 != NULL);
    BOOST_CHECK_EQUAL(firstCpn3->referencePeriodStart(), Date(15, Sep, 2017));
    BOOST_CHECK_EQUAL(firstCpn3->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpn3->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpn3->referencePeriodEnd(), Date(30, Sep, 2020));

    // same check as above for a floating leg
    boost::shared_ptr<IborIndex> iborIndex =
        boost::make_shared<USDLibor>(3 * Months);
    Leg legf = IborLeg(schedule, iborIndex)
                   .withNotionals(100.0)
                   .withPaymentDayCounter(ActualActual(ActualActual::ISMA));
    Leg legf2 = IborLeg(schedule2, iborIndex)
                    .withNotionals(100.0)
                    .withPaymentDayCounter(ActualActual(ActualActual::ISMA));
    Leg legf3 = IborLeg(schedule3, iborIndex)
                    .withNotionals(100.0)
                    .withPaymentDayCounter(ActualActual(ActualActual::ISMA));

    boost::shared_ptr<FloatingRateCoupon> firstCpnF =
        boost::dynamic_pointer_cast<FloatingRateCoupon>(legf.front());
    boost::shared_ptr<FloatingRateCoupon> lastCpnF =
        boost::dynamic_pointer_cast<FloatingRateCoupon>(legf.back());
    BOOST_REQUIRE(firstCpnF != NULL);
    BOOST_REQUIRE(lastCpnF != NULL);
    BOOST_CHECK_EQUAL(firstCpnF->referencePeriodStart(), Date(25, Mar, 2017));
    BOOST_CHECK_EQUAL(firstCpnF->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpnF->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpnF->referencePeriodEnd(), Date(25, Mar, 2021));

    boost::shared_ptr<FloatingRateCoupon> firstCpnF2 =
        boost::dynamic_pointer_cast<FloatingRateCoupon>(legf2.front());
    boost::shared_ptr<FloatingRateCoupon> lastCpnF2 =
        boost::dynamic_pointer_cast<FloatingRateCoupon>(legf2.back());
    BOOST_REQUIRE(firstCpnF2 != NULL);
    BOOST_REQUIRE(lastCpnF2 != NULL);
    BOOST_CHECK_EQUAL(firstCpnF2->referencePeriodStart(), Date(25, Mar, 2017));
    BOOST_CHECK_EQUAL(firstCpnF2->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpnF2->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpnF2->referencePeriodEnd(), Date(25, Mar, 2021));

    boost::shared_ptr<FloatingRateCoupon> firstCpnF3 =
        boost::dynamic_pointer_cast<FloatingRateCoupon>(legf3.front());
    boost::shared_ptr<FloatingRateCoupon> lastCpnF3 =
        boost::dynamic_pointer_cast<FloatingRateCoupon>(legf3.back());
    BOOST_REQUIRE(firstCpnF3 != NULL);
    BOOST_REQUIRE(lastCpnF3 != NULL);
    BOOST_CHECK_EQUAL(firstCpnF3->referencePeriodStart(), Date(15, Sep, 2017));
    BOOST_CHECK_EQUAL(firstCpnF3->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpnF3->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpnF3->referencePeriodEnd(), Date(30, Sep, 2020));
}

test_suite* CashFlowsTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Cash flows tests");
    suite->add(QUANTLIB_TEST_CASE(&CashFlowsTest::testSettings));
    suite->add(QUANTLIB_TEST_CASE(&CashFlowsTest::testAccessViolation));
    suite->add(QUANTLIB_TEST_CASE(&CashFlowsTest::testDefaultSettlementDate));
    #ifndef QL_USE_INDEXED_COUPON
    suite->add(QUANTLIB_TEST_CASE(&CashFlowsTest::testNullFixingDays));
    #endif
    suite->add(QUANTLIB_TEST_CASE(
                             &CashFlowsTest::testIrregularFirstCouponReferenceDatesAtEndOfMonth));
    suite->add(QUANTLIB_TEST_CASE(
                             &CashFlowsTest::testIrregularLastCouponReferenceDatesAtEndOfMonth));
    suite->add(QUANTLIB_TEST_CASE(
                             &CashFlowsTest::testPartialScheduleLegConstruction));
    return suite;
}
