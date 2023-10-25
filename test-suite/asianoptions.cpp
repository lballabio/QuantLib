/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2005, 2007, 2008, 2017 StatPro Italia srl
 Copyright (C) 2009, 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis
 Copyright (C) 2014 Bernd Lewerenz
 Copyright (C) 2020, 2021 Jack Gillett
 Copyright (C) 2021 Skandinaviska Enskilda Banken AB (publ)

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

#include "speedlevel.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/instruments/asianoption.hpp>
#include <ql/pricingengines/asian/analytic_discr_geom_av_price.hpp>
#include <ql/pricingengines/asian/analytic_discr_geom_av_strike.hpp>
#include <ql/pricingengines/asian/analytic_cont_geom_av_price.hpp>
#include <ql/pricingengines/asian/mc_discr_geom_av_price.hpp>
#include <ql/pricingengines/asian/mc_discr_geom_av_price_heston.hpp>
#include <ql/pricingengines/asian/mc_discr_arith_av_price.hpp>
#include <ql/pricingengines/asian/mc_discr_arith_av_price_heston.hpp>
#include <ql/pricingengines/asian/mc_discr_arith_av_strike.hpp>
#include <ql/pricingengines/asian/fdblackscholesasianengine.hpp>
#include <ql/experimental/exoticoptions/continuousarithmeticasianlevyengine.hpp>
#include <ql/experimental/exoticoptions/continuousarithmeticasianvecerengine.hpp>
#include <ql/experimental/asian/analytic_cont_geom_av_price_heston.hpp>
#include <ql/experimental/asian/analytic_discr_geom_av_price_heston.hpp>
#include <ql/pricingengines/asian/turnbullwakemanasianengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef REPORT_FAILURE
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

    struct DiscreteAverageDataTermStructure {
        Option::Type type;
        Real underlying;
        Real strike;
        Rate b;
        Rate riskFreeRate;
        Time first; // t1
        Time expiry;
        Size fixings;
        Volatility volatility;
        std::string slope;
        Real result;
    };

    struct VecerData {
        Real spot;
        Rate riskFreeRate;
        Volatility volatility;
        Real strike;
        Natural length;
        Real result;
        Real tolerance;
    };
}

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(AsianOptionTest)

BOOST_AUTO_TEST_CASE(testAnalyticContinuousGeometricAveragePrice) {

    BOOST_TEST_MESSAGE(
             "Testing analytic continuous geometric average-price Asians...");

    // data from "Option Pricing Formulas", Haug, pag.96-97

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(80.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(-0.03));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
        BlackScholesMertonProcess(Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<PricingEngine> engine(new
            AnalyticContinuousGeometricAveragePriceAsianEngine(stochProcess));

    Average::Type averageType = Average::Geometric;
    Option::Type type = Option::Put;
    Real strike = 85.0;
    Date exerciseDate = today + 90;

    Size pastFixings = Null<Size>();
    Real runningAccumulator = Null<Real>();

    ext::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

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
    ext::shared_ptr<PricingEngine> engine2(new
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

BOOST_AUTO_TEST_CASE(testAnalyticContinuousGeometricAveragePriceGreeks) {

    BOOST_TEST_MESSAGE(
       "Testing analytic continuous geometric average-price Asian greeks...");

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
    Date today = Settings::instance().evaluationDate();
    Settings::instance().evaluationDate() = today;

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    ext::shared_ptr<BlackScholesMertonProcess> process(
         new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

    for (auto& type : types) {
        for (Real strike : strikes) {
            for (int length : lengths) {

                ext::shared_ptr<EuropeanExercise> maturity(
                    new EuropeanExercise(today + length * Years));

                ext::shared_ptr<PlainVanillaPayoff> payoff(new PlainVanillaPayoff(type, strike));

                ext::shared_ptr<PricingEngine> engine(
                    new AnalyticContinuousGeometricAveragePriceAsianEngine(process));

                ContinuousAveragingAsianOption option(Average::Geometric, payoff, maturity);
                option.setPricingEngine(engine);

                Size pastFixings = Null<Size>();
                Real runningAverage = Null<Real>();

                for (Real u : underlyings) {
                    for (Real m : qRates) {
                        for (Real n : rRates) {
                            for (Real v : vols) {

                                Rate q = m, r = n;
                                spot->setValue(u);
                                qRate->setValue(q);
                                rRate->setValue(r);
                                vol->setValue(v);

                                Real value = option.NPV();
                                calculated["delta"] = option.delta();
                                calculated["gamma"] = option.gamma();
                                calculated["theta"] = option.theta();
                                calculated["rho"] = option.rho();
                                calculated["divRho"] = option.dividendRho();
                                calculated["vega"] = option.vega();

                                if (value > spot->value() * 1.0e-5) {
                                    // perturb spot and get delta and gamma
                                    Real du = u * 1.0e-4;
                                    spot->setValue(u + du);
                                    Real value_p = option.NPV(), delta_p = option.delta();
                                    spot->setValue(u - du);
                                    Real value_m = option.NPV(), delta_m = option.delta();
                                    spot->setValue(u);
                                    expected["delta"] = (value_p - value_m) / (2 * du);
                                    expected["gamma"] = (delta_p - delta_m) / (2 * du);

                                    // perturb rates and get rho and dividend rho
                                    Spread dr = r * 1.0e-4;
                                    rRate->setValue(r + dr);
                                    value_p = option.NPV();
                                    rRate->setValue(r - dr);
                                    value_m = option.NPV();
                                    rRate->setValue(r);
                                    expected["rho"] = (value_p - value_m) / (2 * dr);

                                    Spread dq = q * 1.0e-4;
                                    qRate->setValue(q + dq);
                                    value_p = option.NPV();
                                    qRate->setValue(q - dq);
                                    value_m = option.NPV();
                                    qRate->setValue(q);
                                    expected["divRho"] = (value_p - value_m) / (2 * dq);

                                    // perturb volatility and get vega
                                    Volatility dv = v * 1.0e-4;
                                    vol->setValue(v + dv);
                                    value_p = option.NPV();
                                    vol->setValue(v - dv);
                                    value_m = option.NPV();
                                    vol->setValue(v);
                                    expected["vega"] = (value_p - value_m) / (2 * dv);

                                    // perturb date and get theta
                                    Time dT = dc.yearFraction(today - 1, today + 1);
                                    Settings::instance().evaluationDate() = today - 1;
                                    value_m = option.NPV();
                                    Settings::instance().evaluationDate() = today + 1;
                                    value_p = option.NPV();
                                    Settings::instance().evaluationDate() = today;
                                    expected["theta"] = (value_p - value_m) / dT;

                                    // compare
                                    std::map<std::string, Real>::iterator it;
                                    for (it = calculated.begin(); it != calculated.end(); ++it) {
                                        std::string greek = it->first;
                                        Real expct = expected[greek], calcl = calculated[greek],
                                             tol = tolerance[greek];
                                        Real error = relativeError(expct, calcl, u);
                                        if (error > tol) {
                                            REPORT_FAILURE(greek, Average::Geometric,
                                                           runningAverage, pastFixings,
                                                           std::vector<Date>(), payoff, maturity, u,
                                                           q, r, today, v, expct, calcl, tol);
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

BOOST_AUTO_TEST_CASE(testAnalyticDiscreteGeometricAveragePrice) {

    BOOST_TEST_MESSAGE(
               "Testing analytic discrete geometric average-price Asians...");

    // data from "Implementing Derivatives Model",
    // Clewlow, Strickland, p.118-123

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
        BlackScholesMertonProcess(Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<PricingEngine> engine(
          new AnalyticDiscreteGeometricAveragePriceAsianEngine(stochProcess));

    Average::Type averageType = Average::Geometric;
    Real runningAccumulator = 1.0;
    Size pastFixings = 0;
    Size futureFixings = 10;
    Option::Type type = Option::Call;
    Real strike = 100.0;
    ext::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

    Date exerciseDate = today + 360;
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    std::vector<Date> fixingDates(futureFixings);
    auto dt = (Integer)std::lround(360.0 / futureFixings);
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

BOOST_AUTO_TEST_CASE(testAnalyticDiscreteGeometricAverageStrike) {

    BOOST_TEST_MESSAGE(
              "Testing analytic discrete geometric average-strike Asians...");

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
        BlackScholesMertonProcess(Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<PricingEngine> engine(
          new AnalyticDiscreteGeometricAverageStrikeAsianEngine(stochProcess));

    Average::Type averageType = Average::Geometric;
    Real runningAccumulator = 1.0;
    Size pastFixings = 0;
    Size futureFixings = 10;
    Option::Type type = Option::Call;
    Real strike = 100.0;
    ext::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

    Date exerciseDate = today + 360;
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    std::vector<Date> fixingDates(futureFixings);
    auto dt = (Integer)std::lround(360.0 / futureFixings);
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

BOOST_AUTO_TEST_CASE(testMCDiscreteGeometricAveragePrice) {

    BOOST_TEST_MESSAGE(
            "Testing Monte Carlo discrete geometric average-price Asians...");

    // data from "Implementing Derivatives Model",
    // Clewlow, Strickland, p.118-123

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
        BlackScholesMertonProcess(Handle<Quote>(spot),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS)));

    Real tolerance = 4.0e-3;

    ext::shared_ptr<PricingEngine> engine =
        MakeMCDiscreteGeometricAPEngine<LowDiscrepancy>(stochProcess)
        .withSamples(8191);

    Average::Type averageType = Average::Geometric;
    Real runningAccumulator = 1.0;
    Size pastFixings = 0;
    Size futureFixings = 10;
    Option::Type type = Option::Call;
    Real strike = 100.0;
    ext::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

    Date exerciseDate = today + 360;
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    std::vector<Date> fixingDates(futureFixings);
    auto dt = (Integer)std::lround(360.0 / futureFixings);
    fixingDates[0] = today + dt;
    for (Size j=1; j<futureFixings; j++)
        fixingDates[j] = fixingDates[j-1] + dt;

    DiscreteAveragingAsianOption option(averageType, runningAccumulator,
                                        pastFixings, fixingDates,
                                        payoff, exercise);
    option.setPricingEngine(engine);

    Real calculated = option.NPV();

    ext::shared_ptr<PricingEngine> engine2(
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

void testDiscreteGeometricAveragePriceHeston(const ext::shared_ptr<PricingEngine>& engine,
                                             const Real tol[]) {

    // data from "A Recursive Method for Discretely Monitored Geometric Asian Option
    // Prices", Kim, Kim, Kim & Wee, Bull. Korean Math. Soc. 53, 733-749, 2016
    int days[] = {
        30, 91, 182, 365, 730, 1095,
        30, 91, 182, 365, 730, 1095,
        30, 91, 182, 365, 730, 1095
    };
    Real strikes[] = {
        90, 90, 90, 90, 90, 90,
        100, 100, 100, 100, 100, 100,
        110, 110, 110, 110, 110, 110
    };

    // Prices from Tables 1, 2 and 3
    Real prices[] = {
        10.2732, 10.9554, 11.9916, 13.6950, 16.1773, 18.0146,
        2.4389, 3.7881, 5.2132, 7.2243, 9.9948, 12.0639,
        0.1012, 0.5949, 1.4444, 2.9479, 5.3531, 7.3315
    };

    DayCounter dc = Actual365Fixed();
    Date today = Settings::instance().evaluationDate();

    Handle<Quote> spot(ext::shared_ptr<Quote>(new SimpleQuote(100)));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));

    Real v0 = 0.09;

    Option::Type type(Option::Call);
    Average::Type averageType = Average::Geometric;

    Real runningAccumulator = 1.0;
    Size pastFixings = 0;

    for (Size i=0; i<LENGTH(strikes); i++) {
        Real strike = strikes[i];
        int day = days[i];
        Real expected = prices[i];
        Real tolerance = tol[i];

        Size futureFixings = int(std::floor(day/7.0));
        std::vector<Date> fixingDates(futureFixings);

        Date expiryDate = today + day*Days;

        // I suppose "weekly fixings" roughly means this?
        for (int i=futureFixings-1; i>=0; i--) {
            fixingDates[i] = expiryDate - i * 7;
        }

        ext::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(expiryDate));
        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

        DiscreteAveragingAsianOption option(averageType, runningAccumulator, pastFixings,
                                            fixingDates, payoff, europeanExercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();

        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value", averageType, 1.0, 0.0,
                       std::vector<Date>(), payoff, europeanExercise, spot->value(),
                       qRate->value(), rRate->value(), today,
                       std::sqrt(v0), expected, calculated, tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testMCDiscreteGeometricAveragePriceHeston, *precondition(if_speed(Fast))) {

    BOOST_TEST_MESSAGE("Testing MC discrete geometric average-price Asians under Heston...");

    // 30-day options need wider tolerance due to uncertainty around what "weekly
    // fixing" dates mean over a 30-day month!
    Real tol[] = {
        4.0e-2, 2.0e-2, 2.0e-2, 4.0e-2, 8.0e-2, 2.0e-1,
        1.0e-1, 4.0e-2, 3.0e-2, 2.0e-2, 9.0e-2, 2.0e-1,
        2.0e-2, 1.0e-2, 2.0e-2, 2.0e-2, 7.0e-2, 2.0e-1
    };

    DayCounter dc = Actual365Fixed();
    Date today = Settings::instance().evaluationDate();

    Handle<Quote> spot(ext::shared_ptr<Quote>(new SimpleQuote(100)));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    Real v0 = 0.09;
    Real kappa = 1.15;
    Real theta = 0.0348;
    Real sigma = 0.39;
    Real rho = -0.64;

    ext::shared_ptr<HestonProcess> hestonProcess(new
        HestonProcess(Handle<YieldTermStructure>(rTS), Handle<YieldTermStructure>(qTS),
            spot, v0, kappa, theta, sigma, rho));

    ext::shared_ptr<PricingEngine> engine =
        MakeMCDiscreteGeometricAPHestonEngine<LowDiscrepancy>(hestonProcess)
        .withSamples(8191)
        .withSeed(43);

    testDiscreteGeometricAveragePriceHeston(engine, tol);
}

BOOST_AUTO_TEST_CASE(testMCDiscreteArithmeticAveragePrice, *precondition(if_speed(Fast))) {

    BOOST_TEST_MESSAGE(
           "Testing Monte Carlo discrete arithmetic average-price Asians...");

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
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);



    Average::Type averageType = Average::Arithmetic;
    Real runningSum = 0.0;
    Size pastFixings = 0;
    for (auto& l : cases4) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(l.type, l.strike));

        Time dt = l.length / (l.fixings - 1);
        std::vector<Time> timeIncrements(l.fixings);
        std::vector<Date> fixingDates(l.fixings);
        timeIncrements[0] = l.first;
        fixingDates[0] = today + timeToDays(timeIncrements[0]);
        for (Size i = 1; i < l.fixings; i++) {
            timeIncrements[i] = i * dt + l.first;
            fixingDates[i] = today + timeToDays(timeIncrements[i]);
        }
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(fixingDates[l.fixings - 1]));

        spot->setValue(l.underlying);
        qRate->setValue(l.dividendYield);
        rRate->setValue(l.riskFreeRate);
        vol->setValue(l.volatility);

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));


        ext::shared_ptr<PricingEngine> engine =
            MakeMCDiscreteArithmeticAPEngine<LowDiscrepancy>(stochProcess)
                .withSamples(2047)
                .withControlVariate(l.controlVariate);

        DiscreteAveragingAsianOption option(averageType, runningSum,
                                            pastFixings, fixingDates,
                                            payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = l.result;
        Real tolerance = 2.0e-2;
        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value", averageType, runningSum, pastFixings,
                        fixingDates, payoff, exercise, spot->value(),
                        qRate->value(), rRate->value(), today,
                        vol->value(), expected, calculated, tolerance);
        }

        if (l.fixings < 100) {
            engine = ext::shared_ptr<PricingEngine>(
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

        engine = ext::make_shared<TurnbullWakemanAsianEngine>(stochProcess);
        option.setPricingEngine(engine);
        calculated = option.NPV();
        tolerance = 3.0e-2;
        if (std::fabs(calculated - expected) > tolerance) {
            BOOST_TEST_MESSAGE(
                "The consistency check of the analytic approximation engine failed");
            REPORT_FAILURE("value", averageType, runningSum, pastFixings, fixingDates, payoff,
                           exercise, spot->value(), qRate->value(), rRate->value(), today,
                           vol->value(), expected, calculated, tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testMCDiscreteArithmeticAveragePriceHeston, *precondition(if_speed(Slow))) {

    BOOST_TEST_MESSAGE(
           "Testing Monte Carlo discrete arithmetic average-price Asians in Heston model...");

    // data from "A numerical method to price exotic path-dependent
    // options on an underlying described by the Heston stochastic
    // volatility model", Ballestra, Pacelli and Zirilli, Journal
    // of Banking & Finance, 2007 (section 4 - Numerical Results)

    // nb. for Heston, the volatility param below is ignored
    DiscreteAverageData cases[] = {
        { Option::Call, 120.0, 100.0, 0.0, 0.05, 1.0/12.0, 11.0/12.0, 12,
          0.1, false, 22.50 }
    };

    Real vol = 0.3;
    Real v0 = vol*vol;
    Real kappa = 11.35;
    Real theta = 0.022;
    Real sigma = 0.618;
    Real rho = -0.5;

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    Average::Type averageType = Average::Arithmetic;
    Real runningSum = 0.0;
    Size pastFixings = 0;

    for (auto& l : cases) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(l.type, l.strike));

        Time dt = l.length / (l.fixings - 1);
        std::vector<Time> timeIncrements(l.fixings);
        std::vector<Date> fixingDates(l.fixings);
        timeIncrements[0] = l.first;
        fixingDates[0] = today + Integer(timeIncrements[0]*365.25);
        for (Size i = 1; i < l.fixings; i++) {
            timeIncrements[i] = i * dt + l.first;
            fixingDates[i] = today + Integer(timeIncrements[i]*365.25);
        }
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(fixingDates[l.fixings - 1]));

        spot->setValue(l.underlying);
        qRate->setValue(l.dividendYield);
        rRate->setValue(l.riskFreeRate);

        ext::shared_ptr<HestonProcess> hestonProcess(new
            HestonProcess(Handle<YieldTermStructure>(rTS),
            Handle<YieldTermStructure>(qTS),
            Handle<Quote>(spot),
            v0, kappa, theta, sigma, rho));

        ext::shared_ptr<PricingEngine> engine =
            MakeMCDiscreteArithmeticAPHestonEngine<LowDiscrepancy>(hestonProcess)
                .withSeed(42)
                .withSamples(4095);

        DiscreteAveragingAsianOption option(averageType, runningSum,
                                            pastFixings, fixingDates,
                                            payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = l.result;
        // Bounds given in paper, "22.48 to 22.52"
        Real tolerance = 5.0e-2;

        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value", averageType, runningSum, pastFixings,
                        fixingDates, payoff, exercise, spot->value(),
                        qRate->value(), rRate->value(), today,
                        vol, expected, calculated, tolerance);
        }

        // Also test the control variate version of the pricer
        ext::shared_ptr<PricingEngine> engine2 =
            MakeMCDiscreteArithmeticAPHestonEngine<LowDiscrepancy>(hestonProcess)
                .withSeed(42)
                .withSteps(48)
                .withSamples(4095)
                .withControlVariate(true);

        option.setPricingEngine(engine2);

        Real calculatedCV = option.NPV();
        Real expectedCV = l.result;
        tolerance = 3.0e-2;

        if (std::fabs(calculatedCV-expectedCV) > tolerance) {
            REPORT_FAILURE("value", averageType, runningSum, pastFixings,
                        fixingDates, payoff, exercise, spot->value(),
                        qRate->value(), rRate->value(), today,
                        vol, expectedCV, calculatedCV, tolerance);
        }
    }

    // An additional dataset using the Heston parameters coming from "General lower 
    // bounds for arithmetic Asian option prices", Applied Mathematical Finance 15(2)
    // 123-149 (2008), by Albrecher, H., Mayer, P., and Schoutens, W. The numerical
    // accuracy of prices given in Table 6 is low, but higher accuracy prices for the
    // same parameters and options are reported by in "Pricing bounds and approximations
    // for discrete arithmetic Asian options under time-changed Levy processes" by Zeng,
    // P.P., and Kwok Y.K. (2013) in Table 4.
    Real strikes[] = {60.0, 80.0, 100.0, 120.0, 140.0};
    Real prices[] = {42.5990, 29.3698, 18.2360, 10.0565, 4.9609};

    Real v02 = 0.0175;
    Real kappa2 = 1.5768;
    Real theta2 = 0.0398;
    Real sigma2 = 0.5751;
    Real rho2 = -0.5711;

    DayCounter dc2 = Actual365Fixed();

    ext::shared_ptr<SimpleQuote> spot2(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate2(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS2 = flatRate(today, qRate2, dc2);
    ext::shared_ptr<SimpleQuote> rRate2(new SimpleQuote(0.03));
    ext::shared_ptr<YieldTermStructure> rTS2 = flatRate(today, rRate2, dc2);

    ext::shared_ptr<HestonProcess> hestonProcess2(new
        HestonProcess(Handle<YieldTermStructure>(rTS2),
        Handle<YieldTermStructure>(qTS2),
        Handle<Quote>(spot2),
        v02, kappa2, theta2, sigma2, rho2));

    ext::shared_ptr<PricingEngine> engine3 =
        MakeMCDiscreteArithmeticAPHestonEngine<LowDiscrepancy>(hestonProcess2)
            .withSeed(42)
            .withSteps(180)
            .withSamples(8191);

    ext::shared_ptr<PricingEngine> engine4 =
        MakeMCDiscreteArithmeticAPHestonEngine<LowDiscrepancy>(hestonProcess2)
            .withSeed(42)
            .withSteps(180)
            .withSamples(8191)
            .withControlVariate(true);

    std::vector<Date> fixingDates(120);
    for (Size i=1; i<=120; i++) {
        fixingDates[i-1] = today + Period(i, Months);
    }

    ext::shared_ptr<Exercise> exercise(new
        EuropeanExercise(fixingDates[119]));

    for (Size i=0; i<LENGTH(prices); i++) {
        Real strike = strikes[i];
        Real expected = prices[i];

        ext::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(Option::Call, strike));

        DiscreteAveragingAsianOption option(averageType, runningSum,
                                            pastFixings, fixingDates,
                                            payoff, exercise);

        option.setPricingEngine(engine3);
        Real calculated = option.NPV();
        Real tolerance = 9.0e-2;

        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value", averageType, runningSum, pastFixings,
                        fixingDates, payoff, exercise, spot->value(),
                        qRate2->value(), rRate2->value(), today,
                        vol, expected, calculated, tolerance);
        }

        option.setPricingEngine(engine4);
        calculated = option.NPV();
        tolerance = 3.0e-2;

        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value", averageType, runningSum, pastFixings,
                        fixingDates, payoff, exercise, spot->value(),
                        qRate2->value(), rRate2->value(), today,
                        vol, expected, calculated, tolerance);
        }
    }
}


BOOST_AUTO_TEST_CASE(testMCDiscreteArithmeticAverageStrike) {

    BOOST_TEST_MESSAGE(
          "Testing Monte Carlo discrete arithmetic average-strike Asians...");

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
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    Average::Type averageType = Average::Arithmetic;
    Real runningSum = 0.0;
    Size pastFixings = 0;
    for (auto& l : cases5) {

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(l.type, l.strike));

        Time dt = l.length / (l.fixings - 1);
        std::vector<Time> timeIncrements(l.fixings);
        std::vector<Date> fixingDates(l.fixings);
        timeIncrements[0] = l.first;
        fixingDates[0] = today + timeToDays(timeIncrements[0]);
        for (Size i = 1; i < l.fixings; i++) {
            timeIncrements[i] = i * dt + l.first;
            fixingDates[i] = today + timeToDays(timeIncrements[i]);
        }
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(fixingDates[l.fixings - 1]));

        spot->setValue(l.underlying);
        qRate->setValue(l.dividendYield);
        rRate->setValue(l.riskFreeRate);
        vol->setValue(l.volatility);

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
            BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine =
            MakeMCDiscreteArithmeticASEngine<LowDiscrepancy>(stochProcess)
            .withSeed(3456789)
            .withSamples(1023);

        DiscreteAveragingAsianOption option(averageType, runningSum,
                                            pastFixings, fixingDates,
                                            payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = l.result;
        Real tolerance = 2.0e-2;
        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value", averageType, runningSum, pastFixings,
                           fixingDates, payoff, exercise, spot->value(),
                           qRate->value(), rRate->value(), today,
                           vol->value(), expected, calculated, tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testAnalyticDiscreteGeometricAveragePriceGreeks) {

    BOOST_TEST_MESSAGE("Testing discrete-averaging geometric Asian greeks...");

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
    Date today = Settings::instance().evaluationDate();
    Settings::instance().evaluationDate() = today;

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    ext::shared_ptr<BlackScholesMertonProcess> process(
         new BlackScholesMertonProcess(Handle<Quote>(spot), qTS, rTS, volTS));

    for (auto& type : types) {
        for (Real strike : strikes) {
            for (int length : lengths) {

                ext::shared_ptr<EuropeanExercise> maturity(
                    new EuropeanExercise(today + length * Years));

                ext::shared_ptr<PlainVanillaPayoff> payoff(new PlainVanillaPayoff(type, strike));

                Real runningAverage = 120;
                Size pastFixings = 1;

                std::vector<Date> fixingDates;
                for (Date d = today + 3 * Months; d <= maturity->lastDate(); d += 3 * Months)
                    fixingDates.push_back(d);


                ext::shared_ptr<PricingEngine> engine(
                    new AnalyticDiscreteGeometricAveragePriceAsianEngine(process));

                DiscreteAveragingAsianOption option(Average::Geometric, runningAverage, pastFixings,
                                                    fixingDates, payoff, maturity);
                option.setPricingEngine(engine);

                for (Real u : underlyings) {
                    for (Real m : qRates) {
                        for (Real n : rRates) {
                            for (Real v : vols) {

                                Rate q = m, r = n;
                                spot->setValue(u);
                                qRate->setValue(q);
                                rRate->setValue(r);
                                vol->setValue(v);

                                Real value = option.NPV();
                                calculated["delta"] = option.delta();
                                calculated["gamma"] = option.gamma();
                                calculated["theta"] = option.theta();
                                calculated["rho"] = option.rho();
                                calculated["divRho"] = option.dividendRho();
                                calculated["vega"] = option.vega();

                                if (value > spot->value() * 1.0e-5) {
                                    // perturb spot and get delta and gamma
                                    Real du = u * 1.0e-4;
                                    spot->setValue(u + du);
                                    Real value_p = option.NPV(), delta_p = option.delta();
                                    spot->setValue(u - du);
                                    Real value_m = option.NPV(), delta_m = option.delta();
                                    spot->setValue(u);
                                    expected["delta"] = (value_p - value_m) / (2 * du);
                                    expected["gamma"] = (delta_p - delta_m) / (2 * du);

                                    // perturb rates and get rho and dividend rho
                                    Spread dr = r * 1.0e-4;
                                    rRate->setValue(r + dr);
                                    value_p = option.NPV();
                                    rRate->setValue(r - dr);
                                    value_m = option.NPV();
                                    rRate->setValue(r);
                                    expected["rho"] = (value_p - value_m) / (2 * dr);

                                    Spread dq = q * 1.0e-4;
                                    qRate->setValue(q + dq);
                                    value_p = option.NPV();
                                    qRate->setValue(q - dq);
                                    value_m = option.NPV();
                                    qRate->setValue(q);
                                    expected["divRho"] = (value_p - value_m) / (2 * dq);

                                    // perturb volatility and get vega
                                    Volatility dv = v * 1.0e-4;
                                    vol->setValue(v + dv);
                                    value_p = option.NPV();
                                    vol->setValue(v - dv);
                                    value_m = option.NPV();
                                    vol->setValue(v);
                                    expected["vega"] = (value_p - value_m) / (2 * dv);

                                    // perturb date and get theta
                                    Time dT = dc.yearFraction(today - 1, today + 1);
                                    Settings::instance().evaluationDate() = today - 1;
                                    value_m = option.NPV();
                                    Settings::instance().evaluationDate() = today + 1;
                                    value_p = option.NPV();
                                    Settings::instance().evaluationDate() = today;
                                    expected["theta"] = (value_p - value_m) / dT;

                                    // compare
                                    std::map<std::string, Real>::iterator it;
                                    for (it = calculated.begin(); it != calculated.end(); ++it) {
                                        std::string greek = it->first;
                                        Real expct = expected[greek], calcl = calculated[greek],
                                             tol = tolerance[greek];
                                        Real error = relativeError(expct, calcl, u);
                                        if (error > tol) {
                                            REPORT_FAILURE(greek, Average::Geometric,
                                                           runningAverage, pastFixings,
                                                           std::vector<Date>(), payoff, maturity, u,
                                                           q, r, today, v, expct, calcl, tol);
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

BOOST_AUTO_TEST_CASE(testPastFixings) {

    BOOST_TEST_MESSAGE("Testing use of past fixings in Asian options...");

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<StrikedTypePayoff> payoff(
                                  new PlainVanillaPayoff(Option::Put, 100.0));


    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(today + 1*Years));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
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

    ext::shared_ptr<PricingEngine> engine =
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

    // Test past-fixings-as-a-vector interface

    std::vector<Real> allPastFixings = {spot->value() * 0.8, spot->value() * 0.8};

    DiscreteAveragingAsianOption option1a(Average::Arithmetic, fixingDates1,
                                          payoff, exercise);

    DiscreteAveragingAsianOption option2a(Average::Arithmetic, fixingDates2,
                                          payoff, exercise, allPastFixings);

    option1a.setPricingEngine(engine);
    option2a.setPricingEngine(engine);

    Real price1a = option1a.NPV();
    Real price2a = option2a.NPV();

    if (std::fabs(price1 - price1a) > 1e-8) {
        BOOST_ERROR(
             "Unseasoned option prices do not match in old and new interface"
             << "\n  Old Interface: " << price1
             << "\n  New Interface: " << price1a);
    }

    if (std::fabs(price2 - price2a) > 1e-8) {
        BOOST_ERROR(
             "Seasoned option prices do not match in old and new interface"
             << "\n  Old Interface: " << price2
             << "\n  New Interface: " << price2a);
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

    engine = ext::shared_ptr<PricingEngine>(
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

BOOST_AUTO_TEST_CASE(testPastFixingsModelDependency) {

    BOOST_TEST_MESSAGE(
        "Testing use of past fixings in Asian options where model dependency is flagged...");

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.06));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    ext::shared_ptr<StrikedTypePayoff> call_payoff(new PlainVanillaPayoff(Option::Call, 20.0));
    ext::shared_ptr<StrikedTypePayoff> put_payoff(new PlainVanillaPayoff(Option::Put, 20.0));

    std::vector<Date> fixingDates = {today - 6 * Weeks, today - 2 * Weeks, today + 2 * Weeks,
                                     today + 6 * Weeks};

    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(today + 6 * Weeks));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new BlackScholesMertonProcess(
        Handle<Quote>(spot), Handle<YieldTermStructure>(qTS), Handle<YieldTermStructure>(rTS),
        Handle<BlackVolTermStructure>(volTS)));

    // Test guaranteed exercise (calls) and permanent OTMness (puts), with the average price TW
    // engine

    ext::shared_ptr<PricingEngine> engine = ext::shared_ptr<PricingEngine>(
        new TurnbullWakemanAsianEngine(stochProcess));

    std::vector<Real> allPastFixings = {spot->value(), spot->value()};

    DiscreteAveragingAsianOption call_option(Average::Arithmetic, fixingDates, call_payoff,
                                             exercise, allPastFixings);
    DiscreteAveragingAsianOption put_option(Average::Arithmetic, fixingDates, put_payoff, exercise,
                                            allPastFixings);

    call_option.setPricingEngine(engine);
    put_option.setPricingEngine(engine);

    // The expected call NPV is equal to that of an averaging forward over the same fixing dates,
    // since exercise is guaranteed
    Real expected_call_option_npv =
        rTS->discount(exercise->lastDate()) *
        ((100.0 + 100.0 + 100.0 * qTS->discount(fixingDates[2]) / rTS->discount(fixingDates[2]) +
          100.0 * qTS->discount(fixingDates[3]) / rTS->discount(fixingDates[3])) /
             fixingDates.size() -
         call_payoff->strike());

    BOOST_CHECK_EQUAL(call_option.NPV(), expected_call_option_npv);
    BOOST_CHECK_EQUAL(put_option.NPV(), 0.0);

    // Compare greeks to numerical greeks
    Real dS = 0.001;
    Real callPrice = call_option.NPV();
    Real putPrice = put_option.NPV();
    Real callDelta = call_option.delta();
    Real callGamma = call_option.gamma();
    Real putDelta = put_option.delta();
    Real putGamma = put_option.gamma();

    ext::shared_ptr<SimpleQuote> spotUp(new SimpleQuote(100.0+dS));
    ext::shared_ptr<SimpleQuote> spotDown(new SimpleQuote(100.0-dS));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcessUp(new BlackScholesMertonProcess(
        Handle<Quote>(spotUp), Handle<YieldTermStructure>(qTS), Handle<YieldTermStructure>(rTS),
        Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<BlackScholesMertonProcess> stochProcessDown(new BlackScholesMertonProcess(
        Handle<Quote>(spotDown), Handle<YieldTermStructure>(qTS), Handle<YieldTermStructure>(rTS),
        Handle<BlackVolTermStructure>(volTS)));

    ext::shared_ptr<PricingEngine> engineUp(
        new TurnbullWakemanAsianEngine(stochProcessUp));

    ext::shared_ptr<PricingEngine> engineDown(
        new TurnbullWakemanAsianEngine(stochProcessDown));

    call_option.setPricingEngine(engineUp);
    Real callCalculatedUp = call_option.NPV();
    put_option.setPricingEngine(engineUp);
    Real putCalculatedUp = put_option.NPV();

    call_option.setPricingEngine(engineDown);
    Real callCalculatedDown = call_option.NPV();
    put_option.setPricingEngine(engineDown);
    Real putCalculatedDown = put_option.NPV();

    Real callDeltaBump = (callCalculatedUp - callCalculatedDown) / (2 * dS);
    Real callGammaBump = (callCalculatedUp + callCalculatedDown - 2*callPrice) / (dS * dS);

    Real putDeltaBump = (putCalculatedUp - putCalculatedDown) / (2 * dS);
    Real putGammaBump = (putCalculatedUp + putCalculatedDown - 2*putPrice) / (dS * dS);

    Real tolerance = 1.0e-8;
    if (std::fabs(callDeltaBump - callDelta) > tolerance) {
        BOOST_ERROR(
            "Seasoned analytic call delta did not match numerical delta:"
            << "\n    analytic delta:  " << callDelta << "\n    bump delta:      " << callDeltaBump
            << "\n    error:           " << std::fabs(callDeltaBump - callDelta));
    }
    if (std::fabs(callGammaBump - callGamma) > tolerance) {
        BOOST_ERROR(
            "Seasoned analytic call gamma did not match numerical gamma:"
            << "\n    analytic gamma:  " << callGamma << "\n    bump gamma:      " << callGammaBump
            << "\n    error:           " << std::fabs(callGammaBump - callGamma));
    }
    if (std::fabs(putDeltaBump - putDelta) > tolerance) {
        BOOST_ERROR(
            "Seasoned analytic put delta did not match numerical delta:"
            << "\n    analytic delta:  " << putDelta << "\n    bump delta:      " << putDeltaBump
            << "\n    error:           " << std::fabs(putDeltaBump - putDelta));
    }
    if (std::fabs(putGammaBump - putGamma) > tolerance) {
        BOOST_ERROR(
            "Seasoned analytic put gamma did not match numerical gamma:"
            << "\n    analytic gamma:  " << putGamma << "\n    bump gamma:      " << putGammaBump
            << "\n    error:           " << std::fabs(putGammaBump - putGamma));
    }
}

BOOST_AUTO_TEST_CASE(testAllFixingsInThePast) {

    BOOST_TEST_MESSAGE(
        "Testing Asian options with all fixing dates in the past...");

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.005));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.01));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.20));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(vol, dc);

    ext::shared_ptr<BlackScholesMertonProcess> stochProcess(
        new BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));

    Date exerciseDate = today + 2*Weeks;
    Date startDate = exerciseDate - 1*Years;
    std::vector<Date> fixingDates;
    fixingDates.reserve(12);
    for (Integer i = 0; i < 12; ++i)
        fixingDates.push_back(startDate + i*Months);
    Size pastFixings = 12;

    ext::shared_ptr<StrikedTypePayoff> payoff(
                                  new PlainVanillaPayoff(Option::Put, 100.0));
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    // MC arithmetic average-price

    Real runningSum = pastFixings * spot->value();

    DiscreteAveragingAsianOption option1(Average::Arithmetic, runningSum,
                                         pastFixings, fixingDates,
                                         payoff, exercise);
    option1.setPricingEngine(
        MakeMCDiscreteArithmeticAPEngine<LowDiscrepancy>(stochProcess)
        .withSamples(2047));

    // MC arithmetic average-strike

    DiscreteAveragingAsianOption option2(Average::Arithmetic, runningSum,
                                         pastFixings, fixingDates,
                                         payoff, exercise);
    option2.setPricingEngine(
        MakeMCDiscreteArithmeticASEngine<LowDiscrepancy>(stochProcess)
        .withSamples(2047));

    // MC geometric average-price

    Real runningProduct = std::pow(spot->value(), int(pastFixings));

    DiscreteAveragingAsianOption option3(Average::Geometric, runningProduct,
                                         pastFixings, fixingDates,
                                         payoff, exercise);
    option3.setPricingEngine(
        MakeMCDiscreteGeometricAPEngine<LowDiscrepancy>(stochProcess)
        .withSamples(2047));

    // Check that NPV raises a specific exception instead of crashing.
    // (It used to do that.)

    bool raised = false;
    try {
        option1.NPV();
    } catch (detail::PastFixingsOnly&) {
        raised = true;
    }
    if (!raised) {
        BOOST_FAIL("exception expected");
    }

    raised = false;
    try {
        option1.NPV();
    } catch (detail::PastFixingsOnly&) {
        raised = true;
    }
    if (!raised) {
        BOOST_FAIL("exception expected");
    }

    raised = false;
    try {
        option2.NPV();
    } catch (detail::PastFixingsOnly&) {
        raised = true;
    }
    if (!raised) {
        BOOST_FAIL("exception expected");
    }

    // also check with the evaluation date on last fixing

    Settings::instance().evaluationDate() = fixingDates.back();

    raised = false;
    try {
        option1.NPV();
    } catch (detail::PastFixingsOnly&) {
        raised = true;
    }
    if (!raised) {
        BOOST_FAIL("exception expected");
    }

    raised = false;
    try {
        option1.NPV();
    } catch (detail::PastFixingsOnly&) {
        raised = true;
    }
    if (!raised) {
        BOOST_FAIL("exception expected");
    }

    raised = false;
    try {
        option2.NPV();
    } catch (detail::PastFixingsOnly&) {
        raised = true;
    }
    if (!raised) {
        BOOST_FAIL("exception expected");
    }
}

BOOST_AUTO_TEST_CASE(testTurnbullWakemanAsianEngine) {

    BOOST_TEST_MESSAGE("Testing Turnbull-Wakeman engine for discrete-time arithmetic average-rate "
                       "Asians options with term structure support...");

    // Data from Haug, "Option Pricing Formulas", Table 4-28, p.201
    // Type, underlying, strike, b, rfRate, t1, expiry, fixings, base vol, slope, expected result
    DiscreteAverageDataTermStructure cases[] = {
        {Option::Call, 100, 80, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "flat", 19.5152},
        {Option::Call, 100, 80, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "up", 19.5063},
        {Option::Call, 100, 80, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "down", 19.5885},
        {Option::Put, 100, 80, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "flat", 0.0090},
        {Option::Put, 100, 80, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "up", 0.0001},
        {Option::Put, 100, 80, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "down", 0.0823},

        {Option::Call, 100, 90, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "flat", 10.1437},
        {Option::Call, 100, 90, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "up", 9.8313},
        {Option::Call, 100, 90, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "down", 10.7062},
        {Option::Put, 100, 90, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "flat", 0.3906},
        {Option::Put, 100, 90, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "up", 0.0782},
        {Option::Put, 100, 90, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "down", 0.9531},

        {Option::Call, 100, 100, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "flat", 3.2700},
        {Option::Call, 100, 100, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "up", 2.2819},
        {Option::Call, 100, 100, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "down", 4.3370},
        {Option::Put, 100, 100, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "flat", 3.2700},
        {Option::Put, 100, 100, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "up", 2.2819},
        {Option::Put, 100, 100, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "down", 4.3370},

        {Option::Call, 100, 110, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "flat", 0.5515},
        {Option::Call, 100, 110, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "up", 0.1314},
        {Option::Call, 100, 110, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "down", 1.2429},
        {Option::Put, 100, 110, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "flat", 10.3046},
        {Option::Put, 100, 110, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "up", 9.8845},
        {Option::Put, 100, 110, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "down", 10.9960},

        {Option::Call, 100, 120, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "flat", 0.0479},
        {Option::Call, 100, 120, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "up", 0.0016},
        {Option::Call, 100, 120, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "down", 0.2547},
        {Option::Put, 100, 120, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "flat", 19.5541},
        {Option::Put, 100, 120, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "up", 19.5078},
        {Option::Put, 100, 120, 0, 0.05, 1.0 / 52, 0.5, 26, 0.2, "down", 19.7609}};

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    for (auto& l : cases) {
        Time dt = (l.expiry - l.first) / (l.fixings - 1);
        std::vector<Date> fixingDates(l.fixings);
        fixingDates[0] = today + timeToDays(l.first, 360);

        for (Size i = 1; i < l.fixings; i++) {
            fixingDates[i] = today + timeToDays(i * dt + l.first, 360);
        }

        // Set up market data
        ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(l.underlying));
        ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, l.b + l.riskFreeRate, dc);
        ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, l.riskFreeRate, dc);
        ext::shared_ptr<BlackVolTermStructure> volTS;
        Volatility volSlope = 0.005;
        if (l.slope == "flat") {
            volTS = flatVol(today, l.volatility, dc);
        } else if (l.slope == "up") {
            std::vector<Volatility> volatilities(l.fixings);
            for (Size i = 0; i < l.fixings; ++i) {
                // Loop to fill a vector of vols from 7.5 % to 20 %
                volatilities[i] = l.volatility - (l.fixings - 1) * volSlope + i * volSlope;
            }
            volTS =
                ext::make_shared<BlackVarianceCurve>(today, fixingDates, volatilities, dc, true);
        } else if (l.slope == "down") {
            std::vector<Volatility> volatilities(l.fixings);
            for (Size i = 0; i < l.fixings; ++i) {
                // Loop to fill a vector of vols from 32.5 % to 20 %
                volatilities[i] = l.volatility + (l.fixings - 1) * volSlope - i * volSlope;
            }
            volTS =
                ext::make_shared<BlackVarianceCurve>(today, fixingDates, volatilities, dc, false);
        } else {
            QL_FAIL("unexpected slope type in engine test case");
        }

        Average::Type averageType = Average::Arithmetic;

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(l.type, l.strike));

        Date maturity = today + timeToDays(l.expiry, 360);

        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(maturity));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new BlackScholesMertonProcess(
            Handle<Quote>(spot), Handle<YieldTermStructure>(qTS), Handle<YieldTermStructure>(rTS),
            Handle<BlackVolTermStructure>(volTS)));

        // Construct engine
        ext::shared_ptr<PricingEngine> engine(
            new TurnbullWakemanAsianEngine(stochProcess));

        DiscreteAveragingAsianOption option(averageType, 0, 0, fixingDates, payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = l.result;
        Real tolerance = 2.5e-3;
        Real error = std::fabs(expected - calculated);
        if (error > tolerance) {
            BOOST_ERROR(
                "Failed to reproduce expected NPV:"
                << "\n    type:            " << l.type << "\n    spot:            " << l.underlying
                << "\n    strike:          " << l.strike << "\n    dividend yield:  "
                << l.b + l.riskFreeRate << "\n    risk-free rate:  " << l.riskFreeRate
                << "\n    volatility:      " << l.volatility << "\n    slope:           " << l.slope
                << "\n    reference date:  " << today << "\n    expiry:          " << l.expiry
                << "\n    expected value:  " << expected << "\n    calculated:      " << calculated
                << "\n    error:           " << error);
        }

        // Compare greeks to numerical greeks
        Real dS = 0.001;
        Real delta = option.delta();
        Real gamma = option.gamma();

        ext::shared_ptr<SimpleQuote> spotUp(new SimpleQuote(l.underlying+dS));
        ext::shared_ptr<SimpleQuote> spotDown(new SimpleQuote(l.underlying-dS));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcessUp(new BlackScholesMertonProcess(
            Handle<Quote>(spotUp), Handle<YieldTermStructure>(qTS), Handle<YieldTermStructure>(rTS),
            Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcessDown(new BlackScholesMertonProcess(
            Handle<Quote>(spotDown), Handle<YieldTermStructure>(qTS), Handle<YieldTermStructure>(rTS),
            Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engineUp(
            new TurnbullWakemanAsianEngine(stochProcessUp));

        ext::shared_ptr<PricingEngine> engineDown(
            new TurnbullWakemanAsianEngine(stochProcessDown));

        option.setPricingEngine(engineUp);
        Real calculatedUp = option.NPV();

        option.setPricingEngine(engineDown);
        Real calculatedDown = option.NPV();

        Real deltaBump = (calculatedUp - calculatedDown) / (2 * dS);
        Real gammaBump = (calculatedUp + calculatedDown - 2*calculated) / (dS * dS);

        tolerance = 1.0e-6;
        Real deltaError = std::fabs(deltaBump - delta);
        if (deltaError > tolerance) {
            BOOST_ERROR(
                "Analytical delta failed to match bump delta:"
                << "\n    type:            " << l.type << "\n    spot:            " << l.underlying
                << "\n    strike:          " << l.strike << "\n    dividend yield:  "
                << l.b + l.riskFreeRate << "\n    risk-free rate:  " << l.riskFreeRate
                << "\n    volatility:      " << l.volatility << "\n    slope:           " << l.slope
                << "\n    reference date:  " << today << "\n    expiry:          " << l.expiry
                << "\n    analytic delta:  " << delta << "\n    bump delta:      " << deltaBump
                << "\n    error:           " << deltaError);
        }

        Real gammaError = std::fabs(gammaBump - gamma);
        if (gammaError > tolerance) {
            BOOST_ERROR(
                "Analytical gamma failed to match bump gamma:"
                << "\n    type:            " << l.type << "\n    spot:            " << l.underlying
                << "\n    strike:          " << l.strike << "\n    dividend yield:  "
                << l.b + l.riskFreeRate << "\n    risk-free rate:  " << l.riskFreeRate
                << "\n    volatility:      " << l.volatility << "\n    slope:           " << l.slope
                << "\n    reference date:  " << today << "\n    expiry:          " << l.expiry
                << "\n    analytic gamma:  " << gamma << "\n    bump gamma:      " << gammaBump
                << "\n    error:           " << gammaError);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(AsianOptionExperimentalTest)

BOOST_AUTO_TEST_CASE(testLevyEngine) {

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
    Date today = Settings::instance().evaluationDate();

    for (auto& l : cases) {

        ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(l.spot));
        ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, l.dividendYield, dc);
        ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, l.riskFreeRate, dc);
        ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, l.volatility, dc);

        Average::Type averageType = Average::Arithmetic;
        ext::shared_ptr<Quote> average(new SimpleQuote(l.currentAverage));

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(l.type, l.strike));

        Date startDate = today - l.elapsed;
        Date maturity = startDate + l.length;

        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(maturity));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
                                                                BlackScholesMertonProcess(Handle<Quote>(spot),
                                                                                          Handle<YieldTermStructure>(qTS),
                                                                                          Handle<YieldTermStructure>(rTS),
                                                                                          Handle<BlackVolTermStructure>(volTS)));

        ext::shared_ptr<PricingEngine> engine(
            new ContinuousArithmeticAsianLevyEngine(
                stochProcess, Handle<Quote>(average), startDate));

        ContinuousAveragingAsianOption option(averageType,
                                              payoff, exercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();
        Real expected = l.result;
        Real tolerance = 1.0e-4;
        Real error = std::fabs(expected-calculated);
        if (error > tolerance) {
            BOOST_ERROR(
                "Asian option with Levy engine:"
                << "\n    spot:            " << l.spot << "\n    current average: "
                << l.currentAverage << "\n    strike:          " << l.strike
                << "\n    dividend yield:  " << l.dividendYield << "\n    risk-free rate:  "
                << l.riskFreeRate << "\n    volatility:      " << l.volatility
                << "\n    reference date:  " << today << "\n    length:          " << l.length
                << "\n    elapsed:         " << l.elapsed << "\n    expected value:  " << expected
                << "\n    calculated:      " << calculated << "\n    error:           " << error);
        }
    }
}

BOOST_AUTO_TEST_CASE(testVecerEngine) {
    BOOST_TEST_MESSAGE("Testing Vecer engine for Asian options...");

    VecerData cases[] = {
        { 1.9, 0.05,   0.5,  2.0, 1, 0.193174, 1.0e-5 },
        { 2.0, 0.05,   0.5,  2.0, 1, 0.246416, 1.0e-5 },
        { 2.1, 0.05,   0.5,  2.0, 1, 0.306220, 1.0e-4 },
        { 2.0, 0.02,   0.1,  2.0, 1, 0.055986, 2.0e-4 },
        { 2.0, 0.18,   0.3,  2.0, 1, 0.218388, 1.0e-4 },
        { 2.0, 0.0125, 0.25, 2.0, 2, 0.172269, 1.0e-4 },
        { 2.0, 0.05,   0.5,  2.0, 2, 0.350095, 2.0e-4 }
    };

    Date today = Settings::instance().evaluationDate();
    DayCounter dayCounter = Actual360();

    Option::Type type = Option::Call;
    Handle<YieldTermStructure> q(flatRate(today, 0.0, dayCounter));

    Size timeSteps = 200;
    Size assetSteps = 200;

    for (auto& i : cases) {
        Handle<Quote> u(ext::make_shared<SimpleQuote>(i.spot));
        Handle<YieldTermStructure> r(flatRate(today, i.riskFreeRate, dayCounter));
        Handle<BlackVolTermStructure> sigma(flatVol(today, i.volatility, dayCounter));
        ext::shared_ptr<BlackScholesMertonProcess> process =
            ext::make_shared<BlackScholesMertonProcess>(u, q, r, sigma);

        Date maturity = today + i.length * 360;
        ext::shared_ptr<Exercise> exercise =
            ext::make_shared<EuropeanExercise>(maturity);
        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::make_shared<PlainVanillaPayoff>(type, i.strike);
        Handle<Quote> average(ext::make_shared<SimpleQuote>(0.0));

        ContinuousAveragingAsianOption option(Average::Arithmetic,
                                              payoff, exercise);
        option.setPricingEngine(
            ext::make_shared<ContinuousArithmeticAsianVecerEngine>(
                process,average,today,timeSteps,assetSteps,-1.0,1.0));

        Real calculated = option.NPV();
        Real error = std::fabs(calculated - i.result);
        if (error > i.tolerance)
            BOOST_ERROR("Failed to reproduce expected NPV"
                        << "\n    calculated: " << calculated << "\n    expected:   " << i.result
                        << "\n    expected:   " << i.result << "\n    error:      " << error
                        << "\n    tolerance:  " << i.tolerance);
    }
}

BOOST_AUTO_TEST_CASE(testAnalyticContinuousGeometricAveragePriceHeston) {

    BOOST_TEST_MESSAGE("Testing analytic continuous geometric Asians under Heston...");

    // data from "Pricing of Geometric Asian Options under Heston's Stochastic
    // Volatility Model", Kim & Wee, Quantitative Finance, 14:10, 1795-1809, 2011

    // 73, 348 and 1095 are 0.2, 1.5 and 3.0 years respectively in Actual365Fixed
    Time days[] =    {73, 73, 73, 73, 73, 548, 548, 548, 548, 548, 1095, 1095, 1095, 1095, 1095};
    Real strikes[] = {90.0, 95.0, 100.0, 105.0, 110.0, 90.0, 95.0, 100.0, 105.0, 110.0, 90.0, 95.0,
                      100.0, 105.0, 110.0};

    // Prices from Table 1 (params obey Feller condition)
    Real prices[] =  {10.6571, 6.5871, 3.4478, 1.4552, 0.4724, 16.5030, 13.7625, 11.3374, 9.2245,
                     7.4122, 20.5102, 18.3060, 16.2895, 14.4531, 12.7882};

    // Prices from Table 4 (params do not obey Feller condition)
    Real prices_2[] =  {10.6425, 6.4362, 3.1578, 1.1936, 0.3609, 14.9955, 11.6707, 8.7767, 6.3818,
                       4.5118, 18.1219, 15.2009, 12.5707, 10.2539, 8.2611};

    // 0.2 and 3.0 match to 1e-4. Unfortunatly 1.5 corresponds to 547.5 days, 547 and 548
    // bound the expected answer but are both out by ~5e-3
    Real tolerance = 1.0e-2;

    DayCounter dc = Actual365Fixed();
    Date today = Settings::instance().evaluationDate();
    Option::Type type(Option::Call);
    Average::Type averageType = Average::Geometric;

    Handle<Quote> spot(ext::shared_ptr<Quote>(new SimpleQuote(100)));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    Real v0 = 0.09;
    Real kappa = 1.15;
    Real theta = 0.348;
    Real sigma = 0.39;
    Real rho = -0.64;

    ext::shared_ptr<HestonProcess> hestonProcess(new
                                                 HestonProcess(Handle<YieldTermStructure>(rTS), Handle<YieldTermStructure>(qTS),
                                                               spot, v0, kappa, theta, sigma, rho));

    ext::shared_ptr<AnalyticContinuousGeometricAveragePriceAsianHestonEngine> engine(new
                                                                                     AnalyticContinuousGeometricAveragePriceAsianHestonEngine(hestonProcess));

    for (Size i=0; i<LENGTH(strikes); i++) {
        Real strike = strikes[i];
        Time day = days[i];
        Real expected = prices[i];

        Date expiryDate = today + day*Days;

        ext::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(expiryDate));
        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

        ContinuousAveragingAsianOption option(averageType, payoff, europeanExercise);
        option.setPricingEngine(engine);

        Real calculated = option.NPV();

        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value", averageType, 1.0, 0.0,
                           std::vector<Date>(), payoff, europeanExercise, spot->value(),
                           qRate->value(), rRate->value(), today,
                           std::sqrt(v0), expected, calculated, tolerance);
        }
    }

    Real v0_2 = 0.09;
    Real kappa_2 = 2.0;
    Real theta_2 = 0.09;
    Real sigma_2 = 1.0;
    Real rho_2 = -0.3;

    ext::shared_ptr<HestonProcess> hestonProcess_2(new
                                                   HestonProcess(Handle<YieldTermStructure>(rTS), Handle<YieldTermStructure>(qTS),
                                                                 spot, v0_2, kappa_2, theta_2, sigma_2, rho_2));

    ext::shared_ptr<AnalyticContinuousGeometricAveragePriceAsianHestonEngine> engine_2(new
                                                                                       AnalyticContinuousGeometricAveragePriceAsianHestonEngine(hestonProcess_2));

    for (Size i=0; i<LENGTH(strikes); i++) {
        Real strike = strikes[i];
        Time day = days[i];
        Real expected = prices_2[i];

        Date expiryDate = today + day*Days;

        ext::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(expiryDate));
        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

        ContinuousAveragingAsianOption option(averageType, payoff, europeanExercise);
        option.setPricingEngine(engine_2);

        Real calculated = option.NPV();

        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value", averageType, 1.0, 0.0,
                           std::vector<Date>(), payoff, europeanExercise, spot->value(),
                           qRate->value(), rRate->value(), today,
                           std::sqrt(v0), expected, calculated, tolerance);
        }
    }

    // Also test the continuous data from the authors' subsequent paper

    // data from "A Recursive Method for Discretely Monitored Geometric Asian Option
    // Prices", Kim, Kim, Kim & Wee, Bull. Korean Math. Soc. 53, 733-749, 2016

    // 73, 348 and 1095 are 0.2, 1.5 and 3.0 years respectively in Actual365Fixed
    Time days_3[] =    {30, 91, 182, 365, 730, 1095, 30, 91, 182, 365, 730, 1095, 30,
                     91, 182, 365, 730, 1095};
    Real strikes_3[] = {90, 90, 90, 90, 90, 90, 100, 100, 100, 100, 100, 100, 110,
                        110, 110, 110, 110, 110};

    // 30-day options need wider tolerance due to the day-bracket issue discussed above
    Real tol_3[] =     {2.0e-2, 1.0e-2, 1.0e-2, 1.0e-2, 1.0e-2, 1.0e-2, 2.0e-2, 1.0e-2,
                    1.0e-2, 1.0e-2, 1.0e-2, 1.0e-2, 2.0e-2, 1.0e-2, 1.0e-2, 1.0e-2,
                    1.0e-2, 1.0e-2};

    // Prices from Tables 1, 2 and 3
    Real prices_3[] =  {10.1513, 10.8175, 11.8664, 13.5931, 16.0988, 17.9475, 2.0472,
                       3.5735, 5.0588, 7.1132, 9.9139, 11.9959, 0.0350, 0.4869,
                       1.3376, 2.8569, 5.2804, 7.2682};

    // Note that although these parameters look similar to the first set above, theta
    // is a factor of 10 smaller. I guess there is a mis-transcription somewhere!
    Real v0_3 = 0.09;
    Real kappa_3 = 1.15;
    Real theta_3 = 0.0348;
    Real sigma_3 = 0.39;
    Real rho_3 = -0.64;

    ext::shared_ptr<HestonProcess> hestonProcess_3(new
                                                   HestonProcess(Handle<YieldTermStructure>(rTS), Handle<YieldTermStructure>(qTS),
                                                                 spot, v0_3, kappa_3, theta_3, sigma_3, rho_3));

    ext::shared_ptr<AnalyticContinuousGeometricAveragePriceAsianHestonEngine> engine_3(new
                                                                                       AnalyticContinuousGeometricAveragePriceAsianHestonEngine(hestonProcess_3));

    for (Size i=0; i<LENGTH(strikes_3); i++) {
        Real strike = strikes_3[i];
        Time day = days_3[i];
        Real expected = prices_3[i];
        Real tolerance = tol_3[i];

        Date expiryDate = today + day*Days;

        ext::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(expiryDate));
        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));

        ContinuousAveragingAsianOption option(averageType, payoff, europeanExercise);
        option.setPricingEngine(engine_3);

        Real calculated = option.NPV();

        if (std::fabs(calculated-expected) > tolerance) {
            REPORT_FAILURE("value", averageType, 1.0, 0.0,
                           std::vector<Date>(), payoff, europeanExercise, spot->value(),
                           qRate->value(), rRate->value(), today,
                           std::sqrt(v0), expected, calculated, tolerance);
        }
    }

}

BOOST_AUTO_TEST_CASE(testAnalyticDiscreteGeometricAveragePriceHeston) {

    BOOST_TEST_MESSAGE("Testing analytic discrete geometric average-price Asians under Heston...");

    // 30-day options need wider tolerance due to uncertainty around what "weekly
    // fixing" dates mean over a 30-day month!
    Real tol[] =     {3.0e-2, 2.0e-2, 2.0e-2, 2.0e-2, 3.0e-2, 4.0e-2, 8.0e-2, 1.0e-2,
                  2.0e-2, 3.0e-2, 3.0e-2, 4.0e-2, 2.0e-2, 1.0e-2, 1.0e-2, 2.0e-2,
                  3.0e-2, 4.0e-2};

    DayCounter dc = Actual365Fixed();
    Date today = Settings::instance().evaluationDate();

    Handle<Quote> spot(ext::shared_ptr<Quote>(new SimpleQuote(100)));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    Real v0 = 0.09;
    Real kappa = 1.15;
    Real theta = 0.0348;
    Real sigma = 0.39;
    Real rho = -0.64;

    ext::shared_ptr<HestonProcess> hestonProcess(new
                                                 HestonProcess(Handle<YieldTermStructure>(rTS), Handle<YieldTermStructure>(qTS),
                                                               spot, v0, kappa, theta, sigma, rho));

    ext::shared_ptr<AnalyticDiscreteGeometricAveragePriceAsianHestonEngine> engine(new
                                                                                   AnalyticDiscreteGeometricAveragePriceAsianHestonEngine(hestonProcess));

    AsianOptionTest::testDiscreteGeometricAveragePriceHeston(engine, tol);
}

BOOST_AUTO_TEST_CASE(testDiscreteGeometricAveragePriceHestonPastFixings) {

    BOOST_TEST_MESSAGE("Testing Analytic vs MC for seasoned discrete geometric Asians under Heston...");

    // 30-day options need wider tolerance due to uncertainty around what "weekly
    // fixing" dates mean over a 30-day month!

    int days[] =           {30, 90, 180, 360, 720};
    Real strikes[] =       {90, 100, 110};

    Real tol[3][5][2] = {{{
                              0.04, // strike=90, days=30, k=0
                              0.04, // strike=90, days=30, k=1
                          },
                          {
                              0.04, // strike=90, days=90, k=0
                              0.04, // strike=90, days=90, k=1
                          },
                          {
                              0.04, // strike=90, days=180, k=0
                              0.04, // strike=90, days=180, k=1
                          },
                          {
                              0.05, // strike=90, days=360, k=0
                              0.04, // strike=90, days=360, k=1
                          },
                          {
                              0.04, // strike=90, days=720, k=0
                              0.04, // strike=90, days=720, k=1
                          }},

                         {{
                              0.04, // strike=100, days=30, k=0
                              0.04, // strike=100, days=30, k=1
                          },
                          {
                              0.04, // strike=100, days=90, k=0
                              0.04, // strike=100, days=90, k=1
                          },
                          {
                              0.04, // strike=100, days=180, k=0
                              0.04, // strike=100, days=180, k=1
                          },
                          {
                              0.06, // strike=100, days=360, k=0
                              0.06, // strike=100, days=360, k=1
                          },
                          {
                              0.06, // strike=100, days=720, k=0
                              0.05, // strike=100, days=720, k=1
                          }},

                         {{
                              0.04, // strike=110, days=30, k=0
                              0.04, // strike=110, days=30, k=1
                          },
                          {
                              0.04, // strike=110, days=90, k=0
                              0.04, // strike=110, days=90, k=1
                          },
                          {
                              0.04, // strike=110, days=180, k=0
                              0.04, // strike=110, days=180, k=1
                          },
                          {
                              0.05, // strike=110, days=360, k=0
                              0.04, // strike=110, days=360, k=1
                          },
                          {
                              0.06, // strike=110, days=720, k=0
                              0.05, // strike=110, days=720, k=1
                          }}};

    DayCounter dc = Actual365Fixed();
    Date today = Settings::instance().evaluationDate();

    Handle<Quote> spot(ext::shared_ptr<Quote>(new SimpleQuote(100)));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    Real v0 = 0.09;
    Real kappa = 1.15;
    Real theta = 0.0348;
    Real sigma = 0.39;
    Real rho = -0.64;

    ext::shared_ptr<HestonProcess> hestonProcess(new
                                                 HestonProcess(Handle<YieldTermStructure>(rTS), Handle<YieldTermStructure>(qTS),
                                                               spot, v0, kappa, theta, sigma, rho));

    ext::shared_ptr<AnalyticDiscreteGeometricAveragePriceAsianHestonEngine> analyticEngine(new
                                                                                           AnalyticDiscreteGeometricAveragePriceAsianHestonEngine(hestonProcess));

    ext::shared_ptr<PricingEngine> mcEngine =
        MakeMCDiscreteGeometricAPHestonEngine<LowDiscrepancy>(hestonProcess)
            .withSamples(8191)
            .withSeed(43);

    Option::Type type(Option::Call);
    Average::Type averageType = Average::Geometric;

    for (Size strike_index = 0; strike_index < LENGTH(strikes); strike_index++) {

        for (Size day_index = 0; day_index < LENGTH(days); day_index++) {

            for (Size k=0; k<2; k++) {

                Size futureFixings = int(std::floor(days[day_index] / 30.0));
                std::vector<Date> fixingDates(futureFixings);
                Date expiryDate = today + days[day_index] * Days;

                for (int i=futureFixings-1; i>=0; i--) {
                    fixingDates[i] = expiryDate - i * 30;
                }

                ext::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(expiryDate));
                ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strikes[strike_index]));

                Real runningAccumulator = 1.0;
                Size pastFixingsCount = 0;
                if (k == 0) {
                    runningAccumulator = 100.0;
                    pastFixingsCount = 1;
                } else {
                    runningAccumulator = 95.0 * 100.0 * 105.0;
                    pastFixingsCount = 3;
                }

                DiscreteAveragingAsianOption option(averageType, runningAccumulator, pastFixingsCount,
                                                    fixingDates, payoff, europeanExercise);

                option.setPricingEngine(analyticEngine);
                Real analyticPrice = option.NPV();

                option.setPricingEngine(mcEngine);
                Real mcPrice = option.NPV();

                auto tolerance = tol[strike_index][day_index][k];

                if (std::fabs(analyticPrice-mcPrice) > tolerance) {
                    REPORT_FAILURE("value", averageType, runningAccumulator, pastFixingsCount,
                                   std::vector<Date>(), payoff, europeanExercise, spot->value(),
                                   qRate->value(), rRate->value(), today,
                                   std::sqrt(v0), analyticPrice, mcPrice, tolerance);
                }
            }
        }
    }
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()