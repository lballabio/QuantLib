
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

#include "binarybarrieroption.hpp"
#include "utilities.hpp"
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
#include <ql/Instruments/binarybarrieroption.hpp>
#include <ql/PricingEngines/Barrier/analyticamericanbinarybarrierengine.hpp>
#include <ql/PricingEngines/Barrier/analyticeuropeanbinarybarrierengine.hpp>
#include <ql/PricingEngines/Barrier/mcbinarybarrierengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <map>

using namespace QuantLib;

namespace {

    struct BinaryBarrierOptionData {
        Option::Type optionType;
        int years;
        double volatility;
        Rate rate;
        Rate dividendYield;
        double barrier;
        double rebate;
        double value;
    };

}

void BinaryBarrierOptionTest::testValues() {

    double maxErrorAllowed = 1.0e-4;
    double underlyingPrice = 100.0;
    Rate r = QL_LOG(1.09);
    Rate q = QL_LOG(1.03);


    BinaryBarrierOptionData values[] = {
        { Option::Call, 1,
          0.2, 0.05, 0.02, 110, 100.0, 35.283179 }
    };

    Handle<SimpleQuote> underlyingH_SME(new SimpleQuote(underlyingPrice));
    Handle<SimpleQuote> qH_SME(new SimpleQuote(q));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);
    Handle<SimpleQuote> rH_SME(new SimpleQuote(r));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);

    Handle<SimpleQuote> volatilityH_SME(new SimpleQuote(0.25));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatilityH_SME);

    Handle<Quote> underlyingH = underlyingH_SME;

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();

    for (Size i=0; i<LENGTH(values); i++) {
        volatilityH_SME->setValue(values[i].volatility);

        Date exDate = calendar.advance(today,values[i].years,Years);
        Handle<Exercise> exercise(new EuropeanExercise(exDate));
        Handle<PricingEngine> engine(new AnalyticEuropeanBinaryBarrierEngine);

        Handle<CashOrNothingPayoff> payoff(new CashOrNothingPayoff(
            Option::Call, values[i].barrier, values[i].rebate));

        Handle<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(underlyingH),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        BinaryBarrierOption binaryBarrierOption(stochProcess, payoff, exercise,
            engine);

        double calculated = binaryBarrierOption.NPV();
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

void BinaryBarrierOptionTest::testAmericanValues() {

    double maxErrorAllowed = 1.0e-4;

    double underlyingPrice = 100.0;
    Rate r = 0.01;
    Rate q = 0.04;


    // it was cashAtHit
    BinaryBarrierOptionData values[] = {
        { Option::Call, 1,
          0.11, 0.01, 0.04, 100.5, 100, 94.8825 },
        { Option::Call, 1,
          0.11, 0.01, 0.00, 100.5, 100, 96.5042 },
        { Option::Call, 1,
          0.11, 0.01, 0.04, 120,   100, 5.5676 },
        { Option::Call, 1,
          0.2,  0.01, 0.04, 100.5, 100, 97.3989 },
        { Option::Call, 1,
          0.11, 0.1,  0.04, 100.5, 100, 97.9405 },
        { Option::Call, 2,
          0.11, 0.01, 0.04, 100.5, 100, 95.8913 },
        { Option::Put,  1,
          0.11, 0.01, 0.04, 99.5,  100, 97.7331 },
        { Option::Put,  1,
          0.11, 0.01, 0.00, 99.5,  100, 96.1715 },
        { Option::Put,  1,
          0.11, 0.01, 0.04, 80,    100, 8.1172 },
        { Option::Put,  1,
          0.20, 0.01, 0.04, 99.5,  100, 98.6140 },
        { Option::Put,  1,
          0.11, 0.10, 0.04, 99.5,  100, 93.6491 },
        { Option::Put,  2,
          0.11, 0.01, 0.04, 99.5,  100, 98.7776 }
    };

    Handle<SimpleQuote> underlyingH_SME(new SimpleQuote(underlyingPrice));
    Handle<SimpleQuote> qH_SME(new SimpleQuote(q));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);
    Handle<SimpleQuote> rH_SME(new SimpleQuote(r));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);

    Handle<SimpleQuote> volatilityH_SME(new SimpleQuote(0.25));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatilityH_SME);

    Handle<Quote> underlyingH = underlyingH_SME;

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();

    for (Size i=0; i<LENGTH(values); i++) {
        volatilityH_SME->setValue(values[i].volatility);
        rH_SME->setValue(values[i].rate);
        qH_SME->setValue(values[i].dividendYield);

        Date exDate = calendar.advance(today,values[i].years,Years);
        Handle<Exercise> amExercise(new AmericanExercise(today, exDate));
        Handle<PricingEngine> engine(new AnalyticAmericanBinaryBarrierEngine);

        Handle<CashOrNothingPayoff> payoff(new CashOrNothingPayoff(
            values[i].optionType, values[i].barrier, values[i].rebate));

        Handle<BlackScholesStochasticProcess> stochProcess(new
            BlackScholesStochasticProcess(
                RelinkableHandle<Quote>(underlyingH),
                RelinkableHandle<TermStructure>(qTS),
                RelinkableHandle<TermStructure>(rTS),
                RelinkableHandle<BlackVolTermStructure>(volTS)));

        BinaryBarrierOption binaryBarrierOption(stochProcess,
            payoff,
            amExercise,
            engine);

        double calculated = binaryBarrierOption.NPV();
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

void BinaryBarrierOptionTest::testSelfConsistency() {

    std::map<std::string,double> calculated, expected, tolerance;
    tolerance["delta"]  = 5.0e-5;
    tolerance["gamma"]  = 5.0e-5;
    tolerance["theta"]  = 5.0e-5;
    tolerance["rho"]    = 5.0e-5;
    tolerance["divRho"] = 5.0e-5;
    tolerance["vega"]   = 5.0e-5;

    double rebate = 100.0;
    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    double underlyings[] = { 100 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Time residualTimes[] = { 1.0 };
    double strikes[] = { 50.0, 99.5, 100.5, 150.0 };
    double volatilities[] = { 0.11, 0.5, 1.2 };

    Handle<SimpleQuote> underlyingH_SME(new SimpleQuote(underlyings[0]));
    Handle<SimpleQuote> rH_SME(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);
    Handle<SimpleQuote> qH_SME(new SimpleQuote(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);

    Handle<SimpleQuote> volatilityH_SME(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatilityH_SME);

    Handle<Quote> underlyingH = underlyingH_SME;

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,1,Years);
    Handle<Exercise> exercise(new EuropeanExercise(exDate));
    Handle<Exercise> amExercise(new AmericanExercise(today, exDate, false));
    Handle<Exercise> exercises[] = { exercise, amExercise };

    Handle<PricingEngine> euroEngine = Handle<PricingEngine>(
        new AnalyticEuropeanBinaryBarrierEngine());

    Handle<PricingEngine> amEngine = Handle<PricingEngine>(
        new AnalyticAmericanBinaryBarrierEngine());

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

                  Handle<CashOrNothingPayoff> payoff(
                      new CashOrNothingPayoff(type, k, rebate));

                  Handle<BlackScholesStochasticProcess> stochProcess(new
                        BlackScholesStochasticProcess(
                            RelinkableHandle<Quote>(underlyingH),
                            RelinkableHandle<TermStructure>(qTS),
                            RelinkableHandle<TermStructure>(rTS),
                            RelinkableHandle<BlackVolTermStructure>(volTS)));

                  // reference option
                  BinaryBarrierOption opt(stochProcess,
                      payoff,
                      exercises[j],
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
                      //BinaryBarrierOption optPt(type, u, k, q, r, T+dT, v, 1.0);
                      //underlyingH->setValue(u);
                      //double optPtValue = opt.NPV();
                      //BinaryBarrierOption optMt(type, u, k, q, r, T-dT, v, 1.0);

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
                          if (relativeError(expct,calcl,u) > tol)
                              CPPUNIT_FAIL(
                                  "Option details: \n"
                                  "    type:           " +
                                  OptionTypeFormatter::toString(type)
                                  + "\n"
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

void BinaryBarrierOptionTest::testEngineConsistency() {

    double calcAnalytic, calcMC;
    double tolerance = 1.0e-1;

    Size maxTimeStepsPerYear = 10;
    bool antitheticVariate = false;
    bool controlVariate = false;
    Size requiredSamples = 1023;
    double requiredTolerance = 0.05;
    Size maxSamples = 1000000;
    // bool isBiased = false;
    long seed = 1;

    double cashPayoff = 100.0;
    Option::Type types[] = { Option::Call };
    double underlyings[] = { 100 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Time residualTimes[] = { 1.0 };
    //double barriers[] = { 50, 99.5, 100, 100.5, 150 };
    double barriers[] = { 100.5, 150 };
    //double strikes[] = { 50, 99.5 };
    double volatilities[] = { 0.11, 0.5, 1.2 };

    Handle<SimpleQuote> underlyingH_SME(new SimpleQuote(underlyings[0]));
    Handle<SimpleQuote> rH_SME(new SimpleQuote(0.0));
    Handle<TermStructure> rTS = makeFlatCurve(rH_SME);
    Handle<SimpleQuote> qH_SME(new SimpleQuote(0.0));
    Handle<TermStructure> qTS = makeFlatCurve(qH_SME);

    Handle<SimpleQuote> volatilityH_SME(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS = makeFlatVolatility(volatilityH_SME);

    Handle<Quote> underlyingH = underlyingH_SME;

    Date today = Date::todaysDate();
    Calendar calendar = NullCalendar();
    Date exDate = calendar.advance(today,1,Years);
    Handle<Exercise> exercise(new EuropeanExercise(exDate));
    Handle<Exercise> amExercise(new AmericanExercise(today, exDate));
    Handle<Exercise> exercises[] = {amExercise};

    Handle<PricingEngine> euroEngine = Handle<PricingEngine>(
        new AnalyticEuropeanBinaryBarrierEngine());

    Handle<PricingEngine> amEngine = Handle<PricingEngine>(
        new AnalyticAmericanBinaryBarrierEngine());

    Handle<PricingEngine> mcEngine = Handle<PricingEngine>(
        new MCBinaryBarrierEngine<PseudoRandom, Statistics>
                  (maxTimeStepsPerYear, antitheticVariate, controlVariate,
                  requiredSamples, requiredTolerance,
                  maxSamples, seed));

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

                  Handle<CashOrNothingPayoff> payoff(new CashOrNothingPayoff(
                      type, barrier, cashPayoff));

                  Handle<BlackScholesStochasticProcess> stochProcess(new
                    BlackScholesStochasticProcess(
                        RelinkableHandle<Quote>(underlyingH),
                        RelinkableHandle<TermStructure>(qTS),
                        RelinkableHandle<TermStructure>(rTS),
                        RelinkableHandle<BlackVolTermStructure>(volTS)));

                  // reference option
                  BinaryBarrierOption opt(stochProcess,
                      payoff,
                      exercises[j],
                      engines[j]);
                  calcAnalytic = opt.NPV();

                  opt.setPricingEngine(mcEngine);
                  calcMC = opt.NPV();

                  // check
                  if (relativeError(calcAnalytic,calcMC,u) > tolerance) {
                      CPPUNIT_FAIL(
                          "Option details: \n"
                          "    type:           " +
                          OptionTypeFormatter::toString(type) + "\n"
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


/*! \deprecated use DigitalOptionTest instead. */
CppUnit::Test* BinaryBarrierOptionTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("BinaryBarrier option tests");
    tests->addTest(new CppUnit::TestCaller<BinaryBarrierOptionTest>
                   ("Testing European binary barrier option",
                    &BinaryBarrierOptionTest::testValues));
    tests->addTest(new CppUnit::TestCaller<BinaryBarrierOptionTest>
                   ("Testing American binary barrier option",
                    &BinaryBarrierOptionTest::testAmericanValues));
    tests->addTest(new CppUnit::TestCaller<BinaryBarrierOptionTest>
                   ("Testing binary barrier option greeks",
                   &BinaryBarrierOptionTest::testSelfConsistency));

    tests->addTest(new CppUnit::TestCaller<BinaryBarrierOptionTest>
                   ("Testing Monte Carlo pricing engine "
                    "for binary barrier options",
                    &BinaryBarrierOptionTest::testEngineConsistency));

    return tests;
}

