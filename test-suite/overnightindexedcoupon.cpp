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

#include "overnightindexedcoupon.hpp"
#include "utilities.hpp"
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/settings.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace overnight_indexed_coupon_tests {

    struct CommonVars {
        // cleanup
        SavedSettings backup;
        IndexHistoryCleaner cleaner;

        Date today;
        Real notional = 10000.0;
        ext::shared_ptr<OvernightIndex> sofr;
        RelinkableHandle<YieldTermStructure> forecastCurve;

        ext::shared_ptr<OvernightIndexedCoupon> makeCoupon(Date startDate, Date endDate) {
            return ext::make_shared<OvernightIndexedCoupon>(endDate, notional, startDate, endDate, sofr);
        }

        CommonVars() {
            today = Date(23, November, 2021);

            Settings::instance().evaluationDate() = today;

            sofr = ext::make_shared<Sofr>(forecastCurve);

            std::vector<Date> pastDates = {
                Date(18, October, 2021), Date(19, October, 2021), Date(20, October, 2021),
                Date(21, October, 2021), Date(22, October, 2021), Date(25, October, 2021),
                Date(26, October, 2021), Date(27, October, 2021), Date(28, October, 2021),
                Date(29, October, 2021), Date(1, November, 2021), Date(2, November, 2021),
                Date(3, November, 2021), Date(4, November, 2021), Date(5, November, 2021),
                Date(8, November, 2021), Date(9, November, 2021), Date(10, November, 2021),
                Date(12, November, 2021), Date(15, November, 2021), Date(16, November, 2021),
                Date(17, November, 2021), Date(18, November, 2021), Date(19, November, 2021),
                Date(22, November, 2021)
            };
            std::vector<Rate> pastRates = {
                0.0008, 0.0009, 0.0008,
                0.0010, 0.0012, 0.0011,
                0.0013, 0.0012, 0.0012,
                0.0008, 0.0009, 0.0010,
                0.0011, 0.0014, 0.0013,
                0.0011, 0.0009, 0.0008,
                0.0007, 0.0008, 0.0008,
                0.0007, 0.0009, 0.0010,
                0.0009
            };

            sofr->addFixings(pastDates.begin(), pastDates.end(), pastRates.begin());
        }
    };

}

#define CHECK_OIS_COUPON_RESULT(what, calculated, expected, tolerance)   \
    if (std::fabs(calculated-expected) > tolerance) { \
        BOOST_ERROR("Failed to reproduce " what ":" \
                    << "\n    expected:   " << std::setprecision(12) << expected \
                    << "\n    calculated: " << std::setprecision(12) << calculated \
                    << "\n    error:      " << std::setprecision(12) << std::fabs(calculated-expected)); \
    }
        
void OvernightIndexedCouponTest::testPastCouponRate() {
    BOOST_TEST_MESSAGE("Testing rates for past overnight-indexed coupon...");

    using namespace overnight_indexed_coupon_tests;

    CommonVars vars;

    // coupon entirely in the past

    auto pastCoupon = vars.makeCoupon(Date(18, October, 2021),
                                      Date(18, November, 2021));

    // expected values here and below come from manual calculations based on past dates and rates
    Rate expectedRate = 0.000987136104;
    Real expectedAmount = vars.notional * expectedRate * 31.0/360;
    CHECK_OIS_COUPON_RESULT("coupon rate", pastCoupon->rate(), expectedRate, 1e-12);
    CHECK_OIS_COUPON_RESULT("coupon amount", pastCoupon->amount(), expectedAmount, 1e-8);
}

void OvernightIndexedCouponTest::testCurrentCouponRate() {
    BOOST_TEST_MESSAGE("Testing rates for current overnight-indexed coupon...");

    using namespace overnight_indexed_coupon_tests;

    CommonVars vars;

    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    // coupon partly in the past, today not fixed

    auto currentCoupon = vars.makeCoupon(Date(10, November, 2021),
                                         Date(10, December, 2021));

    Rate expectedRate = 0.000926701551;
    Real expectedAmount = vars.notional * expectedRate * 30.0/360;
    CHECK_OIS_COUPON_RESULT("coupon rate", currentCoupon->rate(), expectedRate, 1e-12);
    CHECK_OIS_COUPON_RESULT("coupon amount", currentCoupon->amount(), expectedAmount, 1e-8);

    // coupon partly in the past, today fixed

    vars.sofr->addFixing(Date(23, November, 2021), 0.0007);

    expectedRate = 0.000916700760;
    expectedAmount = vars.notional * expectedRate * 30.0/360;
    CHECK_OIS_COUPON_RESULT("coupon rate", currentCoupon->rate(), expectedRate, 1e-12);
    CHECK_OIS_COUPON_RESULT("coupon amount", currentCoupon->amount(), expectedAmount, 1e-8);
}

void OvernightIndexedCouponTest::testFutureCouponRate() {
    BOOST_TEST_MESSAGE("Testing rates for future overnight-indexed coupon...");

    using namespace overnight_indexed_coupon_tests;

    CommonVars vars;

    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    // coupon entirely in the future

    Date startDate = Date(10, December, 2021);
    Date endDate = Date(10, January, 2022);

    auto futureCoupon = vars.makeCoupon(startDate, endDate);

    Rate expectedRate = vars.forecastCurve->forwardRate(startDate, endDate, vars.sofr->dayCounter(), Simple);
    Real expectedAmount = vars.notional * expectedRate * 31.0/360;
    CHECK_OIS_COUPON_RESULT("coupon rate", futureCoupon->rate(), expectedRate, 1e-12);
    CHECK_OIS_COUPON_RESULT("coupon amount", futureCoupon->amount(), expectedAmount, 1e-8);
}
    
test_suite* OvernightIndexedCouponTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Overnight-indexed coupon tests");
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedCouponTest::testPastCouponRate));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedCouponTest::testCurrentCouponRate));
    suite->add(QUANTLIB_TEST_CASE(&OvernightIndexedCouponTest::testFutureCouponRate));

    return suite;
}
