/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 StatPro Italia srl

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/cashflows/cdicoupon.hpp>
#include <ql/indexes/ibor/cdi.hpp>
#include <ql/settings.hpp>
#include <ql/time/daycounters/business252.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CdiCouponTests)

struct CommonVars {
    Date today;
    const Real notional = 100000.0;
    const Real cdiForeCast = 0.015;
    ext::shared_ptr<Cdi> cdi;
    RelinkableHandle<YieldTermStructure> forecastCurve;

    ext::shared_ptr<CdiCoupon>
    makeCoupon(Date startDate, Date endDate, Real gearing = 1.0, Real spread = 0.0) {
        return ext::make_shared<CdiCoupon>(endDate, notional, startDate, endDate, cdi, gearing,
                                           spread);
    }

    CommonVars(const Date& evaluationDate) {
        today = evaluationDate;

        Settings::instance().evaluationDate() = today;

        cdi = ext::make_shared<Cdi>(forecastCurve);

        std::vector<Date> pastDates = {
            Date(29, April, 2025), Date(30, April, 2025), Date(2, May, 2025),  Date(5, May, 2025),
            Date(6, May, 2025),    Date(7, May, 2025),    Date(8, May, 2025),  Date(9, May, 2025),
            Date(12, May, 2025),   Date(13, May, 2025),   Date(14, May, 2025), Date(15, May, 2025),
            Date(16, May, 2025),   Date(19, May, 2025),   Date(20, May, 2025), Date(21, May, 2025),
            Date(22, May, 2025),   Date(23, May, 2025),   Date(26, May, 2025), Date(27, May, 2025),
            Date(28, May, 2025),   Date(29, May, 2025),
        };
        std::vector<Rate> pastRates = {
            0.1415, 0.1415, 0.1415, 0.1415, 0.1415, 0.1415, 0.1465, 0.1465, 0.1465, 0.1465, 0.1465,
            0.1465, 0.1465, 0.1465, 0.1465, 0.1465, 0.1465, 0.1465, 0.1465, 0.1465, 0.1465, 0.1465,
        };

        cdi->addFixings(pastDates.begin(), pastDates.end(), pastRates.begin());
    }

    CommonVars() : CommonVars(Date(30, May, 2025)) {}
};

#define CHECK_CDI_OIS_COUPON_RESULT(what, calculated, expected, tolerance)         \
    if (std::fabs(calculated - expected) > tolerance) {                            \
        BOOST_ERROR("Failed to reproduce " what ":"                                \
                    << "\n    expected:   " << std::setprecision(12) << expected   \
                    << "\n    calculated: " << std::setprecision(12) << calculated \
                    << "\n    error:      " << std::setprecision(12)               \
                    << std::fabs(calculated - expected));                          \
    }

BOOST_AUTO_TEST_CASE(testPastCouponRate) {
    BOOST_TEST_MESSAGE("Testing rate for past cdi-indexed coupon...");

    CommonVars vars;

    // coupon entirely in the past

    auto pastCoupon = vars.makeCoupon(Date(29, April, 2025), Date(30, May, 2025));
    auto pastCoupon2 = vars.makeCoupon(Date(29, April, 2025), Date(30, May, 2025), 1.7, 0.0045);

    // expected values here come from manual calculations while looking at bbg swpm
    Real expectedAccrued = 1190.15373385;
    Real expectedAccruedSpreadGearing = 2071.29478688;
    // Rate expectedRate = 0.000987136104;
    // Real expectedAmount = vars.notional * expectedRate * 31.0 / 360;
    // CHECK_CDI_OIS_COUPON_RESULT("coupon rate", pastCoupon->rate(), expectedRate, 1e-12);
    CHECK_CDI_OIS_COUPON_RESULT("coupon amount", pastCoupon->amount(), expectedAccrued, 1e-8);
    CHECK_CDI_OIS_COUPON_RESULT("coupon amount", pastCoupon2->amount(),
                                expectedAccruedSpreadGearing, 1e-4);
}

// BOOST_AUTO_TEST_CASE(testCurrentCouponRate) {
//     BOOST_TEST_MESSAGE("Testing rate for current overnight-indexed coupon...");
//
//     CommonVars vars;
//
//     vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));
//
//     // coupon partly in the past, today not fixed
//
//     auto currentCoupon = vars.makeCoupon(Date(10, November, 2021), Date(10, December, 2021));
//
//     Rate expectedRate = 0.000926701551;
//     Real expectedAmount = vars.notional * expectedRate * 30.0 / 360;
//     CHECK_CDI_OIS_COUPON_RESULT("coupon rate", currentCoupon->rate(), expectedRate, 1e-12);
//     CHECK_CDI_OIS_COUPON_RESULT("coupon amount", currentCoupon->amount(), expectedAmount, 1e-8);
//
//     // coupon partly in the past, today fixed
//
//     vars.cdi->addFixing(Date(23, November, 2021), 0.0007);
//
//     expectedRate = 0.000916700760;
//     expectedAmount = vars.notional * expectedRate * 30.0 / 360;
//     CHECK_CDI_OIS_COUPON_RESULT("coupon rate", currentCoupon->rate(), expectedRate, 1e-12);
//     CHECK_CDI_OIS_COUPON_RESULT("coupon amount", currentCoupon->amount(), expectedAmount, 1e-8);
// }
//
// BOOST_AUTO_TEST_CASE(testFutureCouponRate) {
//     BOOST_TEST_MESSAGE("Testing rate for future overnight-indexed coupon...");
//
//     CommonVars vars;
//
//     vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));
//
//     // coupon entirely in the future
//
//     auto futureCoupon = vars.makeCoupon(Date(10, December, 2021), Date(10, January, 2022));
//
//     Rate expectedRate = 0.001000043057;
//     Real expectedAmount = vars.notional * expectedRate * 31.0 / 360;
//     CHECK_CDI_OIS_COUPON_RESULT("coupon rate", futureCoupon->rate(), expectedRate, 1e-12);
//     CHECK_CDI_OIS_COUPON_RESULT("coupon amount", futureCoupon->amount(), expectedAmount, 1e-8);
// }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()