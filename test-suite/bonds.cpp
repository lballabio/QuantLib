
/*
 Copyright (C) 2004 StatPro Italia srl

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

#include "bonds.hpp"
#include "utilities.hpp"
#include <ql/Instruments/fixedcouponbond.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/Calendars/unitedstates.hpp>
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/DayCounters/actualactual.hpp>
#include <ql/Utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    Calendar calendar;
    Date today;

    void setup() {
        calendar = TARGET();
        today = calendar.adjust(Date::todaysDate());
        Settings::instance().setEvaluationDate(today);
    }

    void teardown() {
        Settings::instance().setEvaluationDate(Date());
    }

}

// tests

void BondTest::testYield() {

    BOOST_MESSAGE("Testing consistency of bond price/yield calculation...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Real tolerance = 1.0e-7;
    Size maxEvaluations = 100;

    Integer issueMonths[] = { -24, -18, -12, -6, 0, 6, 12, 18, 24 };
    Integer lengths[] = { 3, 5, 10, 15, 20 };
    Integer settlementDays = 3;
    Real coupons[] = { 0.02, 0.05, 0.08 };
    Frequency frequencies[] = { Semiannual, Annual };
    DayCounter bondDayCount = Thirty360();
    BusinessDayConvention convention = ModifiedFollowing;
    Real redemption = 100.0;

    Rate yields[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    DayCounter yieldDayCount = Actual360();

    for (Size i=0; i<LENGTH(issueMonths); i++) {
      for (Size j=0; j<LENGTH(lengths); j++) {
        for (Size k=0; k<LENGTH(coupons); k++) {
          for (Size l=0; l<LENGTH(frequencies); l++) {

            Date dated = calendar.advance(today, issueMonths[i], Months);
            Date issue = dated;
            Date maturity = calendar.advance(issue, lengths[j], Years);

            FixedCouponBond bond(issue, dated, maturity, settlementDays,
                                 coupons[k], frequencies[l],
                                 bondDayCount, calendar,
                                 convention, redemption);

            for (Size m=0; m<LENGTH(yields); m++) {

              Real price = bond.cleanPrice(yields[m]);
              Rate calculated = bond.yield(price, Date(),
                                           tolerance, maxEvaluations);

              if (std::fabs(yields[m]-calculated) > tolerance) {
                  // the difference might not matter
                  Real price2 = bond.cleanPrice(calculated);
                  if (std::fabs(price-price2)/price > tolerance) {
                      BOOST_FAIL(
                          "yield recalculation failed:\n"
                          << "    issue:     "
                          << DateFormatter::toString(issue) << "\n"
                          << "    maturity:  "
                          << DateFormatter::toString(maturity) << "\n"
                          << "    coupon:    " << io::rate(coupons[k]) << "\n"
                          << "    frequency: "
                          << FrequencyFormatter::toString(frequencies[l])
                          << "\n\n"
                          << "    yield:  " << io::rate(yields[m]) << "\n"
                          << std::setprecision(7)
                          << "    price:  " << price << "\n"
                          << "    yield': " << io::rate(calculated) << "\n"
                          << "    price': " << price2);
                  }
              }
            }
          }
        }
      }
    }

    QL_TEST_TEARDOWN
}


void BondTest::testCachedYield() {

    BOOST_MESSAGE(
        "Testing bond price/yield calculation against cached values...");

    QL_TEST_BEGIN

    // with implicit settlement calculation:

    Settings::instance().setEvaluationDate(Date(22,November,2004));

    Calendar bondCalendar = NullCalendar();
    DayCounter bondDayCount = ActualActual(ActualActual::ISMA);
    Integer settlementDays = 1;

    // actual market values from the evaluation date

    FixedCouponBond bond1(Date(1,November,2004),
                          Date(31,October,2004),
                          Date(31,October,2006),
                          settlementDays,
                          0.025, Semiannual,
                          bondDayCount, bondCalendar,
                          Unadjusted, 100.0);

    Real marketPrice1 = 99.203125;
    Rate marketYield1 = 0.02925;

    FixedCouponBond bond2(Date(15,November,2004),
                          Date(15,November,2004),
                          Date(15,November,2009),
                          settlementDays,
                          0.035, Semiannual,
                          bondDayCount, bondCalendar,
                          Unadjusted, 100.0);

    Real marketPrice2 = 99.6875;
    Rate marketYield2 = 0.03569;

    // calculated values

    Real cachedPrice1 = 99.204505, cachedPrice2 = 99.687192;
    Rate cachedYield1 = 0.029257, cachedYield2 = 0.035689;

    // check
    Real tolerance = 1.0e-6;
    Real price, yield;

    price = bond1.cleanPrice(marketYield1);
    if (std::fabs(price-cachedPrice1) > tolerance) {
        BOOST_FAIL("failed to reproduce cached price:\n"
                   << "    calculated: " << price << "\n"
                   << "    expected:   " << cachedPrice1 << "\n"
                   << "    error:      " << price-cachedPrice1);
    }

    yield = bond1.yield(marketPrice1);
    if (std::fabs(yield-cachedYield1) > tolerance) {
        BOOST_FAIL("failed to reproduce cached yield:\n"
                   << std::setprecision(4)
                   << "    calculated: " << io::rate(yield) << "\n"
                   << "    expected:   " << io::rate(cachedYield1) << "\n"
                   << "    error:      " << io::rate(yield-cachedYield1));
    }


    price = bond2.cleanPrice(marketYield2);
    if (std::fabs(price-cachedPrice2) > tolerance) {
        BOOST_FAIL("failed to reproduce cached price:\n"
                   << "    calculated: " << price << "\n"
                   << "    expected:   " << cachedPrice2 << "\n"
                   << "    error:      " << price-cachedPrice2);
    }

    yield = bond2.yield(marketPrice2);
    if (std::fabs(yield-cachedYield2) > tolerance) {
        BOOST_FAIL("failed to reproduce cached yield:\n"
                   << std::setprecision(4)
                   << "    calculated: " << io::rate(yield) << "\n"
                   << "    expected:   " << io::rate(cachedYield2) << "\n"
                   << "    error:      " << io::rate(yield-cachedYield2));
    }

    // with explicit settlement date:

    FixedCouponBond bond3(Date(30,November,2004),
                          Date(30,November,2004),
                          Date(30,November,2006),
                          settlementDays,
                          0.02875, Semiannual,
                          ActualActual(ActualActual::ISMA),
                          UnitedStates(UnitedStates::Exchange),
                          Unadjusted, 100.0);

    Rate marketYield3 = 0.02997;

    Date settlementDate = Date(30,November,2004);
    Real cachedPrice3 = 99.764874;

    price = bond3.cleanPrice(marketYield3, settlementDate);
    if (std::fabs(price-cachedPrice3) > tolerance) {
        BOOST_FAIL("failed to reproduce cached price:\n"
                   << "    calculated: " << price << "\n"
                   << "    expected:   " << cachedPrice3 << "\n"
                   << "    error:      " << price-cachedPrice3);
    }

    // this should give the same result since the issue date is the
    // earliest possible settlement date

    Settings::instance().setEvaluationDate(Date(22,November,2004));

    price = bond3.cleanPrice(marketYield3);
    if (std::fabs(price-cachedPrice3) > tolerance) {
        BOOST_FAIL("failed to reproduce cached price:\n"
                   << "    calculated: " << price << "\n"
                   << "    expected:   " << cachedPrice3 << "\n"
                   << "    error:      " << price-cachedPrice3);
    }

    QL_TEST_TEARDOWN
}


test_suite* BondTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Bond tests");

    suite->add(BOOST_TEST_CASE(&BondTest::testYield));
    suite->add(BOOST_TEST_CASE(&BondTest::testCachedYield));
    return suite;
}

