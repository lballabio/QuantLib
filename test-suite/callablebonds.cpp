/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 StatPro Italia srl
 Copyright (C) 2021, 2022 Ralf Konrad Eckel

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
#include <ql/experimental/callablebonds/callablebond.hpp>
#include <ql/experimental/callablebonds/treecallablebondengine.hpp>
#include <ql/experimental/callablebonds/blackcallablebondengine.hpp>
#include <ql/instruments/bonds/zerocouponbond.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/schedule.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/shared_ptr.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CallableBondTests)

struct Globals {
    // global data
    Date today, settlement;
    Calendar calendar;
    DayCounter dayCounter;
    BusinessDayConvention rollingConvention;

    RelinkableHandle<YieldTermStructure> termStructure;
    RelinkableHandle<ShortRateModel> model;

    Date issueDate() const {
        // ensure that we're in mid-coupon
        return calendar.adjust(today - 100*Days);
    }

    Date maturityDate() const {
        // ensure that we're in mid-coupon
        return calendar.advance(issueDate(),10,Years);
    }

    std::vector<Date> evenYears() const {
        std::vector<Date> dates;
        for (Size i=2; i<10; i+=2)
            dates.push_back(calendar.advance(issueDate(),i,Years));
        return dates;
    }

    std::vector<Date> oddYears() const {
        std::vector<Date> dates;
        for (Size i=1; i<10; i+=2)
            dates.push_back(calendar.advance(issueDate(),i,Years));
        return dates;
    }

    template <class R>
    ext::shared_ptr<YieldTermStructure> makeFlatCurve(const R& r) const {
        return ext::shared_ptr<YieldTermStructure>(
                                  new FlatForward(settlement, r, dayCounter));
    }

    Globals() {
        calendar = TARGET();
        dayCounter = Actual365Fixed();
        rollingConvention = ModifiedFollowing;

        today = Settings::instance().evaluationDate();
        settlement = calendar.advance(today,2,Days);
    }
};


BOOST_AUTO_TEST_CASE(testInterplay) {

    BOOST_TEST_MESSAGE("Testing interplay of callability and puttability for callable bonds...");

    Globals vars;

    vars.termStructure.linkTo(vars.makeFlatCurve(0.03));
    vars.model.linkTo(ext::make_shared<HullWhite>(vars.termStructure));

    Size timeSteps = 240;

    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<TreeCallableZeroCouponBondEngine>(
                                *(vars.model), timeSteps, vars.termStructure);

    /* case 1: an earlier out-of-the-money callability must prevent
               a later in-the-money puttability
    */

    CallabilitySchedule callabilities;

    callabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(100.0, Bond::Price::Clean),
                         Callability::Call,
                         vars.calendar.advance(vars.issueDate(),4,Years)));

    callabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(1000.0, Bond::Price::Clean),
                         Callability::Put,
                         vars.calendar.advance(vars.issueDate(),6,Years)));

    CallableZeroCouponBond bond(3, 100.0, vars.calendar,
                                vars.maturityDate(), Thirty360(Thirty360::BondBasis),
                                vars.rollingConvention, 100.0,
                                vars.issueDate(), callabilities);
    bond.setPricingEngine(engine);

    Real expected = callabilities[0]->price().amount() *
                      vars.termStructure->discount(callabilities[0]->date()) /
                      vars.termStructure->discount(bond.settlementDate());

    if (std::fabs(bond.settlementValue() - expected) > 1.0e-2)
        BOOST_ERROR(
            "callability not exercised correctly:\n"
            << std::setprecision(5)
            << "    calculated NPV: " << bond.settlementValue() << "\n"
            << "    expected:       " << expected << "\n"
            << "    difference:     " << bond.settlementValue()-expected);

    /* case 2: same as case 1, with an added callability later on */

    callabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(100.0, Bond::Price::Clean),
                         Callability::Call,
                         vars.calendar.advance(vars.issueDate(),8,Years)));

    bond = CallableZeroCouponBond(3, 100.0, vars.calendar,
                                  vars.maturityDate(), Thirty360(Thirty360::BondBasis),
                                  vars.rollingConvention, 100.0,
                                  vars.issueDate(), callabilities);
    bond.setPricingEngine(engine);

    if (std::fabs(bond.settlementValue() - expected) > 1.0e-2)
        BOOST_ERROR(
            "callability not exercised correctly:\n"
            << std::setprecision(5)
            << "    calculated NPV: " << bond.settlementValue() << "\n"
            << "    expected:       " << expected << "\n"
            << "    difference:     " << bond.settlementValue()-expected);

    /* case 3: an earlier in-the-money puttability must prevent
               a later in-the-money callability
    */

    callabilities.clear();

    callabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(100.0, Bond::Price::Clean),
                         Callability::Put,
                         vars.calendar.advance(vars.issueDate(),4,Years)));

    callabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(10.0, Bond::Price::Clean),
                         Callability::Call,
                         vars.calendar.advance(vars.issueDate(),6,Years)));

    bond = CallableZeroCouponBond(3, 100.0, vars.calendar,
                                  vars.maturityDate(), Thirty360(Thirty360::BondBasis),
                                  vars.rollingConvention, 100.0,
                                  vars.issueDate(), callabilities);
    bond.setPricingEngine(engine);

    expected = callabilities[0]->price().amount() *
               vars.termStructure->discount(callabilities[0]->date()) /
               vars.termStructure->discount(bond.settlementDate());

    if (std::fabs(bond.settlementValue() - expected) > 1.0e-2)
        BOOST_ERROR(
            "puttability not exercised correctly:\n"
            << std::setprecision(5)
            << "    calculated NPV: " << bond.settlementValue() << "\n"
            << "    expected:       " << expected << "\n"
            << "    difference:     " << bond.settlementValue()-expected);

    /* case 4: same as case 3, with an added puttability later on */

    callabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(100.0, Bond::Price::Clean),
                         Callability::Put,
                         vars.calendar.advance(vars.issueDate(),8,Years)));

    bond = CallableZeroCouponBond(3, 100.0, vars.calendar,
                                  vars.maturityDate(), Thirty360(Thirty360::BondBasis),
                                  vars.rollingConvention, 100.0,
                                  vars.issueDate(), callabilities);
    bond.setPricingEngine(engine);

    if (std::fabs(bond.settlementValue() - expected) > 1.0e-2)
        BOOST_ERROR(
            "puttability not exercised correctly:\n"
            << std::setprecision(5)
            << "    calculated NPV: " << bond.settlementValue() << "\n"
            << "    expected:       " << expected << "\n"
            << "    difference:     " << bond.settlementValue()-expected);
}

BOOST_AUTO_TEST_CASE(testConsistency) {

    BOOST_TEST_MESSAGE("Testing consistency of callable bonds...");

    Globals vars;

    vars.termStructure.linkTo(vars.makeFlatCurve(0.032));
    vars.model.linkTo(ext::make_shared<HullWhite>(vars.termStructure));

    Schedule schedule =
        MakeSchedule()
        .from(vars.issueDate())
        .to(vars.maturityDate())
        .withCalendar(vars.calendar)
        .withFrequency(Semiannual)
        .withConvention(vars.rollingConvention)
        .withRule(DateGeneration::Backward);

    std::vector<Rate> coupons(1, 0.05);

    FixedRateBond bond(3, 100.0, schedule,
                       coupons, Thirty360(Thirty360::BondBasis));
    bond.setPricingEngine(
               ext::make_shared<DiscountingBondEngine>(vars.termStructure));

    CallabilitySchedule callabilities;
    std::vector<Date> callabilityDates = vars.evenYears();
    for (auto& callabilityDate : callabilityDates) {
        callabilities.push_back(ext::make_shared<Callability>(
            Bond::Price(110.0, Bond::Price::Clean), Callability::Call, callabilityDate));
    }

    CallabilitySchedule puttabilities;
    std::vector<Date> puttabilityDates = vars.oddYears();
    for (auto& puttabilityDate : puttabilityDates) {
        puttabilities.push_back(ext::make_shared<Callability>(Bond::Price(90.0, Bond::Price::Clean),
                                                              Callability::Put, puttabilityDate));
    }

    Size timeSteps = 240;

    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<TreeCallableFixedRateBondEngine>(
                                *(vars.model), timeSteps, vars.termStructure);

    CallableFixedRateBond callable(3, 100.0, schedule,
                                   coupons, Thirty360(Thirty360::BondBasis),
                                   vars.rollingConvention,
                                   100.0, vars.issueDate(),
                                   callabilities);
    callable.setPricingEngine(engine);

    CallableFixedRateBond puttable(3, 100.0, schedule,
                                   coupons, Thirty360(Thirty360::BondBasis),
                                   vars.rollingConvention,
                                   100.0, vars.issueDate(),
                                   puttabilities);
    puttable.setPricingEngine(engine);

    if (bond.cleanPrice() <= callable.cleanPrice())
        BOOST_ERROR(
            "inconsistent prices:\n"
            << std::setprecision(8)
            << "    plain bond: " << bond.cleanPrice() << "\n"
            << "    callable:   " << callable.cleanPrice() << "\n"
            << " (should be lower)");

    if (bond.cleanPrice() >= puttable.cleanPrice())
        BOOST_ERROR(
            "inconsistent prices:\n"
            << std::setprecision(8)
            << "    plain bond: " << bond.cleanPrice() << "\n"
            << "    puttable:   " << puttable.cleanPrice() << "\n"
            << " (should be higher)");
}

BOOST_AUTO_TEST_CASE(testObservability) {

    BOOST_TEST_MESSAGE("Testing observability of callable bonds...");

    Globals vars;

    ext::shared_ptr<SimpleQuote> observable(new SimpleQuote(0.03));
    Handle<Quote> h(observable);
    vars.termStructure.linkTo(vars.makeFlatCurve(h));
    vars.model.linkTo(ext::make_shared<HullWhite>(vars.termStructure));

    Schedule schedule =
        MakeSchedule()
        .from(vars.issueDate())
        .to(vars.maturityDate())
        .withCalendar(vars.calendar)
        .withFrequency(Semiannual)
        .withConvention(vars.rollingConvention)
        .withRule(DateGeneration::Backward);

    std::vector<Rate> coupons(1, 0.05);

    CallabilitySchedule callabilities;

    std::vector<Date> callabilityDates = vars.evenYears();
    for (auto& callabilityDate : callabilityDates) {
        callabilities.push_back(ext::make_shared<Callability>(
            Bond::Price(110.0, Bond::Price::Clean), Callability::Call, callabilityDate));
    }
    std::vector<Date> puttabilityDates = vars.oddYears();
    for (auto& puttabilityDate : puttabilityDates) {
        callabilities.push_back(ext::make_shared<Callability>(Bond::Price(90.0, Bond::Price::Clean),
                                                              Callability::Put, puttabilityDate));
    }

    CallableZeroCouponBond bond(3, 100.0, vars.calendar,
                                vars.maturityDate(), Thirty360(Thirty360::BondBasis),
                                vars.rollingConvention, 100.0,
                                vars.issueDate(), callabilities);

    Size timeSteps = 240;

    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<TreeCallableFixedRateBondEngine>(
                                *(vars.model), timeSteps, vars.termStructure);

    bond.setPricingEngine(engine);

    Real originalValue = bond.NPV();

    observable->setValue(0.04);

    if (bond.NPV() == originalValue)
        BOOST_ERROR(
            "callable coupon bond was not notified of observable change");


}

BOOST_AUTO_TEST_CASE(testDegenerate) {

    BOOST_TEST_MESSAGE("Repricing bonds using degenerate callable bonds...");

    Globals vars;

    vars.termStructure.linkTo(vars.makeFlatCurve(0.034));
    vars.model.linkTo(ext::make_shared<HullWhite>(vars.termStructure));

    Schedule schedule =
        MakeSchedule()
        .from(vars.issueDate())
        .to(vars.maturityDate())
        .withCalendar(vars.calendar)
        .withFrequency(Semiannual)
        .withConvention(vars.rollingConvention)
        .withRule(DateGeneration::Backward);

    std::vector<Rate> coupons(1, 0.05);

    ZeroCouponBond zeroCouponBond(3, vars.calendar, 100.0,
                                  vars.maturityDate(),
                                  vars.rollingConvention);
    FixedRateBond couponBond(3, 100.0, schedule,
                             coupons, Thirty360(Thirty360::BondBasis));

    // no callability
    CallabilitySchedule callabilities;

    CallableZeroCouponBond bond1(3, 100.0, vars.calendar,
                                 vars.maturityDate(), Thirty360(Thirty360::BondBasis),
                                 vars.rollingConvention, 100.0,
                                 vars.issueDate(), callabilities);

    CallableFixedRateBond bond2(3, 100.0, schedule,
                                coupons, Thirty360(Thirty360::BondBasis),
                                vars.rollingConvention,
                                100.0, vars.issueDate(),
                                callabilities);

    ext::shared_ptr<PricingEngine> discountingEngine =
        ext::make_shared<DiscountingBondEngine>(vars.termStructure);

    zeroCouponBond.setPricingEngine(discountingEngine);
    couponBond.setPricingEngine(discountingEngine);

    Size timeSteps = 240;

    ext::shared_ptr<PricingEngine> treeEngine =
        ext::make_shared<TreeCallableFixedRateBondEngine>(
                                *(vars.model), timeSteps, vars.termStructure);

    bond1.setPricingEngine(treeEngine);
    bond2.setPricingEngine(treeEngine);

    double tolerance = 1.0e-4;

    if (std::fabs(bond1.cleanPrice() - zeroCouponBond.cleanPrice()) > tolerance)
        BOOST_ERROR(
            "failed to reproduce zero-coupon bond price:\n"
            << std::setprecision(7)
            << "    calculated: " << bond1.cleanPrice() << "\n"
            << "    expected:   " << zeroCouponBond.cleanPrice());

    if (std::fabs(bond2.cleanPrice() - couponBond.cleanPrice()) > tolerance)
        BOOST_ERROR(
            "failed to reproduce fixed-rate bond price:\n"
            << std::setprecision(7)
            << "    calculated: " << bond2.cleanPrice() << "\n"
            << "    expected:   " << couponBond.cleanPrice());

    // out-of-the-money callability

    std::vector<Date> callabilityDates = vars.evenYears();
    for (auto& callabilityDate : callabilityDates) {
        callabilities.push_back(ext::make_shared<Callability>(
            Bond::Price(10000.0, Bond::Price::Clean), Callability::Call, callabilityDate));
    }
    std::vector<Date> puttabilityDates = vars.oddYears();
    for (auto& puttabilityDate : puttabilityDates) {
        callabilities.push_back(ext::make_shared<Callability>(Bond::Price(0.0, Bond::Price::Clean),
                                                              Callability::Put, puttabilityDate));
    }

    bond1 = CallableZeroCouponBond(3, 100.0, vars.calendar,
                                   vars.maturityDate(), Thirty360(Thirty360::BondBasis),
                                   vars.rollingConvention, 100.0,
                                   vars.issueDate(), callabilities);

    bond2 = CallableFixedRateBond(3, 100.0, schedule,
                                  coupons, Thirty360(Thirty360::BondBasis),
                                  vars.rollingConvention,
                                  100.0, vars.issueDate(),
                                  callabilities);

    bond1.setPricingEngine(treeEngine);
    bond2.setPricingEngine(treeEngine);

    if (std::fabs(bond1.cleanPrice() - zeroCouponBond.cleanPrice()) > tolerance)
        BOOST_ERROR(
            "failed to reproduce zero-coupon bond price:\n"
            << std::setprecision(7)
            << "    calculated: " << bond1.cleanPrice() << "\n"
            << "    expected:   " << zeroCouponBond.cleanPrice());

    if (std::fabs(bond2.cleanPrice() - couponBond.cleanPrice()) > tolerance)
        BOOST_ERROR(
            "failed to reproduce fixed-rate bond price:\n"
            << std::setprecision(7)
            << "    calculated: " << bond2.cleanPrice() << "\n"
            << "    expected:   " << couponBond.cleanPrice());
}

BOOST_AUTO_TEST_CASE(testCached) {

    BOOST_TEST_MESSAGE("Testing callable-bond value against cached values...");

    Globals vars;

    vars.today = Date(3,June,2004);
    Settings::instance().evaluationDate() = vars.today;
    vars.settlement = vars.calendar.advance(vars.today,3,Days);

    vars.termStructure.linkTo(vars.makeFlatCurve(0.032));
    vars.model.linkTo(ext::make_shared<HullWhite>(vars.termStructure));

    Schedule schedule =
        MakeSchedule()
        .from(vars.issueDate())
        .to(vars.maturityDate())
        .withCalendar(vars.calendar)
        .withFrequency(Semiannual)
        .withConvention(vars.rollingConvention)
        .withRule(DateGeneration::Backward);

    std::vector<Rate> coupons(1, 0.05);

    CallabilitySchedule callabilities;
    CallabilitySchedule puttabilities;
    CallabilitySchedule all_exercises;

    std::vector<Date> callabilityDates = vars.evenYears();
    for (auto& callabilityDate : callabilityDates) {
        ext::shared_ptr<Callability> exercise = ext::make_shared<Callability>(
            Bond::Price(110.0, Bond::Price::Clean), Callability::Call, callabilityDate);
        callabilities.push_back(exercise);
        all_exercises.push_back(exercise);
    }
    std::vector<Date> puttabilityDates = vars.oddYears();
    for (auto& puttabilityDate : puttabilityDates) {
        ext::shared_ptr<Callability> exercise = ext::make_shared<Callability>(
            Bond::Price(100.0, Bond::Price::Clean), Callability::Put, puttabilityDate);
        puttabilities.push_back(exercise);
        all_exercises.push_back(exercise);
    }

    Size timeSteps = 240;

    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<TreeCallableFixedRateBondEngine>(
                                *(vars.model), timeSteps, vars.termStructure);

    double tolerance = 1.0e-8;

    double storedPrice1 = 110.60975477;
    CallableFixedRateBond bond1(3, 10000.0, schedule,
                                coupons, Thirty360(Thirty360::BondBasis),
                                vars.rollingConvention,
                                100.0, vars.issueDate(),
                                callabilities);
    bond1.setPricingEngine(engine);

    if (std::fabs(bond1.cleanPrice() - storedPrice1) > tolerance)
        BOOST_ERROR(
            "failed to reproduce cached callable-bond price:\n"
            << std::setprecision(12)
            << "    calculated: " << bond1.cleanPrice() << "\n"
            << "    expected:   " << storedPrice1);

    double storedPrice2 = 115.16559362;
    CallableFixedRateBond bond2(3, 10000.0, schedule,
                                coupons, Thirty360(Thirty360::BondBasis),
                                vars.rollingConvention,
                                100.0, vars.issueDate(),
                                puttabilities);
    bond2.setPricingEngine(engine);

    if (std::fabs(bond2.cleanPrice() - storedPrice2) > tolerance)
        BOOST_ERROR(
            "failed to reproduce cached puttable-bond price:\n"
            << std::setprecision(12)
            << "    calculated: " << bond2.cleanPrice() << "\n"
            << "    expected:   " << storedPrice2);

    double storedPrice3 = 110.97509625;
    CallableFixedRateBond bond3(3, 10000.0, schedule,
                                coupons, Thirty360(Thirty360::BondBasis),
                                vars.rollingConvention,
                                100.0, vars.issueDate(),
                                all_exercises);
    bond3.setPricingEngine(engine);

    if (std::fabs(bond3.cleanPrice() - storedPrice3) > tolerance)
        BOOST_ERROR(
            "failed to reproduce cached callable/puttable-bond price:\n"
            << std::setprecision(12)
            << "    calculated: " << bond3.cleanPrice() << "\n"
            << "    expected:   " << storedPrice3);


}

BOOST_AUTO_TEST_CASE(testSnappingExerciseDate2ClosestCouponDate) {

    BOOST_TEST_MESSAGE("Testing snap of callability dates to the closest coupon date...");

    /* This is a test case inspired by
     * https://github.com/lballabio/QuantLib/issues/930#issuecomment-853886024 */

    auto today = Date(18, May, 2021);

    Settings::instance().evaluationDate() = today;

    auto calendar = UnitedStates(UnitedStates::FederalReserve);
    auto accrualDCC = Thirty360(Thirty360::Convention::USA);
    auto frequency = Semiannual;
    RelinkableHandle<YieldTermStructure> termStructure;
    termStructure.linkTo(ext::make_shared<FlatForward>(today, 0.02, Actual365Fixed()));

    auto makeBonds = [&calendar, &accrualDCC, frequency,
                      &termStructure](Date callDate, ext::shared_ptr<FixedRateBond>& fixedRateBond,
                                      ext::shared_ptr<CallableFixedRateBond>& callableBond) {
        auto settlementDays = 2;
        auto settlementDate = Date(20, May, 2021);
        auto coupon = 0.05;
        auto faceAmount = 100.00;
        auto redemption = faceAmount;
        auto maturityDate = Date(14, Feb, 2026);
        auto issueDate = settlementDate - 2 * 366 * Days;
        Schedule schedule = MakeSchedule()
                                .from(issueDate)
                                .to(maturityDate)
                                .withFrequency(frequency)
                                .withCalendar(calendar)
                                .withConvention(Unadjusted)
                                .withTerminationDateConvention(Unadjusted)
                                .backwards()
                                .endOfMonth(false);
        auto coupons = std::vector<Rate>(schedule.size() - 1, coupon);

        CallabilitySchedule callabilitySchedule;
        callabilitySchedule.push_back(ext::make_shared<Callability>(
            Bond::Price(faceAmount, Bond::Price::Clean), Callability::Type::Call, callDate));

        auto newCallableBond = ext::make_shared<CallableFixedRateBond>(
            settlementDays, faceAmount, schedule, coupons, accrualDCC,
            BusinessDayConvention::Following, redemption, issueDate, callabilitySchedule);

        auto model = ext::make_shared<HullWhite>(termStructure, 1e-12, 0.003);
        auto treeEngine = ext::make_shared<TreeCallableFixedRateBondEngine>(model, 40);
        newCallableBond->setPricingEngine(treeEngine);

        callableBond.swap(newCallableBond);

        auto fixedRateBondSchedule = schedule.until(callDate);
        auto fixedRateBondCoupons = std::vector<Rate>(schedule.size() - 1, coupon);

        auto newFixedRateBond = ext::make_shared<FixedRateBond>(
            settlementDays, faceAmount, fixedRateBondSchedule, fixedRateBondCoupons, accrualDCC,
            BusinessDayConvention::Following, redemption, issueDate);
        auto discountingEngine = ext::make_shared<DiscountingBondEngine>(termStructure);
        newFixedRateBond->setPricingEngine(discountingEngine);

        fixedRateBond.swap(newFixedRateBond);
    };

    auto initialCallDate = Date(14, Feb, 2022);
    Real tolerance = 1e-10;
    Real prevOAS = 0.0266;
    Real expectedOasStep = 0.00005;

    ext::shared_ptr<CallableFixedRateBond> callableBond;
    ext::shared_ptr<FixedRateBond> fixedRateBond;

    for (int i = -10; i < 11; i++) {
        auto callDate = initialCallDate + i * Days;
        if (calendar.isBusinessDay(callDate)) {
            makeBonds(callDate, fixedRateBond, callableBond);
            auto npvFixedRateBond = fixedRateBond->NPV();
            auto npvCallable = callableBond->NPV();

            if (std::fabs(npvCallable - npvFixedRateBond) > tolerance) {
                BOOST_ERROR("failed to reproduce bond price at "
                            << io::iso_date(callDate) << ":\n"
                            << std::setprecision(7) << "    calculated: " << npvCallable << "\n"
                            << "    expected:   " << npvFixedRateBond << " +/- " << std::scientific
                            << std::setprecision(1) << tolerance);
            }

            auto cleanPrice = callableBond->cleanPrice() - 2.0;
            auto oas = callableBond->OAS(cleanPrice, termStructure, accrualDCC,
                                         QuantLib::Continuous, frequency);
            if (prevOAS - oas < expectedOasStep) {
                BOOST_ERROR("failed to get expected change in OAS at "
                            << io::iso_date(callDate) << ":\n"
                            << std::setprecision(7) << "    calculated: " << oas << "\n"
                            << "      previous: " << prevOAS << "\n"
                            << "  should at least change by " << expectedOasStep);
            }
            prevOAS = oas;
        }
    }
}

BOOST_AUTO_TEST_CASE(testBlackEngine) {

    BOOST_TEST_MESSAGE("Testing Black engine for European callable bonds...");

    Globals vars;

    vars.today = Date(20, September, 2022);
    Settings::instance().evaluationDate() = vars.today;
    vars.settlement = vars.calendar.advance(vars.today, 3, Days);

    vars.termStructure.linkTo(vars.makeFlatCurve(0.03));

    CallabilitySchedule callabilities = {
        ext::make_shared<Callability>(
                         Bond::Price(100.0, Bond::Price::Clean),
                         Callability::Call,
                         vars.calendar.advance(vars.issueDate(),4,Years))
    };

    CallableZeroCouponBond bond(3, 10000.0, vars.calendar,
                                vars.maturityDate(), Thirty360(Thirty360::BondBasis),
                                vars.rollingConvention, 100.0,
                                vars.issueDate(), callabilities);

    bond.setPricingEngine(ext::make_shared<BlackCallableZeroCouponBondEngine>(
        Handle<Quote>(ext::make_shared<SimpleQuote>(0.3)), vars.termStructure));

    Real expected = 74.52915084;
    Real calculated = bond.cleanPrice();

    if (std::fabs(calculated - expected) > 1.0e-4)
        BOOST_ERROR(
            "failed to reproduce cached price:\n"
            << std::setprecision(5)
            << "    calculated NPV: " << calculated << "\n"
            << "    expected:       " << expected << "\n"
            << "    difference:     " << calculated - expected);
}

BOOST_AUTO_TEST_CASE(testImpliedVol) {

    BOOST_TEST_MESSAGE("Testing implied-volatility calculation for callable bonds...");

    Globals vars;

    vars.termStructure.linkTo(vars.makeFlatCurve(0.03));

    Schedule schedule =
        MakeSchedule()
        .from(vars.issueDate())
        .to(vars.maturityDate())
        .withCalendar(vars.calendar)
        .withFrequency(Semiannual)
        .withConvention(vars.rollingConvention)
        .withRule(DateGeneration::Backward);

    std::vector<Rate> coupons = { 0.01 };

    CallabilitySchedule callabilities = {
        ext::make_shared<Callability>(
                         Bond::Price(100.0, Bond::Price::Clean),
                         Callability::Call,
                         schedule.at(8))
    };

    CallableFixedRateBond bond(3, 10000.0, schedule,
                               coupons, Thirty360(Thirty360::BondBasis),
                               vars.rollingConvention,
                               100.0, vars.issueDate(),
                               callabilities);

    auto targetPrice = Bond::Price(78.50, Bond::Price::Dirty);
    Real volatility = bond.impliedVolatility(targetPrice,
                                             vars.termStructure,
                                             1e-8,  // accuracy
                                             200,   // max evaluations
                                             1e-4,  // min vol
                                             1.0);  // max vol

    bond.setPricingEngine(ext::make_shared<BlackCallableZeroCouponBondEngine>(
        Handle<Quote>(ext::make_shared<SimpleQuote>(volatility)), vars.termStructure));

    if (std::fabs(bond.dirtyPrice() - targetPrice.amount()) > 1.0e-4)
        BOOST_ERROR(
            "failed to reproduce target dirty price with implied volatility:\n"
            << std::setprecision(5)
            << "    calculated price: " << bond.dirtyPrice() << "\n"
            << "    expected:         " << targetPrice.amount() << "\n"
            << "    difference:       " << bond.dirtyPrice() - targetPrice.amount());

    targetPrice = Bond::Price(78.50, Bond::Price::Clean);
    volatility = bond.impliedVolatility(targetPrice,
                                        vars.termStructure,
                                        1e-8,  // accuracy
                                        200,   // max evaluations
                                        1e-4,  // min vol
                                        1.0);  // max vol

    bond.setPricingEngine(ext::make_shared<BlackCallableZeroCouponBondEngine>(
        Handle<Quote>(ext::make_shared<SimpleQuote>(volatility)), vars.termStructure));

    if (std::fabs(bond.cleanPrice() - targetPrice.amount()) > 1.0e-4)
        BOOST_ERROR(
            "failed to reproduce target clean price with implied volatility:\n"
            << std::setprecision(5)
            << "    calculated price: " << bond.cleanPrice() << "\n"
            << "    expected:         " << targetPrice.amount() << "\n"
            << "    difference:       " << bond.cleanPrice() - targetPrice.amount());
}

BOOST_AUTO_TEST_CASE(testCallableFixedRateBondWithArbitrarySchedule) {
    BOOST_TEST_MESSAGE("Testing callable fixed-rate bond with arbitrary schedule...");

    Globals vars;

    Natural settlementDays = 2;
    vars.today = Date(10, Jan, 2020);
    Settings::instance().evaluationDate() = vars.today;
    vars.settlement = vars.calendar.advance(vars.today, settlementDays, Days);

    vars.termStructure.linkTo(vars.makeFlatCurve(0.03));
    vars.model.linkTo(ext::make_shared<HullWhite>(vars.termStructure));

    Size timeSteps = 240;
    ext::shared_ptr<PricingEngine> engine = ext::make_shared<TreeCallableFixedRateBondEngine>(
        *(vars.model), timeSteps, vars.termStructure);

    std::vector<Date> dates(4);
    dates[0] = Date(20, February, 2020);
    dates[1] = Date(15, Aug, 2020);
    dates[2] = Date(25, Sep, 2021);
    dates[3] = Date(27, Jan, 2022);

    Schedule schedule(dates, vars.calendar, Unadjusted);

    CallabilitySchedule callabilities = {
        ext::make_shared<Callability>(
                         Bond::Price(100.0, Bond::Price::Clean), 
                         Callability::Call, 
                         dates[2])
    };

    std::vector<Rate> coupons(1, 0.06);

    CallableFixedRateBond callableBond(settlementDays, 100.0, schedule, coupons, vars.dayCounter,
                                       vars.rollingConvention, 100.0, vars.issueDate(), callabilities);
    callableBond.setPricingEngine(engine);

    BOOST_CHECK_NO_THROW(callableBond.cleanPrice());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
