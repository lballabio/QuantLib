
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#include "asianoptions.hpp"
#include "utilities.hpp"
#include <ql/DayCounters/actual365.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
#include <ql/Instruments/asianoption.hpp>
#include <ql/PricingEngines/Asian/analyticcontinuousasianengine.hpp>
#include <ql/PricingEngines/Asian/analyticdiscreteasianengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, averageType, runningProduct, pastFixings, \
                       fixingDates, payoff, exercise, s, q, r, today, v, \
                       expected, calculated, tolerance) \
    BOOST_FAIL(exerciseTypeToString(exercise) + \
               " Asian option with " \
               + averageTypeToString(averageType) + " and " \
               + payoffTypeToString(payoff) + " payoff:\n" \
               "    running product:  " \
               + DoubleFormatter::toString(runningProduct) + "\n" \
               "    past fixings:     " \
               + SizeFormatter::toString(pastFixings) + "\n" \
               "    future fixings:   " \
               + SizeFormatter::toString(fixingDates.size()) + "\n" \
               "    underlying value: " \
               + DoubleFormatter::toString(s) + "\n" \
               "    strike:           " \
               + DoubleFormatter::toString(payoff->strike()) +"\n" \
               "    dividend yield:   " \
               + DoubleFormatter::toString(q) + "\n" \
               "    risk-free rate:   " \
               + DoubleFormatter::toString(r) + "\n" \
               "    reference date:   " \
               + DateFormatter::toString(today) + "\n" \
               "    maturity:         " \
               + DateFormatter::toString(exercise->lastDate()) + "\n" \
               "    volatility:       " \
               + DoubleFormatter::toString(v) + "\n\n" \
               "    expected   " + greekName + ": " \
               + DoubleFormatter::toString(expected) + "\n" \
               "    calculated " + greekName + ": " \
               + DoubleFormatter::toString(calculated) + "\n" \
               "    error:            " \
               + DoubleFormatter::toString(QL_FABS(expected-calculated)) \
               + "\n" \
               + (tolerance==Null<double>() ? std::string("") : \
                  "    tolerance:        " \
                  + DoubleFormatter::toString(tolerance)));

namespace {

    std::string averageTypeToString(Average::Type averageType) {

        if (averageType == Average::Geometric)
            return "Geometric Averaging";
        else if (averageType == Average::Arithmetic)
            return "Arithmetic Averaging";
        else
            QL_FAIL("unknown averaging");
    }

    boost::shared_ptr<TermStructure> 
    flatRate(const Date& today,
             const boost::shared_ptr<Quote>& forward,
             const DayCounter& dc = Actual365()) {
        return boost::shared_ptr<TermStructure>(
                       new FlatForward(today, today, 
                                       RelinkableHandle<Quote>(forward), dc));
    }

    boost::shared_ptr<BlackVolTermStructure> 
    flatVol(const Date& today,
            const boost::shared_ptr<Quote>& vol,
            const DayCounter& dc = Actual365()) {
        return boost::shared_ptr<BlackVolTermStructure>(
                      new BlackConstantVol(today, 
                                           RelinkableHandle<Quote>(vol), dc));
    }

}

void AsianOptionTest::testGeometricContinuousAverage() {

    BOOST_MESSAGE("Testing continuous-averaging geometric Asian options...");

    // data from "Option Pricing Formulas", Haug, pag.96-97

    DayCounter dc = SimpleDayCounter();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(80.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(-0.03));
    boost::shared_ptr<TermStructure> qTS = makeFlatCurve(qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    boost::shared_ptr<TermStructure> rTS = makeFlatCurve(rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    boost::shared_ptr<BlackVolTermStructure> volTS = 
        makeFlatVolatility(vol, dc);

    boost::shared_ptr<BlackScholesProcess> stochProcess(new
        BlackScholesProcess(
            RelinkableHandle<Quote>(spot),
            RelinkableHandle<TermStructure>(qTS),
            RelinkableHandle<TermStructure>(rTS),
            RelinkableHandle<BlackVolTermStructure>(volTS)));

    boost::shared_ptr<PricingEngine> engine(
                                  new AnalyticContinuousAveragingAsianEngine);

    Average::Type averageType = Average::Geometric;
    Option::Type type = Option::Put;
    double strike = 85.0;
    Date exerciseDate = today.plusMonths(3);

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    ContinuousAveragingAsianOption option(averageType, stochProcess, payoff,
                                          exercise, engine);

    double calculated = option.NPV();
    double expected = 4.6922;
    double tolerance = 1.0e-4;
    if (QL_FABS(calculated-expected) > tolerance) {
        REPORT_FAILURE("value", averageType, Null<double>(), Null<int>(),
                       std::vector<Date>(), payoff, exercise, spot->value(),
                       qRate->value(), rRate->value(), today, 
                       vol->value(), expected, calculated, tolerance);
    }
}


void AsianOptionTest::testGeometricContinuousGreeks() {

    std::map<std::string,double> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-5;
    tolerance["gamma"]  = 1.0e-5;
    tolerance["theta"]  = 1.0e-5;
    tolerance["rho"]    = 1.0e-5;
    tolerance["divRho"] = 1.0e-5;
    tolerance["vega"]   = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    double underlyings[] = { 100.0 };
    double strikes[] = { 90.0, 100.0, 110.0 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    int lengths[] = { 1, 2 };
    double vols[] = { 0.11, 0.50, 1.20 };

    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> qTS(flatRate(today, qRate));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> rTS(flatRate(today, rRate));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    RelinkableHandle<BlackVolTermStructure> volTS(flatVol(today, vol));

    boost::shared_ptr<BlackScholesProcess> process(
                       new BlackScholesProcess(
                             RelinkableHandle<Quote>(spot), qTS, rTS, volTS));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {

            boost::shared_ptr<EuropeanExercise> maturity(
                           new EuropeanExercise(today.plusYears(lengths[k])));

            boost::shared_ptr<PlainVanillaPayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

            boost::shared_ptr<PricingEngine> engine(
                                  new AnalyticContinuousAveragingAsianEngine);

            ContinuousAveragingAsianOption option(Average::Geometric,
                                                  process, payoff, 
                                                  maturity, engine);

            for (Size l=0; l<LENGTH(underlyings); l++) {
              for (Size m=0; m<LENGTH(qRates); m++) {
                for (Size n=0; n<LENGTH(rRates); n++) {
                  for (Size p=0; p<LENGTH(vols); p++) {

                      double u = underlyings[l],
                             q = qRates[m],
                             r = rRates[n],
                             v = vols[p];
                      spot->setValue(u);
                      qRate->setValue(q);
                      rRate->setValue(r);
                      vol->setValue(v);

                      double value         = option.NPV();
                      calculated["delta"]  = option.delta();
                      calculated["gamma"]  = option.gamma();
                      calculated["theta"]  = option.theta();
                      calculated["rho"]    = option.rho();
                      calculated["divRho"] = option.dividendRho();
                      calculated["vega"]   = option.vega();

                      if (value > spot->value()*1.0e-5) {
                          // perturb spot and get delta and gamma
                          double du = u*1.0e-4;
                          spot->setValue(u+du);
                          double value_p = option.NPV(),
                                 delta_p = option.delta();
                          spot->setValue(u-du);
                          double value_m = option.NPV(),
                                 delta_m = option.delta();
                          spot->setValue(u);
                          expected["delta"] = (value_p - value_m)/(2*du);
                          expected["gamma"] = (delta_p - delta_m)/(2*du);

                          // perturb rates and get rho and dividend rho
                          double dr = r*1.0e-4;
                          rRate->setValue(r+dr);
                          value_p = option.NPV();
                          rRate->setValue(r-dr);
                          value_m = option.NPV();
                          rRate->setValue(r);
                          expected["rho"] = (value_p - value_m)/(2*dr);

                          double dq = q*1.0e-4;
                          qRate->setValue(q+dq);
                          value_p = option.NPV();
                          qRate->setValue(q-dq);
                          value_m = option.NPV();
                          qRate->setValue(q);
                          expected["divRho"] = (value_p - value_m)/(2*dq);

                          // perturb volatility and get vega
                          double dv = v*1.0e-4;
                          vol->setValue(v+dv);
                          value_p = option.NPV();
                          vol->setValue(v-dv);
                          value_m = option.NPV();
                          vol->setValue(v);
                          expected["vega"] = (value_p - value_m)/(2*dv);

                          // perturb date and get theta
                          double dT = 1.0/365;
                          qTS.linkTo(flatRate(today-1,qRate));
                          rTS.linkTo(flatRate(today-1,rRate));
                          volTS.linkTo(flatVol(today-1, vol));
                          value_m = option.NPV();
                          qTS.linkTo(flatRate(today+1,qRate));
                          rTS.linkTo(flatRate(today+1,rRate));
                          volTS.linkTo(flatVol(today+1, vol));
                          value_p = option.NPV();
                          qTS.linkTo(flatRate(today,qRate));
                          rTS.linkTo(flatRate(today,rRate));
                          volTS.linkTo(flatVol(today, vol));
                          expected["theta"] = (value_p - value_m)/(2*dT);

                          // compare
                          std::map<std::string,double>::iterator it;
                          for (it = calculated.begin();
                               it != calculated.end(); ++it) {
                              std::string greek = it->first;
                              double expct = expected  [greek],
                                     calcl = calculated[greek],
                                     tol   = tolerance [greek];
                              double error = relativeError(expct,calcl,u);
                              if (error>tol) {
                                  REPORT_FAILURE(greek, Average::Geometric,
                                                 Null<double>(), Null<int>(),
                                                 std::vector<Date>(),
                                                 payoff, maturity,
                                                 u, q, r, today, v, 
                                                 expct, calcl, tol);
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


void AsianOptionTest::testGeometricDiscreteAverage() {

    BOOST_MESSAGE("Testing discrete-averaging geometric Asian options...");

    // data from "Implementing Derivatives Model",
    // Clewlow, Strickland, p.118-123

    DayCounter dc = SimpleDayCounter();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    boost::shared_ptr<TermStructure> qTS = makeFlatCurve(qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    boost::shared_ptr<TermStructure> rTS = makeFlatCurve(rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    boost::shared_ptr<BlackVolTermStructure> volTS = 
        makeFlatVolatility(vol, dc);

    boost::shared_ptr<BlackScholesProcess> stochProcess(new
        BlackScholesProcess(
            RelinkableHandle<Quote>(spot),
            RelinkableHandle<TermStructure>(qTS),
            RelinkableHandle<TermStructure>(rTS),
            RelinkableHandle<BlackVolTermStructure>(volTS)));

    boost::shared_ptr<PricingEngine> engine(
                                    new AnalyticDiscreteAveragingAsianEngine);

    Average::Type averageType = Average::Geometric;
    double runningProduct = 1.0;
    Size pastFixings = 0;
    Size futureFixings = 10;
    Option::Type type = Option::Call;
    double strike = 100.0;
    Date exerciseDate = today.plusYears(1);

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    std::vector<Date> fixingDates(futureFixings);
    Time t = dc.yearFraction(today, exerciseDate);
    int dt = int(t*360/futureFixings+0.5);
    fixingDates[0]=today.plusDays(dt);
    for (Size j=1; j<futureFixings; j++)
        fixingDates[j]=fixingDates[j-1].plusDays(dt);

    DiscreteAveragingAsianOption option(averageType, runningProduct,
                                        pastFixings, fixingDates,
                                        stochProcess, payoff,
                                        exercise, engine);

    double calculated = option.NPV();
    double expected = 5.2865;
    double tolerance = 1.0e-4;
    if (QL_FABS(calculated-expected) > tolerance) {
        REPORT_FAILURE("value", averageType, runningProduct, pastFixings,
                       fixingDates, payoff, exercise, spot->value(),
                       qRate->value(), rRate->value(), today, 
                       vol->value(), expected, calculated, tolerance);
    }
}


void AsianOptionTest::testGeometricDiscreteGreeks() {

    std::map<std::string,double> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-5;
    tolerance["gamma"]  = 1.0e-5;
    tolerance["theta"]  = 1.0e-5;
    tolerance["rho"]    = 1.0e-5;
    tolerance["divRho"] = 1.0e-5;
    tolerance["vega"]   = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    double underlyings[] = { 100.0 };
    double strikes[] = { 90.0, 100.0, 110.0 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    int lengths[] = { 1, 2 };
    double vols[] = { 0.11, 0.50, 1.20 };

    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> qTS(flatRate(today, qRate));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> rTS(flatRate(today, rRate));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    RelinkableHandle<BlackVolTermStructure> volTS(flatVol(today, vol));

    boost::shared_ptr<BlackScholesProcess> process(
                       new BlackScholesProcess(
                             RelinkableHandle<Quote>(spot), qTS, rTS, volTS));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {

            boost::shared_ptr<EuropeanExercise> maturity(
                           new EuropeanExercise(today.plusYears(lengths[k])));

            boost::shared_ptr<PlainVanillaPayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

            double runningAverage = 1.0;
            Size pastFixings = 0;

            std::vector<Date> fixingDates;
            for (Date d = today.plusMonths(3); 
                 d <= maturity->lastDate();
                 d = d.plusMonths(3))
                fixingDates.push_back(d);


            boost::shared_ptr<PricingEngine> engine(
                                    new AnalyticDiscreteAveragingAsianEngine);

            DiscreteAveragingAsianOption option(Average::Geometric, 
                                                runningAverage, pastFixings,
                                                fixingDates, process, payoff, 
                                                maturity, engine);

            for (Size l=0; l<LENGTH(underlyings); l++) {
              for (Size m=0; m<LENGTH(qRates); m++) {
                for (Size n=0; n<LENGTH(rRates); n++) {
                  for (Size p=0; p<LENGTH(vols); p++) {

                      double u = underlyings[l],
                             q = qRates[m],
                             r = rRates[n],
                             v = vols[p];
                      spot->setValue(u);
                      qRate->setValue(q);
                      rRate->setValue(r);
                      vol->setValue(v);

                      double value         = option.NPV();
                      calculated["delta"]  = option.delta();
                      calculated["gamma"]  = option.gamma();
                      calculated["theta"]  = option.theta();
                      calculated["rho"]    = option.rho();
                      calculated["divRho"] = option.dividendRho();
                      calculated["vega"]   = option.vega();

                      if (value > spot->value()*1.0e-5) {
                          // perturb spot and get delta and gamma
                          double du = u*1.0e-4;
                          spot->setValue(u+du);
                          double value_p = option.NPV(),
                                 delta_p = option.delta();
                          spot->setValue(u-du);
                          double value_m = option.NPV(),
                                 delta_m = option.delta();
                          spot->setValue(u);
                          expected["delta"] = (value_p - value_m)/(2*du);
                          expected["gamma"] = (delta_p - delta_m)/(2*du);

                          // perturb rates and get rho and dividend rho
                          double dr = r*1.0e-4;
                          rRate->setValue(r+dr);
                          value_p = option.NPV();
                          rRate->setValue(r-dr);
                          value_m = option.NPV();
                          rRate->setValue(r);
                          expected["rho"] = (value_p - value_m)/(2*dr);

                          double dq = q*1.0e-4;
                          qRate->setValue(q+dq);
                          value_p = option.NPV();
                          qRate->setValue(q-dq);
                          value_m = option.NPV();
                          qRate->setValue(q);
                          expected["divRho"] = (value_p - value_m)/(2*dq);

                          // perturb volatility and get vega
                          double dv = v*1.0e-4;
                          vol->setValue(v+dv);
                          value_p = option.NPV();
                          vol->setValue(v-dv);
                          value_m = option.NPV();
                          vol->setValue(v);
                          expected["vega"] = (value_p - value_m)/(2*dv);

                          // perturb date and get theta
                          double dT = 1.0/365;
                          qTS.linkTo(flatRate(today-1,qRate));
                          rTS.linkTo(flatRate(today-1,rRate));
                          volTS.linkTo(flatVol(today-1, vol));
                          value_m = option.NPV();
                          qTS.linkTo(flatRate(today+1,qRate));
                          rTS.linkTo(flatRate(today+1,rRate));
                          volTS.linkTo(flatVol(today+1, vol));
                          value_p = option.NPV();
                          qTS.linkTo(flatRate(today,qRate));
                          rTS.linkTo(flatRate(today,rRate));
                          volTS.linkTo(flatVol(today, vol));
                          expected["theta"] = (value_p - value_m)/(2*dT);

                          // compare
                          std::map<std::string,double>::iterator it;
                          for (it = calculated.begin();
                               it != calculated.end(); ++it) {
                              std::string greek = it->first;
                              double expct = expected  [greek],
                                     calcl = calculated[greek],
                                     tol   = tolerance [greek];
                              double error = relativeError(expct,calcl,u);
                              if (error>tol) {
                                  REPORT_FAILURE(greek, Average::Geometric,
                                                 Null<double>(), Null<int>(),
                                                 std::vector<Date>(),
                                                 payoff, maturity,
                                                 u, q, r, today, v, 
                                                 expct, calcl, tol);
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

test_suite* AsianOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Asian option tests");
    suite->add(
          BOOST_TEST_CASE(&AsianOptionTest::testGeometricContinuousAverage));
    suite->add(
          BOOST_TEST_CASE(&AsianOptionTest::testGeometricDiscreteAverage));
    /* Greeks are broken
    suite->add(
          BOOST_TEST_CASE(&AsianOptionTest::testGeometricContinuousGreeks));
    suite->add(
          BOOST_TEST_CASE(&AsianOptionTest::testGeometricDiscreteGreeks));
    */
    return suite;
}

