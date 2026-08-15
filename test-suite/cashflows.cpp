/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2012 StatPro Italia srl
 Copyright (C) 2026 Kyrylo Protsenko

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/bkbm.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/optional.hpp>
#include <ql/settings.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CashFlowTests)

BOOST_AUTO_TEST_CASE(testSettings) {

    BOOST_TEST_MESSAGE("Testing cash-flow settings...");

    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    // cash flows at T+0, T+1, T+2
    std::vector<ext::shared_ptr<CashFlow> > leg;
    leg.reserve(3);
    for (Integer i = 0; i < 3; ++i)
        leg.push_back(ext::make_shared<SimpleCashFlow>(1.0, today+i));


    #define CHECK_INCLUSION(n, days, expected) \
    if ((!leg[n]->hasOccurred(today+days)) != expected) { \
        BOOST_ERROR("cashflow at T+" << n << " " \
                    << (expected ? "not" : "") << "included" \
                    << " at T+" << days); \
    }

    // case 1: don't include reference-date payments, no override at
    //         today's date

    Settings::instance().includeReferenceDateEvents() = false;
    Settings::instance().includeTodaysCashFlows() = std::nullopt;

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
    Settings::instance().includeTodaysCashFlows() = std::nullopt;

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
    Settings::instance().includeTodaysCashFlows() = std::nullopt;

    CHECK_NPV(false, 2.0);
    CHECK_NPV(true, 3.0);
    
    // override
    Settings::instance().includeTodaysCashFlows() = false;
    
    CHECK_NPV(false, 2.0);
    CHECK_NPV(true, 2.0);

}

BOOST_AUTO_TEST_CASE(testAccessViolation) {
    BOOST_TEST_MESSAGE("Testing dynamic cast of coupon in Black pricer...");

    Date todaysDate(7, April, 2010);
    Date settlementDate(9, April, 2010);
    Settings::instance().evaluationDate() = todaysDate;
    Calendar calendar = TARGET();

    Handle<YieldTermStructure> rhTermStructure(
        flatRate(settlementDate, 0.04875825, Actual365Fixed()));

    Volatility volatility = 0.10;
    Handle<OptionletVolatilityStructure> vol;
    vol = Handle<OptionletVolatilityStructure>(
             ext::shared_ptr<OptionletVolatilityStructure>(
                 new ConstantOptionletVolatility(
                             2,
                             calendar,
                             ModifiedFollowing,
                             volatility,
                             Actual365Fixed())));

    ext::shared_ptr<IborIndex> index3m (new USDLibor(3*Months,
                                                       rhTermStructure));

    Date payDate(20, December, 2013);
    Date startDate(20, September, 2013);
    Date endDate(20, December, 2013);
    Rate spread = 0.0115;
    ext::shared_ptr<IborCouponPricer> pricer(new BlackIborCouponPricer(vol));
    ext::shared_ptr<FloatingRateCoupon> coupon(
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

BOOST_AUTO_TEST_CASE(testDefaultSettlementDate) {
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

BOOST_AUTO_TEST_CASE(testNullFixingDays, *precondition(usingAtParCoupons())) {
    BOOST_TEST_MESSAGE("Testing ibor leg construction with null fixing days...");
    Date today = Settings::instance().evaluationDate();
    Schedule schedule =
        MakeSchedule()
        .from(today-2*Months).to(today+4*Months)
        .withFrequency(Semiannual)
        .withCalendar(TARGET())
        .withConvention(Following)
        .backwards();

    ext::shared_ptr<IborIndex> index(new USDLibor(6*Months));
    Leg leg = IborLeg(schedule, index)
        .withNotionals(100.0)
        // this can happen with default values, and caused an
        // exception when the null was not managed properly
        .withFixingDays(Null<Natural>());
}

BOOST_AUTO_TEST_CASE(testExCouponDates) {
    BOOST_TEST_MESSAGE("Testing ex-coupon date calculation...");

    Date today = Date::todaysDate();
    Schedule schedule = MakeSchedule()
                            .from(today)
                            .to(today + 5 * Years)
                            .withFrequency(Monthly)
                            .withCalendar(TARGET())
                            .withConvention(Following);

    // no ex-coupon dates
    Leg l1 = FixedRateLeg(schedule).withNotionals(100.0).withCouponRates(0.03, Actual360());
    for (auto& i : l1) {
        ext::shared_ptr<Coupon> c = coupon_cast(i);
        if (c->exCouponDate() != Date()) {
            BOOST_ERROR("ex-coupon date found (none expected)");
        }
    }

    // same for floating legs
    ext::shared_ptr<IborIndex> index(new Euribor3M);
    Leg l2 = IborLeg(schedule, index).withNotionals(100.0);
    for (auto& i : l2) {
        ext::shared_ptr<Coupon> c = coupon_cast(i);
        if (c->exCouponDate() != Date()) {
            BOOST_ERROR("ex-coupon date found (none expected)");
        }
    }

    // calendar days
    Leg l5 = FixedRateLeg(schedule)
                 .withNotionals(100.0)
                 .withCouponRates(0.03, Actual360())
                 .withExCouponPeriod(Period(2, Days), NullCalendar(), Unadjusted, false);
    for (auto& i : l5) {
        ext::shared_ptr<Coupon> c = coupon_cast(i);
        Date expected = c->accrualEndDate() - 2;
        if (c->exCouponDate() != expected) {
            BOOST_ERROR("ex-coupon date = " << c->exCouponDate() << " (" << expected
                                            << " expected)");
        }
    }

    Leg l6 = IborLeg(schedule, index)
                 .withNotionals(100.0)
                 .withExCouponPeriod(Period(2, Days), NullCalendar(), Unadjusted, false);
    for (auto& i : l6) {
        ext::shared_ptr<Coupon> c = coupon_cast(i);
        Date expected = c->accrualEndDate() - 2;
        if (c->exCouponDate() != expected) {
            BOOST_ERROR("ex-coupon date = " << c->exCouponDate() << " (" << expected
                                            << " expected)");
        }
    }

    // business days
    Leg l7 = FixedRateLeg(schedule)
                 .withNotionals(100.0)
                 .withCouponRates(0.03, Actual360())
                 .withExCouponPeriod(Period(2, Days), TARGET(), Preceding, false);
    for (auto& i : l7) {
        ext::shared_ptr<Coupon> c = coupon_cast(i);
        Date expected = TARGET().advance(c->accrualEndDate(), -2, Days);
        if (c->exCouponDate() != expected) {
            BOOST_ERROR("ex-coupon date = " << c->exCouponDate() << " (" << expected
                                            << " expected)");
        }
    }

    Leg l8 = IborLeg(schedule, index)
                 .withNotionals(100.0)
                 .withExCouponPeriod(Period(2, Days), TARGET(), Preceding, false);
    for (auto& i : l8) {
        ext::shared_ptr<Coupon> c = coupon_cast(i);
        Date expected = TARGET().advance(c->accrualEndDate(), -2, Days);
        if (c->exCouponDate() != expected) {
            BOOST_ERROR("ex-coupon date = " << c->exCouponDate() << " (" << expected
                                            << " expected)");
        }
    }
}

BOOST_AUTO_TEST_CASE(testIrregularFirstCouponReferenceDatesAtEndOfMonth) {
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

    ext::shared_ptr<Coupon> firstCoupon =
        coupon_cast(leg.front());

    if (firstCoupon->referencePeriodStart() != Date(31, August, 2016))
        BOOST_ERROR("Expected reference start date at end of month, "
                    "got " << firstCoupon->referencePeriodStart());
}

BOOST_AUTO_TEST_CASE(testIrregularFirstCouponReferenceDatesAtEndOfCalendarMonth) {
    BOOST_TEST_MESSAGE("Testing irregular first coupon reference dates at end of calendar month with end of month enabled...");
    Schedule schedule =
        MakeSchedule()
        .withCalendar(UnitedStates(UnitedStates::GovernmentBond))
        .from(Date(30, September, 2017)).to(Date(30, September, 2022))
        .withTenor(6*Months)
        .withConvention(Unadjusted)
        .withTerminationDateConvention(Unadjusted)
        .withFirstDate(Date(31, March, 2018))
        .withNextToLastDate(Date(31, March, 2022))
        .endOfMonth()
        .backwards();

    Leg leg = FixedRateLeg(schedule)
        .withNotionals(100.0)
        .withCouponRates(0.01875, ActualActual(ActualActual::ISMA));

    ext::shared_ptr<Coupon> firstCoupon =
        coupon_cast(leg.front());
    if (firstCoupon->referencePeriodStart() != Date(30, September, 2017))
        BOOST_ERROR("Expected reference start date at end of calendar day of the month, "
                    "got " << firstCoupon->referencePeriodStart());
    // Expect first cashflow to be 0.9375
    BOOST_TEST(firstCoupon->amount() == 0.9375, boost::test_tools::tolerance<Real>(0.0001));
}

BOOST_AUTO_TEST_CASE(testIrregularLastCouponReferenceDatesAtEndOfMonth) {
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

    ext::shared_ptr<Coupon> lastCoupon =
            coupon_cast(leg.back());

    if (lastCoupon->referencePeriodEnd() != Date(31, August, 2018))
        BOOST_ERROR("Expected reference end date at end of month, "
                            "got " << lastCoupon->referencePeriodEnd());
}

BOOST_AUTO_TEST_CASE(testPartialScheduleLegConstruction) {
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
                       6 * Months, std::nullopt, schedule.endOfMonth(),
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
    ext::shared_ptr<FixedRateCoupon> firstCpn =
        ext::dynamic_pointer_cast<FixedRateCoupon>(leg.front());
    ext::shared_ptr<FixedRateCoupon> lastCpn =
        ext::dynamic_pointer_cast<FixedRateCoupon>(leg.back());
    BOOST_REQUIRE(firstCpn != nullptr);
    BOOST_REQUIRE(lastCpn != nullptr);
    BOOST_CHECK_EQUAL(firstCpn->referencePeriodStart(), Date(25, Mar, 2017));
    BOOST_CHECK_EQUAL(firstCpn->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpn->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpn->referencePeriodEnd(), Date(25, Mar, 2021));

    ext::shared_ptr<FixedRateCoupon> firstCpn2 =
        ext::dynamic_pointer_cast<FixedRateCoupon>(leg2.front());
    ext::shared_ptr<FixedRateCoupon> lastCpn2 =
        ext::dynamic_pointer_cast<FixedRateCoupon>(leg2.back());
    BOOST_REQUIRE(firstCpn2 != nullptr);
    BOOST_REQUIRE(lastCpn2 != nullptr);
    BOOST_CHECK_EQUAL(firstCpn2->referencePeriodStart(), Date(25, Mar, 2017));
    BOOST_CHECK_EQUAL(firstCpn2->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpn2->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpn2->referencePeriodEnd(), Date(25, Mar, 2021));

    ext::shared_ptr<FixedRateCoupon> firstCpn3 =
        ext::dynamic_pointer_cast<FixedRateCoupon>(leg3.front());
    ext::shared_ptr<FixedRateCoupon> lastCpn3 =
        ext::dynamic_pointer_cast<FixedRateCoupon>(leg3.back());
    BOOST_REQUIRE(firstCpn3 != nullptr);
    BOOST_REQUIRE(lastCpn3 != nullptr);
    BOOST_CHECK_EQUAL(firstCpn3->referencePeriodStart(), Date(15, Sep, 2017));
    BOOST_CHECK_EQUAL(firstCpn3->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpn3->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpn3->referencePeriodEnd(), Date(30, Sep, 2020));

    // same check as above for a floating leg
    ext::shared_ptr<IborIndex> iborIndex =
        ext::make_shared<USDLibor>(3 * Months);
    Leg legf = IborLeg(schedule, iborIndex)
                   .withNotionals(100.0)
                   .withPaymentDayCounter(ActualActual(ActualActual::ISMA));
    Leg legf2 = IborLeg(schedule2, iborIndex)
                    .withNotionals(100.0)
                    .withPaymentDayCounter(ActualActual(ActualActual::ISMA));
    Leg legf3 = IborLeg(schedule3, iborIndex)
                    .withNotionals(100.0)
                    .withPaymentDayCounter(ActualActual(ActualActual::ISMA));

    ext::shared_ptr<FloatingRateCoupon> firstCpnF =
        ext::dynamic_pointer_cast<FloatingRateCoupon>(legf.front());
    ext::shared_ptr<FloatingRateCoupon> lastCpnF =
        ext::dynamic_pointer_cast<FloatingRateCoupon>(legf.back());
    BOOST_REQUIRE(firstCpnF != nullptr);
    BOOST_REQUIRE(lastCpnF != nullptr);
    BOOST_CHECK_EQUAL(firstCpnF->referencePeriodStart(), Date(25, Mar, 2017));
    BOOST_CHECK_EQUAL(firstCpnF->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpnF->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpnF->referencePeriodEnd(), Date(25, Mar, 2021));

    ext::shared_ptr<FloatingRateCoupon> firstCpnF2 =
        ext::dynamic_pointer_cast<FloatingRateCoupon>(legf2.front());
    ext::shared_ptr<FloatingRateCoupon> lastCpnF2 =
        ext::dynamic_pointer_cast<FloatingRateCoupon>(legf2.back());
    BOOST_REQUIRE(firstCpnF2 != nullptr);
    BOOST_REQUIRE(lastCpnF2 != nullptr);
    BOOST_CHECK_EQUAL(firstCpnF2->referencePeriodStart(), Date(25, Mar, 2017));
    BOOST_CHECK_EQUAL(firstCpnF2->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpnF2->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpnF2->referencePeriodEnd(), Date(25, Mar, 2021));

    ext::shared_ptr<FloatingRateCoupon> firstCpnF3 =
        ext::dynamic_pointer_cast<FloatingRateCoupon>(legf3.front());
    ext::shared_ptr<FloatingRateCoupon> lastCpnF3 =
        ext::dynamic_pointer_cast<FloatingRateCoupon>(legf3.back());
    BOOST_REQUIRE(firstCpnF3 != nullptr);
    BOOST_REQUIRE(lastCpnF3 != nullptr);
    BOOST_CHECK_EQUAL(firstCpnF3->referencePeriodStart(), Date(15, Sep, 2017));
    BOOST_CHECK_EQUAL(firstCpnF3->referencePeriodEnd(), Date(25, Sep, 2017));
    BOOST_CHECK_EQUAL(lastCpnF3->referencePeriodStart(), Date(25, Sep, 2020));
    BOOST_CHECK_EQUAL(lastCpnF3->referencePeriodEnd(), Date(30, Sep, 2020));
}

BOOST_AUTO_TEST_CASE(testFixedIborCouponWithoutForecastCurve) {
    BOOST_TEST_MESSAGE("Testing past ibor coupon without forecast curve...");

    Date today = Settings::instance().evaluationDate();

    auto index = ext::make_shared<USDLibor>(6*Months);
    auto calendar = index->fixingCalendar();

    Date fixingDate = calendar.advance(today, -2, Months);
    Rate pastFixing = 0.01;
    index->addFixing(fixingDate, pastFixing);

    Date startDate = index->valueDate(fixingDate);
    Date endDate = index->maturityDate(fixingDate);

    IborCoupon coupon(endDate, 100.0, startDate, endDate, index->fixingDays(), index);
    coupon.setPricer(ext::make_shared<BlackIborCouponPricer>());

    BOOST_CHECK_NO_THROW(coupon.amount());

    // the main check is the one above, but let's check for consistency too:
    Real amount = coupon.amount();
    Real expected = pastFixing * coupon.nominal() * coupon.accrualPeriod();
    if (std::fabs(amount - expected) > 1e-8) {
        BOOST_ERROR("amount mismatch:"
                    << "\n    calculated: " << amount
                    << "\n    expected: " << expected);
    }
}

IborCoupon iborCouponForFixingDate(const ext::shared_ptr<IborIndex>& index, Date fixingDate) {
    Date startDate = index->valueDate(fixingDate);
    Date endDate = index->maturityDate(fixingDate);

    IborCoupon coupon(endDate, 100.0, startDate, endDate, index->fixingDays(), index);
    coupon.setPricer(ext::make_shared<BlackIborCouponPricer>());

    return coupon;
}

BOOST_AUTO_TEST_CASE(testIborCouponKnowsWhenitHasFixed) {
    BOOST_TEST_MESSAGE("Testing that ibor coupon knows when it has fixed...");

    Date today = Settings::instance().evaluationDate();

    auto index = ext::make_shared<Euribor3M>();
    auto calendar = index->fixingCalendar();

    {
        IborCoupon coupon = iborCouponForFixingDate(index, calendar.advance(today, -1, Days));
        index->clearFixings();
        // this should not throw an exception if the fixing is missing
        BOOST_CHECK_EQUAL(coupon.hasFixed(), true);
        // but this should
        BOOST_CHECK_THROW(coupon.rate(), Error);
    }

    {
        IborCoupon coupon = iborCouponForFixingDate(index, today);
        QuantLib::Settings::instance().enforcesTodaysHistoricFixings() = false;
        index->clearFixings();
        BOOST_CHECK_EQUAL(coupon.hasFixed(), false);
    }

    {
        IborCoupon coupon = iborCouponForFixingDate(index, today);
        QuantLib::Settings::instance().enforcesTodaysHistoricFixings() = false;
        index->addFixing(coupon.fixingDate(), 0.01);
        BOOST_CHECK_EQUAL(coupon.hasFixed(), true);
    }

    {
        IborCoupon coupon = iborCouponForFixingDate(index, today);
        QuantLib::Settings::instance().enforcesTodaysHistoricFixings() = true;
        index->clearFixings();
        BOOST_CHECK_EQUAL(coupon.hasFixed(), true);
        BOOST_CHECK_THROW(coupon.rate(), Error);
    }

    {
        IborCoupon coupon = iborCouponForFixingDate(index, calendar.advance(today, 1, Days));
        BOOST_CHECK_EQUAL(coupon.hasFixed(), false);
    }
}

BOOST_AUTO_TEST_CASE(testInterpolatedIborStubCoupon) {
    BOOST_TEST_MESSAGE("Testing interpolation of an irregular Ibor coupon...");

    SavedSettings backup;
    const Date today(27, May, 2026);
    Settings::instance().evaluationDate() = today;

    RelinkableHandle<YieldTermStructure> twoMonthCurve, threeMonthCurve;
    twoMonthCurve.linkTo(flatRate(today, 0.02, Actual365Fixed()));
    threeMonthCurve.linkTo(flatRate(today, 0.04, Actual365Fixed()));

    const Calendar calendar = NewZealand();
    auto bkbm2m = ext::make_shared<IborIndex>(
        "Bkbm", 2 * Months, 0, NZDCurrency(), calendar, ModifiedFollowing, false,
        Actual365Fixed(), twoMonthCurve);
    auto bkbm3m = ext::make_shared<IborIndex>(
        "Bkbm", 3 * Months, 0, NZDCurrency(), calendar, ModifiedFollowing, false,
        Actual365Fixed(), threeMonthCurve);
    const Date start(29, May, 2026);
    const Date maturity(28, February, 2027);

    Schedule schedule = MakeSchedule()
                            .from(start)
                            .to(maturity)
                            .withTenor(3 * Months)
                            .withCalendar(calendar)
                            .withConvention(ModifiedFollowing)
                            .backwards();
    BOOST_REQUIRE(!schedule.isRegular(1));
    BOOST_CHECK_EQUAL(schedule[1], Date(28, August, 2026));

    StubIndexConfig convention{
        StubIndexConvention::Interpolated, {bkbm2m, bkbm3m}};
    Leg leg = IborLeg(schedule, bkbm3m)
                  .withNotionals(1.0)
                  .withIndexedCoupons(true)
                  .withStubIndexConfig(convention);

    auto coupon = ext::dynamic_pointer_cast<StubIborCoupon>(leg.front());
    BOOST_REQUIRE(coupon);
    BOOST_CHECK(!ext::dynamic_pointer_cast<StubIborCoupon>(leg[1]));

    const Date fixingDate = coupon->fixingDate();
    const Date shortMaturity = bkbm2m->maturityDate(start);
    const Date longMaturity = bkbm3m->maturityDate(start);
    BOOST_CHECK_EQUAL(shortMaturity, Date(29, July, 2026));
    BOOST_CHECK_EQUAL(longMaturity, Date(31, August, 2026));

    const Rate shortRate = bkbm2m->fixing(fixingDate);
    const Rate longRate = bkbm3m->fixing(fixingDate);
    const Real weight = Real(coupon->accrualEndDate() - shortMaturity) /
                        Real(longMaturity - shortMaturity);
    const Rate expected = shortRate + (longRate - shortRate) * weight;
    BOOST_CHECK_SMALL(coupon->indexFixing() - expected, 1.0e-14);

    StubIndexConfig closest{
        StubIndexConvention::ClosestIndex, {bkbm2m, bkbm3m}};
    Leg closestLeg = IborLeg(schedule, bkbm3m)
                         .withNotionals(1.0)
                         .withIndexedCoupons(true)
                         .withStubIndexConfig(closest);
    auto closestCoupon = ext::dynamic_pointer_cast<StubIborCoupon>(closestLeg.front());
    BOOST_REQUIRE(closestCoupon);
    BOOST_CHECK_SMALL(closestCoupon->indexFixing() - longRate, 1.0e-14);

    // stub index conventions require indexed coupons...
    BOOST_CHECK_THROW(static_cast<Leg>(IborLeg(schedule, bkbm3m)
                                           .withNotionals(1.0)
                                           .withIndexedCoupons(false)
                                           .withStubIndexConfig(convention)),
                      Error);

    // ...and candidates with distinct maturities
    StubIndexConfig duplicate{StubIndexConvention::Interpolated, {bkbm2m, bkbm3m, bkbm3m}};
    BOOST_CHECK_THROW(static_cast<Leg>(IborLeg(schedule, bkbm3m)
                                           .withNotionals(1.0)
                                           .withIndexedCoupons(true)
                                           .withStubIndexConfig(duplicate)),
                      Error);

    // a default-constructed configuration is empty and leaves the leg alone
    BOOST_CHECK(StubIndexConfig().empty());
    Leg defaultLeg = IborLeg(schedule, bkbm3m)
                         .withNotionals(1.0)
                         .withIndexedCoupons(true)
                         .withStubIndexConfig(StubIndexConfig());
    BOOST_CHECK(!ext::dynamic_pointer_cast<StubIborCoupon>(defaultLeg.front()));

    // configurations without usable candidates are rejected on construction
    BOOST_CHECK_THROW(StubIndexConfig(StubIndexConvention::Interpolated, {}), Error);
    BOOST_CHECK_THROW(
        StubIndexConfig(StubIndexConvention::Interpolated, {bkbm2m, nullptr}), Error);
}

BOOST_AUTO_TEST_CASE(testIborStubInterpolationEndpointsAndRegularSchedule) {
    BOOST_TEST_MESSAGE("Testing Ibor stub endpoints and unchanged regular coupons...");

    SavedSettings backup;
    const Date today(27, May, 2026);
    Settings::instance().evaluationDate() = today;

    Handle<YieldTermStructure> twoMonthCurve(flatRate(today, 0.02, Actual365Fixed()));
    Handle<YieldTermStructure> threeMonthCurve(flatRate(today, 0.04, Actual365Fixed()));
    auto bkbm2m = ext::make_shared<Bkbm2M>(twoMonthCurve);
    auto bkbm3m = ext::make_shared<Bkbm3M>(threeMonthCurve);
    const Date start(29, May, 2026);
    StubIndexConfig convention{
        StubIndexConvention::Interpolated, {bkbm2m, bkbm3m}};

    for (const auto& endpoint : std::vector<ext::shared_ptr<IborIndex> >{bkbm2m, bkbm3m}) {
        const Date end = endpoint->maturityDate(start);
        Schedule endpointSchedule({start, end}, endpoint->fixingCalendar(), ModifiedFollowing,
                                  std::nullopt, 3 * Months, DateGeneration::Backward, true,
                                  {false});
        Leg leg = IborLeg(endpointSchedule, bkbm3m)
                      .withNotionals(1.0)
                      .withIndexedCoupons(true)
                      .withStubIndexConfig(convention);
        auto coupon = ext::dynamic_pointer_cast<StubIborCoupon>(leg.front());
        BOOST_REQUIRE(coupon);
        BOOST_CHECK_SMALL(coupon->indexFixing() - endpoint->fixing(coupon->fixingDate()),
                          1.0e-14);
    }

    const Date regularEnd = bkbm3m->maturityDate(start);
    Schedule regularSchedule({start, regularEnd}, bkbm3m->fixingCalendar(), ModifiedFollowing,
                             std::nullopt, 3 * Months, DateGeneration::Backward, true, {true});
    Leg defaultLeg = IborLeg(regularSchedule, bkbm3m)
                         .withNotionals(1.0)
                         .withIndexedCoupons(true);
    Leg configuredLeg = IborLeg(regularSchedule, bkbm3m)
                            .withNotionals(1.0)
                            .withIndexedCoupons(true)
                            .withStubIndexConfig(convention);
    BOOST_CHECK(!ext::dynamic_pointer_cast<StubIborCoupon>(configuredLeg.front()));
    auto defaultCoupon = ext::dynamic_pointer_cast<IborCoupon>(defaultLeg.front());
    auto configuredCoupon = ext::dynamic_pointer_cast<IborCoupon>(configuredLeg.front());
    BOOST_REQUIRE(defaultCoupon && configuredCoupon);
    BOOST_CHECK_SMALL(defaultCoupon->rate() - configuredCoupon->rate(), 1.0e-14);
}

BOOST_AUTO_TEST_CASE(testFinalAndLongIborStubCoupons) {
    BOOST_TEST_MESSAGE("Testing final and long Ibor stub coupons...");

    SavedSettings backup;
    const Date today(27, May, 2026);
    Settings::instance().evaluationDate() = today;
    const Calendar calendar = NewZealand();

    Handle<YieldTermStructure> oneMonthCurve(flatRate(today, 0.01, Actual365Fixed()));
    Handle<YieldTermStructure> threeMonthCurve(flatRate(today, 0.03, Actual365Fixed()));
    Handle<YieldTermStructure> sixMonthCurve(flatRate(today, 0.06, Actual365Fixed()));
    auto bkbm1m = ext::make_shared<Bkbm1M>(oneMonthCurve);
    auto bkbm3m = ext::make_shared<Bkbm3M>(threeMonthCurve);
    auto bkbm6m = ext::make_shared<Bkbm6M>(sixMonthCurve);

    Schedule finalStubSchedule(
        {Date(29, May, 2026), Date(31, August, 2026), Date(30, October, 2026)},
        calendar, ModifiedFollowing, std::nullopt, 3 * Months, DateGeneration::Forward,
        true, {true, false});
    StubIndexConfig finalConvention{
        StubIndexConvention::Interpolated, {bkbm1m, bkbm3m}};
    Leg finalStubLeg = IborLeg(finalStubSchedule, bkbm3m)
                           .withNotionals(1.0)
                           .withIndexedCoupons(true)
                           .withStubIndexConfig(finalConvention);
    BOOST_CHECK(!ext::dynamic_pointer_cast<StubIborCoupon>(finalStubLeg.front()));
    auto finalCoupon = ext::dynamic_pointer_cast<StubIborCoupon>(finalStubLeg.back());
    BOOST_REQUIRE(finalCoupon);

    const Date finalStart = finalCoupon->accrualStartDate();
    const Date oneMonthMaturity = bkbm1m->maturityDate(finalStart);
    const Date threeMonthMaturity = bkbm3m->maturityDate(finalStart);
    const Real finalWeight = Real(finalCoupon->accrualEndDate() - oneMonthMaturity) /
                             Real(threeMonthMaturity - oneMonthMaturity);
    const Rate expectedFinal =
        bkbm1m->fixing(finalCoupon->fixingDate()) +
        (bkbm3m->fixing(finalCoupon->fixingDate()) -
         bkbm1m->fixing(finalCoupon->fixingDate())) * finalWeight;
    BOOST_CHECK_SMALL(finalCoupon->indexFixing() - expectedFinal, 1.0e-14);

    const Date longStart(29, May, 2026);
    const Date longEnd(30, October, 2026);
    Schedule longStubSchedule({longStart, longEnd}, calendar, ModifiedFollowing,
                              std::nullopt, 3 * Months, DateGeneration::Backward, true,
                              {false});
    StubIndexConfig longConvention{
        StubIndexConvention::Interpolated, {bkbm3m, bkbm6m}};
    Leg longStubLeg = IborLeg(longStubSchedule, bkbm3m)
                          .withNotionals(1.0)
                          .withIndexedCoupons(true)
                          .withStubIndexConfig(longConvention);
    auto longCoupon = ext::dynamic_pointer_cast<StubIborCoupon>(longStubLeg.front());
    BOOST_REQUIRE(longCoupon);
    const Date threeMonthLongMaturity = bkbm3m->maturityDate(longStart);
    const Date sixMonthMaturity = bkbm6m->maturityDate(longStart);
    const Real longWeight = Real(longEnd - threeMonthLongMaturity) /
                            Real(sixMonthMaturity - threeMonthLongMaturity);
    const Rate expectedLong =
        bkbm3m->fixing(longCoupon->fixingDate()) +
        (bkbm6m->fixing(longCoupon->fixingDate()) -
         bkbm3m->fixing(longCoupon->fixingDate())) * longWeight;
    BOOST_CHECK_SMALL(longCoupon->indexFixing() - expectedLong, 1.0e-14);
}

BOOST_AUTO_TEST_CASE(testIborStubCouponHistoricalFixingsAndRelinking) {
    BOOST_TEST_MESSAGE("Testing Ibor stub historical fixings and forwarding-curve relinking...");

    SavedSettings backup;
    const Date fixingDate(29, May, 2026);
    const Date start = fixingDate;
    const Date end(28, August, 2026);

    RelinkableHandle<YieldTermStructure> twoMonthCurve, threeMonthCurve;
    twoMonthCurve.linkTo(flatRate(fixingDate, 0.02, Actual365Fixed()));
    threeMonthCurve.linkTo(flatRate(fixingDate, 0.04, Actual365Fixed()));
    auto bkbm2m = ext::make_shared<Bkbm2M>(twoMonthCurve);
    auto bkbm3m = ext::make_shared<Bkbm3M>(threeMonthCurve);
    StubIndexConfig convention{
        StubIndexConvention::Interpolated, {bkbm2m, bkbm3m}};
    Schedule schedule({start, end}, bkbm3m->fixingCalendar(), ModifiedFollowing,
                      std::nullopt, 3 * Months, DateGeneration::Backward, true, {false});
    Leg leg = IborLeg(schedule, bkbm3m)
                  .withNotionals(1.0)
                  .withIndexedCoupons(true)
                  .withStubIndexConfig(convention);
    auto coupon = ext::dynamic_pointer_cast<StubIborCoupon>(leg.front());
    BOOST_REQUIRE(coupon);

    Settings::instance().evaluationDate() = Date(27, May, 2026);
    const Rate initialRate = coupon->rate();
    Flag flag;
    flag.registerWith(coupon);
    twoMonthCurve.linkTo(flatRate(fixingDate, 0.03, Actual365Fixed()));
    BOOST_CHECK(flag.isUp());
    BOOST_CHECK(std::fabs(coupon->rate() - initialRate) > 1.0e-6);

    Settings::instance().evaluationDate() = Date(1, June, 2026);
    bkbm2m->addFixing(fixingDate, 0.05);
    bkbm3m->addFixing(fixingDate, 0.08);
    const Date shortMaturity = bkbm2m->maturityDate(start);
    const Date longMaturity = bkbm3m->maturityDate(start);
    const Real weight = Real(end - shortMaturity) / Real(longMaturity - shortMaturity);
    const Rate expected = 0.05 + (0.08 - 0.05) * weight;
    BOOST_CHECK_SMALL(coupon->indexFixing() - expected, 1.0e-14);

    bkbm2m->clearFixings();
    bkbm3m->clearFixings();
}

BOOST_AUTO_TEST_CASE(testIborStubCouponHasFixedUsesSelectedIndices) {
    BOOST_TEST_MESSAGE("Testing that a stub coupon has fixed only once its selected indices have...");

    SavedSettings backup;
    const Date fixingDate(29, May, 2026);
    const Date start = fixingDate;
    const Date end(28, August, 2026);
    Settings::instance().evaluationDate() = fixingDate;

    Handle<YieldTermStructure> twoMonthCurve(flatRate(fixingDate, 0.02, Actual365Fixed()));
    Handle<YieldTermStructure> threeMonthCurve(flatRate(fixingDate, 0.04, Actual365Fixed()));
    auto bkbm2m = ext::make_shared<Bkbm2M>(twoMonthCurve);
    auto bkbm3m = ext::make_shared<Bkbm3M>(threeMonthCurve);
    StubIndexConfig convention{
        StubIndexConvention::Interpolated, {bkbm2m, bkbm3m}};
    Schedule schedule({start, end}, bkbm3m->fixingCalendar(), ModifiedFollowing,
                      std::nullopt, 3 * Months, DateGeneration::Backward, true, {false});
    Leg leg = IborLeg(schedule, bkbm3m)
                  .withNotionals(1.0)
                  .withIndexedCoupons(true)
                  .withStubIndexConfig(convention);
    auto coupon = ext::dynamic_pointer_cast<StubIborCoupon>(leg.front());
    BOOST_REQUIRE(coupon);
    BOOST_CHECK_EQUAL(coupon->fixingDate(), fixingDate);

    BOOST_CHECK(!coupon->hasFixed());

    // The leg index's own print must not flip the stub to fixed: the coupon
    // reads the bracketing candidates, and the 2M print is still missing.
    // Checked through a base-class pointer so the virtual dispatch is covered.
    bkbm3m->addFixing(fixingDate, 0.08);
    const ext::shared_ptr<IborCoupon> asIborCoupon = coupon;
    BOOST_CHECK(!asIborCoupon->hasFixed());

    bkbm2m->addFixing(fixingDate, 0.05);
    BOOST_CHECK(asIborCoupon->hasFixed());

    const Date shortMaturity = bkbm2m->maturityDate(start);
    const Date longMaturity = bkbm3m->maturityDate(start);
    const Real weight = Real(end - shortMaturity) / Real(longMaturity - shortMaturity);
    BOOST_CHECK_SMALL(coupon->indexFixing() - (0.05 + (0.08 - 0.05) * weight), 1.0e-14);

    bkbm2m->clearFixings();
    bkbm3m->clearFixings();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
