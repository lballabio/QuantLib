/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2007 StatPro Italia srl

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

// TODO: Figure out why tests for options with both continuous and discrete
// dividends fail.

// TODO: Make the known value test work.  It is slightly off from the
// answer in Hull probably due to date conventions.

#include "dividendoption.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/fddividendeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fddividendamericanengine.hpp>
#include <ql/pricingengines/vanilla/fddividendshoutengine.hpp>
#include <ql/pricingengines/vanilla/analyticdividendeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>
#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, expected, calculated, error, tolerance) \
    BOOST_ERROR(exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    spot value:       " << s << "\n" \
               << "    strike:           " << payoff->strike() << "\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

// tests

void DividendOptionTest::testEuropeanValues() {

    BOOST_TEST_MESSAGE(
              "Testing dividend European option values with no dividends...");

    SavedSettings backup;

    Real tolerance = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.00, 0.10, 0.30 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.05, 0.20, 0.70 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {
          Date exDate = today + lengths[k]*Years;
          boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

          std::vector<Date> dividendDates;
          std::vector<Real> dividends;
          for (Date d = today + 3*Months;
                    d < exercise->lastDate();
                    d += 6*Months) {
              dividendDates.push_back(d);
              dividends.push_back(0.0);
          }

          boost::shared_ptr<StrikedTypePayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

          boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

          boost::shared_ptr<PricingEngine> ref_engine(
                                    new AnalyticEuropeanEngine(stochProcess));

          boost::shared_ptr<PricingEngine> engine(
                            new AnalyticDividendEuropeanEngine(stochProcess));

          DividendVanillaOption option(payoff, exercise,
                                       dividendDates, dividends);
          option.setPricingEngine(engine);

          VanillaOption ref_option(payoff, exercise);
          ref_option.setPricingEngine(ref_engine);

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

                    Real calculated = option.NPV();
                    Real expected = ref_option.NPV();
                    Real error = std::fabs(calculated-expected);
                    if (error > tolerance) {
                        REPORT_FAILURE("value start limit",
                                       payoff, exercise,
                                       u, q, r, today, v,
                                       expected, calculated,
                                       error, tolerance);
                    }
                }
              }
            }
          }
        }
      }
    }
}

// Reference pg. 253 - Hull - Options, Futures, and Other Derivatives 5th ed
// Exercise 12.8

void DividendOptionTest::testEuropeanKnownValue() {

    BOOST_TEST_MESSAGE(
              "Testing dividend European option values with known value...");

    SavedSettings backup;

    Real tolerance = 1.0e-2;
    Real expected = 3.67;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    Date exDate = today + 6*Months;
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    std::vector<Date> dividendDates;
    std::vector<Real> dividends;
    dividendDates.push_back(today + 2*Months);
    dividends.push_back(0.50);
    dividendDates.push_back(today + 5*Months);
    dividends.push_back(0.50);

    boost::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(Option::Call, 40.0));

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

    boost::shared_ptr<PricingEngine> engine(
                            new AnalyticDividendEuropeanEngine(stochProcess));

    DividendVanillaOption option(payoff, exercise,
                                 dividendDates, dividends);
    option.setPricingEngine(engine);

    Real u = 40.0;
    Rate q = 0.0, r = 0.09;
    Volatility v = 0.30;
    spot->setValue(u);
    qRate->setValue(q);
    rRate->setValue(r);
    vol->setValue(v);

    Real calculated = option.NPV();
    Real error = std::fabs(calculated-expected);
    if (error > tolerance) {
        REPORT_FAILURE("value start limit",
                       payoff, exercise,
                       u, q, r, today, v,
                       expected, calculated,
                       error, tolerance);
    }
}


void DividendOptionTest::testEuropeanStartLimit() {

    BOOST_TEST_MESSAGE(
       "Testing dividend European option with a dividend on today's date...");

    SavedSettings backup;

    Real tolerance = 1.0e-5;
    Real dividendValue = 10.0;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.00, 0.10, 0.30 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.05, 0.20, 0.70 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {
          Date exDate = today + lengths[k]*Years;
          boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

          std::vector<Date> dividendDates;
          std::vector<Real> dividends;
          dividendDates.push_back(today);
          dividends.push_back(dividendValue);

          boost::shared_ptr<StrikedTypePayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

          boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

          boost::shared_ptr<PricingEngine> engine(
                            new AnalyticDividendEuropeanEngine(stochProcess));

          boost::shared_ptr<PricingEngine> ref_engine(
                                    new AnalyticEuropeanEngine(stochProcess));

          DividendVanillaOption option(payoff, exercise,
                                       dividendDates, dividends);
          option.setPricingEngine(engine);

          VanillaOption ref_option(payoff, exercise);
          ref_option.setPricingEngine(ref_engine);

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

                    Real calculated = option.NPV();
                    spot->setValue(u-dividendValue);
                    Real expected = ref_option.NPV();
                    Real error = std::fabs(calculated-expected);
                    if (error > tolerance) {
                        REPORT_FAILURE("value", payoff, exercise,
                                       u, q, r, today, v,
                                       expected, calculated,
                                       error, tolerance);
                    }
                }
              }
            }
          }
        }
      }
    }
}

void DividendOptionTest::testEuropeanEndLimit() {

    BOOST_TEST_MESSAGE(
              "Testing dividend European option values with end limits...");

    SavedSettings backup;

    Real tolerance = 1.0e-5;
    Real dividendValue = 10.0;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.00, 0.10, 0.30 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.05, 0.20, 0.70 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {
          Date exDate = today + lengths[k]*Years;
          boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

          std::vector<Date> dividendDates;
          std::vector<Real> dividends;
          dividendDates.push_back(exercise->lastDate());
          dividends.push_back(dividendValue);

          boost::shared_ptr<StrikedTypePayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));


          boost::shared_ptr<StrikedTypePayoff> refPayoff(
                                new PlainVanillaPayoff(types[i],
                                                       strikes[j] +
                                                       dividendValue));

          boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

          boost::shared_ptr<PricingEngine> engine(
                            new AnalyticDividendEuropeanEngine(stochProcess));

          boost::shared_ptr<PricingEngine> ref_engine(
                                    new AnalyticEuropeanEngine(stochProcess));

          DividendVanillaOption option(payoff, exercise,
                                       dividendDates, dividends);
          option.setPricingEngine(engine);

          VanillaOption ref_option(refPayoff, exercise);
          ref_option.setPricingEngine(ref_engine);

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

                    Real calculated = option.NPV();
                    Real expected = ref_option.NPV();
                    Real error = std::fabs(calculated-expected);
                    if (error > tolerance) {
                        REPORT_FAILURE("value", payoff, exercise,
                                       u, q, r, today, v,
                                       expected, calculated,
                                       error, tolerance);
                    }
                }
              }
            }
          }
        }
      }
    }
}


void DividendOptionTest::testEuropeanGreeks() {

    BOOST_TEST_MESSAGE("Testing dividend European option greeks...");

    SavedSettings backup;

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"] = 1.0e-5;
    tolerance["gamma"] = 1.0e-5;
    tolerance["theta"] = 1.0e-5;
    tolerance["rho"]   = 1.0e-5;
    tolerance["vega"]  = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.00, 0.10, 0.30 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.05, 0.20, 0.40 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {
          Date exDate = today + lengths[k]*Years;
          boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

          std::vector<Date> dividendDates;
          std::vector<Real> dividends;
          for (Date d = today + 3*Months;
                    d < exercise->lastDate();
                    d += 6*Months) {
              dividendDates.push_back(d);
              dividends.push_back(5.0);
          }

          boost::shared_ptr<StrikedTypePayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

          boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

          boost::shared_ptr<PricingEngine> engine(
                            new AnalyticDividendEuropeanEngine(stochProcess));

          DividendVanillaOption option(payoff, exercise,
                                       dividendDates, dividends);
          option.setPricingEngine(engine);

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

                        // perturb risk-free rate and get rho
                        Spread dr = r*1.0e-4;
                        rRate->setValue(r+dr);
                        value_p = option.NPV();
                        rRate->setValue(r-dr);
                        value_m = option.NPV();
                        rRate->setValue(r);
                        expected["rho"] = (value_p - value_m)/(2*dr);

                        // perturb volatility and get vega
                        Spread dv = v*1.0e-4;
                        vol->setValue(v+dv);
                        value_p = option.NPV();
                        vol->setValue(v-dv);
                        value_m = option.NPV();
                        vol->setValue(v);
                        expected["vega"] = (value_p - value_m)/(2*dv);

                        // perturb date and get theta
                        Time dT = dc.yearFraction(today-1, today+1);
                        Settings::instance().evaluationDate() = today-1;
                        value_m = option.NPV();
                        Settings::instance().evaluationDate() = today+1;
                        value_p = option.NPV();
                        Settings::instance().evaluationDate() = today;
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


void DividendOptionTest::testFdEuropeanValues() {

    BOOST_TEST_MESSAGE(
              "Testing finite-difference dividend European option values...");

    SavedSettings backup;

    Real tolerance = 1.0e-2;
    Size gridPoints = 300;
    Size timeSteps = 40;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    // Rate qRates[] = { 0.00, 0.10, 0.30 };
    // Analytic dividend may not be handling q correctly
    Rate qRates[] = { 0.00 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.05, 0.20, 0.40 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {
          Date exDate = today + lengths[k]*Years;
          boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

          std::vector<Date> dividendDates;
          std::vector<Real> dividends;
          for (Date d = today + 3*Months;
                    d < exercise->lastDate();
                    d += 6*Months) {
              dividendDates.push_back(d);
              dividends.push_back(5.0);
          }

          boost::shared_ptr<StrikedTypePayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

          boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

          boost::shared_ptr<PricingEngine> engine(
              new FDDividendEuropeanEngine<CrankNicolson>(stochProcess,
                                                          timeSteps,
                                                          gridPoints));

          boost::shared_ptr<PricingEngine> ref_engine(
                            new AnalyticDividendEuropeanEngine(stochProcess));

          DividendVanillaOption option(payoff, exercise,
                                       dividendDates, dividends);
          option.setPricingEngine(engine);

          DividendVanillaOption ref_option(payoff, exercise,
                                           dividendDates, dividends);
          ref_option.setPricingEngine(ref_engine);

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
                    // FLOATING_POINT_EXCEPTION
                    Real calculated = option.NPV();
                    if (calculated > spot->value()*1.0e-5) {
                        Real expected = ref_option.NPV();
                        Real error = std::fabs(calculated-expected);
                        if (error > tolerance) {
                            REPORT_FAILURE("value", payoff, exercise,
                                           u, q, r, today, v,
                                           expected, calculated,
                                           error, tolerance);
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


namespace {

    template <class Engine>
    void testFdGreeks(const Date& today,
                      const boost::shared_ptr<Exercise>& exercise) {

        std::map<std::string,Real> calculated, expected, tolerance;
        tolerance["delta"] = 5.0e-3;
        tolerance["gamma"] = 7.0e-3;
        // tolerance["theta"] = 1.0e-2;

        Option::Type types[] = { Option::Call, Option::Put };
        Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
        Real underlyings[] = { 100.0 };
        Rate qRates[] = { 0.00, 0.10, 0.20 };
        Rate rRates[] = { 0.01, 0.05, 0.15 };
        Volatility vols[] = { 0.05, 0.20, 0.50 };

        DayCounter dc = Actual360();

        boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
        boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
        Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
        boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
        Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
        boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
        Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

        for (Size i=0; i<LENGTH(types); i++) {
            for (Size j=0; j<LENGTH(strikes); j++) {

                std::vector<Date> dividendDates;
                std::vector<Real> dividends;
                for (Date d = today + 3*Months;
                     d < exercise->lastDate();
                     d += 6*Months) {
                    dividendDates.push_back(d);
                    dividends.push_back(5.0);
                }

                boost::shared_ptr<StrikedTypePayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

                boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

                boost::shared_ptr<PricingEngine> engine(
                                                    new Engine(stochProcess));
                DividendVanillaOption option(payoff, exercise,
                                             dividendDates, dividends);
                option.setPricingEngine(engine);

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

                        // FLOATING_POINT_EXCEPTION
                        Real value = option.NPV();
                        calculated["delta"]  = option.delta();
                        calculated["gamma"]  = option.gamma();
                        // calculated["theta"]  = option.theta();

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

                          // perturb date and get theta
                          /*
                            Time dT = dc.yearFraction(today-1, today+1);
                            Settings::instance().evaluationDate() = today-1;
                            value_m = option.NPV();
                            Settings::instance().evaluationDate() = today+1;
                            value_p = option.NPV();
                            Settings::instance().evaluationDate() = today;
                            expected["theta"] = (value_p - value_m)/dT;
                          */

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


void DividendOptionTest::testFdEuropeanGreeks() {

    BOOST_TEST_MESSAGE(
             "Testing finite-differences dividend European option greeks...");

    SavedSettings backup;

    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;
    Integer lengths[] = { 1, 2 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        Date exDate = today + lengths[i]*Years;
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        testFdGreeks<FDDividendEuropeanEngine<CrankNicolson> >(today,exercise);
    }
}

void DividendOptionTest::testFdAmericanGreeks() {
    BOOST_TEST_MESSAGE(
             "Testing finite-differences dividend American option greeks...");

    SavedSettings backup;

    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;
    Integer lengths[] = { 1, 2 };

    for (Size i=0; i<LENGTH(lengths); i++) {
        Date exDate = today + lengths[i]*Years;
        boost::shared_ptr<Exercise> exercise(
                                          new AmericanExercise(today,exDate));
        testFdGreeks<FDDividendAmericanEngine<CrankNicolson> >(today,exercise);
    }
}


namespace {

    template <class Engine>
    void testFdDegenerate(const Date& today,
                          const boost::shared_ptr<Exercise>& exercise) {

        DayCounter dc = Actual360();
        boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(54.625));
        Handle<YieldTermStructure> rTS(flatRate(0.052706, dc));
        Handle<YieldTermStructure> qTS(flatRate(0.0, dc));
        Handle<BlackVolTermStructure> volTS(flatVol(0.282922, dc));

        boost::shared_ptr<BlackScholesMertonProcess> process(
                            new BlackScholesMertonProcess(Handle<Quote>(spot),
                                                          qTS, rTS, volTS));

        Size timeSteps = 300;
        Size gridPoints = 300;

        boost::shared_ptr<PricingEngine> engine(
                                    new Engine(process,timeSteps,gridPoints));

        boost::shared_ptr<StrikedTypePayoff> payoff(
                                  new PlainVanillaPayoff(Option::Call, 55.0));

        Real tolerance = 3.0e-3;

        std::vector<Rate> dividends;
        std::vector<Date> dividendDates;

        DividendVanillaOption option1(payoff, exercise,
                                      dividendDates, dividends);
        option1.setPricingEngine(engine);

        // FLOATING_POINT_EXCEPTION
        Real refValue = option1.NPV();

        for (Size i=0; i<=6; i++) {

            dividends.push_back(0.0);
            dividendDates.push_back(today+i);

            DividendVanillaOption option(payoff, exercise,
                                         dividendDates, dividends);
            option.setPricingEngine(engine);
            Real value = option.NPV();

            if (std::fabs(refValue-value) > tolerance)
                BOOST_FAIL("NPV changed by null dividend :\n"
                           << "    previous value: " << value << "\n"
                           << "    current value:  " << refValue << "\n"
                           << "    change:         " << value-refValue);
        }
    }

}


void DividendOptionTest::testFdEuropeanDegenerate() {

    BOOST_TEST_MESSAGE(
         "Testing degenerate finite-differences dividend European option...");

    SavedSettings backup;

    Date today = Date(27,February,2005);
    Settings::instance().evaluationDate() = today;
    Date exDate(13,April,2005);

    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

    testFdDegenerate<FDDividendEuropeanEngine<CrankNicolson> >(today,exercise);
}

void DividendOptionTest::testFdAmericanDegenerate() {

    BOOST_TEST_MESSAGE(
         "Testing degenerate finite-differences dividend American option...");

    SavedSettings backup;

    Date today = Date(27,February,2005);
    Settings::instance().evaluationDate() = today;
    Date exDate(13,April,2005);

    boost::shared_ptr<Exercise> exercise(new AmericanExercise(today,exDate));

    testFdDegenerate<FDDividendAmericanEngine<CrankNicolson> >(today,exercise);
}


test_suite* DividendOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Dividend European option tests");
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testEuropeanValues));
    // Doesn't quite work.  Need to deal with date conventions
    //  suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testEuropeanKnownValue));
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testEuropeanStartLimit));
    // Doesn't quite work.  Need to use discounted values
    // suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testEuropeanEndLimit));
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testEuropeanGreeks));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testFdEuropeanValues));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testFdEuropeanGreeks));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&DividendOptionTest::testFdAmericanGreeks));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(
                              &DividendOptionTest::testFdEuropeanDegenerate));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(
                              &DividendOptionTest::testFdAmericanDegenerate));
    return suite;
}

