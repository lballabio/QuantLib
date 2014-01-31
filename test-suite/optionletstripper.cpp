/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2007, 2008 Laurent Hoffmann

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

#include "optionletstripper.hpp"
#include "utilities.hpp"
#include <ql/termstructures/volatility/optionlet/optionletstripper1.hpp>
#include <ql/termstructures/volatility/optionlet/optionletstripper2.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionletadapter.hpp>
#include <ql/termstructures/volatility/capfloor/constantcapfloortermvol.hpp>
#include <ql/termstructures/volatility/capfloor/capfloortermvolcurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using boost::shared_ptr;

namespace {

    struct CommonVars{
        // global data
        Calendar calendar;
        DayCounter dayCounter;

        RelinkableHandle<YieldTermStructure> yieldTermStructure;

        std::vector<Rate> strikes;
        std::vector<Period> optionTenors;
        Matrix termV;
        std::vector<Rate> atmTermV;
        std::vector<Handle<Quote> > atmTermVolHandle;

        Handle<CapFloorTermVolCurve> capFloorVolCurve;
        Handle<CapFloorTermVolCurve> flatTermVolCurve;

        boost::shared_ptr<CapFloorTermVolSurface> capFloorVolSurface;
        boost::shared_ptr<CapFloorTermVolSurface> flatTermVolSurface;

        Real accuracy;
        Real tolerance;

        // cleanup
        SavedSettings backup;

        CommonVars() {
            accuracy = 1.0e-6;
            tolerance = 2.5e-5;
        }

        void setTermStructure() {

            calendar = TARGET();
            dayCounter = Actual365Fixed();

            Rate flatFwdRate = 0.04;
            yieldTermStructure.linkTo(
                boost::shared_ptr<FlatForward>(new FlatForward(0,
                                                               calendar,
                                                               flatFwdRate,
                                                               dayCounter)));
        }

        void setFlatTermVolCurve() {

          setTermStructure();

          optionTenors.resize(10);
          for (Size i = 0; i < optionTenors.size(); ++i)
              optionTenors[i] = Period(i + 1, Years);

          Volatility flatVol = .18;

          std::vector<Handle<Quote> >  curveVHandle(optionTenors.size());
          for (Size i=0; i<optionTenors.size(); ++i)
              curveVHandle[i] = Handle<Quote>(boost::shared_ptr<Quote>(new
                                                        SimpleQuote(flatVol)));

          flatTermVolCurve = Handle<CapFloorTermVolCurve>(
              shared_ptr<CapFloorTermVolCurve>(new
                  CapFloorTermVolCurve(0, calendar, Following, optionTenors,
                                       curveVHandle, dayCounter)));

        }

        void setFlatTermVolSurface() {

            setTermStructure();

            optionTenors.resize(10);
            for (Size i = 0; i < optionTenors.size(); ++i)
                optionTenors[i] = Period(i + 1, Years);

            strikes.resize(10);
            for (Size j = 0; j < strikes.size(); ++j)
                strikes[j] = Real(j + 1) / 100.0;

            Volatility flatVol = .18;
            termV = Matrix(optionTenors.size(), strikes.size(), flatVol);
            flatTermVolSurface = boost::shared_ptr<CapFloorTermVolSurface>(new
                CapFloorTermVolSurface(0, calendar, Following,
                                       optionTenors, strikes,
                                       termV, dayCounter));
        }


        void setCapFloorTermVolCurve() {

          setTermStructure();

          //atm cap volatility curve
          optionTenors = std::vector<Period>();
          optionTenors.push_back(Period(1, Years));
          optionTenors.push_back(Period(18, Months));
          optionTenors.push_back(Period(2, Years));
          optionTenors.push_back(Period(3, Years));
          optionTenors.push_back(Period(4, Years));
          optionTenors.push_back(Period(5, Years));
          optionTenors.push_back(Period(6, Years));
          optionTenors.push_back(Period(7, Years));
          optionTenors.push_back(Period(8, Years));
          optionTenors.push_back(Period(9, Years));
          optionTenors.push_back(Period(10, Years));
          optionTenors.push_back(Period(12, Years));
          optionTenors.push_back(Period(15, Years));
          optionTenors.push_back(Period(20, Years));
          optionTenors.push_back(Period(25, Years));
          optionTenors.push_back(Period(30, Years));

          //atm capfloor vols from mkt vol matrix using flat yield curve
          atmTermV = std::vector<Volatility>();
          atmTermV.push_back(0.090304);
          atmTermV.push_back(0.12180);
          atmTermV.push_back(0.13077);
          atmTermV.push_back(0.14832);
          atmTermV.push_back(0.15570);
          atmTermV.push_back(0.15816);
          atmTermV.push_back(0.15932);
          atmTermV.push_back(0.16035);
          atmTermV.push_back(0.15951);
          atmTermV.push_back(0.15855);
          atmTermV.push_back(0.15754);
          atmTermV.push_back(0.15459);
          atmTermV.push_back(0.15163);
          atmTermV.push_back(0.14575);
          atmTermV.push_back(0.14175);
          atmTermV.push_back(0.13889);
          atmTermVolHandle.resize(optionTenors.size());
          for (Size i=0; i<optionTenors.size(); ++i) {
            atmTermVolHandle[i] = Handle<Quote>(boost::shared_ptr<Quote>(new
                            SimpleQuote(atmTermV[i])));
          }

          capFloorVolCurve = Handle<CapFloorTermVolCurve>(
            shared_ptr<CapFloorTermVolCurve>(new
                CapFloorTermVolCurve(0, calendar, Following,
                                     optionTenors, atmTermVolHandle,
                                     dayCounter)));

         }

        void setCapFloorTermVolSurface() {

            setTermStructure();

            //cap volatility smile matrix
            optionTenors = std::vector<Period>();
            optionTenors.push_back(Period(1, Years));
            optionTenors.push_back(Period(18, Months));
            optionTenors.push_back(Period(2, Years));
            optionTenors.push_back(Period(3, Years));
            optionTenors.push_back(Period(4, Years));
            optionTenors.push_back(Period(5, Years));
            optionTenors.push_back(Period(6, Years));
            optionTenors.push_back(Period(7, Years));
            optionTenors.push_back(Period(8, Years));
            optionTenors.push_back(Period(9, Years));
            optionTenors.push_back(Period(10, Years));
            optionTenors.push_back(Period(12, Years));
            optionTenors.push_back(Period(15, Years));
            optionTenors.push_back(Period(20, Years));
            optionTenors.push_back(Period(25, Years));
            optionTenors.push_back(Period(30, Years));

            strikes = std::vector<Rate>();
            strikes.push_back(0.015);
            strikes.push_back(0.0175);
            strikes.push_back(0.02);
            strikes.push_back(0.0225);
            strikes.push_back(0.025);
            strikes.push_back(0.03);
            strikes.push_back(0.035);
            strikes.push_back(0.04);
            strikes.push_back(0.05);
            strikes.push_back(0.06);
            strikes.push_back(0.07);
            strikes.push_back(0.08);
            strikes.push_back(0.1);

            termV = Matrix(optionTenors.size(), strikes.size());
            termV[0][0]=0.287;  termV[0][1]=0.274;  termV[0][2]=0.256;  termV[0][3]=0.245;  termV[0][4]=0.227;  termV[0][5]=0.148;  termV[0][6]=0.096;  termV[0][7]=0.09;   termV[0][8]=0.11;   termV[0][9]=0.139;  termV[0][10]=0.166;  termV[0][11]=0.19;   termV[0][12]=0.214;
            termV[1][0]=0.303;  termV[1][1]=0.258;  termV[1][2]=0.22;   termV[1][3]=0.203;  termV[1][4]=0.19;   termV[1][5]=0.153;  termV[1][6]=0.126;  termV[1][7]=0.118;  termV[1][8]=0.147;  termV[1][9]=0.165;  termV[1][10]=0.18;   termV[1][11]=0.192;  termV[1][12]=0.212;
            termV[2][0]=0.303;  termV[2][1]=0.257;  termV[2][2]=0.216;  termV[2][3]=0.196;  termV[2][4]=0.182;  termV[2][5]=0.154;  termV[2][6]=0.134;  termV[2][7]=0.127;  termV[2][8]=0.149;  termV[2][9]=0.166;  termV[2][10]=0.18;   termV[2][11]=0.192;  termV[2][12]=0.212;
            termV[3][0]=0.305;  termV[3][1]=0.266;  termV[3][2]=0.226;  termV[3][3]=0.203;  termV[3][4]=0.19;   termV[3][5]=0.167;  termV[3][6]=0.151;  termV[3][7]=0.144;  termV[3][8]=0.16;   termV[3][9]=0.172;  termV[3][10]=0.183;  termV[3][11]=0.193;  termV[3][12]=0.209;
            termV[4][0]=0.294;  termV[4][1]=0.261;  termV[4][2]=0.216;  termV[4][3]=0.201;  termV[4][4]=0.19;   termV[4][5]=0.171;  termV[4][6]=0.158;  termV[4][7]=0.151;  termV[4][8]=0.163;  termV[4][9]=0.172;  termV[4][10]=0.181;  termV[4][11]=0.188;  termV[4][12]=0.201;
            termV[5][0]=0.276;  termV[5][1]=0.248;  termV[5][2]=0.212;  termV[5][3]=0.199;  termV[5][4]=0.189;  termV[5][5]=0.172;  termV[5][6]=0.16;   termV[5][7]=0.155;  termV[5][8]=0.162;  termV[5][9]=0.17;   termV[5][10]=0.177;  termV[5][11]=0.183;  termV[5][12]=0.195;
            termV[6][0]=0.26;   termV[6][1]=0.237;  termV[6][2]=0.21;   termV[6][3]=0.198;  termV[6][4]=0.188;  termV[6][5]=0.172;  termV[6][6]=0.161;  termV[6][7]=0.156;  termV[6][8]=0.161;  termV[6][9]=0.167;  termV[6][10]=0.173;  termV[6][11]=0.179;  termV[6][12]=0.19;
            termV[7][0]=0.25;   termV[7][1]=0.231;  termV[7][2]=0.208;  termV[7][3]=0.196;  termV[7][4]=0.187;  termV[7][5]=0.172;  termV[7][6]=0.162;  termV[7][7]=0.156;  termV[7][8]=0.16;   termV[7][9]=0.165;  termV[7][10]=0.17;   termV[7][11]=0.175;  termV[7][12]=0.185;
            termV[8][0]=0.244;  termV[8][1]=0.226;  termV[8][2]=0.206;  termV[8][3]=0.195;  termV[8][4]=0.186;  termV[8][5]=0.171;  termV[8][6]=0.161;  termV[8][7]=0.156;  termV[8][8]=0.158;  termV[8][9]=0.162;  termV[8][10]=0.166;  termV[8][11]=0.171;  termV[8][12]=0.18;
            termV[9][0]=0.239;  termV[9][1]=0.222;  termV[9][2]=0.204;  termV[9][3]=0.193;  termV[9][4]=0.185;  termV[9][5]=0.17;   termV[9][6]=0.16;   termV[9][7]=0.155;  termV[9][8]=0.156;  termV[9][9]=0.159;  termV[9][10]=0.163;  termV[9][11]=0.168;  termV[9][12]=0.177;
            termV[10][0]=0.235; termV[10][1]=0.219; termV[10][2]=0.202; termV[10][3]=0.192; termV[10][4]=0.183; termV[10][5]=0.169; termV[10][6]=0.159; termV[10][7]=0.154; termV[10][8]=0.154; termV[10][9]=0.156; termV[10][10]=0.16;  termV[10][11]=0.164; termV[10][12]=0.173;
            termV[11][0]=0.227; termV[11][1]=0.212; termV[11][2]=0.197; termV[11][3]=0.187; termV[11][4]=0.179; termV[11][5]=0.166; termV[11][6]=0.156; termV[11][7]=0.151; termV[11][8]=0.149; termV[11][9]=0.15;  termV[11][10]=0.153; termV[11][11]=0.157; termV[11][12]=0.165;
            termV[12][0]=0.22;  termV[12][1]=0.206; termV[12][2]=0.192; termV[12][3]=0.183; termV[12][4]=0.175; termV[12][5]=0.162; termV[12][6]=0.153; termV[12][7]=0.147; termV[12][8]=0.144; termV[12][9]=0.144; termV[12][10]=0.147; termV[12][11]=0.151; termV[12][12]=0.158;
            termV[13][0]=0.211; termV[13][1]=0.197; termV[13][2]=0.185; termV[13][3]=0.176; termV[13][4]=0.168; termV[13][5]=0.156; termV[13][6]=0.147; termV[13][7]=0.142; termV[13][8]=0.138; termV[13][9]=0.138; termV[13][10]=0.14;  termV[13][11]=0.144; termV[13][12]=0.151;
            termV[14][0]=0.204; termV[14][1]=0.192; termV[14][2]=0.18;  termV[14][3]=0.171; termV[14][4]=0.164; termV[14][5]=0.152; termV[14][6]=0.143; termV[14][7]=0.138; termV[14][8]=0.134; termV[14][9]=0.134; termV[14][10]=0.137; termV[14][11]=0.14;  termV[14][12]=0.148;
            termV[15][0]=0.2;   termV[15][1]=0.187; termV[15][2]=0.176; termV[15][3]=0.167; termV[15][4]=0.16;  termV[15][5]=0.148; termV[15][6]=0.14;  termV[15][7]=0.135; termV[15][8]=0.131; termV[15][9]=0.132; termV[15][10]=0.135; termV[15][11]=0.139; termV[15][12]=0.146;

            capFloorVolSurface = boost::shared_ptr<CapFloorTermVolSurface>(new
                CapFloorTermVolSurface(0, calendar, Following,
                                       optionTenors, strikes,
                                       termV, dayCounter));
        }
    };

}

void OptionletStripperTest::testFlatTermVolatilityStripping1() {

    BOOST_TEST_MESSAGE(
        "Testing forward/forward vol stripping from flat term vol "
        "surface using OptionletStripper1 class...");

    CommonVars vars;
    Settings::instance().evaluationDate() = Date(28, October, 2013);

    vars.setFlatTermVolSurface();

    shared_ptr<IborIndex> iborIndex(new Euribor6M(vars.yieldTermStructure));

    boost::shared_ptr<OptionletStripper> optionletStripper1(new
        OptionletStripper1(vars.flatTermVolSurface,
                           iborIndex,
                           Null<Rate>(),
                           vars.accuracy));

    boost::shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter(new
        StrippedOptionletAdapter(optionletStripper1));

    Handle<OptionletVolatilityStructure> vol(strippedOptionletAdapter);

    vol->enableExtrapolation();

    boost::shared_ptr<BlackCapFloorEngine> strippedVolEngine(new
        BlackCapFloorEngine(vars.yieldTermStructure,
                            vol));

    boost::shared_ptr<CapFloor> cap;
    for (Size tenorIndex=0; tenorIndex<vars.optionTenors.size(); ++tenorIndex) {
        for (Size strikeIndex=0; strikeIndex<vars.strikes.size(); ++strikeIndex) {
            cap = MakeCapFloor(CapFloor::Cap,
                               vars.optionTenors[tenorIndex],
                               iborIndex,
                               vars.strikes[strikeIndex],
                               0*Days)
                  .withPricingEngine(strippedVolEngine);

            Real priceFromStrippedVolatility = cap->NPV();

            boost::shared_ptr<PricingEngine> blackCapFloorEngineConstantVolatility(new
                BlackCapFloorEngine(vars.yieldTermStructure,
                                    vars.termV[tenorIndex][strikeIndex]));

            cap->setPricingEngine(blackCapFloorEngineConstantVolatility);
            Real priceFromConstantVolatility = cap->NPV();

            Real error = std::fabs(priceFromStrippedVolatility - priceFromConstantVolatility);
            if (error>vars.tolerance)
                BOOST_FAIL("\noption tenor:       " << vars.optionTenors[tenorIndex] <<
                           "\nstrike:             " << io::rate(vars.strikes[strikeIndex]) <<
                           "\nstripped vol price: " << io::rate(priceFromStrippedVolatility) <<
                           "\nconstant vol price: " << io::rate(priceFromConstantVolatility) <<
                           "\nerror:              " << io::rate(error) <<
                           "\ntolerance:          " << io::rate(vars.tolerance));
            }
    }
}

void OptionletStripperTest::testTermVolatilityStripping1() {

    BOOST_TEST_MESSAGE(
        "Testing forward/forward vol stripping from non-flat term "
        "vol surface using OptionletStripper1 class...");

    CommonVars vars;
    Settings::instance().evaluationDate() = Date(28, October, 2013);

    vars.setCapFloorTermVolSurface();

    shared_ptr<IborIndex> iborIndex(new Euribor6M(vars.yieldTermStructure));

    boost::shared_ptr<OptionletStripper> optionletStripper1(new
        OptionletStripper1(vars.capFloorVolSurface,
                           iborIndex,
                           Null<Rate>(),
                           vars.accuracy));

    boost::shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter =
        boost::shared_ptr<StrippedOptionletAdapter>(new
            StrippedOptionletAdapter(optionletStripper1));

    Handle<OptionletVolatilityStructure> vol(strippedOptionletAdapter);

    vol->enableExtrapolation();

    boost::shared_ptr<BlackCapFloorEngine> strippedVolEngine(new
        BlackCapFloorEngine(vars.yieldTermStructure,
                            vol));

    boost::shared_ptr<CapFloor> cap;
    for (Size tenorIndex=0; tenorIndex<vars.optionTenors.size(); ++tenorIndex) {
        for (Size strikeIndex=0; strikeIndex<vars.strikes.size(); ++strikeIndex) {
            cap = MakeCapFloor(CapFloor::Cap,
                               vars.optionTenors[tenorIndex],
                               iborIndex,
                               vars.strikes[strikeIndex],
                               0*Days)
                  .withPricingEngine(strippedVolEngine);

            Real priceFromStrippedVolatility = cap->NPV();

            boost::shared_ptr<PricingEngine> blackCapFloorEngineConstantVolatility(new
                BlackCapFloorEngine(vars.yieldTermStructure,
                                    vars.termV[tenorIndex][strikeIndex]));

            cap->setPricingEngine(blackCapFloorEngineConstantVolatility);
            Real priceFromConstantVolatility = cap->NPV();

            Real error = std::fabs(priceFromStrippedVolatility - priceFromConstantVolatility);
            if (error>vars.tolerance)
                BOOST_FAIL("\noption tenor:       " << vars.optionTenors[tenorIndex] <<
                           "\nstrike:             " << io::rate(vars.strikes[strikeIndex]) <<
                           "\nstripped vol price: " << io::rate(priceFromStrippedVolatility) <<
                           "\nconstant vol price: " << io::rate(priceFromConstantVolatility) <<
                           "\nerror:              " << io::rate(error) <<
                           "\ntolerance:          " << io::rate(vars.tolerance));
            }
    }
}


void OptionletStripperTest::testFlatTermVolatilityStripping2() {

  BOOST_TEST_MESSAGE(
        "Testing forward/forward vol stripping from flat term vol "
        "surface using OptionletStripper2 class...");

  CommonVars vars;
  Settings::instance().evaluationDate() = Date::todaysDate();

  vars.setFlatTermVolCurve();
  vars.setFlatTermVolSurface();

  shared_ptr<IborIndex> iborIndex(new Euribor6M(vars.yieldTermStructure));

  // optionletstripper1
  shared_ptr<OptionletStripper1> optionletStripper1(new
        OptionletStripper1(vars.flatTermVolSurface,
                           iborIndex,
                           Null<Rate>(),
                           vars.accuracy));

  boost::shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter1(new
        StrippedOptionletAdapter(optionletStripper1));

  Handle<OptionletVolatilityStructure> vol1(strippedOptionletAdapter1);

  vol1->enableExtrapolation();

  // optionletstripper2
  shared_ptr<OptionletStripper> optionletStripper2(new
        OptionletStripper2(optionletStripper1, vars.flatTermVolCurve));

  shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter2(new
        StrippedOptionletAdapter(optionletStripper2));

  Handle<OptionletVolatilityStructure> vol2(strippedOptionletAdapter2);

  vol2->enableExtrapolation();

  // consistency check: diff(stripped vol1-stripped vol2)
  for (Size strikeIndex=0; strikeIndex<vars.strikes.size(); ++strikeIndex) {
    for (Size tenorIndex=0; tenorIndex<vars.optionTenors.size(); ++tenorIndex) {

      Volatility strippedVol1 = vol1->volatility(vars.optionTenors[tenorIndex],
                                                 vars.strikes[strikeIndex], true);

      Volatility strippedVol2 = vol2->volatility(vars.optionTenors[tenorIndex],
                                                 vars.strikes[strikeIndex], true);

      // vol from flat vol surface (for comparison only)
      Volatility flatVol = vars.flatTermVolSurface->volatility(vars.optionTenors[tenorIndex],
                                                               vars.strikes[strikeIndex], true);

    Real error = std::fabs(strippedVol1-strippedVol2);
      if (error>vars.tolerance)
      BOOST_FAIL("\noption tenor:  " << vars.optionTenors[tenorIndex] <<
                 "\nstrike:        " << io::rate(vars.strikes[strikeIndex]) <<
                 "\nstripped vol1: " << io::rate(strippedVol1) <<
                 "\nstripped vol2: " << io::rate(strippedVol2) <<
                 "\nflat vol:      " << io::rate(flatVol) <<
                 "\nerror:         " << io::rate(error) <<
                 "\ntolerance:     " << io::rate(vars.tolerance));
    }
  }

}

void OptionletStripperTest::testTermVolatilityStripping2() {

  BOOST_TEST_MESSAGE(
        "Testing forward/forward vol stripping from non-flat term vol "
        "surface using OptionletStripper2 class...");

  CommonVars vars;
  Settings::instance().evaluationDate() = Date::todaysDate();

  vars.setCapFloorTermVolCurve();
  vars.setCapFloorTermVolSurface();

  shared_ptr<IborIndex> iborIndex(new Euribor6M(vars.yieldTermStructure));

  // optionletstripper1
  boost::shared_ptr<OptionletStripper1> optionletStripper1(new
        OptionletStripper1(vars.capFloorVolSurface,
                           iborIndex,
                           Null<Rate>(),
                           vars.accuracy));

  boost::shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter1 =
        boost::shared_ptr<StrippedOptionletAdapter>(new
            StrippedOptionletAdapter(optionletStripper1));

  Handle<OptionletVolatilityStructure> vol1(strippedOptionletAdapter1);
  vol1->enableExtrapolation();

  // optionletstripper2
  boost::shared_ptr<OptionletStripper> optionletStripper2(new
                OptionletStripper2(optionletStripper1,
                                   vars.capFloorVolCurve));

  boost::shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter2(new
        StrippedOptionletAdapter(optionletStripper2));

  Handle<OptionletVolatilityStructure> vol2(strippedOptionletAdapter2);
  vol2->enableExtrapolation();

  // consistency check: diff(stripped vol1-stripped vol2)
  for (Size strikeIndex=0; strikeIndex<vars.strikes.size(); ++strikeIndex) {
    for (Size tenorIndex=0; tenorIndex<vars.optionTenors.size(); ++tenorIndex) {

      Volatility strippedVol1 = vol1->volatility(vars.optionTenors[tenorIndex],
                                                 vars.strikes[strikeIndex], true);

      Volatility strippedVol2 = vol2->volatility(vars.optionTenors[tenorIndex],
                                                 vars.strikes[strikeIndex], true);

      // vol from flat vol surface (for comparison only)
      Volatility flatVol = vars.capFloorVolSurface->volatility(vars.optionTenors[tenorIndex],
                                                               vars.strikes[strikeIndex], true);

      Real error = std::fabs(strippedVol1-strippedVol2);
      if (error>vars.tolerance)
      BOOST_FAIL("\noption tenor:  " << vars.optionTenors[tenorIndex] <<
                 "\nstrike:        " << io::rate(vars.strikes[strikeIndex]) <<
                 "\nstripped vol1: " << io::rate(strippedVol1) <<
                 "\nstripped vol2: " << io::rate(strippedVol2) <<
                 "\nflat vol:      " << io::rate(flatVol) <<
                 "\nerror:         " << io::rate(error) <<
                 "\ntolerance:     " << io::rate(vars.tolerance));
    }
  }
}

test_suite* OptionletStripperTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("OptionletStripper Tests");
    suite->add(QUANTLIB_TEST_CASE(
                   &OptionletStripperTest::testFlatTermVolatilityStripping1));
    suite->add(QUANTLIB_TEST_CASE(
                       &OptionletStripperTest::testTermVolatilityStripping1));
    suite->add(QUANTLIB_TEST_CASE(
                   &OptionletStripperTest::testFlatTermVolatilityStripping2));
    suite->add(QUANTLIB_TEST_CASE(
                       &OptionletStripperTest::testTermVolatilityStripping2));
    return suite;
}
