/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2007, 2008 Laurent Hoffmann
 Copyright (C) 2015, 2016 Michael von den Driesch

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/currencies/america.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/pricingengines/capfloor/bacheliercapfloorengine.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/capfloor/capfloortermvolcurve.hpp>
#include <ql/termstructures/volatility/capfloor/constantcapfloortermvol.hpp>
#include <ql/termstructures/volatility/optionlet/optionletstripper1.hpp>
#include <ql/termstructures/volatility/optionlet/optionletstripper2.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionletadapter.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <algorithm>
#include <iterator>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(OptionletStripperTests)

struct CommonVars {
    // global data
    Calendar calendar;
    DayCounter dayCounter;

    RelinkableHandle<YieldTermStructure> yieldTermStructure;
    RelinkableHandle< YieldTermStructure > discountingYTS;
    RelinkableHandle< YieldTermStructure > forwardingYTS;

    std::vector<Rate> strikes;
    std::vector<Period> optionTenors;
    Matrix termV;
    std::vector<Rate> atmTermV;
    std::vector<Handle<Quote> > atmTermVolHandle;

    Handle<CapFloorTermVolCurve> capFloorVolCurve;
    Handle<CapFloorTermVolCurve> flatTermVolCurve;

    ext::shared_ptr<CapFloorTermVolSurface> capFloorVolSurface;
    ext::shared_ptr<CapFloorTermVolSurface> flatTermVolSurface;
    ext::shared_ptr< CapFloorTermVolSurface > capFloorVolRealSurface;

    Real accuracy;
    Real tolerance;

    CommonVars() {
        accuracy = 1.0e-6;
        tolerance = 2.5e-8;
    }

    void setTermStructure() {

        calendar = TARGET();
        dayCounter = Actual365Fixed();

        Rate flatFwdRate = 0.04;
        yieldTermStructure.linkTo(
                ext::make_shared<FlatForward>(0,
                                              calendar,
                                              flatFwdRate,
                                              dayCounter));
    }

    void setRealTermStructure() {

        calendar = TARGET();
        dayCounter = Actual365Fixed();

        std::vector< int > datesTmp = {
            42124, 42129, 42143, 42221, 42254, 42282, 42313, 42345,
            42374, 42405, 42465, 42495, 42587, 42681, 42772, 42860, 43227,
            43956, 44321, 44686, 45051, 45418, 45782, 46147, 46512, 47609,
            49436, 51263, 53087, 56739, 60392
        };

        std::vector< Date > dates;
        dates.reserve(datesTmp.size());
        for (int& it : datesTmp)
            dates.emplace_back(it);

        std::vector< Rate > rates = {
            -0.00292, -0.00292, -0.001441, -0.00117, -0.001204,
            -0.001212, -0.001223, -0.001236, -0.001221, -0.001238,
            -0.001262, -0.00125, -0.001256, -0.001233, -0.00118, -0.001108,
            -0.000619, 0.000833, 0.001617, 0.002414, 0.003183, 0.003883,
            0.004514, 0.005074, 0.005606, 0.006856, 0.00813, 0.008709,
            0.009136, 0.009601, 0.009384
        };

        discountingYTS.linkTo(
                ext::make_shared< InterpolatedZeroCurve< Linear > >(
                    dates, rates,
                    dayCounter, calendar));

        datesTmp.clear();
        dates.clear();
        rates.clear();

        datesTmp = {
            42124, 42313, 42436, 42556, 42618, 42800, 42830, 42860,
            43227, 43591, 43956, 44321, 44686, 45051, 45418, 45782, 46147,
            46512, 46878, 47245, 47609, 47973, 48339, 48704, 49069, 49436,
            49800, 50165, 50530, 50895, 51263, 51627, 51991, 52356, 52722,
            53087, 54913, 56739, 60392, 64045
        };

        for (int& it : datesTmp)
            dates.emplace_back(it);

        rates = {
            0.000649, 0.000649, 0.000684, 0.000717, 0.000745, 0.000872,
            0.000905, 0.000954, 0.001532, 0.002319, 0.003147, 0.003949,
            0.004743, 0.00551, 0.006198, 0.006798, 0.007339, 0.007832,
            0.008242, 0.008614, 0.008935, 0.009205, 0.009443, 0.009651,
            0.009818, 0.009952, 0.010054, 0.010146, 0.010206, 0.010266,
            0.010315, 0.010365, 0.010416, 0.010468, 0.010519, 0.010571,
            0.010757, 0.010806, 0.010423, 0.010217
        };

        forwardingYTS.linkTo(
                ext::make_shared< InterpolatedZeroCurve< Linear > >(
                    dates, rates, dayCounter, calendar));
    }

    void setFlatTermVolCurve() {

        setTermStructure();

        optionTenors.resize(10);
        for (Size i = 0; i < optionTenors.size(); ++i)
            optionTenors[i] = Period(i + 1, Years);

        Volatility flatVol = .18;

        std::vector<Handle<Quote> >  curveVHandle(optionTenors.size());
        for (Size i=0; i<optionTenors.size(); ++i)
            curveVHandle[i] = Handle<Quote>(ext::shared_ptr<Quote>(new
                                                        SimpleQuote(flatVol)));

        flatTermVolCurve = Handle<CapFloorTermVolCurve>(
              ext::make_shared<CapFloorTermVolCurve>(0, calendar, Following, optionTenors,
                                       curveVHandle, dayCounter));

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
        flatTermVolSurface = ext::make_shared<CapFloorTermVolSurface>(0, calendar, Following,
                                                                      optionTenors, strikes,
                                                                      termV, dayCounter);
    }


    void setCapFloorTermVolCurve() {

        setTermStructure();

        //atm cap volatility curve
        optionTenors = {
            {1, Years},
            {18, Months},
            {2, Years},
            {3, Years},
            {4, Years},
            {5, Years},
            {6, Years},
            {7, Years},
            {8, Years},
            {9, Years},
            {10, Years},
            {12, Years},
            {15, Years},
            {20, Years},
            {25, Years},
            {30, Years}
        };

        //atm capfloor vols from mkt vol matrix using flat yield curve
        atmTermV = {
            0.090304,
            0.12180,
            0.13077,
            0.14832,
            0.15570,
            0.15816,
            0.15932,
            0.16035,
            0.15951,
            0.15855,
            0.15754,
            0.15459,
            0.15163,
            0.14575,
            0.14175,
            0.13889
        };

        atmTermVolHandle.resize(optionTenors.size());
        for (Size i=0; i<optionTenors.size(); ++i) {
            atmTermVolHandle[i] = Handle<Quote>(ext::shared_ptr<Quote>(new
                            SimpleQuote(atmTermV[i])));
        }

        capFloorVolCurve = Handle<CapFloorTermVolCurve>(
            ext::make_shared<CapFloorTermVolCurve>(0, calendar, Following,
                                                   optionTenors, atmTermVolHandle,
                                                   dayCounter));

    }

    void setCapFloorTermVolSurface() {

        setTermStructure();

        //cap volatility smile matrix
        optionTenors = {
            {1, Years},
            {18, Months},
            {2, Years},
            {3, Years},
            {4, Years},
            {5, Years},
            {6, Years},
            {7, Years},
            {8, Years},
            {9, Years},
            {10, Years},
            {12, Years},
            {15, Years},
            {20, Years},
            {25, Years},
            {30, Years}
        };

        strikes = {
            0.015,
            0.0175,
            0.02,
            0.0225,
            0.025,
            0.03,
            0.035,
            0.04,
            0.05,
            0.06,
            0.07,
            0.08,
            0.1
        };

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

        capFloorVolSurface = ext::make_shared<CapFloorTermVolSurface>(0, calendar, Following,
                                                                      optionTenors, strikes,
                                                                      termV, dayCounter);
    }

    void setRealCapFloorTermVolSurface() {

        setRealTermStructure();

        // cap volatility smile matrix
        optionTenors = {
            {1, Years},
            {18, Months},
            {2, Years},
            {3, Years},
            {4, Years},
            {5, Years},
            {6, Years},
            {7, Years},
            {8, Years},
            {9, Years},
            {10, Years},
            {12, Years},
            {15, Years},
            {20, Years},
            {25, Years},
            {30, Years}
        };
        // 16

        strikes = {
            -0.005,
            -0.0025,
            -0.00125,
            0.0,
            0.00125,
            0.0025,
            0.005,
            0.01,
            0.015,
            0.02,
            0.03,
            0.05,
            0.1
        };
        // 13

        std::vector< Real > rawVols = {
            0.49, 0.39, 0.34, 0.31, 0.34, 0.37, 0.50, 0.75, 0.99, 1.21, 1.64, 2.44, 4.29,
            0.44, 0.36, 0.33, 0.31, 0.33, 0.35,0.45, 0.65, 0.83, 1.00, 1.32, 1.93, 3.30,
            0.40, 0.35, 0.33,0.31, 0.33, 0.34, 0.41, 0.55, 0.69, 0.82, 1.08, 1.56, 2.68,
            0.42, 0.39, 0.38, 0.37, 0.38, 0.39, 0.43, 0.54, 0.64, 0.74,0.94, 1.31, 2.18,
            0.46, 0.43, 0.42, 0.41, 0.42, 0.43, 0.47,0.56, 0.66, 0.75, 0.93, 1.28, 2.07,
            0.49, 0.47, 0.46, 0.45,0.46, 0.47, 0.51, 0.59, 0.68, 0.76, 0.93, 1.25, 1.99,
            0.51, 0.49, 0.49, 0.48, 0.49, 0.50, 0.54, 0.62, 0.70, 0.78, 0.94,1.24, 1.94,
            0.52, 0.51, 0.51, 0.51, 0.52, 0.53, 0.56, 0.63,0.71, 0.79, 0.94, 1.23, 1.89,
            0.53, 0.52, 0.52, 0.52, 0.53,0.54, 0.57, 0.65, 0.72, 0.79, 0.94, 1.21, 1.83,
            0.55, 0.54, 0.54, 0.54, 0.55, 0.56, 0.59, 0.66, 0.72, 0.79, 0.91, 1.15,1.71,
            0.56, 0.56, 0.56, 0.56, 0.57, 0.58, 0.61, 0.67, 0.72,0.78, 0.89, 1.09, 1.59,
            0.59, 0.58, 0.58, 0.59, 0.59, 0.60,0.63, 0.68, 0.73, 0.78, 0.86, 1.03, 1.45,
            0.61, 0.61, 0.61,0.61, 0.62, 0.62, 0.64, 0.69, 0.73, 0.77, 0.85, 1.02, 1.44,
            0.62, 0.62, 0.63, 0.63, 0.64, 0.64, 0.65, 0.69, 0.72, 0.76,0.82, 0.96, 1.32,
            0.62, 0.63, 0.63, 0.63, 0.65, 0.66, 0.66,0.68, 0.72, 0.74, 0.80, 0.93, 1.25,
            0.62, 0.62, 0.62, 0.62,0.66, 0.67, 0.67, 0.67, 0.72, 0.72, 0.78, 0.90, 1.25
        };

        termV = Matrix(optionTenors.size(), strikes.size());
        std::copy(rawVols.begin(), rawVols.end(), termV.begin());
        termV /= 100;

        capFloorVolRealSurface =
            ext::make_shared< CapFloorTermVolSurface >(
                    0, calendar, Following,
                    optionTenors, strikes, termV,
                    dayCounter);
    }
};

struct CommonVarsON {
    Date today;
    Date startDate, endDate;
    Period tenor;
    Calendar calendar;
    BusinessDayConvention convention;
    DayCounter dc;
    RelinkableHandle<YieldTermStructure> sofrCurveHandle;
    std::vector<Rate> strikes;
    std::vector<Period> expiries;
    ext::shared_ptr<CapFloorTermVolSurface> capfloorVol;

    CommonVarsON() {
        today = Date(15, April, 2025);
        startDate = Date(17, April, 2025);
        endDate = Date(17, April, 2030);
        calendar = UnitedStates(UnitedStates::FederalReserve);
        convention = ModifiedFollowing;
        dc = Actual360();
    }

    void setSofrHandle() {
        std::vector<Date> dates = {
            Date(15, Apr, 2025),
            Date(16, Apr, 2025),
            Date(28, Apr, 2025),
            Date(21, May, 2025),
            Date(21, Jul, 2025),
            Date(21, Oct, 2025),
            Date(21, Apr, 2026),
            Date(21, Apr, 2027),
            Date(19, Apr, 2028),
            Date(22, Apr, 2030),
            Date(21, Apr, 2032),
            Date(19, Apr, 2035),
            Date(21, Apr, 2037),
            Date(19, Apr, 2040),
            Date(19, Apr, 2045),
            Date(20, Apr, 2050),
            Date(21, Apr, 2055)
        };

        std::vector<Rate> zeroRates = {
            3.039872 / 100.0,
            3.082092 / 100.0,
            3.67902  / 100.0,
            3.791077 / 100.0,
            4.147655 / 100.0,
            4.498917 / 100.0,
            4.688082 / 100.0,
            4.486636 / 100.0,
            4.228873 / 100.0,
            3.949601 / 100.0,
            3.814579 / 100.0,
            3.731412 / 100.0,
            3.718794 / 100.0,
            3.704788 / 100.0,
            3.599069 / 100.0,
            3.401666 / 100.0,
            3.221372 / 100.0
        };

        ext::shared_ptr<YieldTermStructure> sofrCurve(
            new ZeroCurve(dates, zeroRates, Actual365Fixed(), calendar));
        sofrCurveHandle.linkTo(sofrCurve);
    }

    void setRealCapFloorVolSurface() {
        strikes = {0.03, 0.035, 0.04};
        
        for (int i = 1; i <= 10; ++i)
            expiries.emplace_back(i, Years);

        Matrix vols(expiries.size(), strikes.size());
        Real data[10][3] = {
            {12.52, 24.73, 26.8},
            {15.81, 24.94, 27.95},
            {18.91, 41.48, 38.94},
            {21,    40.14, 37.17},
            {22.46, 41.69, 38.96},
            {23.39, 43.06, 38.48},
            {23.95, 43.98, 39.61},
            {24.29, 44.58, 39.51},
            {24.42, 44.7,  39.09},
            {24.42, 44.36, 37.41}
        };
            
        for (Size i = 0; i < vols.rows(); ++i)
            for (Size j = 0; j < vols.columns(); ++j)
                vols[i][j] = data[i][j] / 10000.0;

        capfloorVol = ext::make_shared<CapFloorTermVolSurface>(
                                        2, calendar, convention,
                                        expiries, strikes, vols, dc);
    }

};


BOOST_AUTO_TEST_CASE(testFlatTermVolatilityStripping1) {

    BOOST_TEST_MESSAGE(
        "Testing forward/forward vol stripping from flat term vol "
        "surface using OptionletStripper1 class...");

    CommonVars vars;
    Settings::instance().evaluationDate() = Date(28, October, 2013);

    vars.setFlatTermVolSurface();

    ext::shared_ptr<IborIndex> iborIndex(new Euribor6M(vars.yieldTermStructure));

    ext::shared_ptr<OptionletStripper> optionletStripper1(new
        OptionletStripper1(vars.flatTermVolSurface,
                           iborIndex,
                           Null<Rate>(),
                           vars.accuracy));

    ext::shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter(new
        StrippedOptionletAdapter(optionletStripper1));

    Handle<OptionletVolatilityStructure> vol(strippedOptionletAdapter);

    vol->enableExtrapolation();

    ext::shared_ptr<BlackCapFloorEngine> strippedVolEngine(new
        BlackCapFloorEngine(vars.yieldTermStructure,
                            vol));

    ext::shared_ptr<CapFloor> cap;
    for (Size tenorIndex=0; tenorIndex<vars.optionTenors.size(); ++tenorIndex) {
        for (Size strikeIndex=0; strikeIndex<vars.strikes.size(); ++strikeIndex) {
            cap = MakeCapFloor(CapFloor::Cap,
                               vars.optionTenors[tenorIndex],
                               iborIndex,
                               vars.strikes[strikeIndex],
                               0*Days)
                  .withPricingEngine(strippedVolEngine);

            Real priceFromStrippedVolatility = cap->NPV();

            ext::shared_ptr<PricingEngine> blackCapFloorEngineConstantVolatility(new
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

BOOST_AUTO_TEST_CASE(testTermVolatilityStripping1) {

    BOOST_TEST_MESSAGE(
        "Testing forward/forward vol stripping from non-flat term "
        "vol surface using OptionletStripper1 class...");

    CommonVars vars;
    Settings::instance().evaluationDate() = Date(28, October, 2013);

    vars.setCapFloorTermVolSurface();

    ext::shared_ptr<IborIndex> iborIndex(new Euribor6M(vars.yieldTermStructure));

    ext::shared_ptr<OptionletStripper> optionletStripper1(new
        OptionletStripper1(vars.capFloorVolSurface,
                           iborIndex,
                           Null<Rate>(),
                           vars.accuracy));

    ext::shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter =
        ext::make_shared<StrippedOptionletAdapter>(optionletStripper1);

    Handle<OptionletVolatilityStructure> vol(strippedOptionletAdapter);

    vol->enableExtrapolation();

    ext::shared_ptr<BlackCapFloorEngine> strippedVolEngine(new
        BlackCapFloorEngine(vars.yieldTermStructure,
                            vol));

    ext::shared_ptr<CapFloor> cap;
    for (Size tenorIndex=0; tenorIndex<vars.optionTenors.size(); ++tenorIndex) {
        for (Size strikeIndex=0; strikeIndex<vars.strikes.size(); ++strikeIndex) {
            cap = MakeCapFloor(CapFloor::Cap,
                               vars.optionTenors[tenorIndex],
                               iborIndex,
                               vars.strikes[strikeIndex],
                               0*Days)
                  .withPricingEngine(strippedVolEngine);

            Real priceFromStrippedVolatility = cap->NPV();

            ext::shared_ptr<PricingEngine> blackCapFloorEngineConstantVolatility(new
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

BOOST_AUTO_TEST_CASE(testTermVolatilityStrippingNormalVol) {

    BOOST_TEST_MESSAGE(
        "Testing forward/forward vol stripping from non-flat normal vol term "
        "vol surface for normal vol setup using OptionletStripper1 class...");

    CommonVars vars;
    Settings::instance().evaluationDate() = Date(30, April, 2015);

    vars.setRealCapFloorTermVolSurface();

    ext::shared_ptr< IborIndex > iborIndex(new Euribor6M(vars.forwardingYTS));

    ext::shared_ptr< OptionletStripper > optionletStripper1(
        new OptionletStripper1(vars.capFloorVolRealSurface, iborIndex,
                               Null< Rate >(), vars.accuracy, 100,
                               vars.discountingYTS, Normal));

    ext::shared_ptr< StrippedOptionletAdapter > strippedOptionletAdapter =
        ext::make_shared< StrippedOptionletAdapter >(
            optionletStripper1);

    Handle< OptionletVolatilityStructure > vol(strippedOptionletAdapter);

    vol->enableExtrapolation();

    ext::shared_ptr< BachelierCapFloorEngine > strippedVolEngine(
        new BachelierCapFloorEngine(vars.discountingYTS, vol));

    ext::shared_ptr< CapFloor > cap;
    for (Size tenorIndex = 0; tenorIndex < vars.optionTenors.size();
         ++tenorIndex) {
        for (Size strikeIndex = 0; strikeIndex < vars.strikes.size();
             ++strikeIndex) {
            cap = MakeCapFloor(CapFloor::Cap, vars.optionTenors[tenorIndex],
                               iborIndex, vars.strikes[strikeIndex],
                               0 * Days).withPricingEngine(strippedVolEngine);

            Real priceFromStrippedVolatility = cap->NPV();

            ext::shared_ptr< PricingEngine >
                bachelierCapFloorEngineConstantVolatility(
                    new BachelierCapFloorEngine(
                        vars.discountingYTS,
                        vars.termV[tenorIndex][strikeIndex]));

            cap->setPricingEngine(bachelierCapFloorEngineConstantVolatility);
            Real priceFromConstantVolatility = cap->NPV();

            Real error = std::fabs(priceFromStrippedVolatility -
                                   priceFromConstantVolatility);
            if (error > vars.tolerance)
                BOOST_FAIL(
                    "\noption tenor:       "
                    << vars.optionTenors[tenorIndex] << "\nstrike:             "
                    << io::rate(vars.strikes[strikeIndex])
                    << "\nstripped vol price: "
                    << io::rate(priceFromStrippedVolatility)
                    << "\nconstant vol price: "
                    << io::rate(priceFromConstantVolatility)
                    << "\nerror:              " << io::rate(error)
                    << "\ntolerance:          " << io::rate(vars.tolerance));
        }
    }
}

BOOST_AUTO_TEST_CASE(testTermVolatilityStrippingShiftedLogNormalVol) {

    BOOST_TEST_MESSAGE(
        "Testing forward/forward vol stripping from non-flat normal vol term "
        "vol surface for normal vol setup using OptionletStripper1 class...");

    CommonVars vars;
    Real shift = 0.03;
    Settings::instance().evaluationDate() = Date(30, April, 2015);

    vars.setRealCapFloorTermVolSurface();

    ext::shared_ptr< IborIndex > iborIndex(new Euribor6M(vars.forwardingYTS));

    ext::shared_ptr< OptionletStripper > optionletStripper1(
        new OptionletStripper1(vars.capFloorVolRealSurface, iborIndex,
                               Null< Rate >(), vars.accuracy, 100,
                               vars.discountingYTS, ShiftedLognormal, shift,
                               true));

    ext::shared_ptr< StrippedOptionletAdapter > strippedOptionletAdapter =
        ext::make_shared< StrippedOptionletAdapter >(
            optionletStripper1);

    Handle< OptionletVolatilityStructure > vol(strippedOptionletAdapter);

    vol->enableExtrapolation();

    ext::shared_ptr< BlackCapFloorEngine > strippedVolEngine(
        new BlackCapFloorEngine(vars.discountingYTS, vol));

    ext::shared_ptr< CapFloor > cap;
    for (Size strikeIndex = 0; strikeIndex < vars.strikes.size();
         ++strikeIndex) {
        for (Size tenorIndex = 0; tenorIndex < vars.optionTenors.size();
             ++tenorIndex) {
            cap = MakeCapFloor(CapFloor::Cap, vars.optionTenors[tenorIndex],
                               iborIndex, vars.strikes[strikeIndex],
                               0 * Days).withPricingEngine(strippedVolEngine);

            Real priceFromStrippedVolatility = cap->NPV();

            ext::shared_ptr< PricingEngine >
                blackCapFloorEngineConstantVolatility(new BlackCapFloorEngine(
                    vars.discountingYTS, vars.termV[tenorIndex][strikeIndex],
                    vars.capFloorVolRealSurface->dayCounter(), shift));

            cap->setPricingEngine(blackCapFloorEngineConstantVolatility);
            Real priceFromConstantVolatility = cap->NPV();

            Real error = std::fabs(priceFromStrippedVolatility -
                                   priceFromConstantVolatility);
            if (error > vars.tolerance)
                BOOST_FAIL(
                    "\noption tenor:       "
                    << vars.optionTenors[tenorIndex] << "\nstrike:             "
                    << io::rate(vars.strikes[strikeIndex])
                    << "\nstripped vol price: "
                    << io::rate(priceFromStrippedVolatility)
                    << "\nconstant vol price: "
                    << io::rate(priceFromConstantVolatility)
                    << "\nerror:              " << io::rate(error)
                    << "\ntolerance:          " << io::rate(vars.tolerance));
        }
    }
}

BOOST_AUTO_TEST_CASE(testFlatTermVolatilityStripping2) {

  BOOST_TEST_MESSAGE(
        "Testing forward/forward vol stripping from flat term vol "
        "surface using OptionletStripper2 class...");

  CommonVars vars;
  Settings::instance().evaluationDate() = Date(28, October, 2013);

  vars.setFlatTermVolCurve();
  vars.setFlatTermVolSurface();

  ext::shared_ptr<IborIndex> iborIndex(new Euribor6M(vars.yieldTermStructure));

  // optionletstripper1
  ext::shared_ptr<OptionletStripper1> optionletStripper1(new
        OptionletStripper1(vars.flatTermVolSurface,
                           iborIndex,
                           Null<Rate>(),
                           vars.accuracy));

  ext::shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter1(new
        StrippedOptionletAdapter(optionletStripper1));

  Handle<OptionletVolatilityStructure> vol1(strippedOptionletAdapter1);

  vol1->enableExtrapolation();

  // optionletstripper2
  ext::shared_ptr<OptionletStripper> optionletStripper2(new
        OptionletStripper2(optionletStripper1, vars.flatTermVolCurve));

  ext::shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter2(new
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

BOOST_AUTO_TEST_CASE(testTermVolatilityStripping2) {

  BOOST_TEST_MESSAGE(
        "Testing forward/forward vol stripping from non-flat term vol "
        "surface using OptionletStripper2 class...");

  CommonVars vars;
  Settings::instance().evaluationDate() = Date(30, April, 2015);

  vars.setCapFloorTermVolCurve();
  vars.setCapFloorTermVolSurface();

  ext::shared_ptr<IborIndex> iborIndex(new Euribor6M(vars.yieldTermStructure));

  // optionletstripper1
  ext::shared_ptr<OptionletStripper1> optionletStripper1(new
        OptionletStripper1(vars.capFloorVolSurface,
                           iborIndex,
                           Null<Rate>(),
                           vars.accuracy));

  ext::shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter1 =
        ext::make_shared<StrippedOptionletAdapter>(optionletStripper1);

  Handle<OptionletVolatilityStructure> vol1(strippedOptionletAdapter1);
  vol1->enableExtrapolation();

  // optionletstripper2
  ext::shared_ptr<OptionletStripper> optionletStripper2(new
                OptionletStripper2(optionletStripper1,
                                   vars.capFloorVolCurve));

  ext::shared_ptr<StrippedOptionletAdapter> strippedOptionletAdapter2(new
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

BOOST_AUTO_TEST_CASE(testSwitchStrike) {
    BOOST_TEST_MESSAGE("Testing switch strike level and recalibration of level "
                       "in case of curve relinking...");

    bool usingAtParCoupons  = IborCoupon::Settings::instance().usingAtParCoupons();

    CommonVars vars;
    Settings::instance().evaluationDate() = Date(28, October, 2013);
    vars.setCapFloorTermVolSurface();

    RelinkableHandle< YieldTermStructure > yieldTermStructure;
    yieldTermStructure.linkTo(ext::make_shared< FlatForward >(
        0, vars.calendar, 0.03, vars.dayCounter));

    ext::shared_ptr< IborIndex > iborIndex(new Euribor6M(yieldTermStructure));

    ext::shared_ptr< OptionletStripper1 > optionletStripper1(
        new OptionletStripper1(vars.capFloorVolSurface, iborIndex,
                               Null< Rate >(), vars.accuracy));

    Real expected = usingAtParCoupons ? 0.02981223 : 0.02981258;

    Real error = std::fabs(optionletStripper1->switchStrike() - expected);
    if (error > vars.tolerance)
        BOOST_FAIL("\nSwitchstrike not correctly computed:  "
                   << "\nexpected switch strike: " << io::rate(expected)
                   << "\ncomputed switch strike: "
                   << io::rate(optionletStripper1->switchStrike())
                   << "\nerror:         " << io::rate(error)
                   << "\ntolerance:     " << io::rate(vars.tolerance));

    yieldTermStructure.linkTo(ext::make_shared< FlatForward >(
        0, vars.calendar, 0.05, vars.dayCounter));

    expected = usingAtParCoupons ? 0.0499371 : 0.0499381;

    error = std::fabs(optionletStripper1->switchStrike() - expected);
    if (error > vars.tolerance)
        BOOST_FAIL("\nSwitchstrike not correctly computed:  "
                   << "\nexpected switch strike: " << io::rate(expected)
                   << "\ncomputed switch strike: "
                   << io::rate(optionletStripper1->switchStrike())
                   << "\nerror:         " << io::rate(error)
                   << "\ntolerance:     " << io::rate(vars.tolerance));
}

BOOST_AUTO_TEST_CASE(testTermVolatilityStripping1ON, *precondition(if_speed(Slow))) {
    BOOST_TEST_MESSAGE("Testing optionlet stripping with overnight index...");
    CommonVarsON vars;
    Settings::instance().evaluationDate() = vars.today;
    Schedule schedule(vars.startDate, vars.endDate, vars.tenor,
                      vars.calendar, vars.convention, vars.convention,
                      DateGeneration::Forward, false);
    vars.setSofrHandle();
    vars.setRealCapFloorVolSurface();

    ext::shared_ptr<OvernightIndex> sofrIndex(new Sofr(vars.sofrCurveHandle));
    sofrIndex->addFixing(Date(15, April, 2025), 3.04/100.0);

    Real notional = 1'000'000;
    OvernightLeg sofrLeg(schedule, sofrIndex);
    sofrLeg.withNotionals(notional)
           .withPaymentAdjustment(ModifiedFollowing)
           .withPaymentLag(2);

    Rate strikeRate = 0.04;
    std::vector<Rate> strikes(1, strikeRate);
    Cap cap(sofrLeg, strikes);
    Cap cap1(sofrLeg, strikes);

    ext::shared_ptr<OptionletStripper1> optionletSurf(
            new OptionletStripper1(vars.capfloorVol, sofrIndex,
                                   Null<Real>(), 1e-6, 100,
                                   vars.sofrCurveHandle, Normal,
                                   0.0, true, Period(3, Months)));

    Handle<OptionletVolatilityStructure> ovsHandle(
        ext::shared_ptr<OptionletVolatilityStructure>(
            new StrippedOptionletAdapter(optionletSurf)));

     ext::shared_ptr<IborIndex> sofr3m(new IborIndex(
        "SOFR", Period(3, Months), 2,
        USDCurrency(), vars.calendar, vars.convention, false, vars.dc, vars.sofrCurveHandle
    ));

    ext::shared_ptr<OptionletStripper1> optionletSurf1(
        new OptionletStripper1(vars.capfloorVol, sofr3m,
                               Null<Real>(), 1e-6, 100, vars.sofrCurveHandle, Normal)
    );

    ext::shared_ptr<OptionletVolatilityStructure> ovs(
        new StrippedOptionletAdapter(optionletSurf)
    );
    Handle<OptionletVolatilityStructure> ovsHandle1(ovs);

    // Use optionlet surface for pricing
    ext::shared_ptr<PricingEngine> engineOvs(
        new BachelierCapFloorEngine(vars.sofrCurveHandle, ovsHandle));
    cap.setPricingEngine(engineOvs);
    ext::shared_ptr<PricingEngine> engineOvs1(
        new BachelierCapFloorEngine(vars.sofrCurveHandle, ovsHandle1));
    cap1.setPricingEngine(engineOvs1);
    
    Real tolerance = 2.5e-8;
    Real capPrice = cap.NPV();
    Real cap1Price = cap1.NPV();
    Real error = std::fabs(capPrice - cap1Price);
    if (error> tolerance)
      BOOST_FAIL("\nerror:         " << error <<
                 "\ntolerance:     " << io::rate(tolerance));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
