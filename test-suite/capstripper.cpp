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
    strikesNb = 10;
    tenorsNb = 10;
    calendar = TARGET();
    dayCounter = Actual360();
    fixingDays = 2;
    businessDayConvention = Unadjusted;
    settlementDate = Settings::instance().evaluationDate();
    for (Size i = 0 ; i < tenorsNb; i++)
        tenors.push_back(Period(i+1, Years));
    for (Size j = 0 ; j < strikesNb; j++)
        strikes.push_back(double(j)/100);
    

    // volatilities quotes creation
    vols.resize(tenorsNb);
    for (Size i = 0 ; i < tenorsNb; i++){
        vols[i].resize(strikesNb);
        for (Size j = 0 ; j < strikesNb; j++){
            vols[i][j] = Handle<Quote>(boost::shared_ptr<SimpleQuote>
                (new SimpleQuote(.1)),true);
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
    CapsStripper 
        CapsStripper(calendar, 
                                        businessDayConvention,
                                        2,
                                        tenors,
                                        strikes,
                                        vols, 
                                        dayCounter,
                                        index,
                                        rhTermStructure);
    Rate test = CapsStripper.volatility(3,2.5/100);
    //std::cout << CapsStripper.marketDataPrices_ << std::endl;
    //std::cout << CapsStripper.volatilities_ << std::endl;
    QL_TEST_END
}


void CapsStripperTest::testConsistency() {

    BOOST_MESSAGE("Testing bootstrap consistency...");
    
    QL_TEST_BEGIN
    setup();
    boost::shared_ptr<CapsStripper> 
        CapsStripper(new CapsStripper(
                                        calendar, 
                                        businessDayConvention,
                                        fixingDays,
                                        tenors,
                                        strikes,
                                        vols, 
                                        dayCounter,
                                        index,
                                        rhTermStructure));
    Handle<CapletVolatilityStructure> capsStripperHandle(CapsStripper); 
    boost::shared_ptr<PricingEngine> blackCapFloorEngineCapStripper
            (new BlackCapFloorEngine(capsStripperHandle));
    // for every tenor 
    std::cout << "tenor\tstrike\tStripped Volatilty\tConstant Volatilty"<< std::endl;
    for (Size tenorTestedIndex = 0; tenorTestedIndex < tenorsNb ; tenorTestedIndex++){
        LegHelper legHelper(settlementDate, calendar, fixingDays, businessDayConvention, index);
        FloatingLeg floatingLeg = 
            legHelper.makeLeg(index->tenor(), tenors[tenorTestedIndex]);
        // for every strike     
        for (Size strikeTestedIndex = 0; strikeTestedIndex < strikesNb ; strikeTestedIndex ++){
            boost::shared_ptr<CapFloor> cap(new Cap(floatingLeg,
                                            std::vector<Real>(1,strikes[strikeTestedIndex]),
                                            rhTermStructure, blackCapFloorEngineCapStripper));
            Real priceFromStrippedVolatilty = cap->NPV();
            // then with the original constant volatility structure
            boost::shared_ptr<PricingEngine> blackCapFloorEngineConstantVolatility
                    (new BlackCapFloorEngine(vols[tenorTestedIndex][strikeTestedIndex]));
            cap->setPricingEngine(blackCapFloorEngineConstantVolatility);
            Real priceFromConstantVolatilty = cap->NPV();
            std::cout << tenors[tenorTestedIndex] << "\t" << strikes[strikeTestedIndex]*100 
                << "%\t" << priceFromStrippedVolatilty << "\t\t\t" 
                << priceFromConstantVolatilty<< std::endl;
        }
    }
    QL_TEST_END
}


//std::vector<boost::shared_ptr<CashFlow> > makeLeg(const Date& startDate,
//                                                  Integer length, 
//                                                  bool floatingLeg) {
//    Date endDate = calendar.advance(startDate,length,Years, businessDayConvention);
//    Date effectiveStartDate = calendar.advance(startDate, Period(1,Years), businessDayConvention);
//    Schedule schedule(effectiveStartDate , endDate, Period(1,Years), calendar,
//                      businessDayConvention, businessDayConvention, false, false);
//    if (floatingLeg) 
//    return FloatingRateCouponVector(schedule, businessDayConvention, std::vector<Real>(10,1),
//                                    2, index,
//                                    std::vector<Real>(),
//                                    std::vector<Spread>(),
//                                    index->dayCounter());
//    else
//    return FixedRateCouponVector(schedule, businessDayConvention, 
//                                    std::vector<Real>(10,1),
//                                    std::vector<Real>(10,.05),
//                                    index->dayCounter(),
//                                    index->dayCounter());
//}
//
//#include <ql/Instruments/swap.hpp>
//void CapsStripperTest::testHandle() {
//    std::cout << "Testing handle deep copy ..." << std::endl;
//    
//    QL_TEST_BEGIN
//    setup();
//    std::vector<boost::shared_ptr<CashFlow> > floatingLeg =
//        makeLeg(settlementDate, 5, true );
//    std::vector<boost::shared_ptr<CashFlow> > fixedLeg =
//        makeLeg(settlementDate, 5, false );
//    Swap swap(rhTermStructure, floatingLeg, fixedLeg);
//    std::cout << swap.NPV()<< std::endl;
//    boost::shared_ptr<Quote> forwardRate2(new SimpleQuote(0.04));
//    Handle<Quote> forwardRateQuote2 = forwardRateQuote; 
//    forwardRateQuote.linkTo(forwardRate2);
//    std::cout << swap.NPV()<< std::endl;
//    QL_TEST_END
//}

test_suite* CapsStripperTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("CapsStripperTest tests");
    //suite->add(BOOST_TEST_CASE(&CapsStripperTest::testConstruction));
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::testConsistency));
    //suite->add(BOOST_TEST_CASE(&CapsStripperTest::testHandle));
    return suite;
}

