/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007 StatPro Italia srl
 Copyright (C) 2016 Klaus Spanderen

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

#include "bermudanswaption.hpp"
#include "utilities.hpp"
#include <ql/instruments/swaption.hpp>
#include <ql/pricingengines/swaption/treeswaptionengine.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/pricingengines/swaption/fdhullwhiteswaptionengine.hpp>
#include <ql/pricingengines/swaption/fdg2swaptionengine.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/models/shortrate/twofactormodels/g2.hpp>
#include <ql/cashflows/coupon.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/schedule.hpp>


using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct CommonVars {
        // global data
        Date today, settlement;
        Calendar calendar;

        // underlying swap parameters
        Integer startYears, length;
        VanillaSwap::Type type;
        Real nominal;
        BusinessDayConvention fixedConvention, floatingConvention;
        Frequency fixedFrequency, floatingFrequency;
        DayCounter fixedDayCount;
        ext::shared_ptr<IborIndex> index;
        Natural settlementDays;

        RelinkableHandle<YieldTermStructure> termStructure;

        // cleanup
        SavedSettings backup;

        // setup
        CommonVars() {
            startYears = 1;
            length = 5;
            type = VanillaSwap::Payer;
            nominal = 1000.0;
            settlementDays = 2;
            fixedConvention = Unadjusted;
            floatingConvention = ModifiedFollowing;
            fixedFrequency = Annual;
            floatingFrequency = Semiannual;
            fixedDayCount = Thirty360();
            index = ext::shared_ptr<IborIndex>(new Euribor6M(termStructure));
            calendar = index->fixingCalendar();
            today = calendar.adjust(Date::todaysDate());
            settlement = calendar.advance(today,settlementDays,Days);
        }

        // utilities
        ext::shared_ptr<VanillaSwap> makeSwap(Rate fixedRate) {
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

}


void BermudanSwaptionTest::testCachedValues() {

    BOOST_TEST_MESSAGE(
        "Testing Bermudan swaption with HW model against cached values...");

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
    for (Size i=0; i<leg.size(); i++) {
        ext::shared_ptr<Coupon> coupon =
            ext::dynamic_pointer_cast<Coupon>(leg[i]);
            exerciseDates.push_back(coupon->accrualStartDate());
    }
    ext::shared_ptr<Exercise> exercise(new BermudanExercise(exerciseDates));

    ext::shared_ptr<PricingEngine> treeEngine(
                                            new TreeSwaptionEngine(model, 50));
    ext::shared_ptr<PricingEngine> fdmEngine(
                                         new FdHullWhiteSwaptionEngine(model));

    #if defined(QL_USE_INDEXED_COUPON)
    Real itmValue = 42.2413, atmValue = 12.8789, otmValue = 2.4759;
    Real itmValueFdm = 42.2111, atmValueFdm = 12.8879, otmValueFdm = 2.44443;
    #else
    Real itmValue = 42.2470, atmValue = 12.8826, otmValue = 2.4769;
    Real itmValueFdm = 42.2091, atmValueFdm = 12.8864, otmValueFdm = 2.4437;
    #endif

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


    for (Size j=0; j<exerciseDates.size(); j++)
        exerciseDates[j] = vars.calendar.adjust(exerciseDates[j]-10);
    exercise =
        ext::shared_ptr<Exercise>(new BermudanExercise(exerciseDates));

    #if defined(QL_USE_INDEXED_COUPON)
    itmValue = 42.1917; atmValue = 12.7788; otmValue = 2.4388;
    #else
    itmValue = 42.1974; atmValue = 12.7825; otmValue = 2.4399;
    #endif

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

void BermudanSwaptionTest::testCachedG2Values() {
    BOOST_TEST_MESSAGE(
        "Testing Bermudan swaption with G2 model against cached values...");

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
        for (Size i=0; i < swap->fixedLeg().size(); i++) {
            exerciseDates.push_back(ext::dynamic_pointer_cast<Coupon>(
                swap->fixedLeg()[i])->accrualStartDate());
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

#if defined(QL_USE_INDEXED_COUPON)
    const Real expectedFdm[] = { 103.231, 54.6519, 20.0475, 5.26941, 1.07097 };
    const Real expectedTree[]= { 103.253, 54.6685, 20.1399, 5.40517, 1.10642 };
#else
    const Real expectedFdm[] = { 103.227, 54.6502, 20.0469, 5.26924, 1.07093 };
    const Real expectedTree[]= { 103.256, 54.6726, 20.1429, 5.4064 , 1.10677 };
#endif

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

test_suite* BermudanSwaptionTest::suite(SpeedLevel speed) {
    test_suite* suite = BOOST_TEST_SUITE("Bermudan swaption tests");

    suite->add(QUANTLIB_TEST_CASE(&BermudanSwaptionTest::testCachedValues));

    if (speed == Slow) {
        suite->add(QUANTLIB_TEST_CASE(
            &BermudanSwaptionTest::testCachedG2Values));
    }

    return suite;
}

