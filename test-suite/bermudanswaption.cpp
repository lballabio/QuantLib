/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007 StatPro Italia srl

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
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
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
        boost::shared_ptr<IborIndex> index;
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
            index = boost::shared_ptr<IborIndex>(new Euribor6M(termStructure));
            calendar = index->fixingCalendar();
            today = calendar.adjust(Date::todaysDate());
            settlement = calendar.advance(today,settlementDays,Days);
        }

        // utilities
        boost::shared_ptr<VanillaSwap> makeSwap(Rate fixedRate) {
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
            boost::shared_ptr<VanillaSwap> swap(
                      new VanillaSwap(type, nominal,
                                      fixedSchedule, fixedRate, fixedDayCount,
                                      floatSchedule, index, 0.0,
                                      index->dayCounter()));
            swap->setPricingEngine(boost::shared_ptr<PricingEngine>(
                                   new DiscountingSwapEngine(termStructure)));
            return swap;
        }
    };

}


void BermudanSwaptionTest::testCachedValues() {

    BOOST_TEST_MESSAGE("Testing Bermudan swaption against cached values...");

    CommonVars vars;

    vars.today = Date(15, February, 2002);

    Settings::instance().evaluationDate() = vars.today;

    vars.settlement = Date(19, February, 2002);
    // flat yield term structure impling 1x5 swap at 5%
    vars.termStructure.linkTo(flatRate(vars.settlement,
                                          0.04875825,
                                          Actual365Fixed()));

    Rate atmRate = vars.makeSwap(0.0)->fairRate();

    boost::shared_ptr<VanillaSwap> itmSwap = vars.makeSwap(0.8*atmRate);
    boost::shared_ptr<VanillaSwap> atmSwap = vars.makeSwap(atmRate);
    boost::shared_ptr<VanillaSwap> otmSwap = vars.makeSwap(1.2*atmRate);

    Real a = 0.048696, sigma = 0.0058904;
    boost::shared_ptr<HullWhite> model(new HullWhite(vars.termStructure,
                                                     a, sigma));
    std::vector<Date> exerciseDates;
    const Leg& leg = atmSwap->fixedLeg();
    for (Size i=0; i<leg.size(); i++) {
        boost::shared_ptr<Coupon> coupon =
            boost::dynamic_pointer_cast<Coupon>(leg[i]);
            exerciseDates.push_back(coupon->accrualStartDate());
    }
    boost::shared_ptr<Exercise> exercise(new BermudanExercise(exerciseDates));

    boost::shared_ptr<PricingEngine> treeEngine(
                                            new TreeSwaptionEngine(model, 50));
    boost::shared_ptr<PricingEngine> fdmEngine(
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
        boost::shared_ptr<Exercise>(new BermudanExercise(exerciseDates));

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


test_suite* BermudanSwaptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Bermudan swaption tests");
    suite->add(QUANTLIB_TEST_CASE(&BermudanSwaptionTest::testCachedValues));
    return suite;
}

