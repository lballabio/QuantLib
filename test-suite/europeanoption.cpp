
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

#include "europeanoption.hpp"
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <map>
#include <string>

// This makes it easier to use array literals (alas, no std::vector literals)
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

using namespace QuantLib;
using QuantLib::PricingEngines::EuropeanAnalyticalEngine;
using QuantLib::PricingEngines::EuropeanBinomialEngine;
using QuantLib::Instruments::VanillaOption;
using QuantLib::TermStructures::FlatForward;
using QuantLib::VolTermStructures::BlackConstantVol;
using QuantLib::DayCounters::Actual365;
using QuantLib::Calendars::TARGET;

double EuropeanOptionTest::relativeError(double x1, double x2, 
                                         double reference) {
    if (reference != 0.0)
        return QL_FABS(x1-x2)/reference;
    else
        return 1.0e+10;
}

Handle<Instrument> EuropeanOptionTest::makeEuropeanOption(
        Option::Type type,
        const Handle<MarketElement>& underlying,
        double strike,
        const Handle<TermStructure>& divCurve,
        const Handle<TermStructure>& rfCurve,
        const Date& exDate,
        const Handle<BlackVolTermStructure>& volatility,
        EuropeanOptionTest::EngineType engineType) {
    Handle<PricingEngine> engine;
    switch (engineType) {
      case Analytic:
        engine = Handle<PricingEngine>(new EuropeanAnalyticalEngine);
        break;
      case JR:
        engine = Handle<PricingEngine>(
            new EuropeanBinomialEngine(
                EuropeanBinomialEngine::JarrowRudd, 800));
        break;
      case CRR:
        engine = Handle<PricingEngine>(
            new EuropeanBinomialEngine(
                EuropeanBinomialEngine::CoxRossRubinstein, 800));
        break;
    }
    
    return Handle<Instrument>(
        new VanillaOption(type,
                          RelinkableHandle<MarketElement>(underlying),
                          strike,
                          RelinkableHandle<TermStructure>(divCurve), 
                          RelinkableHandle<TermStructure>(rfCurve),
                          EuropeanExercise(exDate),
                          RelinkableHandle<BlackVolTermStructure>(volatility),
                          engine));
}

Handle<TermStructure> EuropeanOptionTest::makeFlatCurve(
            const Handle<MarketElement>& forward) {
    Date today = Date::todaysDate();
    Calendar calendar = TARGET();
    Date reference = calendar.advance(today,2,Days);
    return Handle<TermStructure>(
        new FlatForward(today,reference,
                        RelinkableHandle<MarketElement>(forward),
                        Actual365()));
}

Handle<BlackVolTermStructure> EuropeanOptionTest::makeFlatVolatility(
            const Handle<MarketElement>& volatility) {
    Date today = Date::todaysDate();
    Calendar calendar = TARGET();
    Date reference = calendar.advance(today,2,Days);
    return Handle<BlackVolTermStructure>(
        new BlackConstantVol(reference,
                             RelinkableHandle<MarketElement>(volatility),
                             Actual365()));
}

std::string EuropeanOptionTest::typeToString(Option::Type type) {
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


void EuropeanOptionTest::testGreeks() {

    std::map<std::string,double> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-4;
    tolerance["gamma"]  = 1.0e-4;
    tolerance["theta"]  = 1.0e-2;
    tolerance["rho"]    = 1.0e-4;
    tolerance["divRho"] = 1.0e-4;
    tolerance["vega"]   = 1.0e-4;

    // test options
    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    double strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    int lengths[] = { 2 };
    
    // test data
    double underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    double vols[] = { 0.11, 0.50, 1.20 };

    Handle<SimpleMarketElement> underlying(new SimpleMarketElement(0.0));
    Handle<SimpleMarketElement> volatility(new SimpleMarketElement(0.0));
    Handle<BlackVolTermStructure> volCurve = makeFlatVolatility(volatility);
    Handle<SimpleMarketElement> qRate(new SimpleMarketElement(0.0));
    Handle<TermStructure> divCurve = makeFlatCurve(qRate);
    Handle<SimpleMarketElement> rRate(new SimpleMarketElement(0.0));
    Handle<TermStructure> rfCurve = makeFlatCurve(rRate);

    Date today = Date::todaysDate();
    Calendar calendar = TARGET();

    for (int i=0; i<LENGTH(types); i++) {
      for (int j=0; j<LENGTH(strikes); j++) {
        for (int k=0; k<LENGTH(lengths); k++) {
          // option to check
          Date exDate = calendar.advance(today,lengths[k],Years);
          Handle<VanillaOption> option = 
              makeEuropeanOption(types[i],underlying,strikes[j],
                                 divCurve,rfCurve,exDate,volCurve);
          // time-shifted exercise dates and options
          Date exDateP = calendar.advance(exDate,1,Days),
               exDateM = calendar.advance(exDate,-1,Days);
          Time dT = (exDateP-exDateM)/365.0;
          Handle<VanillaOption> optionP = 
              makeEuropeanOption(types[i],underlying,strikes[j],
                                 divCurve,rfCurve,exDateP,volCurve);
          Handle<VanillaOption> optionM = 
              makeEuropeanOption(types[i],underlying,strikes[j],
                                 divCurve,rfCurve,exDateM,volCurve);

          for (int l=0; l<LENGTH(underlyings); l++) {
            for (int m=0; m<LENGTH(qRates); m++) {
              for (int n=0; n<LENGTH(rRates); n++) {
                for (int p=0; p<LENGTH(vols); p++) {
                  double u = underlyings[l],
                         q = qRates[m],
                         r = rRates[n],
                         v = vols[p];
                  underlying->setValue(u);
                  qRate->setValue(q);
                  rRate->setValue(r);
                  volatility->setValue(v);
                  
                  double value = option->NPV();
                  calculated["delta"]  = option->delta();
                  calculated["gamma"]  = option->gamma();
                  calculated["theta"]  = option->theta();
                  calculated["rho"]    = option->rho();
                  calculated["divRho"] = option->dividendRho();
                  calculated["vega"]   = option->vega();
                  
                  if (value > underlying->value()*1.0e-5) {
                      // perturb underlying and get delta and gamma
                      double du = u*1.0e-4;
                      underlying->setValue(u+du);
                      double value_p = option->NPV(),
                             delta_p = option->delta();
                      underlying->setValue(u-du);
                      double value_m = option->NPV(),
                             delta_m = option->delta();
                      underlying->setValue(u);
                      expected["delta"] = (value_p - value_m)/(2*du);
                      expected["gamma"] = (delta_p - delta_m)/(2*du);
                      
                      // perturb rates and get rho and dividend rho
                      double dr = r*1.0e-4;
                      rRate->setValue(r+dr);
                      value_p = option->NPV();
                      rRate->setValue(r-dr);
                      value_m = option->NPV();
                      rRate->setValue(r);
                      expected["rho"] = (value_p - value_m)/(2*dr);

                      double dq = q*1.0e-4;
                      qRate->setValue(q+dq);
                      value_p = option->NPV();
                      qRate->setValue(q-dq);
                      value_m = option->NPV();
                      qRate->setValue(q);
                      expected["divRho"] = (value_p - value_m)/(2*dq);

                      // perturb volatility and get vega
                      double dv = v*1.0e-4;
                      volatility->setValue(v+dv);
                      value_p = option->NPV();
                      volatility->setValue(v-dv);
                      value_m = option->NPV();
                      volatility->setValue(v);
                      expected["vega"] = (value_p - value_m)/(2*dv);

                      // get theta from time-shifted options
                      expected["theta"] = (optionM->NPV() - optionP->NPV())/dT;

                      // compare
                      std::map<std::string,double>::iterator it;
                      for (it = calculated.begin(); 
                           it != calculated.end(); ++it) {
                          std::string greek = it->first;
                          double calcl = calculated[greek],
                                 expct = expected[greek],
                                 tol = tolerance[greek];
                          if (relativeError(calcl,expct,u) > tol) {
                              CPPUNIT_FAIL(
                                  typeToString(types[i]) + " option :\n"
                                  "    underlying value: "
                                  + DoubleFormatter::toString(u) + "\n"
                                  "    strike:           "
                                  + DoubleFormatter::toString(strikes[j]) +"\n"
                                  "    dividend yield:   "
                                  + DoubleFormatter::toString(q) + "\n"
                                  "    risk-free rate:   "
                                  + DoubleFormatter::toString(r) + "\n"
                                  "    maturity:         "
                                  + DateFormatter::toString(exDate) + "\n"
                                  "    volatility:       "
                                  + DoubleFormatter::toString(v) + "\n\n"
                                  "    calculated " + greek + ": "
                                  + DoubleFormatter::toString(calcl) + "\n"
                                  "    expected " + greek + ":   "
                                  + DoubleFormatter::toString(expct));
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
}

void EuropeanOptionTest::testImpliedVol() {
    Size maxEvaluations = 100;
    double tolerance = 1.0e-6;

    // test options
    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    double strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    int lengths[] = { 36, 180, 360, 1080 };
    
    // test data
    double underlyings[] = { 80.0, 95.0, 99.9, 100.0, 100.1, 105.0, 120.0 };
    Rate qRates[] = { 0.01, 0.05, 0.10 };
    Rate rRates[] = { 0.01, 0.05, 0.10 };
    double vols[] = { 0.01, 0.20, 0.30, 0.70, 0.90 };
    
    Handle<SimpleMarketElement> underlying(new SimpleMarketElement(0.0));
    Handle<SimpleMarketElement> volatility(new SimpleMarketElement(0.0));
    Handle<BlackVolTermStructure> volCurve = makeFlatVolatility(volatility);
    Handle<SimpleMarketElement> qRate(new SimpleMarketElement(0.0));
    Handle<TermStructure> divCurve = makeFlatCurve(qRate);
    Handle<SimpleMarketElement> rRate(new SimpleMarketElement(0.0));
    Handle<TermStructure> rfCurve = makeFlatCurve(rRate);

    Date today = Date::todaysDate();

    for (int i=0; i<LENGTH(types); i++) {
      for (int j=0; j<LENGTH(strikes); j++) {
        for (int k=0; k<LENGTH(lengths); k++) {
          // option to check
          Date exDate = today.plusDays(lengths[k]);
          Handle<VanillaOption> option = 
              makeEuropeanOption(types[i],underlying,strikes[j],
                                 divCurve,rfCurve,exDate,volCurve);
          
          for (int l=0; l<LENGTH(underlyings); l++) {
            for (int m=0; m<LENGTH(qRates); m++) {
              for (int n=0; n<LENGTH(rRates); n++) {
                for (int p=0; p<LENGTH(vols); p++) {
                  double u = underlyings[l],
                         q = qRates[m],
                         r = rRates[n],
                         v = vols[p];
                  underlying->setValue(u);
                  qRate->setValue(q);
                  rRate->setValue(r);
                  volatility->setValue(v);
                  
                  double value = option->NPV();
                  double implVol;
                  if (value != 0.0) {
                      // shift guess somehow
                      volatility->setValue(v*1.5);
                      try {
                          implVol = option->impliedVolatility(value,
                                                              tolerance,
                                                              maxEvaluations);
                      } catch (std::exception& e) {
                          CPPUNIT_FAIL(
                              typeToString(types[i]) + " option :\n"
                              "    underlying value: "
                              + DoubleFormatter::toString(u) + "\n"
                              "    strike:           "
                              + DoubleFormatter::toString(strikes[j]) +"\n"
                              "    dividend yield:   "
                              + DoubleFormatter::toString(q) + "\n"
                              "    risk-free rate:   "
                              + DoubleFormatter::toString(r) + "\n"
                              "    maturity:         "
                              + DateFormatter::toString(exDate) + "\n"
                              "    volatility:       "
                              + DoubleFormatter::toString(v) + "\n\n"
                              + std::string(e.what()));
                      }
                      if (QL_FABS(implVol-v) > tolerance) {
                          // the difference might not matter
                          volatility->setValue(implVol);
                          double value2 = option->NPV();
                          if (relativeError(value,value2,u) > tolerance) {
                              CPPUNIT_FAIL(
                                  typeToString(types[i]) + " option :\n"
                                  "    underlying value: "
                                  + DoubleFormatter::toString(u) + "\n"
                                  "    strike:           "
                                  + DoubleFormatter::toString(strikes[j]) +"\n"
                                  "    dividend yield:   "
                                  + DoubleFormatter::toString(q) + "\n"
                                  "    risk-free rate:   "
                                  + DoubleFormatter::toString(r) + "\n"
                                  "    maturity:         "
                                  + DateFormatter::toString(exDate) + "\n\n"
                                  "    original volatility: "
                                  + DoubleFormatter::toString(v) + "\n"
                                  "    price:               "
                                  + DoubleFormatter::toString(value) + "\n"
                                  "    implied volatility:  "
                                  + DoubleFormatter::toString(implVol) + "\n"
                                  "    corresponding price: "
                                  + DoubleFormatter::toString(value2));
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
}

void EuropeanOptionTest::testBinomialEngines() {
    double tolerance = 0.1;

    // test options
    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    double strikes[] = { 50.0, 100.0, 150.0 };
    int lengths[] = { 1 };
    
    // test data
    double underlyings[] = { 100.0 };
    Rate qRates[] = { 0.00, 0.05 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    double vols[] = { 0.11, 0.50, 1.20 };
    
    Handle<SimpleMarketElement> underlying(new SimpleMarketElement(0.0));
    Handle<SimpleMarketElement> volatility(new SimpleMarketElement(0.0));
    Handle<BlackVolTermStructure> volCurve = makeFlatVolatility(volatility);
    Handle<SimpleMarketElement> qRate(new SimpleMarketElement(0.0));
    Handle<TermStructure> divCurve = makeFlatCurve(qRate);
    Handle<SimpleMarketElement> rRate(new SimpleMarketElement(0.0));
    Handle<TermStructure> rfCurve = makeFlatCurve(rRate);

    Date today = Date::todaysDate();
    Calendar calendar = TARGET();

    for (int i=0; i<LENGTH(types); i++) {
      for (int j=0; j<LENGTH(strikes); j++) {
        for (int k=0; k<LENGTH(lengths); k++) {
          // option to check
          Date exDate = calendar.advance(today,lengths[k],Years);
          Handle<VanillaOption> option1 = 
              makeEuropeanOption(types[i],underlying,strikes[j],
                                 divCurve,rfCurve,exDate,volCurve);
          Handle<VanillaOption> option2 = 
              makeEuropeanOption(types[i],underlying,strikes[j],
                                 divCurve,rfCurve,exDate,volCurve,JR);
          Handle<VanillaOption> option3 = 
              makeEuropeanOption(types[i],underlying,strikes[j],
                                 divCurve,rfCurve,exDate,volCurve,CRR);
          
          for (int l=0; l<LENGTH(underlyings); l++) {
            for (int m=0; m<LENGTH(qRates); m++) {
              for (int n=0; n<LENGTH(rRates); n++) {
                for (int p=0; p<LENGTH(vols); p++) {
                  double u = underlyings[l],
                         q = qRates[m],
                         r = rRates[n],
                         v = vols[p];
                  underlying->setValue(u);
                  qRate->setValue(q);
                  rRate->setValue(r);
                  volatility->setValue(v);
                  
                  double value     = option1->NPV(),
                         value_jr  = option2->NPV(),
                         value_crr = option3->NPV();

                  if (relativeError(value,value_jr,u) > tolerance) {
                      CPPUNIT_FAIL(
                          typeToString(types[i]) + " option :\n"
                          "    underlying value: "
                          + DoubleFormatter::toString(u) + "\n"
                          "    strike:           "
                          + DoubleFormatter::toString(strikes[j]) +"\n"
                          "    dividend yield:   "
                          + DoubleFormatter::toString(q) + "\n"
                          "    risk-free rate:   "
                          + DoubleFormatter::toString(r) + "\n"
                          "    maturity:         "
                          + DateFormatter::toString(exDate) + "\n"
                          "    volatility:       "
                          + DoubleFormatter::toString(v) + "\n\n"
                          "    analytic value: " 
                          + DoubleFormatter::toString(value) + "\n"
                          "    binomial (JR):  " 
                          + DoubleFormatter::toString(value_jr));
                  }

                  if (relativeError(value,value_crr,u) > tolerance) {
                      CPPUNIT_FAIL(
                          typeToString(types[i]) + " option :\n"
                          "    underlying value: "
                          + DoubleFormatter::toString(u) + "\n"
                          "    strike:           "
                          + DoubleFormatter::toString(strikes[j]) +"\n"
                          "    dividend yield:   "
                          + DoubleFormatter::toString(q) + "\n"
                          "    risk-free rate:   "
                          + DoubleFormatter::toString(r) + "\n"
                          "    maturity:         "
                          + DateFormatter::toString(exDate) + "\n"
                          "    volatility:       "
                          + DoubleFormatter::toString(v) + "\n\n"
                          "    analytic value: " 
                          + DoubleFormatter::toString(value) + "\n"
                          "    binomial (CRR): " 
                          + DoubleFormatter::toString(value_crr));
                  }
                }
              }
            }
          }
        }
      }
    }
}

CppUnit::Test* EuropeanOptionTest::suite() {
    CppUnit::TestSuite* tests = 
        new CppUnit::TestSuite("European option tests");
    tests->addTest(new CppUnit::TestCaller<EuropeanOptionTest>
                   ("Testing European option greeks",
                    &EuropeanOptionTest::testGreeks));
    tests->addTest(new CppUnit::TestCaller<EuropeanOptionTest>
                   ("Testing European option implied volatility",
                    &EuropeanOptionTest::testImpliedVol));
    tests->addTest(new CppUnit::TestCaller<EuropeanOptionTest>
                   ("Testing binomial European engines "
                    "against analytic results",
                    &EuropeanOptionTest::testBinomialEngines));
    return tests;
}

