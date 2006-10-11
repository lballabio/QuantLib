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
std::vector<std::vector<Handle<Quote> > > flatVolatilities;
boost::shared_ptr<FlatForward> myTermStructure;
boost::shared_ptr<Quote> forwardRate;
Handle<Quote> forwardRateQuote;
Handle<YieldTermStructure> rhTermStructure;
Date settlementDate;
boost::shared_ptr<Xibor> index;
int fixingDays;
BusinessDayConvention businessDayConvention;
boost::shared_ptr<CapsStripper> capsStripper;
Volatility flatVolatility;
Rate flatForwardRate;
QL_END_TEST_LOCALS(CapsStripperTest)


void setup() {
    strikesNb = 10;
    tenorsNb = 10;
    calendar = TARGET();
    dayCounter = Actual365Fixed();
    fixingDays = 2;
    businessDayConvention = Unadjusted;
    settlementDate = Settings::instance().evaluationDate();
    tenors.resize(tenorsNb);
    for (Size i = 0 ; i < tenorsNb; i++)
        tenors[i] = Period(i+1, Years);
    strikes.resize(strikesNb);
    for (Size j = 0 ; j < strikesNb; j++)
        strikes[j] = double(j+1)/100;
    
    // volatilities quotes creation
    flatVolatility = .13;
    flatVolatilities.resize(tenorsNb);
    boost::shared_ptr<SimpleQuote> 
        flatVolatitlityQuote(new SimpleQuote(flatVolatility));
    for (Size i = 0 ; i < tenorsNb; i++){
        flatVolatilities[i].resize(strikesNb);
        for (Size j = 0 ; j < strikesNb; j++){
            flatVolatilities[i][j] = Handle<Quote>(flatVolatitlityQuote,true);
        }
    }
    flatForwardRate = 0.04875825;
    forwardRate = boost::shared_ptr<Quote>(new SimpleQuote(flatForwardRate));
    forwardRateQuote.linkTo(forwardRate);
    myTermStructure = boost::shared_ptr<FlatForward>(
                  new FlatForward(settlementDate, forwardRateQuote,
                                  Actual365Fixed()));
    rhTermStructure.linkTo(myTermStructure);
    index = boost::shared_ptr<Xibor>(new Euribor6M(rhTermStructure));
    capsStripper = boost::shared_ptr<CapsStripper>(new CapsStripper(  calendar, 
                                        businessDayConvention,
                                        fixingDays,
                                        tenors,
                                        strikes,
                                        flatVolatilities, 
                                        dayCounter,
                                        index,
                                        rhTermStructure));
}

void CapsStripperTest::FlatVolatilityStripping() {

    BOOST_MESSAGE("Testing Flat Volatility Stripping...");
    QL_TEST_BEGIN
    setup();
    for (Size tenorTestedIndex = 0; tenorTestedIndex < tenorsNb ; tenorTestedIndex++){
        Date tenorDate = settlementDate + tenors[tenorTestedIndex];
        Time tenorTime =  dayCounter.yearFraction(settlementDate, tenorDate);
        for (Size strikeTestedIndex = 0; strikeTestedIndex < strikesNb ; strikeTestedIndex ++){
            Real blackVariance = 
                capsStripper->blackVariance(tenorDate, strikes[strikeTestedIndex],true);
            Volatility volatility = std::sqrt(blackVariance/tenorTime);
            Real relativeError = (volatility - flatVolatility)/flatVolatility *100;
            if (std::fabs(relativeError)>1e-2)
                BOOST_ERROR(tenors[tenorTestedIndex] << "\t" <<
                            strikes[strikeTestedIndex]*100 << "%\t" << 
                            volatility << "\t" <<
                            relativeError);
        }
    }
    QL_TEST_END
}


void CapsStripperTest::constantVolatilityConsistency() {

    BOOST_MESSAGE("Testing bootstrap consistency...");
    
    QL_TEST_BEGIN
    setup();

    // we test cap prices using the resulting CapletVolatilityStructure
    Handle<CapletVolatilityStructure> capsStripperHandle(capsStripper); 
    boost::shared_ptr<PricingEngine> blackCapFloorEngineCapStripper
            (new BlackCapFloorEngine(capsStripperHandle));
    
    Real tolerance = 1e-4;
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
                    (new BlackCapFloorEngine(flatVolatilities[tenorTestedIndex][strikeTestedIndex]));
            cap->setPricingEngine(blackCapFloorEngineConstantVolatility);
            Real priceFromConstantVolatilty = cap->NPV();
            
            // we compute empirically the cap vega ... 
            Real relativeError = (priceFromStrippedVolatilty - priceFromConstantVolatilty)
                /priceFromConstantVolatilty * 100;
            if(std::fabs(relativeError) > tolerance)
                BOOST_ERROR( tenors[tenorTestedIndex] << "\t" 
                            << strikes[strikeTestedIndex]*100 << "%\t" 
                            << "stripped :" << priceFromStrippedVolatilty *1000 << "\t"
                            << "constant :" << priceFromConstantVolatilty *1000 << "\t"
                            << "rel error:" << relativeError << "%");
        }
    }
    QL_TEST_END
}


test_suite* CapsStripperTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("CapsStripper tests");
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::FlatVolatilityStripping));
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::constantVolatilityConsistency));
    return suite;
}