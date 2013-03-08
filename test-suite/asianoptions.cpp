/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2005, 2007, 2008 StatPro Italia srl
 Copyright (C) 2009, 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include "asianoptions.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/asianoption.hpp>
#include <ql/pricingengines/asian/analytic_discr_geom_av_price.hpp>
#include <ql/pricingengines/asian/analytic_discr_geom_av_strike.hpp>
#include <ql/pricingengines/asian/analytic_cont_geom_av_price.hpp>
#include <ql/pricingengines/asian/mc_discr_geom_av_price.hpp>
#include <ql/pricingengines/asian/mc_discr_arith_av_price.hpp>
#include <ql/pricingengines/asian/mc_discr_arith_av_strike.hpp>
#include <ql/pricingengines/asian/fdblackscholesasianengine.hpp>
#include <ql/experimental/exoticoptions/continuousarithmeticasianlevyengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <boost/progress.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#define REPORT_FAILURE(greekName, averageType, \
                       runningAccumulator, pastFixings, \
                       fixingDates, payoff, exercise, s, q, r, today, v, \
                       expected, calculated, tolerance) \
    BOOST_ERROR( \
        exerciseTypeToString(exercise) \
        << " Asian option with " \
        << averageTypeToString(averageType) << " and " \
        << payoffTypeToString(payoff) << " payoff:\n" \
        << "    running variable: " \
        << io::checknull(runningAccumulator) << "\n" \
        << "    past fixings:     " \
        << io::checknull(pastFixings) << "\n" \
        << "    future fixings:   " << fixingDates.size() << "\n" \
        << "    underlying value: " << s << "\n" \
        << "    strike:           " << payoff->strike() << "\n" \
        << "    dividend yield:   " << io::rate(q) << "\n" \
        << "    risk-free rate:   " << io::rate(r) << "\n" \
        << "    reference date:   " << today << "\n" \
        << "    maturity:         " << exercise->lastDate() << "\n" \
        << "    volatility:       " << io::volatility(v) << "\n\n" \
        << "    expected   " << greekName << ": " << expected << "\n" \
        << "    calculated " << greekName << ": " << calculated << "\n"\
        << "    error:            " << std::fabs(expected-calculated) \
        << "\n" \
        << "    tolerance:        " << tolerance);

namespace {

    std::string averageTypeToString(Average::Type averageType) {

        if (averageType == Average::Geometric)
            return "Geometric Averaging";
        else if (averageType == Average::Arithmetic)
            return "Arithmetic Averaging";
        else
            QL_FAIL("unknown averaging");
    }

}


void AsianOptionTest::testAnalyticContinuousGeometricAveragePrice() {

    BOOST_TEST_MESSAGE(
             "Testing analytic continuous geometric average-price Asians...");

    // data from "Option Pricing Formulas", Haug, pag.96-97

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(80.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(-0.03));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
        BlackScholesMertonProcess(Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS)));

    boost::shared_ptr<PricingEngine> engine(new
            AnalyticContinuousGeometricAveragePriceAsianEngine(stochProcess));

    Average::Type averageType = Average::Geometric;
    Option::Type type = Option::Put;
    Real strike = 85.0;
    Date exerciseDate = today + 90;

    Size pastFixings = Null<Size>();
    Real runningAccumulator = Null<Real>();

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    ContinuousAveragingAsianOption option(averageType, payoff, exercise);
    option.setPricingEngine(engine);

    Real calculated = option.NPV();
    Real expected = 4.6922;
    Real tolerance = 1.0e-4;
    if (std::fabs(calculated-expected) > tolerance) {
        REPORT_FAILURE("value", averageType, runningAccumulator, pastFixings,
                       std::vector<Date>(), payoff, exercise, spot->value(),
                       qRate->value(), rRate->value(), today,
                       vol->value(), expected, calculated, tolerance);
    }

    // trying to approximate the continuous version with the discrete version
    runningAccumulator = 1.0;
    pastFixings = 0;
    std::vector<Date> fixingDates(exerciseDate-today+1);
    for (Size i=0; i<fixingDates.size(); i++) {
        fixingDates[i] = today + i;
    }
    boost::shared_ptr<PricingEngine> engine2(new
              AnalyticDiscreteGeometricAveragePriceAsianEngine(stochProcess));
    DiscreteAveragingAsianOption option2(averageType,
                                         runningAccumulator, pastFixings,
                                         fixingDates,
                                         payoff,
                                         exercise);
    option2.setPricingEngine(engine2);

    calculated = option2.NPV();
    tolerance = 3.0e-3;
    if (std::fabs(calculated-expected) > tolerance) {
        REPORT_FAILURE("value", averageType, runningAccumulator, pastFixings,
                       fixingDates, payoff, exercise, spot->value(),
                       qRate->value(), rRate->value(), today,
                       vol->value(), expected, calculated, tolerance);
    }

}


void AsianOptionTest::testAnalyticContinuousGeometricAveragePriceGreeks() {

    BOOST_TEST_MESSAGE(
       "Testing analytic continuous geometric average-price Asian greeks...");

    SavedSettings backup;

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-5;
    tolerance["gamma"]  = 1.0e-5;
    tolerance["theta"]  = 1.0e-5;
    tolerance["rho"]    = 1.0e-5;
    tolerance["divRho"] = 1.0e-5;
    tolerance["vega"]   = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real underlyings[] = { 100.0 };
    Real strikes[] = { 90.0, 100.0, 110.0 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.11, 0.50, 1.20 };

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

    boost::shared_ptr<BlackScholesMertonProcess> process(
         new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {

            boost::shared_ptr<EuropeanExercise> maturity(
                              new EuropeanExercise(today + lengths[k]*Years));

            boost::shared_ptr<PlainVanillaPayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

            boost::shared_ptr<PricingEngine> engine(new
                 AnalyticContinuousGeometricAveragePriceAsianEngine(process));

            ContinuousAveragingAsianOption option(Average::Geometric,
                                                  payoff, maturity);
            option.setPricingEngine(engine);

            Size pastFixings = Null<Size>();
            Real runningAverage = Null<Real>();

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
                                  REPORT_FAILURE(greek, Average::Geometric,
                                                 runningAverage, pastFixings,
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


void AsianOptionTest::testAnalyticDiscreteGeometricAveragePrice() {

    BOOST_TEST_MESSAGE(
               "Testing analytic discrete geometric average-price Asians...");

    // data from "Implementing Derivatives Model",
    // Clewlow, Strickland, p.118-123

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
        BlackScholesMertonProcess(Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS)));

    boost::shared_ptr<PricingEngine> engine(
          new AnalyticDiscreteGeometricAveragePriceAsianEngine(stochProcess));

    Average::Type averageType = Average::Geometric;
    Real runningAccumulator = 1.0;
    Size pastFixings = 0;
    Size futureFixings = 10;
    Option::Type type = Option::Call;
    Real strike = 100.0;
    boost::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

    Date exerciseDate = today + 360;
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    std::vector<Date> fixingDates(futureFixings);
    Integer dt = Integer(360/futureFixings+0.5);
    fixingDates[0] = today + dt;
    for (Size j=1; j<futureFixings; j++)
        fixingDates[j] = fixingDates[j-1] + dt;

    DiscreteAveragingAsianOption option(averageType, runningAccumulator,
                                        pastFixings, fixingDates,
                                        payoff, exercise);
    option.setPricingEngine(engine);

    Real calculated = option.NPV();
    Real expected = 5.3425606635;
    Real tolerance = 1e-10;
    if (std::fabs(calculated-expected) > tolerance) {
        REPORT_FAILURE("value", averageType, runningAccumulator, pastFixings,
                       fixingDates, payoff, exercise, spot->value(),
                       qRate->value(), rRate->value(), today,
                       vol->value(), expected, calculated, tolerance);
    }
}

void AsianOptionTest::testAnalyticDiscreteGeometricAverageStrike() {

    BOOST_TEST_MESSAGE(
              "Testing analytic discrete geometric average-strike Asians...");

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
        BlackScholesMertonProcess(Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS)));

    boost::shared_ptr<PricingEngine> engine(
          new AnalyticDiscreteGeometricAverageStrikeAsianEngine(stochProcess));

    Average::Type averageType = Average::Geometric;
    Real runningAccumulator = 1.0;
    Size pastFixings = 0;
    Size futureFixings = 10;
    Option::Type type = Option::Call;
    Real strike = 100.0;
    boost::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

    Date exerciseDate = today + 360;
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    std::vector<Date> fixingDates(futureFixings);
    Integer dt = Integer(360/futureFixings+0.5);
    fixingDates[0] = today + dt;
    for (Size j=1; j<futureFixings; j++)
        fixingDates[j] = fixingDates[j-1] + dt;

    DiscreteAveragingAsianOption option(averageType, runningAccumulator,
                                        pastFixings, fixingDates,
                                        payoff, exercise);
    option.setPricingEngine(engine);

    Real calculated = option.NPV();
    Real expected = 4.97109;
    Real tolerance = 1e-5;
    if (std::fabs(calculated-expected) > tolerance) {
        REPORT_FAILURE("value", averageType, runningAccumulator, pastFixings,
                       fixingDates, payoff, exercise, spot->value(),
                       qRate->value(), rRate->value(), today,
                       vol->value(), expected, calculated, tolerance);
    }
}


void AsianOptionTest::testMCDiscreteGeometricAveragePrice() {

    BOOST_TEST_MESSAGE(
            "Testing Monte Carlo discrete geometric average-price Asians...");

    // data from "Implementing Derivatives Model",
    // Clewlow, Strickland, p.118-123

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
        BlackScholesMertonProcess(Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS)));

    Real tolerance = 4.0e-3;

    boost::shared_ptr<PricingEngine> engine =
        MakeMCDiscreteGeometricAPEngine<LowDiscrepancy>(stochProcess)
        .withSamples(8191);

    Average::Type averageType = Average::Geometric;
    Real runningAccumulator = 1.0;
    Size pastFixings = 0;
    Size futureFixings = 10;
    Option::Type type = Option::Call;
    Real strike = 100.0;
    boost::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

    Date exerciseDate = today + 360;
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    std::vector<Date> fixingDates(futureFixings);
    Integer dt = Integer(360/futureFixings+0.5);
    fixingDates[0] = today + dt;
    for (Size j=1; j<futureFixings; j++)
        fixingDates[j] = fixingDates[j-1] + dt;

    DiscreteAveragingAsianOption option(averageType, runningAccumulator,
                                        pastFixings, fixingDates,
                                        payoff, exercise);
    option.setPricingEngine(engine);

    Real calculated = option.NPV();

    boost::shared_ptr<PricingEngine> engine2(
          new AnalyticDiscreteGeometricAveragePriceAsianEngine(stochProcess));
    option.setPricingEngine(engine2);
    Real expected = option.NPV();

    if (std::fabs(calculated-expected) > tolerance) {
        REPORT_FAILURE("value", averageType, runningAccumulator, pastFixings,
                       fixingDates, payoff, exercise, spot->value(),
                       qRate->value(), rRate->value(), today,
                       vol->value(), expected, calculated, tolerance);
    }
}


namespace {

    struct DiscreteAverageData {
        Option::Type type;
        Real underlying;
        Real strike;
        Rate dividendYield;
        Rate riskFreeRate;
        Time first;
        Time length;
        Size fixings;
        Volatility volatility;
        bool controlVariate;
        Real result;
    };

}


void AsianOptionTest::testMCDiscreteArithmeticAveragePrice() {

    BOOST_TEST_MESSAGE(
           "Testing Monte Carlo discrete arithmetic average-price Asians...");

    QL_TEST_START_TIMING

    // data from "Asian Option", Levy, 1997
    // in "Exotic Options: The State of the Art",
    // edited by Clewlow, Strickland
    DiscreteAverageData cases4[] = {
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 2,
          0.13, true, 1.3942835683 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 4,
          0.13, true, 1.5852442983 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 8,
          0.13, true, 1.66970673 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 12,
          0.13, true, 1.6980019214 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 26,
          0.13, true, 1.7255070456 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 52,
          0.13, true, 1.7401553533 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 100,
          0.13, true, 1.7478303712 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 250,
          0.13, true, 1.7490291943 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 500,
          0.13, true, 1.7515113291 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 1000,
          0.13, true, 1.7537344885 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 2,
          0.13, true, 1.8496053697 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 4,
          0.13, true, 2.0111495205 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 8,
          0.13, true, 2.0852138818 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 12,
          0.13, true, 2.1105094397 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 26,
          0.13, true, 2.1346526695 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 52,
          0.13, true, 2.147489651 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 100,
          0.13, true, 2.154728109 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 250,
          0.13, true, 2.1564276565 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 500,
          0.13, true, 2.1594238588 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 1000,
          0.13, true, 2.1595367326 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 2,
          0.13, true, 2.63315092584 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 4,
          0.13, true, 2.76723962361 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 8,
          0.13, true, 2.83124836881 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 12,
          0.13, true, 2.84290301412 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 26,
          0.13, true, 2.88179560417 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 52,
          0.13, true, 2.88447044543 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 100,
          0.13, true, 2.89985329603 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 250,
          0.13, true, 2.90047296063 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 500,
          0.13, true, 2.89813412160 },
        { Option::Put, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 1000,
          0.13, true, 2.89703362437 }
   };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);



    Average::Type averageType = Average::Arithmetic;
    Real runningSum = 0.0;
    Size pastFixings = 0;
    for (Size l=0; l<LENGTH(cases4); l++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(cases4[l].type, cases4[l].strike));

        Time dt = cases4[l].length/(cases4[l].fixings-1);
        std::vector<Time> timeIncrements(cases4[l].fixings);
        std::vector<Date> fixingDates(cases4[l].fixings);
        timeIncrements[0] = cases4[l].first;
        fixingDates[0] = today + Integer(timeIncrements[0]*360+0.5);
        for (Size i=1; i<cases4[l].fixings; i++) {
            timeIncrements[i] = i*dt + cases4[l].first;
            fixingDates[i] = today + Integer(timeIncrements[i]*360+0.5);
        }
        boost::shared_ptr<Exercise> exercise(new
            EuropeanExercise(fixingDates[cases4[l].fixings-1]));

        spot ->setValue(cases4[l].underlying);
        qRate->setValue(cases4[l].dividendYield);
        rRate->setValue(cases4[l].riskFreeRate);
        vol  ->setValue(cases4[l].volatility);

        boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));


        boost::shared_ptr<PricingEngine> engine =
            MakeMCDiscreteArithmeticAPEngine<LowDiscrepancy>(stochProcess)
            .withSamples(2047)
            .withControlVariate();

        DiscreteAveragingAsianOption option(averageType, runningSum,
                                            pastFixings, fixingDates,
                                            payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = cases4[l].result;
        Real tolerance = 2.0e-2;
        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value", averageType, runningSum, pastFixings,
                        fixingDates, payoff, exercise, spot->value(),
                        qRate->value(), rRate->value(), today,
                        vol->value(), expected, calculated, tolerance);
        }

        if(cases4[l].fixings < 100) {
            engine = boost::shared_ptr<PricingEngine>(
                    new FdBlackScholesAsianEngine(stochProcess, 100, 100, 100));
            option.setPricingEngine(engine);
            calculated = option.NPV();
            if (std::fabs(calculated-expected) > tolerance) {
                REPORT_FAILURE("value", averageType, runningSum, pastFixings,
                            fixingDates, payoff, exercise, spot->value(),
                            qRate->value(), rRate->value(), today,
                            vol->value(), expected, calculated, tolerance);
            }
        }
    }

}


void AsianOptionTest::testMCDiscreteArithmeticAverageStrike() {

    BOOST_TEST_MESSAGE(
          "Testing Monte Carlo discrete arithmetic average-strike Asians...");

    QL_TEST_START_TIMING

    // data from "Asian Option", Levy, 1997
    // in "Exotic Options: The State of the Art",
    // edited by Clewlow, Strickland
    DiscreteAverageData cases5[] = {
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 2,
          0.13, true, 1.51917595129 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 4,
          0.13, true, 1.67940165674 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 8,
          0.13, true, 1.75371215251 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 12,
          0.13, true, 1.77595318693 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 26,
          0.13, true, 1.81430536630 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 52,
          0.13, true, 1.82269246898 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 100,
          0.13, true, 1.83822402464 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 250,
          0.13, true, 1.83875059026 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 500,
          0.13, true, 1.83750703638 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 0.0, 11.0/12.0, 1000,
          0.13, true, 1.83887181884 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 2,
          0.13, true, 1.51154400089 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 4,
          0.13, true, 1.67103508506 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 8,
          0.13, true, 1.74529684070 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 12,
          0.13, true, 1.76667074564 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 26,
          0.13, true, 1.80528400613 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 52,
          0.13, true, 1.81400883891 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 100,
          0.13, true, 1.82922901451 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 250,
          0.13, true, 1.82937111773 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 500,
          0.13, true, 1.82826193186 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 1.0/12.0, 11.0/12.0, 1000,
          0.13, true, 1.82967846654 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 2,
          0.13, true, 1.49648170891 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 4,
          0.13, true, 1.65443100462 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 8,
          0.13, true, 1.72817806731 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 12,
          0.13, true, 1.74877367895 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 26,
          0.13, true, 1.78733801988 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 52,
          0.13, true, 1.79624826757 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 100,
          0.13, true, 1.81114186876 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 250,
          0.13, true, 1.81101152587 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 500,
          0.13, true, 1.81002311939 },
        { Option::Call, 90.0, 87.0, 0.06, 0.025, 3.0/12.0, 11.0/12.0, 1000,
          0.13, true, 1.81145760308 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    Average::Type averageType = Average::Arithmetic;
    Real runningSum = 0.0;
    Size pastFixings = 0;
    for (Size l=0; l<LENGTH(cases5); l++) {

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(cases5[l].type, cases5[l].strike));

        Time dt = cases5[l].length/(cases5[l].fixings-1);
        std::vector<Time> timeIncrements(cases5[l].fixings);
        std::vector<Date> fixingDates(cases5[l].fixings);
        timeIncrements[0] = cases5[l].first;
        fixingDates[0] = today + Integer(timeIncrements[0]*360+0.5);
        for (Size i=1; i<cases5[l].fixings; i++) {
            timeIncrements[i] = i*dt + cases5[l].first;
            fixingDates[i] = today + Integer(timeIncrements[i]*360+0.5);
        }
        boost::shared_ptr<Exercise> exercise(new
            EuropeanExercise(fixingDates[cases5[l].fixings-1]));

        spot ->setValue(cases5[l].underlying);
        qRate->setValue(cases5[l].dividendYield);
        rRate->setValue(cases5[l].riskFreeRate);
        vol  ->setValue(cases5[l].volatility);

        boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        boost::shared_ptr<PricingEngine> engine =
            MakeMCDiscreteArithmeticASEngine<LowDiscrepancy>(stochProcess)
            .withSeed(3456789)
            .withSamples(1023);

        DiscreteAveragingAsianOption option(averageType, runningSum,
                                            pastFixings, fixingDates,
                                            payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = cases5[l].result;
        Real tolerance = 2.0e-2;
        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value", averageType, runningSum, pastFixings,
                           fixingDates, payoff, exercise, spot->value(),
                           qRate->value(), rRate->value(), today,
                           vol->value(), expected, calculated, tolerance);
        }
    }

}

void AsianOptionTest::testAnalyticDiscreteGeometricAveragePriceGreeks() {

    BOOST_TEST_MESSAGE("Testing discrete-averaging geometric Asian greeks...");

    SavedSettings backup;

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta"]  = 1.0e-5;
    tolerance["gamma"]  = 1.0e-5;
    tolerance["theta"]  = 1.0e-5;
    tolerance["rho"]    = 1.0e-5;
    tolerance["divRho"] = 1.0e-5;
    tolerance["vega"]   = 1.0e-5;

    Option::Type types[] = { Option::Call, Option::Put };
    Real underlyings[] = { 100.0 };
    Real strikes[] = { 90.0, 100.0, 110.0 };
    Rate qRates[] = { 0.04, 0.05, 0.06 };
    Rate rRates[] = { 0.01, 0.05, 0.15 };
    Integer lengths[] = { 1, 2 };
    Volatility vols[] = { 0.11, 0.50, 1.20 };

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

    boost::shared_ptr<BlackScholesMertonProcess> process(
         new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

    for (Size i=0; i<LENGTH(types); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(lengths); k++) {

            boost::shared_ptr<EuropeanExercise> maturity(
                              new EuropeanExercise(today + lengths[k]*Years));

            boost::shared_ptr<PlainVanillaPayoff> payoff(
                                new PlainVanillaPayoff(types[i], strikes[j]));

            Real runningAverage = 120;
            Size pastFixings = 1;

            std::vector<Date> fixingDates;
            for (Date d = today + 3*Months;
                      d <= maturity->lastDate();
                      d += 3*Months)
                fixingDates.push_back(d);


            boost::shared_ptr<PricingEngine> engine(
               new AnalyticDiscreteGeometricAveragePriceAsianEngine(process));

            DiscreteAveragingAsianOption option(Average::Geometric,
                                                runningAverage, pastFixings,
                                                fixingDates, payoff, maturity);
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
                                  REPORT_FAILURE(greek, Average::Geometric,
                                                 runningAverage, pastFixings,
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


void AsianOptionTest::testPastFixings() {

    BOOST_TEST_MESSAGE("Testing use of past fixings in Asian options...");

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    boost::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    boost::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                  new PlainVanillaPayoff(Option::Put, 100.0));


    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(today + 1*Years));

    boost::shared_ptr<BlackScholesMertonProcess> stochProcess(
        new BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

    // MC arithmetic average-price

    Real runningSum = 0.0;
    Size pastFixings = 0;
    std::vector<Date> fixingDates1;
    for (Integer i=0; i<=12; ++i)
        fixingDates1.push_back(today + i*Months);

    DiscreteAveragingAsianOption option1(Average::Arithmetic, runningSum,
                                         pastFixings, fixingDates1,
                                         payoff, exercise);

    pastFixings = 2;
    runningSum = pastFixings * spot->value() * 0.8;
    std::vector<Date> fixingDates2;
    for (Integer i=-2; i<=12; ++i)
        fixingDates2.push_back(today + i*Months);

    DiscreteAveragingAsianOption option2(Average::Arithmetic, runningSum,
                                         pastFixings, fixingDates2,
                                         payoff, exercise);

    boost::shared_ptr<PricingEngine> engine =
        MakeMCDiscreteArithmeticAPEngine<LowDiscrepancy>(stochProcess)
        .withSamples(2047);

    option1.setPricingEngine(engine);
    option2.setPricingEngine(engine);

    Real price1 = option1.NPV();
    Real price2 = option2.NPV();

    if (close(price1, price2)) {
        BOOST_ERROR(
             "past fixings had no effect on arithmetic average-price option"
             << "\n  without fixings: " << price1
             << "\n  with fixings:    " << price2);
    }

    // MC arithmetic average-strike

    engine =
        MakeMCDiscreteArithmeticASEngine<LowDiscrepancy>(stochProcess)
        .withSamples(2047);

    option1.setPricingEngine(engine);
    option2.setPricingEngine(engine);

    price1 = option1.NPV();
    price2 = option2.NPV();

    if (close(price1, price2)) {
        BOOST_ERROR(
             "past fixings had no effect on arithmetic average-strike option"
             << "\n  without fixings: " << price1
             << "\n  with fixings:    " << price2);
    }

    // analytic geometric average-price

    Real runningProduct = 1.0;
    pastFixings = 0;

    DiscreteAveragingAsianOption option3(Average::Geometric, runningProduct,
                                         pastFixings, fixingDates1,
                                         payoff, exercise);

    pastFixings = 2;
    runningProduct = spot->value() * spot->value();

    DiscreteAveragingAsianOption option4(Average::Geometric, runningProduct,
                                         pastFixings, fixingDates2,
                                         payoff, exercise);

    engine = boost::shared_ptr<PricingEngine>(
          new AnalyticDiscreteGeometricAveragePriceAsianEngine(stochProcess));

    option3.setPricingEngine(engine);
    option4.setPricingEngine(engine);

    Real price3 = option3.NPV();
    Real price4 = option4.NPV();

    if (close(price3, price4)) {
        BOOST_ERROR(
             "past fixings had no effect on geometric average-price option"
             << "\n  without fixings: " << price3
             << "\n  with fixings:    " << price4);
    }

    // MC geometric average-price

    engine =
        MakeMCDiscreteGeometricAPEngine<LowDiscrepancy>(stochProcess)
        .withSamples(2047);

    option3.setPricingEngine(engine);
    option4.setPricingEngine(engine);

    price3 = option3.NPV();
    price4 = option4.NPV();

    if (close(price3, price4)) {
        BOOST_ERROR(
             "past fixings had no effect on geometric average-price option"
             << "\n  without fixings: " << price3
             << "\n  with fixings:    " << price4);
    }

}

namespace {

    struct ContinuousAverageData {
        Option::Type type;
        Real spot;
        Real currentAverage;
        Real strike;
        Rate dividendYield;
        Rate riskFreeRate;
        Volatility volatility;
        Natural length;
        Natural elapsed;
        Real result;
    };

}

void AsianOptionTest::testLevyEngine() {

    BOOST_TEST_MESSAGE("Testing Levy engine for Asians options...");

    // data from Haug, "Option Pricing Formulas", p.99-100
    ContinuousAverageData cases[] = {
      { Option::Call, 6.80, 6.80, 6.90, 0.09, 0.07, 0.14, 180, 0, 0.0944 },
      { Option::Put,  6.80, 6.80, 6.90, 0.09, 0.07, 0.14, 180, 0, 0.2237 },
      { Option::Call, 100.0, 100.0, 95.0, 0.05, 0.1, 0.15, 270, 0, 7.0544 },
      { Option::Call, 100.0, 100.0, 95.0, 0.05, 0.1, 0.15, 270, 90, 5.6731 },
      { Option::Call, 100.0, 100.0, 95.0, 0.05, 0.1, 0.15, 270, 180, 5.0806 },
      { Option::Call, 100.0, 100.0, 95.0, 0.05, 0.1, 0.35, 270, 0, 10.1213 },
      { Option::Call, 100.0, 100.0, 95.0, 0.05, 0.1, 0.35, 270, 90, 6.9705 },
      { Option::Call, 100.0, 100.0, 95.0, 0.05, 0.1, 0.35, 270, 180, 5.1411 },
      { Option::Call, 100.0, 100.0, 100.0, 0.05, 0.1, 0.15, 270, 0, 3.7845 },
      { Option::Call, 100.0, 100.0, 100.0, 0.05, 0.1, 0.15, 270, 90, 1.9964 },
      { Option::Call, 100.0, 100.0, 100.0, 0.05, 0.1, 0.15, 270, 180, 0.6722 },
      { Option::Call, 100.0, 100.0, 100.0, 0.05, 0.1, 0.35, 270, 0, 7.5038 },
      { Option::Call, 100.0, 100.0, 100.0, 0.05, 0.1, 0.35, 270, 90, 4.0687 },
      { Option::Call, 100.0, 100.0, 100.0, 0.05, 0.1, 0.35, 270, 180, 1.4222 },
      { Option::Call, 100.0, 100.0, 105.0, 0.05, 0.1, 0.15, 270, 0, 1.6729 },
      { Option::Call, 100.0, 100.0, 105.0, 0.05, 0.1, 0.15, 270, 90, 0.3565 },
      { Option::Call, 100.0, 100.0, 105.0, 0.05, 0.1, 0.15, 270, 180, 0.0004 },
      { Option::Call, 100.0, 100.0, 105.0, 0.05, 0.1, 0.35, 270, 0, 5.4071 },
      { Option::Call, 100.0, 100.0, 105.0, 0.05, 0.1, 0.35, 270, 90, 2.1359 },
      { Option::Call, 100.0, 100.0, 105.0, 0.05, 0.1, 0.35, 270, 180, 0.1552 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    for (Size l=0; l<LENGTH(cases); l++) {

        boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(cases[l].spot));
        boost::shared_ptr<YieldTermStructure> qTS =
            flatRate(today, cases[l].dividendYield, dc);
        boost::shared_ptr<YieldTermStructure> rTS =
            flatRate(today, cases[l].riskFreeRate, dc);
        boost::shared_ptr<BlackVolTermStructure> volTS =
            flatVol(today, cases[l].volatility, dc);

        Average::Type averageType = Average::Arithmetic;
        boost::shared_ptr<Quote> average(
                                    new SimpleQuote(cases[l].currentAverage));

        boost::shared_ptr<StrikedTypePayoff> payoff(
                      new PlainVanillaPayoff(cases[l].type, cases[l].strike));

        Date startDate = today - cases[l].elapsed;
        Date maturity = startDate + cases[l].length;

        boost::shared_ptr<Exercise> exercise(new EuropeanExercise(maturity));

        boost::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        boost::shared_ptr<PricingEngine> engine(
            new ContinuousArithmeticAsianLevyEngine(
                     stochProcess, Handle<Quote>(average), startDate));

        ContinuousAveragingAsianOption option(averageType,
                                              payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = cases[l].result;
        Real tolerance = 1.0e-4;
        Real error = std::fabs(expected-calculated);
        if (error > tolerance) {
            BOOST_ERROR("Asian option with Levy engine:"
                        << "\n    spot:            " << cases[l].spot
                        << "\n    current average: " << cases[l].currentAverage
                        << "\n    strike:          " << cases[l].strike
                        << "\n    dividend yield:  " << cases[l].dividendYield
                        << "\n    risk-free rate:  " << cases[l].riskFreeRate
                        << "\n    volatility:      " << cases[l].volatility
                        << "\n    reference date:  " << today
                        << "\n    length:          " << cases[l].length
                        << "\n    elapsed:         " << cases[l].elapsed
                        << "\n    expected value:  " << expected
                        << "\n    calculated:      " << calculated
                        << "\n    error:           " << error);
        }
    }
}

test_suite* AsianOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Asian option tests");

    suite->add(QUANTLIB_TEST_CASE(
        &AsianOptionTest::testAnalyticContinuousGeometricAveragePrice));
    suite->add(QUANTLIB_TEST_CASE(
        &AsianOptionTest::testAnalyticContinuousGeometricAveragePriceGreeks));
    suite->add(QUANTLIB_TEST_CASE(
        &AsianOptionTest::testAnalyticDiscreteGeometricAveragePrice));
    suite->add(QUANTLIB_TEST_CASE(
        &AsianOptionTest::testAnalyticDiscreteGeometricAverageStrike));
    suite->add(QUANTLIB_TEST_CASE(
        &AsianOptionTest::testMCDiscreteGeometricAveragePrice));
    suite->add(QUANTLIB_TEST_CASE(
        &AsianOptionTest::testMCDiscreteArithmeticAveragePrice));
    suite->add(QUANTLIB_TEST_CASE(
        &AsianOptionTest::testMCDiscreteArithmeticAverageStrike));
    suite->add(QUANTLIB_TEST_CASE(
        &AsianOptionTest::testAnalyticDiscreteGeometricAveragePriceGreeks));
    suite->add(QUANTLIB_TEST_CASE(
        &AsianOptionTest::testPastFixings));

    return suite;
}

test_suite* AsianOptionTest::experimental() {
    test_suite* suite = BOOST_TEST_SUITE("Asian option tests");
    suite->add(QUANTLIB_TEST_CASE(&AsianOptionTest::testLevyEngine));
    return suite;
}

