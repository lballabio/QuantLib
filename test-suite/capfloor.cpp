
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "capfloor.hpp"
#include <ql/Instruments/capfloor.hpp>
#include <ql/Instruments/simpleswap.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/Pricers/blackcapfloor.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

// This makes it easier to use array literals (alas, no std::vector literals)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace QuantLib;
using namespace QuantLib::Indexes;
using namespace QuantLib::TermStructures;
using namespace QuantLib::Calendars;
using namespace QuantLib::DayCounters;
using namespace QuantLib::CashFlows;
using namespace QuantLib::Instruments;
using namespace QuantLib::Pricers;

namespace {

    // global data

    Date today_, settlement_;
    std::vector<double> nominals_;
    RollingConvention rollingConvention_;
    int frequency_;
    Handle<Xibor> index_;
    Calendar calendar_;
    int settlementDays_, fixingDays_;
    RelinkableHandle<TermStructure> termStructure_;

    // utilities

    std::vector<Handle<CashFlow> > makeLeg(const Date& startDate,
                                           int length) {
        Date endDate = calendar_.advance(startDate,length,Years,
                                         rollingConvention_);
        return FloatingRateCouponVector(nominals_,startDate,endDate,
                                        frequency_,calendar_,
                                        rollingConvention_, index_,
                                        fixingDays_,std::vector<Spread>());
    }

    Handle<PricingEngine> makeEngine(double volatility) {
        RelinkableHandle<MarketElement> vol(
            Handle<MarketElement>(new SimpleMarketElement(volatility)));
        Handle<BlackModel> model(new BlackModel(vol,termStructure_));
        return Handle<PricingEngine>(new BlackCapFloor(model));
    }

    Handle<Instrument> makeCapFloor(VanillaCapFloor::Type type,
                                    const std::vector<Handle<CashFlow> >& leg,
                                    Rate strike, 
                                    double volatility) {
        switch (type) {
          case VanillaCapFloor::Cap:
            return Handle<Instrument>(
               new VanillaCap(leg, std::vector<Rate>(1, strike),
                              termStructure_, makeEngine(volatility)));
          case VanillaCapFloor::Floor:
            return Handle<Instrument>(
                new VanillaFloor(leg, std::vector<Rate>(1, strike),
                                 termStructure_, makeEngine(volatility)));
          default:
            throw Error("unknown cap/floor type");
        }
    }

}


void CapFloorTest::setUp() {
    nominals_ = std::vector<double>(1,100.0);
    frequency_ = 2;
    index_ = Handle<Xibor>(new Euribor(12/frequency_,Months,termStructure_));
    calendar_ = index_->calendar();
    rollingConvention_ = ModifiedFollowing;
    today_ = calendar_.roll(Date::todaysDate());
    settlementDays_ = 2;
    fixingDays_ = 2;
    settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(
        Handle<TermStructure>(new FlatForward(today_,settlement_,0.05,
                                              Actual360())));
}

void CapFloorTest::testStrikeDependency() {

    int lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    double vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };
    double strikes[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    Date startDate = termStructure_->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(vols); j++) {
            // store the results for different strikes...
            std::vector<double> cap_values, floor_values;
            for (Size k=0; k<LENGTH(strikes); k++) {
                std::vector<Handle<CashFlow> > leg = 
                    makeLeg(startDate,lengths[i]);
                Handle<Instrument> cap = 
                    makeCapFloor(VanillaCapFloor::Cap,leg,
                                 strikes[k],vols[j]);
                cap_values.push_back(cap->NPV());
                Handle<Instrument> floor = 
                    makeCapFloor(VanillaCapFloor::Floor,leg,
                                 strikes[k],vols[j]);
                floor_values.push_back(floor->NPV());
            }
            // and check that they go the right way
            std::vector<double>::iterator it = 
                std::adjacent_find(cap_values.begin(),cap_values.end(),
                                   std::less<double>());
            if (it != cap_values.end()) {
                int n = it - cap_values.begin();
                CPPUNIT_FAIL(
                    "NPV is increasing with the strike in a cap: \n"
                    "    length: " + 
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    volatility: " +
                    DoubleFormatter::toString(vols[j]*100,2) + " %\n"
                    "    value: " +
                    DoubleFormatter::toString(cap_values[n]) +
                    " at strike: " +
                    DoubleFormatter::toString(strikes[n]*100,2) + " %\n"
                    "    value: " +
                    DoubleFormatter::toString(cap_values[n+1]) +
                    " at strike: " +
                    DoubleFormatter::toString(strikes[n+1]*100,2) + " %");
            }
            // same for floors
            it = std::adjacent_find(floor_values.begin(),floor_values.end(),
                                    std::greater<double>());
            if (it != floor_values.end()) {
                int n = it - floor_values.begin();
                CPPUNIT_FAIL(
                    "NPV is decreasing with the strike in a floor: \n"
                    "    length: " + 
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    volatility: " +
                    DoubleFormatter::toString(vols[j]*100,2) + " %\n"
                    "    value: " +
                    DoubleFormatter::toString(floor_values[n]) +
                    " at strike: " +
                    DoubleFormatter::toString(strikes[n]*100,2) + " %\n"
                    "    value: " +
                    DoubleFormatter::toString(floor_values[n+1]) +
                    " at strike: " +
                    DoubleFormatter::toString(strikes[n+1]*100,2) + " %");
            }
        }
    }
}

void CapFloorTest::testConsistency() {

    int lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    double cap_rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    double floor_rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    double vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };

    Date startDate = termStructure_->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
      for (Size j=0; j<LENGTH(cap_rates); j++) {
        for (Size k=0; k<LENGTH(floor_rates); k++) {
          for (Size l=0; l<LENGTH(vols); l++) {
              
              std::vector<Handle<CashFlow> > leg = 
                  makeLeg(startDate,lengths[i]);
              Handle<Instrument> cap = 
                  makeCapFloor(VanillaCapFloor::Cap,leg,
                               cap_rates[j],vols[l]);
              Handle<Instrument> floor = 
                  makeCapFloor(VanillaCapFloor::Floor,leg,
                               floor_rates[k],vols[l]);
              VanillaCollar collar(leg,std::vector<Rate>(1,cap_rates[j]),
                                   std::vector<Rate>(1,floor_rates[k]),
                                   termStructure_,makeEngine(vols[l]));
              
              if (QL_FABS((cap->NPV()-floor->NPV())-collar.NPV()) > 1.0e-10) {
                  CPPUNIT_FAIL(
                    "inconsistency between cap, floor and collar:\n"
                    "    length: " + 
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    volatility: " +
                    DoubleFormatter::toString(vols[l]*100,2) + " %\n"
                    "    cap value: " +
                    DoubleFormatter::toString(cap->NPV()) +
                    " at strike: " +
                    DoubleFormatter::toString(cap_rates[j]*100,2) + " %\n"
                    "    floor value: " +
                    DoubleFormatter::toString(floor->NPV()) +
                    " at strike: " +
                    DoubleFormatter::toString(floor_rates[k]*100,2) + " %\n"
                    "    collar value: " +
                    DoubleFormatter::toString(collar.NPV()));
              }
          }
        }
      }
    }
}

void CapFloorTest::testParity() {

    int lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    double strikes[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    double vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };

    Date startDate = termStructure_->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(vols); k++) {
                
            std::vector<Handle<CashFlow> > leg = 
                makeLeg(startDate,lengths[i]);
            Handle<Instrument> cap = 
                makeCapFloor(VanillaCapFloor::Cap,leg,
                             strikes[j],vols[k]);
            Handle<Instrument> floor = 
                makeCapFloor(VanillaCapFloor::Floor,leg,
                             strikes[j],vols[k]);
            SimpleSwap swap(true,startDate,lengths[i],Years,
                            calendar_,rollingConvention_,
                            nominals_[0],frequency_,strikes[j],
                            index_->isAdjusted(),index_->dayCounter(),
                            frequency_,index_,fixingDays_,0.0,
                            termStructure_);
            if (QL_FABS((cap->NPV()-floor->NPV()) - swap.NPV()) > 1.0e-10) {
                CPPUNIT_FAIL(
                    "put/call parity violated:\n"
                    "    length: " + 
                    IntegerFormatter::toString(lengths[i]) + " years\n"
                    "    volatility: " +
                    DoubleFormatter::toString(vols[k]*100,2) + " %\n"
                    "    strike: " +
                    DoubleFormatter::toString(strikes[j]*100,2) + " %\n"
                    "    cap value: " +
                    DoubleFormatter::toString(cap->NPV()) + "\n"
                    "    floor value: " +
                    DoubleFormatter::toString(floor->NPV()) + "\n"
                    "    swap value: " +
                    DoubleFormatter::toString(swap.NPV()));
            }
        }
      }
    }
}

void CapFloorTest::testCachedValue() {

    Date cachedToday(14,March,2002),
         cachedSettlement(18,March,2002);
    termStructure_.linkTo(
        Handle<TermStructure>(new FlatForward(cachedToday,cachedSettlement,
                                              0.05, Actual360())));
    Date startDate = termStructure_->referenceDate();
    std::vector<Handle<CashFlow> > leg = makeLeg(startDate,20);
    Handle<Instrument> cap = makeCapFloor(VanillaCapFloor::Cap,leg,
                                          0.07,0.20);
    Handle<Instrument> floor = makeCapFloor(VanillaCapFloor::Floor,leg,
                                            0.03,0.20);
    double cachedCapNPV   = 6.958275537948,
           cachedFloorNPV = 2.700478483500;
    
    if (QL_FABS(cap->NPV()-cachedCapNPV) > 1.0e-11)
        CPPUNIT_FAIL(
            "failed to reproduce cached cap value:\n"
            "    calculated: " +
            DoubleFormatter::toString(cap->NPV(),12) + "\n"
            "    expected:   " +
            DoubleFormatter::toString(cachedCapNPV,12));

    if (QL_FABS(floor->NPV()-cachedFloorNPV) > 1.0e-11)
        CPPUNIT_FAIL(
            "failed to reproduce cached floor value:\n"
            "    calculated: " +
            DoubleFormatter::toString(floor->NPV(),12) + "\n"
            "    expected:   " +
            DoubleFormatter::toString(cachedFloorNPV,12));
}
        
CppUnit::Test* CapFloorTest::suite() {
    CppUnit::TestSuite* tests = new CppUnit::TestSuite("Cap/floor tests");
    tests->addTest(new CppUnit::TestCaller<CapFloorTest>
                   ("Testing cap/floor dependency on strike",
                    &CapFloorTest::testStrikeDependency));
    tests->addTest(new CppUnit::TestCaller<CapFloorTest>
                   ("Testing consistency between cap, floor and collar",
                    &CapFloorTest::testConsistency));
    tests->addTest(new CppUnit::TestCaller<CapFloorTest>
                   ("Testing put/call parity for cap and floor",
                    &CapFloorTest::testParity));
    tests->addTest(new CppUnit::TestCaller<CapFloorTest>
                   ("Testing cap/floor value against cached values",
                    &CapFloorTest::testCachedValue));
    return tests;
}

