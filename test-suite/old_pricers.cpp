
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
// $Id$

#include "old_pricers.hpp"
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <vector>
#include <map>
#include <string>

// This makes it easier to use array literals (alas, no std::vector literals)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace QuantLib;
using QuantLib::Pricers::BarrierOption;
using QuantLib::Pricers::BinaryOption;
using QuantLib::Pricers::CliquetOption;
using QuantLib::Pricers::FdDividendEuropeanOption;
using QuantLib::Pricers::EuropeanOption;
using QuantLib::Pricers::FdEuropean;
using QuantLib::RandomNumbers::UniformRandomGenerator;

namespace {

    double relError(double x1, double x2, double ref) {
        if (ref != 0.0)
            return QL_FABS((x1-x2)/ref);
        else
            return 1.0e+10;
    }

    std::string typeToString(Option::Type type) {
        switch (type) {
          case Option::Call:
            return "call";
          case Option::Put:
            return "put";
          case Option::Straddle:
            return "straddle";
          default:
            throw Error("unknown option type");
        }
    }

}

CppUnit::Test* OldPricerTest::suite() {
    CppUnit::TestSuite* tests = 
        new CppUnit::TestSuite("Old-style pricer tests");
    tests->addTest(new CppUnit::TestCaller<OldPricerTest>
                   ("Testing old-style barrier option pricer",
                    &OldPricerTest::testBarrierPricer));
    tests->addTest(new CppUnit::TestCaller<OldPricerTest>
                   ("Testing old-style binary option pricer",
                    &OldPricerTest::testBinaryPricer));
    tests->addTest(new CppUnit::TestCaller<OldPricerTest>
                   ("Testing old-style cliquet option pricer",
                    &OldPricerTest::testCliquetPricer));
    tests->addTest(new CppUnit::TestCaller<OldPricerTest>
                   ("Testing old-style European option pricer with dividends",
                    &OldPricerTest::testDividendEuropeanPricer));
    tests->addTest(new CppUnit::TestCaller<OldPricerTest>
                   ("Testing old-style finite-difference European pricer",
                    &OldPricerTest::testFdEuropeanPricer));
    /*
    tests->addTest(new CppUnit::TestCaller<OldPricerTest>
                   ("Testing old-style American-type pricers",
                    &OldPricerTest::testAmericanPricers));
    tests->addTest(new CppUnit::TestCaller<OldPricerTest>
                   ("Testing old-style Monte Carlo single-factor pricers",
                    &OldPricerTest::testMcSingleFactorPricers));
    tests->addTest(new CppUnit::TestCaller<OldPricerTest>
                   ("Testing old-style Monte Carlo multi-factor pricers",
                    &OldPricerTest::testMcMultiFactorPricers));
    */
    return tests;
}


namespace {
    
    struct BarrierOptionData {
        BarrierOption::BarrierType type;
        double volatility;
        double strike;
        double barrier;
        double callValue;
        double putValue;
    };

}

void OldPricerTest::testBarrierPricer() {

    double maxErrorAllowed = 5.0e-5;
    double maxStraddleErrorAllowed = 5.0e-4;
    double underlyingPrice = 100.0;
    double rebate = 3.0;
    Time residualTime = 0.5;
    Rate rRate = 0.08;
    Rate qRate = 0.04;
    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
       pag. 72 */
    BarrierOptionData values[] = {
        { BarrierOption::DownOut, 0.25,    90,      95,  9.0246,  2.2798 },
        { BarrierOption::DownOut, 0.25,   100,      95,  6.7924,  2.2947 },
        { BarrierOption::DownOut, 0.25,   110,      95,  4.8759,  2.6252 },
        { BarrierOption::DownOut, 0.25,    90,     100,  3.0000,  3.0000 },
        { BarrierOption::DownOut, 0.25,   100,     100,  3.0000,  3.0000 },
        { BarrierOption::DownOut, 0.25,   110,     100,  3.0000,  3.0000 },
        { BarrierOption::UpOut,   0.25,    90,     105,  2.6789,  3.7760 },
        { BarrierOption::UpOut,   0.25,   100,     105,  2.3580,  5.4932 },
        { BarrierOption::UpOut,   0.25,   110,     105,  2.3453,  7.5187 },

        { BarrierOption::DownIn,  0.25,    90,      95,  7.7627,  2.9586 },
        { BarrierOption::DownIn,  0.25,   100,      95,  4.0109,  6.5677 },
        { BarrierOption::DownIn,  0.25,   110,      95,  2.0576, 11.9752 },
        { BarrierOption::DownIn,  0.25,    90,     100, 13.8333,  2.2845 },
        { BarrierOption::DownIn,  0.25,   100,     100,  7.8494,  5.9085 },
        { BarrierOption::DownIn,  0.25,   110,     100,  3.9795, 11.6465 },
        { BarrierOption::UpIn,    0.25,    90,     105, 14.1112,  1.4653 },
        { BarrierOption::UpIn,    0.25,   100,     105,  8.4482,  3.3721 },
        { BarrierOption::UpIn,    0.25,   110,     105,  4.5910,  7.0846 },

        { BarrierOption::DownOut, 0.30,    90,      95,  8.8334,  2.4170 },
        { BarrierOption::DownOut, 0.30,   100,      95,  7.0285,  2.4258 },
        { BarrierOption::DownOut, 0.30,   110,      95,  5.4137,  2.6246 },
        { BarrierOption::DownOut, 0.30,    90,     100,  3.0000,  3.0000 },
        { BarrierOption::DownOut, 0.30,   100,     100,  3.0000,  3.0000 },
        { BarrierOption::DownOut, 0.30,   110,     100,  3.0000,  3.0000 },
        { BarrierOption::UpOut,   0.30,    90,     105,  2.6341,  4.2293 },
        { BarrierOption::UpOut,   0.30,   100,     105,  2.4389,  5.8032 },
        { BarrierOption::UpOut,   0.30,   110,     105,  2.4315,  7.5649 },

        { BarrierOption::DownIn,  0.30,    90,      95,  9.0093,  3.8769 },
        { BarrierOption::DownIn,  0.30,   100,      95,  5.1370,  7.7989 },
        { BarrierOption::DownIn,  0.30,   110,      95,  2.8517, 13.3078 },
        { BarrierOption::DownIn,  0.30,    90,     100, 14.8816,  3.3328 },
        { BarrierOption::DownIn,  0.30,   100,     100,  9.2045,  7.2636 },
        { BarrierOption::DownIn,  0.30,   110,     100,  5.3043, 12.9713 },
        { BarrierOption::UpIn,    0.30,    90,     105, 15.2098,  2.0658 },
        { BarrierOption::UpIn,    0.30,   100,     105,  9.7278,  4.4226 },
        { BarrierOption::UpIn,    0.30,   110,     105,  5.8350,  8.3686 }
    };

    for (int i=0; i<LENGTH(values); i++) {
        BarrierOption opCall(values[i].type, Option::Call, underlyingPrice,
                             values[i].strike, qRate, rRate, residualTime,
                             values[i].volatility, values[i].barrier, rebate);
        double calculated = opCall.value();
        double expected = values[i].callValue;
        if (QL_FABS(calculated-expected) > maxErrorAllowed)
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Call option:\n"
                    "    value:    " + 
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));

        BarrierOption opPut(values[i].type, Option::Put, underlyingPrice,
                            values[i].strike, qRate, rRate, residualTime,
                            values[i].volatility, values[i].barrier, rebate);
        calculated = opPut.value();
        expected = values[i].putValue;
        if (QL_FABS(calculated-expected) > maxErrorAllowed)
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Put option:\n"
                    "    value:    " + 
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));

        BarrierOption opStraddle(values[i].type, Option::Straddle, 
                                 underlyingPrice, values[i].strike, 
                                 qRate, rRate, residualTime,
                                 values[i].volatility, values[i].barrier, 
                                 rebate);
        calculated = opStraddle.value();
        expected = values[i].callValue+values[i].putValue;
        if (QL_FABS(calculated-expected) > maxStraddleErrorAllowed)
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Straddle:\n"
                    "    value:    " + 
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
    }
}

void OldPricerTest::testBinaryPricer() {

    std::map<std::string,double> calculated, expected, tolerance;
    tolerance["delta"]  = 5.0e-5;
    tolerance["gamma"]  = 5.0e-5;
    tolerance["theta"]  = 5.0e-5;
    tolerance["rho"]    = 5.0e-5;
    tolerance["divRho"] = 5.0e-5;
    tolerance["vega"]   = 5.0e-5;
    
    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    double underlyings[] = { 100 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Time residualTimes[] = { 1.0 };
    double strikes[] = { 50, 99.5, 100, 100.5, 150 };
    double volatilities[] = { 0.11, 0.5, 1.2 };

    for (int i1=0; i1<LENGTH(types); i1++) {
      for (int i2=0; i2<LENGTH(underlyings); i2++) {
        for (int i3=0; i3<LENGTH(rRates); i3++) {
          for (int i4=0; i4<LENGTH(qRates); i4++) {
            for (int i5=0; i5<LENGTH(residualTimes); i5++) {
              for (int i6=0; i6<LENGTH(strikes); i6++) {
                for (int i7=0; i7<LENGTH(volatilities); i7++) {
                  // test data
                  Option::Type type = types[i1];
                  double u = underlyings[i2];
                  Rate r = rRates[i3];
                  Rate q = qRates[i4];
                  Time T = residualTimes[i5];
                  double k = strikes[i6];
                  double v = volatilities[i7];
                  // increments
                  double dS = u*1.0e-4;
                  Time dT = T*1.0e-4;
                  double dV = v*1.0e-4;
                  Spread dR = r*1.0e-4;
                  Spread dQ = q*1.0e-4;

                  // reference option
                  BinaryOption opt(type,u,k,q,r,T,v);
                  if (opt.value() > 1.0e-6) {
                    // greeks
                    calculated["delta"]  = opt.delta();
                    calculated["gamma"]  = opt.gamma();
                    calculated["theta"]  = opt.theta();
                    calculated["rho"]    = opt.rho();
                    calculated["divRho"] = opt.dividendRho();
                    calculated["vega"]   = opt.vega();

                    // recalculate greeks numerically
                    BinaryOption optPs(type, u+dS, k, q   , r,    T ,   v);
                    BinaryOption optMs(type, u-dS, k, q   , r,    T ,   v);
                    BinaryOption optPt(type, u   , k, q   , r,    T+dT, v);
                    BinaryOption optMt(type, u   , k, q   , r,    T-dT, v);
                    BinaryOption optPr(type, u   , k, q   , r+dR, T   , v);
                    BinaryOption optMr(type, u   , k, q   , r-dR, T   , v);
                    BinaryOption optPq(type, u   , k, q+dQ, r   , T   , v);
                    BinaryOption optMq(type, u   , k, q-dQ, r   , T   , v);
                    BinaryOption optPv(type, u   , k, q   , r   , T   , v+dV);
                    BinaryOption optMv(type, u   , k, q   , r   , T   , v-dV);

                    expected["delta"]  =  (optPs.value()-optMs.value())/(2*dS);
                    expected["gamma"]  =  (optPs.delta()-optMs.delta())/(2*dS);
                    expected["theta"]  = -(optPt.value()-optMt.value())/(2*dT);
                    expected["rho"]    =  (optPr.value()-optMr.value())/(2*dR);
                    expected["divRho"] =  (optPq.value()-optMq.value())/(2*dQ);
                    expected["vega"]   =  (optPv.value()-optMv.value())/(2*dV);
                    
                    // check
                    std::map<std::string,double>::iterator it;
                    for (it = expected.begin(); it != expected.end(); ++it) {
                      std::string greek = it->first;
                      double expct = expected[greek];
                      double calcl = calculated[greek];
                      double tol = tolerance[greek];
                      if (relError(expct,calcl,u) > tol)
                          CPPUNIT_FAIL(
                              "Option details: \n"
                              "    type:           " +
                              typeToString(type) + "\n"
                              "    underlying:     " +
                              DoubleFormatter::toString(u) + "\n"
                              "    strike:         " +
                              DoubleFormatter::toString(k) + "\n"
                              "    dividend yield: " +
                              RateFormatter::toString(q) + "\n"
                              "    risk-free rate: " +
                              RateFormatter::toString(r) + "\n"
                              "    residual time:  " +
                              DoubleFormatter::toString(T) + "\n"
                              "    volatility:     " +
                              RateFormatter::toString(v) + "\n\n"
                              "    calculated " + greek + ": " +
                              DoubleFormatter::toString(calcl) + "\n"
                              "    expected:    " + 
                              std::string(greek.size(),' ') +
                              DoubleFormatter::toString(expct));
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
}

void OldPricerTest::testCliquetPricer() {
    double spot = 60.0;
    double moneyness = 1.1;
    std::vector<Spread> divYield(2);
    divYield[0] = 0.04; divYield[1] = 0.04;
    std::vector<Rate> rRate(2);
    rRate[0] = 0.08; rRate[1] = 0.08;
    std::vector<Time> dates(2);
    dates[0] = 0.25; dates[1] = 1.00;
    std::vector<double> vol(2);
    vol[0] = 0.30; vol[1] = 0.30;
    CliquetOption cliquet(Option::Call, spot, moneyness,
                          divYield, rRate, dates, vol);
    double calculated = cliquet.value();
    double expected = 4.4064; // Haug, p.37
    if (QL_FABS(calculated-expected) > 1.0e-4)
        CPPUNIT_FAIL(
            "calculated value: " +
            DoubleFormatter::toString(calculated) + "\n"
            "expected:         " +
            DoubleFormatter::toString(expected));
}

void OldPricerTest::testDividendEuropeanPricer() {

    Size nstp = 150;
    Size ngrd = nstp+1;
    std::vector<double> div(2);
    div[0] = 3.92; div[1] = 4.21;
    std::vector<Time> dates(2);
    dates[0] = 0.333; dates[1] = 0.667;

    std::map<std::string,double> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-4;
    tolerance["gamma"]  = 1.0e-4;
    tolerance["theta"]  = 1.0e-4;
    tolerance["rho"]    = 1.0e-4;
    tolerance["vega"]   = 1.0e-4;
    
    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    double underlyings[] = { 100 };
    Rate rRates[] = { 0.01, 0.10, 0.30 };
    Rate qRates[] = { 0.00, 0.05, 0.15 };
    Time residualTimes[] = { 1.0, 2.0 };
    double strikes[] = { 50, 99.5, 100, 100.5, 150 };
    double volatilities[] = { 0.04, 0.2, 0.7 };

    for (int i1=0; i1<LENGTH(types); i1++) {
      for (int i2=0; i2<LENGTH(underlyings); i2++) {
        for (int i3=0; i3<LENGTH(rRates); i3++) {
          for (int i4=0; i4<LENGTH(qRates); i4++) {
            for (int i5=0; i5<LENGTH(residualTimes); i5++) {
              for (int i6=0; i6<LENGTH(strikes); i6++) {
                for (int i7=0; i7<LENGTH(volatilities); i7++) {
                  // test data
                  Option::Type type = types[i1];
                  double u = underlyings[i2];
                  Rate r = rRates[i3];
                  Rate q = qRates[i4];
                  Time T = residualTimes[i5];
                  double k = strikes[i6];
                  double v = volatilities[i7];
                  // increments
                  double du = u*1.0e-4;
                  Time dT = T/nstp;
                  double dv = v*1.0e-4;
                  Spread dr = r*1.0e-4;

                  // reference option
                  FdDividendEuropeanOption opt(type,u,k,q,r,T,v,div,dates);
                  if (opt.value() > u*1.0e-5) {
                    // greeks
                    calculated["delta"]  = opt.delta();
                    calculated["gamma"]  = opt.gamma();
                    calculated["theta"]  = opt.theta();
                    calculated["rho"]    = opt.rho();
                    calculated["vega"]   = opt.vega();

                    // recalculate greeks numerically
                    std::vector<Time> datesP(2), datesM(2);
                    std::transform(dates.begin(),dates.end(),datesP.begin(),
                                   std::bind2nd(std::plus<Time>(),dT));
                    std::transform(dates.begin(),dates.end(),datesM.begin(),
                                   std::bind2nd(std::minus<Time>(),dT));
                    FdDividendEuropeanOption 
                        optPs(type, u+du, k, q, r,    T ,   v   , div, dates),
                        optMs(type, u-du, k, q, r,    T ,   v   , div, dates),
                        optPt(type, u   , k, q, r,    T+dT, v   , div, datesP),
                        optMt(type, u   , k, q, r,    T-dT, v   , div, datesM),
                        optPr(type, u   , k, q, r+dr, T   , v   , div, dates),
                        optMr(type, u   , k, q, r-dr, T   , v   , div, dates),
                        optPv(type, u   , k, q, r   , T   , v+dv, div, dates),
                        optMv(type, u   , k, q, r   , T   , v-dv, div, dates);

                    expected["delta"]  =  (optPs.value()-optMs.value())/(2*du);
                    expected["gamma"]  =  (optPs.delta()-optMs.delta())/(2*du);
                    expected["theta"]  = -(optPt.value()-optMt.value())/(2*dT);
                    expected["rho"]    =  (optPr.value()-optMr.value())/(2*dr);
                    expected["vega"]   =  (optPv.value()-optMv.value())/(2*dv);
                    
                    // check
                    std::map<std::string,double>::iterator it;
                    for (it = expected.begin(); it != expected.end(); ++it) {
                      std::string greek = it->first;
                      double expct = expected[greek];
                      double calcl = calculated[greek];
                      double tol = tolerance[greek];
                      if (relError(expct,calcl,u) > tol)
                          CPPUNIT_FAIL(
                              "Option details: \n"
                              "    type:           " +
                              typeToString(type) + "\n"
                              "    underlying:     " +
                              DoubleFormatter::toString(u) + "\n"
                              "    strike:         " +
                              DoubleFormatter::toString(k) + "\n"
                              "    dividend yield: " +
                              RateFormatter::toString(q) + "\n"
                              "    risk-free rate: " +
                              RateFormatter::toString(r) + "\n"
                              "    residual time:  " +
                              DoubleFormatter::toString(T) + "\n"
                              "    volatility:     " +
                              RateFormatter::toString(v) + "\n\n"
                              "    calculated " + greek + ": " +
                              DoubleFormatter::toString(calcl) + "\n"
                              "    expected:    " + 
                              std::string(greek.size(),' ') +
                              DoubleFormatter::toString(expct));
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
}

void OldPricerTest::testFdEuropeanPricer() {

    double under = 100.0;
    double strikeMin = 60.0, strikeRange = 100.0;
    Rate rRateMin = 0.0,  rRateRange = 0.18;
    Rate qRateMin = 0.0,  qRateRange = 0.02;
    double volMin = 0.0, volRange = 1.2;
    Time timeMin = 0.5, timeRange = 2.0;

    double tolerance = 1.0e-2;
    Size totCases = 200;

    UniformRandomGenerator rng(56789012);

    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };

    for (int i=0; i<totCases; i++) {

        double strike = strikeMin + strikeRange * rng.next().value;
        Rate qRate = qRateMin + qRateRange * rng.next().value;
        Rate rRate = rRateMin + rRateRange * rng.next().value;
        double vol = volMin + volRange * rng.next().value;
        Time resTime = timeMin + timeRange * rng.next().value;

        for (int j=0; j<LENGTH(types); j++) {

            double anValue = EuropeanOption(types[j], under, strike, 
                                            qRate, rRate, resTime, 
                                            vol).value();
            double numValue = FdEuropean(types[j], under, strike, 
                                         qRate, rRate, resTime, 
                                         vol, 100, 400).value();
            if (QL_FABS(anValue-numValue) > tolerance)
                CPPUNIT_FAIL(
                    "Option details: \n"
                    "    type:           " +
                    typeToString(types[j]) + "\n"
                    "    underlying:     " +
                    DoubleFormatter::toString(under) + "\n"
                    "    strike:         " +
                    DoubleFormatter::toString(strike) + "\n"
                    "    dividend yield: " +
                    RateFormatter::toString(qRate) + "\n"
                    "    risk-free rate: " +
                    RateFormatter::toString(rRate) + "\n"
                    "    residual time:  " +
                    DoubleFormatter::toString(resTime) + "\n"
                    "    volatility:     " +
                    RateFormatter::toString(vol) + "\n\n"
                    "    calculated value: " +
                    DoubleFormatter::toString(numValue) + "\n"
                    "    expected:         " + 
                    DoubleFormatter::toString(anValue));
        }
    }
}


                
