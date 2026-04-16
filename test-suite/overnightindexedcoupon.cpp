/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 StatPro Italia srl

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/overnightindexedcouponpricer.hpp>
#include <ql/cashflows/blackovernightindexedcouponpricer.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/indexes/ibor/estr.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
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
                                                       RateAveraging::Type averaging = RateAveraging::Compound,
                                                       Date paymentDate = Date(),
                                                       Real notional = 10000.0,
                                                       const DayCounter& daycounter = DayCounter(),
                                                       Date rateComputationStartDate = Date(),
                                                       Date rateComputationEndDate = Date(),
                                                       Date exCouponDate = Date()) {
        return ext::make_shared<OvernightIndexedCoupon>(
            paymentDate != Date() ? paymentDate : endDate, notional, startDate, endDate, sofr, 1.0, 0.0, Date(),
            Date(), daycounter, telescopicValueDates, averaging, fixingDays, lockoutDays, applyObservationShift,
            false, rateComputationStartDate, rateComputationEndDate, exCouponDate);
    }

    ext::shared_ptr<OvernightIndexedCoupon> makeSpreadedCoupon(Date startDate,
                                                       Date endDate,
                                                       Spread spread = 0.0001,
                                                       bool compoundSpreadDaily = true,
                                                       Natural fixingDays = Null<Natural>(),
                                                       Natural lockoutDays = 0,
                                                       bool applyObservationShift = false,
                                                       bool telescopicValueDates = false,
                                                       RateAveraging::Type averaging = RateAveraging::Compound) {
        return ext::make_shared<OvernightIndexedCoupon>(
            endDate, notional, startDate, endDate, sofr, 1.0, spread, Date(), Date(), DayCounter(),
            telescopicValueDates, averaging, fixingDays, lockoutDays, applyObservationShift, compoundSpreadDaily);
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
    DayCounter dc;

    BlackONPricerVars(const Date& evalDate = Date(1, July, 2025)) {
        today = evalDate;
        dc = Actual360();
        Settings::instance().evaluationDate() = today;
        auto optionletVol = makeQuoteHandle(0.1);

        // Flat forward curve
        forecastCurve.linkTo(flatRate(today, 0.04, dc));
        sofr = ext::make_shared<Sofr>(forecastCurve);

        // Flat volatility
        vol.linkTo(ext::make_shared<ConstantOptionletVolatility>(today, TARGET(), Following, optionletVol, dc));
    }

    ext::shared_ptr<OvernightIndexedCoupon> makeBaseCoupon(Date start, Date end,
                                                           RateAveraging::Type avgMethod = RateAveraging::Compound) {
        auto onCoupon = ext::make_shared<OvernightIndexedCoupon>(
            end, notional, start, end, sofr, 1.0, 0.0, Date(), Date(), dc,
            false, avgMethod, Null<Natural>(), 0, false, 
            false);

        if (avgMethod == RateAveraging::Compound)
            onCoupon->setPricer(ext::make_shared<CompoundingOvernightIndexedCouponPricer>());
        else
            onCoupon->setPricer(ext::make_shared<ArithmeticAveragedOvernightIndexedCouponPricer>());

        return onCoupon;
    }

    ext::shared_ptr<CappedFlooredOvernightIndexedCoupon> makeCoupon(Date start, Date end, Rate cap = Null<Rate>(), Rate floor = Null<Rate>(),
                                                                    RateAveraging::Type avgMethod = RateAveraging::Compound) {
        auto onCoupon = makeBaseCoupon(start, end, avgMethod);

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
        auto optionletVol = makeQuoteHandle(0.05);
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
            if (averaging == RateAveraging::Compound)
                leg.withCouponPricer(ext::make_shared<BlackCompoundingOvernightIndexedCouponPricer>(rateVolTS));
            else
                leg.withCouponPricer(ext::make_shared<BlackAveragingOvernightIndexedCouponPricer>(rateVolTS));
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
                              Period(3, Months), 
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
    BOOST_TEST_MESSAGE("Testing rate for past overnight-indexed coupon with compounded spread...");

    CommonVars vars;

    // coupon entirely in the past
    auto pastCoupon = vars.makeSpreadedCoupon(Date(18, October, 2021),
                                              Date(18, November, 2021),
                                              0.0001);
    auto pastCouponCompoundingSpread = vars.makeSpreadedCoupon(Date(18, October, 2021),
                                                               Date(18, November, 2021),
                                                               0.0001, false);

    // expected values here and below come from manual calculations based on past dates and rates
    Rate expectedRate = 0.0010871445057780704;
    Real expectedAmount = vars.notional * expectedRate * 31.0/360;
    CHECK_OIS_COUPON_RESULT("coupon rate", pastCoupon->rate(), expectedRate, 1e-12);
    CHECK_OIS_COUPON_RESULT("coupon amount", pastCoupon->amount(), expectedAmount, 1e-8);

    expectedRate = 0.0010871361040194164;
    CHECK_OIS_COUPON_RESULT("coupon rate", pastCouponCompoundingSpread->rate(), expectedRate, 1e-12);
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
    Rate expectedRate = 0.025003472543756455;

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
    BOOST_TEST_MESSAGE("Testing Black compounding overnight-indexed coupon pricer...");

    BlackONPricerVars vars;
    Date start = Date(1, July, 2035);
    Date end = Date(1, October, 2035);

    // Vanilla
    auto vanillaCoupon = vars.makeBaseCoupon(start, end);
    Rate expectedRate = vanillaCoupon->rate();

    auto pricer = ext::make_shared<BlackCompoundingOvernightIndexedCouponPricer>(vars.vol);
    vanillaCoupon->setPricer(pricer);

    Rate rate = vanillaCoupon->rate();
    CHECK_OIS_COUPON_RESULT("Base Rate", rate, expectedRate, 1e-8);

    // Caplet
    Rate cap = 0.045;
    auto cappedCoupon = vars.makeCoupon(start, end, cap, Null<Rate>());
    cappedCoupon->setPricer(pricer);

    rate = cappedCoupon->rate();
    expectedRate = 0.036862168;
    BOOST_CHECK(rate <= cap + 1e-8); // Should not exceed cap
    CHECK_OIS_COUPON_RESULT("Capped Rate", rate, expectedRate, 1e-8);

    // Floorlet
    Rate floor = 0.035;
    auto flooredCoupon = vars.makeCoupon(start, end, Null<Rate>(), floor);
    flooredCoupon->setPricer(pricer);
    BOOST_CHECK(!flooredCoupon->isCalculated());

    rate = flooredCoupon->rate();
    expectedRate = 0.04281620;
    BOOST_CHECK(rate >= floor - 1e-8); // Should not be below floor
    CHECK_OIS_COUPON_RESULT("Floored Rate", rate, expectedRate, 1e-8);

    // Capped and Floored
    auto cappedFlooredCoupon = vars.makeCoupon(start, end, cap, floor);
    cappedFlooredCoupon->setPricer(pricer);

    rate = cappedFlooredCoupon->rate();
    expectedRate = 0.039473179;
    BOOST_CHECK(rate <= cap + 1e-8 && rate >= floor - 1e-8);
    CHECK_OIS_COUPON_RESULT("Capped and Floored Rate", rate, expectedRate, 1e-8);
}

BOOST_AUTO_TEST_CASE(testBlackAverageONIndexedCouponPricerCapletFloorlet) {
    BOOST_TEST_MESSAGE("Testing Black averaging overnight-indexed coupon pricer...");

    BlackONPricerVars vars;
    Date start = Date(1, July, 2035);
    Date end = Date(1, October, 2035);

    // Vanilla
    auto vanillaCoupon = vars.makeBaseCoupon(start, end, RateAveraging::Simple);
    Rate expectedRate = vanillaCoupon->rate();

    auto pricer = ext::make_shared<BlackAveragingOvernightIndexedCouponPricer>(vars.vol);
    vanillaCoupon->setPricer(pricer);

    Rate rate = vanillaCoupon->rate();
    CHECK_OIS_COUPON_RESULT("Base Rate", rate, expectedRate, 1e-8);

    // Caplet
    Rate cap = 0.045;
    auto cappedCoupon = vars.makeCoupon(start, end, cap, Null<Rate>(), RateAveraging::Simple);
    cappedCoupon->setPricer(pricer);

    rate = cappedCoupon->rate();
    expectedRate = 0.036745802;
    BOOST_CHECK(rate <= cap + 1e-8);
    CHECK_OIS_COUPON_RESULT("Capped Rate", rate, expectedRate, 1e-8);

    // Floorlet
    Rate floor = 0.035;
    auto flooredCoupon = vars.makeCoupon(start, end, Null<Rate>(), floor, RateAveraging::Simple);
    flooredCoupon->setPricer(pricer);

    rate = flooredCoupon->rate();
    expectedRate = 0.042671405;
    BOOST_CHECK(rate >= floor - 1e-8);
    CHECK_OIS_COUPON_RESULT("Capped Rate", rate, expectedRate, 1e-8);

    // Capped and Floored
    auto cappedFlooredCoupon = vars.makeCoupon(start, end, cap, floor, RateAveraging::Simple);
    cappedFlooredCoupon->setPricer(pricer);

    rate = cappedFlooredCoupon->rate();
    expectedRate = 0.039412858;
    BOOST_CHECK(rate <= cap + 1e-8 && rate >= floor - 1e-8);
    CHECK_OIS_COUPON_RESULT("Capped and Floored Rate", rate, expectedRate, 1e-8);
}

BOOST_AUTO_TEST_CASE(testBlackONPricerConsistencyWithNoVol) {
    BOOST_TEST_MESSAGE("Testing Black compounding pricer with zero volatility (should match vanilla pricer)...");

    BlackONPricerVars vars;
    auto optionletVol = makeQuoteHandle(0.0);
    vars.vol.linkTo(ext::make_shared<ConstantOptionletVolatility>(vars.today, TARGET(), Following, optionletVol, vars.dc));
    Date start = Date(1, July, 2035);
    Date end = Date(1, October, 2035);

    auto cappedFlooredCoupon = vars.makeCoupon(start, end, 0.045, 0.035);
    auto blackPricer = ext::make_shared<BlackCompoundingOvernightIndexedCouponPricer>(vars.vol);
    cappedFlooredCoupon->setPricer(blackPricer);
    Rate blackRate = cappedFlooredCoupon->rate();

    // Compare with standard compounding pricer
    auto baseONCoupon = vars.makeBaseCoupon(start, end);
    baseONCoupon->setPricer(ext::make_shared<CompoundingOvernightIndexedCouponPricer>());
    Rate vanillaRate = baseONCoupon->rate();

    CHECK_OIS_COUPON_RESULT("Zero capped coupon rate", blackRate, vanillaRate, 1e-10);

    baseONCoupon->setPricer(blackPricer);
    vanillaRate = baseONCoupon->rate();
    CHECK_OIS_COUPON_RESULT("Zero capped coupon rate (same pricer)", blackRate, vanillaRate, 1e-10);
}

BOOST_AUTO_TEST_CASE(testBlackONAveragingPricerConsistencyWithNoVol) {
    BOOST_TEST_MESSAGE("Testing Black averaging pricer with zero volatility (should match vanilla pricer)...");

    BlackONPricerVars vars;
    auto optionletVol = makeQuoteHandle(0.0);
    vars.vol.linkTo(ext::make_shared<ConstantOptionletVolatility>(vars.today, TARGET(), Following, optionletVol, vars.dc));
    Date start = Date(1, July, 2035);
    Date end = Date(1, October, 2035);

    auto cappedFlooredCoupon = vars.makeCoupon(start, end, 0.045, 0.035, RateAveraging::Simple);
    auto blackPricer = ext::make_shared<BlackAveragingOvernightIndexedCouponPricer>(vars.vol);
    cappedFlooredCoupon->setPricer(blackPricer);
    Rate blackRate = cappedFlooredCoupon->rate();

    // Compare with standard compounding pricer
    auto baseONCoupon = vars.makeBaseCoupon(start, end, RateAveraging::Simple);
    baseONCoupon->setPricer(ext::make_shared<ArithmeticAveragedOvernightIndexedCouponPricer>());
    Rate vanillaRate = baseONCoupon->rate();

    CHECK_OIS_COUPON_RESULT("Zero capped coupon rate", blackRate, vanillaRate, 1e-10);

    baseONCoupon->setPricer(blackPricer);
    vanillaRate = baseONCoupon->rate();
    CHECK_OIS_COUPON_RESULT("Zero capped coupon rate (same pricer)", blackRate, vanillaRate, 1e-10);
}

BOOST_AUTO_TEST_CASE(testOvernightLegBasicFunctionality) {
    BOOST_TEST_MESSAGE("Testing basic functionality of overnight leg...");

    CommonVarsONLeg vars;
    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    Leg leg = vars.makeLeg();
    
    // Check that we have the expected number of coupons (monthly over 1 year = 12 coupons)
    BOOST_CHECK_EQUAL(leg.size(), 4);
    
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
    BOOST_TEST_MESSAGE("Testing overnight leg construction with lookback days...");

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
    BOOST_TEST_MESSAGE("Testing overnight leg construction with lockout days...");

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
    BOOST_TEST_MESSAGE("Testing overnight leg construction with observation shift...");

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
    BOOST_TEST_MESSAGE("Testing overnight leg construction with gearings and spreads...");

    CommonVarsONLeg vars;
    vars.setupForecastCurve();

    std::vector<Real> gearings = {1.0, 1.25, 2.0, 0.5};
    std::vector<Spread> spreads = {0.0001, 0.0001, 0.0002, 0.0002};
    
    Leg leg = vars.makeLeg(Null<Natural>(), 0, false, false, 
                          RateAveraging::Compound, gearings, spreads);
    
    BOOST_CHECK_EQUAL(leg.size(), 4);
    
    for (Size i = 0; i < leg.size(); ++i) {
        auto oisCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(leg[i]);
        BOOST_CHECK(oisCoupon != nullptr);
        if (oisCoupon) {
            QL_CHECK_CLOSE(oisCoupon->gearing(), gearings[i], 1e-12);
            QL_CHECK_CLOSE(oisCoupon->spread(), spreads[i], 1e-12);
        }
    }
}

BOOST_AUTO_TEST_CASE(testOvernightLegNPV) {
    BOOST_TEST_MESSAGE("Testing overnight leg NPV...");

    CommonVarsONLeg vars;
    vars.setupForecastCurve();

    Leg leg = vars.makeLeg(Null<Natural>(), 3, false, true, RateAveraging::Compound);
    
    Handle<YieldTermStructure> discountCurve(flatRate(0.0015, Actual360()));
    
    // Calculate NPV
    Real expectedNpv = 34883.949669756257;
    Real npv = 0.0;
    for (const auto& cf : leg) {
        npv += cf->amount() * discountCurve->discount(cf->date());
    }
    
    CHECK_OIS_COUPON_RESULT("OvernightLeg NPV", npv, expectedNpv, 1e-8);
}

BOOST_AUTO_TEST_CASE(testOvernightLegWithCapsAndFloors) {
    BOOST_TEST_MESSAGE("Testing overnight leg with caps and floors...");

    CommonVarsONLeg vars;
    vars.setupForecastCurve();
    Handle<YieldTermStructure> discountCurve(flatRate(0.0015, Actual360()));

    std::vector<Rate> caps = {0.0435, 0.0435, 0.04, 0.04};
    std::vector<Rate> floors = {0.025, 0.025, 0.025, 0.025};
    
    Leg leg = vars.makeLeg(Null<Natural>(), 0, false, false, 
                          RateAveraging::Compound, 
                          std::vector<Real>(), std::vector<Spread>(),
                          caps, floors);
    
    BOOST_CHECK_EQUAL(leg.size(), 4);

    Real expectedNpv = 34648.328606210489;
    Real npv = 0.0;
    
    for (Size i = 0; i < leg.size(); ++i) {
        auto cappedFlooredCoupon = ext::dynamic_pointer_cast<CappedFlooredOvernightIndexedCoupon>(leg[i]);
        BOOST_CHECK(cappedFlooredCoupon != nullptr);
        if (cappedFlooredCoupon) {
            QL_CHECK_CLOSE(cappedFlooredCoupon->cap(), caps[i], 1e-12);
            QL_CHECK_CLOSE(cappedFlooredCoupon->floor(), floors[i], 1e-12);
            BOOST_CHECK(cappedFlooredCoupon->isCapped());
            BOOST_CHECK(cappedFlooredCoupon->isFloored());
        }
        npv += cappedFlooredCoupon->amount() * discountCurve->discount(cappedFlooredCoupon->date());
    }

    CHECK_OIS_COUPON_RESULT("Capped-Floored OvernightLeg NPV", npv, expectedNpv, 1e-8);
}

BOOST_AUTO_TEST_CASE(testOvernightLegSimpleAveraging) {
    BOOST_TEST_MESSAGE("Testing overnight leg construction with simple averaging...");

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
    BOOST_TEST_MESSAGE("Testing error conditions for overnight leg...");

    CommonVarsONLeg vars;
    vars.forecastCurve.linkTo(flatRate(0.0010, Actual360()));

    // Test that lookback with simple averaging throws an error
    BOOST_CHECK_THROW(vars.makeLeg(5, 0, false, false, RateAveraging::Simple), Error);
    
    // Test that lockout with simple averaging throws an error  
    BOOST_CHECK_THROW(vars.makeLeg(Null<Natural>(), 3, false, false, RateAveraging::Simple), Error);
    
    // Test that observation shift with simple averaging throws an error
    BOOST_CHECK_THROW(vars.makeLeg(Null<Natural>(), 0, true, false, RateAveraging::Simple), Error);
}

BOOST_AUTO_TEST_CASE(testOvernightIndexedCouponPaymentBeforeAccrualEnd) {
    BOOST_TEST_MESSAGE("Testing that an overnight coupon with inconsistent dates throws...");

    Date accrualStart(18, September, 2025);
    Settings::instance().evaluationDate() = accrualStart;

    Handle<YieldTermStructure> h(
        ext::make_shared<FlatForward>(accrualStart, 0.05, Actual365Fixed()));
    ext::shared_ptr<OvernightIndex> estr =
        ext::make_shared<Estr>(h);

    Calendar cal = WeekendsOnly();
    Date accrualEnd = cal.advance(accrualStart, Period(6, Months));
    Date paymentDate = cal.advance(accrualEnd, Period(-1, Days));

    BOOST_CHECK_THROW(
        OvernightIndexedCoupon(paymentDate, 1.0,
                               accrualStart, accrualEnd, estr),
        Error
    );
}

BOOST_AUTO_TEST_CASE(testInterestCalculatedAccrualDateFixingHoliday) {
    BOOST_TEST_MESSAGE("Testing compounded interest when an accrual start/end date lands on a fixing holiday...");
    CommonVars vars(Date(19, April, 2023));
    auto sofr = vars.sofr;
    auto fixingCal = sofr->fixingCalendar();
    vars.forecastCurve.linkTo(flatRate(0.0432, Actual360()));
    Schedule sch = MakeSchedule()
        .from(Date(15, October, 2021))
        .to(Date(14, April, 2023))
        .withTenor(1 * Days)
        .withCalendar(fixingCal)
        .withConvention(sofr->businessDayConvention())
        .forwards();
    auto pastDates = sch.dates();
    auto pastRates = std::vector<Rate>(pastDates.size(), 0.048);
    // Overwrite the fixings provided within CommonVars to cover interest allocation to coupons
    // which start/end on good friday, the only holiday that is in the SOFR fixing calendar
    // but not the federal reserve calendar used to generate SOFR swap coupons
    sofr->clearFixings();
    sofr->addFixings(pastDates.begin(), pastDates.end(), pastRates.begin());
    auto dc = sofr->dayCounter();
    struct TestCase {
        Date startDate, endDate, splitAt;
    };
    const TestCase cases[] = {
        // fixed
        {Date(15, October, 2021), Date(17, April, 2023), Date(15, April, 2022)},
        // forward
        {Date(18, October, 2024), Date(20, April, 2026), Date(18, April, 2025)},
    };
    const auto compoundFactor = [](Rate rate, Time yearFraction) {
        return 1.0 + rate * yearFraction;
    };
    const auto payDate = [&](Date endDate) {
        return fixingCal.advance(endDate, 2, Days);
    };
    for (const auto& tc : cases) {
        BOOST_CHECK(fixingCal.isHoliday(tc.splitAt));
        auto cpnTotal = OvernightIndexedCoupon(payDate(tc.endDate), 10000000.0, tc.startDate, tc.endDate, sofr);
        auto cpnLeft = OvernightIndexedCoupon(payDate(tc.splitAt), 10000000.0, tc.startDate, tc.splitAt, sofr);
        auto cpnRight = OvernightIndexedCoupon(cpnTotal.date(), 10000000.0, tc.splitAt, tc.endDate, sofr);
        auto compounded = compoundFactor(cpnLeft.rate(), cpnLeft.accrualPeriod())
                          * compoundFactor(cpnRight.rate(), cpnRight.accrualPeriod());
        auto total = compoundFactor(cpnTotal.rate(), cpnTotal.accrualPeriod());
        auto valueDate = fixingCal.adjust(tc.splitAt, Preceding);
        auto maturityDate = fixingCal.adjust(tc.splitAt, Following);
        auto fixing = sofr->fixing(valueDate);
        // check that interest is split across coupons as expected
        auto adjustment = compoundFactor(fixing, dc.yearFraction(valueDate, maturityDate))
                     / compoundFactor(fixing, dc.yearFraction(valueDate, tc.splitAt))
                     / compoundFactor(fixing, dc.yearFraction(tc.splitAt, maturityDate));
        CHECK_OIS_COUPON_RESULT("compound factor", compounded * adjustment, total, 1e-12);
    }
}

BOOST_AUTO_TEST_CASE(testInterestCalculatedAccrualDateFixingHolidayAccruals) {
    BOOST_TEST_MESSAGE("Testing compounded interest when an accrual start/end date lands on a fixing holiday...");

    CommonVars vars(Date(29, May, 2025));
    vars.forecastCurve.linkTo(flatRate(0.0433, Actual360()));

    static const Date pastDates[] = {
        Date(10,October,2024),    Date(11,October,2024),    Date(15,October,2024),    Date(16,October,2024),
        Date(17,October,2024),    Date(18,October,2024),    Date(21,October,2024),    Date(22,October,2024),
        Date(23,October,2024),    Date(24,October,2024),    Date(25,October,2024),    Date(28,October,2024),
        Date(29,October,2024),    Date(30,October,2024),    Date(31,October,2024),    Date(1,November,2024),
        Date(4,November,2024),    Date(5,November,2024),    Date(6,November,2024),    Date(7,November,2024),
        Date(8,November,2024),    Date(12,November,2024),   Date(13,November,2024),   Date(14,November,2024),
        Date(15,November,2024),   Date(18,November,2024),   Date(19,November,2024),   Date(20,November,2024),
        Date(21,November,2024),   Date(22,November,2024),   Date(25,November,2024),   Date(26,November,2024),
        Date(27,November,2024),   Date(29,November,2024),   Date(2,December,2024),    Date(3,December,2024),
        Date(4,December,2024),    Date(5,December,2024),    Date(6,December,2024),    Date(9,December,2024),
        Date(10,December,2024),   Date(11,December,2024),   Date(12,December,2024),   Date(13,December,2024),
        Date(16,December,2024),   Date(17,December,2024),   Date(18,December,2024),   Date(19,December,2024),
        Date(20,December,2024),   Date(23,December,2024),   Date(24,December,2024),   Date(26,December,2024),
        Date(27,December,2024),   Date(30,December,2024),   Date(31,December,2024),   Date(2,January,2025),
        Date(3,January,2025),     Date(6,January,2025),     Date(7,January,2025),     Date(8,January,2025),
        Date(9,January,2025),     Date(10,January,2025),    Date(13,January,2025),    Date(14,January,2025),
        Date(15,January,2025),    Date(16,January,2025),    Date(17,January,2025),    Date(21,January,2025),
        Date(22,January,2025),    Date(23,January,2025),    Date(24,January,2025),    Date(27,January,2025),
        Date(28,January,2025),    Date(29,January,2025),    Date(30,January,2025),    Date(31,January,2025),
        Date(3,February,2025),    Date(4,February,2025),    Date(5,February,2025),    Date(6,February,2025),
        Date(7,February,2025),    Date(10,February,2025),   Date(11,February,2025),   Date(12,February,2025),
        Date(13,February,2025),   Date(14,February,2025),   Date(18,February,2025),   Date(19,February,2025),
        Date(20,February,2025),   Date(21,February,2025),   Date(24,February,2025),   Date(25,February,2025),
        Date(26,February,2025),   Date(27,February,2025),   Date(28,February,2025),   Date(3,March,2025),
        Date(4,March,2025),       Date(5,March,2025),       Date(6,March,2025),       Date(7,March,2025),
        Date(10,March,2025),      Date(11,March,2025),      Date(12,March,2025),      Date(13,March,2025),
        Date(14,March,2025),      Date(17,March,2025),      Date(18,March,2025),      Date(19,March,2025),
        Date(20,March,2025),      Date(21,March,2025),      Date(24,March,2025),      Date(25,March,2025),
        Date(26,March,2025),      Date(27,March,2025),      Date(28,March,2025),      Date(31,March,2025),
        Date(1,April,2025),       Date(2,April,2025),       Date(3,April,2025),       Date(4,April,2025),
        Date(7,April,2025),       Date(8,April,2025),       Date(9,April,2025),       Date(10,April,2025),
        Date(11,April,2025),      Date(14,April,2025),      Date(15,April,2025),      Date(16,April,2025),
        Date(17,April,2025),      Date(21,April,2025),      Date(22,April,2025),      Date(23,April,2025),
        Date(24,April,2025),      Date(25,April,2025),      Date(28,April,2025),      Date(29,April,2025),
        Date(30,April,2025),      Date(1,May,2025),         Date(2,May,2025),         Date(5,May,2025),
        Date(6,May,2025),         Date(7,May,2025),         Date(8,May,2025),         Date(9,May,2025),
        Date(12,May,2025),        Date(13,May,2025),        Date(14,May,2025),        Date(15,May,2025),
        Date(16,May,2025),        Date(19,May,2025),        Date(20,May,2025),        Date(21,May,2025),
        Date(22,May,2025),        Date(23,May,2025),        Date(27,May,2025),        Date(28,May,2025),
        Date(29,May,2025),
    };

    static const Rate pastRates[] = {
        0.0482,  0.0481,  0.0486,  0.0486,
        0.0485,  0.0484,  0.0482,  0.0483,
        0.0483,  0.0483,  0.0483,  0.0482,
        0.0482,  0.0481,  0.049,   0.0486,
        0.0482,  0.0482,  0.0481,  0.0482,
        0.046,   0.046,   0.0459,  0.0458,
        0.0457,  0.0457,  0.0457,  0.0456,
        0.0457,  0.0457,  0.0458,  0.0458,
        0.0457,  0.0459,  0.0464,  0.0464,
        0.0459,  0.0459,  0.046,   0.0463,
        0.0464,  0.0462,  0.0462,  0.046,
        0.0465,  0.0462,  0.0457,  0.043,
        0.043,   0.0431,  0.044,   0.0453,
        0.0446,  0.0437,  0.0449,  0.044,
        0.0431,  0.0427,  0.0427,  0.0429,
        0.043,   0.043,   0.0429,  0.0428,
        0.0428,  0.0429,  0.0429,  0.0429,
        0.043,   0.0435,  0.0434,  0.0434,
        0.0435,  0.0435,  0.0436,  0.0438,
        0.0435,  0.0433,  0.0433,  0.0436,
        0.0435,  0.0435,  0.0434,  0.0432,
        0.0433,  0.0433,  0.0437,  0.0435,
        0.0433,  0.0434,  0.0434,  0.0433,
        0.0433,  0.0436,  0.0439,  0.0433,
        0.0433,  0.0434,  0.0435,  0.0434,
        0.0433,  0.0432,  0.0431,  0.043,
        0.043,   0.0432,  0.0431,  0.0429,
        0.0429,  0.043,   0.0431,  0.0433,
        0.0435,  0.0436,  0.0434,  0.0441,
        0.0439,  0.0437,  0.0439,  0.0435,
        0.0433,  0.044,   0.0442,  0.0437,
        0.0433,  0.0433,  0.0436,  0.0431,
        0.0432,  0.0432,  0.043,   0.0428,
        0.0429,  0.0433,  0.0436,  0.0436,
        0.0441,  0.0439,  0.0436,  0.0433,
        0.0432,  0.043,   0.0429,  0.0428,
        0.0428,  0.043,   0.0429,  0.0431,
        0.043,   0.0429,  0.0427,  0.0426,
        0.0426,  0.0426,  0.0431,  0.0433,
        0.0433
    };
    // Overwrite the fixings provided within CommonVars to cover interest accrual for coupons which
    // start on, end on, or otherwise intersect good friday, the only holiday that is in the SOFR
    // fixing calendar but not the federal reserve calendar used to generate SOFR swap coupons
    vars.sofr->clearFixings();
    vars.sofr->addFixings(std::begin(pastDates), std::end(pastDates), std::begin(pastRates));
    auto fedCal = UnitedStates(UnitedStates::FederalReserve);
    struct TestCase {
        Date startDate, endDate;
        Natural lookbackDays;
        bool obsShift;
        Natural lockoutDays;
        DayCounter dayCounter;
        Date telescopicLowerCut, telescopicUpperCut;
        std::vector<std::pair<Date, double>> accruals;
    };
    const TestCase cases[] = {
        // coupon ends on a fixing holiday, vanilla case
        {Date(18, October, 2024), Date(18, April, 2025), 0, false, 0, DayCounter(), Date(), Date(),
            {
                {Date(18, October, 2024), 0.0},
                {Date(19, October, 2024), 1344.444444444},
                {Date(20, October, 2024), 2688.88888888},
                {Date(21, October, 2024), 4033.333333333},
                {Date(22, October, 2024), 5372.76224074},
                {Date(2, March, 2025), 169984.93805796764},
                {Date(3, March, 2025), 171224.80883054345},
                {Date(17, April, 2025), 226499.35892772756},
                {Date(18, April, 2025), 227726.5388507987},
                {Date(19, April, 2025), 227726.5388507987},
                {Date(20, April, 2025), 227726.5388507987},
                {Date(21, April, 2025), 227726.5388507987},
                {Date(22, April, 2025), 227726.5388507987},
                {Date(23, April, 2025), 0.0},
            }
        },
        // coupon ends on a fixing holiday with obs-shift, but no lookback
        {Date(18, October, 2024), Date(18, April, 2025), 0, true, 0, DayCounter(), Date(), Date(),
            {
                {Date(18, October, 2024), 0.0},
                {Date(19, October, 2024), 1344.444444444},
                {Date(20, October, 2024), 2688.88888888},
                {Date(21, October, 2024), 4033.333333333},
                {Date(22, October, 2024), 5372.76224074},
                {Date(2, March, 2025), 169984.93805796764},
                {Date(3, March, 2025), 171224.80883054345},
                {Date(17, April, 2025), 226499.35892772756},
                {Date(18, April, 2025), 227726.5388507987},
                {Date(19, April, 2025), 227726.5388507987},
                {Date(20, April, 2025), 227726.5388507987},
                {Date(21, April, 2025), 227726.5388507987},
                {Date(22, April, 2025), 227726.5388507987},
                {Date(23, April, 2025), 0.0},
            }
        },
        // coupon ends on a fixing holiday with 4d lockout
        {Date(18, October, 2024), Date(18, April, 2025), 0, false, 4, DayCounter(), Date(), Date(),
            {
                {Date(18, October, 2024), 0.0},
                {Date(19, October, 2024), 1344.444444444},
                {Date(20, October, 2024), 2688.88888888},
                {Date(21, October, 2024), 4033.333333333},
                {Date(22, October, 2024), 5372.76224074},
                {Date(2, March, 2025), 169984.93805796764},
                {Date(3, March, 2025), 171224.80883054345},
                {Date(17, April, 2025), 226496.51858061668},
                {Date(18, April, 2025), 227726.53885632323},
                {Date(19, April, 2025), 227726.53885632323},
                {Date(20, April, 2025), 227726.53885632323},
                {Date(21, April, 2025), 227726.53885632323},
                {Date(22, April, 2025), 227726.53885632323},
                {Date(23, April, 2025), 0.0},
            }
        },
        // coupon ends on a fixing holiday with 4d lockout, with obs-shift but no lookback
        {Date(18, October, 2024), Date(18, April, 2025), 0, true, 4, DayCounter(), Date(), Date(),
            {
                {Date(18, October, 2024), 0.0},
                {Date(19, October, 2024), 1344.444444444},
                {Date(20, October, 2024), 2688.88888888},
                {Date(21, October, 2024), 4033.333333333},
                {Date(22, October, 2024), 5372.76224074},
                {Date(2, March, 2025), 169984.93805796764},
                {Date(3, March, 2025), 171224.80883054345},
                {Date(17, April, 2025), 226496.51858061668},
                {Date(18, April, 2025), 227726.53885632323},
                {Date(19, April, 2025), 227726.53885632323},
                {Date(20, April, 2025), 227726.53885632323},
                {Date(21, April, 2025), 227726.53885632323},
                {Date(22, April, 2025), 227726.53885632323},
                {Date(23, April, 2025), 0.0},
            }
        },
        // coupon ends on a fixing holiday with 5d lookback
        {Date(18, October, 2024), Date(18, April, 2025), 5, false, 0, DayCounter(), Date(), Date(),
            {
                {Date(18, October, 2024), 0.0},
                {Date(19, October, 2024), 1338.888888888},
                {Date(20, October, 2024), 2677.777777777},
                {Date(21, October, 2024), 4016.666666666},
                {Date(22, October, 2024), 5353.314449075},
                {Date(2, March, 2025), 171188.0732570225},
                {Date(3, March, 2025), 172413.9709083072},
                {Date(17, April, 2025), 227743.262731344},
                {Date(18, April, 2025), 228984.79712184728},
                {Date(19, April, 2025), 228984.79712184728},
                {Date(20, April, 2025), 228984.79712184728},
                {Date(21, April, 2025), 228984.79712184728},
                {Date(22, April, 2025), 228984.79712184728},
                {Date(23, April, 2025), 0.0},
            }
        },
        // coupon ends on a fixing holiday with 5d lookback and obs-shift
        {Date(18, October, 2024), Date(18, April, 2025), 5, true, 0, DayCounter(), Date(), Date(),
            {
                {Date(18, October, 2024), 0.0},
                {Date(19, October, 2024), 1338.888888888},
                {Date(20, October, 2024), 2677.777777777},
                {Date(21, October, 2024), 4016.666666666},
                {Date(22, October, 2024), 5347.239116048},
                {Date(1, March, 2025), 169713.19769125758},
                {Date(2, March, 2025), 170979.7140919386},
                {Date(3, March, 2025), 172246.23049261962},
                {Date(17, April, 2025), 227590.6084479198},
                {Date(18, April, 2025), 228832.36294407956},
                {Date(19, April, 2025), 228832.36294407956},
                {Date(20, April, 2025), 228832.36294407956},
                {Date(21, April, 2025), 228832.36294407956},
                {Date(22, April, 2025), 228832.36294407956},
                {Date(23, April, 2025), 0.0},
            }
        },
        // coupon ends on a fixing holiday with 5d lookback and 4d lockout
        {Date(18, October, 2024), Date(18, April, 2025), 5, false, 4, DayCounter(), Date(), Date(),
            {
                {Date(18, October, 2024), 0.0},
                {Date(19, October, 2024), 1338.8888888887784},
                {Date(20, October, 2024), 2677.7777777775568},
                {Date(21, October, 2024), 4016.666666666},
                {Date(22, October, 2024), 5353.314449074},
                {Date(2, March, 2025), 171188.07325702257},
                {Date(3, March, 2025), 172413.970908307},
                {Date(17, April, 2025), 227714.85583612081},
                {Date(18, April, 2025), 228950.70471453448},
                {Date(19, April, 2025), 228950.70471453448},
                {Date(20, April, 2025), 228950.70471453448},
                {Date(21, April, 2025), 228950.70471453448},
                {Date(22, April, 2025), 228950.70471453448},
                {Date(23, April, 2025), 0.0},
            }
        },
        // coupon ends on a fixing holiday with 5d lookback, obs-shift, and 4d lockout
        {Date(18, October, 2024), Date(18, April, 2025), 5, true, 4, DayCounter(), Date(), Date(),
            {
                {Date(18, October, 2024), 0.0},
                {Date(19, October, 2024), 1338.8888888887784},
                {Date(20, October, 2024), 2677.7777777775568},
                {Date(21, October, 2024), 4016.666666666},
                {Date(22, October, 2024), 5347.2391160482857},
                {Date(1, March, 2025), 169713.19769125758},
                {Date(2, March, 2025), 170979.7140919386},
                {Date(3, March, 2025), 172246.23049261962},
                {Date(17, April, 2025), 227562.35458301706},
                {Date(18, April, 2025), 228798.4531716943},
                {Date(19, April, 2025), 228798.4531716943},
                {Date(20, April, 2025), 228798.4531716943},
                {Date(21, April, 2025), 228798.4531716943},
                {Date(22, April, 2025), 228798.4531716943},
                {Date(23, April, 2025), 0.0},
            }
        },
        // coupon starts on a fixing holiday
        {Date(18, April, 2025), Date(19, May, 2025), 0, false, 0, DayCounter(), Date(), Date(),
            {
                {Date(18, April, 2025), 0.0},
                {Date(19, April, 2025), 1200.0},
                {Date(20, April, 2025), 2400.0},
                {Date(21, April, 2025), 3600.0},
                {Date(22, April, 2025), 4800.432},
                {Date(15, May, 2025), 32462.79161128074},
                {Date(16, May, 2025), 33663.900351055265},
                {Date(17, May, 2025), 34862.365761374473},
                {Date(18, May, 2025), 36060.831171695892},
                {Date(19, May, 2025), 37259.296582012881},
                {Date(20, May, 2025), 37259.296582012881},
                {Date(21, May, 2025), 37259.296582012881},
                {Date(22, May, 2025), 0.0},
            }
        },
        // test daycounter different than index with lookback
        {Date(18, April, 2025), Date(19, May, 2025), 5, true, 0, Actual365Fixed(), Date(), Date(),
            {
                {Date(18, April, 2025), 0.0},
                {Date(19, April, 2025), 1197.2602739727813},
                {Date(20, April, 2025), 2394.5205479455626},
                {Date(21, April, 2025), 3591.780821918343},
                {Date(22, April, 2025), 4756.5963949772768},
                {Date(15, May, 2025), 32114.584259337709},
                {Date(16, May, 2025), 33294.244344813727},
                {Date(17, May, 2025), 34449.672542982727},
                {Date(18, May, 2025), 35637.592285844192},
                {Date(19, May, 2025), 36825.512028705671},
                {Date(20, May, 2025), 36825.512028705671},
                {Date(21, May, 2025), 36825.512028705671},
                {Date(22, May, 2025), 0.0},
            }
        },
        // test daycounter different than index
        {Date(18, April, 2025), Date(19, May, 2025), 0, false, 0, Actual365Fixed(), Date(), Date(),
            {
                {Date(18, April, 2025), 0.0},
                {Date(19, April, 2025), 1183.5616438346101},
                {Date(20, April, 2025), 2367.1232876714103},
                {Date(21, April, 2025), 3550.6849315060199},
                {Date(22, April, 2025), 4734.6726575326247},
                {Date(15, May, 2025), 32018.095835783744},
                {Date(16, May, 2025), 33202.751031177795},
                {Date(17, May, 2025), 34384.799107109066},
                {Date(18, May, 2025), 35566.847183042526},
                {Date(19, May, 2025), 36748.895258971606},
                {Date(20, May, 2025), 36748.895258971606},
                {Date(21, May, 2025), 36748.895258971606},
                {Date(22, May, 2025), 0.0},
            }
        },
        // lookback lands on a fixing holiday without obs-shift
        {Date(25, April, 2025), Date(27, May, 2025), 5, false, 0, DayCounter(), Date(), Date(),
            {
                {Date(25, April, 2025), 0.0},
                {Date(26, April, 2025), 1200.0},
                {Date(27, April, 2025), 2400.0},
                {Date(28, April, 2025), 3600.0},
                {Date(22, May, 2025), 32462.79161128074},
                {Date(23, May, 2025), 33663.900351055265},
                {Date(24, May, 2025), 34862.365761374473},
                {Date(25, May, 2025), 36060.831171695892},
                {Date(26, May, 2025), 37259.296582012881},
                {Date(27, May, 2025), 38457.761992334308},
                {Date(28, May, 2025), 38457.761992334308},
                {Date(29, May, 2025), 38457.761992334308},
                {Date(30, May, 2025), 0.0},
            }
        },
        // lookback lands on a fixing holiday with obs-shift
        {Date(25, April, 2025), Date(27, May, 2025), 5, true, 0, DayCounter(), Date(), Date(),
            {
                {Date(25, April, 2025), 0.0},
                {Date(26, April, 2025), 1200.0},
                {Date(27, April, 2025), 2400.0},
                {Date(28, April, 2025), 3600.0},
                {Date(22, May, 2025), 32463.887689144285},
                {Date(23, May, 2025), 33665.178902847132},
                {Date(24, May, 2025), 34857.396658668215},
                {Date(25, May, 2025), 36059.3758537947},
                {Date(26, May, 2025), 37261.355048921192},
                {Date(27, May, 2025), 38463.334244047684},
                {Date(28, May, 2025), 38463.334244047684},
                {Date(29, May, 2025), 38463.334244047684},
                {Date(30, May, 2025), 0.0},
            }
        },
        // Coupon period spans fixing holiday
        {Date(14, April, 2025), Date(14, May, 2025), 0, false, 0, DayCounter(), Date(), Date(),
            {
                {Date(14, April, 2025), 0.0},
                {Date(15, April, 2025), 1202.7777777778858},
                {Date(16, April, 2025), 2414.0345586420867},
                {Date(17, April, 2025), 3611.5457944463355},
                {Date(18, April, 2025), 4811.9791799416953},
                {Date(19, April, 2025), 6012.4125654370528},
                {Date(10, May, 2025), 31317.54145558663},
                {Date(11, May, 2025), 32510.011880697577},
                {Date(12, May, 2025), 33702.482305808524},
                {Date(13, May, 2025), 34895.378045369798},
                {Date(14, May, 2025), 36093.990548857757},
                {Date(15, May, 2025), 36093.990548857757},
                {Date(16, May, 2025), 36093.990548857757},
                {Date(17, May, 2025), 0.0},
            }
        },
        // Partially fixed coupon
        {Date(14, April, 2025), Date(16, Jun, 2025), 0, false, 0, DayCounter(), Date(11, Jun, 2025), Date(14, Jun, 2025),
            {
                {Date(14, April, 2025), 0.0},
                {Date(15, April, 2025), 1202.7777777778858},
                {Date(16, April, 2025), 2414.0345586420867},
                {Date(17, April, 2025), 3611.5457944463355},
                {Date(18, April, 2025), 4811.9791799416953},
                {Date(19, April, 2025), 6012.4125654370528},
                {Date(28, May, 2025), 52812.944149152048},
                {Date(29, May, 2025), 54022.074150490429},
                {Date(30, May, 2025), 55231.349583297902},
                {Date(31, May, 2025), 56440.988690997779},
                {Date(1, Jun, 2025), 57650.627798697671},
                {Date(2, Jun, 2025), 58860.266906397563},
                {Date(3, Jun, 2025), 60070.197028672112},
                {Date(4, Jun, 2025), 61280.272687407327},
                {Date(5, Jun, 2025), 62490.493900104753},
                {Date(6, Jun, 2025), 63700.860684274834},
                {Date(7, Jun, 2025), 64911.518669761303},
                {Date(8, Jun, 2025), 66122.176655245552},
                {Date(9, Jun, 2025), 67332.83464073202},
                {Date(10, Jun, 2025), 68543.783885914861},
                {Date(11, Jun, 2025), 69754.878790140312},
                {Date(12, Jun, 2025), 70966.119370929882},
                {Date(13, Jun, 2025), 72177.505645807381},
                {Date(14, Jun, 2025), 73389.183367278398},
                {Date(15, Jun, 2025), 74600.861088749429},
                {Date(16, Jun, 2025), 75812.538810220431},
                {Date(17, Jun, 2025), 75812.538810220431},
                {Date(18, Jun, 2025), 75812.538810220431},
                {Date(19, Jun, 2025), 0.0},
            }
        },
        // Partially fixed coupon with 1d lockout
        {Date(14, April, 2025), Date(16, Jun, 2025), 0, false, 1, DayCounter(), Date(11, Jun, 2025), Date(14, Jun, 2025),
            {
                {Date(14, April, 2025), 0.0},
                {Date(15, April, 2025), 1202.7777777778858},
                {Date(16, April, 2025), 2414.0345586420867},
                {Date(17, April, 2025), 3611.5457944463355},
                {Date(18, April, 2025), 4811.9791799416953},
                {Date(19, April, 2025), 6012.4125654370528},
                {Date(28, May, 2025), 52812.944149152048},
                {Date(29, May, 2025), 54022.074150490429},
                {Date(30, May, 2025), 55231.349583297902},
                {Date(31, May, 2025), 56440.988690997779},
                {Date(1, Jun, 2025), 57650.627798697671},
                {Date(2, Jun, 2025), 58860.266906397563},
                {Date(3, Jun, 2025), 60070.197028672112},
                {Date(4, Jun, 2025), 61280.272687407327},
                {Date(5, Jun, 2025), 62490.493900104753},
                {Date(6, Jun, 2025), 63700.860684274834},
                {Date(7, Jun, 2025), 64911.518669761303},
                {Date(8, Jun, 2025), 66122.176655245552},
                {Date(9, Jun, 2025), 67332.83464073202},
                {Date(10, Jun, 2025), 68543.783885914861},
                {Date(11, Jun, 2025), 69754.878790140312},
                {Date(12, Jun, 2025), 70966.119370929882},
                {Date(13, Jun, 2025), 72177.505645807381},
                {Date(14, Jun, 2025), 73389.037632296531},
                {Date(15, Jun, 2025), 74600.569618785696},
                {Date(16, Jun, 2025), 75812.101605274845},
                {Date(17, Jun, 2025), 75812.101605274845},
                {Date(18, Jun, 2025), 75812.101605274845},
                {Date(19, Jun, 2025), 0.0},
            }
        },
        // Partially fixed coupon with 5d lockout
        {Date(14, April, 2025), Date(16, Jun, 2025), 0, false, 5, DayCounter(), Date(11, Jun, 2025), Date(19, Jun, 2025),
            {
                {Date(14, April, 2025), 0.0},
                {Date(15, April, 2025), 1202.7777777778858},
                {Date(16, April, 2025), 2414.0345586420867},
                {Date(17, April, 2025), 3611.5457944463355},
                {Date(18, April, 2025), 4811.9791799416953},
                {Date(19, April, 2025), 6012.4125654370528},
                {Date(28, May, 2025), 52812.944149152048},
                {Date(29, May, 2025), 54022.074150490429},
                {Date(30, May, 2025), 55231.349583297902},
                {Date(31, May, 2025), 56440.988690997779},
                {Date(1, Jun, 2025), 57650.627798697671},
                {Date(2, Jun, 2025), 58860.266906397563},
                {Date(3, Jun, 2025), 60070.197028672112},
                {Date(4, Jun, 2025), 61280.272687407327},
                {Date(5, Jun, 2025), 62490.493900104753},
                {Date(6, Jun, 2025), 63700.860684274834},
                {Date(7, Jun, 2025), 64911.518669761303},
                {Date(8, Jun, 2025), 66122.176655245552},
                {Date(9, Jun, 2025), 67332.83464073202},
                {Date(10, Jun, 2025), 68543.929550799468},
                {Date(11, Jun, 2025), 69755.170154954802},
                {Date(12, Jun, 2025), 70966.556470726253},
                {Date(13, Jun, 2025), 72178.088515641997},
                {Date(14, Jun, 2025), 73389.766307232479},
                {Date(15, Jun, 2025), 74601.44409882075},
                {Date(16, Jun, 2025), 75813.121890411217},
                {Date(17, Jun, 2025), 75813.121890411217},
                {Date(18, Jun, 2025), 75813.121890411217},
                {Date(19, Jun, 2025), 0.0},
            }
        },
        // partially fixed 5d lookback
        {Date(23, April, 2025), Date(23, Jun, 2025), 5, false, 0, DayCounter(), Date(), Date(),
            {
                {Date(23, April, 2025), 0.0},
                {Date(24, April, 2025), 1211.111111110160},
                {Date(25, April, 2025), 2408.478330246930},
                {Date(26, April, 2025), 3608.767347644810},
                {Date(27, April, 2025), 4809.056365047140},
                {Date(28, April, 2025), 6009.345382445020},
                {Date(28, May, 2025), 42072.0404553032},
                {Date(29, May, 2025), 43263.1417778784},
                {Date(30, May, 2025), 44451.5945829904},
                {Date(31, May, 2025), 45640.1880216828},
                {Date(8, Jun, 2025), 55246.0306584401},
                {Date(9, Jun, 2025), 56455.3805639378},
                {Date(10, Jun, 2025), 57665.0214144325},
                {Date(11, Jun, 2025), 58874.8077665890},
                {Date(12, Jun, 2025), 60084.7396379112},
                {Date(13, Jun, 2025), 61294.8170459027},
                {Date(14, Jun, 2025), 62505.1855855885},
                {Date(15, Jun, 2025), 63715.5541252765},
                {Date(16, Jun, 2025), 64925.9226649645},
                {Date(17, Jun, 2025), 66136.5823947113},
                {Date(18, Jun, 2025), 67347.3877486774},
                {Date(19, Jun, 2025), 68558.3387443800},
                {Date(20, Jun, 2025), 69769.2897400826},
                {Date(21, Jun, 2025), 70980.5320542944},
                {Date(22, Jun, 2025), 72191.7743685063},
                {Date(23, Jun, 2025), 73403.0166827182},
                {Date(24, Jun, 2025), 73403.0166827182},
                {Date(25, Jun, 2025), 73403.0166827182},
                {Date(26, Jun, 2025), 0.0},
            }
        },
        // partially fixed 5d lookback with obsshift
        {Date(23, April, 2025), Date(23, Jun, 2025), 5, true, 0, DayCounter(), Date(10, Jun, 2025), Date(21, Jun, 2025),
            {
                {Date(23, April, 2025), 0.0},
                {Date(24, April, 2025), 1211.111111110160},
                {Date(25, April, 2025), 2408.478330246930},
                {Date(26, April, 2025), 3604.817199922560},
                {Date(27, April, 2025), 4806.422933230080},
                {Date(28, April, 2025), 6008.0286665376},
                {Date(29, April, 2025), 7208.999619405480},
                {Date(28, May, 2025), 42077.6147132629},
                {Date(29, May, 2025), 43268.7166970069},
                {Date(30, May, 2025), 44457.1701618179},
                {Date(31, May, 2025), 45645.7642602892},
                {Date(8, Jun, 2025), 55248.4485876401},
                {Date(9, Jun, 2025), 56449.5018178061},
                {Date(10, Jun, 2025), 57659.1082384094},
                {Date(11, Jun, 2025), 58868.8643952469},
                {Date(12, Jun, 2025), 60078.7700574061},
                {Date(13, Jun, 2025), 61288.8250130815},
                {Date(14, Jun, 2025), 62515.4436336153},
                {Date(15, Jun, 2025), 63717.6637034925},
                {Date(16, Jun, 2025), 64919.8837733697},
                {Date(17, Jun, 2025), 66130.5341082391},
                {Date(18, Jun, 2025), 67341.3329076791},
                {Date(19, Jun, 2025), 68552.2800428873},
                {Date(20, Jun, 2025), 69754.9516225870},
                {Date(21, Jun, 2025), 70966.1922121385},
                {Date(22, Jun, 2025), 72169.0090292934},
                {Date(23, Jun, 2025), 73371.8258464483},
                {Date(24, Jun, 2025), 73371.8258464483},
                {Date(25, Jun, 2025), 73371.8258464483},
                {Date(26, Jun, 2025), 0.0},
            }
        },
        // partially fixed 5d lookback with 4d lockout
        {Date(23, April, 2025), Date(23, Jun, 2025), 5, false, 5, DayCounter(), Date(), Date(),
            {
                {Date(23, April, 2025), 0.0},
                {Date(24, April, 2025), 1211.111111110160},
                {Date(25, April, 2025), 2408.478330246930},
                {Date(26, April, 2025), 3608.767347644810},
                {Date(27, April, 2025), 4809.056365047140},
                {Date(28, April, 2025), 6009.345382445020},
                {Date(28, May, 2025), 42072.0404553032},
                {Date(29, May, 2025), 43263.1417778784},
                {Date(30, May, 2025), 44451.5945829904},
                {Date(31, May, 2025), 45640.1880216828},
                {Date(8, Jun, 2025), 55246.0306584401},
                {Date(9, Jun, 2025), 56455.3805639378},
                {Date(10, Jun, 2025), 57665.0214144325},
                {Date(11, Jun, 2025), 58874.8077665890},
                {Date(12, Jun, 2025), 60084.7396379112},
                {Date(13, Jun, 2025), 61294.8170459027},
                {Date(14, Jun, 2025), 62505.0400080696},
                {Date(15, Jun, 2025), 63715.2629702364},
                {Date(16, Jun, 2025), 64925.4859324033},
                {Date(17, Jun, 2025), 66136.1456096188},
                {Date(18, Jun, 2025), 67346.9509110469},
                {Date(19, Jun, 2025), 68557.9018542048},
                {Date(20, Jun, 2025), 69768.8527973628},
                {Date(21, Jun, 2025), 70980.0950590167},
                {Date(22, Jun, 2025), 72191.3373206706},
                {Date(23, Jun, 2025), 73402.5795823268},
                {Date(24, Jun, 2025), 73402.5795823268},
                {Date(25, Jun, 2025), 73402.5795823268},
                {Date(26, Jun, 2025), 0.0},
            }
        },
        // partially fixed 5d lookback with obsshift and 4d lockout
        {Date(23, April, 2025), Date(23, Jun, 2025), 5, true, 4, DayCounter(), Date(10, Jun, 2025), Date(26, Jun, 2025),
            {
                {Date(23, April, 2025), 0.0},
                {Date(24, April, 2025), 1211.111111110160},
                {Date(25, April, 2025), 2408.478330246930},
                {Date(26, April, 2025), 3604.817199922560},
                {Date(27, April, 2025), 4806.422933230080},
                {Date(28, April, 2025), 6008.0286665376},
                {Date(29, April, 2025), 7208.999619405480},
                {Date(28, May, 2025), 42077.6147132629},
                {Date(29, May, 2025), 43268.7166970069},
                {Date(30, May, 2025), 44457.1701618179},
                {Date(31, May, 2025), 45645.7642602892},
                {Date(8, Jun, 2025), 55248.4485876401},
                {Date(9, Jun, 2025), 56449.5018178061},
                {Date(10, Jun, 2025), 57659.1082384094},
                {Date(11, Jun, 2025), 58868.8643952469},
                {Date(12, Jun, 2025), 60078.7700574061},
                {Date(13, Jun, 2025), 61288.8250130815},
                {Date(14, Jun, 2025), 62515.4436336153},
                {Date(15, Jun, 2025), 63717.6637034925},
                {Date(16, Jun, 2025), 64919.8837733697},
                {Date(17, Jun, 2025), 66130.6771547586},
                {Date(18, Jun, 2025), 67341.6191264041},
                {Date(19, Jun, 2025), 68552.7095548175},
                {Date(20, Jun, 2025), 69755.3886698143},
                {Date(21, Jun, 2025), 70966.7750118714},
                {Date(22, Jun, 2025), 72169.6017069879},
                {Date(23, Jun, 2025), 73372.4284021044},
                {Date(24, Jun, 2025), 73372.4284021044},
                {Date(25, Jun, 2025), 73372.4284021044},
                {Date(26, Jun, 2025), 0.0},
            }
        },
        // floating starts on holiday
        {Date(26, Mar, 2027), Date(28, Jun, 2027), 0, false, 0, DayCounter(), Date(), Date(),
            {
                {Date(26, Mar, 2027), 0.0},
                {Date(27, Mar, 2027), 1203.0671590612662}, // 1d of interest
                {Date(28, Mar, 2027), 2 * 1203.0671590612662}, // 2d of interest
                {Date(29, Mar, 2027), 3 * 1203.0671590612662}, // 3d of interest
                {Date(30, Mar, 2027), 4812.4857244213899},
                {Date(31, Mar, 2027), 6015.914708716874},
                {Date(25, Jun, 2027), 110054.18406154988},
                {Date(26, Jun, 2027), 111270.41832775576},
                {Date(27, Jun, 2027), 112486.65259396384},
                {Date(28, Jun, 2027), 113702.88686016972},
                {Date(29, Jun, 2027), 113702.88686016972},
                {Date(30, Jun, 2027), 113702.88686016972},
                {Date(1, Jul, 2027), 0.0},
            }
        },
        // floating ends on holiday
        {Date(26, Feb, 2027), Date(26, Mar, 2027), 0, false, 1, DayCounter(), Date(25, Mar, 2027), Date(26, Mar, 2027),
            {
                {Date(25, Mar, 2027), 32527.788409235647},
                {Date(26, Mar, 2027), 33734.551129034335}, // only get 1d of interest
                {Date(31, Mar, 2027), 0.0},
            }
        },
    };
    const auto makeCoupon = [&](const TestCase& tc, bool telescopicValueDates) {
        auto& dc = tc.dayCounter.empty() ? vars.sofr->dayCounter() : tc.dayCounter;
        return vars.makeCoupon(tc.startDate, tc.endDate, tc.lookbackDays, tc.lockoutDays, tc.obsShift,
                               telescopicValueDates, RateAveraging::Compound,
                               fedCal.advance(tc.endDate, 2, Days), 10000000.0, dc);
    };

    const auto checkAccruals = [](const ext::shared_ptr<OvernightIndexedCoupon>& cpn,
                                  const std::vector<std::pair<Date, double>>& accruals,
                                  Date telescopicLowerCut,
                                  Date telescopicUpperCut){
        for (const auto& [accrualDate, accrualAmnt] : accruals) {
            if (accrualDate < telescopicLowerCut || accrualDate >= telescopicUpperCut) {
                CHECK_OIS_COUPON_RESULT("accrual amount", cpn->accruedAmount(accrualDate), accrualAmnt, 1e-8);
            }
        }
    };

    for (const auto& tc : cases) {
        auto cpn = makeCoupon(tc, false);
        checkAccruals(cpn, tc.accruals, Date(), Date());
        if (cpn->canApplyTelescopicFormula()) {
            auto cpnTelescopic = makeCoupon(tc, true);
            checkAccruals(cpnTelescopic, tc.accruals, tc.telescopicLowerCut, tc.telescopicUpperCut);
        }
    }
}

BOOST_AUTO_TEST_CASE(testErrorWhenCurveNullOrTooNarrow) {
    BOOST_TEST_MESSAGE("Test expected error is raised when the curve cannot value the coupon...");

    CommonVars vars(Date(26, March, 2026));
    auto coupon = vars.makeCoupon(Date(31, March, 2026), Date(31, March, 2027), 0, 0, false,
                               true, RateAveraging::Compound, Date(2, April, 2027), 1.0);
    BOOST_CHECK_EXCEPTION(coupon->rate(), Error, ExpectedErrorMessage("null term structure set to this instance"));

    const auto getCurve = [&](Date endDate, bool extrapolate=false){
        auto curve = ext::make_shared<DiscountCurve>(
            std::vector<Date>{vars.today, endDate},
            std::vector<DiscountFactor>{1.0, 0.9},
            Actual360());
        if (extrapolate) {
            curve->enableExtrapolation();
        }
        return curve;
    };

    vars.forecastCurve.linkTo(getCurve(coupon->accrualEndDate() - 1));
    BOOST_CHECK_EXCEPTION(coupon->rate(), Error, ExpectedErrorMessage("coupon requires a range"));
    vars.forecastCurve.linkTo(getCurve(coupon->accrualEndDate()));
    BOOST_CHECK_NO_THROW(coupon->rate());
    vars.forecastCurve.linkTo(getCurve(coupon->accrualEndDate() - 1, true));
    BOOST_CHECK_NO_THROW(coupon->rate());
}

BOOST_AUTO_TEST_CASE(testAccruedAmountTradingExCouponAfterAccrualEndDate) {
    BOOST_TEST_MESSAGE("Test accruedAmount handles dates beyond accrualEndDate..");
    CommonVars vars(Date(26, March, 2026));
    vars.forecastCurve.linkTo(flatRate(0.04, Actual360()));
    const auto exCpnDate = Date(1, April, 2027);
    auto coupon = vars.makeCoupon(Date(31, March, 2026), Date(31, March, 2027), 0, 0, false, true,
                                  RateAveraging::Compound, Date(2, April, 2027), 1.0, DayCounter(),
                                  Date(), Date(), exCpnDate);
    BOOST_CHECK(coupon->tradingExCoupon(exCpnDate));
    CHECK_OIS_COUPON_RESULT("exCoupon accrued amount", coupon->accruedAmount(exCpnDate), 0.0, 1e-12);
}

BOOST_AUTO_TEST_CASE(testAccruedAmountInAdvance) {
    BOOST_TEST_MESSAGE("Test rate and accruedAmount for in advance compounding sofr swaps");
    CommonVars vars(Date(21, April, 2026));
    vars.forecastCurve.linkTo(flatRate(0.04, Actual360()));
    auto couponAdvance = vars.makeCoupon(Date(23, April, 2027), Date(23, April, 2028), 0, 0, false, true,
                                         RateAveraging::Compound, Date(25, April, 2028), 10000000.0,
                                         DayCounter(), Date(23, April, 2026), Date(23, April, 2027));
    auto couponArrears = vars.makeCoupon(couponAdvance->rateComputationStartDate(),
                                         couponAdvance->rateComputationEndDate(), 0, 0, false, true,
                                         RateAveraging::Compound, Date(), 10000000.0,
                                         DayCounter(), Date(), Date());
    const auto& advanceInterestDates = couponAdvance->interestDates();
    const auto& arrearsInterestDates = couponArrears->interestDates();
    BOOST_CHECK_EQUAL_COLLECTIONS(advanceInterestDates.begin(), advanceInterestDates.end(),
                                  arrearsInterestDates.begin(), arrearsInterestDates.end());
    const auto& advanceValueDates = couponAdvance->valueDates();
    const auto& arrearsValueDates = couponArrears->valueDates();
    BOOST_CHECK_EQUAL_COLLECTIONS(advanceValueDates.begin(), advanceValueDates.end(),
                                  arrearsValueDates.begin(), arrearsValueDates.end());
    CHECK_OIS_COUPON_RESULT("in advance swapletRate", couponAdvance->rate(),
                            couponArrears->rate(), 1e-12);
    const auto oneDayInterest = couponAdvance->rate() / 360.0 * couponAdvance->nominal();
    CHECK_OIS_COUPON_RESULT("in advance accruedAmount",
                            couponAdvance->accruedAmount(vars.today), 0.0, 1e-12);
    for (auto i = 0; i < 4; ++i){
        CHECK_OIS_COUPON_RESULT("in advance accruedAmount " << i << " days after accrualStartDate",
                                couponAdvance->accruedAmount(couponAdvance->accrualStartDate() + i),
                                i * oneDayInterest, 1e-12);
    }
    const auto totalAmount = couponAdvance->amount();
    CHECK_OIS_COUPON_RESULT("in advance accruedAmount 1 day prior to accrualEndDate",
                            couponAdvance->accruedAmount(couponAdvance->accrualEndDate() - 1),
                            (totalAmount - oneDayInterest), 1e-12);
    for (auto i = 0; i < 3; ++i) {
        CHECK_OIS_COUPON_RESULT("in advance accruedAmount " << i << " days after accrualEndDate",
                                totalAmount,
                                couponAdvance->accruedAmount(couponAdvance->accrualEndDate() + i), 1e-12);
    }
    CHECK_OIS_COUPON_RESULT("in advance accruedAmount day after paymentDate",
                            couponAdvance->accruedAmount(couponAdvance->date() + 1), 0.0, 1e-12);
}

BOOST_AUTO_TEST_CASE(testRateComputationStartDateFixingHoliday) {
    BOOST_TEST_MESSAGE("Test forward accruedAmount when rateComputationStartDate lands on a fixing holiday");
    CommonVars vars(Date(16, April, 2025));
    vars.forecastCurve.linkTo(flatRate(0.04, Actual360()));
    auto coupon = vars.makeCoupon(Date(21, April, 2025), Date(21, April, 2026), 0, 0, false, true,
                                  RateAveraging::Compound, Date(23, April, 2026), 10000000.0,
                                  DayCounter(), Date(18, April, 2025), Date(20, April, 2026));
    BOOST_CHECK(vars.sofr->fixingCalendar().isHoliday(coupon->rateComputationStartDate()));
    CHECK_OIS_COUPON_RESULT("accruedAmount", coupon->accruedAmount(vars.today), 0.0, 1e-12);
    CHECK_OIS_COUPON_RESULT("accruedAmount", coupon->accruedAmount(coupon->accrualStartDate()), 0.0, 1e-12);
    auto accrued = (
        (1.0 + vars.sofr->fixing(Date(17, April, 2025)) * 3.0 / 360.0)
        * (1.0 + vars.sofr->fixing(Date(21, April, 2025)) / 360.0) - 1.0
    ) / 4.0 * coupon->nominal();
    CHECK_OIS_COUPON_RESULT("accruedAmount", coupon->accruedAmount(coupon->accrualStartDate() + 1), accrued, 1e-8);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
