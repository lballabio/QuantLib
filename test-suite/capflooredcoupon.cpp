/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Cristina Duminuco

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#include "capflooredcoupon.hpp"
#include "utilities.hpp"
#include <ql/Instruments/capfloor.hpp>
#include <ql/Instruments/vanillaswap.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/PricingEngines/CapFloor/blackcapfloorengine.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Volatilities/capletconstantvol.hpp>
#ifdef QL_USE_INDEXED_COUPON
#include <ql/CashFlows/indexedcashflowvectors.hpp>
#include <ql/CashFlows/upfrontindexedcoupon.hpp>
#endif
#include <ql/DayCounters/thirty360.hpp>
#include <ql/DayCounters/actualactual.hpp>
#include <ql/Utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(CapFlooredCouponTest)

// global data

Date today_, settlement_, startDate_;
Calendar calendar_;
Real nominal_;
std::vector<Real> nominals_;
BusinessDayConvention convention_;
Frequency frequency_;
boost::shared_ptr<IborIndex> index_;
Integer settlementDays_, fixingDays_;
Handle<YieldTermStructure> termStructure_;
std::vector<Rate> caps_;
std::vector<Rate> floors_;
Integer lenght_;
Volatility volatility_;

void setup() {
    lenght_ = 25;           //years
    volatility_ = 0.20;
    nominal_ = 100.;
    nominals_ = std::vector<Real>(lenght_,nominal_);
    frequency_ = Annual;
    index_ = boost::shared_ptr<IborIndex>(new Euribor1Y(termStructure_));
    calendar_ = index_->calendar();
    convention_ = ModifiedFollowing;
    today_ = calendar_.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today_;
    settlementDays_ = 2;
    fixingDays_ = 2;
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    startDate_ = settlement_;
    termStructure_.linkTo(flatRate(settlement_,0.05,
                                   ActualActual(ActualActual::ISDA)));
}

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

// utilities

std::vector<boost::shared_ptr<CashFlow> > makeFixedLeg(const Date& startDate,
                                                       Integer length) {

    Date endDate = calendar_.advance(startDate, length, Years, convention_);
    Schedule schedule(startDate, endDate, Period(frequency_), calendar_,
                      convention_, convention_, false, false);
    std::vector<Rate> coupons(length, 0.0);
    return FixedRateCouponVector(schedule,Following,nominals_,coupons, Thirty360());
}

std::vector<boost::shared_ptr<CashFlow> > makeFloatingLeg(const Date& startDate,
                                                          Integer length) {

    Date endDate = calendar_.advance(startDate,length,Years,convention_);
    Schedule schedule(startDate,endDate,Period(frequency_),calendar_,
                      convention_,convention_,false,false);

    return FloatingRateCouponVector(schedule,convention_,nominals_,fixingDays_,index_,
                                    std::vector<Real>(),std::vector<Spread>(),
                                    index_->dayCounter());
}

std::vector<boost::shared_ptr<CashFlow> > makeCapFlooredLeg(const Date& startDate,
                                                     Integer length,
                                                     const std::vector<Rate> caps,
                                                     const std::vector<Rate> floors,
                                                     Volatility volatility) {

    Date endDate = calendar_.advance(startDate,length,Years,convention_);
    Schedule schedule(startDate,endDate,Period(frequency_),calendar_,
                      convention_,convention_,false,false);

    Handle<CapletVolatilityStructure> vol;

    vol = Handle<CapletVolatilityStructure>(
          boost::shared_ptr<CapletVolatilityStructure>(new
            CapletConstantVolatility(volatility, Actual365Fixed())));

    return CappedFlooredFloatingRateCouponVector(schedule,convention_,nominals_,
                                           fixingDays_,index_,
                                           std::vector<Real>(),std::vector<Spread>(),
                                           caps,floors,index_->dayCounter(),vol);
}

boost::shared_ptr<PricingEngine> makeEngine(Volatility volatility) {
    Handle<Quote> vol(boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
    return boost::shared_ptr<PricingEngine>(new BlackCapFloorEngine(vol));
}

boost::shared_ptr<CapFloor> makeCapFloor(
                         CapFloor::Type type,
                         const std::vector<boost::shared_ptr<CashFlow> >& leg,
                         Rate capstrike, Rate floorstrike,
                         Volatility volatility) {
    switch (type) {
      case CapFloor::Cap:
        return boost::shared_ptr<CapFloor>(
               new Cap(leg, std::vector<Rate>(1, capstrike),
                       termStructure_, makeEngine(volatility)));
      case CapFloor::Floor:
            return boost::shared_ptr<CapFloor>(
                new Floor(leg, std::vector<Rate>(1, floorstrike),
                          termStructure_, makeEngine(volatility)));
      case CapFloor::Collar:
            return boost::shared_ptr<CapFloor>(
                new Collar(leg, std::vector<Rate>(1, capstrike),
                           std::vector<Rate>(1, floorstrike),
                          termStructure_, makeEngine(volatility)));
      default:
        QL_FAIL("unknown cap/floor type");
    }
}

std::string typeToString(CapFloor::Type type) {
    switch (type) {
      case CapFloor::Cap:
        return "cap";
      case CapFloor::Floor:
        return "floor";
      case CapFloor::Collar:
        return "collar";
      default:
        QL_FAIL("unknown cap/floor type");
    }
}

QL_END_TEST_LOCALS(CapFlooredCouponTest)

void CapFlooredCouponTest::testLargeRates() {

    BOOST_MESSAGE("Testing collared coupon with cap rate 100 and floor rate 0 ...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    /* A vanilla floating leg and a capped floating leg with strike
       equal to 100 and floor equal to 0 must have (about) the same NPV
       (depending on variance: option expiry and volatility)
    */

    std::vector<Rate> caps(lenght_,100.0);
    std::vector<Rate> floors(lenght_,0.0);
    Real tolerance = 1e-10;

    // fixed leg with zero rate
    std::vector<boost::shared_ptr<CashFlow> > fixedLeg =
        makeFixedLeg(startDate_,lenght_);
    std::vector<boost::shared_ptr<CashFlow> > floatLeg =
        makeFloatingLeg(startDate_,lenght_);
    std::vector<boost::shared_ptr<CashFlow> > collaredLeg =
        makeCapFlooredLeg(startDate_,lenght_,caps,floors,volatility_);

    Swap vanillaLeg(termStructure_,fixedLeg,floatLeg);
    Swap collarLeg( termStructure_,fixedLeg,collaredLeg);

    if (std::abs(vanillaLeg.NPV()-collarLeg.NPV())>tolerance) {
        BOOST_MESSAGE("Lenght: " << lenght_ << " y" << "\n" <<
            "Volatility: " << volatility_*100 << "%\n" <<
            "Notional: " << nominal_ << "\n" <<
            "Vanilla floating leg NPV: " << vanillaLeg.NPV()
            << "\n" <<
            "Collared floating leg NPV (strikes 0 and 100): " << collarLeg.NPV()
            << "\n" <<
            "Diff: " << std::abs(vanillaLeg.NPV()-collarLeg.NPV()));
   }
    QL_TEST_TEARDOWN
}

void CapFlooredCouponTest::testDecomposition() {

    BOOST_MESSAGE("Testing collared coupon against its decomposition ...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Real tolerance = 1e-10;

    /* CAPPED coupon - Decomposition of payoff
       Payoff = Nom * Min(rate,strike) * accrualperiod =
              = Nom * [rate + Min(0,strike-rate)] * accrualperiod =
              = Nom * rate * accrualperiod - Nom * Max(rate-strike,0) * accrualperiod =
              = VanillaFloatingLeg - Call
    */

    Rate floorstrike = 0.0;
    Rate capstrike = 0.05;
    std::vector<Rate> caps(lenght_,capstrike);
    std::vector<Rate> floors(lenght_,floorstrike);
    // fixed leg with zero rate
    std::vector<boost::shared_ptr<CashFlow> > fixedLeg  =
        makeFixedLeg(startDate_,lenght_);
    std::vector<boost::shared_ptr<CashFlow> > cappedLeg =
        makeCapFlooredLeg(startDate_,lenght_,caps,floors,volatility_);
    std::vector<boost::shared_ptr<CashFlow> > floatLeg  =
        makeFloatingLeg(startDate_,lenght_);

    Swap vanillaLeg(termStructure_,fixedLeg,floatLeg);
    Swap capLeg(termStructure_,fixedLeg,cappedLeg);
    Cap cap(floatLeg, std::vector<Rate>(1, capstrike), termStructure_, makeEngine(volatility_));

    Real npvVanilla = vanillaLeg.NPV();
    Real npvCappedLeg = capLeg.NPV();
    Real npvCap = cap.NPV();
    Real error = std::abs(npvCappedLeg - (npvVanilla-npvCap));
    if (error>tolerance) {
        BOOST_MESSAGE("Capped Floating Leg NPV: " << npvCappedLeg << "\n" <<
                      "Vanilla Floating Leg NPV - Cap NPV: " <<
                      npvVanilla - npvCap << "\n" <<
                      "Diff: " << error );
    }

   /* FLOORED coupon - Decomposition of payoff
       Payoff = Nom * Max(rate,strike) * accrualperiod =
              = Nom * [rate + Max(0,strike-rate)] * accrualperiod =
              = Nom * rate * accrualperiod + Nom * Max(strike-rate,0) * accrualperiod =
              = VanillaFloatingLeg + Put
    */

    Rate floorstrike1 = 0.025;
    Rate capstrike1 = 100;
    std::vector<Rate> caps1(lenght_,capstrike1);
    std::vector<Rate> floors1(lenght_,floorstrike1);

    std::vector<boost::shared_ptr<CashFlow> > flooredLeg =
        makeCapFlooredLeg(startDate_,lenght_,caps1,floors1,volatility_);

    Swap floorLeg(termStructure_,fixedLeg,flooredLeg);
    Floor floor(floatLeg, std::vector<Rate>(1, floorstrike1), termStructure_, makeEngine(volatility_));

    Real npvFlooredLeg = floorLeg.NPV();
    Real npvFloor = floor.NPV();
    error = std::abs(npvFlooredLeg-(npvVanilla + npvFloor));
    if (error>tolerance) {
        BOOST_ERROR("Floored Floating Leg NPV: " << npvFlooredLeg << "\n" <<
                    "Vanilla Floating Leg NPV + floor NPV: " <<
                    npvVanilla + npvFloor << "\n" <<
                    "Diff: " << error );
    }

    /* COLLARED coupon - Decomposition of payoff
       Payoff = Nom * Min(strikem,Max(rate,strikeM)) * accrualperiod =
              = VanillaFloatingLeg - Collar
    */

    Rate floorstrike2 = 0.025;
    Rate capstrike2 = 0.05;
    std::vector<Rate> caps2(lenght_,capstrike2);
    std::vector<Rate> floors2(lenght_,floorstrike2);

    std::vector<boost::shared_ptr<CashFlow> > collaredLeg =
        makeCapFlooredLeg(startDate_,lenght_,caps2,floors2,volatility_);

    Swap collarLeg(termStructure_,fixedLeg,collaredLeg);
    boost::shared_ptr<CapFloor> collar =
        makeCapFloor(CapFloor::Collar,floatLeg,capstrike2,floorstrike2,volatility_);

    Real npvCollaredLeg = collarLeg.NPV();
    Real npvCollar = collar->NPV();
    error = std::abs(npvCollaredLeg -(npvVanilla - npvCollar));
    if (error>tolerance) {
        BOOST_ERROR("Collared Floating Leg NPV: " << npvCollar << "\n" <<
                    "Vanilla Floating Leg NPV - collar NPV: " <<
                    npvVanilla - npvCollar << "\n" <<
                    "Diff: " << error );
    }
    QL_TEST_TEARDOWN
}

test_suite* CapFlooredCouponTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Capped/floored coupon tests");
    suite->add(BOOST_TEST_CASE(&CapFlooredCouponTest::testLargeRates));
    suite->add(BOOST_TEST_CASE(&CapFlooredCouponTest::testDecomposition));
    return suite;
}

