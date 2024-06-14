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
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/settings.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(OvernightIndexedCouponTests)

struct CommonVars {
    Date today;
    Real notional = 10000.0;
    ext::shared_ptr<OvernightIndex> sofr;
    RelinkableHandle<YieldTermStructure> forecastCurve;

    ext::shared_ptr<OvernightIndexedCoupon> makeCoupon(Date startDate,
                                                       Date endDate,
                                                       Natural fixingDays = Null<Natural>(),
                                                       Natural lockoutDays = 0,
                                                       bool applyObservationShift = false,
                                                       bool telescopicValueDates = false,
                                                       RateAveraging::Type averaging = RateAveraging::Compound) {
        return ext::make_shared<OvernightIndexedCoupon>(
            endDate, notional, startDate, endDate, sofr, 1.0, 0.0, Date(), Date(), DayCounter(),
            telescopicValueDates, averaging, fixingDays, lockoutDays, applyObservationShift);
    }

    CommonVars(const Date& evaluationDate) {
        today = evaluationDate;

        Settings::instance().evaluationDate() = today;

        sofr = ext::make_shared<Sofr>(forecastCurve);

        std::vector<Date> pastDates = {
            Date(21, June, 2019),    Date(24, June, 2019),    Date(25, June, 2019),
            Date(26, June, 2019),    Date(27, June, 2019),    Date(28, June, 2019),
            Date(1, July, 2019),     Date(2, July, 2019),     Date(3, July, 2019),
            Date(5, July, 2019),     Date(8, July, 2019),     Date(9, July, 2019),
            Date(10, July, 2019),    Date(11, July, 2019),    Date(12, July, 2019),
            Date(15, July, 2019),    Date(16, July, 2019),    Date(17, July, 2019),
            Date(18, July, 2019),    Date(19, July, 2019),    Date(22, July, 2019),
            Date(23, July, 2019),    Date(24, July, 2019),    Date(25, July, 2019),
            Date(26, July, 2019),    Date(29, July, 2019),    Date(30, July, 2019),
            Date(31, July, 2019),    Date(1, August, 2019),   Date(2, August, 2019),
            Date(5, August, 2019),

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
            0.0237, 0.0239, 0.0241, 
            0.0243, 0.0242, 0.025,  
            0.0242, 0.0251, 0.0256, 
            0.0259, 0.0248, 0.0245, 
            0.0246, 0.0241, 0.0236, 
            0.0246, 0.0247, 0.0247, 
            0.0246, 0.0241, 0.024,  
            0.024,  0.0241, 0.0242, 
            0.0241, 0.024,  0.0239, 
            0.0255, 0.0219, 0.0219, 
            0.0213,

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

    CommonVars() : CommonVars(Date(23, November, 2021)) {}
};

#define CHECK_OIS_COUPON_RESULT(what, calculated, expected, tolerance)   \
    if (std::fabs(calculated-expected) > tolerance) { \
        BOOST_ERROR("Failed to reproduce " what ":" \
                    << "\n    expected:   " << std::setprecision(12) << expected \
                    << "\n    calculated: " << std::setprecision(12) << calculated \
                    << "\n    error:      " << std::setprecision(12) << std::fabs(calculated-expected)); \
    }

BOOST_AUTO_TEST_CASE(testPastCouponRate) {
    BOOST_TEST_MESSAGE("Testing rate for past overnight-indexed coupon...");

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

BOOST_AUTO_TEST_CASE(testCurrentCouponRate) {
    BOOST_TEST_MESSAGE("Testing rate for current overnight-indexed coupon...");

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

BOOST_AUTO_TEST_CASE(testFutureCouponRate) {
    BOOST_TEST_MESSAGE("Testing rate for future overnight-indexed coupon...");

    CommonVars vars;

    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    // coupon entirely in the future

    auto futureCoupon = vars.makeCoupon(Date(10, December, 2021),
                                        Date(10, January, 2022));

    Rate expectedRate = 0.001000043057;
    Real expectedAmount = vars.notional * expectedRate * 31.0/360;
    CHECK_OIS_COUPON_RESULT("coupon rate", futureCoupon->rate(), expectedRate, 1e-12);
    CHECK_OIS_COUPON_RESULT("coupon amount", futureCoupon->amount(), expectedAmount, 1e-8);
}

BOOST_AUTO_TEST_CASE(testRateWhenTodayIsHoliday) {
    BOOST_TEST_MESSAGE("Testing rate for overnight-indexed coupon when today is a holiday...");

    CommonVars vars;

    Settings::instance().evaluationDate() = Date(20, November, 2021);

    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    auto coupon = vars.makeCoupon(Date(10, November, 2021),
                                  Date(10, December, 2021));

    Rate expectedRate = 0.000930035180;
    Real expectedAmount = vars.notional * expectedRate * 30.0/360;
    CHECK_OIS_COUPON_RESULT("coupon rate", coupon->rate(), expectedRate, 1e-12);
    CHECK_OIS_COUPON_RESULT("coupon amount", coupon->amount(), expectedAmount, 1e-8);
}

BOOST_AUTO_TEST_CASE(testAccruedAmountInThePast) {
    BOOST_TEST_MESSAGE("Testing accrued amount in the past for overnight-indexed coupon...");

    CommonVars vars;

    auto coupon = vars.makeCoupon(Date(18, October, 2021),
                                  Date(18, January, 2022));

    Real expectedAmount = vars.notional * 0.000987136104 * 31.0/360;
    CHECK_OIS_COUPON_RESULT("coupon amount", coupon->accruedAmount(Date(18, November, 2021)), expectedAmount, 1e-8);
}

BOOST_AUTO_TEST_CASE(testAccruedAmountSpanningToday) {
    BOOST_TEST_MESSAGE("Testing accrued amount spanning today for current overnight-indexed coupon...");

    CommonVars vars;

    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    // coupon partly in the past, today not fixed

    auto coupon = vars.makeCoupon(Date(10, November, 2021),
                                  Date(10, January, 2022));

    Real expectedAmount = vars.notional * 0.000926701551 * 30.0/360;
    CHECK_OIS_COUPON_RESULT("coupon amount", coupon->accruedAmount(Date(10, December, 2021)), expectedAmount, 1e-8);

    // coupon partly in the past, today fixed

    vars.sofr->addFixing(Date(23, November, 2021), 0.0007);

    expectedAmount = vars.notional * 0.000916700760 * 30.0/360;
    CHECK_OIS_COUPON_RESULT("coupon amount", coupon->accruedAmount(Date(10, December, 2021)), expectedAmount, 1e-8);
}

BOOST_AUTO_TEST_CASE(testAccruedAmountInTheFuture) {
    BOOST_TEST_MESSAGE("Testing accrued amount in the future for overnight-indexed coupon...");

    CommonVars vars;

    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    // coupon entirely in the future

    auto coupon = vars.makeCoupon(Date(10, December, 2021),
                                  Date(10, March, 2022));

    Date accrualDate = Date(10, January, 2022);
    Rate expectedRate = 0.001000043057;
    Real expectedAmount = vars.notional * expectedRate * 31.0/360;
    CHECK_OIS_COUPON_RESULT("coupon amount", coupon->accruedAmount(accrualDate), expectedAmount, 1e-8);
}

BOOST_AUTO_TEST_CASE(testAccruedAmountOnPastHoliday) {
    BOOST_TEST_MESSAGE("Testing accrued amount on a past holiday for overnight-indexed coupon...");

    CommonVars vars;

    // coupon entirely in the past

    auto coupon = vars.makeCoupon(Date(18, October, 2021),
                                  Date(18, January, 2022));

    Date accrualDate = Date(13, November, 2021);
    Real expectedAmount = vars.notional * 0.000074724810;
    CHECK_OIS_COUPON_RESULT("coupon amount", coupon->accruedAmount(accrualDate), expectedAmount, 1e-8);
}
BOOST_AUTO_TEST_CASE(testAccruedAmountOnFutureHoliday) {
    BOOST_TEST_MESSAGE("Testing accrued amount on a future holiday for overnight-indexed coupon...");

    CommonVars vars;

    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    // coupon entirely in the future

    auto coupon = vars.makeCoupon(Date(10, December, 2021),
                                  Date(10, March, 2022));

    Date accrualDate = Date(15, January, 2022);
    Real expectedAmount = vars.notional * 0.000100005012;
    CHECK_OIS_COUPON_RESULT("coupon amount", coupon->accruedAmount(accrualDate), expectedAmount, 1e-8);
}
BOOST_AUTO_TEST_CASE(testPastCouponRateWithLookback) {
    BOOST_TEST_MESSAGE("Testing rate for past overnight-indexed coupon with lookback period...");

    CommonVars vars;

    // coupon entirely in the past with lookback period
    // this unit test replicates an example available on NY FED website:
    // https://www.newyorkfed.org/medialibrary/Microsites/arrc/files/2020/ARRC-BWLG-Examples-Other-Lookback-Options.xlsx

    auto pastCoupon = vars.makeCoupon(Date(1, July, 2019), Date(15, July, 2019), 5);

    // expected values here and below come from manual calculations based on past dates and rates
    Rate expectedRate = 0.024781644454;

    CHECK_OIS_COUPON_RESULT("coupon rate", pastCoupon->rate(), expectedRate, 1e-12);
}

BOOST_AUTO_TEST_CASE(testPastCouponRateWithLookbackAndObservationShift) {
    BOOST_TEST_MESSAGE("Testing rate for past overnight-indexed coupon with lookback period and "
                       "observation shift...");

    CommonVars vars;

    // coupon entirely in the past with lookback period with observation shift
    // this unit test replicates an example available on NY FED website:
    // https://www.newyorkfed.org/medialibrary/Microsites/arrc/files/2020/ARRC-BWLG-Examples-Other-Lookback-Options.xlsx

    auto pastCoupon =
        vars.makeCoupon(Date(1, July, 2019), Date(31, July, 2019), 5, 0, true);

    // expected values here and below come from manual calculations based on past dates and rates
    Rate expectedRate = 0.024603611707;

    CHECK_OIS_COUPON_RESULT("coupon rate", pastCoupon->rate(), expectedRate, 1e-12);
}

#define CHECK_OIS_COUPON_DATES(what, actual, expected)   \
    if (actual != expected) {                            \
        BOOST_ERROR("Failed to reproduce " what ":"      \
                    << "\n    expected:   " << expected  \
                    << "\n    actual: " << actual);      \
    }

BOOST_AUTO_TEST_CASE(testPastCouponRateWithLockout) {
    BOOST_TEST_MESSAGE("Testing rate for past overnight-indexed coupon with lockout...");

    CommonVars vars;

    auto couponWithLockout =
        vars.makeCoupon(Date(1, July, 2019), Date(31, July, 2019), Null<Natural>(), 3);
    const std::vector<Date>& fixingDates = couponWithLockout->fixingDates();
    const Size n = fixingDates.size();

    Date expectedLockoutDate = Date(25, July, 2019);
    CHECK_OIS_COUPON_DATES("lockout date", fixingDates[n - 4], expectedLockoutDate);
    CHECK_OIS_COUPON_DATES("day T - 2 fixing", fixingDates[n - 3], expectedLockoutDate);
    CHECK_OIS_COUPON_DATES("day T - 1 fixing", fixingDates[n - 2], expectedLockoutDate);
    CHECK_OIS_COUPON_DATES("day T fixing", fixingDates[n - 1], expectedLockoutDate);
}

BOOST_AUTO_TEST_CASE(testPastCouponRateWithLookbackObservationShiftAndLockout) {
    BOOST_TEST_MESSAGE("Testing rate for past overnight-indexed coupon with lookback period, "
                       "observation shift and lockout...");

    CommonVars vars;

    // coupon entirely in the past with lookback period with observation shift
    // and lockout this unit test replicates an example available on NY FED website:
    // https://www.newyorkfed.org/medialibrary/Microsites/arrc/files/2020/ARRC-BWLG-Examples-Other-Lookback-Options.xlsx

    auto pastCoupon = vars.makeCoupon(Date(1, July, 2019), Date(31, July, 2019), 5, 3, true);

    // expected values here and below come from manual calculations based on past dates and rates
    Rate expectedRate = 0.024693783702;

    CHECK_OIS_COUPON_RESULT("coupon rate", pastCoupon->rate(), expectedRate, 1e-12);
}

BOOST_AUTO_TEST_CASE(testIncorrectNumberOfLockoutDays) {
    BOOST_TEST_MESSAGE("Testing incorrect number of lockout days...");

    CommonVars vars;

    auto couponWithoutLockout = vars.makeCoupon(Date(1, July, 2019), Date(31, July, 2019));
    const Size numberOfFixings = couponWithoutLockout->fixingDates().size();

    // Lockout days equal to the number of daily fixings
    BOOST_CHECK_THROW(vars.makeCoupon(Date(1, July, 2019), Date(31, July, 2019), Null<Natural>(),
                                      numberOfFixings),
                      Error);

    // Negative number of lockout days
    BOOST_CHECK_THROW(vars.makeCoupon(Date(1, July, 2019), Date(31, July, 2019), Null<Natural>(), -1),
                      Error);
}

BOOST_AUTO_TEST_CASE(testFutureCouponRateWithLookback) {
    BOOST_TEST_MESSAGE("Testing rate for future overnight-indexed coupon with lookback period...");

    CommonVars vars(Date(12, March, 2019));

    vars.forecastCurve.linkTo(flatRate(0.0250, Actual360()));

    auto coupon8July = vars.makeCoupon(Date(1, July, 2019), Date(8, July, 2019), 5, 0, false);
    Rate expectedRate8July = 0.0250050849311315;

    CHECK_OIS_COUPON_RESULT("coupon rate", coupon8July->rate(), expectedRate8July, 1e-12);

    auto coupon15July = vars.makeCoupon(Date(1, July, 2019), Date(15, July, 2019), 5, 0, false);
    Rate expectedRate15July = 0.0250118464503275;

    CHECK_OIS_COUPON_RESULT("coupon rate", coupon15July->rate(), expectedRate15July, 1e-12);
}

BOOST_AUTO_TEST_CASE(testFutureCouponRateWithLookbackAndObservationShift) {
    BOOST_TEST_MESSAGE("Testing rate for future overnight-indexed coupon with lookback period and "
                       "observation shift...");

    CommonVars vars(Date(12, March, 2019));

    vars.forecastCurve.linkTo(flatRate(0.0250, Actual360()));

    auto futureCoupon = vars.makeCoupon(Date(1, July, 2019), Date(8, July, 2019), 5, 0, true);

    // The discrepancies between the results with and without observation shift,
    // especially in the short term horizon, show that interest-period weighted 
    // overnight cumulative compounded annualized rates are considered as an
    // alternative.
    Rate expectedRate = 0.0142876985964208;

    CHECK_OIS_COUPON_RESULT("coupon rate", futureCoupon->rate(), expectedRate, 1e-12);
}

BOOST_AUTO_TEST_CASE(testFutureCouponRateWithLookout) {
    BOOST_TEST_MESSAGE("Testing rate for future overnight-indexed coupon with lockout...");

    CommonVars vars(Date(12, March, 2019));

    vars.forecastCurve.linkTo(flatRate(0.0250, Actual360()));

    auto coupon15July =
        vars.makeCoupon(Date(1, July, 2019), Date(15, July, 2019), Null<Natural>(), 2, false);
    
    // expected = 0.025011784374543
    Rate lockoutFixing = vars.sofr->fixing(Date(10, July, 2019));
    Rate expectedRate15July =
        (vars.forecastCurve->discount(Date(1, July, 2019)) /
             vars.forecastCurve->discount(Date(11, July, 2019)) *
             (1.0 + 1.0 / 360.0 * lockoutFixing) * (1.0 + 3.0 / 360.0 * lockoutFixing) -
         1.0) *
        360.0 / 14;

    CHECK_OIS_COUPON_RESULT("coupon rate", coupon15July->rate(), expectedRate15July, 1e-12);
}

BOOST_AUTO_TEST_CASE(testPartiallyAccruedAmountOfFutureCouponWithLookout) {
    BOOST_TEST_MESSAGE(
        "Testing partially accrued amount for future overnight-indexed coupon with lockout...");

    CommonVars vars(Date(12, March, 2019));

    vars.forecastCurve.linkTo(flatRate(0.0250, Actual360()));

    auto coupon15July =
        vars.makeCoupon(Date(1, July, 2019), Date(15, July, 2019), Null<Natural>(), 2, false);

    Rate lockoutFixing = vars.sofr->fixing(Date(10, July, 2019));
    Rate expectedRate15July =
        (vars.forecastCurve->discount(Date(1, July, 2019)) /
             vars.forecastCurve->discount(Date(11, July, 2019)) *
             (1.0 + 1.0 / 360.0 * lockoutFixing) * (1.0 + 2.0 / 360.0 * lockoutFixing) -
         1.0) *
        360.0 / 13;

    Real expectedAccruedAmount = coupon15July->nominal() *
                                 coupon15July->accruedPeriod(Date(14, July, 2019)) *
                                 expectedRate15July;
    
    CHECK_OIS_COUPON_RESULT("accrued amount", coupon15July->accruedAmount(Date(14, July, 2019)),
                            expectedAccruedAmount, 1e-12);
}

BOOST_AUTO_TEST_CASE(testTelescopicFormulaWhenLookbackWithObservationShiftAndNoIndexFixingDelay) {
    BOOST_TEST_MESSAGE("Testing telescopic formula when lookback with observation shift is applied "
                       "and the index has no fixing delay...");

    CommonVars vars(Date(12, March, 2019));

    vars.forecastCurve.linkTo(flatRate(0.0250, Actual360()));

    auto coupon15July =
        vars.makeCoupon(Date(1, July, 2019), Date(15, July, 2019), 3, 0, true);

    Rate actualRate = coupon15July->rate();

    Rate expectedRateTelescopicSeries =
        (vars.forecastCurve->discount(Date(26, June, 2019)) /
             vars.forecastCurve->discount(Date(10, July, 2019)) - 1.0) * 360.0 / 14;

    auto& fixingDates = coupon15July->fixingDates();
    auto& dts = coupon15July->dt();
    Size n = fixingDates.size();
    
    Rate expectedRateIterativeFormula = 1.0;
    for (Size i = 0; i < n; ++i) {
        expectedRateIterativeFormula *=
            (1.0 + dts[i] * coupon15July->index()->fixing(fixingDates[i]));
	}
    expectedRateIterativeFormula -= 1.0;
    expectedRateIterativeFormula /= coupon15July->accrualPeriod();

    CHECK_OIS_COUPON_RESULT("coupon rate using telescopic formula", actualRate,
                            expectedRateTelescopicSeries, 1e-12);

    CHECK_OIS_COUPON_RESULT("coupon rate using iterative formula", actualRate,
                            expectedRateIterativeFormula, 1e-12);


}

BOOST_AUTO_TEST_CASE(testErrorWhenTelescopicSeriesEnforcedWithLookback) {
    BOOST_TEST_MESSAGE("Testing error when telescopic series enforced with lookback...");

    CommonVars vars;

    BOOST_CHECK_THROW(vars.makeCoupon(Date(1, July, 2019), Date(31, July, 2019), 2, 0, false, true),
                      Error);
}

BOOST_AUTO_TEST_CASE(testErrorWhenLookbackOrLockoutAppliedForSimpleAveraging) {
    BOOST_TEST_MESSAGE("Testing error when lookback or lockout applied for simple averaging...");

    CommonVars vars;

    BOOST_CHECK_THROW(vars.makeCoupon(Date(1, July, 2019), Date(31, July, 2019), 2, 0, false, false,
                                      RateAveraging::Simple),
                      Error);

    BOOST_CHECK_THROW(vars.makeCoupon(Date(1, July, 2019), Date(31, July, 2019), Null<Natural>(), 2,
                                      false, false, RateAveraging::Simple),
                      Error);

    BOOST_CHECK_THROW(vars.makeCoupon(Date(1, July, 2019), Date(31, July, 2019), Null<Natural>(), 0,
                                      true, false, RateAveraging::Simple),
                      Error);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
