/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2008 StatPro Italia srl

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
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/math/matrix.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace capfloored_coupon_test {

    struct CommonVars {
        // global data
        Date today, settlement, startDate;
        Calendar calendar;
        Real nominal;
        std::vector<Real> nominals;
        BusinessDayConvention convention;
        Frequency frequency;
        ext::shared_ptr<IborIndex> index;
        Natural settlementDays, fixingDays;
        RelinkableHandle<YieldTermStructure> termStructure;
        std::vector<Rate> caps;
        std::vector<Rate> floors;
        Integer length;
        Volatility volatility;

        // setup
        CommonVars() {
            length = 20;           //years
            volatility = 0.20;
            nominal = 100.;
            nominals = std::vector<Real>(length,nominal);
            frequency = Annual;
            index = ext::shared_ptr<IborIndex>(new Euribor1Y(termStructure));
            calendar = index->fixingCalendar();
            convention = ModifiedFollowing;
            today = calendar.adjust(Date::todaysDate());
            Settings::instance().evaluationDate() = today;
            settlementDays = 2;
            fixingDays = 2;
            settlement = calendar.advance(today,settlementDays,Days);
            startDate = settlement;
            termStructure.linkTo(flatRate(settlement,0.05,
                                          ActualActual(ActualActual::ISDA)));
        }

        // utilities
        Leg makeFixedLeg(const Date& startDate, Integer length) const {

            Date endDate = calendar.advance(startDate, length, Years,
                                            convention);
            Schedule schedule(startDate, endDate, Period(frequency), calendar,
                              convention, convention,
                              DateGeneration::Forward, false);
            std::vector<Rate> coupons(length, 0.0);
            return FixedRateLeg(schedule)
                .withNotionals(nominals)
                .withCouponRates(coupons, Thirty360(Thirty360::BondBasis));
        }

        Leg makeFloatingLeg(const Date& startDate,
                            Integer length,
                            const Rate gearing = 1.0,
                            const Rate spread = 0.0) const {

            Date endDate = calendar.advance(startDate,length,Years,convention);
            Schedule schedule(startDate,endDate,Period(frequency),calendar,
                              convention,convention,
                              DateGeneration::Forward,false);
            std::vector<Real> gearingVector(length, gearing);
            std::vector<Spread> spreadVector(length, spread);
            return IborLeg(schedule, index)
                .withNotionals(nominals)
                .withPaymentDayCounter(index->dayCounter())
                .withPaymentAdjustment(convention)
                .withFixingDays(fixingDays)
                .withGearings(gearingVector)
                .withSpreads(spreadVector);
        }

        Leg makeCapFlooredLeg(const Date& startDate,
                              Integer length,
                              const std::vector<Rate>& caps,
                              const std::vector<Rate>& floors,
                              Volatility volatility,
                              const Rate gearing = 1.0,
                              const Rate spread = 0.0) const {

            Date endDate = calendar.advance(startDate,length,Years,convention);
            Schedule schedule(startDate,endDate,Period(frequency),calendar,
                              convention,convention,
                              DateGeneration::Forward,false);
            Handle<OptionletVolatilityStructure> vol(
                ext::shared_ptr<OptionletVolatilityStructure>(new
                    ConstantOptionletVolatility(0, calendar, Following,
                                                volatility,Actual365Fixed())));

            ext::shared_ptr<IborCouponPricer> pricer(new
                BlackIborCouponPricer(vol));
            std::vector<Rate> gearingVector(length, gearing);
            std::vector<Spread> spreadVector(length, spread);

            Leg iborLeg = IborLeg(schedule, index)
                .withNotionals(nominals)
                .withPaymentDayCounter(index->dayCounter())
                .withPaymentAdjustment(convention)
                .withFixingDays(fixingDays)
                .withGearings(gearingVector)
                .withSpreads(spreadVector)
                .withCaps(caps)
                .withFloors(floors);
            setCouponPricer(iborLeg, pricer);
            return iborLeg;
        }

        ext::shared_ptr<PricingEngine> makeEngine(Volatility volatility) const {
            Handle<Quote> vol(ext::shared_ptr<Quote>(
                                                new SimpleQuote(volatility)));
            return ext::shared_ptr<PricingEngine>(
                                 new BlackCapFloorEngine(termStructure, vol));
        }

        ext::shared_ptr<CapFloor> makeCapFloor(CapFloor::Type type,
                                               const Leg& leg,
                                               Rate capStrike,
                                               Rate floorStrike,
                                               Volatility volatility) const {
            ext::shared_ptr<CapFloor> result;
            switch (type) {
              case CapFloor::Cap:
                result = ext::shared_ptr<CapFloor>(
                               new Cap(leg, std::vector<Rate>(1, capStrike)));
                break;
              case CapFloor::Floor:
                result = ext::shared_ptr<CapFloor>(
                           new Floor(leg, std::vector<Rate>(1, floorStrike)));
                break;
              case CapFloor::Collar:
                result = ext::shared_ptr<CapFloor>(
                               new Collar(leg,
                                          std::vector<Rate>(1, capStrike),
                                          std::vector<Rate>(1, floorStrike)));
                break;
              default:
                QL_FAIL("unknown cap/floor type");
            }
            result->setPricingEngine(makeEngine(volatility));
            return result;
        }
    };

}


void CapFlooredCouponTest::testLargeRates() {

    BOOST_TEST_MESSAGE("Testing degenerate collared coupon...");

    using namespace capfloored_coupon_test;

    CommonVars vars;

    /* A vanilla floating leg and a capped floating leg with strike
       equal to 100 and floor equal to 0 must have (about) the same NPV
       (depending on variance: option expiry and volatility)
    */

    std::vector<Rate> caps(vars.length,100.0);
    std::vector<Rate> floors(vars.length,0.0);
    Real tolerance = 1e-10;

    // fixed leg with zero rate
    Leg fixedLeg =
        vars.makeFixedLeg(vars.startDate,vars.length);
    Leg floatLeg =
        vars.makeFloatingLeg(vars.startDate,vars.length);
    Leg collaredLeg =
        vars.makeCapFlooredLeg(vars.startDate,vars.length,
                               caps,floors,vars.volatility);

    ext::shared_ptr<PricingEngine> engine(
                               new DiscountingSwapEngine(vars.termStructure));
    Swap vanillaLeg(fixedLeg,floatLeg);
    Swap collarLeg(fixedLeg,collaredLeg);
    vanillaLeg.setPricingEngine(engine);
    collarLeg.setPricingEngine(engine);

    if (std::abs(vanillaLeg.NPV()-collarLeg.NPV())>tolerance) {
        BOOST_ERROR("Length: " << vars.length << " y" << "\n" <<
                    "Volatility: " << vars.volatility*100 << "%\n" <<
                    "Notional: " << vars.nominal << "\n" <<
                    "Vanilla floating leg NPV: " << vanillaLeg.NPV()
                    << "\n" <<
                    "Collared floating leg NPV (strikes 0 and 100): "
                    << collarLeg.NPV()
                    << "\n" <<
                    "Diff: " << std::abs(vanillaLeg.NPV()-collarLeg.NPV()));
   }
}

void CapFlooredCouponTest::testDecomposition() {

    BOOST_TEST_MESSAGE("Testing collared coupon against its decomposition...");

    using namespace capfloored_coupon_test;

    CommonVars vars;

    Real tolerance = 1e-12;
    Real npvVanilla,npvCappedLeg,npvFlooredLeg,npvCollaredLeg,npvCap,npvFloor,npvCollar;
    Real error;
    Rate floorstrike = 0.05;
    Rate capstrike = 0.10;
    std::vector<Rate> caps(vars.length,capstrike);
    std::vector<Rate> caps0 = std::vector<Rate>();
    std::vector<Rate> floors(vars.length,floorstrike);
    std::vector<Rate> floors0 = std::vector<Rate>();
    Rate gearing_p = Rate(0.5);
    auto spread_p = Spread(0.002);
    Rate gearing_n = Rate(-1.5);
    auto spread_n = Spread(0.12);
    // fixed leg with zero rate
    Leg fixedLeg  =
        vars.makeFixedLeg(vars.startDate,vars.length);
    // floating leg with gearing=1 and spread=0
    Leg floatLeg  =
        vars.makeFloatingLeg(vars.startDate,vars.length);
    // floating leg with positive gearing (gearing_p) and spread<>0
    Leg floatLeg_p =
        vars.makeFloatingLeg(vars.startDate,vars.length,gearing_p,spread_p);
    // floating leg with negative gearing (gearing_n) and spread<>0
    Leg floatLeg_n =
        vars.makeFloatingLeg(vars.startDate,vars.length,gearing_n,spread_n);
    // Swap with null fixed leg and floating leg with gearing=1 and spread=0
    Swap vanillaLeg(fixedLeg,floatLeg);
    // Swap with null fixed leg and floating leg with positive gearing and spread<>0
    Swap vanillaLeg_p(fixedLeg,floatLeg_p);
    // Swap with null fixed leg and floating leg with negative gearing and spread<>0
    Swap vanillaLeg_n(fixedLeg,floatLeg_n);

    ext::shared_ptr<PricingEngine> engine(
                               new DiscountingSwapEngine(vars.termStructure));
    vanillaLeg.setPricingEngine(engine);
    vanillaLeg_p.setPricingEngine(engine);
    vanillaLeg_n.setPricingEngine(engine);

    /* CAPPED coupon - Decomposition of payoff
       Payoff = Nom * Min(rate,strike) * accrualperiod =
              = Nom * [rate + Min(0,strike-rate)] * accrualperiod =
              = Nom * rate * accrualperiod - Nom * Max(rate-strike,0) * accrualperiod =
              = VanillaFloatingLeg - Call
    */

    // Case gearing = 1 and spread = 0
    Leg cappedLeg =
        vars.makeCapFlooredLeg(vars.startDate,vars.length,
                               caps,floors0,vars.volatility);
    Swap capLeg(fixedLeg,cappedLeg);
    capLeg.setPricingEngine(engine);
    Cap cap(floatLeg, std::vector<Rate>(1, capstrike));
    cap.setPricingEngine(vars.makeEngine(vars.volatility));
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
        vars.makeCapFlooredLeg(vars.startDate,vars.length,
                               caps0,floors,vars.volatility);
    Swap floorLeg(fixedLeg,flooredLeg);
    floorLeg.setPricingEngine(engine);
    Floor floor(floatLeg, std::vector<Rate>(1, floorstrike));
    floor.setPricingEngine(vars.makeEngine(vars.volatility));
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
        vars.makeCapFlooredLeg(vars.startDate,vars.length,
                               caps,floors,vars.volatility);
    Swap collarLeg(fixedLeg,collaredLeg);
    collarLeg.setPricingEngine(engine);
    Collar collar(floatLeg,
                  std::vector<Rate>(1, capstrike),
                  std::vector<Rate>(1, floorstrike));
    collar.setPricingEngine(vars.makeEngine(vars.volatility));
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
        vars.makeCapFlooredLeg(vars.startDate,vars.length,caps,floors0,
                               vars.volatility,gearing_p,spread_p);
    Swap capLeg_p(fixedLeg,cappedLeg_p);
    capLeg_p.setPricingEngine(engine);
    Cap cap_p(floatLeg_p,std::vector<Rate>(1,capstrike));
    cap_p.setPricingEngine(vars.makeEngine(vars.volatility));
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
        vars.makeCapFlooredLeg(vars.startDate,vars.length,caps,floors0,
                               vars.volatility,gearing_n,spread_n);
    Swap capLeg_n(fixedLeg,cappedLeg_n);
    capLeg_n.setPricingEngine(engine);
    Floor floor_n(floatLeg,std::vector<Rate>(1,(capstrike-spread_n)/gearing_n));
    floor_n.setPricingEngine(vars.makeEngine(vars.volatility));
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
        vars.makeCapFlooredLeg(vars.startDate,vars.length,caps0,floors,
                               vars.volatility,gearing_p,spread_p);
    Swap floorLeg_p1(fixedLeg,flooredLeg_p1);
    floorLeg_p1.setPricingEngine(engine);
    Floor floor_p1(floatLeg_p,std::vector<Rate>(1,floorstrike));
    floor_p1.setPricingEngine(vars.makeEngine(vars.volatility));
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
        vars.makeCapFlooredLeg(vars.startDate,vars.length,caps0,floors,
                               vars.volatility,gearing_n,spread_n);
    Swap floorLeg_n(fixedLeg,flooredLeg_n);
    floorLeg_n.setPricingEngine(engine);
    Cap cap_n(floatLeg,std::vector<Rate>(1,(floorstrike-spread_n)/gearing_n));
    cap_n.setPricingEngine(vars.makeEngine(vars.volatility));
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
        vars.makeCapFlooredLeg(vars.startDate,vars.length,caps,floors,
                               vars.volatility,gearing_p,spread_p);
    Swap collarLeg_p1(fixedLeg,collaredLeg_p);
    collarLeg_p1.setPricingEngine(engine);
    Collar collar_p(floatLeg_p,
                    std::vector<Rate>(1,capstrike),
                    std::vector<Rate>(1,floorstrike));
    collar_p.setPricingEngine(vars.makeEngine(vars.volatility));
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
        vars.makeCapFlooredLeg(vars.startDate,vars.length,caps,floors,
                               vars.volatility,gearing_n,spread_n);
    Swap collarLeg_n1(fixedLeg,collaredLeg_n);
    collarLeg_n1.setPricingEngine(engine);
    Collar collar_n(floatLeg,
                    std::vector<Rate>(1,(floorstrike-spread_n)/gearing_n),
                    std::vector<Rate>(1,(capstrike-spread_n)/gearing_n));
    collar_n.setPricingEngine(vars.makeEngine(vars.volatility));
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
    auto* suite = BOOST_TEST_SUITE("Capped and floored coupon tests");
    suite->add(QUANTLIB_TEST_CASE(&CapFlooredCouponTest::testLargeRates));
    suite->add(QUANTLIB_TEST_CASE(&CapFlooredCouponTest::testDecomposition));
    return suite;
}

