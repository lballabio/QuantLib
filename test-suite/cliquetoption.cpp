
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
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/cliquetoption.hpp>
#include <ql/PricingEngines/Cliquet/analyticcliquetengine.hpp>
#include <ql/PricingEngines/Cliquet/analyticperformanceengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, v, \
                       expected, calculated, error, tolerance) \
    BOOST_FAIL(OptionTypeFormatter::toString(payoff->optionType()) \
               << " option:\n" \
               << "    spot value:       " << s << "\n" \
               << "    moneyness:        " << payoff->strike() << "\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

namespace {

    void teardown() {
        Settings::instance().setEvaluationDate(Date());
    }

}

// tests

void CliquetOptionTest::testValues() {

    BOOST_MESSAGE("Testing Cliquet option values...");

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(60.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.04));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.08));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.30));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);
    boost::shared_ptr<PricingEngine> engine(new AnalyticCliquetEngine);

    boost::shared_ptr<BlackScholesProcess> process(
               new BlackScholesProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

    std::vector<Date> reset;
    reset.push_back(today + 90);
    Date maturity = today + 360;
    Option::Type type = Option::Call;
    Real moneyness = 1.1;

    boost::shared_ptr<PercentageStrikePayoff> payoff(
                                 new PercentageStrikePayoff(type, moneyness));
    boost::shared_ptr<EuropeanExercise> exercise(
                                              new EuropeanExercise(maturity));

    CliquetOption option(process, payoff, exercise, reset, engine);

    Real calculated = option.NPV();
    Real expected = 4.4064; // Haug, p.37
    Real error = std::fabs(calculated-expected);
    Real tolerance = 1e-4;
    if (error > tolerance) {
        REPORT_FAILURE("value", payoff, exercise, spot->value(),
                       qRate->value(), rRate->value(), today,
                       vol->value(), expected, calculated,
                       error, tolerance);
    }
}


namespace {

  template <class T>
  void testOptionGreeks() {

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-5;
    tolerance["gamma"]  = 1.0e-5;
    tolerance["theta"]  = 1.0e-5;
    tolerance["rho"]    = 1.0e-5;
    tolerance["divRho"] = 1.0e-5;
    tolerance["vega"]   = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real moneyness[] = { 0.9, 1.0, 1.1 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Integer frequencies[] = { 2, 4 };
    Volatility vols[] = { 0.11, 0.50, 1.20 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().setEvaluationDate(today);

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    boost::shared_ptr<BlackScholesProcess> process(
               new BlackScholesProcess(Handle<Quote>(spot), qTS, rTS, volTS));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(moneyness); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {
          for (Size kk=0; kk<LENGTH(frequencies); kk++) {

            boost::shared_ptr<EuropeanExercise> maturity(
                              new EuropeanExercise(today + lengths[k]*Years));

            boost::shared_ptr<PercentageStrikePayoff> payoff(
                          new PercentageStrikePayoff(types[i], moneyness[j]));

            std::vector<Date> reset;
            Integer months = 12/frequencies[kk];
            for (Date d = today + months*Months;
                 d < maturity->lastDate();
                 d += months*Months)
                reset.push_back(d);

            boost::shared_ptr<PricingEngine> engine(new T);

            CliquetOption option(process, payoff, maturity, reset, engine);

            for (Size l=0; l<LENGTH(underlyings); l++) {
              for (Size m=0; m<LENGTH(qRates); m++) {
                for (Size n=0; n<LENGTH(rRates); n++) {
                  for (Size p=0; p<LENGTH(vols); p++) {

                      Real u = underlyings[l];
                      Rate q = qRates[m],
                           r = rRates[n];
                      Volatility v = vols[p];
                      spot->setValue(u);
                      qRate->setValue(q);
                      rRate->setValue(r);
                      vol->setValue(v);

                      Real value = option.NPV();
                      calculated["delta"]  = option.delta();
                      calculated["gamma"]  = option.gamma();
                      calculated["theta"]  = option.theta();
                      calculated["rho"]    = option.rho();
                      calculated["divRho"] = option.dividendRho();
                      calculated["vega"]   = option.vega();

                      if (value > spot->value()*1.0e-5) {
                          // perturb spot and get delta and gamma
                          Real du = u*1.0e-4;
                          spot->setValue(u+du);
                          Real value_p = option.NPV(),
                               delta_p = option.delta();
                          spot->setValue(u-du);
                          Real value_m = option.NPV(),
                               delta_m = option.delta();
                          spot->setValue(u);
                          expected["delta"] = (value_p - value_m)/(2*du);
                          expected["gamma"] = (delta_p - delta_m)/(2*du);

                          // perturb rates and get rho and dividend rho
                          Spread dr = r*1.0e-4;
                          rRate->setValue(r+dr);
                          value_p = option.NPV();
                          rRate->setValue(r-dr);
                          value_m = option.NPV();
                          rRate->setValue(r);
                          expected["rho"] = (value_p - value_m)/(2*dr);

                          Spread dq = q*1.0e-4;
                          qRate->setValue(q+dq);
                          value_p = option.NPV();
                          qRate->setValue(q-dq);
                          value_m = option.NPV();
                          qRate->setValue(q);
                          expected["divRho"] = (value_p - value_m)/(2*dq);

                          // perturb volatility and get vega
                          Volatility dv = v*1.0e-4;
                          vol->setValue(v+dv);
                          value_p = option.NPV();
                          vol->setValue(v-dv);
                          value_m = option.NPV();
                          vol->setValue(v);
                          expected["vega"] = (value_p - value_m)/(2*dv);

                          // perturb date and get theta
                          Time dT = dc.yearFraction(today-1, today+1);
                          Settings::instance().setEvaluationDate(today-1);
                          value_m = option.NPV();
                          Settings::instance().setEvaluationDate(today+1);
                          value_p = option.NPV();
                          Settings::instance().setEvaluationDate(today);
                          expected["theta"] = (value_p - value_m)/dT;

                          // compare
                          std::map<std::string,Real>::iterator it;
                          for (it = calculated.begin();
                               it != calculated.end(); ++it) {
                              std::string greek = it->first;
                              Real expct = expected  [greek],
                                   calcl = calculated[greek],
                                   tol   = tolerance [greek];
                              Real error = relativeError(expct,calcl,u);
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

}


void CliquetOptionTest::testGreeks() {

    BOOST_MESSAGE("Testing Cliquet option greeks...");

    QL_TEST_BEGIN

    testOptionGreeks<AnalyticCliquetEngine>();

    QL_TEST_TEARDOWN
}


void CliquetOptionTest::testPerformanceGreeks() {

    BOOST_MESSAGE("Testing performance option greeks...");

    QL_TEST_BEGIN

    testOptionGreeks<AnalyticPerformanceEngine>();

    QL_TEST_TEARDOWN
}


test_suite* CliquetOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Cliquet option tests");
    suite->add(BOOST_TEST_CASE(&CliquetOptionTest::testValues));
    suite->add(BOOST_TEST_CASE(&CliquetOptionTest::testGreeks));
    suite->add(BOOST_TEST_CASE(&CliquetOptionTest::testPerformanceGreeks));
    return suite;
}

