/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2021 Marcin Rybacki

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
#include <ql/cashflows/subperiodcoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/calendars/target.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(SubPeriodsCouponTests)

struct CommonVars {

    Date today;
    Calendar calendar;
    DayCounter dayCount;
    BusinessDayConvention businessConvention;

    ext::shared_ptr<IborIndex> euribor;
    RelinkableHandle<YieldTermStructure> euriborHandle;

    // utilities

    CommonVars() {
        dayCount = Actual365Fixed();
        businessConvention = ModifiedFollowing;

        euribor = ext::make_shared<Euribor1M>(euriborHandle);
        euribor->addFixing(Date(13, January, 2021), 0.0077);
        euribor->addFixing(Date(11, February, 2021), 0.0075);
        euribor->addFixing(Date(11, March, 2021), 0.0073);

        calendar = euribor->fixingCalendar();
        today = calendar.adjust(Date(15, March, 2021));
        Settings::instance().evaluationDate() = today;

        euriborHandle.linkTo(flatRate(today, 0.007, dayCount));
    }

    Schedule createSchedule(const Date& start, const Date& end) {
        Schedule s = MakeSchedule()
            .from(start)
            .to(end)
            .withTenor(euribor->tenor())
            .withCalendar(euribor->fixingCalendar())
            .withConvention(euribor->businessDayConvention())
            .backwards();
        return s;
    }

    Leg createIborLeg(const Date& start, const Date& end, Spread spread) {
        Schedule sch = createSchedule(start, end);
        return IborLeg(sch, euribor)
            .withNotionals(1.0)
            .withSpreads(spread)
            .withExCouponPeriod(2 * Days, calendar, businessConvention)
            .withPaymentLag(1)
            .withFixingDays(euribor->fixingDays());
    }

    ext::shared_ptr<CashFlow> createSubPeriodsCoupon(const Date& start,
                                                     const Date& end,
                                                     Spread rateSpread = 0.0,
                                                     RateAveraging::Type averaging = RateAveraging::Compound) {
        Calendar paymentCalendar = euribor->fixingCalendar();
        BusinessDayConvention paymentBdc = euribor->businessDayConvention();
        Date paymentDate = paymentCalendar.advance(end, 1 * Days, paymentBdc);
        Date exCouponDate = paymentCalendar.advance(paymentDate, -2 * Days, paymentBdc);
        ext::shared_ptr<FloatingRateCoupon> cpn(new SubPeriodsCoupon(
                paymentDate, 1.0, start, end, euribor->fixingDays(), euribor, 1.0, 0.0,
                rateSpread, Date(), Date(), DayCounter(), exCouponDate));
        if (averaging == RateAveraging::Compound)
            cpn->setPricer(ext::make_shared<CompoundingRatePricer>());
        else
            cpn->setPricer(ext::make_shared<AveragingRatePricer>());
        return cpn;
    }

    SubPeriodsLeg createSubPeriodsLeg(const Date& start,
                                      const Date& end) {
        Schedule s = createSchedule(start, end);
        return SubPeriodsLeg(s, euribor)
            .withNotionals(1.0)
            .withExCouponPeriod(2 * Days, calendar, businessConvention)
            .withPaymentLag(1)
            .withFixingDays(2)
            .withRateSpreads(0.0)
            .withCouponSpreads(0.0)
            .withAveragingMethod(RateAveraging::Compound);
    }

};


BOOST_AUTO_TEST_CASE(testRegularCompoundedForwardStartingCouponWithMultipleSubPeriods) {
    BOOST_TEST_MESSAGE("Testing coupon with multiple compounded sub-periods...");

    CommonVars vars;

    Date start = vars.today - 2 * Months;
    Date end = start + 6 * Months;

    Spread spread = 0.001;

    Leg iborLeg = vars.createIborLeg(start, end, spread);

    auto subPeriodCpn = vars.createSubPeriodsCoupon(start, end, spread, RateAveraging::Compound);

    const Real tolerance = 1.0e-14;

    Real actualPayment = subPeriodCpn->amount();

    Real compound = 1.0;
    for (const auto& cf : iborLeg) {
        auto cpn = ext::dynamic_pointer_cast<IborCoupon>(cf);
        Real yearFraction = cpn->accrualPeriod();
        Rate fixing = vars.euribor->fixing(cpn->fixingDate());
        compound *= (1.0 + yearFraction * (fixing + cpn->spread()));
    }
    Real expectedPayment = compound - 1.0;

    if (std::fabs(actualPayment - expectedPayment) > tolerance)
        BOOST_ERROR("unable to replicate compounded multiple sub-period coupon payment\n"
                    << std::setprecision(5) << "    calculated:    " << actualPayment << "\n"
                    << "    expected:    " << expectedPayment << "\n"
                    << "    start:    " << start << "\n"
                    << "    end:    " << end << "\n");
}

BOOST_AUTO_TEST_CASE(testRegularAveragedForwardStartingCouponWithMultipleSubPeriods) {
    BOOST_TEST_MESSAGE("Testing coupon with multiple averaged sub-periods...");

    CommonVars vars;

    Date start = vars.today - 2 * Months;
    Date end = start + 6 * Months;

    Spread spread = 0.001;

    Leg iborLeg = vars.createIborLeg(start, end, spread);

    auto subPeriodCpn = vars.createSubPeriodsCoupon(start, end, spread, RateAveraging::Simple);

    const Real tolerance = 1.0e-14;

    Real actualPayment = subPeriodCpn->amount();

    Real expectedPayment = 0.0;
    for (const auto& cf : iborLeg) {
        auto cpn = ext::dynamic_pointer_cast<IborCoupon>(cf);
        Real yearFraction = cpn->accrualPeriod();
        Rate fixing = vars.euribor->fixing(cpn->fixingDate());
        expectedPayment += yearFraction * (fixing + cpn->spread());
    }

    if (std::fabs(actualPayment - expectedPayment) > tolerance)
        BOOST_ERROR("unable to replicate averaged multiple sub-period coupon payment\n"
                    << std::setprecision(5) << "    calculated:    " << actualPayment << "\n"
                    << "    expected:    " << expectedPayment << "\n"
                    << "    start:    " << start << "\n"
                    << "    end:    " << end << "\n");
}

BOOST_AUTO_TEST_CASE(testExCouponCashFlow) {
    BOOST_TEST_MESSAGE("Testing ex-coupon cash flow...");

    CommonVars vars;

    Date start = vars.calendar.advance(vars.today, - 6 * Months);
    Date end = vars.today;

    Calendar paymentCalendar = vars.euribor->fixingCalendar();
    Date paymentDate = paymentCalendar.advance(end, 2 * Days);
    Date exCouponDate = paymentCalendar.advance(end, -2 * Days);

    auto cpn = ext::make_shared<SubPeriodsCoupon>(
                paymentDate, 1.0, start, end, 2, vars.euribor,
                1.0, 0.0, 0.0, Date(), Date(), DayCounter(), exCouponDate);
    cpn->setPricer(ext::make_shared<CompoundingRatePricer>());

    Real npv = CashFlows::npv({cpn}, **vars.euriborHandle, false, vars.today, vars.today);

    const Real tolerance = 1.0e-14;

    if (std::fabs(npv) > tolerance)
        BOOST_ERROR("cash flow was expected to go ex-coupon\n"
                    << std::setprecision(5) << "    calculated:    " << npv << "\n"
                    << "    expected:    " << 0.0 << "\n"
                    << "    start:    " << start << "\n"
                    << "    end:    " << end << "\n");
}

BOOST_AUTO_TEST_CASE(testSubPeriodsLegConsistencyChecks) {
    BOOST_TEST_MESSAGE("Testing sub-periods leg consistency checks...");

    CommonVars vars;

    Date start(18, March, 2021);
    Date end(18, March, 2031);

    Leg validLeg = vars.createSubPeriodsLeg(start, end);
    Size N = validLeg.size();

    BOOST_CHECK_THROW(
        Leg l0(vars.createSubPeriodsLeg(start, end).withNotionals(std::vector<Real>())),
        Error);

    BOOST_CHECK_THROW(
        Leg l1(vars.createSubPeriodsLeg(start, end)
               .withNotionals(std::vector<Real>(N + 1, 1.0))),
        Error);

    BOOST_CHECK_THROW(
        Leg l2(vars.createSubPeriodsLeg(start, end)
               .withFixingDays(std::vector<Natural>(N + 1, 2))),
        Error);

    BOOST_CHECK_THROW(
        Leg l3(vars.createSubPeriodsLeg(start, end).withGearings(0.0)),
        Error);

    BOOST_CHECK_THROW(
        Leg l4(vars.createSubPeriodsLeg(start, end)
               .withGearings(std::vector<Real>(N + 1, 1.0))),
        Error);

    BOOST_CHECK_THROW(
        Leg l5(vars.createSubPeriodsLeg(start, end)
               .withCouponSpreads(std::vector<Spread>(N + 1, 0.0))),
        Error);

    BOOST_CHECK_THROW(
        Leg l6(vars.createSubPeriodsLeg(start, end)
               .withRateSpreads(std::vector<Spread>(N + 1, 0.0))),
        Error);
}

BOOST_AUTO_TEST_CASE(testSubPeriodsLegRegression) {
    BOOST_TEST_MESSAGE("Testing number of fixing dates in sub-periods coupons...");

    Schedule schedule = MakeSchedule()
        .from({1, August, 2024})
        .to({1, August, 2025})
        .withFrequency(Monthly)
        .withCalendar(TARGET());

    Size resetsPerCoupon = 3;
    Leg leg = MultipleResetsLeg(schedule, ext::make_shared<Euribor1M>(), resetsPerCoupon)
        .withNotionals(100.0)
        .withAveragingMethod(RateAveraging::Compound);

    for (const auto& cf : leg) {
        auto c = ext::dynamic_pointer_cast<SubPeriodsCoupon>(cf);
        if (c->fixingDates().size() != 3)
            BOOST_ERROR("Unexpected number of fixing dates (" << c->fixingDates().size() << ") "
                        "in coupon paying on " << c->date());
    }
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
