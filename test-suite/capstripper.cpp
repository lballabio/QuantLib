/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

#include "capstripper.hpp"
#include "utilities.hpp"
#include <ql/time/calendars/target.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/yieldtermstructures/flatforward.hpp>
#include <ql/utilities/dataparsers.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/voltermstructures/interestrate/caplet/capstripper.hpp>
#include <ql/voltermstructures/interpolatedsmilesection.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/voltermstructures/interestrate/caplet/spreadedcapletvolstructure.hpp>

#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(CapsStripperTest)

Calendar calendar;
DayCounter dayCounter;
std::vector<Rate> strikes;
std::vector<Period> tenors;
std::vector<std::vector<Handle<Quote> > > volatilityQuoteHandle;
boost::shared_ptr<FlatForward> myTermStructure;
RelinkableHandle<YieldTermStructure> rhTermStructure;
boost::shared_ptr<IborIndex> xiborIndex;
Natural fixingDays;
BusinessDayConvention businessDayConvention;
boost::shared_ptr<CapsStripper> capsStripper;
Matrix v;

Real maxAbs(const Matrix& m){
    Real max = QL_MIN_REAL;
    for (Size i=0; i<m.rows(); i++) {
        for (Size j=0; j<m.columns(); j++)
            max = std::max(std::fabs(m[i][j]), max);
    }
    return max;
}


// set a Flat Volatility Term Structure at a given level
void setFlatVolatilityTermStructure(Volatility flatVolatility){
    dayCounter = Actual360();
    strikes.resize(10);
    tenors.resize(10);
    tenors.resize(tenors.size());
    for (Size i = 0 ; i < tenors.size(); i++)
        tenors[i] = Period(i+1, Years);
    strikes.resize(strikes.size());
    for (Size j = 0 ; j < strikes.size(); j++)
        strikes[j] = double(j+1)/100;

    volatilityQuoteHandle.resize(tenors.size());
    boost::shared_ptr<SimpleQuote>
        volatilityQuote(new SimpleQuote(flatVolatility));
    for (Size i = 0 ; i < tenors.size(); i++){
        volatilityQuoteHandle[i].resize(strikes.size());
        for (Size j = 0 ; j < strikes.size(); j++){
            volatilityQuoteHandle[i][j] = Handle<Quote>(volatilityQuote,true);
        }
    }

}

void setMarketVolatilityTermStructure(){

    dayCounter = Actual360();
    strikes.resize(13);
    tenors.resize(16);
    v = Matrix(tenors.size(), strikes.size());
    v[0][0]=0.287;  v[0][1]=0.274;  v[0][2]=0.256;  v[0][3]=0.245;  v[0][4]=0.227;  v[0][5]=0.148;  v[0][6]=0.096;  v[0][7]=0.09;   v[0][8]=0.11;   v[0][9]=0.139;  v[0][10]=0.166; v[0][11]=0.19;  v[0][12]=0.214;
    v[1][0]=0.303;  v[1][1]=0.258;  v[1][2]=0.22;   v[1][3]=0.203;  v[1][4]=0.19;   v[1][5]=0.153;  v[1][6]=0.126;  v[1][7]=0.118;  v[1][8]=0.147;  v[1][9]=0.165;  v[1][10]=0.18;  v[1][11]=0.192; v[1][12]=0.212;
    v[2][0]=0.303;  v[2][1]=0.257;  v[2][2]=0.216;  v[2][3]=0.196;  v[2][4]=0.182;  v[2][5]=0.154;  v[2][6]=0.134;  v[2][7]=0.127;  v[2][8]=0.149;  v[2][9]=0.166;  v[2][10]=0.18;  v[2][11]=0.192; v[2][12]=0.212;
    v[3][0]=0.305;  v[3][1]=0.266;  v[3][2]=0.226;  v[3][3]=0.203;  v[3][4]=0.19;   v[3][5]=0.167;  v[3][6]=0.151;  v[3][7]=0.144;  v[3][8]=0.16;   v[3][9]=0.172;  v[3][10]=0.183; v[3][11]=0.193; v[3][12]=0.209;
    v[4][0]=0.294;  v[4][1]=0.261;  v[4][2]=0.216;  v[4][3]=0.201;  v[4][4]=0.19;   v[4][5]=0.171;  v[4][6]=0.158;  v[4][7]=0.151;  v[4][8]=0.163;  v[4][9]=0.172;  v[4][10]=0.181; v[4][11]=0.188; v[4][12]=0.201;
    v[5][0]=0.276;  v[5][1]=0.248;  v[5][2]=0.212;  v[5][3]=0.199;  v[5][4]=0.189;  v[5][5]=0.172;  v[5][6]=0.16;   v[5][7]=0.155;  v[5][8]=0.162;  v[5][9]=0.17;   v[5][10]=0.177; v[5][11]=0.183; v[5][12]=0.195;
    v[6][0]=0.26;   v[6][1]=0.237;  v[6][2]=0.21;   v[6][3]=0.198;  v[6][4]=0.188;  v[6][5]=0.172;  v[6][6]=0.161;  v[6][7]=0.156;  v[6][8]=0.161;  v[6][9]=0.167;  v[6][10]=0.173; v[6][11]=0.179; v[6][12]=0.19;
    v[7][0]=0.25;   v[7][1]=0.231;  v[7][2]=0.208;  v[7][3]=0.196;  v[7][4]=0.187;  v[7][5]=0.172;  v[7][6]=0.162;  v[7][7]=0.156;  v[7][8]=0.16;   v[7][9]=0.165;  v[7][10]=0.17;  v[7][11]=0.175; v[7][12]=0.185;
    v[8][0]=0.244;  v[8][1]=0.226;  v[8][2]=0.206;  v[8][3]=0.195;  v[8][4]=0.186;  v[8][5]=0.171;  v[8][6]=0.161;  v[8][7]=0.156;  v[8][8]=0.158;  v[8][9]=0.162;  v[8][10]=0.166; v[8][11]=0.171; v[8][12]=0.18;
    v[9][0]=0.239;  v[9][1]=0.222;  v[9][2]=0.204;  v[9][3]=0.193;  v[9][4]=0.185;  v[9][5]=0.17;   v[9][6]=0.16;   v[9][7]=0.155;  v[9][8]=0.156;  v[9][9]=0.159;  v[9][10]=0.163; v[9][11]=0.168; v[9][12]=0.177;
    v[10][0]=0.235; v[10][1]=0.219; v[10][2]=0.202; v[10][3]=0.192; v[10][4]=0.183; v[10][5]=0.169; v[10][6]=0.159; v[10][7]=0.154; v[10][8]=0.154; v[10][9]=0.156; v[10][10]=0.16; v[10][11]=0.164;    v[10][12]=0.173;
    v[11][0]=0.227; v[11][1]=0.212; v[11][2]=0.197; v[11][3]=0.187; v[11][4]=0.179; v[11][5]=0.166; v[11][6]=0.156; v[11][7]=0.151; v[11][8]=0.149; v[11][9]=0.15;  v[11][10]=0.153;    v[11][11]=0.157;    v[11][12]=0.165;
    v[12][0]=0.22;  v[12][1]=0.206; v[12][2]=0.192; v[12][3]=0.183; v[12][4]=0.175; v[12][5]=0.162; v[12][6]=0.153; v[12][7]=0.147; v[12][8]=0.144; v[12][9]=0.144; v[12][10]=0.147;    v[12][11]=0.151;    v[12][12]=0.158;
    v[13][0]=0.211; v[13][1]=0.197; v[13][2]=0.185; v[13][3]=0.176; v[13][4]=0.168; v[13][5]=0.156; v[13][6]=0.147; v[13][7]=0.142; v[13][8]=0.138; v[13][9]=0.138; v[13][10]=0.14; v[13][11]=0.144;    v[13][12]=0.151;
    v[14][0]=0.204; v[14][1]=0.192; v[14][2]=0.18;  v[14][3]=0.171; v[14][4]=0.164; v[14][5]=0.152; v[14][6]=0.143; v[14][7]=0.138; v[14][8]=0.134; v[14][9]=0.134; v[14][10]=0.137;    v[14][11]=0.14; v[14][12]=0.148;
    v[15][0]=0.2;   v[15][1]=0.187; v[15][2]=0.176; v[15][3]=0.167; v[15][4]=0.16;  v[15][5]=0.148; v[15][6]=0.14;  v[15][7]=0.135; v[15][8]=0.131; v[15][9]=0.132; v[15][10]=0.135;    v[15][11]=0.139;    v[15][12]=0.146;


    volatilityQuoteHandle.resize(tenors.size());
    for (Size i = 0 ; i < tenors.size(); i++){
        volatilityQuoteHandle[i].resize(strikes.size());
        for (Size j = 0 ; j < strikes.size(); j++){
            boost::shared_ptr<SimpleQuote>
                volatilityQuote(new SimpleQuote(v[i][j]));
            volatilityQuoteHandle[i][j] = Handle<Quote>(volatilityQuote,true);
        }

    }

    strikes.resize(strikes.size());
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

    tenors.resize(tenors.size());
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

    void initializeSmileSections (
        std::vector<boost::shared_ptr<SmileSection> >& smileSections) {
        smileSections.resize(4);
        std::vector<Rate> futureOptionStrikes(33);
        std::vector<Volatility> volatilities(33);
        Date computationDate = Settings::instance().evaluationDate();
        Real dummyAtmLevel = .0;
        futureOptionStrikes[0]=0.015;   futureOptionStrikes[1]=0.01625; futureOptionStrikes[2]=0.0175;  futureOptionStrikes[3]=0.01875; futureOptionStrikes[4]=0.02;    futureOptionStrikes[5]=0.02125; futureOptionStrikes[6]=0.0225;  futureOptionStrikes[7]=0.02375; futureOptionStrikes[8]=0.025;   futureOptionStrikes[9]=0.02625; futureOptionStrikes[10]=0.0275; futureOptionStrikes[11]=0.0287500000000001; futureOptionStrikes[12]=0.03;   futureOptionStrikes[13]=0.03125;    futureOptionStrikes[14]=0.0325; futureOptionStrikes[15]=0.0337499999999999; futureOptionStrikes[16]=0.035;  futureOptionStrikes[17]=0.03625;    futureOptionStrikes[18]=0.0375; futureOptionStrikes[19]=0.03875;    futureOptionStrikes[20]=0.04;   futureOptionStrikes[21]=0.04125;    futureOptionStrikes[22]=0.0425; futureOptionStrikes[23]=0.04375;    futureOptionStrikes[24]=0.045;  futureOptionStrikes[25]=0.04625;    futureOptionStrikes[26]=0.0475; futureOptionStrikes[27]=0.04875;    futureOptionStrikes[28]=0.05;   futureOptionStrikes[29]=0.05125;    futureOptionStrikes[30]=0.0525; futureOptionStrikes[31]=0.05375;    futureOptionStrikes[32]=0.055;
        volatilities[0]=0.78;   volatilities[1]=0.71;   volatilities[2]=0.65;   volatilities[3]=0.59;   volatilities[4]=0.54;   volatilities[5]=0.49;   volatilities[6]=0.44;   volatilities[7]=0.40;   volatilities[8]=0.36;   volatilities[9]=0.32;   volatilities[10]=0.28;  volatilities[11]=0.24;  volatilities[12]=0.20;  volatilities[13]=0.17;  volatilities[14]=0.15;  volatilities[15]=0.12;  volatilities[16]=0.09;  volatilities[17]=0.06;  volatilities[18]=0.05;  volatilities[19]=0.02;  volatilities[20]=0.03;  volatilities[21]=0.12;  volatilities[22]=0.15;  volatilities[23]=0.17;  volatilities[24]=0.20;  volatilities[25]=0.22;  volatilities[26]=0.24;  volatilities[27]=0.26;  volatilities[28]=0.28;  volatilities[29]=0.30;  volatilities[30]=0.32;  volatilities[31]=0.34;  volatilities[32]=0.36;
        smileSections[0] = boost::shared_ptr<InterpolatedSmileSection<> >
            (new InterpolatedSmileSection<>(Date(computationDate + 15), futureOptionStrikes, volatilities, dummyAtmLevel, dayCounter));
        volatilities[0]=0.53;   volatilities[1]=0.48;   volatilities[2]=0.44;   volatilities[3]=0.41;   volatilities[4]=0.37;   volatilities[5]=0.34;   volatilities[6]=0.31;   volatilities[7]=0.28;   volatilities[8]=0.25;   volatilities[9]=0.22;   volatilities[10]=0.20;  volatilities[11]=0.17;  volatilities[12]=0.15;  volatilities[13]=0.12;  volatilities[14]=0.10;  volatilities[15]=0.10;  volatilities[16]=0.09;  volatilities[17]=0.08;  volatilities[18]=0.08;  volatilities[19]=0.08;  volatilities[20]=0.07;  volatilities[21]=0.07;  volatilities[22]=0.10;  volatilities[23]=0.12;  volatilities[24]=0.12;  volatilities[25]=0.13;  volatilities[26]=0.15;  volatilities[27]=0.16;  volatilities[28]=0.18;  volatilities[29]=0.19;  volatilities[30]=0.20;  volatilities[31]=0.21;  volatilities[32]=0.23;
        smileSections[1] = boost::shared_ptr<InterpolatedSmileSection<> >
            (new InterpolatedSmileSection<>(Date(computationDate + 115), futureOptionStrikes, volatilities, dummyAtmLevel, dayCounter));
        volatilities[0]=0.42;   volatilities[1]=0.39;   volatilities[2]=0.36;   volatilities[3]=0.33;   volatilities[4]=0.30;   volatilities[5]=0.27;   volatilities[6]=0.25;   volatilities[7]=0.22;   volatilities[8]=0.20;   volatilities[9]=0.18;   volatilities[10]=0.16;  volatilities[11]=0.14;  volatilities[12]=0.12;  volatilities[13]=0.12;  volatilities[14]=0.13;  volatilities[15]=0.12;  volatilities[16]=0.12;  volatilities[17]=0.11;  volatilities[18]=0.11;  volatilities[19]=0.11;  volatilities[20]=0.11;  volatilities[21]=0.10;  volatilities[22]=0.10;  volatilities[23]=0.10;  volatilities[24]=0.10;  volatilities[25]=0.11;  volatilities[26]=0.14;  volatilities[27]=0.13;  volatilities[28]=0.14;  volatilities[29]=0.15;  volatilities[30]=0.16;  volatilities[31]=0.17;  volatilities[32]=0.18;
        smileSections[2] = boost::shared_ptr<InterpolatedSmileSection<> >
            (new InterpolatedSmileSection<>(Date(computationDate + 195),  futureOptionStrikes, volatilities, dummyAtmLevel, dayCounter));
        volatilities[0]=0.36;   volatilities[1]=0.33;   volatilities[2]=0.30;   volatilities[3]=0.28;   volatilities[4]=0.25;   volatilities[5]=0.23;   volatilities[6]=0.21;   volatilities[7]=0.19;   volatilities[8]=0.17;   volatilities[9]=0.15;   volatilities[10]=0.16;  volatilities[11]=0.15;  volatilities[12]=0.15;  volatilities[13]=0.15;  volatilities[14]=0.14;  volatilities[15]=0.14;  volatilities[16]=0.14;  volatilities[17]=0.13;  volatilities[18]=0.13;  volatilities[19]=0.13;  volatilities[20]=0.13;  volatilities[21]=0.13;  volatilities[22]=0.12;  volatilities[23]=0.13;  volatilities[24]=0.12;  volatilities[25]=0.12;  volatilities[26]=0.13;  volatilities[27]=0.12;  volatilities[28]=0.13;  volatilities[29]=0.14;  volatilities[30]=0.14;  volatilities[31]=0.15;  volatilities[32]=0.16;
        smileSections[3] = boost::shared_ptr<InterpolatedSmileSection<> >
            (new InterpolatedSmileSection<>(Date(computationDate + 285), futureOptionStrikes, volatilities, dummyAtmLevel, dayCounter));
    }

void setup(std::vector<boost::shared_ptr<SmileSection> >& smileSections,
           Real impliedVolatilityPrecision = 1e-5) {

    calendar = TARGET();
    fixingDays = 2;
    businessDayConvention = Unadjusted;
    Natural settlementDays = 2;
    Rate flatForwardRate = 0.04;
    Integer maxIterations = 100;
    boost::shared_ptr<SimpleQuote> forwardRate(new SimpleQuote);
    Handle<Quote> forwardRateQuote(forwardRate);
    myTermStructure = boost::shared_ptr<FlatForward>(
                  new FlatForward(settlementDays, calendar, forwardRateQuote,
                                  dayCounter));
    rhTermStructure.linkTo(myTermStructure);

    xiborIndex = boost::shared_ptr<IborIndex>(new Euribor6M(rhTermStructure));
    capsStripper = boost::shared_ptr<CapsStripper>(new CapsStripper(tenors,
                                                strikes,
                                                volatilityQuoteHandle,
                                                xiborIndex,
                                                rhTermStructure,
                                                dayCounter,
                                                impliedVolatilityPrecision,
                                                maxIterations,
                                                smileSections));
    forwardRate->setValue(flatForwardRate);
}

QL_END_TEST_LOCALS(CapsStripperTest)

/* We strip a flat volatility surface and we check if
    the result is equal to the initial surface
*/
void CapsStripperTest::FlatVolatilityStripping() {

    BOOST_MESSAGE("Testing flat-volatility stripping...");

    SavedSettings backup;

    Date today = TARGET().adjust(Settings::instance().evaluationDate());
    Settings::instance().evaluationDate() = today;

    Volatility flatVolatility = .18;
    setFlatVolatilityTermStructure(flatVolatility);
    std::vector<boost::shared_ptr<SmileSection> > smileSections;
    setup(smileSections);
    const CapMatrix& marketDataCap = capsStripper->marketDataCap();
    for (Size tenorIndex = 0; tenorIndex < tenors.size() ; tenorIndex++){
        Date tenorDate = marketDataCap[tenorIndex][0]->lastFixingDate();
        Time tenorTime =  dayCounter.yearFraction(
            Settings::instance().evaluationDate(), tenorDate);
        for (Size strikeIndex = 0; strikeIndex < strikes.size() ; strikeIndex ++){
            Real blackVariance =
                capsStripper->blackVariance(tenorDate, strikes[strikeIndex],true);
            Volatility volatility = std::sqrt(blackVariance/tenorTime);
            Real relativeError = (volatility - flatVolatility)/flatVolatility *100;

            if (std::fabs(relativeError)>1e-2)
                BOOST_ERROR(   "tenor:\t" << tenors[tenorIndex]
                            << "\nstrike:\t" << strikes[strikeIndex]*100 << "%"
                            << "\nvolatility=:\t" << volatility
                            << "\nrelativeError=:\t" << relativeError
                            << "\n-------------\n");
        }
    }
}





/* High precision consistency test*/

void CapsStripperTest::highPrecisionTest(){
    BOOST_MESSAGE("Testing consistency of cap volatilities...");

    SavedSettings backup;

    Date today = TARGET().adjust(Settings::instance().evaluationDate());
    Settings::instance().evaluationDate() = today;

    setMarketVolatilityTermStructure();
    std::vector<boost::shared_ptr<SmileSection> > smileSections;
    Real impliedVolatilityPrecision = 1e-20;
    setup(smileSections, impliedVolatilityPrecision);
    static const Real tolerance = 1e-12;
    static const Real priceThreshold = 1e-6;

    Handle <OptionletVolatilityStructure> strippedVolatilityStructureHandle(capsStripper);
    boost::shared_ptr<BlackCapFloorEngine> strippedVolatilityBlackCapFloorEngine
        (new BlackCapFloorEngine(strippedVolatilityStructureHandle));
    for (Size tenorIndex = 0; tenorIndex < tenors.size() ; tenorIndex++){
        for (Size strikeIndex = 0; strikeIndex < strikes.size() ; strikeIndex ++){
            boost::shared_ptr<CapFloor> cap = MakeCapFloor(CapFloor::Cap,
                tenors[tenorIndex], xiborIndex, strikes[strikeIndex],
                0*Days, strippedVolatilityBlackCapFloorEngine);
            Real priceFromStrippedVolatilty = cap->NPV();
            boost::shared_ptr<PricingEngine> blackCapFloorEngineConstantVolatility(
                new BlackCapFloorEngine(
                    volatilityQuoteHandle[tenorIndex][strikeIndex], dayCounter));
            cap->setPricingEngine(blackCapFloorEngineConstantVolatility);
            Real priceFromConstantVolatilty = cap->NPV();
            Real absError = std::fabs(priceFromStrippedVolatilty - priceFromConstantVolatilty);
            Real relativeError = absError/priceFromConstantVolatilty;
            bool strippedPriceIsAccurate;
            // if we test a short maturity the tolerance is increased because the
            // vega might be too low so that we haven't stripped the volatility
            if (tenorIndex <=1)
                strippedPriceIsAccurate = relativeError < tolerance *1e2;
            else
                strippedPriceIsAccurate = relativeError < tolerance;
            // if prices are too low the relative discrepancy is not relevant
            bool priceIsBigEnough = priceFromConstantVolatilty > priceThreshold;

            if(!strippedPriceIsAccurate && priceIsBigEnough)
                BOOST_FAIL("\ntenor: " << tenors[tenorIndex] <<
                           "\nstrike: " << io::rate(strikes[strikeIndex]) <<
                           "\nstripped: " << priceFromStrippedVolatilty * 1e4 <<
                           "\nconstant: " << priceFromConstantVolatilty * 1e4 <<
                           "\nabs error: " << QL_SCIENTIFIC << absError << "\n"
                           "\nrel error: " << io::percent(relativeError) << "\n");
         }
    }
}

/* Spreaded volatility stripper test*/

void CapsStripperTest::testSpreadedStripper() {

    BOOST_MESSAGE("Testing spreaded caplet volatility stripper...");

    SavedSettings backup;

    Date today = TARGET().adjust(Settings::instance().evaluationDate());
    Settings::instance().evaluationDate() = today;

    setMarketVolatilityTermStructure();
    std::vector<boost::shared_ptr<SmileSection> > smileSections;
    setup(smileSections);

    Handle <OptionletVolatilityStructure> strippedVolatilityStructureHandle(capsStripper);
    boost::shared_ptr<SimpleQuote> spread (new SimpleQuote(0.0001));
    Handle<Quote> spreadHandle(spread);
    boost::shared_ptr<OptionletVolatilityStructure> spreadedStripper(
        new SpreadedCapletVolatilityStructure
        (strippedVolatilityStructureHandle, spreadHandle));
    std::vector<Real> strikes;
    for (Size k=1; k<100; k++)
        strikes.push_back(k*.01);
    for (Size i=0; i<tenors.size(); i++) {
        for (Size k=0; k<strikes.size(); k++) {
            Real strike = strikes[k];
            Real diff = spreadedStripper->volatility(tenors[i], strike)
                        - strippedVolatilityStructureHandle->volatility(tenors[i], strike);
            if (fabs(diff-spread->value())>1e-16)
                BOOST_ERROR("\ndiff!=spread in volatility method:"
                            "\nexpiry time = " << tenors[i] <<
                            "\n atm strike = " << io::rate(strike) <<
                            "\ndiff = " << diff <<
                            "\nspread = " << spread->value());
        }
    }
    //testing observability
    Flag f;
    f.registerWith(spreadedStripper);
    strippedVolatilityStructureHandle->update();
    if(!f.isUp())
        BOOST_ERROR("spreadedCapletVolatilityStructure "
                    << "does not propagate notifications");
    f.lower();
    spread->setValue(.001);
    if(!f.isUp())
        BOOST_ERROR("spreadedCapletVolatilityStructure "
                    << "does not propagate notifications");
}


test_suite* CapsStripperTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("CapsStripper tests");
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::FlatVolatilityStripping));
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::highPrecisionTest));
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::testSpreadedStripper));
    return suite;
}
