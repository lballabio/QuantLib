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
#include <ql/indexes/ibor/cdi.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/time/daycounters/business252.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CdiCouponTests)

namespace CdiTestData {

    Real fixing(const Date& date) {
        if (date <= Date(2, August, 2023))
            return 13.65;
        if (date <= Date(20, September, 2023))
            return 13.15;
        if (date <= Date(1, November, 2023))
            return 12.65;
        if (date <= Date(13, December, 2023))
            return 12.15;
        if (date <= Date(31, January, 2024))
            return 11.65;
        if (date <= Date(20, March, 2024))
            return 11.15;
        if (date <= Date(8, May, 2024))
            return 10.65;
        if (date <= Date(18, September, 2024))
            return 10.4;
        if (date <= Date(6, November, 2024))
            return 10.65;
        if (date <= Date(11, December, 2024))
            return 11.15;
        if (date <= Date(29, January, 2025))
            return 12.15;
        if (date <= Date(19, March, 2025))
            return 13.15;
        if (date <= Date(7, May, 2025))
            return 14.15;
        return 14.65;
    }

    void addFixings(const ext::shared_ptr<OvernightIndex>& index,
                    const Date& first,
                    const Date& last,
                    const Calendar& cal) {
        Schedule s = MakeSchedule()
                         .from(first)
                         .to(last)
                         .withTenor(1 * Days)
                         .withCalendar(cal)
                         .withConvention(Following)
                         .forwards()
                         .endOfMonth(false);
        for (const auto& d : s) {
            index->addFixing(d, fixing(d) / 100);
        }
    }

    std::vector<Date> curveDates(const Date& today) {
        return {today,
                Date(23, June, 2025),
                Date(1, July, 2025),
                Date(1, August, 2025),
                Date(1, September, 2025),
                Date(1, October, 2025),
                Date(3, November, 2025),
                Date(1, December, 2025),
                Date(2, January, 2026),
                Date(2, February, 2026),
                Date(2, March, 2026),
                Date(1, April, 2026),
                Date(4, May, 2026),
                Date(1, June, 2026),
                Date(1, July, 2026),
                Date(1, October, 2026),
                Date(4, January, 2027),
                Date(1, April, 2027),
                Date(1, July, 2027)};
    }

    std::vector<Rate>
    curveRates(const Date& today, const Date& first, const Date& second, const DayCounter& dc) {
        // some randomized rates with linear extrapolation for today's rate
        const Real r_1 = 0.14;
        const Real r_2 = 0.145;
        const Real r_0 =
            r_1 - dc.yearFraction(today, first) * (r_2 - r_1) / dc.yearFraction(first, second);
        return {r_0,     r_1,     r_2,     0.14512, 0.14683, 0.14614, 0.14707,
                0.14762, 0.14886, 0.15101, 0.14961, 0.14958, 0.15110, 0.14881,
                0.14942, 0.14782, 0.14641, 0.14573, 0.14293};
    }

    ext::shared_ptr<ZeroCurve> makeCurve(const Date& today) {
        const auto dc = Business252();
        const auto dates = curveDates(today);
        const auto rates = curveRates(today, dates[0], dates[1], dc);
        return ext::make_shared<InterpolatedZeroCurve<Linear>>(dates, rates, dc, Calendar(),
                                                               Linear(), Compounded, Annual);
    }

}

struct CommonVars {
    const Calendar calendar = Brazil();
    const DayCounter dc = Business252();
    const Date today = Date(19, June, 2025); // holiday
    const Date start = Date(23, June, 2023);
    const Real notional = 10000000.0;
    const Rate fixedRate = 14.2103910923;

    ext::shared_ptr<Cdi> cdi;
    RelinkableHandle<YieldTermStructure> forecastCurve;

    ext::shared_ptr<OvernightIndexedCoupon>
    makeCoupon(Date startDate, Date endDate, Real gearing = 1.0, Real spread = 0.0) {
        return ext::make_shared<OvernightIndexedCoupon>(endDate, notional, startDate, endDate, cdi,
                                                        gearing, spread);
    }

    CommonVars() {
        Settings::instance().evaluationDate() = today;
        cdi = ext::make_shared<Cdi>(forecastCurve);
        Date lastFixingDate = calendar.adjust(today, Preceding);
        CdiTestData::addFixings(cdi, start, lastFixingDate, calendar);
    }
};

#define CHECK_CDI_OIS_COUPON_RESULT(what, calculated, expected, tolerance)         \
    if (std::fabs(calculated - expected) > tolerance) {                            \
        BOOST_ERROR("Failed to reproduce " what ":"                                \
                    << "\n    expected:   " << std::setprecision(12) << expected   \
                    << "\n    calculated: " << std::setprecision(12) << calculated \
                    << "\n    error:      " << std::setprecision(12)               \
                    << std::fabs(calculated - expected));                          \
    }

BOOST_AUTO_TEST_CASE(testPastCoupon) {
    BOOST_TEST_MESSAGE("Testing rate for cdi-indexed coupon in the past...");

    CommonVars vars;

    // coupon entirely in the past

    const Date end = Date(18, June, 2025);

    auto coupon1 = vars.makeCoupon(vars.start, end);
    auto coupon2 = vars.makeCoupon(vars.start, end, 1.1, 0.005);
    auto coupon3 = vars.makeCoupon(vars.start, end, 0.6, -0.003);

    // expected values here come from manual calculations while looking at BBG SWPM
    Real expectedAccrued1 = 2507099.48795;
    Real expectedAccrued2 = 2916664.74186;
    Real expectedAccrued3 = 1368957.56541;
    CHECK_CDI_OIS_COUPON_RESULT("coupon amount", coupon1->amount(), expectedAccrued1, 1e-5);
    CHECK_CDI_OIS_COUPON_RESULT("coupon amount", coupon2->amount(), expectedAccrued2, 1e-5);
    CHECK_CDI_OIS_COUPON_RESULT("coupon amount", coupon3->amount(), expectedAccrued3, 1e-5);
    CHECK_CDI_OIS_COUPON_RESULT("accrued amount", coupon1->accruedAmount(end), expectedAccrued1,
                                1e-5);
    CHECK_CDI_OIS_COUPON_RESULT("accrued amount", coupon2->accruedAmount(end), expectedAccrued2,
                                1e-5);
    CHECK_CDI_OIS_COUPON_RESULT("accrued amount", coupon3->accruedAmount(end), expectedAccrued3,
                                1e-5);
}

BOOST_AUTO_TEST_CASE(testCurrentCoupon) {
    BOOST_TEST_MESSAGE("Testing rate for cdi-indexed coupon...");

    CommonVars vars;

    const auto curve = CdiTestData::makeCurve(vars.today);
    vars.forecastCurve.linkTo(curve);

    // coupon partly in the past, today not fixed

    const Date end = Date(23, June, 2027);

    auto coupon1 = vars.makeCoupon(vars.start, end);
    auto coupon2 = vars.makeCoupon(vars.start, end, 1.1, 0.005);
    auto coupon3 = vars.makeCoupon(vars.start, end, 0.6, -0.003);

    // expected values here come from manual calculations while looking at BBG SWPM
    Real expAccrued1 = 2513886.62007;
    Real expAccrued2 = 2924630.86799;
    Real expAccrued3 = 1372523.67713;

    Real expAmount1 = 6345689.77964;
    // here we can use the telescope formula
    Real compFactor = (expAccrued1 / vars.notional + 1) / vars.forecastCurve->discount(end);
    Rate expRate1 = (compFactor - 1) / vars.dc.yearFraction(vars.start, end);

    Real expAmount2 = 7512591.22333;
    Real expAmount3 = 3269861.76315;

    auto d = vars.today;

    CHECK_CDI_OIS_COUPON_RESULT("accrued amount", coupon1->accruedAmount(d), expAccrued1, 1e-5);
    CHECK_CDI_OIS_COUPON_RESULT("accrued amount", coupon2->accruedAmount(d), expAccrued2, 1e-5);
    CHECK_CDI_OIS_COUPON_RESULT("accrued amount", coupon3->accruedAmount(d), expAccrued3, 1e-5);

    CHECK_CDI_OIS_COUPON_RESULT("coupon rate", coupon1->rate(), expRate1, 1e-12);

     CHECK_CDI_OIS_COUPON_RESULT("coupon amount", coupon1->amount(), expAmount1, 1e-5)
     CHECK_CDI_OIS_COUPON_RESULT("coupon amount", coupon2->amount(), expAmount2, 1e-5);
     CHECK_CDI_OIS_COUPON_RESULT("coupon amount", coupon3->amount(), expAmount3, 1e-5);
}
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