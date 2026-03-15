/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007 StatPro Italia srl
 Copyright (C) 2016 Klaus Spanderen
 Copyright (C) 2021, 2022 Ralf Konrad Eckel

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

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/cashflows/coupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/indexes/ibor/eonia.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/instruments/overnightindexedswap.hpp>
#include <ql/instruments/swaption.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/models/shortrate/twofactormodels/g2.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/pricingengines/swaption/fdg2swaptionengine.hpp>
#include <ql/pricingengines/swaption/fdhullwhiteswaptionengine.hpp>
#include <ql/pricingengines/swaption/treeswaptionengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/schedule.hpp>


using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(BermudanSwaptionTests)

struct CommonVars {
    // global data
    Date today, settlement;
    Calendar calendar;

    // underlying swap parameters
    Integer startYears, length;
    Swap::Type type;
    Real nominal;
    BusinessDayConvention fixedConvention, floatingConvention;
    Frequency fixedFrequency, floatingFrequency;
    DayCounter fixedDayCount;
    ext::shared_ptr<IborIndex> index;
    Natural settlementDays;

    RelinkableHandle<YieldTermStructure> termStructure;

    // setup
    CommonVars() {
        startYears = 1;
        length = 5;
        type = Swap::Payer;
        nominal = 1000.0;
        settlementDays = 2;
        fixedConvention = Unadjusted;
        floatingConvention = ModifiedFollowing;
        fixedFrequency = Annual;
        floatingFrequency = Semiannual;
        fixedDayCount = Thirty360(Thirty360::BondBasis);
        index = ext::shared_ptr<IborIndex>(new Euribor6M(termStructure));
        calendar = index->fixingCalendar();
        today = calendar.adjust(Date::todaysDate());
        settlement = calendar.advance(today,settlementDays,Days);
    }

    // utilities
    ext::shared_ptr<VanillaSwap> makeSwap(Rate fixedRate) const {
        Date start = calendar.advance(settlement, startYears, Years);
        Date maturity = calendar.advance(start, length, Years);
        Schedule fixedSchedule(start, maturity,
                               Period(fixedFrequency),
                               calendar,
                               fixedConvention,
                               fixedConvention,
                               DateGeneration::Forward, false);
        Schedule floatSchedule(start, maturity,
                               Period(floatingFrequency),
                               calendar,
                               floatingConvention,
                               floatingConvention,
                               DateGeneration::Forward, false);
        ext::shared_ptr<VanillaSwap> swap(
                      new VanillaSwap(type, nominal,
                                      fixedSchedule, fixedRate, fixedDayCount,
                                      floatSchedule, index, 0.0,
                                      index->dayCounter()));
        swap->setPricingEngine(ext::shared_ptr<PricingEngine>(
                                   new DiscountingSwapEngine(termStructure)));
        return swap;
    }
};


BOOST_AUTO_TEST_CASE(testCachedValues) {

    BOOST_TEST_MESSAGE(
        "Testing Bermudan swaption with HW model against cached values...");

    bool usingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();

    CommonVars vars;

    vars.today = Date(15, February, 2002);

    Settings::instance().evaluationDate() = vars.today;

    vars.settlement = Date(19, February, 2002);
    // flat yield term structure impling 1x5 swap at 5%
    vars.termStructure.linkTo(flatRate(vars.settlement,
                                          0.04875825,
                                          Actual365Fixed()));

    Rate atmRate = vars.makeSwap(0.0)->fairRate();

    ext::shared_ptr<VanillaSwap> itmSwap = vars.makeSwap(0.8*atmRate);
    ext::shared_ptr<VanillaSwap> atmSwap = vars.makeSwap(atmRate);
    ext::shared_ptr<VanillaSwap> otmSwap = vars.makeSwap(1.2*atmRate);

    Real a = 0.048696, sigma = 0.0058904;
    ext::shared_ptr<HullWhite> model(new HullWhite(vars.termStructure,
                                                     a, sigma));
    std::vector<Date> exerciseDates;
    const Leg& leg = atmSwap->fixedLeg();
    for (const auto& i : leg) {
        ext::shared_ptr<Coupon> coupon = ext::dynamic_pointer_cast<Coupon>(i);
        exerciseDates.push_back(coupon->accrualStartDate());
    }
    ext::shared_ptr<Exercise> exercise(new BermudanExercise(exerciseDates));

    ext::shared_ptr<PricingEngine> treeEngine(
                                            new TreeSwaptionEngine(model, 50));
    ext::shared_ptr<PricingEngine> fdmEngine(
                                         new FdHullWhiteSwaptionEngine(model));

    Real itmValue,    atmValue,    otmValue;
    Real itmValueFdm, atmValueFdm, otmValueFdm;
    if (!usingAtParCoupons) {
        itmValue    = 42.2402,    atmValue = 12.9032,    otmValue = 2.49758;
        itmValueFdm = 42.2111, atmValueFdm = 12.8879, otmValueFdm = 2.44443;
    } else {
        itmValue    = 42.2460,    atmValue = 12.9069,    otmValue = 2.4985;
        itmValueFdm = 42.2091, atmValueFdm = 12.8864, otmValueFdm = 2.4437;
    }

    Real tolerance = 1.0e-4;

    Swaption swaption(itmSwap, exercise);
    swaption.setPricingEngine(treeEngine);
    if (std::fabs(swaption.NPV()-itmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached in-the-money swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << itmValue);
    swaption.setPricingEngine(fdmEngine);
    if (std::fabs(swaption.NPV()-itmValueFdm) > tolerance)
        BOOST_ERROR("failed to reproduce cached in-the-money swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << itmValueFdm);

    swaption = Swaption(atmSwap, exercise);
    swaption.setPricingEngine(treeEngine);
    if (std::fabs(swaption.NPV()-atmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached at-the-money swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << atmValue);

    swaption.setPricingEngine(fdmEngine);
    if (std::fabs(swaption.NPV()-atmValueFdm) > tolerance)
        BOOST_ERROR("failed to reproduce cached at-the-money swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << atmValueFdm);

    swaption = Swaption(otmSwap, exercise);
    swaption.setPricingEngine(treeEngine);
    if (std::fabs(swaption.NPV()-otmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached out-of-the-money "
                    << "swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << otmValue);

    swaption.setPricingEngine(fdmEngine);
    if (std::fabs(swaption.NPV()-otmValueFdm) > tolerance)
        BOOST_ERROR("failed to reproduce cached out-of-the-money "
                    << "swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << otmValueFdm);


    for (auto& exerciseDate : exerciseDates)
        exerciseDate = vars.calendar.adjust(exerciseDate - 10);
    exercise =
        ext::shared_ptr<Exercise>(new BermudanExercise(exerciseDates));

    if (!usingAtParCoupons) {
        itmValue = 42.1791; atmValue = 12.7699; otmValue = 2.4368;
    } else {
        itmValue = 42.1849; atmValue = 12.7736; otmValue = 2.4379;
    }

    swaption = Swaption(itmSwap, exercise);
    swaption.setPricingEngine(treeEngine);
    if (std::fabs(swaption.NPV()-itmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached in-the-money swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << itmValue);
    swaption = Swaption(atmSwap, exercise);
    swaption.setPricingEngine(treeEngine);
    if (std::fabs(swaption.NPV()-atmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached at-the-money swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << atmValue);
    swaption = Swaption(otmSwap, exercise);
    swaption.setPricingEngine(treeEngine);
    if (std::fabs(swaption.NPV()-otmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached out-of-the-money "
                    << "swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << otmValue);
}

BOOST_AUTO_TEST_CASE(testCachedG2Values, *precondition(if_speed(Fast))) {
    BOOST_TEST_MESSAGE(
        "Testing Bermudan swaption with G2 model against cached values...");

    bool usingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();

    CommonVars vars;

    vars.today = Date(15, September, 2016);
    Settings::instance().evaluationDate() = vars.today;
    vars.settlement = Date(19, September, 2016);

    // flat yield term structure impling 1x5 swap at 5%
    vars.termStructure.linkTo(flatRate(vars.settlement,
                                          0.04875825,
                                          Actual365Fixed()));

    const Rate atmRate = vars.makeSwap(0.0)->fairRate();
    std::vector<ext::shared_ptr<Swaption> > swaptions;
    for (Real s=0.5; s<1.51; s+=0.25) {
        const ext::shared_ptr<VanillaSwap> swap(vars.makeSwap(s*atmRate));

        std::vector<Date> exerciseDates;
        for (const auto& i : swap->fixedLeg()) {
            exerciseDates.push_back(ext::dynamic_pointer_cast<Coupon>(i)->accrualStartDate());
        }

        swaptions.push_back(ext::make_shared<Swaption>(swap,
            ext::make_shared<BermudanExercise>(exerciseDates)));
    }

    const Real a=0.1, sigma=0.01, b=0.2, eta=0.013, rho=-0.5;

    const ext::shared_ptr<G2> g2Model(ext::make_shared<G2>(
        vars.termStructure, a, sigma, b, eta, rho));
    const ext::shared_ptr<PricingEngine> fdmEngine(
        ext::make_shared<FdG2SwaptionEngine>(g2Model, 50, 75, 75, 0, 1e-3));
    const ext::shared_ptr<PricingEngine> treeEngine(
        ext::make_shared<TreeSwaptionEngine>(g2Model, 50));

    Real expectedFdm[5], expectedTree[5];
    if (!usingAtParCoupons) {
        Real tmpExpectedFdm[]  = { 103.231, 54.6519, 20.0475, 5.26941, 1.07097 };
        Real tmpExpectedTree[] = { 103.245, 54.6685, 20.1656, 5.43999, 1.12702 };
        std::copy(tmpExpectedFdm,  tmpExpectedFdm + 5,  expectedFdm);
        std::copy(tmpExpectedTree, tmpExpectedTree + 5, expectedTree);
    } else {
        Real tmpExpectedFdm[]  = { 103.227, 54.6502, 20.0469, 5.26924, 1.07093 };
        Real tmpExpectedTree[] = { 103.248, 54.6726, 20.1685, 5.44118, 1.12737 };
        std::copy(tmpExpectedFdm,  tmpExpectedFdm + 5,  expectedFdm);
        std::copy(tmpExpectedTree, tmpExpectedTree + 5, expectedTree);
    }

    const Real tol = 0.005;
    for (Size i=0; i < swaptions.size(); ++i) {
        swaptions[i]->setPricingEngine(fdmEngine);
        const Real calculatedFdm = swaptions[i]->NPV();

        if (std::fabs(calculatedFdm - expectedFdm[i]) > tol) {
            BOOST_ERROR("failed to reproduce cached G2 FDM swaption value:\n"
                        << "calculated: " << calculatedFdm << "\n"
                        << "expected:   " << expectedFdm[i]);
        }

        swaptions[i]->setPricingEngine(treeEngine);
        const Real calculatedTree = swaptions[i]->NPV();

        if (std::fabs(calculatedTree - expectedTree[i]) > tol) {
            BOOST_ERROR("failed to reproduce cached G2 Tree swaption value:\n"
                        << "calculated: " << calculatedTree << "\n"
                        << "expected:   " << expectedTree[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(testTreeEngineTimeSnapping) {
    BOOST_TEST_MESSAGE("Testing snap of exercise dates for discretized swaption...");

    Date today = Date(8, Jul, 2021);
    Settings::instance().evaluationDate() = today;

    RelinkableHandle<YieldTermStructure> termStructure;
    termStructure.linkTo(ext::make_shared<FlatForward>(today, 0.02, Actual365Fixed()));
    auto index = ext::make_shared<Euribor3M>(termStructure);

    auto makeBermudanSwaption = [&index](Date callDate) {
        auto effectiveDate = Date(15, May, 2025);
        ext::shared_ptr<VanillaSwap> swap = MakeVanillaSwap(Period(10, Years), index, 0.05)
                                                .withEffectiveDate(effectiveDate)
                                                .withNominal(10000.00)
                                                .withType(Swap::Type::Payer);

        std::vector<Date> exerciseDates{effectiveDate, callDate};
        auto bermudanExercise = ext::make_shared<BermudanExercise>(exerciseDates);
        auto bermudanSwaption = ext::make_shared<Swaption>(swap, bermudanExercise);

        return bermudanSwaption;
    };

    int intervalOfDaysToTest = 10;

    for (int i = -intervalOfDaysToTest; i < intervalOfDaysToTest + 1; i++) {
        static auto initialCallDate = Date(15, May, 2030);
        static auto calendar = index->fixingCalendar();

        auto callDate = initialCallDate + i * Days;
        if (calendar.isBusinessDay(callDate)) {

            auto bermudanSwaption = makeBermudanSwaption(callDate);

            auto model = ext::make_shared<HullWhite>(termStructure);

            bermudanSwaption->setPricingEngine(ext::make_shared<FdHullWhiteSwaptionEngine>(model));
            auto npvFD = bermudanSwaption->NPV();

            constexpr auto timesteps = 14 * 4 * 4;

            bermudanSwaption->setPricingEngine(
                ext::make_shared<TreeSwaptionEngine>(model, timesteps));
            auto npvTree = bermudanSwaption->NPV();

            auto npvDiff = npvTree - npvFD;

            static auto tolerance = 1.0;
            if (std::abs(npvTree - npvFD) > tolerance) {
                BOOST_ERROR(std::fixed << std::setprecision(2) << std::setw(5) << "At "
                                       << io::iso_date(callDate)
                                       << ": The difference between the npv of the FD and the tree "
                                          "engine is expected to be smaller than "
                                       << tolerance << " but was " << npvDiff << ". (FD: " << npvFD
                                       << ", tree: " << npvTree << ")");
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testBermudanOISSwaptionWithHW) {

    BOOST_TEST_MESSAGE(
        "Testing Bermudan swaption with OIS underlying and HW model...");

    CommonVars vars;

    vars.today = Date(15, February, 2002);
    Settings::instance().evaluationDate() = vars.today;
    vars.settlement = Date(19, February, 2002);

    vars.termStructure.linkTo(flatRate(vars.settlement,
                                       0.04875825,
                                       Actual365Fixed()));

    // Build OIS with same economics as the VanillaSwap test
    auto overnightIndex = ext::make_shared<Eonia>(vars.termStructure);

    Date start = vars.calendar.advance(vars.settlement, vars.startYears, Years);
    Date maturity = vars.calendar.advance(start, vars.length, Years);

    Schedule fixedSchedule(start, maturity,
                           Period(vars.fixedFrequency),
                           vars.calendar,
                           vars.fixedConvention,
                           vars.fixedConvention,
                           DateGeneration::Forward, false);
    Schedule overnightSchedule(start, maturity,
                               Period(vars.floatingFrequency),
                               vars.calendar,
                               vars.floatingConvention,
                               vars.floatingConvention,
                               DateGeneration::Forward, false);

    // Under a flat single curve, the VanillaSwap ATM rate equals the OIS ATM rate
    Rate atmRate = vars.makeSwap(0.0)->fairRate();

    auto makeOIS = [&](Rate fixedRate) {
        auto ois = ext::make_shared<OvernightIndexedSwap>(
            vars.type, vars.nominal,
            fixedSchedule, fixedRate, vars.fixedDayCount,
            overnightSchedule, overnightIndex, 0.0);
        ois->setPricingEngine(ext::make_shared<DiscountingSwapEngine>(vars.termStructure));
        return ois;
    };

    auto itmOIS = makeOIS(0.8 * atmRate);
    auto atmOIS = makeOIS(atmRate);
    auto otmOIS = makeOIS(1.2 * atmRate);

    // Build Bermudan exercise from fixed leg
    std::vector<Date> exerciseDates;
    for (const auto& cf : atmOIS->fixedLeg()) {
        auto coupon = ext::dynamic_pointer_cast<Coupon>(cf);
        exerciseDates.push_back(coupon->accrualStartDate());
    }
    auto exercise = ext::make_shared<BermudanExercise>(exerciseDates);

    Real a = 0.048696, sigma = 0.0058904;
    auto model = ext::make_shared<HullWhite>(vars.termStructure, a, sigma);
    auto fdmEngine = ext::make_shared<FdHullWhiteSwaptionEngine>(model);

    // Price OIS Bermudan swaptions
    Swaption itmSwaption(itmOIS, exercise);
    itmSwaption.setPricingEngine(fdmEngine);
    Real itmValue = itmSwaption.NPV();

    Swaption atmSwaption(atmOIS, exercise);
    atmSwaption.setPricingEngine(fdmEngine);
    Real atmValue = atmSwaption.NPV();

    Swaption otmSwaption(otmOIS, exercise);
    otmSwaption.setPricingEngine(fdmEngine);
    Real otmValue = otmSwaption.NPV();

    // OIS Bermudan should produce positive values
    if (itmValue <= 0.0)
        BOOST_ERROR("ITM OIS Bermudan swaption has non-positive value: "
                    << itmValue);
    if (atmValue <= 0.0)
        BOOST_ERROR("ATM OIS Bermudan swaption has non-positive value: "
                    << atmValue);
    if (otmValue <= 0.0)
        BOOST_ERROR("OTM OIS Bermudan swaption has non-positive value: "
                    << otmValue);

    // ITM > ATM > OTM monotonicity
    if (itmValue <= atmValue)
        BOOST_ERROR("ITM OIS Bermudan (" << itmValue
                    << ") should exceed ATM (" << atmValue << ")");
    if (atmValue <= otmValue)
        BOOST_ERROR("ATM OIS Bermudan (" << atmValue
                    << ") should exceed OTM (" << otmValue << ")");

    // Compare with VanillaSwap Bermudan - under HW, difference should be small
    auto itmVanilla = vars.makeSwap(0.8 * atmRate);
    auto atmVanilla = vars.makeSwap(atmRate);
    auto otmVanilla = vars.makeSwap(1.2 * atmRate);

    Swaption itmVS(itmVanilla, exercise);
    itmVS.setPricingEngine(fdmEngine);
    Swaption atmVS(atmVanilla, exercise);
    atmVS.setPricingEngine(fdmEngine);
    Swaption otmVS(otmVanilla, exercise);
    otmVS.setPricingEngine(fdmEngine);

    // Under single-factor HW with flat curve, VanillaSwap and OIS
    // Bermudans should be close (floating leg ≈ par in both cases).
    // Allow up to 5% relative difference.
    Real relTol = 0.05;

    auto relDiff = [](Real a, Real b) {
        return std::fabs(a - b) / std::max(std::fabs(b), 1e-10);
    };
    Real itmDiff = relDiff(itmValue, itmVS.NPV());
    Real atmDiff = relDiff(atmValue, atmVS.NPV());
    Real otmDiff = relDiff(otmValue, otmVS.NPV());

    if (itmDiff > relTol)
        BOOST_ERROR("ITM OIS vs VanillaSwap Bermudan difference too large: "
                    << std::setprecision(16) << std::scientific
                    << itmValue << " vs " << itmVS.NPV()
                    << " (rel diff " << itmDiff << ")");
    if (atmDiff > relTol)
        BOOST_ERROR("ATM OIS vs VanillaSwap Bermudan difference too large: "
                    << std::setprecision(16) << std::scientific
                    << atmValue << " vs " << atmVS.NPV()
                    << " (rel diff " << atmDiff << ")");
    if (otmDiff > relTol)
        BOOST_ERROR("OTM OIS vs VanillaSwap Bermudan difference too large: "
                    << std::setprecision(16) << std::scientific
                    << otmValue << " vs " << otmVS.NPV()
                    << " (rel diff " << otmDiff << ")");
}

BOOST_AUTO_TEST_CASE(testBermudanOISSwaptionWithG2) {

    BOOST_TEST_MESSAGE(
        "Testing Bermudan swaption with OIS underlying and G2 model...");

    CommonVars vars;

    vars.today = Date(15, February, 2002);
    Settings::instance().evaluationDate() = vars.today;
    vars.settlement = Date(19, February, 2002);

    vars.termStructure.linkTo(flatRate(vars.settlement,
                                       0.04875825,
                                       Actual365Fixed()));

    auto overnightIndex = ext::make_shared<Eonia>(vars.termStructure);

    Date start = vars.calendar.advance(vars.settlement, vars.startYears, Years);
    Date maturity = vars.calendar.advance(start, vars.length, Years);

    Schedule fixedSchedule(start, maturity,
                           Period(vars.fixedFrequency),
                           vars.calendar,
                           vars.fixedConvention,
                           vars.fixedConvention,
                           DateGeneration::Forward, false);
    Schedule overnightSchedule(start, maturity,
                               Period(vars.floatingFrequency),
                               vars.calendar,
                               vars.floatingConvention,
                               vars.floatingConvention,
                               DateGeneration::Forward, false);

    // Under a flat single curve, the VanillaSwap ATM rate equals the OIS ATM rate
    Rate atmRate = vars.makeSwap(0.0)->fairRate();

    auto atmOIS = ext::make_shared<OvernightIndexedSwap>(
        vars.type, vars.nominal,
        fixedSchedule, atmRate, vars.fixedDayCount,
        overnightSchedule, overnightIndex, 0.0);
    atmOIS->setPricingEngine(ext::make_shared<DiscountingSwapEngine>(vars.termStructure));

    std::vector<Date> exerciseDates;
    for (const auto& cf : atmOIS->fixedLeg()) {
        auto coupon = ext::dynamic_pointer_cast<Coupon>(cf);
        exerciseDates.push_back(coupon->accrualStartDate());
    }
    auto exercise = ext::make_shared<BermudanExercise>(exerciseDates);

    auto model = ext::make_shared<G2>(vars.termStructure);
    auto fdmEngine = ext::make_shared<FdG2SwaptionEngine>(model);

    Swaption oisSwaption(atmOIS, exercise);
    oisSwaption.setPricingEngine(fdmEngine);
    Real oisValue = oisSwaption.NPV();

    if (oisValue <= 0.0)
        BOOST_ERROR("ATM OIS Bermudan swaption (G2) has non-positive value: "
                    << oisValue);

    // Compare with VanillaSwap Bermudan
    auto atmVanilla = vars.makeSwap(atmRate);
    Swaption vsSwaption(atmVanilla, exercise);
    vsSwaption.setPricingEngine(fdmEngine);
    Real vsValue = vsSwaption.NPV();

    Real relDiff = std::fabs(oisValue - vsValue) / std::max(std::fabs(vsValue), 1e-10);
    Real relTol = 0.05;
    if (relDiff > relTol)
        BOOST_ERROR("ATM OIS vs VanillaSwap Bermudan (G2) difference too large: "
                    << std::setprecision(16) << std::scientific
                    << oisValue << " vs " << vsValue
                    << " (rel diff " << relDiff << ")");
}

BOOST_AUTO_TEST_CASE(testBermudanOISSwaptionPreservesFeatures) {

    BOOST_TEST_MESSAGE(
        "Testing that Bermudan OIS swaption preserves averaging and lockout...");

    CommonVars vars;

    vars.today = Date(15, February, 2002);
    Settings::instance().evaluationDate() = vars.today;
    vars.settlement = Date(19, February, 2002);

    vars.termStructure.linkTo(flatRate(vars.settlement,
                                       0.04875825,
                                       Actual365Fixed()));

    auto overnightIndex = ext::make_shared<Eonia>(vars.termStructure);

    Date start = vars.calendar.advance(vars.settlement, vars.startYears, Years);
    Date maturity = vars.calendar.advance(start, vars.length, Years);

    Schedule fixedSchedule(start, maturity,
                           Period(vars.fixedFrequency),
                           vars.calendar,
                           vars.fixedConvention,
                           vars.fixedConvention,
                           DateGeneration::Forward, false);
    Schedule overnightSchedule(start, maturity,
                               Period(vars.floatingFrequency),
                               vars.calendar,
                               vars.floatingConvention,
                               vars.floatingConvention,
                               DateGeneration::Forward, false);

    // Under a flat single curve, the VanillaSwap ATM rate equals the OIS ATM rate
    Rate atmRate = vars.makeSwap(0.0)->fairRate();

    auto makeOIS = [&](RateAveraging::Type avg, Natural lockout) {
        auto ois = ext::make_shared<OvernightIndexedSwap>(
            vars.type, vars.nominal,
            fixedSchedule, atmRate, vars.fixedDayCount,
            overnightSchedule, overnightIndex, 0.0,
            0,                    // paymentLag
            vars.floatingConvention,
            Calendar(),           // paymentCalendar
            false,                // telescopicValueDates
            avg,
            Null<Natural>(),      // lookbackDays
            lockout);
        ois->setPricingEngine(ext::make_shared<DiscountingSwapEngine>(vars.termStructure));
        return ois;
    };

    // Build exercise from fixed leg
    auto refOIS = makeOIS(RateAveraging::Compound, 0);
    std::vector<Date> exerciseDates;
    for (const auto& cf : refOIS->fixedLeg()) {
        auto coupon = ext::dynamic_pointer_cast<Coupon>(cf);
        exerciseDates.push_back(coupon->accrualStartDate());
    }
    auto exercise = ext::make_shared<BermudanExercise>(exerciseDates);

    Real a = 0.048696, sigma = 0.0058904;
    auto model = ext::make_shared<HullWhite>(vars.termStructure, a, sigma);
    auto fdmEngine = ext::make_shared<FdHullWhiteSwaptionEngine>(model);

    // Price with compound averaging (default)
    auto compoundOIS = makeOIS(RateAveraging::Compound, 0);
    Swaption compoundSwaption(compoundOIS, exercise);
    compoundSwaption.setPricingEngine(fdmEngine);
    Real compoundValue = compoundSwaption.NPV();

    // Price with simple averaging — should differ meaningfully
    auto simpleOIS = makeOIS(RateAveraging::Simple, 0);
    Swaption simpleSwaption(simpleOIS, exercise);
    simpleSwaption.setPricingEngine(fdmEngine);
    Real simpleValue = simpleSwaption.NPV();

    // Simple vs compound averaging produces ~9-10% difference at 5% rate
    // level (arithmetic vs geometric compounding over semi-annual periods).
    // Use a conservative 0.1% floor to avoid false passes.
    Real avgDiff = std::fabs(compoundValue - simpleValue)
                   / std::max(compoundValue, 1e-10);
    if (avgDiff < 0.001)
        BOOST_ERROR("Simple vs compound OIS Bermudan should differ,"
                    << " got " << std::setprecision(16) << std::scientific
                    << avgDiff * 100 << "% (compound=" << compoundValue
                    << ", simple=" << simpleValue << ")");

    // Price with lockout — should differ from plain compound
    auto lockoutOIS = makeOIS(RateAveraging::Compound, 5);
    Swaption lockoutSwaption(lockoutOIS, exercise);
    lockoutSwaption.setPricingEngine(fdmEngine);
    Real lockoutValue = lockoutSwaption.NPV();

    // Lockout freezes the last N fixings, producing a small but
    // non-zero change. Use relative tolerance rather than exact equality.
    Real lockDiff = std::fabs(lockoutValue - compoundValue)
                    / std::max(compoundValue, 1e-10);
    if (lockDiff < 1e-8)
        BOOST_ERROR("5-day lockout OIS Bermudan should differ from plain,"
                    << " rel diff = " << std::setprecision(16) << std::scientific
                    << lockDiff
                    << " (lockout=" << lockoutValue
                    << ", plain=" << compoundValue << ")");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
