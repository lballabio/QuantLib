
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

#include "quantooption.hpp"
#include "utilities.hpp"
#include <ql/DayCounters/simpledaycounter.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/Instruments/quantovanillaoption.hpp>
#include <ql/Instruments/quantoforwardvanillaoption.hpp>
#include <ql/PricingEngines/Vanilla/analyticeuropeanengine.hpp>
#include <ql/PricingEngines/Quanto/quantoengine.hpp>
#include <ql/PricingEngines/Forward/forwardengine.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, fxr, fxv, corr, expected, calculated, \
                       error, tolerance) \
    BOOST_FAIL("Quanto " + exerciseTypeToString(exercise) + " " \
               + OptionTypeFormatter::toString(payoff->optionType()) + \
               " option with " \
               + payoffTypeToString(payoff) + " payoff:\n" \
               "    spot value:        " \
               + DecimalFormatter::toString(s) + "\n" \
               "    strike:            " \
               + DecimalFormatter::toString(payoff->strike()) +"\n" \
               "    dividend yield:    " \
               + RateFormatter::toString(q) + "\n" \
               "    risk-free rate:    " \
               + RateFormatter::toString(r) + "\n" \
               "    fx risk-free rate: " \
               + RateFormatter::toString(fxr) + "\n" \
               "    reference date:    " \
               + DateFormatter::toString(today) + "\n" \
               "    maturity:          " \
               + DateFormatter::toString(exercise->lastDate()) + "\n" \
               "    volatility:        " \
               + VolatilityFormatter::toString(v) + "\n" \
               "    fx volatility:     " \
               + VolatilityFormatter::toString(fxv) + "\n" \
               "    correlation:       " \
               + DecimalFormatter::toString(corr) + "\n\n" \
               "    expected   " + greekName + ": " \
               + DecimalFormatter::toString(expected) + "\n" \
               "    calculated " + greekName + ": " \
               + DecimalFormatter::toString(calculated) + "\n" \
               "    error:            " \
               + DecimalFormatter::toString(error) + "\n" \
               "    tolerance:        " \
               + DecimalFormatter::toString(tolerance));

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

}

// tests

void QuantoOptionTest::testValues() {

    BOOST_MESSAGE("Testing quanto option values...");

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
    RelinkableHandle<TermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    RelinkableHandle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    boost::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> fxrTS(flatRate(today, fxRate, dc));
    boost::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    RelinkableHandle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    boost::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    boost::shared_ptr<VanillaOption::engine> underlyingEngine(
                                                  new AnalyticEuropeanEngine);
    boost::shared_ptr<PricingEngine> engine(
                  new QuantoEngine<VanillaOption::arguments,
                                   VanillaOption::results>(underlyingEngine));

    boost::shared_ptr<BlackScholesProcess> stochProcess(
            new BlackScholesProcess(
                 RelinkableHandle<Quote>(spot),
                 RelinkableHandle<TermStructure>(qTS),
                 RelinkableHandle<TermStructure>(rTS),
                 RelinkableHandle<BlackVolTermStructure>(volTS)));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(
                    new PlainVanillaPayoff(values[i].type, values[i].strike));
        Date exDate = today.plusDays(Integer(values[i].t*360+0.5));
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        fxRate->setValue(values[i].fxr);
        fxVol->setValue(values[i].fxv);
        correlation->setValue(values[i].corr);

        QuantoVanillaOption option(fxrTS, fxVolTS,
                                   RelinkableHandle<Quote>(correlation),
                                   stochProcess, payoff, exercise, engine);

        Real calculated = option.NPV();
        Real error = QL_FABS(calculated-values[i].result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today,
                           values[i].v, values[i].fxr, values[i].fxv,
                           values[i].corr, values[i].result, calculated,
                           error, tolerance);
        }
    }

}


void QuantoOptionTest::testGreeks() {

    BOOST_MESSAGE("Testing quanto option greeks...");

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

    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
    Real strikes[] = { 50.0, 99.5, 100.0, 100.5, 150.0 };
    Real underlyings[] = { 100.0 };
    Rate qRates[] = { 0.04, 0.05 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 2 };
    Volatility vols[] = { 0.11, 1.20 };
    Real correlations[] = { 0.10, 0.90 };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    RelinkableHandle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));
    boost::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> fxrTS(flatRate(today, fxRate, dc));
    boost::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    RelinkableHandle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    boost::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    boost::shared_ptr<BlackScholesProcess> stochProcess(
                 new BlackScholesProcess(
                             RelinkableHandle<Quote>(spot), qTS, rTS, volTS));

    boost::shared_ptr<VanillaOption::engine> underlyingEngine(
                                                  new AnalyticEuropeanEngine);
    boost::shared_ptr<PricingEngine> engine(
                  new QuantoEngine<VanillaOption::arguments,
                                   VanillaOption::results>(underlyingEngine));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {

          Date exDate = today.plusYears(lengths[k]);
          boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

          boost::shared_ptr<StrikedTypePayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

          QuantoVanillaOption option(fxrTS, fxVolTS, 
                                     RelinkableHandle<Quote>(correlation),
                                     stochProcess, payoff, exercise, engine);

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
                          Time dT = 1.0/360;
                          qTS.linkTo(flatRate(today-1,qRate,dc));
                          rTS.linkTo(flatRate(today-1,rRate,dc));
                          volTS.linkTo(flatVol(today-1,vol,dc));
                          fxrTS.linkTo(flatRate(today-1,fxRate,dc));
                          fxVolTS.linkTo(flatVol(today-1,fxVol,dc));
                          value_m = option.NPV();
                          qTS.linkTo(flatRate(today+1,qRate,dc));
                          rTS.linkTo(flatRate(today+1,rRate,dc));
                          volTS.linkTo(flatVol(today+1,vol,dc));
                          fxrTS.linkTo(flatRate(today+1,fxRate,dc));
                          fxVolTS.linkTo(flatVol(today+1,fxVol,dc));
                          value_p = option.NPV();
                          qTS.linkTo(flatRate(today,qRate,dc));
                          rTS.linkTo(flatRate(today,rRate,dc));
                          volTS.linkTo(flatVol(today,vol,dc));
                          fxrTS.linkTo(flatRate(today,fxRate,dc));
                          fxVolTS.linkTo(flatVol(today,fxVol,dc));
                          expected["theta"] = (value_p - value_m)/(2*dT);

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

    BOOST_MESSAGE("Testing quanto-forward option values...");

    QuantoForwardOptionData values[] = {
        //   type, moneyness,  spot,  div, risk-free rate, reset, maturity,  vol, fx risk-free rate, fx vol, corr,     result, tol
        // reset=0.0, that is a quanto (not-forward) option
        { Option::Call, 1.05, 100.0, 0.04,           0.08,  0.00,      0.5, 0.20,              0.05,   0.10,  0.3, 5.3280/1.5, 1.0e-4 },
        {  Option::Put, 1.05, 100.0, 0.04,           0.08,  0.00,      0.5, 0.20,              0.05,   0.10,  0.3,     8.1636, 1.0e-4 }
    };

    DayCounter dc = SimpleDayCounter();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    RelinkableHandle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    boost::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> fxrTS(flatRate(today, fxRate, dc));
    boost::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    RelinkableHandle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    boost::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    boost::shared_ptr<VanillaOption::engine> underlyingEngine(
                                                  new AnalyticEuropeanEngine);
    boost::shared_ptr<ForwardVanillaOption::engine> forwardEngine(
                 new ForwardEngine<VanillaOption::arguments,
                                   VanillaOption::results>(underlyingEngine));
    boost::shared_ptr<PricingEngine> engine(
              new QuantoEngine<ForwardVanillaOption::arguments,
                               ForwardVanillaOption::results>(forwardEngine));

    boost::shared_ptr<BlackScholesProcess> stochProcess(
            new BlackScholesProcess(
                 RelinkableHandle<Quote>(spot),
                 RelinkableHandle<TermStructure>(qTS),
                 RelinkableHandle<TermStructure>(rTS),
                 RelinkableHandle<BlackVolTermStructure>(volTS)));

    for (Size i=0; i<LENGTH(values); i++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(
//                               new PercentageStrikePayoff(values[i].type, values[i].moneyness));
                                 new PlainVanillaPayoff(values[i].type, 0.0));
        Date exDate = today.plusDays(Integer(values[i].t*360+0.5));
        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        Date reset = today.plusDays(Integer(values[i].start*360+0.5));

        spot ->setValue(values[i].s);
        qRate->setValue(values[i].q);
        rRate->setValue(values[i].r);
        vol  ->setValue(values[i].v);

        fxRate->setValue(values[i].fxr);
        fxVol->setValue(values[i].fxv);
        correlation->setValue(values[i].corr);

        QuantoForwardVanillaOption option(fxrTS, fxVolTS,
                                          RelinkableHandle<Quote>(correlation),
                                          values[i].moneyness, reset,
                                          stochProcess, payoff, exercise, 
                                          engine);

        Real calculated = option.NPV();
        Real error = QL_FABS(calculated-values[i].result);
        Real tolerance = 1e-4;
        if (error>tolerance) {
            REPORT_FAILURE("value", payoff, exercise, values[i].s,
                           values[i].q, values[i].r, today,
                           values[i].v, values[i].fxr, values[i].fxv,
                           values[i].corr, values[i].result, calculated,
                           error, tolerance);
        }
    }

}


void QuantoOptionTest::testForwardGreeks() {

    BOOST_MESSAGE("Testing quanto-forward option greeks...");

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

    Option::Type types[] = { Option::Call, Option::Put, Option::Straddle };
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

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> qTS(flatRate(today, qRate, dc));
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> rTS(flatRate(today, rRate, dc));
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    RelinkableHandle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));
    boost::shared_ptr<SimpleQuote> fxRate(new SimpleQuote(0.0));
    RelinkableHandle<TermStructure> fxrTS(flatRate(today, fxRate, dc));
    boost::shared_ptr<SimpleQuote> fxVol(new SimpleQuote(0.0));
    RelinkableHandle<BlackVolTermStructure> fxVolTS(flatVol(today, fxVol, dc));
    boost::shared_ptr<SimpleQuote> correlation(new SimpleQuote(0.0));

    boost::shared_ptr<BlackScholesProcess> stochProcess(
                 new BlackScholesProcess(
                             RelinkableHandle<Quote>(spot), qTS, rTS, volTS));

    boost::shared_ptr<VanillaOption::engine> underlyingEngine(
                                                  new AnalyticEuropeanEngine);
    boost::shared_ptr<ForwardVanillaOption::engine> forwardEngine(
                 new ForwardEngine<VanillaOption::arguments,
                                   VanillaOption::results>(underlyingEngine));
    boost::shared_ptr<PricingEngine> engine(
              new QuantoEngine<ForwardVanillaOption::arguments,
                               ForwardVanillaOption::results>(forwardEngine));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(moneyness); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {
          for (Size h=0; h<LENGTH(startMonths); h++) {

            Date exDate = today.plusYears(lengths[k]);
            boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

            Date reset = today.plusMonths(startMonths[h]);

            boost::shared_ptr<StrikedTypePayoff> payoff(
                                       new PlainVanillaPayoff(types[i], 0.0));

            QuantoForwardVanillaOption option(
                                         fxrTS, fxVolTS, 
                                         RelinkableHandle<Quote>(correlation),
                                         moneyness[j], reset,
                                         stochProcess, payoff, 
                                         exercise, engine);

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
                            Time dT = 1.0/360;
                            qTS.linkTo(flatRate(today-1,qRate,dc));
                            rTS.linkTo(flatRate(today-1,rRate,dc));
                            volTS.linkTo(flatVol(today-1,vol,dc));
                            fxrTS.linkTo(flatRate(today-1,fxRate,dc));
                            fxVolTS.linkTo(flatVol(today-1,fxVol,dc));
                            value_m = option.NPV();
                            qTS.linkTo(flatRate(today+1,qRate,dc));
                            rTS.linkTo(flatRate(today+1,rRate,dc));
                            volTS.linkTo(flatVol(today+1,vol,dc));
                            fxrTS.linkTo(flatRate(today+1,fxRate,dc));
                            fxVolTS.linkTo(flatVol(today+1,fxVol,dc));
                            value_p = option.NPV();
                            qTS.linkTo(flatRate(today,qRate,dc));
                            rTS.linkTo(flatRate(today,rRate,dc));
                            volTS.linkTo(flatVol(today,vol,dc));
                            fxrTS.linkTo(flatRate(today,fxRate,dc));
                            fxVolTS.linkTo(flatVol(today,fxVol,dc));
                            expected["theta"] = (value_p - value_m)/(2*dT);

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
}


test_suite* QuantoOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Quanto option tests");
    suite->add(BOOST_TEST_CASE(&QuantoOptionTest::testValues));
    suite->add(BOOST_TEST_CASE(&QuantoOptionTest::testGreeks));
    suite->add(BOOST_TEST_CASE(&QuantoOptionTest::testForwardValues));
    suite->add(BOOST_TEST_CASE(&QuantoOptionTest::testForwardGreeks));
    return suite;
}

