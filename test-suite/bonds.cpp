
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
    Compounding compounding[] = { Compounded, Continuous };

    for (Size i=0; i<LENGTH(issueMonths); i++) {
      for (Size j=0; j<LENGTH(lengths); j++) {
        for (Size k=0; k<LENGTH(coupons); k++) {
          for (Size l=0; l<LENGTH(frequencies); l++) {
            for (Size n=0; n<LENGTH(compounding); n++) {

              Date dated = calendar.advance(today, issueMonths[i], Months);
              Date issue = dated;
              Date maturity = calendar.advance(issue, lengths[j], Years);

              FixedCouponBond bond(issue, dated, maturity, settlementDays,
                                   coupons[k], frequencies[l],
                                   bondDayCount, calendar,
                                   convention, redemption);

              for (Size m=0; m<LENGTH(yields); m++) {

                Real price = bond.cleanPrice(yields[m],compounding[n]);
                Rate calculated = bond.yield(price, compounding[n], Date(),
                                             tolerance, maxEvaluations);

                if (std::fabs(yields[m]-calculated) > tolerance) {
                  // the difference might not matter
                  Real price2 = bond.cleanPrice(calculated,compounding[n]);
                  if (std::fabs(price-price2)/price > tolerance) {
                      BOOST_FAIL(
                          "yield recalculation failed:\n"
                          << "    issue:     " << issue << "\n"
                          << "    maturity:  " << maturity << "\n"
                          << "    coupon:    " << io::rate(coupons[k]) << "\n"
                          << "    frequency: " << frequencies[l] << "\n\n"
                          << "    yield:  " << io::rate(yields[m]) << " "
                          << (compounding[n] == Compounded ?
                              "compounded" : "continuous") << "\n"
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
    }

    QL_TEST_TEARDOWN
}



void BondTest::testTheoretical() {

    BOOST_MESSAGE("Testing theoretical bond price/yield calculation...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Real tolerance = 1.0e-7;
    Size maxEvaluations = 100;

    Integer lengths[] = { 3, 5, 10, 15, 20 };
    Integer settlementDays = 3;
    Real coupons[] = { 0.02, 0.05, 0.08 };
    Frequency frequencies[] = { Semiannual, Annual };
    DayCounter bondDayCount = Thirty360();
    BusinessDayConvention convention = ModifiedFollowing;
    Real redemption = 100.0;

    Rate yields[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    for (Size j=0; j<LENGTH(lengths); j++) {
      for (Size k=0; k<LENGTH(coupons); k++) {
        for (Size l=0; l<LENGTH(frequencies); l++) {

            Date dated = today;
            Date issue = dated;
            Date maturity = calendar.advance(issue, lengths[j], Years);

            boost::shared_ptr<SimpleQuote> rate(new SimpleQuote(0.0));
            Handle<YieldTermStructure> discountCurve(flatRate(today,rate,
                                                              bondDayCount));

            FixedCouponBond bond(issue, dated, maturity, settlementDays,
                                 coupons[k], frequencies[l],
                                 bondDayCount, calendar,
                                 convention, redemption,
                                 discountCurve);

            for (Size m=0; m<LENGTH(yields); m++) {

                rate->setValue(yields[m]);

                Real price = bond.cleanPrice(yields[m],Continuous);
                Real calculatedPrice = bond.cleanPrice();
                Rate calculatedYield = bond.yield(Continuous, tolerance,
                                                  maxEvaluations);

                if (std::fabs(price-calculatedPrice) > tolerance) {
                    BOOST_FAIL(
                        "price calculation failed:"
                        << "\n    issue:     " << issue
                        << "\n    maturity:  " << maturity
                        << "\n    coupon:    " << io::rate(coupons[k])
                        << "\n    frequency: " << frequencies[l] << "\n"
                        << "\n    yield:  " << io::rate(yields[m])
                        << std::setprecision(7)
                        << "\n    expected:    " << price
                        << "\n    calculated': " << calculatedPrice
                        << "\n    error':      " << price-calculatedPrice);
                }

                if (std::fabs(yields[m]-calculatedYield) > tolerance) {
                    BOOST_FAIL(
                        "yield calculation failed:"
                        << "\n    issue:     " << issue
                        << "\n    maturity:  " << maturity
                        << "\n    coupon:    " << io::rate(coupons[k])
                        << "\n    frequency: " << frequencies[l] << "\n"
                        << "\n    yield:  " << io::rate(yields[m])
                        << std::setprecision(7)
                        << "\n    price:  " << price
                        << "\n    yield': " << io::rate(calculatedYield));
                }
            }
        }
      }
    }

    QL_TEST_TEARDOWN
}


void BondTest::testCached() {

    BOOST_MESSAGE(
        "Testing bond price/yield calculation against cached values...");

    QL_TEST_BEGIN

    // with implicit settlement calculation:

    Date today(22,November,2004);
    Settings::instance().setEvaluationDate(today);

    Calendar bondCalendar = NullCalendar();
    DayCounter bondDayCount = ActualActual(ActualActual::ISMA);
    Integer settlementDays = 1;

    Handle<YieldTermStructure> discountCurve(flatRate(today,0.03,Actual360()));

    // actual market values from the evaluation date

    FixedCouponBond bond1(Date(1,November,2004),
                          Date(31,October,2004),
                          Date(31,October,2006),
                          settlementDays,
                          0.025, Semiannual,
                          bondDayCount, bondCalendar,
                          Unadjusted, 100.0,
                          discountCurve);

    Real marketPrice1 = 99.203125;
    Rate marketYield1 = 0.02925;

    FixedCouponBond bond2(Date(15,November,2004),
                          Date(15,November,2004),
                          Date(15,November,2009),
                          settlementDays,
                          0.035, Semiannual,
                          bondDayCount, bondCalendar,
                          Unadjusted, 100.0,
                          discountCurve);

    Real marketPrice2 = 99.6875;
    Rate marketYield2 = 0.03569;

    // calculated values

    Real cachedPrice1a = 99.204505, cachedPrice2a = 99.687192;
    Real cachedPrice1b = 98.943393, cachedPrice2b = 101.986794;
    Rate cachedYield1a = 0.029257, cachedYield2a = 0.035689;
    Rate cachedYield1b = 0.029045, cachedYield2b = 0.035375;
    Rate cachedYield1c = 0.030423, cachedYield2c = 0.030432;

    // check
    Real tolerance = 1.0e-6;
    Real price, yield;

    price = bond1.cleanPrice(marketYield1, Compounded);
    if (std::fabs(price-cachedPrice1a) > tolerance) {
        BOOST_FAIL("failed to reproduce cached price:\n"
                   << std::fixed
                   << "    calculated: " << price << "\n"
                   << "    expected:   " << cachedPrice1a << "\n"
                   << "    error:      " << price-cachedPrice1a);
    }

    price = bond1.cleanPrice();
    if (std::fabs(price-cachedPrice1b) > tolerance) {
        BOOST_FAIL("failed to reproduce cached price:\n"
                   << std::fixed
                   << "    calculated: " << price << "\n"
                   << "    expected:   " << cachedPrice1b << "\n"
                   << "    error:      " << price-cachedPrice1b);
    }

    yield = bond1.yield(marketPrice1, Compounded);
    if (std::fabs(yield-cachedYield1a) > tolerance) {
        BOOST_FAIL("failed to reproduce cached compounded yield:\n"
                   << std::setprecision(4)
                   << "    calculated: " << io::rate(yield) << "\n"
                   << "    expected:   " << io::rate(cachedYield1a) << "\n"
                   << "    error:      " << io::rate(yield-cachedYield1a));
    }

    yield = bond1.yield(marketPrice1, Continuous);
    if (std::fabs(yield-cachedYield1b) > tolerance) {
        BOOST_FAIL("failed to reproduce cached continuous yield:\n"
                   << std::setprecision(4)
                   << "    calculated: " << io::rate(yield) << "\n"
                   << "    expected:   " << io::rate(cachedYield1b) << "\n"
                   << "    error:      " << io::rate(yield-cachedYield1b));
    }

    yield = bond1.yield(Continuous);
    if (std::fabs(yield-cachedYield1c) > tolerance) {
        BOOST_FAIL("failed to reproduce cached continuous yield:\n"
                   << std::setprecision(4)
                   << "    calculated: " << io::rate(yield) << "\n"
                   << "    expected:   " << io::rate(cachedYield1c) << "\n"
                   << "    error:      " << io::rate(yield-cachedYield1c));
    }


    price = bond2.cleanPrice(marketYield2, Compounded);
    if (std::fabs(price-cachedPrice2a) > tolerance) {
        BOOST_FAIL("failed to reproduce cached price:\n"
                   << std::fixed
                   << "    calculated: " << price << "\n"
                   << "    expected:   " << cachedPrice2a << "\n"
                   << "    error:      " << price-cachedPrice2a);
    }

    price = bond2.cleanPrice();
    if (std::fabs(price-cachedPrice2b) > tolerance) {
        BOOST_FAIL("failed to reproduce cached price:\n"
                   << std::fixed
                   << "    calculated: " << price << "\n"
                   << "    expected:   " << cachedPrice2b << "\n"
                   << "    error:      " << price-cachedPrice2b);
    }

    yield = bond2.yield(marketPrice2, Compounded);
    if (std::fabs(yield-cachedYield2a) > tolerance) {
        BOOST_FAIL("failed to reproduce cached compounded yield:\n"
                   << std::setprecision(4)
                   << "    calculated: " << io::rate(yield) << "\n"
                   << "    expected:   " << io::rate(cachedYield2a) << "\n"
                   << "    error:      " << io::rate(yield-cachedYield2a));
    }

    yield = bond2.yield(marketPrice2, Continuous);
    if (std::fabs(yield-cachedYield2b) > tolerance) {
        BOOST_FAIL("failed to reproduce cached continuous yield:\n"
                   << std::setprecision(4)
                   << "    calculated: " << io::rate(yield) << "\n"
                   << "    expected:   " << io::rate(cachedYield2b) << "\n"
                   << "    error:      " << io::rate(yield-cachedYield2b));
    }

    yield = bond2.yield(Continuous);
    if (std::fabs(yield-cachedYield2c) > tolerance) {
        BOOST_FAIL("failed to reproduce cached continuous yield:\n"
                   << std::setprecision(4)
                   << "    calculated: " << io::rate(yield) << "\n"
                   << "    expected:   " << io::rate(cachedYield2c) << "\n"
                   << "    error:      " << io::rate(yield-cachedYield2c));
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

    price = bond3.cleanPrice(marketYield3, Compounded, settlementDate);
    if (std::fabs(price-cachedPrice3) > tolerance) {
        BOOST_FAIL("failed to reproduce cached price:\n"
                   << std::fixed
                   << "    calculated: " << price << "\n"
                   << "    expected:   " << cachedPrice3 << "\n"
                   << "    error:      " << price-cachedPrice3);
    }

    // this should give the same result since the issue date is the
    // earliest possible settlement date

    Settings::instance().setEvaluationDate(Date(22,November,2004));

    price = bond3.cleanPrice(marketYield3, Compounded);
    if (std::fabs(price-cachedPrice3) > tolerance) {
        BOOST_FAIL("failed to reproduce cached price:\n"
                   << std::fixed
                   << "    calculated: " << price << "\n"
                   << "    expected:   " << cachedPrice3 << "\n"
                   << "    error:      " << price-cachedPrice3);
    }

    QL_TEST_TEARDOWN
}


test_suite* BondTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Bond tests");

    suite->add(BOOST_TEST_CASE(&BondTest::testYield));
    suite->add(BOOST_TEST_CASE(&BondTest::testTheoretical));
    suite->add(BOOST_TEST_CASE(&BondTest::testCached));
    return suite;
}

