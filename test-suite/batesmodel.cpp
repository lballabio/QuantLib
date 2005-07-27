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

#include "batesmodel.hpp"
#include "utilities.hpp"
#include <ql/Calendars/target.hpp>
#include <ql/Processes/merton76process.hpp>
#include <ql/Instruments/europeanoption.hpp>
#include <ql/DayCounters/actualactual.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Optimization/simplex.hpp>
#include <ql/TermStructures/zerocurve.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/PricingEngines/Vanilla/batesengine.hpp>
#include <ql/PricingEngines/Vanilla/jumpdiffusionengine.hpp>
#include <ql/PricingEngines/Vanilla/analyticeuropeanengine.hpp>
#include <ql/ShortRateModels/TwoFactorModels/batesmodel.hpp>
#include <ql/ShortRateModels/CalibrationHelpers/hestonmodelhelper.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(BatesModelTest)

    Real getCalibrationError(
               std::vector<boost::shared_ptr<CalibrationHelper> > & options) {
    Real sse = 0;
    for (Size i = 0; i < options.size(); ++i) {
        const Real diff = options[i]->calibrationError()*100.0;
        sse += diff*diff;
    }
    return sse;
}

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

QL_END_TEST_LOCALS(BatesModelTest)

void BatesModelTest::testAnalyticVsBlack() {

    BOOST_MESSAGE("Testing analytic Bates engine against Black formula...");

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

    Real yearFraction = dayCounter.yearFraction(settlementDate, exerciseDate);
    Real forwardPrice = s0->value()*std::exp((0.1-0.04)*yearFraction);
    Real expected = BlackFormula(forwardPrice, std::exp(-0.1*yearFraction),
                                 0.05*yearFraction, payoff).value();

    Real v0 = 0.05;
    Real kappa = 5.0;
    Real theta = 0.05;
    Real sigma = 1.0e-4;
    Real rho = 0.0;

    boost::shared_ptr<HestonProcess> process(new HestonProcess(
        riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

    VanillaOption option(process, payoff, exercise);

    boost::shared_ptr<PricingEngine> engine(new BatesEngine(
        boost::shared_ptr<BatesModel>(
            new BatesModel(process, 0.0001, 0, 0.0001)), 64));

    option.setPricingEngine(engine);
    Real calculated = option.NPV();

    Real tolerance = 1.0e-8;
    if (std::fabs(calculated - expected) > tolerance) {
        BOOST_FAIL("failed to reproduce Black price with BatesEngine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected);
    }

    engine = boost::shared_ptr<PricingEngine>(new BatesDetJumpEngine(
        boost::shared_ptr<BatesDetJumpModel>(
            new BatesDetJumpModel(
                process, 0.0001, 0.0, 0.0001, 1.0, 0.0001)), 64));

    option.setPricingEngine(engine);
    calculated = option.NPV();

    if (std::fabs(calculated - expected) > tolerance) {
        BOOST_FAIL("failed to reproduce Black price with " \
                   "BatesDetJumpEngine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected);
    }

    engine = boost::shared_ptr<PricingEngine>(new BatesDoubleExpEngine(
        boost::shared_ptr<BatesDoubleExpModel>(
            new BatesDoubleExpModel(process, 0.0001, 0.0001, 0.0001)), 64));

    option.setPricingEngine(engine);
    calculated = option.NPV();

    if (std::fabs(calculated - expected) > tolerance) {
        BOOST_FAIL("failed to reproduce Black price with BatesDoubleExpEngine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected);
    }

    engine = boost::shared_ptr<PricingEngine>(new BatesDoubleExpDetJumpEngine(
        boost::shared_ptr<BatesDoubleExpDetJumpModel>(
            new BatesDoubleExpDetJumpModel(
                process, 0.0001, 0.0001, 0.0001, 0.5, 1.0, 0.0001)), 64));

    option.setPricingEngine(engine);
    calculated = option.NPV();

    if (std::fabs(calculated - expected) > tolerance) {
        BOOST_FAIL("failed to reproduce Black price with " \
                   "BatesDoubleExpDetJumpEngine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected);
    }

    QL_TEST_TEARDOWN
}


void BatesModelTest::testAnalyticVsJumpDiffusion() {

    BOOST_MESSAGE("Testing analytic Bates engine against Merton-76 engine...");

    QL_TEST_BEGIN
    Date settlementDate = Date::todaysDate();
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual();

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                     new PlainVanillaPayoff(Option::Put, 95));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.1, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.04, dayCounter));
    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(100)));

    Real v0 = 0.0433;
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(std::sqrt(v0)));
    boost::shared_ptr<BlackVolTermStructure> volTS =
        flatVol(settlementDate, vol, dayCounter);

    Real kappa = 0.5;
    Real theta = v0;
    Real sigma = 1.0e-4;
    Real rho = 0.0;

    boost::shared_ptr<HestonProcess> process(new HestonProcess(
        riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

    boost::shared_ptr<SimpleQuote> jumpIntensity(new SimpleQuote(2));
    boost::shared_ptr<SimpleQuote> meanLogJump(new SimpleQuote(-0.2));
    boost::shared_ptr<SimpleQuote> jumpVol(new SimpleQuote(0.2));

    boost::shared_ptr<PricingEngine> batesEngine(new BatesEngine(
        boost::shared_ptr<BatesModel>(
            new BatesModel(process,
                                 jumpIntensity->value(),
                                 meanLogJump->value(),
                                 jumpVol->value())), 160));

    boost::shared_ptr<StochasticProcess> stochProcess(
        new Merton76Process(s0,
                            dividendTS,
                            riskFreeTS,
                            Handle<BlackVolTermStructure>(volTS),
                            Handle<Quote>(jumpIntensity),
                            Handle<Quote>(meanLogJump),
                            Handle<Quote>(jumpVol)));

    boost::shared_ptr<VanillaOption::engine> baseEngine(
        new AnalyticEuropeanEngine);
    boost::shared_ptr<PricingEngine> mertonEngine(
        new JumpDiffusionEngine(baseEngine, 1e-10, 1000));

    for (Integer i=1; i<48; ++i) {
        Date exerciseDate = settlementDate + i*Months;
        boost::shared_ptr<Exercise> exercise(
            new EuropeanExercise(exerciseDate));
        VanillaOption batesOption(process, payoff, exercise, batesEngine);
        Real calculated = batesOption.NPV();

        EuropeanOption mertonOption(stochProcess, payoff,
                                    exercise, mertonEngine);
        Real expected = mertonOption.NPV();

        Real tolerance = 1e-8;
        if (std::isnan(calculated)
        || std::fabs(calculated - expected)/expected > tolerance) {
            BOOST_FAIL("failed to reproduce Merton76 price with " \
                       "BatesEngine"
                       << "\n    calculated: " << calculated
                       << "\n    expected:   " << expected);
        }
    }

    QL_TEST_TEARDOWN
}

void BatesModelTest::testDAXCalibration() {

    /* this example is taken from A. Sepp
       Pricing European-Style Options under Jump Diffusion Processes
       with Stochstic Volatility: Applications of Fourier Transform
       http://math.ut.ee/~spartak/papers/stochjumpvols.pdf
    */

    BOOST_MESSAGE(
             "Testing Bates model calibration using DAX volatility data...");

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
    for (Size i = 0; i < 8; ++i) {
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


    Real v0 = 0.0433;
    boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(std::sqrt(v0)));
    boost::shared_ptr<BlackVolTermStructure> volTS =
        flatVol(settlementDate, vol, dayCounter);

    Real kappa = 1.0;
    Real theta = v0;
    Real sigma = 1.0e-4;
    Real rho = 0.0;

    boost::shared_ptr<SimpleQuote> jumpIntensity(new SimpleQuote(1.1098));
    boost::shared_ptr<SimpleQuote> meanLogJump(new SimpleQuote(-0.1285));
    boost::shared_ptr<SimpleQuote> jumpVol(new SimpleQuote(0.1702));

    // this is the merton76 option
    boost::shared_ptr<StochasticProcess> mertonProcess(
        new Merton76Process(s0, dividendTS, riskFreeTS,
                            Handle<BlackVolTermStructure>(volTS),
                            Handle<Quote>(jumpIntensity),
                            Handle<Quote>(meanLogJump),
                            Handle<Quote>(jumpVol)));


    std::vector<boost::shared_ptr<CalibrationHelper> > options;
    std::vector<boost::shared_ptr<EuropeanOption> > merton76options;

    for (Size s = 0; s < 13; ++s) {
        for (Size m = 0; m < 8; ++m) {
            Handle<Quote> vol(boost::shared_ptr<Quote>(
                                                  new SimpleQuote(v[s*8+m])));

            Period maturity((int)((t[m]+3)/7.), Weeks); // round to weeks

            // this is the calibration helper for the bates models
            options.push_back(boost::shared_ptr<CalibrationHelper>(
                        new HestonModelHelper(maturity, calendar,
                                              s0->value(), strike[s], vol,
                                              riskFreeTS, dividendTS, true)));

            boost::shared_ptr<Exercise> exercise(
                new EuropeanExercise(settlementDate+maturity));
            boost::shared_ptr<StrikedTypePayoff> payoff(
                new PlainVanillaPayoff(Option::Call, strike[s]));
            merton76options.push_back(
                boost::shared_ptr<EuropeanOption>(new EuropeanOption(
                    mertonProcess, payoff, exercise)));
        }
    }

   boost::shared_ptr<HestonProcess> process(new HestonProcess(
        riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

   boost::shared_ptr<BatesModel> batesModel(
       new BatesModel(process,
                      jumpIntensity->value(),
                      meanLogJump->value(),
                      jumpVol->value()));

   boost::shared_ptr<PricingEngine> batesEngine(
       new BatesEngine(batesModel));

    // set-up the merton engine
    boost::shared_ptr<VanillaOption::engine> baseEngine(
        new AnalyticEuropeanEngine);
    boost::shared_ptr<PricingEngine> mertonEngine(
        new JumpDiffusionEngine(baseEngine, 1e-10, 1000));

    Real tolerance = 1e-6;
    // compare Merton76 and Bates engine
    for (Size i = 0; i < options.size(); ++i) {
        options[i]->setPricingEngine(batesEngine);
        merton76options[i]->setPricingEngine(mertonEngine);

        Real calculated = options[i]->modelValue();
        Real expected   = merton76options[i]->NPV();

        if (std::fabs(calculated-expected)/expected > tolerance)
            BOOST_FAIL("failed to reproduce Merton76 price with " \
                       "BatesEngine"
                       << "\n    calculated: " << calculated
                       << "\n    expected:   " << expected);
    }

    // check calibration engine

    Simplex om(0.6, 1e-7);
    om.setEndCriteria(EndCriteria(1000, 1e-6));
    batesModel->calibrate(options, om);

    Real expected = 36.6;
    Real calculated = getCalibrationError(options);

    if (std::fabs(calculated - expected) > 2.5)
        BOOST_FAIL("failed to calibrate the bates model"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected);

    //check pricing of derived engines
    std::vector<boost::shared_ptr<PricingEngine> > pricingEngines;

    pricingEngines.push_back(boost::shared_ptr<PricingEngine>(
        new BatesDetJumpEngine(
            boost::shared_ptr<BatesDetJumpModel>(
                new BatesDetJumpModel(process, 1, -0.1)))) );

    pricingEngines.push_back(boost::shared_ptr<PricingEngine>(
        new BatesDoubleExpEngine(
            boost::shared_ptr<BatesDoubleExpModel>(
                new BatesDoubleExpModel(process, 1.0)))) );

    pricingEngines.push_back(boost::shared_ptr<PricingEngine>(
        new BatesDoubleExpDetJumpEngine(
            boost::shared_ptr<BatesDoubleExpDetJumpModel>(
                new BatesDoubleExpDetJumpModel(process, 1.0)))) );

    Real expectedValues[] = { 5127.9866758,
                              4947.3386181,
                              5529.0099857};

    tolerance=0.1;
    for (Size i = 0; i < pricingEngines.size(); ++i) {
        for (Size j = 0; j < options.size(); ++j) {
            options[j]->setPricingEngine(pricingEngines[i]);
        }

        Real calculated = std::fabs(getCalibrationError(options));
        if (std::fabs(calculated - expectedValues[i]) > tolerance)
            BOOST_FAIL("failed to calculated prices for derived bates models"
                       << "\n    calculated: " << calculated
                       << "\n    expected:   " << expectedValues[i]);
    }


    QL_TEST_TEARDOWN
}

test_suite* BatesModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Bates model tests");
    suite->add(BOOST_TEST_CASE(&BatesModelTest::testAnalyticVsBlack));
    suite->add(BOOST_TEST_CASE(&BatesModelTest::testAnalyticVsJumpDiffusion));
    suite->add(BOOST_TEST_CASE(&BatesModelTest::testDAXCalibration));

    return suite;
}

