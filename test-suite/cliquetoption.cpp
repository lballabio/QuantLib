
/*
 Copyright (C) 2004 StatPro Italia srl

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

#include "cliquetoption.hpp"
#include "utilities.hpp"
#include <ql/Instruments/cliquetoption.hpp>
#include <ql/PricingEngines/Cliquet/analyticcliquetengine.hpp>
#include <ql/DayCounters/actual365.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, v, \
                       expected, calculated, error, tolerance) \
    BOOST_FAIL(OptionTypeFormatter::toString(payoff->optionType()) + \
               " option:\n" \
               "    spot value:       " \
               + DoubleFormatter::toString(s) + "\n" \
               "    moneyness:        " \
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
               + DoubleFormatter::toString(error) + "\n" \
               "    tolerance:        " \
               + DoubleFormatter::toString(tolerance));

namespace {

    // utilities

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

// tests

void CliquetOptionTest::testValues() {

    BOOST_MESSAGE("Testing Cliquet option values...");

    Date today = Date::todaysDate();
    DayCounter dc = SimpleDayCounter();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(60.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    boost::shared_ptr<TermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.08));
    boost::shared_ptr<TermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.30));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(new AnalyticCliquetEngine);

    boost::shared_ptr<BlackScholesProcess> process(
                       new BlackScholesProcess(
                             RelinkableHandle<Quote>(spot), 
                             RelinkableHandle<TermStructure>(qTS),
                             RelinkableHandle<TermStructure>(rTS),
                             RelinkableHandle<BlackVolTermStructure>(volTS)));

    std::vector<Date> reset;
    reset.push_back(today.plusMonths(3));
    Date maturity = today.plusYears(1);
    Option::Type type = Option::Call;
    double moneyness = 1.1;

    boost::shared_ptr<PercentageStrikePayoff> payoff(
                                 new PercentageStrikePayoff(type, moneyness));
    boost::shared_ptr<EuropeanExercise> exercise(
                                              new EuropeanExercise(maturity));

    CliquetOption option(process, payoff, exercise, reset, engine);

    double calculated = option.NPV();
    double expected = 4.4064; // Haug, p.37
    double error = QL_FABS(calculated-expected);
    double tolerance = 1e-4;
    if (error > tolerance) {
        REPORT_FAILURE("value", payoff, exercise, spot->value(),
                       qRate->value(), rRate->value(), today,
                       vol->value(), expected, calculated,
                       error, tolerance);
    }
}


void CliquetOptionTest::testGreeks() {

    BOOST_MESSAGE("Testing Cliquet option greeks...");

    std::map<std::string,double> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-5;
    tolerance["gamma"]  = 1.0e-5;
    tolerance["theta"]  = 1.0e-5;
    tolerance["rho"]    = 1.0e-5;
    tolerance["divRho"] = 1.0e-5;
    tolerance["vega"]   = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    double moneyness[] = { 0.9, 1.0, 1.1 };
    double underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    int lengths[] = { 1, 2 };
    int frequencies[] = { 2, 4 };
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

    int loop = 0;

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(moneyness); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {
          for (Size kk=0; kk<LENGTH(frequencies); kk++) {

            boost::shared_ptr<EuropeanExercise> maturity(
                           new EuropeanExercise(today.plusYears(lengths[k])));

            boost::shared_ptr<PercentageStrikePayoff> payoff(
                          new PercentageStrikePayoff(types[i], moneyness[j]));

            std::vector<Date> reset;
            int months = 12/frequencies[kk];
            for (Date d = today.plusMonths(months); 
                 d < maturity->lastDate(); 
                 d = d.plusMonths(months))
                reset.push_back(d);

            boost::shared_ptr<PricingEngine> engine(new AnalyticCliquetEngine);

            CliquetOption option(process, payoff, maturity, reset, engine);

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
                                  REPORT_FAILURE(greek, payoff, maturity,
                                                 u, q, r, today, v, 
                                                 expct, calcl, error, tol);
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
}


test_suite* CliquetOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Cliquet option tests");
    suite->add(BOOST_TEST_CASE(&CliquetOptionTest::testValues));
    suite->add(BOOST_TEST_CASE(&CliquetOptionTest::testGreeks));
    return suite;
}

