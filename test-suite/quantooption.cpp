/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004, 2007 StatPro Italia srl
 Copyright (C) 2008 Paul Farrington

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

#include "quantooption.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/quantovanillaoption.hpp>
#include <ql/instruments/quantoforwardvanillaoption.hpp>
#include <ql/instruments/quantobarrieroption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/pricingengines/quanto/quantoengine.hpp>
#include <ql/pricingengines/forward/forwardperformanceengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define QUANTO_REPORT_FAILURE(greekName, payoff, exercise, s, q, r, \
                        today, v, fxr, fxv, corr, expected, \
                        calculated, error, tolerance) \
    BOOST_FAIL("Quanto " << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    spot value:        " << s << "\n" \
               << "    strike:            " << payoff->strike() << "\n" \
               << "    dividend yield:    " << io::rate(q) << "\n" \
               << "    risk-free rate:    " << io::rate(r) << "\n" \
               << "    fx risk-free rate: " << io::rate(fxr) << "\n" \
               << "    reference date:    " << today << "\n" \
               << "    maturity:          " << exercise->lastDate() << "\n" \
               << "    volatility:        " << io::volatility(v) << "\n" \
               << "    fx volatility:     " << io::volatility(fxv) << "\n" \
               << "    correlation:       " << corr << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

#define QUANTO_FORWARD_REPORT_FAILURE(greekName, payoff, moneyness, \
                        exercise, s, q, r, \
                        today, reset, v, fxr, fxv, corr, expected, \
                        calculated, error, tolerance) \
    BOOST_FAIL("Quanto " << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    spot value:        " << s << "\n" \
               << "    strike:            " << payoff->strike() << "\n" \
               << "    moneyness:         " << io::percent(moneyness) << "\n" \
               << "    dividend yield:    " << io::rate(q) << "\n" \
               << "    risk-free rate:    " << io::rate(r) << "\n" \
               << "    fx risk-free rate: " << io::rate(fxr) << "\n" \
               << "    reference date:    " << today << "\n" \
               << "    reset date:        " << reset << "\n" \
               << "    maturity:          " << exercise->lastDate() << "\n" \
               << "    volatility:        " << io::volatility(v) << "\n" \
               << "    fx volatility:     " << io::volatility(fxv) << "\n" \
               << "    correlation:       " << corr << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

#define QUANTO_BARRIER_REPORT_FAILURE(greekName, payoff, \
                        barrierType, barrier, rebate, \
                        exercise, s, q, r, \
                        today, v, fxr, fxv, corr, expected, \
                        calculated, error, tolerance) \
    BOOST_FAIL("Quanto Barrier" << exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << "    barrier type:        " << barrierType << "\n" \
               << "    barrier:             " << barrier << "\n" \
               << "    rebate:              " << rebate << "\n" \
               << "    payoff:              " << payoffTypeToString(payoff) << "\n" \
               << "    spot value:          " << s << "\n" \
               << "    strike:              " << payoff->strike() << "\n" \
               << "    dividend yield:      " << io::rate(q) << "\n" \
               << "    risk-free rate:      " << io::rate(r) << "\n" \
               << "    fx risk-free rate:   " << io::rate(fxr) << "\n" \
               << "    reference date:      " << today << "\n" \
               << "    maturity:            " << exercise->lastDate() << "\n" \
               << "    volatility:          " << io::volatility(v) << "\n" \
               << "    fx volatility:       " << io::volatility(fxv) << "\n" \
               << "    correlation:         " << corr << "\n\n" \
               << "    expected   " << greekName << ": " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);


namespace {

    struct QuantoOptionData {
        Option::Type type;
        Real strike;
        Real s;          // spot
        Rate q;          // dividend
        Rate r;          // risk-free rate
        Time t;          // time to maturity
        Volatility v;    // volatility
        Rate fxr;        // fx risk-free rate
        Volatility fxv;  // fx volatility
        Real corr;       // correlation
        Real result;     // expected result
        Real tol;        // tolerance
    };

    struct QuantoForwardOptionData {
        Option::Type type;
        Real moneyness;
        Real s;          // spot
        Rate q;          // dividend
        Rate r;          // risk-free rate
        Time start;      // time to reset
        Time t;          // time to maturity
        Volatility v;    // volatility
        Rate fxr;        // fx risk-free rate
        Volatility fxv;  // fx volatility
        Real corr;       // correlation
        Real result;     // expected result
        Real tol;        // tolerance
    };

    struct QuantoBarrierOptionData {
        Barrier::Type barrierType;
        Real barrier;
        Real rebate;
        Option::Type type;
        Real s;          // spot
        Real strike;
        Rate q;          // dividend
        Rate r;          // risk-free rate
        Time t;          // time to maturity
        Volatility v;    // volatility
        Rate fxr;        // fx risk-free rate
        Volatility fxv;  // fx volatility
        Real corr;       // correlation
        Real result;     // expected result
        Real tol;        // tolerance
    };

}


void QuantoOptionTest::testValues() {

    BOOST_TEST_MESSAGE("Testing quanto option values...");

    SavedSettings backup;

    /* The data below are from
       from "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
    */
    QuantoOptionData values[] = {
        //       type, strike,  spot,  div, rate,   t, vol, fx risk-free rate, fx volatility, correlation,     result, tol
        // "Option pricing formulas", pag 105-106
        { Option::Call, 105.0, 100.0, 0.04, 0.08, 0.5, 0.2,              0.05,          0.10,         0.3, 5.3280/1.5, 1.0e-4 },
        // "Option pricing formulas", VBA code
        {  Option::Put, 105.0, 100.0, 0.04, 0.08, 0.5, 0.2,              0.05,          0.10,         0.3,     8.1636, 1.0e-4 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    boost::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(today, fxRate, dc));
    boost::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    boost::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));
    boost::shared_ptr<PricingEngine> engine(
        new QuantoEngine<VanillaOption, AnalyticEuropeanEngine>(
                                                 stochProcess, fxrTS, fxVolTS,
                                                 Handle<Quote>(correlation)));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(
                    new PlainVanillaPayoff(values[i].type, values[i].strike));
        Date exDate = today + Integer(values[i].t*360+0.5);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        fxRate->setValue(values[i].fxr);
        fxVol->setValue(values[i].fxv);
        correlation->setValue(values[i].corr);

        QuantoVanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated-values[i].result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            QUANTO_REPORT_FAILURE("value", payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today,
                           values[i].v, values[i].fxr, values[i].fxv,
                           values[i].corr, values[i].result, calculated,
                           error, tolerance);
        }
    }

}


void QuantoOptionTest::testGreeks() {

    BOOST_TEST_MESSAGE("Testing quanto option greeks...");

    SavedSettings backup;

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]   = 1.0e-5;
    tolerance["gamma"]   = 1.0e-5;
    tolerance["theta"]   = 1.0e-5;
    tolerance["rho"]     = 1.0e-5;
    tolerance["divRho"]  = 1.0e-5;
    tolerance["vega"]    = 1.0e-5;
    tolerance["qrho"]    = 1.0e-5;
    tolerance["qvega"]   = 1.0e-5;
    tolerance["qlambda"] = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 2 };
    Volatility vols[] = { 0.11, 1.20 };
    Real correlations[] = { 0.10, 0.90 };

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
    boost::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(fxRate, dc));
    boost::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(fxVol, dc));
    boost::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

    boost::shared_ptr<PricingEngine> engine(
        new QuantoEngine<VanillaOption,AnalyticEuropeanEngine>(
                                                  stochProcess,fxrTS, fxVolTS,
                                                  Handle<Quote>(correlation)));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {

          Date exDate = today + lengths[k]*Years;
          boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

          boost::shared_ptr<StrikedTypePayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

          QuantoVanillaOption option(payoff, exercise);
          option.setPricingEngine(engine);

          for (Size l=0; l<LENGTH(underlyings); l++) {
            for (Size m=0; m<LENGTH(qRates); m++) {
              for (Size n=0; n<LENGTH(rRates); n++) {
                for (Size p=0; p<LENGTH(vols); p++) {
                  for (Size a=0; a<LENGTH(rRates); a++) {
                    for (Size b=0; b<LENGTH(vols); b++) {
                      for (Size c=0; c<LENGTH(correlations); c++) {

                        Real u = underlyings[l];
                        Rate q = qRates[m],
                             r = rRates[n];
                        Volatility v = vols[p];
                        Rate fxr = rRates[a];
                        Volatility fxv = vols[b];
                        Rate corr = correlations[c];
                        spot->setValue(u);
                        qRate->setValue(q);
                        rRate->setValue(r);
                        vol->setValue(v);
                        fxRate->setValue(fxr);
                        fxVol->setValue(fxv);
                        correlation->setValue(corr);

                        Real value = option.NPV();
                        calculated["delta"]   = option.delta();
                        calculated["gamma"]   = option.gamma();
                        calculated["theta"]   = option.theta();
                        calculated["rho"]     = option.rho();
                        calculated["divRho"]  = option.dividendRho();
                        calculated["vega"]    = option.vega();
                        calculated["qrho"]    = option.qrho();
                        calculated["qvega"]   = option.qvega();
                        calculated["qlambda"] = option.qlambda();

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

                          // perturb fx rate and get qrho
                          Spread dfxr = fxr*1.0e-4;
                          fxRate->setValue(fxr+dfxr);
                          value_p = option.NPV();
                          fxRate->setValue(fxr-dfxr);
                          value_m = option.NPV();
                          fxRate->setValue(fxr);
                          expected["qrho"] = (value_p - value_m)/(2*dfxr);

                          // perturb fx volatility and get qvega
                          Volatility dfxv = fxv*1.0e-4;
                          fxVol->setValue(fxv+dfxv);
                          value_p = option.NPV();
                          fxVol->setValue(fxv-dfxv);
                          value_m = option.NPV();
                          fxVol->setValue(fxv);
                          expected["qvega"] = (value_p - value_m)/(2*dfxv);

                          // perturb correlation and get qlambda
                          Real dcorr = corr*1.0e-4;
                          correlation->setValue(corr+dcorr);
                          value_p = option.NPV();
                          correlation->setValue(corr-dcorr);
                          value_m = option.NPV();
                          correlation->setValue(corr);
                          expected["qlambda"] = (value_p - value_m)/(2*dcorr);

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
                                  QUANTO_REPORT_FAILURE(greek, payoff, exercise,
                                                 u, q, r, today, v,
                                                 fxr, fxv, corr,
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
}



void QuantoOptionTest::testForwardValues() {

    BOOST_TEST_MESSAGE("Testing quanto-forward option values...");

    SavedSettings backup;

    QuantoForwardOptionData values[] = {
        //   type, moneyness,  spot,  div, risk-free rate, reset, maturity,  vol, fx risk-free rate, fx vol, corr,     result, tol
        // reset=0.0, quanto (not-forward) options
        { Option::Call, 1.05, 100.0, 0.04,           0.08,  0.00,      0.5, 0.20,              0.05,   0.10,  0.3, 5.3280/1.5, 1.0e-4 },
        {  Option::Put, 1.05, 100.0, 0.04,           0.08,  0.00,      0.5, 0.20,              0.05,   0.10,  0.3,     8.1636, 1.0e-4 },
        // reset!=0.0, quanto-forward options (cursory checked against FinCAD 7)
        { Option::Call, 1.05, 100.0, 0.04,           0.08,  0.25,      0.5, 0.20,              0.05,   0.10,  0.3,     2.0171, 1.0e-4 },
        {  Option::Put, 1.05, 100.0, 0.04,           0.08,  0.25,      0.5, 0.20,              0.05,   0.10,  0.3,     6.7296, 1.0e-4 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    boost::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(today, fxRate, dc));
    boost::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    boost::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

    boost::shared_ptr<PricingEngine> engine(
        new QuantoEngine<ForwardVanillaOption,
                         ForwardVanillaEngine<AnalyticEuropeanEngine> >(
                                                 stochProcess, fxrTS, fxVolTS,
                                                 Handle<Quote>(correlation)));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(
                                 new PlainVanillaPayoff(values[i].type, 0.0));
        Date exDate = today + Integer(values[i].t*360+0.5);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        Date reset = today + Integer(values[i].start*360+0.5);

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        fxRate->setValue(values[i].fxr);
        fxVol->setValue(values[i].fxv);
        correlation->setValue(values[i].corr);

        QuantoForwardVanillaOption option(values[i].moneyness, reset,
                                          payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated-values[i].result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            QUANTO_FORWARD_REPORT_FAILURE("value", payoff, values[i].moneyness,
                            exercise, values[i].s,
                            values[i].q, values[i].r, today, reset,
                            values[i].v, values[i].fxr, values[i].fxv,
                            values[i].corr, values[i].result, calculated,
                            error, tolerance);
        }
    }

}


void QuantoOptionTest::testForwardGreeks() {

    BOOST_TEST_MESSAGE("Testing quanto-forward option greeks...");

    SavedSettings backup;

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]   = 1.0e-5;
    tolerance["gamma"]   = 1.0e-5;
    tolerance["theta"]   = 1.0e-5;
    tolerance["rho"]     = 1.0e-5;
    tolerance["divRho"]  = 1.0e-5;
    tolerance["vega"]    = 1.0e-5;
    tolerance["qrho"]    = 1.0e-5;
    tolerance["qvega"]   = 1.0e-5;
    tolerance["qlambda"] = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real moneyness[] = { 0.9, 1.0, 1.1 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 2 };
    Integer startMonths[] = { 6, 9 };
    Volatility vols[] = { 0.11, 1.20 };
    Real correlations[] = { 0.10, 0.90 };

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
    boost::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(fxRate, dc));
    boost::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(fxVol, dc));
    boost::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

    boost::shared_ptr<PricingEngine> engine(
        new QuantoEngine<ForwardVanillaOption,
                         ForwardVanillaEngine<AnalyticEuropeanEngine> >(
                                                 stochProcess, fxrTS, fxVolTS,
                                                 Handle<Quote>(correlation)));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(moneyness); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {
          for (Size h=0; h<LENGTH(startMonths); h++) {

            Date exDate = today + lengths[k]*Years;
            boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

            Date reset = today + startMonths[h]*Months;

            boost::shared_ptr<StrikedTypePayoff> payoff(
                                       new PlainVanillaPayoff(types[i], 0.0));

            QuantoForwardVanillaOption option(moneyness[j], reset,
                                              payoff, exercise);
            option.setPricingEngine(engine);

            for (Size l=0; l<LENGTH(underlyings); l++) {
              for (Size m=0; m<LENGTH(qRates); m++) {
                for (Size n=0; n<LENGTH(rRates); n++) {
                  for (Size p=0; p<LENGTH(vols); p++) {
                    for (Size a=0; a<LENGTH(rRates); a++) {
                      for (Size b=0; b<LENGTH(vols); b++) {
                        for (Size c=0; c<LENGTH(correlations); c++) {

                          Real u = underlyings[l];
                          Rate q = qRates[m],
                               r = rRates[n];
                          Volatility v = vols[p];
                          Rate fxr = rRates[a];
                          Volatility fxv = vols[b];
                          Real corr = correlations[c];
                          spot->setValue(u);
                          qRate->setValue(q);
                          rRate->setValue(r);
                          vol->setValue(v);
                          fxRate->setValue(fxr);
                          fxVol->setValue(fxv);
                          correlation->setValue(corr);

                          Real value = option.NPV();
                          calculated["delta"]   = option.delta();
                          calculated["gamma"]   = option.gamma();
                          calculated["theta"]   = option.theta();
                          calculated["rho"]     = option.rho();
                          calculated["divRho"]  = option.dividendRho();
                          calculated["vega"]    = option.vega();
                          calculated["qrho"]    = option.qrho();
                          calculated["qvega"]   = option.qvega();
                          calculated["qlambda"] = option.qlambda();

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

                            // perturb fx rate and get qrho
                            Spread dfxr = fxr*1.0e-4;
                            fxRate->setValue(fxr+dfxr);
                            value_p = option.NPV();
                            fxRate->setValue(fxr-dfxr);
                            value_m = option.NPV();
                            fxRate->setValue(fxr);
                            expected["qrho"] = (value_p - value_m)/(2*dfxr);

                            // perturb fx volatility and get qvega
                            Volatility dfxv = fxv*1.0e-4;
                            fxVol->setValue(fxv+dfxv);
                            value_p = option.NPV();
                            fxVol->setValue(fxv-dfxv);
                            value_m = option.NPV();
                            fxVol->setValue(fxv);
                            expected["qvega"] = (value_p - value_m)/(2*dfxv);

                            // perturb correlation and get qlambda
                            Real dcorr = corr*1.0e-4;
                            correlation->setValue(corr+dcorr);
                            value_p = option.NPV();
                            correlation->setValue(corr-dcorr);
                            value_m = option.NPV();
                            correlation->setValue(corr);
                            expected["qlambda"] =
                                (value_p - value_m)/(2*dcorr);

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
                                  QUANTO_FORWARD_REPORT_FAILURE(greek, payoff,
                                      moneyness[j],
                                      exercise, u, q, r, today, reset, v, fxr,
                                      fxv, corr, expct, calcl, error, tol);
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
    }
}


void QuantoOptionTest::testForwardPerformanceValues() {

    BOOST_TEST_MESSAGE("Testing quanto-forward-performance option values...");

    SavedSettings backup;

    QuantoForwardOptionData values[] = {
        //   type, moneyness,  spot,  div, risk-free rate, reset, maturity,  vol, fx risk-free rate, fx vol, corr,     result, tol
        // reset=0.0, quanto-(not-forward)-performance options
        // exactly one hundredth of the non-performance version
        { Option::Call, 1.05, 100.0, 0.04,           0.08,  0.00,      0.5, 0.20,              0.05,   0.10,  0.3, 5.3280/150, 1.0e-4 },
        {  Option::Put, 1.05, 100.0, 0.04,           0.08,  0.00,      0.5, 0.20,              0.05,   0.10,  0.3,     0.0816, 1.0e-4 },
        // reset!=0.0, quanto-forward-performance options (roughly one hundredth of the non-performance version)
        { Option::Call, 1.05, 100.0, 0.04,           0.08,  0.25,      0.5, 0.20,              0.05,   0.10,  0.3,     0.0201, 1.0e-4 },
        {  Option::Put, 1.05, 100.0, 0.04,           0.08,  0.25,      0.5, 0.20,              0.05,   0.10,  0.3,     0.0672, 1.0e-4 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    boost::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(today, fxRate, dc));
    boost::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    boost::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

    boost::shared_ptr<PricingEngine> engine(
        new QuantoEngine<ForwardVanillaOption,
                         ForwardPerformanceVanillaEngine<AnalyticEuropeanEngine> >(
                                                 stochProcess, fxrTS, fxVolTS,
                                                 Handle<Quote>(correlation)));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(
//                               new PercentageStrikePayoff(values[i].type, values[i].moneyness));
                                 new PlainVanillaPayoff(values[i].type, 0.0));
        Date exDate = today + Integer(values[i].t*360+0.5);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        Date reset = today + Integer(values[i].start*360+0.5);

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        fxRate->setValue(values[i].fxr);
        fxVol->setValue(values[i].fxv);
        correlation->setValue(values[i].corr);

        QuantoForwardVanillaOption option(values[i].moneyness, reset,
                                          payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated-values[i].result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            QUANTO_FORWARD_REPORT_FAILURE("value", payoff, values[i].moneyness,
                exercise,
                values[i].s, values[i].q, values[i].r, today, reset,
                values[i].v, values[i].fxr, values[i].fxv, values[i].corr,
                values[i].result, calculated, error, tolerance);
        }
    }

}

void QuantoOptionTest::testBarrierValues()  {

    BOOST_TEST_MESSAGE("Testing quanto-barrier option values...");

    SavedSettings backup;

    QuantoBarrierOptionData values[] = {
         // TODO:  Bench results against an existing prop calculator
         // barrierType, barrier, rebate, type, spot, strike,
         // q, r, T, vol, fx risk-free rate, fx vol, corr, result, tol
        { Barrier::DownOut, 95.0, 3.0, Option::Call, 100, 90,
          0.04, 0.0212, 0.50, 0.25, 0.05, 0.2, 0.3, 8.247, 0.5 },
        { Barrier::DownOut, 95.0, 3.0, Option::Put, 100, 90,
          0.04, 0.0212, 0.50, 0.25, 0.05, 0.2, 0.3, 2.274, 0.5 },
        { Barrier::DownIn, 95.0, 0, Option::Put, 100, 90,
          0.04, 0.0212, 0.50, 0.25, 0.05, 0.2, 0.3, 2.85, 0.5 },
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    boost::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> fxrTS(flatRate(today, fxRate, dc));
    boost::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    boost::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
         new BlackScholesMertonProcess(Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));

    boost::shared_ptr<PricingEngine> engine(
        new QuantoEngine<BarrierOption, AnalyticBarrierEngine>(
                                                 stochProcess, fxrTS, fxVolTS,
                                                 Handle<Quote>(correlation)));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(
                    new PlainVanillaPayoff(values[i].type, values[i].strike));

        Date exDate = today + Integer(values[i].t*360+0.5);
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        fxRate->setValue(values[i].fxr);
        fxVol->setValue(values[i].fxv);
        correlation->setValue(values[i].corr);

        QuantoBarrierOption option(values[i].barrierType,
                                   values[i].barrier,
                                   values[i].rebate,
                                   payoff,
                                   exercise);

        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real error = std::fabs(calculated-values[i].result);
        Real tolerance = values[i].tol;

        if (error>tolerance) {
            QUANTO_BARRIER_REPORT_FAILURE("value", payoff,
                values[i].barrierType,
                values[i].barrier,
                values[i].rebate,
                exercise,
                values[i].s, values[i].q, values[i].r, today,
                values[i].v, values[i].fxr, values[i].fxv, values[i].corr,
                values[i].result, calculated, error, tolerance);
        }
    }
}


test_suite* QuantoOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Quanto option tests");
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testValues));
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testGreeks));
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testForwardValues));
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testForwardGreeks));
    suite->add(QUANTLIB_TEST_CASE(
                            &QuantoOptionTest::testForwardPerformanceValues));
    suite->add(QUANTLIB_TEST_CASE(&QuantoOptionTest::testBarrierValues));
    return suite;
}

