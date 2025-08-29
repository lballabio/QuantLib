/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/margrabeoption.hpp>
#include <ql/pricingengines/exotic/analyticamericanmargrabeengine.hpp>
#include <ql/pricingengines/exotic/analyticeuropeanmargrabeengine.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(MargrabeOptionTests)

#undef REPORT_FAILURE
#define REPORT_FAILURE(greekName, exercise, \
                       s1, s2, Q1, Q2, q1, q2, r, today, v1, v2, rho,   \
                       expected, calculated, error, tolerance)          \
    BOOST_ERROR( \
        exerciseTypeToString(exercise) << " " \
        << "Call option on Exchange Asset s2 for Asset s1" \
        << " with null payoff:\n" \
        << "1st underlying value: " << s1 << "\n" \
        << "2nd underlying value: " << s2 << "\n" \
        << "1st underlying quantity: " << Q1 << "\n" \
        << "2nd underlying quantity: " << Q2 << "\n" \
        << "  1st dividend yield: " << io::rate(q1) << "\n" \
        << "  2nd dividend yield: " << io::rate(q2) << "\n" \
        << "      risk-free rate: " << io::rate(r) << "\n" \
        << "      reference date: " << today << "\n" \
        << "            maturity: " << exercise->lastDate() << "\n" \
        << "1st asset volatility: " << io::volatility(v1) << "\n" \
        << "2nd asset volatility: " << io::volatility(v2) << "\n" \
        << "         correlation: " << rho << "\n\n" \
        << "    expected   " << greekName << ": " << expected << "\n" \
        << "    calculated " << greekName << ": " << calculated << "\n"\
        << "    error:            " << error << "\n" \
        << "    tolerance:        " << tolerance);

#undef REPORT_FAILURE2
#define REPORT_FAILURE2(greekName, exercise, s1, s2, q1, q2, r, today, \
                       v1, v2, expected, calculated, error, tolerance) \
    BOOST_ERROR(exerciseTypeToString(exercise) << " " \
        << "    European option with " \
        << "    null pay off      " << "\n" \
        << "    spot1 value:      " << s1 << "\n" \
        << "    spot2 value:      " << s2 << "\n" \
        << "    strike: 0         " << "\n" \
        << "    dividend yield 1: " << io::rate(q1) << "\n" \
        << "    dividend yield 2: " << io::rate(q2) << "\n" \
        << "    risk-free rate:   " << io::rate(r) << "\n" \
        << "    reference date:   " << today << "\n" \
        << "    maturity:         " << exercise->lastDate() << "\n" \
        << "    volatility 1:     " << io::volatility(v1) << "\n\n" \
        << "    volatility 2:     " << io::volatility(v2) << "\n\n" \
        << "    expected " << greekName << ":   " << expected << "\n" \
        << "    calculated " << greekName << ": " << calculated << "\n"\
        << "    error:            " << error << "\n" \
        << "    tolerance:        " << tolerance);

struct MargrabeOptionTwoData {
    Real s1;
    Real s2;
    Integer Q1;
    Integer Q2;
    Rate q1;
    Rate q2;
    Rate r;
    Time t; // years
    Volatility v1;
    Volatility v2;
    Real rho;
    Real result;
    Real delta1;
    Real delta2;
    Real gamma1;
    Real gamma2;
    Real theta;
    Real rho_greek;
    Real tol;
};

struct MargrabeAmericanOptionTwoData {
    Real s1;
    Real s2;
    Integer Q1;
    Integer Q2;
    Rate q1;
    Rate q2;
    Rate r;
    Time t; // years
    Volatility v1;
    Volatility v2;
    Real rho;
    Real result;
    Real tol;
};


BOOST_AUTO_TEST_CASE(testEuroExchangeTwoAssets) {

    BOOST_TEST_MESSAGE("Testing European one-asset-for-another option...");

    /*
        Exchange-One-Asset-for-Another European Options
    */
    MargrabeOptionTwoData values[] = {
        //Simplification : we assume that the option always exchanges S2 for S1
        //s1,  s2,  Q1,  Q2,  q1,  q2,  r,  t,  v1,  v2,  rho,  result,
                   //delta1,  delta2,  gamma1,  gamma2,  theta, rho, tol
        // data from "given article p.52"
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.15, -0.50, 2.125, 0.841, -0.818, 0.112, 0.135, -2.043, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.20, -0.50, 2.199, 0.813, -0.784, 0.109, 0.132, -2.723, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.25, -0.50, 2.283, 0.788, -0.753, 0.105, 0.126, -3.419, 0.0, 1.0e-3},

        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.15, 0.00, 2.045, 0.883, -0.870, 0.108, 0.131, -1.168, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.20, 0.00, 2.091, 0.857, -0.838, 0.112, 0.135, -1.698, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.25, 0.00, 2.152, 0.830, -0.805, 0.111, 0.134, -2.302, 0.0, 1.0e-3},

        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.15, 0.50, 1.974, 0.946, -0.942, 0.079, 0.096, -0.126, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.20, 0.50, 1.989, 0.929, -0.922, 0.092, 0.111, -0.398, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.25, 0.50, 2.019, 0.902, -0.891, 0.104, 0.125, -0.838, 0.0, 1.0e-3},

        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.15, -0.50, 2.762, 0.672, -0.602, 0.072, 0.087, -1.207, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.20, -0.50, 2.989, 0.661, -0.578, 0.064, 0.078, -1.457, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.25, -0.50, 3.228, 0.653, -0.557, 0.058, 0.070, -1.712, 0.0, 1.0e-3},

        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.15, 0.00, 2.479, 0.695, -0.640, 0.085, 0.102, -0.874, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.20, 0.00, 2.650, 0.680, -0.616, 0.077, 0.093, -1.078, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.25, 0.00, 2.847, 0.668, -0.592, 0.069, 0.083, -1.302, 0.0, 1.0e-3},

        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.15, 0.50, 2.138, 0.746, -0.713, 0.106, 0.128, -0.416, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.20, 0.50, 2.231, 0.728, -0.689, 0.099, 0.120, -0.550, 0.0, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.25, 0.50, 2.374, 0.707, -0.659, 0.090, 0.109, -0.741, 0.0, 1.0e-3},

        //Quantity tests from Excel calcuations
        {22.0, 10.0, 1, 2, 0.06, 0.04, 0.10, 0.50, 0.20, 0.15, 0.50, 2.138, 0.746, -1.426, 0.106, 0.255, -0.987, 0.0, 1.0e-3},
        {11.0, 20.0, 2, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.20, 0.50, 2.231, 1.455, -0.689, 0.198, 0.120, 0.410, 0.0, 1.0e-3},
        {11.0, 10.0, 2, 2, 0.06, 0.04, 0.10, 0.50, 0.20, 0.25, 0.50, 2.374, 1.413, -1.317, 0.181, 0.219, -0.336, 0.0, 1.0e-3}
    };

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    ext::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> spot2(new SimpleQuote(0.0));

    ext::shared_ptr<SimpleQuote> qRate1(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS1 = flatRate(today, qRate1, dc);
    ext::shared_ptr<SimpleQuote> qRate2(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS2 = flatRate(today, qRate2, dc);

    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    ext::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);
    ext::shared_ptr<SimpleQuote> vol2(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS2 = flatVol(today, vol2, dc);

    for (auto& value : values) {

        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot1->setValue(value.s1);
        spot2->setValue(value.s2);
        qRate1->setValue(value.q1);
        qRate2->setValue(value.q2);
        rRate->setValue(value.r);
        vol1->setValue(value.v1);
        vol2->setValue(value.v2);

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess1(new
            BlackScholesMertonProcess(Handle<Quote>(spot1),
                                      Handle<YieldTermStructure>(qTS1),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS1)));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess2(new
            BlackScholesMertonProcess(Handle<Quote>(spot2),
                                      Handle<YieldTermStructure>(qTS2),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS2)));

        std::vector<ext::shared_ptr<StochasticProcess1D> > procs = {stochProcess1, stochProcess2};

        Matrix correlationMatrix(2, 2, value.rho);
        for (Integer j=0; j < 2; j++) {
            correlationMatrix[j][j] = 1.0;
        }

        ext::shared_ptr<PricingEngine> engine(
            new AnalyticEuropeanMargrabeEngine(stochProcess1, stochProcess2, value.rho));

        MargrabeOption margrabeOption(value.Q1, value.Q2, exercise);

        // analytic engine
        margrabeOption.setPricingEngine(engine);

        Real calculated = margrabeOption.NPV();
        Real expected = value.result;
        Real error = std::fabs(calculated-expected);
        Real tolerance = value.tol;
        if (error > tolerance) {
            REPORT_FAILURE("value", exercise, value.s1, value.s2, value.Q1, value.Q2, value.q1,
                           value.q2, value.r, today, value.v1, value.v2, value.rho, expected,
                           calculated, error, tolerance);
        }

        calculated = margrabeOption.delta1();
        expected = value.delta1;
        error= std::fabs(calculated-expected);
        if (error>tolerance) {
            REPORT_FAILURE("delta1", exercise, value.s1, value.s2, value.Q1, value.Q2, value.q1,
                           value.q2, value.r, today, value.v1, value.v2, value.rho, expected,
                           calculated, error, tolerance);
        }

        calculated = margrabeOption.delta2();
        expected = value.delta2;
        error= std::fabs(calculated-expected);
        if (error>tolerance) {
            REPORT_FAILURE("delta2", exercise, value.s1, value.s2, value.Q1, value.Q2, value.q1,
                           value.q2, value.r, today, value.v1, value.v2, value.rho, expected,
                           calculated, error, tolerance);
        }

        calculated = margrabeOption.gamma1();
        expected = value.gamma1;
        error= std::fabs(calculated-expected);
        if (error>tolerance) {
            REPORT_FAILURE("gamma1", exercise, value.s1, value.s2, value.Q1, value.Q2, value.q1,
                           value.q2, value.r, today, value.v1, value.v2, value.rho, expected,
                           calculated, error, tolerance);
        }

        calculated = margrabeOption.gamma2();
        expected = value.gamma2;
        error= std::fabs(calculated-expected);
        if (error>tolerance) {
            REPORT_FAILURE("gamma2", exercise, value.s1, value.s2, value.Q1, value.Q2, value.q1,
                           value.q2, value.r, today, value.v1, value.v2, value.rho, expected,
                           calculated, error, tolerance);
        }

        calculated = margrabeOption.theta();
        expected = value.theta;
        error= std::fabs(calculated-expected);
        if (error>tolerance) {
            REPORT_FAILURE("theta", exercise, value.s1, value.s2, value.Q1, value.Q2, value.q1,
                           value.q2, value.r, today, value.v1, value.v2, value.rho, expected,
                           calculated, error, tolerance);
        }

        calculated = margrabeOption.rho();
        expected = value.rho_greek;
        error= std::fabs(calculated-expected);
        if (error>tolerance) {
            REPORT_FAILURE("rho_greek", exercise, value.s1, value.s2, value.Q1, value.Q2, value.q1,
                           value.q2, value.r, today, value.v1, value.v2, value.rho, expected,
                           calculated, error, tolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testGreeks) {

    BOOST_TEST_MESSAGE("Testing analytic European exchange option greeks...");

    std::map<std::string,Real> calculated, expected, tolerance;
    tolerance["delta1"]  = 1.0e-5;
    tolerance["delta2"]  = 1.0e-5;
    tolerance["gamma1"]  = 1.0e-5;
    tolerance["gamma2"]  = 1.0e-5;
    tolerance["theta"]   = 1.0e-5;
    tolerance["rho"]     = 1.0e-5;

    Real underlyings1[]  = { 22.0 };
    Real underlyings2[]  = { 20.0 };
    Rate qRates1[]       = { 0.06, 0.16, 0.04 };
    Rate qRates2[]       = { 0.04, 0.14, 0.02 };
    Rate rRates[]        = { 0.1, 0.2, 0.08 };
    Time residualTimes[] = { 0.1, 0.5 };
    Volatility vols1[]   = { 0.20 };
    Volatility vols2[]   = { 0.15, 0.20, 0.25};

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    ext::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> spot2(new SimpleQuote(0.0));

    ext::shared_ptr<SimpleQuote> qRate1(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS1 = flatRate(qRate1, dc);
    ext::shared_ptr<SimpleQuote> qRate2(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS2 = flatRate(qRate2, dc);

    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(rRate, dc);

    ext::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(vol1, dc);
    ext::shared_ptr<SimpleQuote> vol2(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS2 = flatVol(vol2, dc);

    for (Real residualTime : residualTimes) {
        Date exDate = today + timeToDays(residualTime);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        // option to check
        ext::shared_ptr<BlackScholesMertonProcess> stochProcess1(new BlackScholesMertonProcess(
            Handle<Quote>(spot1), Handle<YieldTermStructure>(qTS1), Handle<YieldTermStructure>(rTS),
            Handle<BlackVolTermStructure>(volTS1)));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess2(new BlackScholesMertonProcess(
            Handle<Quote>(spot2), Handle<YieldTermStructure>(qTS2), Handle<YieldTermStructure>(rTS),
            Handle<BlackVolTermStructure>(volTS2)));

        std::vector<ext::shared_ptr<StochasticProcess1D> > procs = {stochProcess1, stochProcess2};

        // The correlation -0.5 can be different real between -1 and 1 for more tests
        Real correlation = -0.5;
        Matrix correlationMatrix(2, 2, correlation);
        for (Integer j = 0; j < 2; j++) {
            correlationMatrix[j][j] = 1.0;

            ext::shared_ptr<PricingEngine> engine(
                new AnalyticEuropeanMargrabeEngine(stochProcess1, stochProcess2, correlation));

            // The quantities of S1 and S2 can be different from 1 & 1 for more tests
            MargrabeOption margrabeOption(1, 1, exercise);

            // analytic engine
            margrabeOption.setPricingEngine(engine);

            for (Size l = 0; l < std::size(underlyings1); l++) {
                for (Size m=0; m < std::size(qRates1); m++) {
                    for (Real n : rRates) {
                        for (Size p = 0; p < std::size(vols1); p++) {
                            Real u1 = underlyings1[l], u2 = underlyings2[l], u;
                            Rate q1 = qRates1[m], q2 = qRates2[m], r = n;
                            Volatility v1 = vols1[p], v2 = vols2[p];

                            spot1->setValue(u1);
                            spot2->setValue(u2);
                            qRate1->setValue(q1);
                            qRate2->setValue(q2);
                            rRate->setValue(r);
                            vol1->setValue(v1);
                            vol2->setValue(v2);

                            Real value = margrabeOption.NPV();

                            calculated["delta1"] = margrabeOption.delta1();
                            calculated["delta2"] = margrabeOption.delta2();
                            calculated["gamma1"] = margrabeOption.gamma1();
                            calculated["gamma2"] = margrabeOption.gamma2();
                            calculated["theta"] = margrabeOption.theta();
                            calculated["rho"] = margrabeOption.rho();

                            if (value > spot1->value() * 1.0e-5) {
                                // perturb spot and get delta1 and gamma
                                u = u1;
                                Real du = u * 1.0e-4;
                                spot1->setValue(u + du);
                                Real value_p = margrabeOption.NPV(),
                                     delta_p = margrabeOption.delta1();
                                spot1->setValue(u - du);
                                Real value_m = margrabeOption.NPV(),
                                     delta_m = margrabeOption.delta1();
                                spot1->setValue(u);
                                expected["delta1"] = (value_p - value_m) / (2 * du);
                                expected["gamma1"] = (delta_p - delta_m) / (2 * du);

                                u = u2;
                                spot2->setValue(u + du);
                                value_p = margrabeOption.NPV();
                                delta_p = margrabeOption.delta2();
                                spot2->setValue(u - du);
                                value_m = margrabeOption.NPV();
                                delta_m = margrabeOption.delta2();
                                spot2->setValue(u);
                                expected["delta2"] = (value_p - value_m) / (2 * du);
                                expected["gamma2"] = (delta_p - delta_m) / (2 * du);

                                // perturb rates and get rho
                                Spread dr = r * 1.0e-4;
                                rRate->setValue(r + dr);
                                value_p = margrabeOption.NPV();
                                rRate->setValue(r - dr);
                                value_m = margrabeOption.NPV();
                                rRate->setValue(r);
                                expected["rho"] = (value_p - value_m) / (2 * dr);

                                // perturb date and get theta
                                Time dT = dc.yearFraction(today - 1, today + 1);
                                Settings::instance().evaluationDate() = today - 1;
                                value_m = margrabeOption.NPV();
                                Settings::instance().evaluationDate() = today + 1;
                                value_p = margrabeOption.NPV();
                                Settings::instance().evaluationDate() = today;
                                expected["theta"] = (value_p - value_m) / dT;

                                // compare
                                std::map<std::string, Real>::iterator it;
                                for (it = calculated.begin(); it != calculated.end(); ++it) {
                                    std::string greek = it->first;
                                    Real expct = expected[greek], calcl = calculated[greek],
                                         tol = tolerance[greek];
                                    Real error = relativeError(expct, calcl, u1);
                                    if (error > tol) {
                                        REPORT_FAILURE2(greek, exercise, u1, u2, q1, q2, r, today,
                                                        v1, v2, expct, calcl, error, tol);
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

BOOST_AUTO_TEST_CASE(testAmericanExchangeTwoAssets) {

    BOOST_TEST_MESSAGE("Testing American one-asset-for-another option...");

    MargrabeAmericanOptionTwoData values[] = {
        //Simplification : we assume that the option always exchanges S2 for S1
        //s1, s2, Q1, Q2, q1, q2, r, t, v1, v2, rho, result, tol
        // data from Haug

        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.15, -0.50, 2.1357, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.20, -0.50, 2.2074, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.25, -0.50, 2.2902, 1.0e-3},

        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.15, 0.00, 2.0592, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.20, 0.00, 2.1032, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.25, 0.00, 2.1618, 1.0e-3},

        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.15, 0.50, 2.0001, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.20, 0.50, 2.0110, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.10, 0.20, 0.25, 0.50, 2.0359, 1.0e-3},

        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.15, -0.50, 2.8051, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.20, -0.50, 3.0288, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.25, -0.50, 3.2664, 1.0e-3},

        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.15, 0.00, 2.5282, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.20, 0.00, 2.6945, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.25, 0.00, 2.8893, 1.0e-3},

        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.15, 0.50, 2.2053, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.20, 0.50, 2.2906, 1.0e-3},
        {22.0, 20.0, 1, 1, 0.06, 0.04, 0.10, 0.50, 0.20, 0.25, 0.50, 2.4261, 1.0e-3}
    };

    Date today = Settings::instance().evaluationDate();
    DayCounter dc = Actual360();
    ext::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> spot2(new SimpleQuote(0.0));

    ext::shared_ptr<SimpleQuote> qRate1(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS1 = flatRate(today, qRate1, dc);
    ext::shared_ptr<SimpleQuote> qRate2(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS2 = flatRate(today, qRate2, dc);

    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    ext::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);
    ext::shared_ptr<SimpleQuote> vol2(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS2 = flatVol(today, vol2, dc);

    for (auto& value : values) {

        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new AmericanExercise(today, exDate));

        spot1->setValue(value.s1);
        spot2->setValue(value.s2);
        qRate1->setValue(value.q1);
        qRate2->setValue(value.q2);
        rRate->setValue(value.r);
        vol1->setValue(value.v1);
        vol2->setValue(value.v2);

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess1(new
            BlackScholesMertonProcess(Handle<Quote>(spot1),
                                      Handle<YieldTermStructure>(qTS1),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS1)));

        ext::shared_ptr<BlackScholesMertonProcess> stochProcess2(new
            BlackScholesMertonProcess(Handle<Quote>(spot2),
                                      Handle<YieldTermStructure>(qTS2),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS2)));

        std::vector<ext::shared_ptr<StochasticProcess1D> > procs = {stochProcess1,stochProcess2};

        Matrix correlationMatrix(2, 2, value.rho);
        for (Integer j=0; j < 2; j++) {
            correlationMatrix[j][j] = 1.0;
        }

        ext::shared_ptr<PricingEngine> engine(
            new AnalyticAmericanMargrabeEngine(stochProcess1, stochProcess2, value.rho));

        MargrabeOption margrabeOption(value.Q1, value.Q2, exercise);

        // analytic engine
        margrabeOption.setPricingEngine(engine);

        Real calculated = margrabeOption.NPV();
        Real expected = value.result;
        Real error = std::fabs(calculated-expected);
        Real tolerance = value.tol;
        if (error > tolerance) {
            REPORT_FAILURE("value", exercise, value.s1, value.s2, value.Q1, value.Q2, value.q1,
                           value.q2, value.r, today, value.v1, value.v2, value.rho, expected,
                           calculated, error, tolerance);
        }
    }
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
