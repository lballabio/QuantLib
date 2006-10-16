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
#include <ql/utilities/dataparsers.hpp>
#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(CapsStripperTest)


Calendar calendar;
DayCounter dayCounter;
Size strikesNb, tenorsNb;
std::vector<Rate> strikes;
std::vector<Period> tenors;
std::vector<std::vector<Handle<Quote> > > volatilityQuoteHandle;
boost::shared_ptr<FlatForward> myTermStructure;
boost::shared_ptr<Quote> forwardRate;
Handle<Quote> forwardRateQuote;
Handle<YieldTermStructure> rhTermStructure;
Date settlementDate;
boost::shared_ptr<Xibor> xiborIndex;
int fixingDays;
BusinessDayConvention businessDayConvention;
boost::shared_ptr<CapsStripper> capsStripper;
Volatility flatVolatility;
Rate flatForwardRate;
QL_END_TEST_LOCALS(CapsStripperTest)


void setFlatVolatilityTermStructure(){
    strikesNb = 10;
    tenorsNb = 10;
    tenors.resize(tenorsNb);
    for (Size i = 0 ; i < tenorsNb; i++)
        tenors[i] = Period(i+1, Years);
    strikes.resize(strikesNb);
    for (Size j = 0 ; j < strikesNb; j++)
        strikes[j] = double(j+1)/100;

    flatVolatility = .13;
    volatilityQuoteHandle.resize(tenorsNb);
    boost::shared_ptr<SimpleQuote> 
        volatilityQuote(new SimpleQuote(flatVolatility));
    for (Size i = 0 ; i < tenorsNb; i++){
        volatilityQuoteHandle[i].resize(strikesNb);
        for (Size j = 0 ; j < strikesNb; j++){
            volatilityQuoteHandle[i][j] = Handle<Quote>(volatilityQuote,true);
        }
    }

}

void setMarketVolatilityTermStructure(){
    strikesNb = 13;
    tenorsNb = 16;
    Matrix v(tenorsNb, strikesNb);
    v[0][0]=0.287;	v[0][1]=0.274;	v[0][2]=0.256;	v[0][3]=0.245;	v[0][4]=0.227;	v[0][5]=0.148;	v[0][6]=0.096;	v[0][7]=0.09;	v[0][8]=0.11;	v[0][9]=0.139;	v[0][10]=0.166;	v[0][11]=0.19;	v[0][12]=0.214;
    v[1][0]=0.303;	v[1][1]=0.258;	v[1][2]=0.22;	v[1][3]=0.203;	v[1][4]=0.19;	v[1][5]=0.153;	v[1][6]=0.126;	v[1][7]=0.118;	v[1][8]=0.147;	v[1][9]=0.165;	v[1][10]=0.18;	v[1][11]=0.192;	v[1][12]=0.212;
    v[2][0]=0.303;	v[2][1]=0.257;	v[2][2]=0.216;	v[2][3]=0.196;	v[2][4]=0.182;	v[2][5]=0.154;	v[2][6]=0.134;	v[2][7]=0.127;	v[2][8]=0.149;	v[2][9]=0.166;	v[2][10]=0.18;	v[2][11]=0.192;	v[2][12]=0.212;
    v[3][0]=0.305;	v[3][1]=0.266;	v[3][2]=0.226;	v[3][3]=0.203;	v[3][4]=0.19;	v[3][5]=0.167;	v[3][6]=0.151;	v[3][7]=0.144;	v[3][8]=0.16;	v[3][9]=0.172;	v[3][10]=0.183;	v[3][11]=0.193;	v[3][12]=0.209;
    v[4][0]=0.294;	v[4][1]=0.261;	v[4][2]=0.216;	v[4][3]=0.201;	v[4][4]=0.19;	v[4][5]=0.171;	v[4][6]=0.158;	v[4][7]=0.151;	v[4][8]=0.163;	v[4][9]=0.172;	v[4][10]=0.181;	v[4][11]=0.188;	v[4][12]=0.201;
    v[5][0]=0.276;	v[5][1]=0.248;	v[5][2]=0.212;	v[5][3]=0.199;	v[5][4]=0.189;	v[5][5]=0.172;	v[5][6]=0.16;	v[5][7]=0.155;	v[5][8]=0.162;	v[5][9]=0.17;	v[5][10]=0.177;	v[5][11]=0.183;	v[5][12]=0.195;
    v[6][0]=0.26;	v[6][1]=0.237;	v[6][2]=0.21;	v[6][3]=0.198;	v[6][4]=0.188;	v[6][5]=0.172;	v[6][6]=0.161;	v[6][7]=0.156;	v[6][8]=0.161;	v[6][9]=0.167;	v[6][10]=0.173;	v[6][11]=0.179;	v[6][12]=0.19;
    v[7][0]=0.25;	v[7][1]=0.231;	v[7][2]=0.208;	v[7][3]=0.196;	v[7][4]=0.187;	v[7][5]=0.172;	v[7][6]=0.162;	v[7][7]=0.156;	v[7][8]=0.16;	v[7][9]=0.165;	v[7][10]=0.17;	v[7][11]=0.175;	v[7][12]=0.185;
    v[8][0]=0.244;	v[8][1]=0.226;	v[8][2]=0.206;	v[8][3]=0.195;	v[8][4]=0.186;	v[8][5]=0.171;	v[8][6]=0.161;	v[8][7]=0.156;	v[8][8]=0.158;	v[8][9]=0.162;	v[8][10]=0.166;	v[8][11]=0.171;	v[8][12]=0.18;
    v[9][0]=0.239;	v[9][1]=0.222;	v[9][2]=0.204;	v[9][3]=0.193;	v[9][4]=0.185;	v[9][5]=0.17;	v[9][6]=0.16;	v[9][7]=0.155;	v[9][8]=0.156;	v[9][9]=0.159;	v[9][10]=0.163;	v[9][11]=0.168;	v[9][12]=0.177;
    v[10][0]=0.235;	v[10][1]=0.219;	v[10][2]=0.202;	v[10][3]=0.192;	v[10][4]=0.183;	v[10][5]=0.169;	v[10][6]=0.159;	v[10][7]=0.154;	v[10][8]=0.154;	v[10][9]=0.156;	v[10][10]=0.16;	v[10][11]=0.164;	v[10][12]=0.173;
    v[11][0]=0.227;	v[11][1]=0.212;	v[11][2]=0.197;	v[11][3]=0.187;	v[11][4]=0.179;	v[11][5]=0.166;	v[11][6]=0.156;	v[11][7]=0.151;	v[11][8]=0.149;	v[11][9]=0.15;	v[11][10]=0.153;	v[11][11]=0.157;	v[11][12]=0.165;
    v[12][0]=0.22;	v[12][1]=0.206;	v[12][2]=0.192;	v[12][3]=0.183;	v[12][4]=0.175;	v[12][5]=0.162;	v[12][6]=0.153;	v[12][7]=0.147;	v[12][8]=0.144;	v[12][9]=0.144;	v[12][10]=0.147;	v[12][11]=0.151;	v[12][12]=0.158;
    v[13][0]=0.211;	v[13][1]=0.197;	v[13][2]=0.185;	v[13][3]=0.176;	v[13][4]=0.168;	v[13][5]=0.156;	v[13][6]=0.147;	v[13][7]=0.142;	v[13][8]=0.138;	v[13][9]=0.138;	v[13][10]=0.14;	v[13][11]=0.144;	v[13][12]=0.151;
    v[14][0]=0.204;	v[14][1]=0.192;	v[14][2]=0.18;	v[14][3]=0.171;	v[14][4]=0.164;	v[14][5]=0.152;	v[14][6]=0.143;	v[14][7]=0.138;	v[14][8]=0.134;	v[14][9]=0.134;	v[14][10]=0.137;	v[14][11]=0.14;	v[14][12]=0.148;
    v[15][0]=0.2;	v[15][1]=0.187;	v[15][2]=0.176;	v[15][3]=0.167;	v[15][4]=0.16;	v[15][5]=0.148;	v[15][6]=0.14;	v[15][7]=0.135;	v[15][8]=0.131;	v[15][9]=0.132;	v[15][10]=0.135;	v[15][11]=0.139;	v[15][12]=0.146;

  
    volatilityQuoteHandle.resize(tenorsNb);
    for (Size i = 0 ; i < tenorsNb; i++){
        volatilityQuoteHandle[i].resize(strikesNb);
        for (Size j = 0 ; j < strikesNb; j++){
            boost::shared_ptr<SimpleQuote> 
                volatilityQuote(new SimpleQuote(v[i][j]));
            volatilityQuoteHandle[i][j] = Handle<Quote>(volatilityQuote,true);
        }
        
    }

    strikes.resize(strikesNb);
    strikes[0]=0.015;
    strikes[1]=0.0175;
    strikes[2]=0.02;
    strikes[3]=0.0225;
    strikes[4]=0.025;
    strikes[5]=0.03;
    strikes[6]=0.035;
    strikes[7]=0.04;
    strikes[8]=0.05;
    strikes[9]=0.06;
    strikes[10]=0.07;
    strikes[11]=0.08;
    strikes[12]=0.1;

    tenors.resize(tenorsNb);
    tenors[0]= PeriodParser::parse("1Y");
    tenors[1]= PeriodParser::parse("18M");
    tenors[2]= PeriodParser::parse("2Y");
    tenors[3]= PeriodParser::parse("3Y");
    tenors[4]= PeriodParser::parse("4Y");
    tenors[5]= PeriodParser::parse("5Y");
    tenors[6]= PeriodParser::parse("6Y");
    tenors[7]= PeriodParser::parse("7Y");
    tenors[8]= PeriodParser::parse("8Y");
    tenors[9]= PeriodParser::parse("9Y");
    tenors[10]= PeriodParser::parse("10Y");
    tenors[11]= PeriodParser::parse("12Y");
    tenors[12]= PeriodParser::parse("15Y");
    tenors[13]= PeriodParser::parse("20Y");
    tenors[14]= PeriodParser::parse("25Y");
    tenors[15]= PeriodParser::parse("30Y");
}


void setup() {

    calendar = TARGET();
    dayCounter = Actual365Fixed();
    fixingDays = 2;
    businessDayConvention = Unadjusted;
    settlementDate = Settings::instance().evaluationDate();
    
    flatForwardRate = 0.03875825;
    forwardRate = boost::shared_ptr<Quote>(new SimpleQuote(flatForwardRate));
    forwardRateQuote.linkTo(forwardRate);
    myTermStructure = boost::shared_ptr<FlatForward>(
                  new FlatForward(settlementDate, forwardRateQuote,
                                  Actual365Fixed()));
    rhTermStructure.linkTo(myTermStructure);
    xiborIndex = boost::shared_ptr<Xibor>(new Euribor6M(rhTermStructure));
    capsStripper = boost::shared_ptr<CapsStripper>(new CapsStripper(  calendar, 
                                        businessDayConvention,
                                        fixingDays,
                                        tenors,
                                        strikes,
                                        volatilityQuoteHandle, 
                                        dayCounter,
                                        xiborIndex,
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
        LegHelper legHelper(settlementDate, calendar, fixingDays, businessDayConvention, 
                            xiborIndex);
        FloatingLeg floatingLeg = legHelper.makeLeg(xiborIndex->tenor(), 
                                                    tenors[tenorTestedIndex]);
       
        // for every strike we compute the price using different pricing engines    
        for (Size strikeTestedIndex = 0; strikeTestedIndex < strikesNb ; strikeTestedIndex ++){
            
            // we compute the price using the StrippedVolatilty
            boost::shared_ptr<CapFloor> cap(new Cap(floatingLeg,
                                            std::vector<Real>(1,strikes[strikeTestedIndex]),
                                            rhTermStructure, blackCapFloorEngineCapStripper));
            Real priceFromStrippedVolatilty = cap->NPV();
            
            // then using the original constant volatility structure
            boost::shared_ptr<PricingEngine> blackCapFloorEngineConstantVolatility
                    (new BlackCapFloorEngine(volatilityQuoteHandle[tenorTestedIndex][strikeTestedIndex]));
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

void CapsStripperTest::cachedValues(){
    setMarketVolatilityTermStructure();
    setup();

    Matrix strippedVolatilities(tenorsNb,strikesNb);
    Matrix marketDataPrices(tenorsNb,strikesNb);
    Matrix vega(tenorsNb,strikesNb);
    for (Size tenorTestedIndex = 0; tenorTestedIndex < tenorsNb ; tenorTestedIndex++){
         Date tenorDate = calendar.advance(settlementDate, tenors[tenorTestedIndex]);
         Time tenorTime = dayCounter.yearFraction(settlementDate, tenorDate);
         for (Size strikeTestedIndex = 0; strikeTestedIndex < strikesNb ; strikeTestedIndex ++){
            CapFloor& mktCap = *capsStripper->marketDataCap_[tenorTestedIndex][strikeTestedIndex];
            marketDataPrices[tenorTestedIndex][strikeTestedIndex] = mktCap.NPV()*1e4;
             vega[tenorTestedIndex][strikeTestedIndex] =  mktCap.vega()*1e4;
            Real blackVariance = capsStripper->blackVariance(
                tenorDate, strikes[strikeTestedIndex],true);
            strippedVolatilities[tenorTestedIndex][strikeTestedIndex] 
                = std::sqrt(blackVariance/tenorTime)*100;
                
         }
    }

    std::cout << strippedVolatilities << std::endl;
}


test_suite* CapsStripperTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("CapsStripper tests");
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::FlatVolatilityStripping));
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::constantVolatilityConsistency));
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::cachedValues));
    return suite;
}
