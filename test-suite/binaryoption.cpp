
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "binaryoption.hpp"
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
#include <ql/Instruments/binaryoption.hpp>
#include <ql/PricingEngines/binaryengines.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <map>

// This makes it easier to use array literals (alas, no std::vector literals)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace QuantLib;
using namespace QuantLib::PricingEngines;
using namespace QuantLib::Instruments;
using namespace QuantLib::TermStructures;
using namespace QuantLib::VolTermStructures;
using namespace QuantLib::DayCounters;

namespace {

    Handle<TermStructure> makeFlatCurve(const Handle<MarketElement>& forward) {
        Date today = Date::todaysDate();
        Calendar calendar = NullCalendar();
        Date reference = today;
        return Handle<TermStructure>(
            new FlatForward(today,reference,
                            RelinkableHandle<MarketElement>(forward),
                            SimpleDayCounter()));
    }

    Handle<BlackVolTermStructure> makeFlatVolatility(
                                     const Handle<MarketElement>& volatility) {
        Date today = Date::todaysDate();
        Calendar calendar = NullCalendar();
        Date reference = today;
        return Handle<BlackVolTermStructure>(
            new BlackConstantVol(reference,
                                 RelinkableHandle<MarketElement>(volatility),
                                 SimpleDayCounter()));
    }

    struct BinaryOptionData {
        Binary::Type type;
        Option::Type optionType;
        int years;
        double volatility;
        Rate rate;
        Rate dividendYield;
        double barrier;
        double rebate;
        double value;
    };

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

void BinaryOptionTest::testValues() {

    double maxErrorAllowed = 1.0e-4;
    double underlyingPrice = 100.0;
    Rate r = QL_LOG(1.09);
    Rate q = QL_LOG(1.03);


    BinaryOptionData values[] = {
        { Binary::CashAtExpiry, Option::Call, 1, 
          0.2, 0.05, 0.02, 110, 100.0, 35.283179 }
    };

    Handle<SimpleMarketElement> underlyingH_SME(
        new SimpleMarketElement(underlyingPrice));
    Handle<SimpleMarketElement> qH_SME(new SimpleMarketElement(q));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);
    Handle<SimpleMarketElement> rH_SME(new SimpleMarketElement(r));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);

    Handle<SimpleMarketElement> volatilityH_SME(new SimpleMarketElement(0.25));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatilityH_SME);

    Handle<MarketElement> underlyingH = underlyingH_SME;

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();

    for (Size i=0; i<LENGTH(values); i++) {
        volatilityH_SME->setValue(values[i].volatility);

        Date exDate = calendar.advance(today,values[i].years,Years);
        EuropeanExercise exercise(exDate);

        Instruments::BinaryOption binaryOption(
                values[i].type, 
                values[i].barrier, 
                values[i].rebate, 
                Option::Call, 
                RelinkableHandle<MarketElement>(underlyingH),
                RelinkableHandle<TermStructure>(qTS), 
                RelinkableHandle<TermStructure>(rTS),
                exercise, 
                RelinkableHandle<BlackVolTermStructure>(volTS));

        double calculated = binaryOption.NPV();
        double expected = values[i].value;
        if (QL_FABS(calculated-expected) > maxErrorAllowed) {
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Call option:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }
    }
}

void BinaryOptionTest::testAmericanValues() {

    double maxErrorAllowed = 1.0e-4;

    double underlyingPrice = 100.0;
    Rate r = 0.01;
    Rate q = 0.04;


    BinaryOptionData values[] = {
        { Binary::CashAtHit, Option::Call, 1, 
          0.11, 0.01, 0.04, 100.5, 100, 94.8825 },
        { Binary::CashAtHit, Option::Call, 1, 
          0.11, 0.01, 0.00, 100.5, 100, 96.5042 },
        { Binary::CashAtHit, Option::Call, 1, 
          0.11, 0.01, 0.04, 120,   100, 5.5676 },
        { Binary::CashAtHit, Option::Call, 1, 
          0.2,  0.01, 0.04, 100.5, 100, 97.3989 },
        { Binary::CashAtHit, Option::Call, 1, 
          0.11, 0.1,  0.04, 100.5, 100, 97.9405 },
        { Binary::CashAtHit, Option::Call, 2, 
          0.11, 0.01, 0.04, 100.5, 100, 95.8913 },
        { Binary::CashAtHit, Option::Put,  1, 
          0.11, 0.01, 0.04, 99.5,  100, 97.7331 },
        { Binary::CashAtHit, Option::Put,  1, 
          0.11, 0.01, 0.00, 99.5,  100, 96.1715 },
        { Binary::CashAtHit, Option::Put,  1, 
          0.11, 0.01, 0.04, 80,    100, 8.1172 },
        { Binary::CashAtHit, Option::Put,  1, 
          0.20, 0.01, 0.04, 99.5,  100, 98.6140 },
        { Binary::CashAtHit, Option::Put,  1, 
          0.11, 0.10, 0.04, 99.5,  100, 93.6491 },
        { Binary::CashAtHit, Option::Put,  2, 
          0.11, 0.01, 0.04, 99.5,  100, 98.7776 }
    };

    Handle<SimpleMarketElement> underlyingH_SME(
        new SimpleMarketElement(underlyingPrice));
    Handle<SimpleMarketElement> qH_SME(new SimpleMarketElement(q));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);
    Handle<SimpleMarketElement> rH_SME(new SimpleMarketElement(r));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);

    Handle<SimpleMarketElement> volatilityH_SME(new SimpleMarketElement(0.25));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatilityH_SME);

    Handle<MarketElement> underlyingH = underlyingH_SME;

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();

    for (Size i=0; i<LENGTH(values); i++) {
        volatilityH_SME->setValue(values[i].volatility);
        rH_SME->setValue(values[i].rate);
        qH_SME->setValue(values[i].dividendYield);

        Date exDate = calendar.advance(today,values[i].years,Years);
        AmericanExercise amExercise(today, exDate);

        Instruments::BinaryOption binaryOption(
                values[i].type, 
                values[i].barrier, 
                values[i].rebate, 
                values[i].optionType, 
                RelinkableHandle<MarketElement>(underlyingH),
                RelinkableHandle<TermStructure>(qTS), 
                RelinkableHandle<TermStructure>(rTS),
                amExercise, 
                RelinkableHandle<BlackVolTermStructure>(volTS));

        double calculated = binaryOption.NPV();
        double expected = values[i].value;
        if (QL_FABS(calculated-expected) > maxErrorAllowed) {
            CPPUNIT_FAIL(
                "Data at index " + IntegerFormatter::toString(i) + ", "
                "Call option:\n"
                    "    value:    " +
                    DoubleFormatter::toString(calculated) + "\n"
                    "    expected: " +
                    DoubleFormatter::toString(expected));
        }
    }
}

void BinaryOptionTest::testSelfConsistency() {

    std::map<std::string,double> calculated, expected, tolerance;
    tolerance["delta"]  = 5.0e-5;
    tolerance["gamma"]  = 5.0e-5;
    tolerance["theta"]  = 5.0e-5;
    tolerance["rho"]    = 5.0e-5;
    tolerance["divRho"] = 5.0e-5;
    tolerance["vega"]   = 5.0e-5;

    double rebate = 100.0;
    Binary::Type binaryTypes[] = { Binary::CashAtExpiry, Binary::CashAtHit };
    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    double underlyings[] = { 100 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Time residualTimes[] = { 1.0 };
    double strikes[] = { 50.0, 99.5, 100.5, 150.0 };
    double volatilities[] = { 0.11, 0.5, 1.2 };

    Handle<SimpleMarketElement> underlyingH_SME(
        new SimpleMarketElement(underlyings[0]));
    Handle<SimpleMarketElement> rH_SME(new SimpleMarketElement(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);
    Handle<SimpleMarketElement> qH_SME(new SimpleMarketElement(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);

    Handle<SimpleMarketElement> volatilityH_SME(new SimpleMarketElement(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatilityH_SME);

    Handle<MarketElement> underlyingH = underlyingH_SME;

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,1,Years);
    EuropeanExercise exercise(exDate);
    AmericanExercise amExercise(today, exDate);
    Exercise exercises[] = { exercise, amExercise };

    Handle<PricingEngine> euroEngine = Handle<PricingEngine>(
        new PricingEngines::AnalyticEuropeanBinaryEngine());

    Handle<PricingEngine> amEngine = Handle<PricingEngine>(
        new PricingEngines::AnalyticAmericanBinaryEngine());

    Handle<PricingEngine> engines[] = { euroEngine, amEngine };

    for (Size j=0; j<LENGTH(engines); j++) {
      for (Size i1=0; i1<LENGTH(types); i1++) {
        for (Size i2=0; i2<LENGTH(underlyings); i2++) {
          for (Size i3=0; i3<LENGTH(rRates); i3++) {
            for (Size i4=0; i4<LENGTH(qRates); i4++) {
              for (Size i6=0; i6<LENGTH(strikes); i6++) {
                for (Size i7=0; i7<LENGTH(volatilities); i7++) {
                  // test data
                  Option::Type type = types[i1];
                  double u = underlyings[i2];
                  Rate r = rRates[i3];
                  rH_SME->setValue(r);

                  Rate q = qRates[i4];
                  qH_SME->setValue(q);

                  Time T = residualTimes[0];
                  double k = strikes[i6];

                  double v = volatilities[i7];
                  volatilityH_SME->setValue(v);

                  // increments
                  double dS = u*1.0e-4;
//                  Time dT = T*1.0e-4;
//                  double dV = v*1.0e-4;
                  Spread dR = r*1.0e-4;
//                  Spread dQ = q*1.0e-4;

                  // reference option
                  Instruments::BinaryOption opt(
                      binaryTypes[j], 
                      k, 
                      rebate, 
                      type, 
                      RelinkableHandle<MarketElement>(underlyingH),
                      RelinkableHandle<TermStructure>(qTS), 
                      RelinkableHandle<TermStructure>(rTS),
                      exercises[j], 
                      RelinkableHandle<BlackVolTermStructure>(volTS), 
                      engines[j]);
                  if (opt.NPV() > 1.0e-6) {
                      // greeks
                      calculated["delta"]  = opt.delta();
                      //calculated["gamma"]  = opt.gamma();
                      //calculated["theta"]  = opt.theta();
                      calculated["rho"]    = opt.rho();
                      //calculated["divRho"] = opt.dividendRho();
                      //calculated["vega"]   = opt.vega();

                      // recalculate greeks numerically
                      // bump u up
                      underlyingH_SME->setValue(u+dS);
                      double optPsValue = opt.NPV();
//                      double optPsDelta = opt.delta();

                      // bump u down
                      underlyingH_SME->setValue(u-dS);
                      double optMsValue = opt.NPV();
//                      double optMsDelta = opt.delta();
                      underlyingH_SME->setValue(u);

                      // NOTE - Theta is more tricky
                      //BinaryOption optPt(type, u, k, q, r, T+dT, v, 1.0);
                      //underlyingH->setValue(u);
                      //double optPtValue = opt.NPV();
                      //BinaryOption optMt(type, u, k, q, r, T-dT, v, 1.0);

                      // bump r up
                      rH_SME->setValue(r+dR);
                      double optPrValue = opt.NPV();

                      // bump r down
                      rH_SME->setValue(r-dR);
                      double optMrValue = opt.NPV();
                      rH_SME->setValue(r);

                      // bump q up
//                      rH_SME->setValue(q+dQ);
//                      double optPqValue = opt.NPV();

                      // bump q down
//                      rH_SME->setValue(q-dQ);
//                      double optMqValue = opt.NPV();
//                      rH_SME->setValue(q);

                      // bump v up
//                      volatilityH_SME->setValue(v+dV);
//                      double optPvValue = opt.NPV();

                      // bump v down
//                      volatilityH_SME->setValue(v-dV);
//                      double optMvValue = opt.NPV();
//                      volatilityH_SME->setValue(v);

                      expected["delta"]  =  (optPsValue-optMsValue)/(2*dS);
                      //expected["gamma"]  =  (optPsDelta-optMsDelta)/(2*dS);
                      //expected["theta"]  = -(optPtValue-optMtValue)/(2*dT);
                      expected["rho"]    =  (optPrValue-optMrValue)/(2*dR);
                      //expected["divRho"] =  (optPqValue-optMqValue)/(2*dQ);
                      //expected["vega"]   =  (optPvValue-optMvValue)/(2*dV);

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

void BinaryOptionTest::testEngineConsistency() {

    double calcAnalytic, calcMC;
    double tolerance = 1.0e-1;

    Size maxTimeStepsPerYear = 10;
    bool antitheticVariate = false;
    bool controlVariate = false;
    Size requiredSamples = 1023;
    double requiredTolerance = 0.05;
    Size maxSamples = 1000000;
    bool isBiased = false;
    long seed = 1;

    double cashPayoff = 100.0;
    //Binary::Type binaryTypes[] = { Binary::CashAtExpiry, Binary::CashAtHit};
    Binary::Type binaryTypes[] = { Binary::CashAtHit };
    //Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    Option::Type types[] = { Option::Call };
    double underlyings[] = { 100 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Time residualTimes[] = { 1.0 };
    //double barriers[] = { 50, 99.5, 100, 100.5, 150 };
    double barriers[] = { 100.5, 150 };
    //double strikes[] = { 50, 99.5 };
    double volatilities[] = { 0.11, 0.5, 1.2 };

    Handle<SimpleMarketElement> underlyingH_SME(
        new SimpleMarketElement(underlyings[0]));
    Handle<SimpleMarketElement> rH_SME(new SimpleMarketElement(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);
    Handle<SimpleMarketElement> qH_SME(new SimpleMarketElement(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);

    Handle<SimpleMarketElement> volatilityH_SME(new SimpleMarketElement(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatilityH_SME);

    Handle<MarketElement> underlyingH = underlyingH_SME;

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,1,Years);
    EuropeanExercise exercise(exDate);
    AmericanExercise amExercise(today, exDate);
    //Exercise exercises[] = {exercise, amExercise};
    Exercise exercises[] = {amExercise};

    Handle<PricingEngine> euroEngine = Handle<PricingEngine>(
        new PricingEngines::AnalyticEuropeanBinaryEngine());

    Handle<PricingEngine> amEngine = Handle<PricingEngine>(
        new PricingEngines::AnalyticAmericanBinaryEngine());

    Handle<PricingEngine> mcEngine = Handle<PricingEngine>(
        new PricingEngines::MCBinaryEngine
            <MonteCarlo::PseudoRandom, Math::Statistics>
                  (maxTimeStepsPerYear, antitheticVariate, controlVariate,
                  requiredSamples, requiredTolerance,
                  maxSamples, isBiased, seed));

    //Handle<PricingEngine> engines[] = {euroEngine, amEngine};
    Handle<PricingEngine> engines[] = {amEngine};

    for (Size j=0; j<LENGTH(engines); j++) {
      for (Size i1=0; i1<LENGTH(types); i1++) {
        for (Size i2=0; i2<LENGTH(underlyings); i2++) {
          for (Size i3=0; i3<LENGTH(rRates); i3++) {
            for (Size i4=0; i4<LENGTH(qRates); i4++) {
              for (Size i6=0; i6<LENGTH(barriers); i6++) {
                for (Size i7=0; i7<LENGTH(volatilities); i7++) {
                  // test data
                  Option::Type type = types[i1];
                  double u = underlyings[i2];
                  Rate r = rRates[i3];
                  rH_SME->setValue(r);

                  Rate q = qRates[i4];
                  qH_SME->setValue(q);

                  Time T = residualTimes[0];
                  double barrier = barriers[i6];

                  double v = volatilities[i7];
                  volatilityH_SME->setValue(v);

                  // reference option
                  Instruments::BinaryOption opt(
                      binaryTypes[j], 
                      barrier, 
                      cashPayoff, 
                      type, 
                      RelinkableHandle<MarketElement>(underlyingH),
                      RelinkableHandle<TermStructure>(qTS), 
                      RelinkableHandle<TermStructure>(rTS),
                      exercises[j], 
                      RelinkableHandle<BlackVolTermStructure>(volTS), 
                      engines[j]);
                  calcAnalytic = opt.NPV();

                  opt.setPricingEngine(mcEngine);
                  calcMC = opt.NPV();

                  //std::cout << "\nAnalytic: " +
                  //    DoubleFormatter::toString(calcAnalytic) + 
                  //    "   MC: " + DoubleFormatter::toString(calcMC) 
                  //    << std::endl; 

                  // check                
                  if (relError(calcAnalytic,calcMC,u) > tolerance) {
                      CPPUNIT_FAIL(
                          "Option details: \n"
                          "    type:           " +
                          typeToString(type) + "\n"
                          "    underlying:     " +
                          DoubleFormatter::toString(u) + "\n"
                          "    barrier:        " +
                          DoubleFormatter::toString(barrier) + "\n"
                          "    payoff:         " +
                          DoubleFormatter::toString(cashPayoff) + "\n"
                          "    dividend yield: " +
                          RateFormatter::toString(q) + "\n"
                          "    risk-free rate: " +
                          RateFormatter::toString(r) + "\n"
                          "    residual time:  " +
                          DoubleFormatter::toString(T) + "\n"
                          "    volatility:     " +
                          RateFormatter::toString(v) + "\n\n"
                          "    MonteCarlo: " +
                          DoubleFormatter::toString(calcMC) + "\n"
                          "    Analytic: " +
                          DoubleFormatter::toString(calcAnalytic));
                  }
                }
              }
            }
          }
        }
      }
    }
}


CppUnit::Test* BinaryOptionTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Binary option tests");
    tests->addTest(new CppUnit::TestCaller<BinaryOptionTest>
                   ("Testing European binary option",
                    &BinaryOptionTest::testValues));
    tests->addTest(new CppUnit::TestCaller<BinaryOptionTest>
                   ("Testing American binary option",
                    &BinaryOptionTest::testAmericanValues));
    tests->addTest(new CppUnit::TestCaller<BinaryOptionTest>
                   ("Testing binary option greeks",
                    &BinaryOptionTest::testSelfConsistency));
    tests->addTest(new CppUnit::TestCaller<BinaryOptionTest>
                   ("Testing Monte Carlo pricing engine for binary options",
                    &BinaryOptionTest::testEngineConsistency));
    return tests;
}

