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
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/blackovernightindexedcouponpricer.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/settings.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
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

    ext::shared_ptr<OvernightIndexedCoupon> makeSpreadedCoupon(Date startDate,
                                                       Date endDate,
                                                       Spread spread = 0.0001,
                                                       bool includeSpread = true,
                                                       Natural fixingDays = Null<Natural>(),
                                                       Natural lockoutDays = 0,
                                                       bool applyObservationShift = false,
                                                       bool telescopicValueDates = false,
                                                       RateAveraging::Type averaging = RateAveraging::Compound) {
        return ext::make_shared<OvernightIndexedCoupon>(
            endDate, notional, startDate, endDate, sofr, 1.0, spread, Date(), Date(), DayCounter(),
            telescopicValueDates, averaging, fixingDays, lockoutDays, applyObservationShift, includeSpread);
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

struct BlackONPricerVars {
    Date today;
    Real notional = 1000000.0;
    RelinkableHandle<YieldTermStructure> forecastCurve;
    RelinkableHandle<OptionletVolatilityStructure> vol;
    ext::shared_ptr<OvernightIndex> sofr;
    ext::shared_ptr<OvernightIndexedCoupon> onCoupon;
    DayCounter dc;

    BlackONPricerVars(const Date& evalDate = Date(1, July, 2025)) {
        today = evalDate;
        dc = Actual360();
        Settings::instance().evaluationDate() = today;
        auto optionletVol = makeQuoteHandle(0.01);

        // Flat forward curve
        forecastCurve.linkTo(flatRate(today, 0.04, dc));
        sofr = ext::make_shared<Sofr>(forecastCurve);

        // Flat volatility
        vol.linkTo(ext::make_shared<ConstantOptionletVolatility>(today, TARGET(), Following, optionletVol, dc));
    }

    ext::shared_ptr<CappedFlooredOvernightIndexedCoupon> makeCoupon(Date start, Date end, Rate cap = Null<Rate>(), Rate floor = Null<Rate>(),
                                                                    RateAveraging::Type avgMethod = RateAveraging::Compound) {
        onCoupon = ext::make_shared<OvernightIndexedCoupon>(
            end, notional, start, end, sofr, 1.0, 0.0, Date(), Date(), dc,
            false, avgMethod, Null<Natural>(), 0, false, 
            false);
        return ext::make_shared<CappedFlooredOvernightIndexedCoupon>(onCoupon, cap, floor);
    }
};


struct CommonVarsONLeg {
    Date today;
    Real notional = 1000000.0;
    ext::shared_ptr<OvernightIndex> sofr;
    RelinkableHandle<YieldTermStructure> forecastCurve;
    Schedule legSchedule;
    DayCounter dc;
    RelinkableHandle<OptionletVolatilityStructure> rateVolTS;

    ext::shared_ptr<OptionletVolatilityStructure> returnRateVolTS() {
        auto optionletVol = makeQuoteHandle(0.01);
        return ext::make_shared<ConstantOptionletVolatility>(today, TARGET(), Following, optionletVol, dc);
    }

    Leg makeLeg(Natural fixingDays = Null<Natural>(),
                Natural lockoutDays = 0,
                bool applyObservationShift = false,
                bool telescopicValueDates = false,
                RateAveraging::Type averaging = RateAveraging::Compound,
                const std::vector<Real>& gearings = std::vector<Real>(),
                const std::vector<Spread>& spreads = std::vector<Spread>(),
                const std::vector<Rate>& caps = std::vector<Rate>(),
                const std::vector<Rate>& floors = std::vector<Rate>()) {
        
        OvernightLeg leg(legSchedule, sofr);
        leg.withNotionals(notional)
           .withPaymentDayCounter(dc)
           .withAveragingMethod(averaging)
           .withLockoutDays(lockoutDays)
           .withObservationShift(applyObservationShift)
           .withTelescopicValueDates(telescopicValueDates);
           
        if (fixingDays != Null<Natural>()) {
            leg.withLookbackDays(fixingDays);
        }
        
        if (!gearings.empty()) {
            leg.withGearings(gearings);
        }
        
        if (!spreads.empty()) {
            leg.withSpreads(spreads);
        }
        
        if (!caps.empty()) {
            leg.withCaps(caps);
        }
        
        if (!floors.empty()) {
            leg.withFloors(floors);
        }

        if (!caps.empty() || !floors.empty()) {
            rateVolTS.linkTo(returnRateVolTS());
            leg.withCapFlooredOvernightIndexedCouponPricer(ext::make_shared<BlackOvernightIndexedCouponPricer>(rateVolTS));
        }
        
        return leg;
    }

    CommonVarsONLeg(const Date& evaluationDate) {
        today = evaluationDate;
        dc = Actual360();

        Settings::instance().evaluationDate() = today;

        sofr = ext::make_shared<Sofr>(forecastCurve);
        
        // Create a quarterly schedule for testing
        legSchedule = Schedule(Date(1, July, 2025), Date(1, July, 2026),
                              Period(1, Months), 
                              UnitedStates(UnitedStates::GovernmentBond),
                              ModifiedFollowing, ModifiedFollowing,
                              DateGeneration::Forward, false);

        std::vector<Date> pastDates = {
            Date(2, June, 2025), Date(3, June, 2025), Date(4, June, 2025), Date(5, June, 2025),
            Date(6, June, 2025), Date(9, June, 2025), Date(10, June, 2025), Date(11, June, 2025),
            Date(12, June, 2025), Date(13, June, 2025), Date(16, June, 2025), Date(17, June, 2025),
            Date(18, June, 2025), Date(20, June, 2025), Date(23, June, 2025), Date(24, June, 2025),
            Date(25, June, 2025), Date(26, June, 2025), Date(27, June, 2025), Date(30, June, 2025),
            Date(1, July, 2025), Date(2, July, 2025), Date(3, July, 2025), Date(7, July, 2025),
            Date(8, July, 2025), Date(9, July, 2025), Date(10, July, 2025), Date(11, July, 2025),
            Date(14, July, 2025), Date(15, July, 2025), Date(16, July, 2025), Date(17, July, 2025),
            Date(18, July, 2025), Date(21, July, 2025), Date(22, July, 2025), Date(23, July, 2025),
            Date(24, July, 2025), Date(25, July, 2025), Date(28, July, 2025), Date(29, July, 2025),
            Date(30, July, 2025), Date(31, July, 2025), Date(1, August, 2025)
        };

        std::vector<Rate> pastRates = {
            0.0435, 0.0432, 0.0428, 0.0429, 0.0429, 0.0429, 0.0428, 0.0428, 0.0428, 0.0428,
            0.0432, 0.0431, 0.0428, 0.0429, 0.0429, 0.0430, 0.0436, 0.0440, 0.0439, 0.0445,
            0.0444, 0.0440, 0.0435, 0.0433, 0.0434, 0.0432, 0.0431, 0.0431, 0.0433, 0.0437,
            0.0434, 0.0434, 0.0430, 0.0428, 0.0428, 0.0428, 0.0430, 0.0436, 0.0436, 0.0436,
            0.0432, 0.0439, 0.0434
        };

        sofr->addFixings(pastDates.begin(), pastDates.end(), pastRates.begin());
    }

    void setupForecastCurve() {
        std::vector<Date> curveDates = {
            today,
            Date(30, July, 2025),
            Date(29, August, 2025), 
            Date(30, September, 2025),
            Date(30, December, 2025),
            Date(30, March, 2026),
            Date(30, June, 2026)
        };
        
        std::vector<Rate> zeroRates = {
            0.0434,
            0.0436,
            0.0431,
            0.0413,
            0.0390,
            0.0370,
            0.0348
        };
        
        ext::shared_ptr<InterpolatedZeroCurve<Cubic>> zeroCurve(
            new InterpolatedZeroCurve<Cubic>(curveDates, zeroRates, 
                dc, UnitedStates(UnitedStates::SOFR))
        );

        zeroCurve->enableExtrapolation();
        
        forecastCurve.linkTo(zeroCurve);
    }

    CommonVarsONLeg() : CommonVarsONLeg(Date(1, June, 2025)) {}
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

BOOST_AUTO_TEST_CASE(testPastSpreadedCouponRate) {
    BOOST_TEST_MESSAGE("Testing rate for past overnight-indexed coupon with spread included...");

    CommonVars vars;

    // coupon entirely in the past
    auto pastCoupon = vars.makeSpreadedCoupon(Date(18, October, 2021),
                                      Date(18, November, 2021),
                                      0.0001);
    auto pastCouponNotSpreadedIncluded = vars.makeSpreadedCoupon(Date(18, October, 2021),
                                      Date(18, November, 2021),
                                      0.0001, false);

    // expected values here and below come from manual calculations based on past dates and rates
    Rate expectedRateSpreadInlcuded = 0.0010871445057780704;
    Rate expectedRateSpreadNotInlcuded = 0.0010871361040194164;
    Real expectedAmount = vars.notional * expectedRateSpreadInlcuded * 31.0/360;
    CHECK_OIS_COUPON_RESULT("coupon rate", pastCoupon->rate(), expectedRateSpreadInlcuded, 1e-12);
    CHECK_OIS_COUPON_RESULT("coupon rate", pastCouponNotSpreadedIncluded->rate(), expectedRateSpreadNotInlcuded, 1e-12);
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

    auto coupon15JulyWithTelescopicDates =
        vars.makeCoupon(Date(1, July, 2019), Date(15, July, 2019), 3, 0, true, true);

    CHECK_OIS_COUPON_RESULT("telescopic value dates coupon rate", actualRate,
                            coupon15JulyWithTelescopicDates->rate(), 1e-12);

    Rate expectedRateTelescopicSeries =
        (vars.forecastCurve->discount(Date(26, June, 2019)) /
             vars.forecastCurve->discount(Date(10, July, 2019)) - 1.0) * 360.0 / 14;

    CHECK_OIS_COUPON_RESULT("coupon rate using telescopic formula", actualRate,
                            expectedRateTelescopicSeries, 1e-12);

    const auto& fixingDates = coupon15July->fixingDates();
    const auto& dts = coupon15July->dt();
    Size n = fixingDates.size();
    
    Rate expectedRateIterativeFormula = 1.0;
    for (Size i = 0; i < n; ++i) {
        expectedRateIterativeFormula *=
            (1.0 + dts[i] * coupon15July->index()->fixing(fixingDates[i]));
	}
    expectedRateIterativeFormula -= 1.0;
    expectedRateIterativeFormula /= coupon15July->accrualPeriod();

    CHECK_OIS_COUPON_RESULT("coupon rate using iterative formula", actualRate,
                            expectedRateIterativeFormula, 1e-12);


}

BOOST_AUTO_TEST_CASE(testErrorWhenTelescopicValueDatesEnforcedWithLookback) {
    BOOST_TEST_MESSAGE("Testing error when telescopic value dates enforced with lookback...");

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

BOOST_AUTO_TEST_CASE(testBlackOvernightIndexedCouponPricerCapletFloorlet) {
    BOOST_TEST_MESSAGE("Testing BlackOvernightIndexedCouponPricer caplet/floorlet pricing...");

    BlackONPricerVars vars;
    Date start = Date(1, July, 2025);
    Date end = Date(1, October, 2025);

    // Caplet
    Rate cap = 0.045;
    auto cappedCoupon = vars.makeCoupon(start, end, cap, Null<Rate>());
    auto pricer = ext::make_shared<BlackOvernightIndexedCouponPricer>(vars.vol);
    cappedCoupon->setPricer(pricer);

    Rate rate = cappedCoupon->rate();
    Rate expectedRate = 0.040205142853869814;
    BOOST_CHECK(rate <= cap + 1e-8); // Should not exceed cap
    CHECK_OIS_COUPON_RESULT("Capped Rate", rate, expectedRate, 1e-8);

    // Floorlet
    Rate floor = 0.035;
    auto flooredCoupon = vars.makeCoupon(start, end, Null<Rate>(), floor);
    flooredCoupon->setPricer(pricer);
    BOOST_CHECK(!flooredCoupon->isCalculated());

    rate = flooredCoupon->rate();
    BOOST_CHECK(rate >= floor - 1e-8); // Should not be below floor
    CHECK_OIS_COUPON_RESULT("Floored Rate", rate, expectedRate, 1e-8);

    // Capped and Floored
    auto cappedFlooredCoupon = vars.makeCoupon(start, end, cap, floor);
    cappedFlooredCoupon->setPricer(pricer);
    rate = cappedFlooredCoupon->rate();
    BOOST_CHECK(rate <= cap + 1e-8 && rate >= floor - 1e-8);
    CHECK_OIS_COUPON_RESULT("Capped and Floored Rate", rate, expectedRate, 1e-8);
}

BOOST_AUTO_TEST_CASE(testBlackAverageONIndexedCouponPricerCapletFloorlet) {
    BOOST_TEST_MESSAGE("Testing BlackAverageONIndexedCouponPricer caplet/floorlet pricing...");

    BlackONPricerVars vars;
    Date start = Date(1, July, 2025);
    Date end = Date(1, October, 2025);

    // Caplet
    Rate cap = 0.045;
    auto cappedCoupon = vars.makeCoupon(start, end, cap, Null<Rate>(), RateAveraging::Simple);
    cappedCoupon->setPricer(ext::make_shared<BlackAverageONIndexedCouponPricer>(vars.vol));

    Rate rate = cappedCoupon->rate();
    Rate expectedRate = 0.040004396561138286;
    BOOST_CHECK(rate <= cap + 1e-8);
    CHECK_OIS_COUPON_RESULT("Capped Rate", rate, expectedRate, 1e-8);

    // Floorlet
    Rate floor = 0.035;
    auto flooredCoupon = vars.makeCoupon(start, end, Null<Rate>(), floor, RateAveraging::Simple);
    flooredCoupon->setPricer(ext::make_shared<BlackAverageONIndexedCouponPricer>(vars.vol));

    rate = flooredCoupon->rate();
    BOOST_CHECK(rate >= floor - 1e-8);
    CHECK_OIS_COUPON_RESULT("Capped Rate", rate, expectedRate, 1e-8);

    // Capped and Floored
    auto cappedFlooredCoupon = vars.makeCoupon(start, end, cap, floor, RateAveraging::Simple);
    cappedFlooredCoupon->setPricer(ext::make_shared<BlackAverageONIndexedCouponPricer>(vars.vol));
    rate = cappedFlooredCoupon->rate();
    BOOST_CHECK(rate <= cap + 1e-8 && rate >= floor - 1e-8);
    CHECK_OIS_COUPON_RESULT("Capped and Floored Rate", rate, expectedRate, 1e-8);
}

BOOST_AUTO_TEST_CASE(testBlackONPricerConsistencyWithNoVol) {
    BOOST_TEST_MESSAGE("Testing BlackOvernightIndexedCouponPricer with zero volatility (should match vanilla pricer)...");

    BlackONPricerVars vars;
    auto optionletVol = makeQuoteHandle(0.0);
    vars.vol.linkTo(ext::make_shared<ConstantOptionletVolatility>(vars.today, TARGET(), Following, optionletVol, vars.dc));
    Date start = Date(1, July, 2025);
    Date end = Date(1, October, 2025);

    auto cappedFlooredCoupon = vars.makeCoupon(start, end, Null<Rate>(), Null<Rate>());
    cappedFlooredCoupon->setPricer(ext::make_shared<BlackOvernightIndexedCouponPricer>(vars.vol));
    Rate blackRate = cappedFlooredCoupon->rate();

    // Compare with standard compounding pricer
    auto baseONCoupon = vars.onCoupon;
    baseONCoupon->setPricer(ext::make_shared<CompoundingOvernightIndexedCouponPricer>());
    Rate vanillaRate = baseONCoupon->rate();

    CHECK_OIS_COUPON_RESULT("Zero capped coupon rate", blackRate, vanillaRate, 1e-10);
}

BOOST_AUTO_TEST_CASE(testOvernightLegBasicFunctionality) {
    BOOST_TEST_MESSAGE("Testing basic functionality of OvernightLeg...");

    CommonVarsONLeg vars;
    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    Leg leg = vars.makeLeg();
    
    // Check that we have the expected number of coupons (monthly over 1 year = 12 coupons)
    BOOST_CHECK_EQUAL(leg.size(), 12);
    
    // Check that all cash flows are OvernightIndexedCoupons
    for (const auto& cf : leg) {
        auto oisCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(cf);
        BOOST_CHECK(oisCoupon != nullptr);
        if (oisCoupon) {
            BOOST_CHECK_EQUAL(oisCoupon->nominal(), vars.notional);
            BOOST_CHECK_EQUAL(oisCoupon->averagingMethod(), RateAveraging::Compound);
            BOOST_CHECK_EQUAL(oisCoupon->lockoutDays(), 0);
            BOOST_CHECK_EQUAL(oisCoupon->applyObservationShift(), false);
        }
    }
}

BOOST_AUTO_TEST_CASE(testOvernightLegWithLookback) {
    BOOST_TEST_MESSAGE("Testing OvernightLeg with lookback days...");

    CommonVarsONLeg vars;
    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    Natural lookbackDays = 5;
    Leg leg = vars.makeLeg(lookbackDays);
    
    for (const auto& cf : leg) {
        auto oisCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(cf);
        BOOST_CHECK(oisCoupon != nullptr);
        if (oisCoupon) {
            // The coupon should have lookback configured
            BOOST_CHECK(oisCoupon->fixingDays() == lookbackDays || 
                       oisCoupon->fixingDays() == oisCoupon->index()->fixingDays());
        }
    }
}

BOOST_AUTO_TEST_CASE(testOvernightLegWithLockout) {
    BOOST_TEST_MESSAGE("Testing OvernightLeg with lockout days...");

    CommonVarsONLeg vars;
    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    Natural lockoutDays = 3;
    Leg leg = vars.makeLeg(Null<Natural>(), lockoutDays);
    
    for (const auto& cf : leg) {
        auto oisCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(cf);
        BOOST_CHECK(oisCoupon != nullptr);
        if (oisCoupon) {
            BOOST_CHECK_EQUAL(oisCoupon->lockoutDays(), lockoutDays);
        }
    }
}

BOOST_AUTO_TEST_CASE(testOvernightLegWithObservationShift) {
    BOOST_TEST_MESSAGE("Testing OvernightLeg with observation shift...");

    CommonVarsONLeg vars;
    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    Leg leg = vars.makeLeg(Null<Natural>(), 0, true);
    
    for (const auto& cf : leg) {
        auto oisCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(cf);
        BOOST_CHECK(oisCoupon != nullptr);
        if (oisCoupon) {
            BOOST_CHECK_EQUAL(oisCoupon->applyObservationShift(), true);
        }
    }
}

BOOST_AUTO_TEST_CASE(testOvernightLegWithGearingsAndSpreads) {
    BOOST_TEST_MESSAGE("Testing OvernightLeg with gearings and spreads...");

    CommonVarsONLeg vars;
    vars.setupForecastCurve();

    std::vector<Real> gearings = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                  1.25, 1.25, 1.25, 1.5, 2.0, 0.5};
    std::vector<Spread> spreads = {0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001,
                                  0.0001, 0.0001, 0.0001, 0.0002, 0.0003, 0.0004};
    
    Leg leg = vars.makeLeg(Null<Natural>(), 0, false, false, 
                          RateAveraging::Compound, gearings, spreads);
    
    BOOST_CHECK_EQUAL(leg.size(), 12);
    
    for (Size i = 0; i < leg.size(); ++i) {
        auto oisCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(leg[i]);
        BOOST_CHECK(oisCoupon != nullptr);
        if (oisCoupon) {
            BOOST_CHECK_CLOSE(oisCoupon->gearing(), gearings[i], 1e-12);
            BOOST_CHECK_CLOSE(oisCoupon->spread(), spreads[i], 1e-12);
        }
    }
}

BOOST_AUTO_TEST_CASE(testOvernightLegNPV) {
    BOOST_TEST_MESSAGE("Testing Coupon Leg NPVs...");

    CommonVarsONLeg vars;
    vars.setupForecastCurve();

    Leg leg = vars.makeLeg(Null<Natural>(), 3, false, true, RateAveraging::Compound);
    
    Handle<YieldTermStructure> discountCurve(flatRate(0.0015, Actual360()));
    
    // Calculate NPV
    Real expectedNpv = 34662.920418887923;
    Real npv = 0.0;
    for (const auto& cf : leg) {
        npv += cf->amount() * discountCurve->discount(cf->date());
    }
    
    CHECK_OIS_COUPON_RESULT("OvernightLeg NPV", npv, expectedNpv, 1e-8);
}

BOOST_AUTO_TEST_CASE(testOvernightLegWithCapsAndFloors) {
    BOOST_TEST_MESSAGE("Testing OvernightLeg with caps and floors...");

    CommonVarsONLeg vars;
    vars.setupForecastCurve();
    Handle<YieldTermStructure> discountCurve(flatRate(0.0015, Actual360()));

    std::vector<Rate> caps = {0.0435, 0.0435, 0.0435, 0.0435, 0.0435, 0.0435, 
                              0.04, 0.04, 0.04, 0.04, 0.04, 0.04};
    std::vector<Rate> floors = {0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 
                                0.025, 0.025, 0.025, 0.025, 0.025, 0.025};
    
    Leg leg = vars.makeLeg(Null<Natural>(), 0, false, false, 
                          RateAveraging::Compound, 
                          std::vector<Real>(), std::vector<Spread>(),
                          caps, floors);
    
    BOOST_CHECK_EQUAL(leg.size(), 12);

    Real expectedNpv = -14.317126680266473;
    Real npv = 0.0;
    
    for (Size i = 0; i < leg.size(); ++i) {
        auto cappedFlooredCoupon = ext::dynamic_pointer_cast<CappedFlooredOvernightIndexedCoupon>(leg[i]);
        BOOST_CHECK(cappedFlooredCoupon != nullptr);
        if (cappedFlooredCoupon) {
            BOOST_CHECK_CLOSE(cappedFlooredCoupon->cap(), caps[i], 1e-12);
            BOOST_CHECK_CLOSE(cappedFlooredCoupon->floor(), floors[i], 1e-12);
            BOOST_CHECK(cappedFlooredCoupon->isCapped());
            BOOST_CHECK(cappedFlooredCoupon->isFloored());
        }
        npv += cappedFlooredCoupon->amount() * discountCurve->discount(cappedFlooredCoupon->date());
    }

    CHECK_OIS_COUPON_RESULT("Capped-Floored OvernightLeg NPV", npv, expectedNpv, 1e-8);
}

BOOST_AUTO_TEST_CASE(testOvernightLegSimpleAveraging) {
    BOOST_TEST_MESSAGE("Testing OvernightLeg with simple averaging...");

    CommonVarsONLeg vars;
    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    Leg leg = vars.makeLeg(Null<Natural>(), 0, false, false, RateAveraging::Simple);
    
    for (const auto& cf : leg) {
        auto oisCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(cf);
        BOOST_CHECK(oisCoupon != nullptr);
        if (oisCoupon) {
            BOOST_CHECK_EQUAL(oisCoupon->averagingMethod(), RateAveraging::Simple);
        }
    }
}

BOOST_AUTO_TEST_CASE(testOvernightLegErrorConditions) {
    BOOST_TEST_MESSAGE("Testing error conditions for OvernightLeg...");

    CommonVarsONLeg vars;
    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    // Test that lookback with simple averaging throws an error
    BOOST_CHECK_THROW(vars.makeLeg(5, 0, false, false, RateAveraging::Simple), Error);
    
    // Test that lockout with simple averaging throws an error  
    BOOST_CHECK_THROW(vars.makeLeg(Null<Natural>(), 3, false, false, RateAveraging::Simple), Error);
    
    // Test that observation shift with simple averaging throws an error
    BOOST_CHECK_THROW(vars.makeLeg(Null<Natural>(), 0, true, false, RateAveraging::Simple), Error);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
