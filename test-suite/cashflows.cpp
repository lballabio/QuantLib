/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2012 StatPro Italia srl
 Copyright (C) 2017 Peter Caspers

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
#include <ql/indexes/ibor/euribor.hpp>
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

void CashFlowsTest::testFreeFixingDates() {
    BOOST_TEST_MESSAGE("Testing Ibor coupons with free fixing date...");

    // preparations
    SavedSettings backup;
    Date today(15, October, 2017);
    Settings::instance().evaluationDate() = today;
    Handle<YieldTermStructure> curve(flatRate(today, 0.02, Actual365Fixed()));
    Handle<OptionletVolatilityStructure> vol(
        boost::make_shared<ConstantOptionletVolatility>(
            0, TARGET(), ModifiedFollowing, 0.20, Actual365Fixed()));
    boost::shared_ptr<IborIndex> index(new Euribor(3 * Months, curve));
    boost::shared_ptr<IborCouponPricer> pricer(new BlackIborCouponPricer(vol));
    boost::shared_ptr<IborCouponPricer> pricer2(new BlackIborCouponPricer(
        vol, BlackIborCouponPricer::BivariateLognormal));

    // check free fixing date against standard deduction
    boost::shared_ptr<IborCoupon> c1 = boost::make_shared<IborCoupon>(
        Date(13, October, 2028), 100.0, Date(13, July, 2028),
        Date(13, October, 2028), 2, index);
    boost::shared_ptr<IborCoupon> c1f = boost::make_shared<IborCoupon>(
        Date(13, October, 2028), 100.0, Date(13, July, 2028),
        Date(13, October, 2028), c1->fixingDate(), index);
    c1->setPricer(pricer);
    c1f->setPricer(pricer);
    BOOST_CHECK_CLOSE(c1->amount(), c1f->amount(), 1E-10);

    // check forbidden free fixing date
    BOOST_CHECK_THROW(boost::make_shared<IborCoupon>(
                          Date(13, October, 2028), 100.0, Date(13, July, 2028),
                          Date(13, October, 2028), Date(2, November, 2028),
                          index),
                      QuantLib::Error);

    // check standard in arrears adjustment
    boost::shared_ptr<FloatingRateCoupon> c2 =
        boost::shared_ptr<FloatingRateCoupon>(
            new IborCoupon(Date(13, October, 2028), 100.0, Date(13, July, 2028),
                           Date(13, October, 2028), 2, index, 1.0, 0.0, Date(),
                           Date(), DayCounter(), true));
    boost::shared_ptr<FloatingRateCoupon> c2f = boost::make_shared<IborCoupon>(
        Date(13, October, 2028), 100.0, Date(13, July, 2028),
        Date(13, October, 2028), c2->fixingDate(), index);
    c2->setPricer(pricer);
    c2f->setPricer(pricer);
    BOOST_CHECK_CLOSE(c2->convexityAdjustment(), c2f->convexityAdjustment(),
                      1E-10);

    // check qualitative behaviour of bivariate adjustment
    // standard fixing => close to zero adjustment
    boost::shared_ptr<FloatingRateCoupon> c3a = boost::make_shared<IborCoupon>(
        Date(13, October, 2028), 100.0, Date(13, July, 2028),
        Date(13, October, 2028), index->fixingDate(Date(13, July, 2028)),
        index);
    // early fixing (delayed payment) => negative adjustment
    boost::shared_ptr<FloatingRateCoupon> c3b = boost::make_shared<IborCoupon>(
        Date(13, October, 2028), 100.0, Date(13, July, 2028),
        Date(13, October, 2028), index->fixingDate(Date(13, March, 2028)),
        index);
    // late fixing (early payment) => positive adjustment
    boost::shared_ptr<FloatingRateCoupon> c3c = boost::make_shared<IborCoupon>(
        Date(13, October, 2028), 100.0, Date(13, July, 2028),
        Date(13, October, 2028), index->fixingDate(Date(13, September, 2028)),
        index);
    c3a->setPricer(pricer2);
    c3b->setPricer(pricer2);
    c3c->setPricer(pricer2);
    BOOST_CHECK_SMALL(c3a->convexityAdjustment(), 1E-12);
    // check for correct sign, but also for plausible absolute size
    BOOST_CHECK(c3b->convexityAdjustment() < 0.0 &&
                c3b->convexityAdjustment() > -10E-4);
    BOOST_CHECK(c3c->convexityAdjustment() > 0.0 &&
                c3c->convexityAdjustment() < 10E-4);
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
    suite->add(QUANTLIB_TEST_CASE(&CashFlowsTest::testFreeFixingDates));
    return suite;
}

