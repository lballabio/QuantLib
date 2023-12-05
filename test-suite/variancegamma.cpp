/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2010 Adrian O' Neill
Copyright (C) 2018 Roy Zywina

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/experimental/variancegamma/analyticvariancegammaengine.hpp>
#include <ql/experimental/variancegamma/fftvariancegammaengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef REPORT_FAILURE
#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, sigma, \
    nu, theta, expected, calculated, \
    error, tolerance) \
    BOOST_FAIL(exerciseTypeToString(exercise) << " " \
    << payoff->optionType() << " option with " \
    << payoffTypeToString(payoff) << " payoff:\n" \
    << "    underlying value: " << s << "\n" \
    << "    strike:           " << payoff->strike() <<"\n" \
    << "    dividend yield:   " << io::rate(q) << "\n" \
    << "    risk-free rate:   " << io::rate(r) << "\n" \
    << "    reference date:   " << today << "\n" \
    << "    maturity:         " << exercise->lastDate() << "\n" \
    << "    sigma:            " << sigma << "\n" \
    << "    nu:               " << nu << "\n" \
    << "    theta:            " << theta << "\n\n" \
    << "    expected   " << greekName << ": " << expected << "\n" \
    << "    calculated " << greekName << ": " << calculated << "\n"\
    << "    error:            " << error << "\n" \
    << "    tolerance:        " << tolerance);

namespace {

    struct VarianceGammaProcessData {
        Real s;        // spot
        Rate q;        // dividend
        Rate r;        // risk-free rate
        Real sigma;
        Real nu;
        Real theta;

    };

    struct VarianceGammaOptionData {
        Option::Type type;
        Real strike;
        Time t;        // time to maturity
    };
}

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(VarianceGammaExperimentalTest)

BOOST_AUTO_TEST_CASE(testVarianceGamma) {

    BOOST_TEST_MESSAGE("Testing variance-gamma model for European options...");

    VarianceGammaProcessData processes[] = {
    //    spot,    q,    r,sigma,   nu, theta
        { 6000, 0.00, 0.05, 0.20, 0.05, -0.50 },
        { 6000, 0.02, 0.05, 0.15, 0.01, -0.50 }
    };

    VarianceGammaOptionData options[] = {
    //            type,strike,  t
        { Option::Call, 5550, 1.0},
        { Option::Call, 5600, 1.0},
        { Option::Call, 5650, 1.0},
        { Option::Call, 5700, 1.0},
        { Option::Call, 5750, 1.0},
        { Option::Call, 5800, 1.0},
        { Option::Call, 5850, 1.0},
        { Option::Call, 5900, 1.0},
        { Option::Call, 5950, 1.0},
        { Option::Call, 6000, 1.0},
        { Option::Call, 6050, 1.0},
        { Option::Call, 6100, 1.0},
        { Option::Call, 6150, 1.0},
        { Option::Call, 6200, 1.0},
        { Option::Call, 6250, 1.0},
        { Option::Call, 6300, 1.0},
        { Option::Call, 6350, 1.0},
        { Option::Call, 6400, 1.0},
        { Option::Call, 6450, 1.0},
        { Option::Call, 6500, 1.0},
        { Option::Call, 6550, 1.0},
        { Option::Put,  5550, 1.0}
    };

    Real results[LENGTH(processes)][LENGTH(options)] = {
        {
            955.1637, 922.7529, 890.9872, 859.8739, 829.4197, 799.6303, 770.5104, 742.0640,
            714.2943, 687.2032, 660.7921, 635.0613, 610.0103, 585.6379, 561.9416, 538.9186,
            516.5649, 494.8760, 473.8464, 453.4700, 433.7400, 234.4870
        },
        {
            732.8705, 698.5542, 665.1404, 632.6498, 601.1002, 570.5068, 540.8824, 512.2367,
            484.5766, 457.9064, 432.2273, 407.5381, 383.8346, 361.1102, 339.3559, 318.5599,
            298.7087, 279.7864, 261.7751, 244.6552, 228.4057, 130.9974
        }
    };

    Real tol = 0.01;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    for (Size i=0; i<LENGTH(processes); i++) {
        ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(processes[i].s));
        ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(processes[i].q));
        ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
        ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(processes[i].r));
        ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

        ext::shared_ptr<VarianceGammaProcess> stochProcess(
            new VarianceGammaProcess(Handle<Quote>(spot),
            Handle<YieldTermStructure>(qTS),
            Handle<YieldTermStructure>(rTS),
            processes[i].sigma,
            processes[i].nu,
            processes[i].theta));

        // Analytic engine
        ext::shared_ptr<PricingEngine> analyticEngine(
            new VarianceGammaEngine(stochProcess));

        // FFT engine
        ext::shared_ptr<FFTVarianceGammaEngine> fftEngine(
            new FFTVarianceGammaEngine(stochProcess));

        // which requires a list of options
        std::vector<ext::shared_ptr<Instrument> > optionList;

        std::vector<ext::shared_ptr<StrikedTypePayoff> > payoffs;
        for (Size j=0; j<LENGTH(options); j++)
        {
            Date exDate = today + timeToDays(options[j].t);
            ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

            ext::shared_ptr<StrikedTypePayoff> payoff(new
                PlainVanillaPayoff(options[j].type, options[j].strike));
            payoffs.push_back(payoff);

            // Test analytic engine
            ext::shared_ptr<EuropeanOption> option(new EuropeanOption(payoff, exercise));
            option->setPricingEngine(analyticEngine);

            Real calculated = option->NPV();
            Real expected = results[i][j];
            Real error = std::fabs(calculated-expected);

            if (error>tol) {
                REPORT_FAILURE("analytic value", payoff, exercise,
                    processes[i].s, processes[i].q, processes[i].r,
                    today, processes[i].sigma, processes[i].nu,
                    processes[i].theta, expected, calculated,
                    error, tol);  
            }
            optionList.push_back(option);
        }

        // Test FFT engine
        // FFT engine is extremely efficient when sent a list of options to calculate first
        fftEngine->precalculate(optionList);
        for (Size j=0; j<LENGTH(options); j++)
        {
            ext::shared_ptr<VanillaOption> option = ext::static_pointer_cast<VanillaOption>(optionList[j]);
            option->setPricingEngine(fftEngine);

            Real calculated = option->NPV();
            Real expected = results[i][j];
            Real error = std::fabs(calculated-expected);
            if (error>tol) {
                ext::shared_ptr<StrikedTypePayoff> payoff = 
                    ext::dynamic_pointer_cast<StrikedTypePayoff>(option->payoff());
                REPORT_FAILURE("fft value", payoff, option->exercise(),
                    processes[i].s, processes[i].q, processes[i].r,
                    today, processes[i].sigma, processes[i].nu,
                    processes[i].theta, expected, calculated,
                    error, tol);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testSingularityAtZero) {

    BOOST_TEST_MESSAGE(
        "Testing variance-gamma model integration around zero...");

    Real stock = 100;
    Real strike = 98;
    Volatility sigma = 0.12;
    Real mu = -0.14;
    Real kappa = 0.2;

    Date valuation(1,Jan,2017);
    Date maturity(10,Jan,2017);
    DayCounter discountCounter = Thirty360(Thirty360::BondBasis);

    Settings::instance().evaluationDate() = valuation;

    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturity);
    ext::shared_ptr<StrikedTypePayoff> payoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Call, strike);
    VanillaOption option(payoff, exercise);

    Handle<YieldTermStructure> dividend(
             ext::make_shared<FlatForward>(valuation,0.0,discountCounter));
    Handle<YieldTermStructure> disc(
             ext::make_shared<FlatForward>(valuation,0.05,discountCounter));
    Handle<Quote> S0(ext::make_shared<SimpleQuote>(stock));
    ext::shared_ptr<QuantLib::VarianceGammaProcess> process =
        ext::make_shared<VarianceGammaProcess>(S0, dividend, disc,
                                                 sigma, kappa, mu);

    option.setPricingEngine(ext::make_shared<VarianceGammaEngine>(process));
    // without the fix, the call below goes into an infinite loop,
    // which is hard to test for.  We're just happy to see the test
    // case finish, hence the lack of an assertion.
    option.NPV();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()