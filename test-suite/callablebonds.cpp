/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 StatPro Italia srl

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

#include "callablebonds.hpp"
#include "utilities.hpp"
#include <ql/experimental/callablebonds/callablebond.hpp>
#include <ql/experimental/callablebonds/treecallablebondengine.hpp>
#include <ql/instruments/bonds/zerocouponbond.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/schedule.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <boost/make_shared.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct Globals {
        // global data
        Date today, settlement;
        Calendar calendar;
        DayCounter dayCounter;
        BusinessDayConvention rollingConvention;

        RelinkableHandle<YieldTermStructure> termStructure;
        RelinkableHandle<ShortRateModel> model;

        SavedSettings backup;

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

            today = Date::todaysDate();
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today,2,Days);
        }
    };

}

void CallableBondTest::testInterplay() {

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
                                vars.maturityDate(), Thirty360(),
                                vars.rollingConvention, 100.0,
                                vars.issueDate(), callabilities);
    bond.setPricingEngine(engine);

    double expected = callabilities[0]->price().amount() *
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
                                  vars.maturityDate(), Thirty360(),
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
                                  vars.maturityDate(), Thirty360(),
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
                                  vars.maturityDate(), Thirty360(),
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


void CallableBondTest::testConsistency() {

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
                       coupons, Thirty360());
    bond.setPricingEngine(
               ext::make_shared<DiscountingBondEngine>(vars.termStructure));

    CallabilitySchedule callabilities;
    std::vector<Date> callabilityDates = vars.evenYears();
    for (Size i=0; i<callabilityDates.size(); ++i) {
        callabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(110.0, Bond::Price::Clean),
                         Callability::Call, callabilityDates[i]));
    }

    CallabilitySchedule puttabilities;
    std::vector<Date> puttabilityDates = vars.oddYears();
    for (Size i=0; i<puttabilityDates.size(); ++i) {
        puttabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(90.0, Bond::Price::Clean),
                         Callability::Put, puttabilityDates[i]));
    }

    Size timeSteps = 240;

    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<TreeCallableFixedRateBondEngine>(
                                *(vars.model), timeSteps, vars.termStructure);

    CallableFixedRateBond callable(3, 100.0, schedule,
                                   coupons, Thirty360(),
                                   vars.rollingConvention,
                                   100.0, vars.issueDate(),
                                   callabilities);
    callable.setPricingEngine(engine);

    CallableFixedRateBond puttable(3, 100.0, schedule,
                                   coupons, Thirty360(),
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


void CallableBondTest::testObservability() {

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
    for (Size i=0; i<callabilityDates.size(); ++i) {
        callabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(110.0, Bond::Price::Clean),
                         Callability::Call, callabilityDates[i]));
    }
    std::vector<Date> puttabilityDates = vars.oddYears();
    for (Size i=0; i<puttabilityDates.size(); ++i) {
        callabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(90.0, Bond::Price::Clean),
                         Callability::Put, puttabilityDates[i]));
    }

    CallableZeroCouponBond bond(3, 100.0, vars.calendar,
                                vars.maturityDate(), Thirty360(),
                                vars.rollingConvention, 100.0,
                                vars.issueDate(), callabilities);

    Size timeSteps = 240;

    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<TreeCallableFixedRateBondEngine>(
                                *(vars.model), timeSteps, vars.termStructure);

    bond.setPricingEngine(engine);

    double originalValue = bond.NPV();

    observable->setValue(0.04);

    if (bond.NPV() == originalValue)
        BOOST_ERROR(
            "callable coupon bond was not notified of observable change");


}

void CallableBondTest::testDegenerate() {

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
                             coupons, Thirty360());

    // no callability
    CallabilitySchedule callabilities;

    CallableZeroCouponBond bond1(3, 100.0, vars.calendar,
                                 vars.maturityDate(), Thirty360(),
                                 vars.rollingConvention, 100.0,
                                 vars.issueDate(), callabilities);

    CallableFixedRateBond bond2(3, 100.0, schedule,
                                coupons, Thirty360(),
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
    for (Size i=0; i<callabilityDates.size(); ++i) {
        callabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(10000.0, Bond::Price::Clean),
                         Callability::Call, callabilityDates[i]));
    }
    std::vector<Date> puttabilityDates = vars.oddYears();
    for (Size i=0; i<puttabilityDates.size(); ++i) {
        callabilities.push_back(ext::make_shared<Callability>(
                         Bond::Price(0.0, Bond::Price::Clean),
                         Callability::Put, puttabilityDates[i]));
    }

    bond1 = CallableZeroCouponBond(3, 100.0, vars.calendar,
                                   vars.maturityDate(), Thirty360(),
                                   vars.rollingConvention, 100.0,
                                   vars.issueDate(), callabilities);

    bond2 = CallableFixedRateBond(3, 100.0, schedule,
                                  coupons, Thirty360(),
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

void CallableBondTest::testCached() {

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
    for (Size i=0; i<callabilityDates.size(); ++i) {
        ext::shared_ptr<Callability> exercise =
            ext::make_shared<Callability>(
                         Bond::Price(110.0, Bond::Price::Clean),
                         Callability::Call, callabilityDates[i]);
        callabilities.push_back(exercise);
        all_exercises.push_back(exercise);
    }
    std::vector<Date> puttabilityDates = vars.oddYears();
    for (Size i=0; i<puttabilityDates.size(); ++i) {
        ext::shared_ptr<Callability> exercise =
            ext::make_shared<Callability>(
                         Bond::Price(100.0, Bond::Price::Clean),
                         Callability::Put, puttabilityDates[i]);
        puttabilities.push_back(exercise);
        all_exercises.push_back(exercise);
    }

    Size timeSteps = 240;

    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<TreeCallableFixedRateBondEngine>(
                                *(vars.model), timeSteps, vars.termStructure);

    double tolerance = 1.0e-8;

    double storedPrice1 = 110.60975477;
    CallableFixedRateBond bond1(3, 100.0, schedule,
                                coupons, Thirty360(),
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
    CallableFixedRateBond bond2(3, 100.0, schedule,
                                coupons, Thirty360(),
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
    CallableFixedRateBond bond3(3, 100.0, schedule,
                                coupons, Thirty360(),
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

test_suite* CallableBondTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Convertible-bond tests");
    suite->add(QUANTLIB_TEST_CASE(&CallableBondTest::testConsistency));
    suite->add(QUANTLIB_TEST_CASE(&CallableBondTest::testInterplay));
    suite->add(QUANTLIB_TEST_CASE(&CallableBondTest::testObservability));
    suite->add(QUANTLIB_TEST_CASE(&CallableBondTest::testDegenerate));
    suite->add(QUANTLIB_TEST_CASE(&CallableBondTest::testCached));
    return suite;
}

