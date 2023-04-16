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

#include "inflationcapfloor.hpp"
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

namespace inflation_capfloor_test {

    struct Datum {
        Date date;
        Rate rate;
    };

    template <class T, class U, class I>
    std::vector<std::shared_ptr<BootstrapHelper<T> > > makeHelpers(
                 const std::vector<Datum>& iiData,
                 const std::shared_ptr<I> &ii, const Period &observationLag,
                 const Calendar &calendar,
                 const BusinessDayConvention &bdc,
                 const DayCounter &dc,
                 const Handle<YieldTermStructure>& discountCurve) {

        std::vector<std::shared_ptr<BootstrapHelper<T> > > instruments;
        for (Datum datum : iiData) {
            Date maturity = datum.date;
            Handle<Quote> quote(std::shared_ptr<Quote>(
                    new SimpleQuote(datum.rate/100.0)));
            std::shared_ptr<BootstrapHelper<T> > anInstrument(new U(
                    quote, observationLag, maturity,
                    calendar, bdc, dc, ii, discountCurve));
            instruments.push_back(anInstrument);
        }

        return instruments;
    }


    struct CommonVars {
        // common data

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
        std::shared_ptr<YYUKRPIr> iir;

        RelinkableHandle<YieldTermStructure> nominalTS;
        std::shared_ptr<YoYInflationTermStructure> yoyTS;
        RelinkableHandle<YoYInflationTermStructure> hy;

        // cleanup

        SavedSettings backup;

        // setup
        CommonVars()
        : nominals(1,1000000) {
            // option variables
            frequency = Annual;
            // usual setup
            calendar = UnitedKingdom();
            convention = ModifiedFollowing;
            Date today(13, August, 2007);
            evaluationDate = calendar.adjust(today);
            Settings::instance().evaluationDate() = evaluationDate;
            settlementDays = 0;
            fixingDays = 0;
            settlement = calendar.advance(today,settlementDays,Days);
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
            // link from yoy index to yoy TS
            bool interp = false;
            iir = std::make_shared<YYUKRPIr>(interp, hy);
            for (Size i=0; i<rpiSchedule.size();i++) {
                iir->addFixing(rpiSchedule[i], fixData[i]);
            }

            std::shared_ptr<YieldTermStructure> nominalFF(
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
            std::vector<std::shared_ptr<BootstrapHelper<YoYInflationTermStructure> > > helpers =
            makeHelpers<YoYInflationTermStructure,YearOnYearInflationSwapHelper,
            YoYInflationIndex>(yyData, iir,
                               observationLag,
                               calendar, convention, dc,
                               Handle<YieldTermStructure>(nominalTS));

            Rate baseYYRate = yyData[0].rate/100.0;
            std::shared_ptr<PiecewiseYoYInflationCurve<Linear> > pYYTS(
                new PiecewiseYoYInflationCurve<Linear>(
                        evaluationDate, calendar, dc, observationLag,
                        iir->frequency(),iir->interpolated(), baseYYRate,
                        helpers));
            pYYTS->recalculate();
            yoyTS = std::dynamic_pointer_cast<YoYInflationTermStructure>(pYYTS);


            // make sure that the index has the latest yoy term structure
            hy.linkTo(pYYTS);
        }

        // utilities
        Leg makeYoYLeg(const Date& startDate, Integer length) const {
            std::shared_ptr<YoYInflationIndex> ii =
                std::dynamic_pointer_cast<YoYInflationIndex>(iir);
            Date endDate = calendar.advance(startDate,length*Years,Unadjusted);
            Schedule schedule(startDate, endDate, Period(frequency), calendar,
                              Unadjusted,Unadjusted,// ref periods & acc periods
                              DateGeneration::Forward, false);
            return yoyInflationLeg(schedule, calendar, ii, observationLag)
            .withNotionals(nominals)
            .withPaymentDayCounter(dc)
            .withPaymentAdjustment(convention);
        }


        std::shared_ptr<PricingEngine> makeEngine(Volatility volatility, Size which) const {

            std::shared_ptr<YoYInflationIndex>
            yyii = std::dynamic_pointer_cast<YoYInflationIndex>(iir);

            Handle<YoYOptionletVolatilitySurface>
                vol(std::make_shared<ConstantYoYOptionletVolatility>(
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
                    return std::shared_ptr<PricingEngine>(
                            new YoYInflationBlackCapFloorEngine(iir, vol, nominalTS));
                    break;
                case 1:
                    return std::shared_ptr<PricingEngine>(
                            new YoYInflationUnitDisplacedBlackCapFloorEngine(iir, vol, nominalTS));
                    break;
                case 2:
                    return std::shared_ptr<PricingEngine>(
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


        std::shared_ptr<YoYInflationCapFloor> makeYoYCapFloor(YoYInflationCapFloor::Type type,
                                                              const Leg& leg,
                                                              Rate strike,
                                                              Volatility volatility,
                                                              Size which) const {
            std::shared_ptr<YoYInflationCapFloor> result;
            switch (type) {
                case YoYInflationCapFloor::Cap:
                    result = std::shared_ptr<YoYInflationCapFloor>(
                        new YoYInflationCap(leg, std::vector<Rate>(1, strike)));
                    break;
                case YoYInflationCapFloor::Floor:
                    result = std::shared_ptr<YoYInflationCapFloor>(
                        new YoYInflationFloor(leg, std::vector<Rate>(1, strike)));
                    break;
                default:
                    QL_FAIL("unknown YoYInflation cap/floor type");
            }
            result->setPricingEngine(makeEngine(volatility, which));
            return result;
        }
    };

}



void InflationCapFloorTest::testConsistency() {

    BOOST_TEST_MESSAGE("Testing consistency between yoy inflation cap,"
                       " floor and collar...");

    using namespace inflation_capfloor_test;

    CommonVars vars;

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Rate cap_rates[] = { 0.01, 0.025, 0.029, 0.03, 0.031, 0.035, 0.07 };
    Rate floor_rates[] = { 0.01, 0.025, 0.029, 0.03, 0.031, 0.035, 0.07 };
    Volatility vols[] = { 0.001, 0.005, 0.010, 0.015, 0.020 };

    for (Size whichPricer = 0; whichPricer < 3; whichPricer++) {
        for (int& length : lengths) {
            for (Real& cap_rate : cap_rates) {
                for (Real& floor_rate : floor_rates) {
                    for (Real vol : vols) {

                        Leg leg = vars.makeYoYLeg(vars.evaluationDate, length);

                        std::shared_ptr<YoYInflationCapFloor> cap = vars.makeYoYCapFloor(
                            YoYInflationCapFloor::Cap, leg, cap_rate, vol, whichPricer);

                        std::shared_ptr<YoYInflationCapFloor> floor = vars.makeYoYCapFloor(
                            YoYInflationCapFloor::Floor, leg, floor_rate, vol, whichPricer);

                        YoYInflationCollar collar(leg, std::vector<Rate>(1, cap_rate),
                                                  std::vector<Rate>(1, floor_rate));
                        collar.setPricingEngine(vars.makeEngine(vol, whichPricer));

                        if (std::fabs((cap->NPV() - floor->NPV()) - collar.NPV()) > 1e-6) {
                            BOOST_FAIL("inconsistency between cap, floor and collar:\n"
                                       << "    length:       " << length << " years\n"
                                       << "    volatility:   " << io::volatility(vol) << "\n"
                                       << "    cap value:    " << cap->NPV()
                                       << " at strike: " << io::rate(cap_rate) << "\n"
                                       << "    floor value:  " << floor->NPV()
                                       << " at strike: " << io::rate(floor_rate) << "\n"
                                       << "    collar value: " << collar.NPV());


                            // test re-composition by optionlets, N.B. ONE per year
                            Real capletsNPV = 0.0;
                            std::vector<std::shared_ptr<YoYInflationCapFloor> > caplets;
                            for (Integer m = 0; m < length * 1; m++) {
                                caplets.push_back(cap->optionlet(m));
                                caplets[m]->setPricingEngine(vars.makeEngine(vol, whichPricer));
                                capletsNPV += caplets[m]->NPV();
                            }

                            if (std::fabs(cap->NPV() - capletsNPV) > 1e-6) {
                                BOOST_FAIL("sum of caplet NPVs does not equal cap NPV:\n"
                                           << "    length:       " << length << " years\n"
                                           << "    volatility:   " << io::volatility(vol) << "\n"
                                           << "    cap value:    " << cap->NPV()
                                           << " at strike: " << io::rate(cap_rate) << "\n"
                                           << "    sum of caplets value:  " << capletsNPV
                                           << " at strike (first): "
                                           << io::rate(caplets[0]->capRates()[0]) << "\n");
                            }

                            Real floorletsNPV = 0.0;
                            std::vector<std::shared_ptr<YoYInflationCapFloor> > floorlets;
                            for (Integer m = 0; m < length * 1; m++) {
                                floorlets.push_back(floor->optionlet(m));
                                floorlets[m]->setPricingEngine(vars.makeEngine(vol, whichPricer));
                                floorletsNPV += floorlets[m]->NPV();
                            }

                            if (std::fabs(floor->NPV() - floorletsNPV) > 1e-6) {
                                BOOST_FAIL("sum of floorlet NPVs does not equal floor NPV:\n"
                                           << "    length:       " << length << " years\n"
                                           << "    volatility:   " << io::volatility(vol) << "\n"
                                           << "    cap value:    " << floor->NPV()
                                           << " at strike: " << io::rate(floor_rate) << "\n"
                                           << "    sum of floorlets value:  " << floorletsNPV
                                           << " at strike (first): "
                                           << io::rate(floorlets[0]->floorRates()[0]) << "\n");
                            }

                            Real collarletsNPV = 0.0;
                            std::vector<std::shared_ptr<YoYInflationCapFloor> > collarlets;
                            for (Integer m = 0; m < length * 1; m++) {
                                collarlets.push_back(collar.optionlet(m));
                                collarlets[m]->setPricingEngine(vars.makeEngine(vol, whichPricer));
                                collarletsNPV += collarlets[m]->NPV();
                            }

                            if (std::fabs(collar.NPV() - collarletsNPV) > 1e-6) {
                                BOOST_FAIL("sum of collarlet NPVs does not equal collar NPV:\n"
                                           << "    length:       " << length << " years\n"
                                           << "    volatility:   " << io::volatility(vol) << "\n"
                                           << "    cap value:    " << collar.NPV()
                                           << " at strike floor: " << io::rate(floor_rate)
                                           << " at strike cap: " << io::rate(cap_rate) << "\n"
                                           << "    sum of collarlets value:  " << collarletsNPV
                                           << " at strike floor (first): "
                                           << io::rate(collarlets[0]->floorRates()[0])
                                           << " at strike cap (first): "
                                           << io::rate(collarlets[0]->capRates()[0]) << "\n");
                            }
                        }
                    }
                }
            }
        }
    } // pricer loop
    // remove circular refernce
    vars.hy.linkTo(std::shared_ptr<YoYInflationTermStructure>());
}


// Test inflation cap/floor parity, i.e. that cap-floor = swap, note that this
// is different from nominal because in nominal world standard cap/floors do
// not have the first optionlet.  This is because they set in advance so
// there is no point.  However, yoy inflation generally sets in arrears,
// (actually in arrears with a lag of a few months) thus the first optionlet
// is relevant.  Hence we can do a parity test without a special definition
// of the YoY cap/floor instrument.
void InflationCapFloorTest::testParity() {

    BOOST_TEST_MESSAGE("Testing yoy inflation cap/floor parity...");

    using namespace inflation_capfloor_test;

    CommonVars vars;

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    // vol is low ...
    Rate strikes[] = { 0., 0.025, 0.029, 0.03, 0.031, 0.035, 0.07 };
    // yoy inflation vol is generally very low
    Volatility vols[] = { 0.001, 0.005, 0.010, 0.015, 0.020 };

    // cap-floor-swap parity is model-independent
    for (Size whichPricer = 0; whichPricer < 3; whichPricer++) {
        for (int& length : lengths) {
            for (Real strike : strikes) {
                for (Real vol : vols) {

                    Leg leg = vars.makeYoYLeg(vars.evaluationDate, length);

                    std::shared_ptr<Instrument> cap = vars.makeYoYCapFloor(
                        YoYInflationCapFloor::Cap, leg, strike, vol, whichPricer);

                    std::shared_ptr<Instrument> floor = vars.makeYoYCapFloor(
                        YoYInflationCapFloor::Floor, leg, strike, vol, whichPricer);

                    Date from = vars.nominalTS->referenceDate();
                    Date to = from + length * Years;
                    Schedule yoySchedule = MakeSchedule().from(from).to(to)
                    .withTenor(1*Years)
                    .withCalendar(UnitedKingdom())
                    .withConvention(Unadjusted)
                    .backwards()
                    ;

                    YearOnYearInflationSwap swap(Swap::Payer, 1000000.0,
                                                 yoySchedule, // fixed schedule, but same as yoy
                                                 strike, vars.dc, yoySchedule, vars.iir,
                                                 vars.observationLag,
                                                 0.0, // spread on index
                                                 vars.dc, UnitedKingdom());

                    Handle<YieldTermStructure> hTS(vars.nominalTS);
                    std::shared_ptr<PricingEngine> sppe(new DiscountingSwapEngine(hTS));
                    swap.setPricingEngine(sppe);

                    // N.B. nominals are 10e6
                    if (std::fabs((cap->NPV()-floor->NPV()) - swap.NPV()) > 1.0e-6) {
                        BOOST_FAIL("put/call parity violated:\n"
                                   << "    length:      " << length << " years\n"
                                   << "    volatility:  " << io::volatility(vol) << "\n"
                                   << "    strike:      " << io::rate(strike) << "\n"
                                   << "    cap value:   " << cap->NPV() << "\n"
                                   << "    floor value: " << floor->NPV() << "\n"
                                   << "    swap value:  " << swap.NPV());
                    }
                }
            }
        }
    }
    // remove circular refernce
    vars.hy.linkTo(std::shared_ptr<YoYInflationTermStructure>());
}




void InflationCapFloorTest::testCachedValue() {

    BOOST_TEST_MESSAGE("Testing Black yoy inflation cap/floor price"
                       " against cached values...");

    using namespace inflation_capfloor_test;

    CommonVars vars;

    Size whichPricer = 0; // black

    Real K = 0.0295; // one centi-point is fair rate error i.e. < 1 cp
    Size j = 2;
    Leg leg = vars.makeYoYLeg(vars.evaluationDate,j);
    std::shared_ptr<Instrument> cap
        = vars.makeYoYCapFloor(YoYInflationCapFloor::Cap,leg, K, 0.01, whichPricer);

    std::shared_ptr<Instrument> floor
        = vars.makeYoYCapFloor(YoYInflationCapFloor::Floor,leg, K, 0.01, whichPricer);


    // close to atm prices
    Real cachedCapNPVblack   = 219.452;
    Real cachedFloorNPVblack =  314.641;
    // N.B. notionals are 10e6.
    BOOST_CHECK_MESSAGE(fabs(cap->NPV()-cachedCapNPVblack)<0.02,"yoy cap cached NPV wrong "
                        <<cap->NPV()<<" should be "<<cachedCapNPVblack<<" Black pricer"
                        <<" diff was "<<(fabs(cap->NPV()-cachedCapNPVblack)));
    BOOST_CHECK_MESSAGE(fabs(floor->NPV()-cachedFloorNPVblack)<0.02,"yoy floor cached NPV wrong "
                        <<floor->NPV()<<" should be "<<cachedFloorNPVblack<<" Black pricer"
                        <<" diff was "<<(fabs(floor->NPV()-cachedFloorNPVblack)));

    whichPricer = 1; // dd

    cap
    = vars.makeYoYCapFloor(YoYInflationCapFloor::Cap,leg, K, 0.01, whichPricer);

    floor
    = vars.makeYoYCapFloor(YoYInflationCapFloor::Floor,leg, K, 0.01, whichPricer);

    // close to atm prices
    Real cachedCapNPVdd   = 9114.61;
    Real cachedFloorNPVdd =  9209.8;
    // N.B. notionals are 10e6.
    BOOST_CHECK_MESSAGE(fabs(cap->NPV()-cachedCapNPVdd)<0.22,"yoy cap cached NPV wrong "
                        <<cap->NPV()<<" should be "<<cachedCapNPVdd<<" dd Black pricer"
                        <<" diff was "<<(fabs(cap->NPV()-cachedCapNPVdd)));
    BOOST_CHECK_MESSAGE(fabs(floor->NPV()-cachedFloorNPVdd)<0.22,"yoy floor cached NPV wrong "
                        <<floor->NPV()<<" should be "<<cachedFloorNPVdd<<" dd Black pricer"
                        <<" diff was "<<(fabs(floor->NPV()-cachedFloorNPVdd)));

    whichPricer = 2; // bachelier

    cap
    = vars.makeYoYCapFloor(YoYInflationCapFloor::Cap,leg, K, 0.01, whichPricer);

    floor
    = vars.makeYoYCapFloor(YoYInflationCapFloor::Floor,leg, K, 0.01, whichPricer);

    // close to atm prices
    Real cachedCapNPVbac   = 8852.4;
    Real cachedFloorNPVbac =  8947.59;
    // N.B. notionals are 10e6.
    BOOST_CHECK_MESSAGE(fabs(cap->NPV()-cachedCapNPVbac)<0.22,"yoy cap cached NPV wrong "
                        <<cap->NPV()<<" should be "<<cachedCapNPVbac<<" bac Black pricer"
                        <<" diff was "<<(fabs(cap->NPV()-cachedCapNPVbac)));
    BOOST_CHECK_MESSAGE(fabs(floor->NPV()-cachedFloorNPVbac)<0.22,"yoy floor cached NPV wrong "
                        <<floor->NPV()<<" should be "<<cachedFloorNPVbac<<" bac Black pricer"
                        <<" diff was "<<(fabs(floor->NPV()-cachedFloorNPVbac)));

    // remove circular refernce
    vars.hy.linkTo(std::shared_ptr<YoYInflationTermStructure>());
}


test_suite* InflationCapFloorTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Inflation (year-on-year) Cap and floor tests");
    suite->add(QUANTLIB_TEST_CASE(&InflationCapFloorTest::testConsistency));
    suite->add(QUANTLIB_TEST_CASE(&InflationCapFloorTest::testParity));
    suite->add(QUANTLIB_TEST_CASE(&InflationCapFloorTest::testCachedValue));
    return suite;
}


