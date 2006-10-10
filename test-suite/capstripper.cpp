/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Francois du Vignaud

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

#include "capstripper.hpp"
#include "utilities.hpp"
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/Volatilities/capstripper.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(CapsStripperTest)

Calendar calendar;
DayCounter dayCounter;
Size strikesNb, tenorsNb;
std::vector<Rate> strikes;
std::vector<Period> tenors;
std::vector<std::vector<Handle<Quote> > > vols;
boost::shared_ptr<FlatForward> myTermStructure;
boost::shared_ptr<Quote> forwardRate;
Handle<Quote> forwardRateQuote;
Handle<YieldTermStructure> rhTermStructure;
Date settlementDate;
boost::shared_ptr<Xibor> index;
int fixingDays;
BusinessDayConvention businessDayConvention;
QL_END_TEST_LOCALS(CapsStripperTest)

void setup() {
    strikesNb = 2;
    tenorsNb = 10;
    calendar = TARGET();
    dayCounter = Actual365Fixed();
    fixingDays = 2;
    businessDayConvention = Unadjusted;
    settlementDate = Settings::instance().evaluationDate();
    for (Size i = 0 ; i < tenorsNb; i++)
        tenors.push_back(Period(i+1, Years));
    for (Size j = 0 ; j < strikesNb; j++)
        strikes.push_back(double(j+5)/100);
    

    // volatilities quotes creation
    vols.resize(tenorsNb);
    for (Size i = 0 ; i < tenorsNb; i++){
        vols[i].resize(strikesNb);
        for (Size j = 0 ; j < strikesNb; j++){
            vols[i][j] = Handle<Quote>(boost::shared_ptr<SimpleQuote>
                (new SimpleQuote(.13)),true);
        }
    }
    forwardRate = boost::shared_ptr<Quote>(new SimpleQuote(0.04875825));
    forwardRateQuote.linkTo(forwardRate);
    myTermStructure = boost::shared_ptr<FlatForward>(
                  new FlatForward(settlementDate, forwardRateQuote,
                                  Actual365Fixed()));
    rhTermStructure.linkTo(myTermStructure);
    index = boost::shared_ptr<Xibor>(new Euribor6M(rhTermStructure));
}

void CapsStripperTest::testConstruction() {

    BOOST_MESSAGE("Testing CapsStripper construction...");
    
    QL_TEST_BEGIN
    setup();
    CapsStripper capsStripper(  calendar, 
                                businessDayConvention,
                                2,
                                tenors,
                                strikes,
                                vols, 
                                dayCounter,
                                index,
                                rhTermStructure);
    Volatility test = capsStripper.volatility(3,2.5/100);
    Date maxDate = capsStripper.maxDate();
    QL_TEST_END
}


void CapsStripperTest::testConsistency() {

    BOOST_MESSAGE("Testing bootstrap consistency...");
    
    QL_TEST_BEGIN
    setup();
    boost::shared_ptr<CapsStripper> 
        capsStripper(new CapsStripper(  calendar, 
                                        businessDayConvention,
                                        fixingDays,
                                        tenors,
                                        strikes,
                                        vols, 
                                        dayCounter,
                                        index,
                                        rhTermStructure));
    std::cout << capsStripper->dayCounter() << std::endl;
    // we ask a value to trigger cap stripping
    Volatility test = capsStripper->volatility(3,2.5/100);
    // we display stripped caps volatilities
    std::cout << capsStripper->volatilities_;
    // we test cap prices using the resulting CapletVolatilityStructure
    Handle<CapletVolatilityStructure> capsStripperHandle(capsStripper); 
    boost::shared_ptr<PricingEngine> blackCapFloorEngineCapStripper
            (new BlackCapFloorEngine(capsStripperHandle));
    
    // we expose market data caps
    const std::vector<std::vector<boost::shared_ptr<CapFloor> > >&
        marketDataCap = capsStripper->marketDataCap;

    // we expose stripped caps
    const std::vector<std::vector<boost::shared_ptr<CapFloor> > >& 
        strippedCap = capsStripper->strippedCap;
    std::vector<Real> priceFromStrippedCaps(strikesNb,0);
    
    std::cout << "tenor\tstrike\tStp\tCst\tmkt\tStp2\tATM"<< std::endl;
    
    // for every tenor we create a schedule and the corresponding floating leg
    for (Size tenorTestedIndex = 0; tenorTestedIndex < tenorsNb ; tenorTestedIndex++){
        LegHelper legHelper(settlementDate, calendar, fixingDays, businessDayConvention, index);
        FloatingLeg floatingLeg = legHelper.makeLeg(index->tenor(), tenors[tenorTestedIndex]);
       
        // for every strike we compute the price using different pricing engines    
        for (Size strikeTestedIndex = 0; strikeTestedIndex < strikesNb ; strikeTestedIndex ++){
            
            // we compute the price using the StrippedVolatilty
            boost::shared_ptr<CapFloor> cap(new Cap(floatingLeg,
                                            std::vector<Real>(1,strikes[strikeTestedIndex]),
                                            rhTermStructure, blackCapFloorEngineCapStripper));
            Real priceFromStrippedVolatilty = cap->NPV();
            
            // then using the original constant volatility structure
            boost::shared_ptr<PricingEngine> blackCapFloorEngineConstantVolatility
                    (new BlackCapFloorEngine(vols[tenorTestedIndex][strikeTestedIndex]));
            cap->setPricingEngine(blackCapFloorEngineConstantVolatility);
            Real priceFromConstantVolatilty = cap->NPV();

            // finally using the stripped caps ...
            if (tenorTestedIndex > 0)
                priceFromStrippedCaps[strikeTestedIndex] += 
                    strippedCap[tenorTestedIndex-1][strikeTestedIndex]->NPV();
            else
                priceFromStrippedCaps[strikeTestedIndex] = 
                    marketDataCap[0][strikeTestedIndex]->NPV();

            /*if(std::fabs(priceFromStrippedVolatilty -
                priceFromConstantVolatilty)> .001)*/
                std::cout << tenors[tenorTestedIndex] << "\t" 
                        << strikes[strikeTestedIndex]*100 << "%\t" 
                        << priceFromStrippedVolatilty *1000 << "\t"
                        << priceFromConstantVolatilty *1000 << "\t" 
                        << marketDataCap[tenorTestedIndex][strikeTestedIndex]->NPV() *1000<< "\t"
                        << priceFromStrippedCaps[strikeTestedIndex]*1000 << "\t"
                        << marketDataCap[tenorTestedIndex][strikeTestedIndex]->ATMRate()
                        << std::endl;
        }
    }
    QL_TEST_END
}


test_suite* CapsStripperTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("CapsStripperTest tests");
    //suite->add(BOOST_TEST_CASE(&CapsStripperTest::testConstruction));
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::testConsistency));
    //suite->add(BOOST_TEST_CASE(&CapsStripperTest::testHandle));
    return suite;
}