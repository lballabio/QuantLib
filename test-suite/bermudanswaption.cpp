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
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/cashflows/coupon.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/schedule.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(BermudanSwaptionTest)

// global data
Date today_, settlement_;
Calendar calendar_;

// underlying swap parameters
Integer startYears_, length_;
VanillaSwap::Type type_;
Real nominal_;
BusinessDayConvention fixedConvention_, floatingConvention_;
Frequency fixedFrequency_, floatingFrequency_;
DayCounter fixedDayCount_;
boost::shared_ptr<IborIndex> index_;
Natural settlementDays_;

RelinkableHandle<YieldTermStructure> termStructure_;

// utilities

boost::shared_ptr<VanillaSwap> makeSwap(Rate fixedRate) {
    Date start = calendar_.advance(settlement_,startYears_,Years);
    Date maturity = calendar_.advance(start,length_,Years);
    Schedule fixedSchedule(start, maturity, Period(fixedFrequency_), calendar_,
                           fixedConvention_, fixedConvention_, false, false);
    Schedule floatSchedule(start, maturity, Period(floatingFrequency_), calendar_,
                           floatingConvention_, floatingConvention_, false, false);
    return boost::shared_ptr<VanillaSwap>(
            new VanillaSwap(type_,nominal_,
                            fixedSchedule,fixedRate,fixedDayCount_,
                            floatSchedule,index_,0.0,
                            index_->dayCounter(),termStructure_));
}

void setup() {
    startYears_ = 1;
    length_ = 5;
    type_ = VanillaSwap::Payer;
    nominal_ = 1000.0;
    settlementDays_ = 2;
    fixedConvention_ = Unadjusted;
    floatingConvention_ = ModifiedFollowing;
    fixedFrequency_ = Annual;
    floatingFrequency_ = Semiannual;
    fixedDayCount_ = Thirty360();
    index_ = boost::shared_ptr<IborIndex>(new Euribor6M(termStructure_));
    calendar_ = index_->fixingCalendar();
    today_ = calendar_.adjust(Date::todaysDate());
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
}

QL_END_TEST_LOCALS(BermudanSwaptionTest)


void BermudanSwaptionTest::testCachedValues() {

    BOOST_MESSAGE("Testing Bermudan swaption against cached values...");

    SavedSettings backup;

    setup();

    today_ = Date(15, February, 2002);

    Settings::instance().evaluationDate() = today_;

    settlement_ = Date(19, February, 2002);
    // flat yield term structure impling 1x5 swap at 5%
    termStructure_.linkTo(flatRate(settlement_,0.04875825,
                                   Actual365Fixed()));

    Rate atmRate = makeSwap(0.0)->fairRate();

    boost::shared_ptr<VanillaSwap> itmSwap = makeSwap(0.8*atmRate);
    boost::shared_ptr<VanillaSwap> atmSwap = makeSwap(atmRate);
    boost::shared_ptr<VanillaSwap> otmSwap = makeSwap(1.2*atmRate);

    Real a = 0.048696, sigma = 0.0058904;
    boost::shared_ptr<ShortRateModel> model(new HullWhite(termStructure_,
                                                          a, sigma));
    std::vector<Date> exerciseDates;
    const Leg& leg = atmSwap->fixedLeg();
    for (Size i=0; i<leg.size(); i++) {
        boost::shared_ptr<Coupon> coupon =
            boost::dynamic_pointer_cast<Coupon>(leg[i]);
        exerciseDates.push_back(coupon->accrualStartDate());
    }
    boost::shared_ptr<Exercise> exercise(new BermudanExercise(exerciseDates));

    boost::shared_ptr<PricingEngine> engine(new TreeSwaptionEngine(model, 50));

    #if defined(QL_USE_INDEXED_COUPON)
    Real itmValue = 42.2413, atmValue = 12.8789, otmValue = 2.4759;
    #else
    Real itmValue = 42.2470, atmValue = 12.8826, otmValue = 2.4769;
    #endif

    Real tolerance = 1.0e-4;

    Swaption swaption(itmSwap, exercise, termStructure_, engine);
    if (std::fabs(swaption.NPV()-itmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached in-the-money swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << itmValue);
    swaption = Swaption(atmSwap, exercise, termStructure_, engine);
    if (std::fabs(swaption.NPV()-atmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached at-the-money swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << atmValue);
    swaption = Swaption(otmSwap, exercise, termStructure_, engine);
    if (std::fabs(swaption.NPV()-otmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached out-of-the-money "
                    << "swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << otmValue);

    for (Size j=0; j<exerciseDates.size(); j++)
        exerciseDates[j] = calendar_.adjust(exerciseDates[j]-10);
    exercise =
        boost::shared_ptr<Exercise>(new BermudanExercise(exerciseDates));

    #if defined(QL_USE_INDEXED_COUPON)
    itmValue = 42.1917; atmValue = 12.7788; otmValue = 2.4388;
    #else
    itmValue = 42.1974; atmValue = 12.7825; otmValue = 2.4399;
    #endif

    swaption = Swaption(itmSwap, exercise, termStructure_, engine);
    if (std::fabs(swaption.NPV()-itmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached in-the-money swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << itmValue);
    swaption = Swaption(atmSwap, exercise, termStructure_, engine);
    if (std::fabs(swaption.NPV()-atmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached at-the-money swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << atmValue);
    swaption = Swaption(otmSwap, exercise, termStructure_, engine);
    if (std::fabs(swaption.NPV()-otmValue) > tolerance)
        BOOST_ERROR("failed to reproduce cached out-of-the-money "
                    << "swaption value:\n"
                    << "calculated: " << swaption.NPV() << "\n"
                    << "expected:   " << otmValue);
}


test_suite* BermudanSwaptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Bermudan swaption tests");
    suite->add(BOOST_TEST_CASE(&BermudanSwaptionTest::testCachedValues));
    return suite;
}

