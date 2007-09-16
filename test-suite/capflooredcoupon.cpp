/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Cristina Duminuco

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

#include "capflooredcoupon.hpp"
#include "utilities.hpp"
#include <ql/instruments/capfloor.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/termstructures/yieldcurves/flatforward.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/math/matrix.hpp>
#include <ql/termstructures/volatilities/interestrate/caplet/capletconstantvol.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>

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
Natural settlementDays_, fixingDays_;
RelinkableHandle<YieldTermStructure> termStructure_;
std::vector<Rate> caps_;
std::vector<Rate> floors_;
Integer length_;
Volatility volatility_;

void setup() {
    length_ = 20;           //years
    volatility_ = 0.20;
    nominal_ = 100.;
    nominals_ = std::vector<Real>(length_,nominal_);
    frequency_ = Annual;
    index_ = boost::shared_ptr<IborIndex>(new Euribor1Y(termStructure_));
    calendar_ = index_->fixingCalendar();
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

// utilities

Leg makeFixedLeg(const Date& startDate,
                                                       Integer length) {

    Date endDate = calendar_.advance(startDate, length, Years, convention_);
    Schedule schedule(startDate, endDate, Period(frequency_), calendar_,
                      convention_, convention_, false, false);
    std::vector<Rate> coupons(length, 0.0);
    return FixedRateLeg(nominals_, schedule, coupons, Thirty360(), Following);
}

Leg makeFloatingLeg(const Date& startDate,
                                                          Integer length,
                                                          const Rate gearing = 1.0,
                                                          const Rate spread = 0.0) {

    Date endDate = calendar_.advance(startDate,length,Years,convention_);
    Schedule schedule(startDate,endDate,Period(frequency_),calendar_,
                      convention_,convention_,false,false);
    std::vector<Real> gearingVector(length_, gearing);
    std::vector<Spread> spreadVector(length_, spread);
    Leg floatLeg = IborLeg(nominals_, schedule, index_, index_->dayCounter(),
                           convention_, std::vector<Natural>(1,fixingDays_),
                           gearingVector, spreadVector);
    boost::shared_ptr<IborCouponPricer> fictitiousPricer(new
        BlackIborCouponPricer(Handle<OptionletVolatilityStructure>()));
    setCouponPricer(floatLeg,fictitiousPricer);
    return floatLeg;
}

Leg makeCapFlooredLeg(const Date& startDate,
                      Integer length,
                      const std::vector<Rate> caps,
                      const std::vector<Rate> floors,
                      Volatility volatility,
                      const Rate gearing = 1.0,
                      const Rate spread = 0.0) {

    Date endDate = calendar_.advance(startDate,length,Years,convention_);
    Schedule schedule(startDate,endDate,Period(frequency_),calendar_,
                      convention_,convention_,false,false);
    Handle<OptionletVolatilityStructure> vol;
    vol = Handle<OptionletVolatilityStructure>(
          boost::shared_ptr<OptionletVolatilityStructure>(new
            CapletConstantVolatility(volatility, Actual365Fixed())));

    boost::shared_ptr<IborCouponPricer> pricer(new BlackIborCouponPricer(vol));
    std::vector<Rate> gearingVector(length_, gearing);
    std::vector<Spread> spreadVector(length_, spread);

    Leg iborLeg = IborLeg(nominals_,
                          schedule,
                          index_,
                          index_->dayCounter(),
                          convention_,
                          std::vector<Natural>(1,fixingDays_),
                          gearingVector,
                          spreadVector,
                          caps,
                          floors,
                          false);
     setCouponPricer(iborLeg, pricer);
     return iborLeg;
}

boost::shared_ptr<PricingEngine> makeEngine(Volatility volatility) {
    Handle<Quote> vol(boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
    return boost::shared_ptr<PricingEngine>(new BlackCapFloorEngine(vol));
}

boost::shared_ptr<CapFloor> makeCapFloor(CapFloor::Type type,
                         const Leg& leg,
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

    BOOST_MESSAGE("Testing degenerate collared coupon...");

    SavedSettings backup;

    setup();

    /* A vanilla floating leg and a capped floating leg with strike
       equal to 100 and floor equal to 0 must have (about) the same NPV
       (depending on variance: option expiry and volatility)
    */

    std::vector<Rate> caps(length_,100.0);
    std::vector<Rate> floors(length_,0.0);
    Real tolerance = 1e-10;

    // fixed leg with zero rate
    Leg fixedLeg =
        makeFixedLeg(startDate_,length_);
    Leg floatLeg =
        makeFloatingLeg(startDate_,length_);
    Leg collaredLeg =
        makeCapFlooredLeg(startDate_,length_,caps,floors,volatility_);

    Swap vanillaLeg(termStructure_,fixedLeg,floatLeg);
    Swap collarLeg( termStructure_,fixedLeg,collaredLeg);

    if (std::abs(vanillaLeg.NPV()-collarLeg.NPV())>tolerance) {
        BOOST_MESSAGE("Lenght: " << length_ << " y" << "\n" <<
            "Volatility: " << volatility_*100 << "%\n" <<
            "Notional: " << nominal_ << "\n" <<
            "Vanilla floating leg NPV: " << vanillaLeg.NPV()
            << "\n" <<
            "Collared floating leg NPV (strikes 0 and 100): " << collarLeg.NPV()
            << "\n" <<
            "Diff: " << std::abs(vanillaLeg.NPV()-collarLeg.NPV()));
   }
}

void CapFlooredCouponTest::testDecomposition() {

    BOOST_MESSAGE("Testing collared coupon against its decomposition...");

    SavedSettings backup;

    setup();

    Real tolerance = 1e-12;
    Real npvVanilla,npvCappedLeg,npvFlooredLeg,npvCollaredLeg,npvCap,npvFloor,npvCollar;
    Real error;
    Rate floorstrike = 0.05;
    Rate capstrike = 0.10;
    std::vector<Rate> caps(length_,capstrike);
    std::vector<Rate> caps0 = std::vector<Rate>();
    std::vector<Rate> floors(length_,floorstrike);
    std::vector<Rate> floors0 = std::vector<Rate>();
    Rate gearing_p = Rate(0.5);
    Spread spread_p = Spread(0.002);
    Rate gearing_n = Rate(-1.5);
    Spread spread_n = Spread(0.12);
    // fixed leg with zero rate
    Leg fixedLeg  =
        makeFixedLeg(startDate_,length_);
    // floating leg with gearing=1 and spread=0
    Leg floatLeg  =
        makeFloatingLeg(startDate_,length_);
    // floating leg with positive gearing (gearing_p) and spread<>0
    Leg floatLeg_p =
        makeFloatingLeg(startDate_,length_,gearing_p,spread_p);
    // floating leg with negative gearing (gearing_n) and spread<>0
    Leg floatLeg_n =
        makeFloatingLeg(startDate_,length_,gearing_n,spread_n);
    // Swap with null fixed leg and floating leg with gearing=1 and spread=0
    Swap vanillaLeg(termStructure_,fixedLeg,floatLeg);
    // Swap with null fixed leg and floating leg with positive gearing and spread<>0
    Swap vanillaLeg_p(termStructure_,fixedLeg,floatLeg_p);
    // Swap with null fixed leg and floating leg with negative gearing and spread<>0
    Swap vanillaLeg_n(termStructure_,fixedLeg,floatLeg_n);

    /* CAPPED coupon - Decomposition of payoff
       Payoff = Nom * Min(rate,strike) * accrualperiod =
              = Nom * [rate + Min(0,strike-rate)] * accrualperiod =
              = Nom * rate * accrualperiod - Nom * Max(rate-strike,0) * accrualperiod =
              = VanillaFloatingLeg - Call
    */

    // Case gearing = 1 and spread = 0
    Leg cappedLeg =
        makeCapFlooredLeg(startDate_,length_,caps,floors0,volatility_);
    Swap capLeg(termStructure_,fixedLeg,cappedLeg);
    Cap cap(floatLeg, std::vector<Rate>(1, capstrike),
            termStructure_, makeEngine(volatility_));
    npvVanilla = vanillaLeg.NPV();
    npvCappedLeg = capLeg.NPV();
    npvCap = cap.NPV();
    error = std::abs(npvCappedLeg - (npvVanilla-npvCap));
    if (error>tolerance) {
        BOOST_ERROR("\nCapped Leg: gearing=1, spread=0%, strike=" << capstrike*100 <<
                    "%\n" <<
                    "  Capped Floating Leg NPV: " << npvCappedLeg << "\n" <<
                    "  Floating Leg NPV - Cap NPV: " << npvVanilla - npvCap << "\n" <<
                    "  Diff: " << error );
    }

    /* gearing = 1 and spread = 0
       FLOORED coupon - Decomposition of payoff
       Payoff = Nom * Max(rate,strike) * accrualperiod =
              = Nom * [rate + Max(0,strike-rate)] * accrualperiod =
              = Nom * rate * accrualperiod + Nom * Max(strike-rate,0) * accrualperiod =
              = VanillaFloatingLeg + Put
    */

    Leg flooredLeg =
        makeCapFlooredLeg(startDate_,length_,caps0,floors,volatility_);
    Swap floorLeg(termStructure_,fixedLeg,flooredLeg);
    Floor floor(floatLeg, std::vector<Rate>(1, floorstrike),
                termStructure_, makeEngine(volatility_));
    npvFlooredLeg = floorLeg.NPV();
    npvFloor = floor.NPV();
    error = std::abs(npvFlooredLeg-(npvVanilla + npvFloor));
    if (error>tolerance) {
        BOOST_ERROR("Floored Leg: gearing=1, spread=0%, strike=" << floorstrike *100 <<
                    "%\n" <<
                    "  Floored Floating Leg NPV: " << npvFlooredLeg << "\n" <<
                    "  Floating Leg NPV + Floor NPV: " << npvVanilla + npvFloor << "\n" <<
                    "  Diff: " << error );
    }

    /* gearing = 1 and spread = 0
       COLLARED coupon - Decomposition of payoff
       Payoff = Nom * Min(strikem,Max(rate,strikeM)) * accrualperiod =
              = VanillaFloatingLeg - Collar
    */

    Leg collaredLeg =
        makeCapFlooredLeg(startDate_,length_,caps,floors,volatility_);
    Swap collarLeg(termStructure_,fixedLeg,collaredLeg);
    Collar collar(floatLeg,
                  std::vector<Rate>(1, capstrike),
                  std::vector<Rate>(1, floorstrike),
                  termStructure_, makeEngine(volatility_));
    npvCollaredLeg = collarLeg.NPV();
    npvCollar = collar.NPV();
    error = std::abs(npvCollaredLeg -(npvVanilla - npvCollar));
    if (error>tolerance) {
        BOOST_ERROR("\nCollared Leg: gearing=1, spread=0%, strike=" <<
                    floorstrike*100 << "% and " << capstrike*100 << "%\n" <<
                    "  Collared Floating Leg NPV: " << npvCollaredLeg << "\n" <<
                    "  Floating Leg NPV - Collar NPV: " << npvVanilla - npvCollar << "\n" <<
                    "  Diff: " << error );
    }

    /* gearing = a and spread = b
       CAPPED coupon - Decomposition of payoff
       Payoff
       = Nom * Min(a*rate+b,strike) * accrualperiod =
       = Nom * [a*rate+b + Min(0,strike-a*rate-b)] * accrualperiod =
       = Nom * a*rate+b * accrualperiod + Nom * Min(strike-b-a*rate,0) * accrualperiod
       --> If a>0 (assuming positive effective strike):
           Payoff = VanillaFloatingLeg - Call(a*rate+b,strike)
       --> If a<0 (assuming positive effective strike):
           Payoff = VanillaFloatingLeg + Nom * Min(strike-b+|a|*rate+,0) * accrualperiod =
                  = VanillaFloatingLeg + Put(|a|*rate+b,strike)
    */

    // Positive gearing
    Leg cappedLeg_p =
        makeCapFlooredLeg(startDate_,length_,caps,floors0,
                          volatility_,gearing_p,spread_p);
    Swap capLeg_p(termStructure_,fixedLeg,cappedLeg_p);
    Cap cap_p(floatLeg_p,std::vector<Rate>(1,capstrike),
             termStructure_, makeEngine(volatility_));
    npvVanilla = vanillaLeg_p.NPV();
    npvCappedLeg = capLeg_p.NPV();
    npvCap = cap_p.NPV();
    error = std::abs(npvCappedLeg - (npvVanilla-npvCap));
    if (error>tolerance) {
        BOOST_ERROR("\nCapped Leg: gearing=" << gearing_p << ", " <<
                    "spread= " << spread_p *100 <<
                    "%, strike=" << capstrike*100  << "%, " <<
                    "effective strike= " << (capstrike-spread_p)/gearing_p*100 <<
                     "%\n" <<
                     "  Capped Floating Leg NPV: " << npvCappedLeg << "\n" <<
                     "  Vanilla Leg NPV: " << npvVanilla << "\n" <<
                     "  Cap NPV: " << npvCap << "\n" <<
                     "  Floating Leg NPV - Cap NPV: " << npvVanilla - npvCap << "\n" <<
                     "  Diff: " << error );
    }

    // Negative gearing
    Leg cappedLeg_n =
        makeCapFlooredLeg(startDate_,length_,caps,floors0,
                          volatility_,gearing_n,spread_n);
    Swap capLeg_n(termStructure_,fixedLeg,cappedLeg_n);
    Floor floor_n(floatLeg,std::vector<Rate>(1,(capstrike-spread_n)/gearing_n),
             termStructure_, makeEngine(volatility_));
    npvVanilla = vanillaLeg_n.NPV();
    npvCappedLeg = capLeg_n.NPV();
    npvFloor = floor_n.NPV();
    error = std::abs(npvCappedLeg - (npvVanilla+ gearing_n*npvFloor));
    if (error>tolerance) {
        BOOST_ERROR("\nCapped Leg: gearing=" << gearing_n << ", " <<
                    "spread= " << spread_n *100 <<
                    "%, strike=" << capstrike*100  << "%, " <<
                    "effective strike= " << (capstrike-spread_n)/gearing_n*100 <<
                     "%\n" <<
                     "  Capped Floating Leg NPV: " << npvCappedLeg << "\n" <<
                     "  npv Vanilla: " << npvVanilla << "\n" <<
                     "  npvFloor: " << npvFloor << "\n" <<
                     "  Floating Leg NPV - Cap NPV: " << npvVanilla + gearing_n*npvFloor << "\n" <<
                     "  Diff: " << error );
    }

    /* gearing = a and spread = b
       FLOORED coupon - Decomposition of payoff
       Payoff
       = Nom * Max(a*rate+b,strike) * accrualperiod =
       = Nom * [a*rate+b + Max(0,strike-a*rate-b)] * accrualperiod =
       = Nom * a*rate+b * accrualperiod + Nom * Max(strike-b-a*rate,0) * accrualperiod
       --> If a>0 (assuming positive effective strike):
           Payoff = VanillaFloatingLeg + Put(a*rate+b,strike)
       --> If a<0 (assuming positive effective strike):
           Payoff = VanillaFloatingLeg + Nom * Max(strike-b+|a|*rate+,0) * accrualperiod =
                  = VanillaFloatingLeg - Call(|a|*rate+b,strike)
    */

    // Positive gearing
    Leg flooredLeg_p1 =
        makeCapFlooredLeg(startDate_,length_,caps0,floors,
                          volatility_,gearing_p,spread_p);
    Swap floorLeg_p1(termStructure_,fixedLeg,flooredLeg_p1);
    Floor floor_p1(floatLeg_p,std::vector<Rate>(1,floorstrike),
                termStructure_, makeEngine(volatility_));
    npvVanilla = vanillaLeg_p.NPV();
    npvFlooredLeg = floorLeg_p1.NPV();
    npvFloor = floor_p1.NPV();
    error = std::abs(npvFlooredLeg - (npvVanilla+npvFloor));
    if (error>tolerance) {
        BOOST_ERROR("\nFloored Leg: gearing=" << gearing_p << ", "
                      << "spread= " << spread_p *100<< "%, strike=" << floorstrike *100 << "%, "
                      << "effective strike= " << (floorstrike-spread_p)/gearing_p*100
                      << "%\n" <<
                      "  Floored Floating Leg NPV: "    << npvFlooredLeg
                      << "\n" <<
                      "  Floating Leg NPV + Floor NPV: " << npvVanilla + npvFloor
                      << "\n" <<
                      "  Diff: " << error );
    }
    // Negative gearing
    Leg flooredLeg_n =
        makeCapFlooredLeg(startDate_,length_,caps0,floors,
                          volatility_,gearing_n,spread_n);
    Swap floorLeg_n(termStructure_,fixedLeg,flooredLeg_n);
    Cap cap_n(floatLeg,std::vector<Rate>(1,(floorstrike-spread_n)/gearing_n),
             termStructure_, makeEngine(volatility_));
    npvVanilla = vanillaLeg_n.NPV();
    npvFlooredLeg = floorLeg_n.NPV();
    npvCap = cap_n.NPV();
    error = std::abs(npvFlooredLeg - (npvVanilla - gearing_n*npvCap));
    if (error>tolerance) {
        BOOST_ERROR("\nCapped Leg: gearing=" << gearing_n << ", " <<
                    "spread= " << spread_n *100 <<
                    "%, strike=" << floorstrike*100  << "%, " <<
                    "effective strike= " << (floorstrike-spread_n)/gearing_n*100 <<
                     "%\n" <<
                     "  Capped Floating Leg NPV: " << npvFlooredLeg << "\n" <<
                     "  Floating Leg NPV - Cap NPV: " << npvVanilla - gearing_n*npvCap << "\n" <<
                     "  Diff: " << error );
    }
    /* gearing = a and spread = b
       COLLARED coupon - Decomposition of payoff
       Payoff = Nom * Min(caprate,Max(a*rate+b,floorrate)) * accrualperiod
       --> If a>0 (assuming positive effective strike):
           Payoff = VanillaFloatingLeg - Collar(a*rate+b, floorrate, caprate)
       --> If a<0 (assuming positive effective strike):
           Payoff = VanillaFloatingLeg + Collar(|a|*rate+b, caprate, floorrate)
    */
    // Positive gearing
    Leg collaredLeg_p =
        makeCapFlooredLeg(startDate_,length_,caps,floors,
                          volatility_,gearing_p,spread_p);
    Swap collarLeg_p1(termStructure_,fixedLeg,collaredLeg_p);
    Collar collar_p(floatLeg_p,
                    std::vector<Rate>(1,capstrike),
                    std::vector<Rate>(1,floorstrike),
                    termStructure_, makeEngine(volatility_));
    npvVanilla = vanillaLeg_p.NPV();
    npvCollaredLeg = collarLeg_p1.NPV();
    npvCollar = collar_p.NPV();
    error = std::abs(npvCollaredLeg - (npvVanilla - npvCollar));
    if (error>tolerance) {
        BOOST_ERROR("\nCollared Leg: gearing=" << gearing_p << ", "
                      << "spread= " << spread_p*100 << "%, strike="
                      << floorstrike*100 << "% and " << capstrike*100
                      << "%, "
                      << "effective strike=" << (floorstrike-spread_p)/gearing_p*100
                      <<  "% and " << (capstrike-spread_p)/gearing_p*100
                      << "%\n" <<
                      "  Collared Floating Leg NPV: "    << npvCollaredLeg
                      << "\n" <<
                      "  Floating Leg NPV - Collar NPV: " << npvVanilla - npvCollar
                      << "\n" <<
                      "  Diff: " << error );
    }
    // Negative gearing
    Leg collaredLeg_n =
        makeCapFlooredLeg(startDate_,length_,caps,floors,
                          volatility_,gearing_n,spread_n);
    Swap collarLeg_n1(termStructure_,fixedLeg,collaredLeg_n);
    Collar collar_n(floatLeg,
                    std::vector<Rate>(1,(floorstrike-spread_n)/gearing_n),
                    std::vector<Rate>(1,(capstrike-spread_n)/gearing_n),
                    termStructure_, makeEngine(volatility_));
    npvVanilla = vanillaLeg_n.NPV();
    npvCollaredLeg = collarLeg_n1.NPV();
    npvCollar = collar_n.NPV();
    error = std::abs(npvCollaredLeg - (npvVanilla - gearing_n*npvCollar));
    if (error>tolerance) {
        BOOST_ERROR("\nCollared Leg: gearing=" << gearing_n << ", "
                      << "spread= " << spread_n*100 << "%, strike="
                      << floorstrike*100 << "% and " << capstrike*100
                      << "%, "
                      << "effective strike=" << (floorstrike-spread_n)/gearing_n*100
                      <<  "% and " << (capstrike-spread_n)/gearing_n*100
                      << "%\n" <<
                      "  Collared Floating Leg NPV: "    << npvCollaredLeg
                      << "\n" <<
                      "  Floating Leg NPV - Collar NPV: " << npvVanilla - gearing_n*npvCollar
                      << "\n" <<
                      "  Diff: " << error );
    }
}

test_suite* CapFlooredCouponTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Capped/floored coupon tests");
    suite->add(BOOST_TEST_CASE(&CapFlooredCouponTest::testLargeRates));
    suite->add(BOOST_TEST_CASE(&CapFlooredCouponTest::testDecomposition));
    return suite;
}

