/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2004, 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2009 Chris Kenyon

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
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/inflationcouponpricer.hpp>
#include <ql/cashflows/yoyinflationcoupon.hpp>
#include <ql/indexes/inflation/euhicp.hpp>
#include <ql/indexes/inflation/ukrpi.hpp>
#include <ql/instruments/inflationcapfloor.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/math/matrix.hpp>
#include <ql/models/marketmodels/correlations/expcorrelations.hpp>
#include <ql/models/marketmodels/models/flatvol.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/inflation/inflationcapfloorengines.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/termstructures/inflation/piecewiseyoyinflationcurve.hpp>
#include <ql/termstructures/volatility/inflation/yoyinflationoptionletvolatilitystructure.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/dataformatters.hpp>


using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::fabs;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(InflationCapFlooredCouponTests)

struct Datum {
    Date date;
    Rate rate;
};

template <class T, class U, class I>
std::vector<ext::shared_ptr<BootstrapHelper<T> > > makeHelpers(
                                                               const std::vector<Datum>& iiData,
                                                               const ext::shared_ptr<I> &ii, const Period &observationLag,
                                                               const Calendar &calendar,
                                                               const BusinessDayConvention &bdc,
                                                               const DayCounter &dc,
                                                               const Handle<YieldTermStructure>& discountCurve) {

    std::vector<ext::shared_ptr<BootstrapHelper<T> > > instruments;
    for (Datum datum : iiData) {
        Date maturity = datum.date;
        Handle<Quote> quote(ext::shared_ptr<Quote>(
                            new SimpleQuote(datum.rate/100.0)));
        ext::shared_ptr<BootstrapHelper<T> > anInstrument(new U(
                            quote, observationLag, maturity,
                            calendar, bdc, dc, ii, discountCurve));
        instruments.push_back(anInstrument);
    }

    return instruments;
}


struct CommonVars {
    // common data

    Size length;
    Date startDate;
    Real volatility;

    Frequency frequency;
    std::vector<Real> nominals;
    Calendar calendar;
    BusinessDayConvention convention;
    Natural fixingDays;
    Date evaluationDate;
    Natural settlementDays;
    Date settlement;
    Period observationLag;
    DayCounter dc;
    ext::shared_ptr<YoYInflationIndex> iir;

    RelinkableHandle<YieldTermStructure> nominalTS;
    ext::shared_ptr<YoYInflationTermStructure> yoyTS;
    RelinkableHandle<YoYInflationTermStructure> hy;

    // setup
    CommonVars()
    : nominals(1,1000000) {
        // option variables
        frequency = Annual;
        // usual setup
        volatility = 0.01;
        length = 7;
        calendar = UnitedKingdom();
        convention = ModifiedFollowing;
        Date today(13, August, 2007);
        evaluationDate = calendar.adjust(today);
        Settings::instance().evaluationDate() = evaluationDate;
        settlementDays = 0;
        fixingDays = 0;
        settlement = calendar.advance(today,settlementDays,Days);
        startDate = settlement;
        dc = Thirty360(Thirty360::BondBasis);

        // yoy index
        //      fixing data
        Date from(1, January, 2005);
        Date to(13, August, 2007);
        Schedule rpiSchedule = MakeSchedule().from(from).to(to)
            .withTenor(1*Months)
            .withCalendar(UnitedKingdom())
            .withConvention(ModifiedFollowing);
        Real fixData[] = { 189.9, 189.9, 189.6, 190.5, 191.6, 192.0,
                           192.2, 192.2, 192.6, 193.1, 193.3, 193.6,
                           194.1, 193.4, 194.2, 195.0, 196.5, 197.7,
                           198.5, 198.5, 199.2, 200.1, 200.4, 201.1,
                           202.7, 201.6, 203.1, 204.4, 205.4, 206.2,
                           207.3, -999.0, -999 };
        auto rpi = ext::make_shared<UKRPI>();
        for (Size i=0; i<rpiSchedule.size();i++) {
            rpi->addFixing(rpiSchedule[i], fixData[i]);
        }
        // link from yoy index to yoy TS
        bool interp = false;
        iir = ext::make_shared<YoYInflationIndex>(rpi, interp, hy);

        ext::shared_ptr<YieldTermStructure> nominalFF(
                        new FlatForward(evaluationDate, 0.05, ActualActual(ActualActual::ISDA)));
        nominalTS.linkTo(nominalFF);

        // now build the YoY inflation curve
        Period observationLag = Period(2,Months);

        std::vector<Datum> yyData = {
            { Date(13, August, 2008), 2.95 },
            { Date(13, August, 2009), 2.95 },
            { Date(13, August, 2010), 2.93 },
            { Date(15, August, 2011), 2.955 },
            { Date(13, August, 2012), 2.945 },
            { Date(13, August, 2013), 2.985 },
            { Date(13, August, 2014), 3.01 },
            { Date(13, August, 2015), 3.035 },
            { Date(13, August, 2016), 3.055 },  // note that
            { Date(13, August, 2017), 3.075 },  // some dates will be on
            { Date(13, August, 2019), 3.105 },  // holidays but the payment
            { Date(15, August, 2022), 3.135 },  // calendar will roll them
            { Date(13, August, 2027), 3.155 },
            { Date(13, August, 2032), 3.145 },
            { Date(13, August, 2037), 3.145 }
        };

            // now build the helpers ...
        std::vector<ext::shared_ptr<BootstrapHelper<YoYInflationTermStructure> > > helpers =
            makeHelpers<YoYInflationTermStructure,YearOnYearInflationSwapHelper,
            YoYInflationIndex>(yyData, iir,
                               observationLag,
                               calendar, convention, dc,
                               Handle<YieldTermStructure>(nominalTS));

        Rate baseYYRate = yyData[0].rate/100.0;
        ext::shared_ptr<PiecewiseYoYInflationCurve<Linear> > pYYTS(
                            new PiecewiseYoYInflationCurve<Linear>(
                                evaluationDate, calendar, dc, observationLag,
                                iir->frequency(),iir->interpolated(), baseYYRate,
                                helpers));
        pYYTS->recalculate();
        yoyTS = ext::dynamic_pointer_cast<YoYInflationTermStructure>(pYYTS);


        // make sure that the index has the latest yoy term structure
        hy.linkTo(pYYTS);
    }

    // utilities
    Leg makeYoYLeg(const Date& startDate,
                   Integer length,
                   const Rate gearing = 1.0,
                   const Rate spread = 0.0) const {
        ext::shared_ptr<YoYInflationIndex> ii =
            ext::dynamic_pointer_cast<YoYInflationIndex>(iir);
        Date endDate = calendar.advance(startDate,length*Years,Unadjusted);
        Schedule schedule(startDate, endDate, Period(frequency), calendar,
                          Unadjusted,Unadjusted,// ref periods & acc periods
                          DateGeneration::Forward, false);

        std::vector<Rate> gearingVector(length, gearing);
        std::vector<Spread> spreadVector(length, spread);

        Leg yoyLeg = yoyInflationLeg(schedule, calendar, ii, observationLag)
            .withNotionals(nominals)
            .withPaymentDayCounter(dc)
            .withGearings(gearingVector)
            .withSpreads(spreadVector)
            .withPaymentAdjustment(convention);

        return yoyLeg;
    }

    Leg makeFixedLeg(const Date& startDate, Integer length) const {

        Date endDate = calendar.advance(startDate, length, Years,
                                        convention);
        Schedule schedule(startDate, endDate, Period(frequency), calendar,
                          convention, convention,
                          DateGeneration::Forward, false);
        std::vector<Rate> coupons(length, 0.0);
        return FixedRateLeg(schedule)
            .withNotionals(nominals)
            .withCouponRates(coupons, dc);
    }


    Leg makeYoYCapFlooredLeg(Size which,
                             const Date& startDate,
                             Integer length,
                             const std::vector<Rate>& caps,
                             const std::vector<Rate>& floors,
                             Volatility volatility,
                             const Rate gearing = 1.0,
                             const Rate spread = 0.0) const {

        Handle<YoYOptionletVolatilitySurface>
            vol(ext::make_shared<ConstantYoYOptionletVolatility>(
                            volatility,
                                settlementDays,
                                calendar,
                                convention,
                                dc,
                                observationLag,
                                frequency,
                                iir->interpolated()));

        ext::shared_ptr<YoYInflationCouponPricer> pricer;
        switch (which) {
          case 0:
            pricer = ext::shared_ptr<YoYInflationCouponPricer>(
                            new BlackYoYInflationCouponPricer(vol, nominalTS));
            break;
          case 1:
            pricer = ext::shared_ptr<YoYInflationCouponPricer>(
                            new UnitDisplacedBlackYoYInflationCouponPricer(vol, nominalTS));
            break;
          case 2:
            pricer = ext::shared_ptr<YoYInflationCouponPricer>(
                            new BachelierYoYInflationCouponPricer(vol, nominalTS));
            break;
          default:
            BOOST_FAIL("unknown coupon pricer request: which = "<<which
                       <<"should be 0=Black,1=DD,2=Bachelier");
            break;
        }


        std::vector<Rate> gearingVector(length, gearing);
        std::vector<Spread> spreadVector(length, spread);

        ext::shared_ptr<YoYInflationIndex> ii =
            ext::dynamic_pointer_cast<YoYInflationIndex>(iir);
        Date endDate = calendar.advance(startDate,length*Years,Unadjusted);
        Schedule schedule(startDate, endDate, Period(frequency), calendar,
                          Unadjusted,Unadjusted,// ref periods & acc periods
                          DateGeneration::Forward, false);

        Leg yoyLeg =  yoyInflationLeg(schedule, calendar, ii, observationLag)
            .withNotionals(nominals)
            .withPaymentDayCounter(dc)
            .withPaymentAdjustment(convention)
            .withGearings(gearingVector)
            .withSpreads(spreadVector)
            .withCaps(caps)
            .withFloors(floors);

        setCouponPricer(yoyLeg, pricer);

        return yoyLeg;
    }


    ext::shared_ptr<PricingEngine> makeEngine(Volatility volatility, Size which) const {

        ext::shared_ptr<YoYInflationIndex>
            yyii = ext::dynamic_pointer_cast<YoYInflationIndex>(iir);

        Handle<YoYOptionletVolatilitySurface>
            vol(ext::make_shared<ConstantYoYOptionletVolatility>(
                            volatility,
                            settlementDays,
                            calendar,
                            convention,
                            dc,
                            observationLag,
                            frequency,
                            iir->interpolated()));


        switch (which) {
          case 0:
            return ext::shared_ptr<PricingEngine>(
                            new YoYInflationBlackCapFloorEngine(iir, vol, nominalTS));
            break;
          case 1:
            return ext::shared_ptr<PricingEngine>(
                            new YoYInflationUnitDisplacedBlackCapFloorEngine(iir, vol, nominalTS));
            break;
          case 2:
            return ext::shared_ptr<PricingEngine>(
                            new YoYInflationBachelierCapFloorEngine(iir, vol, nominalTS));
            break;
          default:
            BOOST_FAIL("unknown engine request: which = "<<which
                       <<"should be 0=Black,1=DD,2=Bachelier");
            break;
        }
        // make compiler happy
        QL_FAIL("never get here - no engine resolution");
    }


    ext::shared_ptr<YoYInflationCapFloor> makeYoYCapFloor(YoYInflationCapFloor::Type type,
                                                          const Leg& leg,
                                                          Rate strike,
                                                          Volatility volatility,
                                                          Size which) const {
        ext::shared_ptr<YoYInflationCapFloor> result;
        switch (type) {
          case YoYInflationCapFloor::Cap:
            result = ext::shared_ptr<YoYInflationCapFloor>(
                                new YoYInflationCap(leg, std::vector<Rate>(1, strike)));
            break;
          case YoYInflationCapFloor::Floor:
            result = ext::shared_ptr<YoYInflationCapFloor>(
                                new YoYInflationFloor(leg, std::vector<Rate>(1, strike)));
            break;
          default:
            QL_FAIL("unknown YoYInflation cap/floor type");
        }
        result->setPricingEngine(makeEngine(volatility, which));
        return result;
    }
};


BOOST_AUTO_TEST_CASE(testDecomposition) {

    BOOST_TEST_MESSAGE("Testing collared coupon against its decomposition...");

    CommonVars vars;

    Real tolerance = 1e-10;
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
    vars.makeYoYLeg(vars.startDate,vars.length);
    // floating leg with positive gearing (gearing_p) and spread<>0
    Leg floatLeg_p =
    vars.makeYoYLeg(vars.startDate,vars.length,gearing_p,spread_p);
    // floating leg with negative gearing (gearing_n) and spread<>0
    Leg floatLeg_n =
    vars.makeYoYLeg(vars.startDate,vars.length,gearing_n,spread_n);
    // Swap with null fixed leg and floating leg with gearing=1 and spread=0
    Swap vanillaLeg(fixedLeg,floatLeg);
    // Swap with null fixed leg and floating leg with positive gearing and spread<>0
    Swap vanillaLeg_p(fixedLeg,floatLeg_p);
    // Swap with null fixed leg and floating leg with negative gearing and spread<>0
    Swap vanillaLeg_n(fixedLeg,floatLeg_n);

    ext::shared_ptr<PricingEngine> engine(
            new DiscountingSwapEngine(vars.nominalTS));

    vanillaLeg.setPricingEngine(engine);    // here use the autoset feature
    vanillaLeg_p.setPricingEngine(engine);
    vanillaLeg_n.setPricingEngine(engine);

    // CAPPED coupon - Decomposition of payoff
    // Payoff = Nom * Min(rate,strike) * accrualperiod =
    // = Nom * [rate + Min(0,strike-rate)] * accrualperiod =
    // = Nom * rate * accrualperiod - Nom * Max(rate-strike,0) * accrualperiod =
    // = VanillaFloatingLeg - Call
    //

    Size whichPricer = 0;

    // Case gearing = 1 and spread = 0
    Leg cappedLeg =
    vars.makeYoYCapFlooredLeg(whichPricer,vars.startDate,vars.length,
                           caps,floors0,vars.volatility);
    Swap capLeg(fixedLeg,cappedLeg);
    capLeg.setPricingEngine(engine);
    YoYInflationCap cap(floatLeg, std::vector<Rate>(1, capstrike));
    cap.setPricingEngine(vars.makeEngine(vars.volatility,whichPricer));
    npvVanilla = vanillaLeg.NPV();
    npvCappedLeg = capLeg.NPV();
    npvCap = cap.NPV();
    error = std::abs(npvCappedLeg - (npvVanilla-npvCap));
    if (error>tolerance) {
        BOOST_ERROR("\nYoY Capped Leg: gearing=1, spread=0%, strike=" << capstrike*100 <<
                    "%\n" <<
                    "  Capped Floating Leg NPV: " << npvCappedLeg << "\n" <<
                    "  Floating Leg NPV - Cap NPV: " << npvVanilla - npvCap << "\n" <<
                    "  Diff: " << error );
    }

    // gearing = 1 and spread = 0
    // FLOORED coupon - Decomposition of payoff
    // Payoff = Nom * Max(rate,strike) * accrualperiod =
    // = Nom * [rate + Max(0,strike-rate)] * accrualperiod =
    // = Nom * rate * accrualperiod + Nom * Max(strike-rate,0) * accrualperiod =
    // = VanillaFloatingLeg + Put
    //

    Leg flooredLeg =
    vars.makeYoYCapFlooredLeg(whichPricer,vars.startDate,vars.length,
                           caps0,floors,vars.volatility);
    Swap floorLeg(fixedLeg,flooredLeg);
    floorLeg.setPricingEngine(engine);
    YoYInflationFloor floor(floatLeg, std::vector<Rate>(1, floorstrike));
    floor.setPricingEngine(vars.makeEngine(vars.volatility,whichPricer));
    npvFlooredLeg = floorLeg.NPV();
    npvFloor = floor.NPV();
    error = std::abs(npvFlooredLeg-(npvVanilla + npvFloor));
    if (error>tolerance) {
        BOOST_ERROR("YoY Floored Leg: gearing=1, spread=0%, strike=" << floorstrike *100 <<
                    "%\n" <<
                    "  Floored Floating Leg NPV: " << npvFlooredLeg << "\n" <<
                    "  Floating Leg NPV + Floor NPV: " << npvVanilla + npvFloor << "\n" <<
                    "  Diff: " << error );
    }

    // gearing = 1 and spread = 0
    // COLLARED coupon - Decomposition of payoff
    // Payoff = Nom * Min(strikem,Max(rate,strikeM)) * accrualperiod =
    // = VanillaFloatingLeg - Collar
    //

    Leg collaredLeg =
    vars.makeYoYCapFlooredLeg(whichPricer,vars.startDate,vars.length,
                           caps,floors,vars.volatility);
    Swap collarLeg(fixedLeg,collaredLeg);
    collarLeg.setPricingEngine(engine);
    YoYInflationCollar collar(floatLeg,
                  std::vector<Rate>(1, capstrike),
                  std::vector<Rate>(1, floorstrike));
    collar.setPricingEngine(vars.makeEngine(vars.volatility,whichPricer));
    npvCollaredLeg = collarLeg.NPV();
    npvCollar = collar.NPV();
    error = std::abs(npvCollaredLeg -(npvVanilla - npvCollar));
    if (error>tolerance) {
        BOOST_ERROR("\nYoY Collared Leg: gearing=1, spread=0%, strike=" <<
                    floorstrike*100 << "% and " << capstrike*100 << "%\n" <<
                    "  Collared Floating Leg NPV: " << npvCollaredLeg << "\n" <<
                    "  Floating Leg NPV - Collar NPV: " << npvVanilla - npvCollar << "\n" <<
                    "  Diff: " << error );
    }

    // gearing = a and spread = b
    // CAPPED coupon - Decomposition of payoff
    // Payoff
    // = Nom * Min(a*rate+b,strike) * accrualperiod =
    // = Nom * [a*rate+b + Min(0,strike-a*rate-b)] * accrualperiod =
    // = Nom * a*rate+b * accrualperiod + Nom * Min(strike-b-a*rate,0) * accrualperiod
    // --> If a>0 (assuming positive effective strike):
    // Payoff = VanillaFloatingLeg - Call(a*rate+b,strike)
    // --> If a<0 (assuming positive effective strike):
    // Payoff = VanillaFloatingLeg + Nom * Min(strike-b+|a|*rate+,0) * accrualperiod =
    // = VanillaFloatingLeg + Put(|a|*rate+b,strike)
    //

    // Positive gearing
    Leg cappedLeg_p =
    vars.makeYoYCapFlooredLeg(whichPricer,vars.startDate,vars.length,caps,floors0,
                           vars.volatility,gearing_p,spread_p);
    Swap capLeg_p(fixedLeg,cappedLeg_p);
    capLeg_p.setPricingEngine(engine);
    YoYInflationCap cap_p(floatLeg_p,std::vector<Rate>(1,capstrike));
    cap_p.setPricingEngine(vars.makeEngine(vars.volatility,whichPricer));
    npvVanilla = vanillaLeg_p.NPV();
    npvCappedLeg = capLeg_p.NPV();
    npvCap = cap_p.NPV();
    error = std::abs(npvCappedLeg - (npvVanilla-npvCap));
    if (error>tolerance) {
        BOOST_ERROR("\nYoY Capped Leg: gearing=" << gearing_p << ", " <<
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
    vars.makeYoYCapFlooredLeg(whichPricer,vars.startDate,vars.length,caps,floors0,
                           vars.volatility,gearing_n,spread_n);
    Swap capLeg_n(fixedLeg,cappedLeg_n);
    capLeg_n.setPricingEngine(engine);
    YoYInflationFloor floor_n(floatLeg,std::vector<Rate>(1,(capstrike-spread_n)/gearing_n));
    floor_n.setPricingEngine(vars.makeEngine(vars.volatility,whichPricer));
    npvVanilla = vanillaLeg_n.NPV();
    npvCappedLeg = capLeg_n.NPV();
    npvFloor = floor_n.NPV();
    error = std::abs(npvCappedLeg - (npvVanilla+ gearing_n*npvFloor));
    if (error>tolerance) {
        BOOST_ERROR("\nYoY Capped Leg: gearing=" << gearing_n << ", " <<
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

    // gearing = a and spread = b
    // FLOORED coupon - Decomposition of payoff
    // Payoff
    // = Nom * Max(a*rate+b,strike) * accrualperiod =
    // = Nom * [a*rate+b + Max(0,strike-a*rate-b)] * accrualperiod =
    // = Nom * a*rate+b * accrualperiod + Nom * Max(strike-b-a*rate,0) * accrualperiod
    // --> If a>0 (assuming positive effective strike):
    // Payoff = VanillaFloatingLeg + Put(a*rate+b,strike)
    // --> If a<0 (assuming positive effective strike):
    // Payoff = VanillaFloatingLeg + Nom * Max(strike-b+|a|*rate+,0) * accrualperiod =
    // = VanillaFloatingLeg - Call(|a|*rate+b,strike)
    //

    // Positive gearing
    Leg flooredLeg_p1 =
    vars.makeYoYCapFlooredLeg(whichPricer,vars.startDate,vars.length,caps0,floors,
                           vars.volatility,gearing_p,spread_p);
    Swap floorLeg_p1(fixedLeg,flooredLeg_p1);
    floorLeg_p1.setPricingEngine(engine);
    YoYInflationFloor floor_p1(floatLeg_p,std::vector<Rate>(1,floorstrike));
    floor_p1.setPricingEngine(vars.makeEngine(vars.volatility,whichPricer));
    npvVanilla = vanillaLeg_p.NPV();
    npvFlooredLeg = floorLeg_p1.NPV();
    npvFloor = floor_p1.NPV();
    error = std::abs(npvFlooredLeg - (npvVanilla+npvFloor));
    if (error>tolerance) {
        BOOST_ERROR("\nYoY Floored Leg: gearing=" << gearing_p << ", "
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
    vars.makeYoYCapFlooredLeg(whichPricer,vars.startDate,vars.length,caps0,floors,
                           vars.volatility,gearing_n,spread_n);
    Swap floorLeg_n(fixedLeg,flooredLeg_n);
    floorLeg_n.setPricingEngine(engine);
    YoYInflationCap cap_n(floatLeg,std::vector<Rate>(1,(floorstrike-spread_n)/gearing_n));
    cap_n.setPricingEngine(vars.makeEngine(vars.volatility,whichPricer));
    npvVanilla = vanillaLeg_n.NPV();
    npvFlooredLeg = floorLeg_n.NPV();
    npvCap = cap_n.NPV();
    error = std::abs(npvFlooredLeg - (npvVanilla - gearing_n*npvCap));
    if (error>tolerance) {
        BOOST_ERROR("\nYoY Capped Leg: gearing=" << gearing_n << ", " <<
                    "spread= " << spread_n *100 <<
                    "%, strike=" << floorstrike*100  << "%, " <<
                    "effective strike= " << (floorstrike-spread_n)/gearing_n*100 <<
                    "%\n" <<
                    "  Capped Floating Leg NPV: " << npvFlooredLeg << "\n" <<
                    "  Floating Leg NPV - Cap NPV: " << npvVanilla - gearing_n*npvCap << "\n" <<
                    "  Diff: " << error );
    }
    // gearing = a and spread = b
    // COLLARED coupon - Decomposition of payoff
    // Payoff = Nom * Min(caprate,Max(a*rate+b,floorrate)) * accrualperiod
    // --> If a>0 (assuming positive effective strike):
    // Payoff = VanillaFloatingLeg - Collar(a*rate+b, floorrate, caprate)
    // --> If a<0 (assuming positive effective strike):
    // Payoff = VanillaFloatingLeg + Collar(|a|*rate+b, caprate, floorrate)
    //
    // Positive gearing
    Leg collaredLeg_p =
    vars.makeYoYCapFlooredLeg(whichPricer,vars.startDate,vars.length,caps,floors,
                           vars.volatility,gearing_p,spread_p);
    Swap collarLeg_p1(fixedLeg,collaredLeg_p);
    collarLeg_p1.setPricingEngine(engine);
    YoYInflationCollar collar_p(floatLeg_p,
                    std::vector<Rate>(1,capstrike),
                    std::vector<Rate>(1,floorstrike));
    collar_p.setPricingEngine(vars.makeEngine(vars.volatility,whichPricer));
    npvVanilla = vanillaLeg_p.NPV();
    npvCollaredLeg = collarLeg_p1.NPV();
    npvCollar = collar_p.NPV();
    error = std::abs(npvCollaredLeg - (npvVanilla - npvCollar));
    if (error>tolerance) {
        BOOST_ERROR("\nYoY Collared Leg: gearing=" << gearing_p << ", "
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
    vars.makeYoYCapFlooredLeg(whichPricer,vars.startDate,vars.length,caps,floors,
                           vars.volatility,gearing_n,spread_n);
    Swap collarLeg_n1(fixedLeg,collaredLeg_n);
    collarLeg_n1.setPricingEngine(engine);
    YoYInflationCollar collar_n(floatLeg,
                    std::vector<Rate>(1,(floorstrike-spread_n)/gearing_n),
                    std::vector<Rate>(1,(capstrike-spread_n)/gearing_n));
    collar_n.setPricingEngine(vars.makeEngine(vars.volatility,whichPricer));
    npvVanilla = vanillaLeg_n.NPV();
    npvCollaredLeg = collarLeg_n1.NPV();
    npvCollar = collar_n.NPV();
    error = std::abs(npvCollaredLeg - (npvVanilla - gearing_n*npvCollar));
    if (error>tolerance) {
        BOOST_ERROR("\nYoY Collared Leg: gearing=" << gearing_n << ", "
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
    // remove circular refernce
    vars.hy.linkTo(ext::shared_ptr<YoYInflationTermStructure>());
}

BOOST_AUTO_TEST_CASE(testInstrumentEquality) {

    BOOST_TEST_MESSAGE("Testing inflation capped/floored coupon against"
                       " inflation capfloor instrument...");

    CommonVars vars;

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    // vol is low ...
    Rate strikes[] = { 0.01, 0.025, 0.029, 0.03, 0.031, 0.035, 0.07 };
    // yoy inflation vol is generally very low
    Volatility vols[] = { 0.001, 0.005, 0.010, 0.015, 0.020 };

    // this is model independent
    // capped coupon = fwd - cap, and fwd = swap(0)
    // floored coupon = fwd + floor
    for (Size whichPricer = 0; whichPricer < 3; whichPricer++) {
        for (int& length : lengths) {
            for (Real& strike : strikes) {
                for (Real vol : vols) {

                    Leg leg = vars.makeYoYLeg(vars.evaluationDate, length);

                    ext::shared_ptr<Instrument> cap = vars.makeYoYCapFloor(
                        YoYInflationCapFloor::Cap, leg, strike, vol, whichPricer);

                    ext::shared_ptr<Instrument> floor = vars.makeYoYCapFloor(
                        YoYInflationCapFloor::Floor, leg, strike, vol, whichPricer);

                    Date from = vars.nominalTS->referenceDate();
                    Date to = from + length * Years;
                    Schedule yoySchedule = MakeSchedule().from(from).to(to)
                    .withTenor(1*Years)
                    .withCalendar(UnitedKingdom())
                    .withConvention(Unadjusted)
                    .backwards()
                    ;

                    YearOnYearInflationSwap swap(Swap::Payer,
                                                 1000000.0,
                                                 yoySchedule,//fixed schedule, but same as yoy
                                                 0.0,//strikes[j],
                                                 vars.dc,
                                                 yoySchedule,
                                                 vars.iir,
                                                 vars.observationLag,
                                                 0.0,        //spread on index
                                                 vars.dc,
                                                 UnitedKingdom());

                    Handle<YieldTermStructure> hTS(vars.nominalTS);
                    ext::shared_ptr<PricingEngine> sppe(new DiscountingSwapEngine(hTS));
                    swap.setPricingEngine(sppe);

                    Leg leg2 = vars.makeYoYCapFlooredLeg(whichPricer, from, length,
                                                         std::vector<Rate>(length, strike), // cap
                                                         std::vector<Rate>(),               // floor
                                                         vol,
                                                         1.0,  // gearing
                                                         0.0); // spread

                    Leg leg3 = vars.makeYoYCapFlooredLeg(whichPricer, from, length,
                                                         std::vector<Rate>(),               // cap
                                                         std::vector<Rate>(length, strike), // floor
                                                         vol,
                                                         1.0,  // gearing
                                                         0.0); // spread

                    // N.B. nominals are 10e6
                    Real capped = CashFlows::npv(leg2,(**vars.nominalTS),false);
                    if ( fabs(capped - (swap.NPV() - cap->NPV())) > 1.0e-6) {
                        BOOST_FAIL("capped coupon != swap(0) - cap:\n"
                                   << "    length:      " << length << " years\n"
                                   << "    volatility:  " << io::volatility(vol) << "\n"
                                   << "    strike:      " << io::rate(strike) << "\n"
                                   << "    cap value:   " << cap->NPV() << "\n"
                                   << "    swap value:  " << swap.NPV() << "\n"
                                   << "   capped coupon " << capped);
                    }


                    // N.B. nominals are 10e6
                    Real floored = CashFlows::npv(leg3,(**vars.nominalTS),false);
                    if ( fabs(floored - (swap.NPV() + floor->NPV())) > 1.0e-6) {
                        BOOST_FAIL("floored coupon != swap(0) + floor :\n"
                                   << "    length:      " << length << " years\n"
                                   << "    volatility:  " << io::volatility(vol) << "\n"
                                   << "    strike:      " << io::rate(strike) << "\n"
                                   << "    floor value: " << floor->NPV() << "\n"
                                   << "    swap value:  " << swap.NPV() << "\n"
                                   << "  floored coupon " << floored);
                    }
                }
            }
        }
    }
    // remove circular refernce
    vars.hy.linkTo(ext::shared_ptr<YoYInflationTermStructure>());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
