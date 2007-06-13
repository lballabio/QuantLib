/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2004, 2005, 2006, 2007 StatPro Italia srl

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

#include "capfloor.hpp"
#include "utilities.hpp"
#include <ql/instruments/capfloor.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/termstructures/yieldcurves/flatforward.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/pricingengines/capfloor/marketmodelcapfloorengine.hpp>
#include <ql/models/marketmodels/models/flatvol.hpp>
#include <ql/models/marketmodels/correlations/correlations.hpp>
#include <ql/math/matrix.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(CapFloorTest)

// global data

Date today_, settlement_;
std::vector<Real> nominals_;
BusinessDayConvention convention_;
Frequency frequency_;
boost::shared_ptr<IborIndex> index_;
Calendar calendar_;
Natural settlementDays_, fixingDays_;
RelinkableHandle<YieldTermStructure> termStructure_;

// utilities

bool checkAbsError(Real x1, Real x2, Real tolerance){
    return std::fabs(x1 - x2) < tolerance;
}

Leg makeLeg(const Date& startDate, Integer length) {
    Date endDate = calendar_.advance(startDate,length*Years,convention_);
    Schedule schedule(startDate, endDate, Period(frequency_), calendar_,
                      convention_, convention_, false, false);
    Leg floatLeg =IborLeg(nominals_,
                          schedule,
                          index_,
                          index_->dayCounter(),
                          convention_,
                          std::vector<Natural>(1,fixingDays_),
                          std::vector<Real>(),
                          std::vector<Spread>());
    boost::shared_ptr<IborCouponPricer>
                        fictitiousPricer(new BlackIborCouponPricer(Handle<CapletVolatilityStructure>()));
    setCouponPricer(floatLeg,fictitiousPricer);
    return floatLeg;
}

boost::shared_ptr<PricingEngine> makeEngine(Volatility volatility) {
    Handle<Quote> vol(boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
    return boost::shared_ptr<PricingEngine>(new BlackCapFloorEngine(vol));
}

boost::shared_ptr<CapFloor> makeCapFloor(
                         CapFloor::Type type,
                         const Leg& leg,
                         Rate strike,
                         Volatility volatility) {
    switch (type) {
      case CapFloor::Cap:
        return boost::shared_ptr<CapFloor>(
               new Cap(leg, std::vector<Rate>(1, strike),
                       termStructure_, makeEngine(volatility)));
      case CapFloor::Floor:
            return boost::shared_ptr<CapFloor>(
                new Floor(leg, std::vector<Rate>(1, strike),
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

void setup() {
    nominals_ = std::vector<Real>(1,100);
    frequency_ = Semiannual;
    index_ = boost::shared_ptr<IborIndex>(new Euribor6M(termStructure_));
    calendar_ = index_->fixingCalendar();
    convention_ = ModifiedFollowing;
    today_ = calendar_.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today_;
    settlementDays_ = 2;
    fixingDays_ = 2;
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(flatRate(settlement_,0.05,
                                   ActualActual(ActualActual::ISDA)));
}

QL_END_TEST_LOCALS(CapFloorTest)


void CapFloorTest::testVega() {

    BOOST_MESSAGE("Testing cap/floor vega...");

    SavedSettings backup;

    setup();

    Integer lengths[] = { 1, 2, 3, 4, 5, 6, 7, 10, 15, 20, 30 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };
    Rate strikes[] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09 };
    CapFloor::Type types[] = { CapFloor::Cap, CapFloor::Floor};

    Date startDate = termStructure_->referenceDate();
    static const Real shift = 1e-8;
    static const Real tolerance = 0.005;

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(vols); j++) {
            for (Size k=0; k<LENGTH(strikes); k++) {
                for (Size h=0; h<LENGTH(types); h++) {
                    Leg leg =
                        makeLeg(startDate, lengths[i]);
                    boost::shared_ptr<CapFloor> capFloor =
                        makeCapFloor(types[h],leg,
                                     strikes[k],vols[j]);
                    boost::shared_ptr<CapFloor> shiftedCapFloor2 =
                        makeCapFloor(types[h],leg,
                                     strikes[k],vols[j]+shift);
                     boost::shared_ptr<CapFloor> shiftedCapFloor1 =
                        makeCapFloor(types[h],leg,
                                     strikes[k],vols[j]-shift);
                    Real value1 = shiftedCapFloor1->NPV();
                    Real value2 = shiftedCapFloor2->NPV();
                    Real numericalVega = (value2 - value1) / (2*shift);
                    if (numericalVega>1.0e-4) {
                        Real analyticalVega = capFloor->result<Real>("vega");
                        Real discrepancy =
                            std::fabs(numericalVega - analyticalVega);
                        discrepancy /= numericalVega;
                        if (discrepancy > tolerance)
                            BOOST_FAIL(
                                "failed to compute cap/floor vega:" <<
                                "\n   lengths:     " << lengths[j]*Years <<
                                "\n   strike:      " << io::rate(strikes[k]) <<
                                //"\n   types:       " << types[h] <<
                                QL_FIXED << std::setprecision(12) <<
                                "\n   calculated:  " << analyticalVega <<
                                "\n   expected:    " << numericalVega <<
                                "\n   discrepancy: " << io::rate(discrepancy) <<
                                "\n   tolerance:   " << io::rate(tolerance));
                     }
                }
            }
        }
    }
}

void CapFloorTest::testStrikeDependency() {

    BOOST_MESSAGE("Testing cap/floor dependency on strike...");

    SavedSettings backup;

    setup();

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };
    Rate strikes[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    Date startDate = termStructure_->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(vols); j++) {
            // store the results for different strikes...
            std::vector<Real> cap_values, floor_values;
            for (Size k=0; k<LENGTH(strikes); k++) {
                Leg leg =
                    makeLeg(startDate,lengths[i]);
                boost::shared_ptr<Instrument> cap =
                    makeCapFloor(CapFloor::Cap,leg,
                                 strikes[k],vols[j]);
                cap_values.push_back(cap->NPV());
                boost::shared_ptr<Instrument> floor =
                    makeCapFloor(CapFloor::Floor,leg,
                                 strikes[k],vols[j]);
                floor_values.push_back(floor->NPV());
            }
            // and check that they go the right way
            std::vector<Real>::iterator it =
                std::adjacent_find(cap_values.begin(),cap_values.end(),
                                   std::less<Real>());
            if (it != cap_values.end()) {
                Size n = it - cap_values.begin();
                BOOST_FAIL(
                    "NPV is increasing with the strike in a cap: \n"
                    << std::setprecision(2)
                    << "    length:     " << lengths[i] << " years\n"
                    << "    volatility: " << io::volatility(vols[j]) << "\n"
                    << "    value:      " << cap_values[n]
                    << " at strike: " << io::rate(strikes[n]) << "\n"
                    << "    value:      " << cap_values[n+1]
                    << " at strike: " << io::rate(strikes[n+1]));
            }
            // same for floors
            it = std::adjacent_find(floor_values.begin(),floor_values.end(),
                                    std::greater<Real>());
            if (it != floor_values.end()) {
                Size n = it - floor_values.begin();
                BOOST_FAIL(
                    "NPV is decreasing with the strike in a floor: \n"
                    << std::setprecision(2)
                    << "    length:     " << lengths[i] << " years\n"
                    << "    volatility: " << io::volatility(vols[j]) << "\n"
                    << "    value:      " << floor_values[n]
                    << " at strike: " << io::rate(strikes[n]) << "\n"
                    << "    value:      " << floor_values[n+1]
                    << " at strike: " << io::rate(strikes[n+1]));
            }
        }
    }
}

void CapFloorTest::testConsistency() {

    BOOST_MESSAGE("Testing consistency between cap, floor and collar...");

    SavedSettings backup;

    setup();

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Rate cap_rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    Rate floor_rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };

    Date startDate = termStructure_->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
      for (Size j=0; j<LENGTH(cap_rates); j++) {
        for (Size k=0; k<LENGTH(floor_rates); k++) {
          for (Size l=0; l<LENGTH(vols); l++) {

              Leg leg =
                  makeLeg(startDate,lengths[i]);
              boost::shared_ptr<Instrument> cap =
                  makeCapFloor(CapFloor::Cap,leg,
                               cap_rates[j],vols[l]);
              boost::shared_ptr<Instrument> floor =
                  makeCapFloor(CapFloor::Floor,leg,
                               floor_rates[k],vols[l]);
              Collar collar(leg,std::vector<Rate>(1,cap_rates[j]),
                            std::vector<Rate>(1,floor_rates[k]),
                            termStructure_,makeEngine(vols[l]));

              if (std::fabs((cap->NPV()-floor->NPV())-collar.NPV()) > 1e-10) {
                  BOOST_FAIL(
                    "inconsistency between cap, floor and collar:\n"
                    << "    length:       " << lengths[i] << " years\n"
                    << "    volatility:   " << io::volatility(vols[l]) << "\n"
                    << "    cap value:    " << cap->NPV()
                    << " at strike: " << io::rate(cap_rates[j]) << "\n"
                    << "    floor value:  " << floor->NPV()
                    << " at strike: " << io::rate(floor_rates[k]) << "\n"
                    << "    collar value: " << collar.NPV());
              }
          }
        }
      }
    }
}

void CapFloorTest::testParity() {

    BOOST_MESSAGE("Testing cap/floor parity...");

    SavedSettings backup;

    setup();

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Rate strikes[] = { 0., 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };

    Date startDate = termStructure_->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(vols); k++) {

            Leg leg =
                makeLeg(startDate,lengths[i]);
            boost::shared_ptr<Instrument> cap =
                makeCapFloor(CapFloor::Cap,leg,
                             strikes[j],vols[k]);
            boost::shared_ptr<Instrument> floor =
                makeCapFloor(CapFloor::Floor,leg,
                             strikes[j],vols[k]);
            Date maturity = calendar_.advance(startDate,lengths[i],Years,
                                              convention_);
            Schedule schedule(startDate,maturity,Period(frequency_),calendar_,
                              convention_,convention_,false,false);
            VanillaSwap swap(VanillaSwap::Payer, nominals_[0],
                             schedule, strikes[j], index_->dayCounter(),
                             schedule, index_, 0.0, index_->dayCounter(),
                             termStructure_);
            if (std::fabs((cap->NPV()-floor->NPV()) - swap.NPV()) > 1.0e-10) {
                BOOST_FAIL(
                    "put/call parity violated:\n"
                    << "    length:      " << lengths[i] << " years\n"
                    << "    volatility:  " << io::volatility(vols[k]) << "\n"
                    << "    strike:      " << io::rate(strikes[j]) << "\n"
                    << "    cap value:   " << cap->NPV() << "\n"
                    << "    floor value: " << floor->NPV() << "\n"
                    << "    swap value:  " << swap.NPV());
            }
        }
      }
    }
}

void CapFloorTest::testATMRate() {

    BOOST_MESSAGE("Testing ATM rate...");

    SavedSettings backup;

    setup();

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Rate strikes[] = { 0., 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };

    Date startDate = termStructure_->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
        Leg leg =
            makeLeg(startDate,lengths[i]);
        Date maturity = calendar_.advance(startDate,lengths[i],Years,
                                  convention_);
        Schedule schedule(startDate,maturity,Period(frequency_),calendar_,
                              convention_,convention_,false,false);

        for (Size j=0; j<LENGTH(strikes); j++) {
            for (Size k=0; k<LENGTH(vols); k++) {
                boost::shared_ptr<CapFloor> cap =
                    makeCapFloor(CapFloor::Cap, leg, strikes[j],vols[k]);
                boost::shared_ptr<CapFloor> floor =
                    makeCapFloor(CapFloor::Floor, leg, strikes[j],vols[k]);
                Rate capATMRate = cap->atmRate();
                Rate floorATMRate = floor->atmRate();
                if (!checkAbsError(floorATMRate, capATMRate, 1.0e-10))
                    BOOST_FAIL("Cap ATM Rate and floor ATM Rate should be equal :"
                        << "   length:        " << lengths[i] << " years\n"
                        << "   volatility:    " << io::volatility(vols[k]) << "\n"
                        << "   strike:        " << io::rate(strikes[j]) << "\n"
                        << "   cap ATM rate:  " << capATMRate << "\n"
                        << "   floor ATM rate:" << floorATMRate << "\n"
                        << "   relative Error:"
                        << relativeError(capATMRate, floorATMRate, capATMRate)*100 << "%" );
                VanillaSwap swap(VanillaSwap::Payer, nominals_[0],
                                 schedule, floorATMRate, index_->dayCounter(),
                                 schedule, index_, 0.0, index_->dayCounter(),
                                 termStructure_);
                Real swapNPV = swap.NPV();
                if (!checkAbsError(swapNPV, 0, 1.0e-10))
                    BOOST_FAIL("the NPV of a Swap struck at ATM rate should be equal to 0:"
                        << "   length:        " << lengths[i] << " years\n"
                        << "   volatility:    " << io::volatility(vols[k]) << "\n"
                        << "   ATM rate:      " << io::rate(floorATMRate) << "\n"
                        << "   swap NPV:      " << swapNPV);
        }
      }
    }
}





void CapFloorTest::testImpliedVolatility() {

    BOOST_MESSAGE("Testing implied term volatility for cap and floor...");

    SavedSettings backup;

    setup();

    Size maxEvaluations = 100;
    Real tolerance = 1.0e-6;

    CapFloor::Type types[] = { CapFloor::Cap, CapFloor::Floor };
    Rate strikes[] = { 0.02, 0.03, 0.04 };
    Integer lengths[] = { 1, 5, 10 };

    // test data
    Rate rRates[] = { 0.02, 0.03, 0.04 };
    Volatility vols[] = { 0.01, 0.20, 0.30, 0.70, 0.90 };

    for (Size k=0; k<LENGTH(lengths); k++) {
        Leg leg =
            makeLeg(settlement_, lengths[k]);

        for (Size i=0; i<LENGTH(types); i++) {
            for (Size j=0; j<LENGTH(strikes); j++) {

                boost::shared_ptr<CapFloor> capfloor =
                    makeCapFloor(types[i], leg, strikes[j], 0.0);

                for (Size n=0; n<LENGTH(rRates); n++) {
                    for (Size m=0; m<LENGTH(vols); m++) {

                        Rate r = rRates[n];
                        Volatility v = vols[m];
                        termStructure_.linkTo(flatRate(settlement_,r,
                                                       Actual360()));
                        capfloor->setPricingEngine(makeEngine(v));

                        Real value = capfloor->NPV();
                        Volatility implVol = 0.0;

                        try {
                            implVol =
                                capfloor->impliedVolatility(value,
                                                            tolerance,
                                                            maxEvaluations);
                        } catch (std::exception& e) {
                            BOOST_FAIL(typeToString(types[i]) <<
                                "\n  strike:     " << io::rate(strikes[j]) <<
                                "\n  risk-free:  " << io::rate(r) <<
                                "\n  length:     " << lengths[k] << "Y" <<
                                "\n  volatility: " << io::volatility(v) <<
                                "\n" << e.what());
                        }
                        if (std::fabs(implVol-v) > tolerance) {
                            // the difference might not matter
                            capfloor->setPricingEngine(makeEngine(implVol));
                            Real value2 = capfloor->NPV();
                            if (std::fabs(value-value2) > tolerance) {
                                BOOST_FAIL(
                                    typeToString(types[i]) << ":\n"
                                    << "    strike:           "
                                    << strikes[j] << "\n"
                                    << "    risk-free rate:   "
                                    << io::rate(r) << "\n"
                                    << "    length:         "
                                    << lengths[k] << " years\n\n"
                                    << "    original volatility: "
                                    << io::volatility(v) << "\n"
                                    << "    price:               "
                                    << value << "\n"
                                    << "    implied volatility:  "
                                    << io::volatility(implVol) << "\n"
                                    << "    corresponding price: " << value2);
                            }
                        }
                    }
                }
            }
        }
    }
}

void CapFloorTest::testMarketModel() {

    BOOST_MESSAGE("Testing cap/floor pricing with market-model engine...");

    SavedSettings backup;

    setup();

    Date cachedToday(14,March,2002),
         cachedSettlement(18,March,2002);
    Settings::instance().evaluationDate() = cachedToday;
    termStructure_.linkTo(flatRate(cachedSettlement, 0.05, Actual360()));
    //Date startDate = calendar_.advance(cachedSettlement,1,Months);
    Date startDate = termStructure_->referenceDate();
    Size term = 20;
    Leg leg = makeLeg(startDate,term);
    Real strikeCap = 0.07;
    Real strikeFloor = 0.03;
    Real volatility = 0.20;
    // build & cap&floor instruments
    boost::shared_ptr<Instrument> cap = makeCapFloor(CapFloor::Cap,leg,
                                                     strikeCap,volatility);
    boost::shared_ptr<Instrument> floor = makeCapFloor(CapFloor::Floor,leg,
                                                       strikeFloor,volatility);
    // Black cap/floor price
    Real blackCapNPV = cap->NPV();
    Real blackFloorNPV = floor->NPV();
    std::vector<Real> blackCapletsNpv = cap->result<std::vector<Real> >("optionletsPrice");
    std::vector<Real> blackFloorletsNpv = floor->result<std::vector<Real> >("optionletsPrice");

    // Build market model
    Real longTermCorrelation = 0.5;
    Real beta = 0.2;
    Spread displacement = 0.0;
    std::vector<Time> times(2);
    std::vector<Volatility> vols(2);
    times[0] = 0.0;  vols[0] = volatility;
    times[1] = 30.0;  vols[1] = volatility;
    boost::shared_ptr<MarketModelFactory> factory(new
        FlatVolFactory(longTermCorrelation, beta,
                       times, vols,
                       termStructure_,
                       displacement));
    boost::shared_ptr<PricingEngine> lmmEngine(new
        MarketModelCapFloorEngine(factory));
    cap->setPricingEngine(lmmEngine);
    floor->setPricingEngine(lmmEngine);

    // LMM cap/floor price
    Real lmmCapNPV = cap->NPV();
    Real lmmFloorNPV = floor->NPV();
    std::vector<Real> lmmCapletsNpv = cap->result<std::vector<Real> >("optionletsPrice");
    std::vector<Real> lmmFloorletsNpv = floor->result<std::vector<Real> >("optionletsPrice");
    std::vector<Real> capletsError = cap->result<std::vector<Real> >("optionletsError");
    std::vector<Real> floorletsError = floor->result<std::vector<Real> >("optionletsError");
    QL_REQUIRE(lmmCapletsNpv.size() == blackCapletsNpv.size(),"lmmCapletsNpv.size() != blackCapletsNpv.size()");

    // check results for optionlets
    Real errorThreshold = 1;
    for (Size i=0; i<lmmCapletsNpv.size(); ++i){
        if (std::fabs(lmmCapletsNpv[i]-blackCapletsNpv[i]) > errorThreshold * capletsError[i])
            BOOST_FAIL(
                "failed to reproduce black caplet value: \n"
                << std::setprecision(12)
                << "    caplet #: " << i << "\n"
                << "    calculated: " << lmmCapletsNpv[i] << "\n"
                << "    stdev:   " << capletsError[i] << "\n"
                << "    expected:   " << blackCapletsNpv[i] );
        if (std::fabs(lmmFloorletsNpv[i]-blackFloorletsNpv[i]) > errorThreshold * floorletsError[i])
            BOOST_FAIL(
                "failed to reproduce black floorlet value:\n"
                << std::setprecision(12)
                << "    floorlet #: " << i << "\n"
                << "    calculated: " << lmmFloorletsNpv[i] << "\n"
                 << "    stdev:   " << floorletsError[i] << "\n"
               << "    expected:   " << blackFloorletsNpv[i]);
    }

    // check results for options
    if (std::fabs(lmmCapNPV-blackCapNPV) > errorThreshold * cap->errorEstimate())
        BOOST_ERROR(
            "failed to reproduce black cap value:\n"
            << std::setprecision(12)
            << "    calculated: " << lmmCapNPV << "\n"
            << "    stdev:   " << cap->errorEstimate() << "\n"
            << "    expected:   " << blackCapNPV);
    if (std::fabs(lmmFloorNPV-blackFloorNPV) > errorThreshold * floor->errorEstimate())
        BOOST_ERROR(
            "failed to reproduce black floor value:\n"
            << std::setprecision(12)
            << "    calculated: " << lmmFloorNPV << "\n"
            << "    stdev:   " << floor->errorEstimate() << "\n"
            << "    expected:   " <<blackFloorNPV);
}

void CapFloorTest::testCachedValue() {

    BOOST_MESSAGE("Testing Black cap/floor price against cached values...");

    SavedSettings backup;

    setup();

    Date cachedToday(14,March,2002),
         cachedSettlement(18,March,2002);
    Settings::instance().evaluationDate() = cachedToday;
    termStructure_.linkTo(flatRate(cachedSettlement, 0.05, Actual360()));
    Date startDate = termStructure_->referenceDate();
    Leg leg = makeLeg(startDate,20);
    boost::shared_ptr<Instrument> cap = makeCapFloor(CapFloor::Cap,leg,
                                                     0.07,0.20);
    boost::shared_ptr<Instrument> floor = makeCapFloor(CapFloor::Floor,leg,
                                                       0.03,0.20);
#ifndef QL_USE_INDEXED_COUPON
    // par coupon price
    Real cachedCapNPV   = 6.87570026732,
         cachedFloorNPV = 2.65812927959;
#else
    // index fixing price
    Real cachedCapNPV   = 6.87630307745,
         cachedFloorNPV = 2.65796764715;
#endif
    // test Black cap price against cached value
    if (std::fabs(cap->NPV()-cachedCapNPV) > 1.0e-11)
        BOOST_ERROR(
            "failed to reproduce cached cap value:\n"
            << std::setprecision(12)
            << "    calculated: " << cap->NPV() << "\n"
            << "    expected:   " << cachedCapNPV);
    // test Black floor price against cached value
    if (std::fabs(floor->NPV()-cachedFloorNPV) > 1.0e-11)
        BOOST_ERROR(
            "failed to reproduce cached floor value:\n"
            << std::setprecision(12)
            << "    calculated: " << floor->NPV() << "\n"
            << "    expected:   " <<cachedFloorNPV);
}


test_suite* CapFloorTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Cap/floor tests");
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testStrikeDependency));
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testConsistency));
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testParity));
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testVega));
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testATMRate));
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testImpliedVolatility));
    //suite->add(BOOST_TEST_CASE(&CapFloorTest::testMarketModel));
    suite->add(BOOST_TEST_CASE(&CapFloorTest::testCachedValue));
    return suite;
}

