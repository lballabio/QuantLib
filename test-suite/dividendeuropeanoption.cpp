
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

#include "dividendeuropeanoption.hpp"
#include "utilities.hpp"
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/dividendvanillaoption.hpp>
#include <ql/PricingEngines/Vanilla/analyticdividendeuropeanengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, expected, calculated, error, tolerance) \
    BOOST_FAIL(exerciseTypeToString(exercise) + " " \
               + OptionTypeFormatter::toString(payoff->optionType()) + \
               " option with " \
               + payoffTypeToString(payoff) + " payoff:\n" \
               "    spot value: " \
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
               + DoubleFormatter::toString(error) + "\n" \
               "    tolerance:        " \
               + DoubleFormatter::toString(tolerance));

namespace {

    // utilities

    boost::shared_ptr<DividendVanillaOption>
    makeOption(const boost::shared_ptr<StrikedTypePayoff>& payoff,
               const boost::shared_ptr<Exercise>& exercise,
               const boost::shared_ptr<Quote>& u,
               const boost::shared_ptr<TermStructure>& q,
               const boost::shared_ptr<TermStructure>& r,
               const boost::shared_ptr<BlackVolTermStructure>& vol,
               const std::vector<Date>& dividendDates,
               const std::vector<double>& dividends) {

        boost::shared_ptr<PricingEngine> engine(
                                          new AnalyticDividendEuropeanEngine);

        boost::shared_ptr<BlackScholesProcess> stochProcess(new
            BlackScholesProcess(
                RelinkableHandle<Quote>(u),
                RelinkableHandle<TermStructure>(q),
                RelinkableHandle<TermStructure>(r),
                RelinkableHandle<BlackVolTermStructure>(vol)));

        return boost::shared_ptr<DividendVanillaOption>(new
            DividendVanillaOption(stochProcess, payoff, exercise, 
                                  dividendDates, dividends, engine));
    }

    int timeToDays(Time t) {
        return int(t*360+0.5);
    }

}

// tests

void DividendEuropeanOptionTest::testGreeks() {

    BOOST_MESSAGE("Testing dividend European option greeks...");

    std::map<std::string,double> calculated, expected, tolerance;
    tolerance["delta"] = 1.0e-5;
    tolerance["gamma"] = 1.0e-5;
    tolerance["theta"] = 1.0e-5;
    tolerance["rho"]   = 1.0e-5;
    tolerance["vega"]  = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    double strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    double underlyings[] = { 100.0 };
    Rate qRates[] = { 0.00, 0.10, 0.30 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Time residualTimes[] = { 1.0, 2.0 };
    double vols[] = { 0.05, 0.20, 0.70 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    boost::shared_ptr<TermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    boost::shared_ptr<TermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    boost::shared_ptr<StrikedTypePayoff> payoff;

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(residualTimes); k++) {
          Date exDate = today.plusDays(timeToDays(residualTimes[k]));
          boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
          Date exDateP = exDate.plusDays(1),
               exDateM = exDate.plusDays(-1);
          Time dT = (exDateP-exDateM)/360.0;

          std::vector<Date> dividendDates, dividendDatesP, dividendDatesM;
          std::vector<double> dividends;
          for (Time t = 0.25; t < residualTimes[k]; t += 0.5) {
              dividendDates.push_back(today.plusDays(timeToDays(t)));
              dividendDatesP.push_back(dividendDates.back().plusDays(1));
              dividendDatesM.push_back(dividendDates.back().plusDays(-1));
              dividends.push_back(5.0);
          }

          payoff = boost::shared_ptr<StrikedTypePayoff>(new
                                    PlainVanillaPayoff(types[i], strikes[j]));
          boost::shared_ptr<VanillaOption> option = 
                  makeOption(payoff, exercise, spot, qTS, rTS, volTS,
                             dividendDates, dividends);
          // time-shifted exercise dates and options
          boost::shared_ptr<Exercise> exerciseP(new EuropeanExercise(exDateP));
          boost::shared_ptr<VanillaOption> optionP =
                  makeOption(payoff, exerciseP, spot, qTS, rTS, volTS,
                             dividendDatesP, dividends);
          boost::shared_ptr<Exercise> exerciseM(new EuropeanExercise(exDateM));
          boost::shared_ptr<VanillaOption> optionM =
                  makeOption(payoff, exerciseM, spot, qTS, rTS, volTS,
                             dividendDatesM, dividends);

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

                    double value         = option->NPV();
                    calculated["delta"]  = option->delta();
                    calculated["gamma"]  = option->gamma();
                    calculated["theta"]  = option->theta();
                    calculated["rho"]    = option->rho();
                    calculated["vega"]   = option->vega();

                    if (value > spot->value()*1.0e-5) {
                        // perturb spot and get delta and gamma
                        double du = u*1.0e-4;
                        spot->setValue(u+du);
                        double value_p = option->NPV(),
                               delta_p = option->delta();
                        spot->setValue(u-du);
                        double value_m = option->NPV(),
                               delta_m = option->delta();
                        spot->setValue(u);
                        expected["delta"] = (value_p - value_m)/(2*du);
                        expected["gamma"] = (delta_p - delta_m)/(2*du);

                        // perturb risk-free rate and get rho
                        double dr = r*1.0e-4;
                        rRate->setValue(r+dr);
                        value_p = option->NPV();
                        rRate->setValue(r-dr);
                        value_m = option->NPV();
                        rRate->setValue(r);
                        expected["rho"] = (value_p - value_m)/(2*dr);

                        // perturb volatility and get vega
                        double dv = v*1.0e-4;
                        vol->setValue(v+dv);
                        value_p = option->NPV();
                        vol->setValue(v-dv);
                        value_m = option->NPV();
                        vol->setValue(v);
                        expected["vega"] = (value_p - value_m)/(2*dv);

                        // get theta from time-shifted options
                        expected["theta"] =
                              (optionM->NPV() - optionP->NPV())/dT;

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
                                REPORT_FAILURE(greek, payoff, exercise, 
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


test_suite* DividendEuropeanOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Dividend European option tests");
    suite->add(BOOST_TEST_CASE(&DividendEuropeanOptionTest::testGreeks));
    return suite;
}

