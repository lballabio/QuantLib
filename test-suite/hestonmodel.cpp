/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "hestonmodel.hpp"
#include "utilities.hpp"
#include <ql/Processes/hestonprocess.hpp>
#include <ql/ShortRateModels/TwoFactorModels/hestonmodel.hpp>
#include <ql/ShortRateModels/CalibrationHelpers/hestonmodelhelper.hpp>
#include <ql/PricingEngines/Vanilla/analytichestonengine.hpp>
#include <ql/PricingEngines/Vanilla/mceuropeanhestonengine.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/DayCounters/actualactual.hpp>
#include <ql/TermStructures/zerocurve.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Optimization/levenbergmarquardt.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(HestonModelTest)

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

QL_END_TEST_LOCALS(HestonModelTest)


void HestonModelTest::testBlackCalibration() {
    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_BORLAND)

    BOOST_MESSAGE(
       "Testing Heston model calibration using a flat volatility surface...");

    QL_TEST_BEGIN

    /* calibrate a Heston model to a constant volatility surface without
       smile. expected result is a vanishing volatility of the volatility.
       In addition theta and v0 should be equal to the constant variance */

    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    DayCounter dayCounter = Actual360();
    Calendar calendar = NullCalendar();

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.04, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.50, dayCounter));

    std::vector<Period> optionMaturities;
    optionMaturities.push_back(Period(1, Months));
    optionMaturities.push_back(Period(2, Months));
    optionMaturities.push_back(Period(3, Months));
    optionMaturities.push_back(Period(6, Months));
    optionMaturities.push_back(Period(9, Months));
    optionMaturities.push_back(Period(1, Years));
    optionMaturities.push_back(Period(2, Years));

    std::vector<boost::shared_ptr<CalibrationHelper> > options;
    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(1.0)));
    Handle<Quote> vol(boost::shared_ptr<Quote>(new SimpleQuote(0.1)));
    Volatility volatility = vol->value();

    for (Size i = 0; i < optionMaturities.size(); ++i) {
        for (Real moneyness = -1.0; moneyness < 2.0; moneyness += 1.0) {
            const Time tau = dayCounter.yearFraction(
                                 riskFreeTS->referenceDate(),
                                 calendar.advance(riskFreeTS->referenceDate(),
                                                  optionMaturities[i]));
        const Real fwdPrice = s0->value()*dividendTS->discount(tau)
                            / riskFreeTS->discount(tau);
        const Real strikePrice = fwdPrice * std::exp(-moneyness * volatility
                                                     * std::sqrt(tau));

        options.push_back(boost::shared_ptr<CalibrationHelper>(
                          new HestonModelHelper(optionMaturities[i], calendar,
                                                s0->value(), strikePrice, vol,
                                                riskFreeTS, dividendTS)));
        }
    }

    for (Real sigma = 0.1; sigma < 0.9; sigma += 0.2) {
        boost::shared_ptr<HestonProcess> process(
                                     new HestonProcess(riskFreeTS, dividendTS,
                                                       s0, 0.01, 0.2, 0.02,
                                                       sigma, -0.75));

        boost::shared_ptr<HestonModel> model(new HestonModel(process));
        boost::shared_ptr<PricingEngine> engine(
                                         new AnalyticHestonEngine(model, 96));

        for (Size i = 0; i < options.size(); ++i)
            options[i]->setPricingEngine(engine);

        LevenbergMarquardt om;
        model->calibrate(options, om);

        Real tolerance = 1.0e-3;

        if (model->sigma() > tolerance) {
            BOOST_ERROR("Failed to reproduce expected sigma"
                        << "\n    calculated: " << model->sigma()
                        << "\n    expected:   " << 0.0);
        }

        if (std::fabs(model->kappa()
                  *(model->theta()-volatility*volatility)) > tolerance) {
            BOOST_ERROR("Failed to reproduce expected theta"
                        << "\n    calculated: " << model->theta()
                        << "\n    expected:   " << volatility*volatility);
        }

        if (std::fabs(model->v0()-volatility*volatility) > tolerance) {
            BOOST_ERROR("Failed to reproduce expected v0"
                        << "\n    calculated: " << model->v0()
                        << "\n    expected:   " << volatility*volatility);
        }
    }

    QL_TEST_TEARDOWN
    #endif
}


void HestonModelTest::testDAXCalibration() {
    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_BORLAND)

    /* this example is taken from A. Sepp
       Pricing European-Style Options under Jump Diffusion Processes
       with Stochstic Volatility: Applications of Fourier Transform
       http://math.ut.ee/~spartak/papers/stochjumpvols.pdf
    */

    BOOST_MESSAGE(
             "Testing Heston model calibration using DAX volatility data...");

    QL_TEST_BEGIN

    Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = Actual365Fixed();
    Calendar calendar = TARGET();

    Integer t[] = { 13, 41, 75, 165, 256, 345, 524, 703 };
    Rate r[] = { 0.0357,0.0349,0.0341,0.0355,0.0359,0.0368,0.0386,0.0401 };

    std::vector<Date> dates;
    std::vector<Rate> rates;
    dates.push_back(settlementDate);
    rates.push_back(0.0357);
    Size i;
    for (i = 0; i < 8; ++i) {
        dates.push_back(settlementDate + t[i]);
        rates.push_back(r[i]);
    }

    Handle<YieldTermStructure> riskFreeTS(
                       boost::shared_ptr<YieldTermStructure>(
                                    new ZeroCurve(dates, rates, dayCounter)));

    Handle<YieldTermStructure> dividendTS(
                                   flatRate(settlementDate, 0.0, dayCounter));

    Volatility v[] =
      { 0.6625,0.4875,0.4204,0.3667,0.3431,0.3267,0.3121,0.3121,
        0.6007,0.4543,0.3967,0.3511,0.3279,0.3154,0.2984,0.2921,
        0.5084,0.4221,0.3718,0.3327,0.3155,0.3027,0.2919,0.2889,
        0.4541,0.3869,0.3492,0.3149,0.2963,0.2926,0.2819,0.2800,
        0.4060,0.3607,0.3330,0.2999,0.2887,0.2811,0.2751,0.2775,
        0.3726,0.3396,0.3108,0.2781,0.2788,0.2722,0.2661,0.2686,
        0.3550,0.3277,0.3012,0.2781,0.2781,0.2661,0.2661,0.2681,
        0.3428,0.3209,0.2958,0.2740,0.2688,0.2627,0.2580,0.2620,
        0.3302,0.3062,0.2799,0.2631,0.2573,0.2533,0.2504,0.2544,
        0.3343,0.2959,0.2705,0.2540,0.2504,0.2464,0.2448,0.2462,
        0.3460,0.2845,0.2624,0.2463,0.2425,0.2385,0.2373,0.2422,
        0.3857,0.2860,0.2578,0.2399,0.2357,0.2327,0.2312,0.2351,
        0.3976,0.2860,0.2607,0.2356,0.2297,0.2268,0.2241,0.2320 };

    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(4468.17)));
    Real strike[] = { 3400,3600,3800,4000,4200,4400,
                      4500,4600,4800,5000,5200,5400,5600 };

    std::vector<boost::shared_ptr<CalibrationHelper> > options;

    for (Size s = 0; s < 13; ++s) {
        for (Size m = 0; m < 8; ++m) {
            Handle<Quote> vol(boost::shared_ptr<Quote>(
                                                  new SimpleQuote(v[s*8+m])));

            Period maturity((int)((t[m]+3)/7.), Weeks); // round to weeks
            options.push_back(boost::shared_ptr<CalibrationHelper>(
                        new HestonModelHelper(maturity, calendar,
                                              s0->value(), strike[s], vol,
                                              riskFreeTS, dividendTS, true)));
        }
    }

    boost::shared_ptr<HestonProcess> process(new HestonProcess(
                         riskFreeTS, dividendTS, s0, 0.1, 1, 0.1, 0.5, -0.5));

    boost::shared_ptr<HestonModel> model(new HestonModel(process));

    boost::shared_ptr<PricingEngine> engine(
                                         new AnalyticHestonEngine(model, 64));

    for (i = 0; i < options.size(); ++i)
        options[i]->setPricingEngine(engine);

    LevenbergMarquardt om;
    model->calibrate(options, om);

    Real sse = 0;
    for (i = 0; i < 13*8; ++i) {
        const Real diff = options[i]->calibrationError()*100.0;
        sse += diff*diff;
    }
    Real expected = 177.2; //see article by A. Sepp.
    if (std::fabs(sse - expected) > 1.0) {
        BOOST_FAIL("Failed to reproduce calibration error"
                   << "\n    calculated: " << sse
                   << "\n    expected:   " << expected);
    }

    QL_TEST_TEARDOWN
    #endif
}

void HestonModelTest::testAnalyticVsBlack() {
    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_BORLAND)

    BOOST_MESSAGE("Testing analytic Heston engine against Black formula...");

    QL_TEST_BEGIN

    Date settlementDate = Date::todaysDate();
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual();
    Date exerciseDate = settlementDate + 6*Months;

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                     new PlainVanillaPayoff(Option::Put, 30));
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.1, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.04, dayCounter));

    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(32.0)));
    Real v0 = 0.05;
    Real kappa = 5.0;
    Real theta = 0.05;
    Real sigma = 1.0e-4;
    Real rho = 0.0;

    boost::shared_ptr<HestonProcess> process(new HestonProcess(
                   riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

    VanillaOption option(process, payoff, exercise);

    boost::shared_ptr<PricingEngine> engine(new AnalyticHestonEngine(
              boost::shared_ptr<HestonModel>(new HestonModel(process)), 144));

    option.setPricingEngine(engine);
    Real calculated = option.NPV();

    Real yearFraction = dayCounter.yearFraction(settlementDate, exerciseDate);
    Real forwardPrice = 32*std::exp((0.1-0.04)*yearFraction);
    Real expected = BlackFormula(forwardPrice, std::exp(-0.1*yearFraction),
                                 0.05*yearFraction, payoff).value();
    Real error = std::fabs(calculated - expected);
    Real tolerance = 5.0e-8;
    if (error > tolerance) {
        BOOST_FAIL("failed to reproduce Black price"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << QL_SCIENTIFIC << error);
    }

    QL_TEST_TEARDOWN
    #endif
}


void HestonModelTest::testAnalyticVsCached() {
    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_BORLAND)

    BOOST_MESSAGE("Testing analytic Heston engine against cached values...");

    QL_TEST_BEGIN

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual();
    Date exerciseDate(28, March, 2005);

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                  new PlainVanillaPayoff(Option::Call, 1.05));
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.0225, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(1.0)));
    Real v0 = 0.1;
    Real kappa = 3.16;
    Real theta = 0.09;
    Real sigma = 0.4;
    Real rho = -0.2;

    boost::shared_ptr<HestonProcess> process(new HestonProcess(
                   riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

    VanillaOption option(process, payoff, exercise);

    boost::shared_ptr<AnalyticHestonEngine> engine(new AnalyticHestonEngine(
               boost::shared_ptr<HestonModel>(new HestonModel(process)), 64));

    option.setPricingEngine(engine);

    Real expected1 = 0.0404774515;
    Real calculated1 = option.NPV();
    Real tolerance = 1.0e-8;

    if (std::fabs(calculated1 - expected1) > tolerance) {
        BOOST_ERROR("Failed to reproduce cached analytic price"
                    << "\n    calculated: " << calculated1
                    << "\n    expected:   " << expected1);
    }


    // reference values from www.wilmott.com, technical forum
    // search for "Heston or VG price check"

    Real K[] = {0.9,1.0,1.1};
    Real expected2[] = { 0.1330371,0.0641016, 0.0270645 };
    Real calculated2[6];

    Size i;
    for (i = 0; i < 6; ++i) {
        Date exerciseDate(8+i/3, September, 2005);

        boost::shared_ptr<StrikedTypePayoff> payoff(
                                new PlainVanillaPayoff(Option::Call, K[i%3]));
        boost::shared_ptr<Exercise> exercise(
                                          new EuropeanExercise(exerciseDate));

        Handle<YieldTermStructure> riskFreeTS(flatRate(0.05, dayCounter));
        Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

        Real s = riskFreeTS->discount(0.7)/dividendTS->discount(0.7);
        Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(s)));
        Real v0 = 0.09;
        Real kappa = 1.2;
        Real theta = 0.08;
        Real sigma = 1.8;
        Real rho = -0.45;

        boost::shared_ptr<HestonProcess> process(new HestonProcess(
                   riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

        VanillaOption option(process, payoff, exercise);

        boost::shared_ptr<PricingEngine> engine(new AnalyticHestonEngine(
                   boost::shared_ptr<HestonModel>(new HestonModel(process))));

        option.setPricingEngine(engine);
        calculated2[i] = option.NPV();
    }

    // we are after the value for T=0.7
    Time t1 = dayCounter.yearFraction(settlementDate, Date(8, September,2005));
    Time t2 = dayCounter.yearFraction(settlementDate, Date(9, September,2005));

    for (i = 0; i < 3; ++i) {
        const Real interpolated =
            calculated2[i]+(calculated2[i+3]-calculated2[i])/(t2-t1)*(0.7-t1);

        if (std::fabs(interpolated - expected2[i]) > 100*tolerance) {
            BOOST_ERROR("Failed to reproduce cached analytic prices:"
                        << "\n    calculated: " << interpolated
                        << "\n    expected:   " << expected2[i] );
        }
    }

    QL_TEST_TEARDOWN
    #endif
}


void HestonModelTest::testMcVsCached() {
    #if !defined(QL_PATCH_BORLAND)

    BOOST_MESSAGE(
                "Testing Monte Carlo Heston engine against cached values...");

    QL_TEST_BEGIN

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual();
    Date exerciseDate(28, March, 2005);

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                   new PlainVanillaPayoff(Option::Put, 1.05));
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.7, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.4, dayCounter));

    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(1.05)));
    Real v0 = 0.3;
    Real kappa = 1.16;
    Real theta = 0.2;
    Real sigma = 0.8;
    Real rho = 0.8;

    boost::shared_ptr<HestonProcess> process(new HestonProcess(
                   riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

    VanillaOption option(process, payoff, exercise);

    boost::shared_ptr<PricingEngine> engine;
    engine = MakeMCEuropeanHestonEngine<PseudoRandom>()
        .withStepsPerYear(91)
        .withAntitheticVariate()
        .withSamples(50000)
        .withSeed(1234);

    option.setPricingEngine(engine);

    Real expected = 0.0632851308977151;
    Real calculated = option.NPV();
    Real errorEstimate = option.errorEstimate();
    Real tolerance = 7.5e-4;

    if (std::fabs(calculated - expected) > 2.34*errorEstimate) {
        BOOST_ERROR("Failed to reproduce cached price"
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << " +/- " << errorEstimate);
    }

    if (errorEstimate > tolerance) {
        BOOST_ERROR("failed to reproduce error estimate"
                    << "\n    calculated: " << errorEstimate
                    << "\n    expected:   " << tolerance);
    }

    QL_TEST_TEARDOWN
    #endif
}


void HestonModelTest::testEngines() {
    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_BORLAND)

    BOOST_MESSAGE(
       "Testing Monte Carlo Heston engine against analytic Heston engine...");


    QL_TEST_BEGIN

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual();
    Date exerciseDate(28, July, 2005);

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                   new PlainVanillaPayoff(Option::Put, 1.05));
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.7, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.4, dayCounter));

    Real v0 = 0.8;
    Real theta = 0.4;
    Real rho = -0.8;

    for (Real s0 = 0.5; s0 < 1.6; s0 += 0.5) {
        for (Real kappa = 1.0; kappa < 8.0; kappa += 2.0) {
            for (Real sigma = 0.5; sigma < 7.0; sigma += 2.0) {

                BOOST_MESSAGE("s0 = " << s0
                              << ", kappa = " << kappa
                              << ", sigma = " << sigma);

                Handle<Quote> q(boost::shared_ptr<Quote>(new SimpleQuote(s0)));
                boost::shared_ptr<HestonProcess> process(new HestonProcess(
                    riskFreeTS, dividendTS, q, v0, kappa, theta, sigma, rho));

                VanillaOption option(process, payoff, exercise);

                boost::shared_ptr<PricingEngine> engine1;
                engine1 = MakeMCEuropeanHestonEngine<PseudoRandom>()
                    .withStepsPerYear(1825)
                    .withAntitheticVariate()
                    .withSamples(20000)
                    .withSeed(1234);

                boost::shared_ptr<PricingEngine> engine2(
                    new AnalyticHestonEngine(boost::shared_ptr<HestonModel>(
                                               new HestonModel(process)),192));

                option.setPricingEngine(engine1);
                Real calculated = option.NPV();
                Real errorEstimate = option.errorEstimate();

                option.setPricingEngine(engine2);
                Real expected = option.NPV();
                Real tolerance = 7.5e-4;

                if (std::fabs(calculated - expected) > 1.65*errorEstimate) {
                    BOOST_ERROR("failed to match results from engines"
                                << "\n    analytic:    " << expected
                                << "\n    Monte Carlo: " << calculated
                                << " +/- " << errorEstimate);
                }
                if (errorEstimate > 2.0*tolerance) {
                    BOOST_ERROR("failed to reproduce error estimate"
                                << "\n    calculated: " << errorEstimate
                                << "\n    expected:   " << tolerance);
                }
            }
        }
    }

    QL_TEST_TEARDOWN
    #endif
}

test_suite* HestonModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Heston model tests");
    #if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_BORLAND)
    suite->add(BOOST_TEST_CASE(&HestonModelTest::testBlackCalibration));
    suite->add(BOOST_TEST_CASE(&HestonModelTest::testDAXCalibration));
    suite->add(BOOST_TEST_CASE(&HestonModelTest::testAnalyticVsBlack));
    suite->add(BOOST_TEST_CASE(&HestonModelTest::testAnalyticVsCached));
    // this passes but takes way too long
    // suite->add(BOOST_TEST_CASE(&HestonModelTest::testEngines));
    #endif
    #if !defined(QL_PATCH_BORLAND)
    suite->add(BOOST_TEST_CASE(&HestonModelTest::testMcVsCached));
    #endif
    return suite;
}

