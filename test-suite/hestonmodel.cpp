/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007, 2009, 2010, 2012, 2014, 2017 Klaus Spanderen
 Copyright (C) 2022 Ignacio Anguita

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

#include "hestonmodel.hpp"
#include "utilities.hpp"
#include <ql/experimental/exoticoptions/analyticpdfhestonengine.hpp>
#include <ql/instruments/dividendbarrieroption.hpp>
#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/optimization/differentialevolution.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/functional.hpp>
#include <ql/methods/finitedifferences/operators/numericaldifferentiation.hpp>
#include <ql/methods/montecarlo/pathgenerator.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/models/equity/hestonmodelhelper.hpp>
#include <ql/models/equity/piecewisetimedependenthestonmodel.hpp>
#include <ql/pricingengines/barrier/fdblackscholesbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdhestonbarrierengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/vanilla/analyticdividendeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/analyticptdhestonengine.hpp>
#include <ql/pricingengines/vanilla/coshestonengine.hpp>
#include <ql/pricingengines/vanilla/exponentialfittinghestonengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <ql/pricingengines/vanilla/hestonexpansionengine.hpp>
#include <ql/pricingengines/vanilla/mceuropeanhestonengine.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/hestonblackvolsurface.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/period.hpp>
#include <cmath>
#include <utility>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct CalibrationMarketData {
        Handle<Quote> s0;
        Handle<YieldTermStructure> riskFreeTS, dividendYield;
        std::vector<ext::shared_ptr<CalibrationHelper> > options;
    };

    CalibrationMarketData getDAXCalibrationMarketData() {
        /* this example is taken from A. Sepp
           Pricing European-Style Options under Jump Diffusion Processes
           with Stochstic Volatility: Applications of Fourier Transform
           http://math.ut.ee/~spartak/papers/stochjumpvols.pdf
        */

        Date settlementDate(Settings::instance().evaluationDate());
        
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
            ext::make_shared<ZeroCurve>(dates, rates, dayCounter));
        
        Handle<YieldTermStructure> dividendYield(
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
        
        Handle<Quote> s0(ext::make_shared<SimpleQuote>(4468.17));
        Real strike[] = { 3400,3600,3800,4000,4200,4400,
                          4500,4600,4800,5000,5200,5400,5600 };
        
        std::vector<ext::shared_ptr<CalibrationHelper> > options;
        
        for (Size s = 0; s < 13; ++s) {
            for (Size m = 0; m < 8; ++m) {
                Handle<Quote> vol(ext::make_shared<SimpleQuote>(v[s*8+m]));
        
                Period maturity((int)((t[m]+3)/7.), Weeks); // round to weeks
                options.push_back(ext::make_shared<HestonModelHelper>(maturity, calendar,
                                              s0, strike[s], vol,
                                              riskFreeTS, dividendYield,
                                          BlackCalibrationHelper::ImpliedVolError));
            }
        }
        
        CalibrationMarketData marketData = { s0, riskFreeTS, dividendYield, options };
        
        return marketData;
    }
        
}


void HestonModelTest::testBlackCalibration() {
    BOOST_TEST_MESSAGE(
       "Testing Heston model calibration using a flat volatility surface...");

    /* calibrate a Heston model to a constant volatility surface without
       smile. expected result is a vanishing volatility of the volatility.
       In addition theta and v0 should be equal to the constant variance */

    Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    DayCounter dayCounter = Actual360();
    Calendar calendar = NullCalendar();

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.04, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.50, dayCounter));

    std::vector<Period> optionMaturities = {1 * Months, 2 * Months, 3 * Months, 6 * Months,
                                            9 * Months, 1 * Years,  2 * Years};

    std::vector<ext::shared_ptr<CalibrationHelper> > options;
    Handle<Quote> s0(ext::make_shared<SimpleQuote>(1.0));
    Handle<Quote> vol(ext::make_shared<SimpleQuote>(0.1));
    Volatility volatility = vol->value();

    for (auto& optionMaturitie : optionMaturities) {
        for (Real moneyness = -1.0; moneyness < 2.0; moneyness += 1.0) {
            const Time tau = dayCounter.yearFraction(
                riskFreeTS->referenceDate(),
                calendar.advance(riskFreeTS->referenceDate(), optionMaturitie));
            const Real fwdPrice =
                s0->value() * dividendTS->discount(tau) / riskFreeTS->discount(tau);
            const Real strikePrice = fwdPrice * std::exp(-moneyness * volatility * std::sqrt(tau));

            options.push_back(ext::make_shared<HestonModelHelper>(
                optionMaturitie, calendar, s0, strikePrice, vol, riskFreeTS, dividendTS));
        }
    }

    for (Real sigma = 0.1; sigma < 0.7; sigma += 0.2) {
        const Real v0=0.01;
        const Real kappa=0.2;
        const Real theta=0.02;
        const Real rho=-0.75;

        ext::shared_ptr<HestonProcess> process(
            ext::make_shared<HestonProcess>(riskFreeTS, dividendTS,
                              s0, v0, kappa, theta, sigma, rho));

        ext::shared_ptr<HestonModel> model(ext::make_shared<HestonModel>(process));
        ext::shared_ptr<PricingEngine> engine(
            ext::make_shared<AnalyticHestonEngine>(model, 96));

        for (auto& option : options)
            ext::dynamic_pointer_cast<BlackCalibrationHelper>(option)->setPricingEngine(engine);

        LevenbergMarquardt om(1e-8, 1e-8, 1e-8);
        model->calibrate(options, om, EndCriteria(400, 40, 1.0e-8,
                                                  1.0e-8, 1.0e-8));

        Real tolerance = 3.0e-3;

        if (model->sigma() > tolerance) {
            BOOST_ERROR("Failed to reproduce expected sigma"
                        << "\n    calculated: " << model->sigma()
                        << "\n    expected:   " << 0.0
                        << "\n    tolerance:  " << tolerance);
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
}


void HestonModelTest::testDAXCalibration() {

    BOOST_TEST_MESSAGE(
             "Testing Heston model calibration using DAX volatility data...");

    Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;

    CalibrationMarketData marketData = getDAXCalibrationMarketData();
    
    const Handle<YieldTermStructure> riskFreeTS = marketData.riskFreeTS;
    const Handle<YieldTermStructure> dividendTS = marketData.dividendYield;
    const Handle<Quote> s0 = marketData.s0;

    const std::vector<ext::shared_ptr<CalibrationHelper> >& options = marketData.options;

    const Real v0=0.1;
    const Real kappa=1.0;
    const Real theta=0.1;
    const Real sigma=0.5;
    const Real rho=-0.5;

    const ext::shared_ptr<HestonProcess> process(
        ext::make_shared<HestonProcess>(
            riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

    const ext::shared_ptr<HestonModel> model(
        ext::make_shared<HestonModel>(process));

    const ext::shared_ptr<PricingEngine> engines[] = {
        ext::make_shared<AnalyticHestonEngine>(model, 64),
        ext::make_shared<COSHestonEngine>(model, 12, 75),
        ext::make_shared<ExponentialFittingHestonEngine>(model)
    };

    const Array params = model->params();
    for (const auto& engine : engines) {
        model->setParams(params);
        for (const auto& option : options)
            ext::dynamic_pointer_cast<BlackCalibrationHelper>(option)->setPricingEngine(engine);

        LevenbergMarquardt om(1e-8, 1e-8, 1e-8);
        model->calibrate(options, om,
                         EndCriteria(400, 40, 1.0e-8, 1.0e-8, 1.0e-8));

        Real sse = 0;
        for (Size i = 0; i < 13*8; ++i) {
            const Real diff = options[i]->calibrationError()*100.0;
            sse += diff*diff;
        }
        Real expected = 177.2; //see article by A. Sepp.
        if (std::fabs(sse - expected) > 1.0) {
            BOOST_FAIL("Failed to reproduce calibration error"
                       << "\n    calculated: " << sse
                       << "\n    expected:   " << expected);
        }
    }
}

void HestonModelTest::testAnalyticVsBlack() {
    BOOST_TEST_MESSAGE("Testing analytic Heston engine against Black formula...");

    Date settlementDate = Date::todaysDate();
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual(ActualActual::ISDA);
    Date exerciseDate = settlementDate + 6*Months;

    ext::shared_ptr<StrikedTypePayoff> payoff(
        ext::make_shared<PlainVanillaPayoff>(Option::Put, 30));
    ext::shared_ptr<Exercise> exercise(
        ext::make_shared<EuropeanExercise>(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.1, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.04, dayCounter));

    Handle<Quote> s0(ext::make_shared<SimpleQuote>(32.0));

    const Real v0=0.05;
    const Real kappa=5.0;
    const Real theta=0.05;
    const Real sigma=1.0e-4;
    const Real rho=0.0;

    ext::shared_ptr<HestonProcess> process(ext::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

    VanillaOption option(payoff, exercise);
    ext::shared_ptr<PricingEngine> engine(
        ext::make_shared<AnalyticHestonEngine>(
            ext::make_shared<HestonModel>(process), 144));

    option.setPricingEngine(engine);
    Real calculated = option.NPV();

    Real yearFraction = dayCounter.yearFraction(settlementDate, exerciseDate);
    Real forwardPrice = 32*std::exp((0.1-0.04)*yearFraction);
    Real expected = blackFormula(payoff->optionType(), payoff->strike(),
        forwardPrice, std::sqrt(0.05*yearFraction)) *
                                            std::exp(-0.1*yearFraction);
    Real error = std::fabs(calculated - expected);
    Real tolerance = 2.0e-7;
    if (error > tolerance) {
        BOOST_FAIL("failed to reproduce Black price with AnalyticHestonEngine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }

    engine = 
        ext::make_shared<FdHestonVanillaEngine>(
            ext::make_shared<HestonModel>(process),
              200,200,100);
    option.setPricingEngine(engine);

    calculated = option.NPV();
    error = std::fabs(calculated - expected);
    tolerance = 1.0e-3;
    if (error > tolerance) {
        BOOST_FAIL("failed to reproduce Black price with FdHestonVanillaEngine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }

}


void HestonModelTest::testAnalyticVsCached() {
    BOOST_TEST_MESSAGE("Testing analytic Heston engine against cached values...");

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual(ActualActual::ISDA);
    Date exerciseDate(28, March, 2005);

    ext::shared_ptr<StrikedTypePayoff> payoff(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, 1.05));
    ext::shared_ptr<Exercise> exercise(
        ext::make_shared<EuropeanExercise>(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.0225, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

    Handle<Quote> s0(ext::make_shared<SimpleQuote>(1.0));
    const Real v0 = 0.1;
    const Real kappa = 3.16;
    const Real theta = 0.09;
    const Real sigma = 0.4;
    const Real rho = -0.2;

    ext::shared_ptr<HestonProcess> process(ext::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

    VanillaOption option(payoff, exercise);

    ext::shared_ptr<AnalyticHestonEngine> engine(
        ext::make_shared<AnalyticHestonEngine>(
            ext::make_shared<HestonModel>(process), 64));

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

        ext::shared_ptr<StrikedTypePayoff> payoff(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, K[i%3]));
        ext::shared_ptr<Exercise> exercise(
            ext::make_shared<EuropeanExercise>(exerciseDate));

        Handle<YieldTermStructure> riskFreeTS(flatRate(0.05, dayCounter));
        Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

        Real s = riskFreeTS->discount(0.7)/dividendTS->discount(0.7);
        Handle<Quote> s0(ext::make_shared<SimpleQuote>(s));

        ext::shared_ptr<HestonProcess> process(
            ext::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, 0.09, 1.2, 0.08, 1.8, -0.45));

        VanillaOption option(payoff, exercise);

        ext::shared_ptr<PricingEngine> engine(
            ext::make_shared<AnalyticHestonEngine>(
                ext::make_shared<HestonModel>(process)));

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
}


void HestonModelTest::testMcVsCached() {
    BOOST_TEST_MESSAGE(
                "Testing Monte Carlo Heston engine against cached values...");

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual(ActualActual::ISDA);
    Date exerciseDate(28, March, 2005);

    ext::shared_ptr<StrikedTypePayoff> payoff(
        ext::make_shared<PlainVanillaPayoff>(Option::Put, 1.05));
    ext::shared_ptr<Exercise> exercise(
        ext::make_shared<EuropeanExercise>(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.7, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.4, dayCounter));

    Handle<Quote> s0(ext::make_shared<SimpleQuote>(1.05));

    ext::shared_ptr<HestonProcess> process(
        ext::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, 0.3, 1.16, 0.2, 0.8, 0.8,
                   HestonProcess::QuadraticExponentialMartingale));

    VanillaOption option(payoff, exercise);

    ext::shared_ptr<PricingEngine> engine;
    engine = MakeMCEuropeanHestonEngine<PseudoRandom>(process)
        .withStepsPerYear(11)
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
}

void HestonModelTest::testFdBarrierVsCached() {
    BOOST_TEST_MESSAGE("Testing FD barrier Heston engine against cached values...");

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));
    Handle<YieldTermStructure> rTS(flatRate(today, 0.08, dc));
    Handle<YieldTermStructure> qTS(flatRate(today, 0.04, dc));

    Date exDate = today + 180;
    ext::shared_ptr<Exercise> exercise(
        ext::make_shared<EuropeanExercise>(exDate));

    ext::shared_ptr<StrikedTypePayoff> payoff(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, 90.0));

    ext::shared_ptr<HestonProcess> process(
        ext::make_shared<HestonProcess>(
            rTS, qTS, s0, 0.25*0.25, 1.0, 0.25*0.25, 0.001, 0.0));

    ext::shared_ptr<PricingEngine> engine;
    engine = ext::make_shared<FdHestonBarrierEngine>(
                ext::make_shared<HestonModel>(process),
                    200,400,100);

    BarrierOption option(Barrier::DownOut, 95.0, 3.0, payoff, exercise);
    option.setPricingEngine(engine);

    Real calculated = option.NPV();
    Real expected = 9.0246;
    Real error = std::fabs(calculated-expected);
    if (error > 1.0e-3) {
        BOOST_FAIL("failed to reproduce cached price with FD Barrier engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }

    option = BarrierOption(Barrier::DownIn, 95.0, 3.0, payoff, exercise);
    option.setPricingEngine(engine);

    calculated = option.NPV();
    expected = 7.7627;
    error = std::fabs(calculated-expected);
    if (error > 1.0e-3) {
        BOOST_FAIL("failed to reproduce cached price with FD Barrier engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }
}

void HestonModelTest::testFdVanillaVsCached() {
    BOOST_TEST_MESSAGE("Testing FD vanilla Heston engine against cached values...");

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual(ActualActual::ISDA);
    Date exerciseDate(28, March, 2005);

    ext::shared_ptr<StrikedTypePayoff> payoff(
        ext::make_shared<PlainVanillaPayoff>(Option::Put, 1.05));
    ext::shared_ptr<Exercise> exercise(
        ext::make_shared<EuropeanExercise>(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.7, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.4, dayCounter));

    Handle<Quote> s0(ext::make_shared<SimpleQuote>(1.05));

    VanillaOption option(payoff, exercise);

    ext::shared_ptr<HestonProcess> process(
        ext::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, 0.3, 1.16, 0.2, 0.8, 0.8));

    option.setPricingEngine(
        MakeFdHestonVanillaEngine(ext::make_shared<HestonModel>(process))
            .withTGrid(100)
            .withXGrid(200)
            .withVGrid(100)
        );

    Real expected = 0.06325;
    Real calculated = option.NPV();
    Real error = std::fabs(calculated - expected);
    Real tolerance = 1.0e-4;

    if (error > tolerance) {
        BOOST_FAIL("failed to reproduce cached price with FD engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }
}

void HestonModelTest::testFdVanillaWithDividendsVsCached() {
    BOOST_TEST_MESSAGE("Testing FD vanilla Heston engine for discrete dividends...");

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual(ActualActual::ISDA);

    auto payoff = ext::make_shared<PlainVanillaPayoff>(Option::Call, 95.0);

    auto s0 = Handle<Quote>(ext::make_shared<SimpleQuote>(100.0));
    auto riskFreeTS = Handle<YieldTermStructure>(flatRate(0.05, dayCounter));
    auto dividendTS = Handle<YieldTermStructure>(flatRate(0.0, dayCounter));

    auto exerciseDate = Date(28, March, 2006);
    auto exercise = ext::make_shared<EuropeanExercise>(exerciseDate);

    std::vector<Date> dividendDates;
    std::vector<Real> dividends;
    for (Date d = settlementDate + 3*Months;
              d < exercise->lastDate();
              d += 6*Months) {
        dividendDates.push_back(d);
        dividends.push_back(1.0);
    }

    QL_DEPRECATED_DISABLE_WARNING
    DividendVanillaOption divOption(payoff, exercise,
                                    dividendDates, dividends);
    QL_DEPRECATED_ENABLE_WARNING
    auto process = ext::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, 0.04, 1.0, 0.04, 0.001, 0.0);
    divOption.setPricingEngine(
        MakeFdHestonVanillaEngine(ext::make_shared<HestonModel>(process))
            .withTGrid(200)
            .withXGrid(400)
            .withVGrid(100)
        );

    Real calculated = divOption.NPV();
    // Value calculated with an independent FD framework, validated with
    // an independent MC framework
    Real expected = 12.946;
    Real error = std::fabs(calculated - expected);
    Real tolerance = 5.0e-3;

    if (error > tolerance) {
        BOOST_FAIL("failed to reproduce discrete dividend price with FD engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }


    VanillaOption option(payoff, exercise);
    option.setPricingEngine(
        MakeFdHestonVanillaEngine(ext::make_shared<HestonModel>(process))
        .withCashDividends(dividendDates, dividends)
        .withTGrid(200)
        .withXGrid(400)
        .withVGrid(100)
    );

    calculated = option.NPV();
    error = std::fabs(calculated - expected);

    if (error > tolerance) {
        BOOST_FAIL("failed to reproduce discrete dividend price with FD engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }
}

void HestonModelTest::testFdAmerican() {
    BOOST_TEST_MESSAGE("Testing FD vanilla Heston engine for american exercise...");

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual(ActualActual::ISDA);

    auto s0 = Handle<Quote>(ext::make_shared<SimpleQuote>(100.0));
    auto riskFreeTS = Handle<YieldTermStructure>(flatRate(0.05, dayCounter));
    auto dividendTS = Handle<YieldTermStructure>(flatRate(0.03, dayCounter));
    auto process = ext::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, 0.04, 1.0, 0.04, 0.001, 0.0);
    auto payoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, 95.0);
    auto exerciseDate = Date(28, March, 2006);
    auto exercise = ext::make_shared<AmericanExercise>(
            settlementDate, exerciseDate);
    auto option = VanillaOption(payoff, exercise);
    option.setPricingEngine(
        MakeFdHestonVanillaEngine(ext::make_shared<HestonModel>(process))
            .withTGrid(200)
            .withXGrid(400)
            .withVGrid(100)
        );
    Real calculated = option.NPV();

    Handle<BlackVolTermStructure> volTS(flatVol(settlementDate, 0.2,
                                                  dayCounter));
    ext::shared_ptr<BlackScholesMertonProcess> ref_process(
        ext::make_shared<BlackScholesMertonProcess>(s0, dividendTS, riskFreeTS, volTS));
    ext::shared_ptr<PricingEngine> ref_engine(
        ext::make_shared<FdBlackScholesVanillaEngine>(ref_process, 200, 400));
    option.setPricingEngine(ref_engine);
    Real expected = option.NPV();

    Real error = std::fabs(calculated - expected);
    Real tolerance = 1.0e-3;

    if (error > tolerance) {
        BOOST_FAIL("failed to reproduce american option price with FD engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }
}

namespace {
    struct HestonProcessDiscretizationDesc {
        HestonProcess::Discretization discretization;
        Size nSteps;
        std::string name;
    };
}

void HestonModelTest::testKahlJaeckelCase() {
    BOOST_TEST_MESSAGE(
          "Testing MC and FD Heston engines for the Kahl-Jaeckel example...");

    /* Example taken from Wilmott mag (Sept. 2005).
       "Not-so-complex logarithms in the Heston model",
       Example was also discussed within the Wilmott thread
       "QuantLib code is very high quatlity"
    */

    Date settlementDate(30, March, 2007);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual(ActualActual::ISDA);
    Date exerciseDate(30, March, 2017);

    const ext::shared_ptr<StrikedTypePayoff> payoff(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, 200));
    const ext::shared_ptr<Exercise> exercise(
        ext::make_shared<EuropeanExercise>(exerciseDate));

    VanillaOption option(payoff, exercise);


    Handle<YieldTermStructure> riskFreeTS(flatRate(0.0, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.0, dayCounter));

    Handle<Quote> s0(ext::make_shared<SimpleQuote>(100));

    const Real v0    = 0.16;
    const Real theta = v0;
    const Real kappa = 1.0;
    const Real sigma = 2.0;
    const Real rho   =-0.8;


    const HestonProcessDiscretizationDesc descriptions[] = {
        { HestonProcess::NonCentralChiSquareVariance, 10,
          "NonCentralChiSquareVariance" },
        { HestonProcess::QuadraticExponentialMartingale, 100,
          "QuadraticExponentialMartingale" },
    };

    const Real tolerance = 0.2;
    const Real expected = 4.95212;

    for (const auto& description : descriptions) {
        const ext::shared_ptr<HestonProcess> process(ext::make_shared<HestonProcess>(
            riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho, description.discretization));

        const ext::shared_ptr<PricingEngine> engine =
            MakeMCEuropeanHestonEngine<PseudoRandom>(process)
                .withSteps(description.nSteps)
                .withAntitheticVariate()
                .withAbsoluteTolerance(tolerance)
                .withSeed(1234);
        option.setPricingEngine(engine);

        const Real calculated = option.NPV();
        const Real errorEstimate = option.errorEstimate();

        if (std::fabs(calculated - expected) > 2.34*errorEstimate) {
            BOOST_ERROR("Failed to reproduce cached price with MC engine"
                        << "\n    discretization: " << description.name
                        << "\n    expected:       " << expected
                        << "\n    calculated:     " << calculated << " +/- " << errorEstimate);
        }

        if (errorEstimate > tolerance) {
            BOOST_ERROR("failed to reproduce error estimate with MC engine"
                        << "\n    discretization: " << description.name << "\n    calculated    : "
                        << errorEstimate << "\n    expected      :   " << tolerance);
        }
    }

    option.setPricingEngine(
        MakeMCEuropeanHestonEngine<LowDiscrepancy>(
            ext::make_shared<HestonProcess>(
                    riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho,
                    HestonProcess::BroadieKayaExactSchemeLaguerre))
        .withSteps(1)
        .withSamples(1023));

    Real calculated = option.NPV();
    if (std::fabs(calculated - expected) > 0.5*tolerance) {
        BOOST_ERROR("Failed to reproduce cached price with MC engine"
                    << "\n    discretization: BroadieKayaExactSchemeLobatto"
                    << "\n    calculated:     " << calculated
                    << "\n    expected:       " << expected
                    << "\n    tolerance:      " << tolerance);
    }


    const ext::shared_ptr<HestonModel> hestonModel(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                riskFreeTS, dividendTS, s0, v0,
                kappa, theta, sigma, rho)));

    option.setPricingEngine(
        ext::make_shared<FdHestonVanillaEngine>(hestonModel, 200, 401, 101));

    calculated = option.NPV();
    Real error = std::fabs(calculated - expected);
    if (error > 5.0e-2) {
        BOOST_FAIL("failed to reproduce cached price with FD engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }

    option.setPricingEngine(
        ext::make_shared<AnalyticHestonEngine>(hestonModel, 1e-6, 1000));

    calculated = option.NPV();
    error = std::fabs(calculated - expected);

    if (error > 0.00002) {
        BOOST_FAIL("failed to reproduce cached price with "
                   "GaussLobatto engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }

    option.setPricingEngine(
        ext::make_shared<COSHestonEngine>(hestonModel, 16, 400));
    calculated = option.NPV();
    error = std::fabs(calculated - expected);

    if (error > 0.00002) {
        BOOST_FAIL("failed to reproduce cached price with "
                   "Cosine engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }

    option.setPricingEngine(
        ext::make_shared<ExponentialFittingHestonEngine>(hestonModel));
    calculated = option.NPV();
    error = std::fabs(calculated - expected);

    if (error > 0.00002) {
        BOOST_FAIL("failed to reproduce cached price with "
                   "exponential fitting Heston engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << std::scientific << error);
    }
}

namespace {
    struct HestonParameter {
        Real v0, kappa, theta, sigma, rho; };
}

void HestonModelTest::testDifferentIntegrals() {
    BOOST_TEST_MESSAGE(
       "Testing different numerical Heston integration algorithms...");

    const Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = ActualActual(ActualActual::ISDA);

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.05, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.03, dayCounter));

    const Real strikes[] = { 0.5, 0.7, 1.0, 1.25, 1.5, 2.0 };
    const Integer maturities[] = { 1, 2, 3, 12, 60, 120, 360};
    const Option::Type types[] ={ Option::Put, Option::Call };

    const HestonParameter equityfx      = { 0.07, 2.0, 0.04, 0.55, -0.8 };
    const HestonParameter highCorr      = { 0.07, 1.0, 0.04, 0.55,  0.995 };
    const HestonParameter lowVolOfVol   = { 0.07, 1.0, 0.04, 0.025, -0.75 };
    const HestonParameter highVolOfVol  = { 0.07, 1.0, 0.04, 5.0, -0.75 };
    const HestonParameter kappaEqSigRho = { 0.07, 0.4, 0.04, 0.5, 0.8 };

    std::vector<HestonParameter> params = {
        equityfx,
        highCorr,
        lowVolOfVol,
        highVolOfVol,
        kappaEqSigRho
    };
    const Real tol[] = { 1e-3, 1e-3, 0.2, 0.01, 1e-3 };

    for (std::vector<HestonParameter>::const_iterator iter = params.begin();
         iter != params.end(); ++iter) {

        Handle<Quote> s0(ext::make_shared<SimpleQuote>(1.0));
        ext::shared_ptr<HestonProcess> process(
            ext::make_shared<HestonProcess>(
            riskFreeTS, dividendTS,
            s0, iter->v0, iter->kappa,
            iter->theta, iter->sigma, iter->rho));

        ext::shared_ptr<HestonModel> model(
            ext::make_shared<HestonModel>(process));

        ext::shared_ptr<AnalyticHestonEngine> lobattoEngine(
            ext::make_shared<AnalyticHestonEngine>(model, 1e-10,
                                                       1000000));
        ext::shared_ptr<AnalyticHestonEngine> laguerreEngine(
            ext::make_shared<AnalyticHestonEngine>(model, 128));
        ext::shared_ptr<AnalyticHestonEngine> legendreEngine(
            ext::make_shared<AnalyticHestonEngine>(
                model, AnalyticHestonEngine::Gatheral,
                AnalyticHestonEngine::Integration::gaussLegendre(512)));
        ext::shared_ptr<AnalyticHestonEngine> chebyshevEngine(
            ext::make_shared<AnalyticHestonEngine>(
                model, AnalyticHestonEngine::Gatheral,
                AnalyticHestonEngine::Integration::gaussChebyshev(512)));
        ext::shared_ptr<AnalyticHestonEngine> chebyshev2ndEngine(
            ext::make_shared<AnalyticHestonEngine>(
                model, AnalyticHestonEngine::Gatheral,
                AnalyticHestonEngine::Integration::gaussChebyshev2nd(512)));

        Real maxLegendreDiff    = 0.0;
        Real maxChebyshevDiff   = 0.0;
        Real maxChebyshev2ndDiff= 0.0;
        Real maxLaguerreDiff    = 0.0;

        for (int maturitie : maturities) {
            ext::shared_ptr<Exercise> exercise(
                ext::make_shared<EuropeanExercise>(settlementDate + Period(maturitie, Months)));

            for (Real strike : strikes) {
                for (auto type : types) {

                    ext::shared_ptr<StrikedTypePayoff> payoff(
                        ext::make_shared<PlainVanillaPayoff>(type, strike));

                    VanillaOption option(payoff, exercise);

                    option.setPricingEngine(lobattoEngine);
                    const Real lobattoNPV = option.NPV();

                    option.setPricingEngine(laguerreEngine);
                    const Real laguerre = option.NPV();

                    option.setPricingEngine(legendreEngine);
                    const Real legendre = option.NPV();

                    option.setPricingEngine(chebyshevEngine);
                    const Real chebyshev = option.NPV();

                    option.setPricingEngine(chebyshev2ndEngine);
                    const Real chebyshev2nd = option.NPV();

                    maxLaguerreDiff
                        = std::max(maxLaguerreDiff,
                                   std::fabs(lobattoNPV-laguerre));
                    maxLegendreDiff
                        = std::max(maxLegendreDiff,
                                   std::fabs(lobattoNPV-legendre));
                    maxChebyshevDiff
                        = std::max(maxChebyshevDiff,
                                   std::fabs(lobattoNPV-chebyshev));
                    maxChebyshev2ndDiff
                        = std::max(maxChebyshev2ndDiff,
                                   std::fabs(lobattoNPV-chebyshev2nd));
                }
            }
        }
        const Real maxDiff = std::max(std::max(
            std::max(maxLaguerreDiff,maxLegendreDiff),
                                     maxChebyshevDiff), maxChebyshev2ndDiff);

        const Real tr = tol[iter - params.begin()];
        if (maxDiff > tr) {
            BOOST_ERROR("Failed to reproduce Heston pricing values "
                        "within given tolerance"
                        << "\n    maxDifference: " << maxDiff
                        << "\n    tolerance:     " << tr);
        }
    }
}

void HestonModelTest::testMultipleStrikesEngine() {
    BOOST_TEST_MESSAGE("Testing multiple-strikes FD Heston engine...");

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual(ActualActual::ISDA);
    Date exerciseDate(28, March, 2006);

    ext::shared_ptr<Exercise> exercise(
        ext::make_shared<EuropeanExercise>(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.06, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

    Handle<Quote> s0(ext::make_shared<SimpleQuote>(1.05));

    ext::shared_ptr<HestonProcess> process(
        ext::make_shared<HestonProcess>(
                     riskFreeTS, dividendTS, s0, 0.16, 2.5, 0.09, 0.8, -0.8));
    ext::shared_ptr<HestonModel> model(
		ext::make_shared<HestonModel>(process));

    std::vector<Real> strikes = {1.0, 0.5, 0.75, 1.5, 2.0};

    ext::shared_ptr<FdHestonVanillaEngine> singleStrikeEngine(
		ext::make_shared<FdHestonVanillaEngine>(model, 20, 400, 50));
    ext::shared_ptr<FdHestonVanillaEngine> multiStrikeEngine(
        ext::make_shared<FdHestonVanillaEngine>(model, 20, 400, 50));
    multiStrikeEngine->enableMultipleStrikesCaching(strikes);

    Real relTol = 5e-3;
    for (Real& strike : strikes) {
        ext::shared_ptr<StrikedTypePayoff> payoff(
            ext::make_shared<PlainVanillaPayoff>(Option::Put, strike));

        VanillaOption aOption(payoff, exercise);
        aOption.setPricingEngine(multiStrikeEngine);

        Real npvCalculated   = aOption.NPV();
        Real deltaCalculated = aOption.delta();
        Real gammaCalculated = aOption.gamma();
        Real thetaCalculated = aOption.theta();

        aOption.setPricingEngine(singleStrikeEngine);
        Real npvExpected   = aOption.NPV();
        Real deltaExpected = aOption.delta();
        Real gammaExpected = aOption.gamma();
        Real thetaExpected = aOption.theta();

        if (std::fabs(npvCalculated-npvExpected)/npvExpected > relTol) {
            BOOST_FAIL("failed to reproduce price with FD multi strike engine"
                       << "\n    calculated: " << npvCalculated
                       << "\n    expected:   " << npvExpected
                       << "\n    error:      " << std::scientific << relTol);
        }
        if (std::fabs(deltaCalculated-deltaExpected)/deltaExpected > relTol) {
            BOOST_FAIL("failed to reproduce delta with FD multi strike engine"
                       << "\n    calculated: " << deltaCalculated
                       << "\n    expected:   " << deltaExpected
                       << "\n    error:      " << std::scientific << relTol);
        }
        if (std::fabs(gammaCalculated-gammaExpected)/gammaExpected > relTol) {
            BOOST_FAIL("failed to reproduce gamma with FD multi strike engine"
                       << "\n    calculated: " << gammaCalculated
                       << "\n    expected:   " << gammaExpected
                       << "\n    error:      " << std::scientific << relTol);
        }
        if (std::fabs(thetaCalculated-thetaExpected)/thetaExpected > relTol) {
            BOOST_FAIL("failed to reproduce theta with FD multi strike engine"
                       << "\n    calculated: " << thetaCalculated
                       << "\n    expected:   " << thetaExpected
                       << "\n    error:      " << std::scientific << relTol);
        }
    }
}



void HestonModelTest::testAnalyticPiecewiseTimeDependent() {
    BOOST_TEST_MESSAGE("Testing analytic piecewise time dependent Heston prices...");

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual(ActualActual::ISDA);
    Date exerciseDate(28, March, 2005);

    ext::shared_ptr<StrikedTypePayoff> payoff(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, 1.0));
    ext::shared_ptr<Exercise> exercise(
		ext::make_shared<EuropeanExercise>(exerciseDate));

    std::vector<Date> dates = {settlementDate, {1, January, 2007}};
    std::vector<Rate> irates = {0.0, 0.2};
    Handle<YieldTermStructure> riskFreeTS(
		ext::make_shared<ZeroCurve>(dates, irates, dayCounter));

    std::vector<Rate> qrates = {0.0, 0.3};
    Handle<YieldTermStructure> dividendTS(
		ext::make_shared<ZeroCurve>(dates, qrates, dayCounter));
    
    const Real v0 = 0.1;
    Handle<Quote> s0(ext::make_shared<SimpleQuote>(1.0));

    ConstantParameter theta(0.09, PositiveConstraint());
    ConstantParameter kappa(3.16, PositiveConstraint());
    ConstantParameter sigma(4.40, PositiveConstraint());
    ConstantParameter rho  (-0.8, BoundaryConstraint(-1.0, 1.0));

    ext::shared_ptr<PiecewiseTimeDependentHestonModel> model =
        ext::make_shared<PiecewiseTimeDependentHestonModel>(
                                              riskFreeTS, dividendTS,
                                              s0, v0, theta, kappa, 
                                              sigma, rho, TimeGrid(20.0, 2));
    
    VanillaOption option(payoff, exercise);

    ext::shared_ptr<HestonProcess> hestonProcess(
        ext::make_shared<HestonProcess>(
                          riskFreeTS, dividendTS, s0, v0,
                          kappa(0.0), theta(0.0), sigma(0.0), rho(0.0)));
    ext::shared_ptr<HestonModel> hestonModel =
        ext::make_shared<HestonModel>(hestonProcess);
    option.setPricingEngine(
        ext::make_shared<AnalyticHestonEngine>(hestonModel));
    
    const Real expected = option.NPV();

    option.setPricingEngine(ext::shared_ptr<PricingEngine>(
         new AnalyticPTDHestonEngine(model)));

    const Real calculatedGatheral = option.NPV();
    if (std::fabs(calculatedGatheral-expected) > 1e-12) {
        BOOST_ERROR("failed to reproduce Heston prices with Gatheral ChF"
                   << "\n    calculated: " << calculatedGatheral
                   << "\n    expected:   " << expected);
    }

    option.setPricingEngine(ext::shared_ptr<PricingEngine>(
         new AnalyticPTDHestonEngine(
             model,
             AnalyticPTDHestonEngine::AndersenPiterbarg,
             AnalyticPTDHestonEngine::Integration::gaussLaguerre(164))));
    const Real calculatedAndersenPiterbarg = option.NPV();

    if (std::fabs(calculatedAndersenPiterbarg-expected) > 1e-8) {
        BOOST_ERROR("failed to reproduce Heston prices Andersen-Piterbarg"
                   << "\n    calculated: " << calculatedAndersenPiterbarg
                   << "\n    expected:   " << expected);
    }
}

void HestonModelTest::testDAXCalibrationOfTimeDependentModel() {
    BOOST_TEST_MESSAGE(
             "Testing time-dependent Heston model calibration...");

    Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;

    CalibrationMarketData marketData = getDAXCalibrationMarketData();
    
    const Handle<YieldTermStructure> riskFreeTS = marketData.riskFreeTS;
    const Handle<YieldTermStructure> dividendTS = marketData.dividendYield;
    const Handle<Quote> s0 = marketData.s0;

    const std::vector<ext::shared_ptr<CalibrationHelper> >& options = marketData.options;

    std::vector<Time> modelTimes = {0.25, 10.0};
    const TimeGrid modelGrid(modelTimes.begin(), modelTimes.end());

    const Real v0=0.1;
    ConstantParameter sigma( 0.5, PositiveConstraint());
    ConstantParameter theta( 0.1, PositiveConstraint());
    ConstantParameter rho( -0.5, BoundaryConstraint(-1.0, 1.0));
   
    std::vector<Time> pTimes(1, 0.25);
    PiecewiseConstantParameter kappa(pTimes, PositiveConstraint());
    
    for (Size i=0; i < pTimes.size()+1; ++i) {
        kappa.setParam(i, 10.0);
    }

    ext::shared_ptr<PiecewiseTimeDependentHestonModel> model =
        ext::make_shared<PiecewiseTimeDependentHestonModel>(
                                              riskFreeTS, dividendTS,
                                              s0, v0, theta, kappa, 
                                              sigma, rho, modelGrid);

    const ext::shared_ptr<PricingEngine> engines[] = {
        ext::make_shared<AnalyticPTDHestonEngine>(model),
        ext::make_shared<AnalyticPTDHestonEngine>(
            model,
            AnalyticPTDHestonEngine::AndersenPiterbarg,
            AnalyticPTDHestonEngine::Integration::gaussLaguerre(64)),
        ext::make_shared<AnalyticPTDHestonEngine>(
            model,
            AnalyticPTDHestonEngine::AndersenPiterbarg,
            AnalyticPTDHestonEngine::Integration::discreteTrapezoid(72))
    };

    for (const auto& engine : engines) {
        for (const auto& option : options)
            ext::dynamic_pointer_cast<BlackCalibrationHelper>(option)->setPricingEngine(engine);

        LevenbergMarquardt om(1e-8, 1e-8, 1e-8);
        model->calibrate(options, om,
            EndCriteria(400, 40, 1.0e-8, 1.0e-8, 1.0e-8));
    
        Real sse = 0;
        for (Size i = 0; i < 13*8; ++i) {
            const Real diff = options[i]->calibrationError()*100.0;
            sse += diff*diff;
        }

        Real expected = 74.4;
        if (std::fabs(sse - expected) > 1.0) {
            BOOST_ERROR("Failed to reproduce calibration error"
                       << "\n    calculated: " << sse
                       << "\n    expected:   " << expected);
        }
    }
}

void HestonModelTest::testAlanLewisReferencePrices() {
    BOOST_TEST_MESSAGE("Testing Alan Lewis reference prices...");

    /*
     * testing Alan Lewis reference prices posted in
     * http://wilmott.com/messageview.cfm?catid=34&threadid=90957
     */

    const Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;

    const Date maturityDate(5, July, 2003);
    const ext::shared_ptr<Exercise> exercise(
        ext::make_shared<EuropeanExercise>(maturityDate));

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.01, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.04;
    const Real rho   = -0.5;
    const Real sigma =  1.0;
    const Real kappa =  4.0;
    const Real theta =  0.25;

    const ext::shared_ptr<HestonProcess> process(
        ext::make_shared<HestonProcess>(
            riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));
    const ext::shared_ptr<HestonModel> model(
        ext::make_shared<HestonModel>(process));

    const ext::shared_ptr<PricingEngine> laguerreEngine(
        ext::make_shared<AnalyticHestonEngine>(model, 128U));

    const ext::shared_ptr<PricingEngine> gaussLobattoEngine(
        ext::make_shared<AnalyticHestonEngine>(model, QL_EPSILON, 100000U));

    const ext::shared_ptr<PricingEngine> cosEngine(
        ext::make_shared<COSHestonEngine>(model, 20, 400));

    const ext::shared_ptr<PricingEngine> exponentialFittingEngine(
        ext::make_shared<ExponentialFittingHestonEngine>(model));

    const ext::shared_ptr<PricingEngine> andersenPiterbargEngine(
        new AnalyticHestonEngine(
            model,
            AnalyticHestonEngine::AndersenPiterbarg,
            AnalyticHestonEngine::Integration::discreteTrapezoid(92),
            QL_EPSILON));

    const Real strikes[] = { 80, 90, 100, 110, 120 };
    const Option::Type types[] = { Option::Put, Option::Call };
    const ext::shared_ptr<PricingEngine> engines[]
        = { laguerreEngine, gaussLobattoEngine,
            cosEngine, andersenPiterbargEngine, exponentialFittingEngine };

    const Real expectedResults[][2] = {
        { 7.958878113256768285213263077598987193482161301733,
          26.774758743998854221382195325726949201687074848341 },
        { 12.017966707346304987709573290236471654992071308187,
          20.933349000596710388139445766564068085476194042256 },
        { 17.055270961270109413522653999411000974895436309183,
          16.070154917028834278213466703938231827658768230714 },
        { 23.017825898442800538908781834822560777763225722188,
          12.132211516709844867860534767549426052805766831181 },
        { 29.811026202682471843340682293165857439167301370697,
          9.024913483457835636553375454092357136489051667150  }
    };

    const Real tol = 1e-12; // 3e-15 works on linux/ia32,
                            // but keep some buffer for other platforms

    for (Size i=0; i < LENGTH(strikes); ++i) {
        const Real strike = strikes[i];

        for (Size j=0; j < LENGTH(types); ++j) {
            const Option::Type type = types[j];

            for (Size k=0; k < LENGTH(engines); ++k) {
                const ext::shared_ptr<PricingEngine> engine = engines[k];

                const ext::shared_ptr<StrikedTypePayoff> payoff(
                    ext::make_shared<PlainVanillaPayoff>(type, strike));

                VanillaOption option(payoff, exercise);
                option.setPricingEngine(engine);

                const Real expected = expectedResults[i][j];
                const Real calculated = option.NPV();
                const Real relError = std::fabs(calculated-expected)/expected;

                if (relError > tol || std::isnan(calculated)) {
                    BOOST_ERROR(
                           "failed to reproduce Alan Lewis Reference prices "
                        << "\n    strike     : " << strike
                        << "\n    option type: " << type
                        << "\n    engine type: " << k
                        << "\n    rel. error : " << relError);
                }
            }
        }
    }
}

void HestonModelTest::testAnalyticPDFHestonEngine() {
    BOOST_TEST_MESSAGE("Testing analytic PDF Heston engine...");

    const Date settlementDate(5, January, 2014);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.07, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.185, dayCounter));

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.1;
    const Real rho   = -0.5;
    const Real sigma =  1.0;
    const Real kappa =  4.0;
    const Real theta =  0.05;

    const ext::shared_ptr<HestonModel> model(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(riskFreeTS, dividendTS,
                              s0, v0, kappa, theta, sigma, rho)));

    const Real tol = 1e-6;
    const ext::shared_ptr<AnalyticPDFHestonEngine> pdfEngine(
        ext::make_shared<AnalyticPDFHestonEngine>(model, tol));

    const ext::shared_ptr<PricingEngine> analyticEngine(
        ext::make_shared<AnalyticHestonEngine>(model, 178));

    const Date maturityDate(5, July, 2014);
    const Time maturity = dayCounter.yearFraction(settlementDate, maturityDate);
    const ext::shared_ptr<Exercise> exercise(
        ext::make_shared<EuropeanExercise>(maturityDate));

    // 1. check a plain vanilla call option
    for (Real strike=40; strike < 190; strike+=20) {
        const ext::shared_ptr<StrikedTypePayoff> vanillaPayoff(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, strike));

        VanillaOption planVanillaOption(vanillaPayoff, exercise);

        planVanillaOption.setPricingEngine(pdfEngine);
        const Real calculated = planVanillaOption.NPV();

        planVanillaOption.setPricingEngine(analyticEngine);
        const Real expected = planVanillaOption.NPV();

        if (std::fabs(calculated-expected) > 3*tol) {
            BOOST_FAIL(
                   "failed to reproduce plain vanilla european prices with"
                   " the analytic probability density engine"
                << "\n    strike     : " << strike
                << "\n    expected   : " << expected
                << "\n    calculated : " << calculated
                << "\n    diff       : " << std::fabs(calculated-expected)
                << "\n    tol        ; " << tol);
        }
    }

    // 2. digital call option (approx. with a call spread)
    for (Real strike=40; strike < 190; strike+=10) {
        VanillaOption digitalOption(
            ext::make_shared<CashOrNothingPayoff>(Option::Call, strike, 1.0),
            exercise);
        digitalOption.setPricingEngine(pdfEngine);
        const Real calculated = digitalOption.NPV();

        const Real eps = 0.01;
        VanillaOption longCall(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, strike-eps),
            exercise);
        longCall.setPricingEngine(analyticEngine);

        VanillaOption shortCall(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, strike+eps),
            exercise);
        shortCall.setPricingEngine(analyticEngine);

        const Real expected = (longCall.NPV() - shortCall.NPV())/(2*eps);
        if (std::fabs(calculated-expected) > tol) {
            BOOST_FAIL(
                   "failed to reproduce european digital prices with"
                   " the analytic probability density engine"
                << "\n    strike     : " << strike
                << "\n    expected   : " << expected
                << "\n    calculated : " << calculated
                << "\n    diff       : " << std::fabs(calculated-expected)
                << "\n    tol        : " << tol);
        }

        const DiscountFactor d = riskFreeTS->discount(maturityDate);
        const Real expectedCDF = 1.0 - expected/d;
        const Real calculatedCDF = pdfEngine->cdf(strike, maturity);

        if (std::fabs(expectedCDF - calculatedCDF) > tol) {
            BOOST_FAIL(
                   "failed to reproduce cumulative distribution function"
                << "\n    strike        : " << strike
                << "\n    expected CDF  : " << expectedCDF
                << "\n    calculated CDF: " << calculatedCDF
                << "\n    diff          : "
                << std::fabs(calculatedCDF-expectedCDF)
                << "\n    tol           : " << tol);

        }
    }
}

void HestonModelTest::testExpansionOnAlanLewisReference() {
    BOOST_TEST_MESSAGE("Testing expansion on Alan Lewis reference prices...");

    const Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;

    const Date maturityDate(5, July, 2003);
    const ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturityDate);

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.01, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.04;
    const Real rho   = -0.5;
    const Real sigma =  1.0;
    const Real kappa =  4.0;
    const Real theta =  0.25;

    const ext::shared_ptr<HestonProcess> process =
        ext::make_shared<HestonProcess>(riskFreeTS, dividendTS, s0, v0,
                                          kappa, theta, sigma, rho);
    const ext::shared_ptr<HestonModel> model =
        ext::make_shared<HestonModel>(process);

    const ext::shared_ptr<PricingEngine> lpp2Engine =
        ext::make_shared<HestonExpansionEngine>(model,
                                                  HestonExpansionEngine::LPP2);
    //don't test Forde as it does not behave well on this example
    const ext::shared_ptr<PricingEngine> lpp3Engine =
        ext::make_shared<HestonExpansionEngine>(model,
                                                  HestonExpansionEngine::LPP3);

    const Real strikes[] = { 80, 90, 100, 110, 120 };
    const Option::Type types[] = { Option::Put, Option::Call };
    const ext::shared_ptr<PricingEngine> engines[]
        = { lpp2Engine, lpp3Engine };

    const Real expectedResults[][2] = {
        { 7.958878113256768285213263077598987193482161301733,
          26.774758743998854221382195325726949201687074848341 },
        { 12.017966707346304987709573290236471654992071308187,
          20.933349000596710388139445766564068085476194042256 },
        { 17.055270961270109413522653999411000974895436309183,
          16.070154917028834278213466703938231827658768230714 },
        { 23.017825898442800538908781834822560777763225722188,
          12.132211516709844867860534767549426052805766831181 },
        { 29.811026202682471843340682293165857439167301370697,
          9.024913483457835636553375454092357136489051667150  }
    };

    const Real tol[2] = {1.003e-2, 3.645e-3};

    for (Size i=0; i < LENGTH(strikes); ++i) {
        const Real strike = strikes[i];

        for (Size j=0; j < LENGTH(types); ++j) {
            const Option::Type type = types[j];

            for (Size k=0; k < LENGTH(engines); ++k) {
                const ext::shared_ptr<PricingEngine> engine = engines[k];

                const ext::shared_ptr<StrikedTypePayoff> payoff =
                    ext::make_shared<PlainVanillaPayoff>(type, strike);

                VanillaOption option(payoff, exercise);
                option.setPricingEngine(engine);

                const Real expected = expectedResults[i][j];
                const Real calculated = option.NPV();
                const Real relError = std::fabs(calculated-expected)/expected;

                if (relError > tol[k]) {
                    BOOST_ERROR(
                           "failed to reproduce Alan Lewis Reference prices "
                        << "\n    strike     : " << strike
                        << "\n    option type: " << type
                        << "\n    engine type: " << k
                        << "\n    rel. error : " << relError);
                }
            }
        }
    }
}

void HestonModelTest::testExpansionOnFordeReference() {
    BOOST_TEST_MESSAGE("Testing expansion on Forde reference prices...");

    const Real forward = 100.0;
    const Real v0      =  0.04;
    const Real rho     = -0.4;
    const Real sigma   =  0.2;
    const Real kappa   =  1.15;
    const Real theta   =  0.04;

    const Real terms[] = {0.1, 1.0, 5.0, 10.0};

    const Real strikes[] = { 60, 80, 90, 100, 110, 120, 140 };

    const Real referenceVols[][7] = {
       {0.27284673574924445, 0.22360758200372477, 0.21023988547031242, 0.1990674789471587, 0.19118230678920461, 0.18721342919371017, 0.1899869903378507},
       {0.25200775151345, 0.2127275920953156, 0.20286528150874591, 0.19479398358151515, 0.18872591728967686, 0.18470857955411824, 0.18204457060905446},
       {0.21637821506229973, 0.20077227130455172, 0.19721753043236154, 0.1942233023784151, 0.191693211401571, 0.18955229722896752, 0.18491727548069495},
       {0.20672925973965342, 0.198583062164427, 0.19668274423922746, 0.1950420231354201, 0.193610364344706, 0.1923502827886502, 0.18934360917857015}
    };

    const Real tol[][4] = {
        {0.06, 0.03, 0.03, 0.02},
        {0.15, 0.08, 0.04, 0.02},
        {0.06, 0.08, 1.0, 1.0} //forde breaks down for long maturities
    };
    const Real tolAtm[][4] = {
        {4e-6, 7e-4, 2e-3, 9e-4},
        {7e-6, 4e-4, 9e-4, 4e-4},
        {4e-4, 3e-2, 0.28, 1.0}
    };
    for (Size j=0; j < LENGTH(terms); ++j) {
        const Real term = terms[j];
        const ext::shared_ptr<HestonExpansion> lpp2 =
            ext::make_shared<LPP2HestonExpansion>(kappa, theta, sigma,
                                                    v0, rho, term);
        const ext::shared_ptr<HestonExpansion> lpp3 =
            ext::make_shared<LPP3HestonExpansion>(kappa, theta, sigma,
                                                    v0, rho, term);
        const ext::shared_ptr<HestonExpansion> forde =
            ext::make_shared<FordeHestonExpansion>(kappa, theta, sigma,
                                                     v0, rho, term);
        const ext::shared_ptr<HestonExpansion> expansions[] = { lpp2, lpp3, forde };
        for (Size i=0; i < LENGTH(strikes); ++i) {
            const Real strike = strikes[i];
            for (Size k=0; k < LENGTH(expansions); ++k) {
                const ext::shared_ptr<HestonExpansion> expansion = expansions[k];

                const Real expected = referenceVols[j][i];
                const Real calculated = expansion->impliedVolatility(strike, forward);
                const Real relError = std::fabs(calculated-expected)/expected;
                const Real refTol = strike == forward ? tolAtm[k][j] : tol[k][j];
                if (relError > refTol) {
                    BOOST_ERROR(
                           "failed to reproduce Forde reference vols "
                        << "\n    strike        : " << strike
                        << "\n    expansion type: " << k
                        << "\n    rel. error    : " << relError);
                }
            }
        }
    }
}

namespace {
    void reportOnIntegrationMethodTest(VanillaOption& option,
                                       const ext::shared_ptr<HestonModel>& model,
                                       const AnalyticHestonEngine::Integration& integration,
                                       AnalyticHestonEngine::ComplexLogFormula formula,
                                       bool isAdaptive,
                                       Real expected,
                                       Real tol,
                                       Size valuations,
                                       const std::string& method) {

        if (integration.isAdaptiveIntegration() != isAdaptive)
            BOOST_ERROR(method << " is not an adaptive integration routine");

        const ext::shared_ptr<AnalyticHestonEngine> engine =
            ext::make_shared<AnalyticHestonEngine>(
                model, formula, integration, 1e-9);

        option.setPricingEngine(engine);
        const Real calculated = option.NPV();

        const Real error = std::fabs(calculated - expected);

        if (std::isnan(error) || error > tol) {
            BOOST_ERROR("failed to reproduce simple Heston Pricing with "
                    << "\n    integration method: " << method
                    <<  std::setprecision(12)
                    << "\n    expected          : " << expected
                    << "\n    calculated        : " << calculated
                    << "\n    error             : " << error);
        }

        if (   valuations != Null<Size>()
            && valuations != engine->numberOfEvaluations()) {
            BOOST_ERROR("nubmer of function evaluations does not match "
                    << "\n    integration method      : " << method
                    << "\n    expected function calls : " << valuations
                    << "\n    number of function calls: "
                    << engine->numberOfEvaluations());
        }
    }
}

void HestonModelTest::testAllIntegrationMethods() {
    BOOST_TEST_MESSAGE("Testing semi-analytic Heston pricing with all "
                       "integration methods...");

    const Date settlementDate(7, February, 2017);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.05, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.075, dayCounter));

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.1;
    const Real rho   = -0.75;
    const Real sigma =  0.4;
    const Real kappa =  4.0;
    const Real theta =  0.05;

    const ext::shared_ptr<HestonModel> model =
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                riskFreeTS, dividendTS,
                s0, v0, kappa, theta, sigma, rho));

    const ext::shared_ptr<StrikedTypePayoff> payoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Put, s0->value());

    const Date maturityDate = settlementDate + Period(1, Years);
    const ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturityDate);

    VanillaOption option(payoff, exercise);

    const Real tol = 1e-8;
    const Real expected = 10.147041515497;

    // Gauss-Laguerre with Gatheral logarithm integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussLaguerre(),
        AnalyticHestonEngine::Gatheral,
        false, expected, tol, 256, "Gauss-Laguerre with Gatheral logarithm");

    // Gauss-Laguerre with branch correction integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussLaguerre(),
        AnalyticHestonEngine::BranchCorrection,
        false, expected, tol, 256, "Gauss-Laguerre with branch correction");

    // Gauss-Laguerre with Andersen-Piterbarg integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussLaguerre(),
        AnalyticHestonEngine::AndersenPiterbarg,
        false, expected, tol, 128,
        "Gauss-Laguerre with Andersen Piterbarg control variate");

    // Gauss-Legendre with Gatheral logarithm integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussLegendre(),
        AnalyticHestonEngine::Gatheral,
        false, expected, tol, 256, "Gauss-Legendre with Gatheral logarithm");

    // Gauss-Legendre with branch correction integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussLegendre(),
        AnalyticHestonEngine::BranchCorrection,
        false, expected, tol, 256, "Gauss-Legendre with branch correction");

    // Gauss-Legendre with Andersen-Piterbarg integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussLegendre(256),
        AnalyticHestonEngine::AndersenPiterbarg,
        false, expected, 1e-4, 256,
        "Gauss-Legendre with Andersen Piterbarg control variate");

    // Gauss-Chebyshev with Gatheral logarithm integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussChebyshev(512),
        AnalyticHestonEngine::Gatheral,
        false, expected, 1e-4, 1024, "Gauss-Chebyshev with Gatheral logarithm");

    // Gauss-Chebyshev with branch correction integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussChebyshev(512),
        AnalyticHestonEngine::BranchCorrection,
        false, expected, 1e-4, 1024, "Gauss-Chebyshev with branch correction");

    // Gauss-Chebyshev with Andersen-Piterbarg integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussChebyshev(512),
        AnalyticHestonEngine::AndersenPiterbarg,
        false, expected, 1e-4, 512,
        "Gauss-Laguerre with Andersen Piterbarg control variate");

    // Gauss-Chebyshev2nd with Gatheral logarithm integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussChebyshev2nd(512),
        AnalyticHestonEngine::Gatheral,
        false, expected, 2e-4, 1024,
        "Gauss-Chebyshev2nd with Gatheral logarithm");

    // Gauss-Chebyshev with branch correction integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussChebyshev2nd(512),
        AnalyticHestonEngine::BranchCorrection,
        false, expected, 2e-4, 1024,
        "Gauss-Chebyshev2nd with branch correction");

    // Gauss-Chebyshev with Andersen-Piterbarg integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussChebyshev2nd(512),
        AnalyticHestonEngine::AndersenPiterbarg,
        false, expected, 2e-4, 512,
        "Gauss-Chebyshev2nd with Andersen Piterbarg control variate");

    // Discrete Simpson rule with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::discreteSimpson(512),
        AnalyticHestonEngine::Gatheral,
        false, expected, tol, 1024,
        "Discrete Simpson rule with Gatheral logarithm");

    // Discrete Simpson rule with Andersen-Piterbarg integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::discreteSimpson(64),
        AnalyticHestonEngine::AndersenPiterbarg,
        false, expected, tol, 64,
        "Discrete Simpson rule with Andersen Piterbarg control variate");

    // Discrete Trapezoid rule with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::discreteTrapezoid(512),
        AnalyticHestonEngine::Gatheral,
        false, expected, 2e-4, 1024,
        "Discrete Trapezoid rule with Gatheral logarithm");

    // Discrete Trapezoid rule with Andersen-Piterbarg integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::discreteTrapezoid(64),
        AnalyticHestonEngine::AndersenPiterbarg,
        false, expected, tol, 64,
        "Discrete Trapezoid rule with Andersen Piterbarg control variate");

    // Gauss-Lobatto with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussLobatto(tol, Null<Real>()),
        AnalyticHestonEngine::Gatheral,
        true, expected, tol, Null<Size>(),
        "Gauss-Lobatto with Gatheral logarithm");

    // Gauss-Lobatto with Andersen-Piterbarg integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussLobatto(tol, Null<Real>()),
        AnalyticHestonEngine::AndersenPiterbarg,
        true, expected, tol, Null<Size>(),
        "Gauss-Lobatto with Andersen Piterbarg control variate");

    // Gauss-Konrod with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussKronrod(tol),
        AnalyticHestonEngine::Gatheral,
        true, expected, tol, Null<Size>(),
        "Gauss-Konrod with Gatheral logarithm");

    // Gauss-Konrod with Andersen-Piterbarg integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussKronrod(tol),
        AnalyticHestonEngine::AndersenPiterbarg,
        true, expected, tol, Null<Size>(),
        "Gauss-Konrod with Andersen Piterbarg control variate");

    // Simpson with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::simpson(tol),
        AnalyticHestonEngine::Gatheral,
        true, expected, 1e-6, Null<Size>(),
        "Simpson with Gatheral logarithm");

    // Simpson with Andersen-Piterbarg integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::simpson(tol),
        AnalyticHestonEngine::AndersenPiterbarg,
        true, expected, 1e-6, Null<Size>(),
        "Simpson with Andersen Piterbarg control variate");

    // Trapezoid with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::trapezoid(tol),
        AnalyticHestonEngine::Gatheral,
        true, expected, 1e-6, Null<Size>(),
        "Trapezoid with Gatheral logarithm");

    // Trapezoid with Andersen-Piterbarg integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::trapezoid(tol),
        AnalyticHestonEngine::AndersenPiterbarg,
        true, expected, 1e-6, Null<Size>(),
        "Trapezoid with Andersen Piterbarg control variate");
}

namespace {
    class LogCharacteristicFunction {
      public:
        LogCharacteristicFunction(Size n, Time t, ext::shared_ptr<COSHestonEngine> engine)
        : t_(t), alpha_(0.0, 1.0), engine_(std::move(engine)) {
            for (Size i=1; i < n; ++i, alpha_*=std::complex<Real>(0,1));
        }

        Real operator()(Real u) const {
            return (std::log(engine_->chF(u, t_))/alpha_).real();
        }

      private:
        const Time t_;
        std::complex<Real> alpha_;
        const ext::shared_ptr<COSHestonEngine> engine_;
    };
}

void HestonModelTest::testCosHestonCumulants() {
    BOOST_TEST_MESSAGE("Testing Heston COS cumulants...");

    const Date settlementDate(7, February, 2017);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.15, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.075, dayCounter));

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.1;
    const Real rho   = -0.75;
    const Real sigma =  0.4;
    const Real kappa =  4.0;
    const Real theta =  0.25;

    const ext::shared_ptr<HestonModel> model =
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                riskFreeTS, dividendTS,
                s0, v0, kappa, theta, sigma, rho));

    const ext::shared_ptr<COSHestonEngine> cosEngine =
        ext::make_shared<COSHestonEngine>(model);

    const Real tol = 1e-7;
    const NumericalDifferentiation::Scheme central(
        NumericalDifferentiation::Central);

    for (Time t=0.01; t < 41.0; t+=t) {
        const Real nc1 = NumericalDifferentiation(
            ext::function<Real(Real)>(
                LogCharacteristicFunction(1, t, cosEngine)),
            1, 1e-5, 5, central)(0.0);

        const Real c1 = cosEngine->c1(t);

        if (std::fabs(nc1 - c1) > tol) {
            BOOST_ERROR(" failed to reproduce first cumulant"
                    << "\n    expected:   " << nc1
                    << "\n    calculated: " << c1
                    << "\n    difference: " << std::fabs(nc1 - c1));
        }

        const Real nc2 = NumericalDifferentiation(
            ext::function<Real(Real)>(
                LogCharacteristicFunction(2, t, cosEngine)),
            2, 1e-2, 5, central)(0.0);

        const Real c2 = cosEngine->c2(t);

        if (std::fabs(nc2 - c2) > tol) {
            BOOST_ERROR(" failed to reproduce second cumulant"
                    << "\n    expected:   " << nc2
                    << "\n    calculated: " << c2
                    << "\n    difference: " << std::fabs(nc2 - c2));
        }

        const Real nc3 = NumericalDifferentiation(
            ext::function<Real(Real)>(
                LogCharacteristicFunction(3, t, cosEngine)),
            3, 5e-3, 7, central)(0.0);

        const Real c3 = cosEngine->c3(t);

        if (std::fabs(nc3 - c3) > tol) {
            BOOST_ERROR(" failed to reproduce third cumulant"
                    << "\n    expected:   " << nc3
                    << "\n    calculated: " << c3
                    << "\n    difference: " << std::fabs(nc3 - c3));
        }

        const Real nc4 = NumericalDifferentiation(
            ext::function<Real(Real)>(
                LogCharacteristicFunction(4, t, cosEngine)),
            4, 5e-2, 9, central)(0.0);

        const Real c4 = cosEngine->c4(t);

        if (std::fabs(nc4 - c4) > 10*tol) {
            BOOST_ERROR(" failed to reproduce 4th cumulant"
                    << "\n    expected:   " << nc4
                    << "\n    calculated: " << c4
                    << "\n    difference: " << std::fabs(nc4 - c4));
        }
    }
}

void HestonModelTest::testCosHestonEngine() {
    BOOST_TEST_MESSAGE("Testing Heston pricing via COS method...");

    const Date settlementDate(7, February, 2017);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.15, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.07, dayCounter));

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.1;
    const Real rho   = -0.75;
    const Real sigma =  1.8;
    const Real kappa =  4.0;
    const Real theta =  0.22;

    const ext::shared_ptr<HestonModel> model =
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                riskFreeTS, dividendTS,
                s0, v0, kappa, theta, sigma, rho));

    const Date maturityDate = settlementDate + Period(1, Years);

    const ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturityDate);

    const ext::shared_ptr<PricingEngine> cosEngine(
        ext::make_shared<COSHestonEngine>(model, 25, 600));

    const ext::shared_ptr<StrikedTypePayoff> payoffs[] = {
        ext::make_shared<PlainVanillaPayoff>(Option::Call, s0->value()+20),
        ext::make_shared<PlainVanillaPayoff>(Option::Call, s0->value()+150),
        ext::make_shared<PlainVanillaPayoff>(Option::Put, s0->value()-20),
        ext::make_shared<PlainVanillaPayoff>(Option::Put, s0->value()-90)
    };

    const Real expected[] = {
        9.364410588426075, 0.01036797658132471,
        5.319092971836708, 0.01032681906278383 };

    const Real tol = 1e-10;

    for (Size i=0; i < LENGTH(payoffs); ++i) {
        VanillaOption option(payoffs[i], exercise);

        option.setPricingEngine(cosEngine);
        const Real calculated = option.NPV();

        const Real error = std::fabs(expected[i] - calculated);

        if (error > tol) {
            BOOST_ERROR(" failed to reproduce prices with COSHestonEngine"
                    << "\n    expected:   " << expected[i]
                    << "\n    calculated: " << calculated
                    << "\n    difference: " << error);
        }
    }
}

void HestonModelTest::testCosHestonEngineTruncation() {
    BOOST_TEST_MESSAGE("Testing Heston pricing via COS method outside truncation bounds...");
    
    const Date todaysDate(22, August, 2022);
    const Date maturity(23, August, 2022);
    Settings::instance().evaluationDate() = todaysDate;

    Option::Type type(Option::Call);
    Real underlying = 100;
    Real strike = 200;
    Rate dividendYield = 0;
    Rate riskFreeRate = 0;
    DayCounter dayCounter = Actual365Fixed();

    ext::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(maturity));
    Handle<Quote> underlyingH(ext::shared_ptr<Quote>(new SimpleQuote(underlying)));
    Handle<YieldTermStructure> riskFreeTS(
        ext::shared_ptr<YieldTermStructure>(
            new FlatForward(todaysDate, riskFreeRate, dayCounter)));
    Handle<YieldTermStructure> dividendTS(
        ext::shared_ptr<YieldTermStructure>(
            new FlatForward(todaysDate, dividendYield, dayCounter)));

    ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));
    VanillaOption europeanOption(payoff, europeanExercise);

    ext::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(riskFreeTS, dividendTS, underlyingH, .007, .8, .007, .1, -.2));
    ext::shared_ptr<HestonModel> hestonModel(
            new HestonModel(hestonProcess));
    
    europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                new COSHestonEngine(hestonModel)));
                
    const Real tol = 1e-7;
    const Real error = std::fabs(europeanOption.NPV() - 0.0);

    if (error > tol) {
        BOOST_ERROR(" failed to reproduce prices with COSHestonEngine"
                << "\n    expected:   " << 0.0
                << "\n    calculated: " << europeanOption.NPV()
                << "\n    difference: " << error);
    }
    
}

void HestonModelTest::testCharacteristicFct() {
    BOOST_TEST_MESSAGE("Testing Heston characteristic function...");

    const Date settlementDate(30, March, 2017);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.35, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.17, dayCounter));

    const Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));

    const Real v0    =  0.1;
    const Real rho   = -0.85;
    const Real sigma =  0.8;
    const Real kappa =  2.0;
    const Real theta =  0.15;

    const ext::shared_ptr<HestonModel> model =
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                riskFreeTS, dividendTS,
                s0, v0, kappa, theta, sigma, rho));

    const Real u[] = { 1.0, 0.45, 3,4 };
    const Real t[] = { 0.01, 23.2, 3.2};

    const COSHestonEngine cosEngine(model);
    const AnalyticHestonEngine analyticEngine(model);

    constexpr double tol = 100*QL_EPSILON;
    for (Real i : u) {
        for (Real j : t) {
            const std::complex<Real> c = cosEngine.chF(i, j);
            const std::complex<Real> a = analyticEngine.chF(i, j);

            const Real error = std::abs(a-c);
            if (error > tol) {
                BOOST_ERROR(" failed to reproduce prices with characteristic Fct"
                        << "\n    Cos Engine:      " << c
                        << "\n    analytic engine: " << a
                        << "\n    difference:      " << error);
            }
        }
    }
}

void HestonModelTest::testAndersenPiterbargPricing() {
    BOOST_TEST_MESSAGE("Testing Andersen-Piterbarg method to "
                       "price under the Heston model...");

    const Date settlementDate(30, March, 2017);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.10, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.06, dayCounter));

    const Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));

    const Real v0    =  0.1;
    const Real rho   =  0.80;
    const Real sigma =  0.75;
    const Real kappa =  1.0;
    const Real theta =  0.1;

    const ext::shared_ptr<HestonModel> model =
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                riskFreeTS, dividendTS,
                s0, v0, kappa, theta, sigma, rho));

    const ext::shared_ptr<AnalyticHestonEngine>
        andersenPiterbargLaguerreEngine(
            ext::make_shared<AnalyticHestonEngine>(
                model,
                AnalyticHestonEngine::AndersenPiterbarg,
                AnalyticHestonEngine::Integration::gaussLaguerre()));

    const ext::shared_ptr<AnalyticHestonEngine>
        andersenPiterbargLobattoEngine(
            ext::make_shared<AnalyticHestonEngine>(
                model,
                AnalyticHestonEngine::AndersenPiterbarg,
                AnalyticHestonEngine::Integration::gaussLobatto(
                    Null<Real>(), 1e-9, 10000), 1e-9));

    const ext::shared_ptr<AnalyticHestonEngine>
        andersenPiterbargSimpsonEngine(
            ext::make_shared<AnalyticHestonEngine>(
                model,
                AnalyticHestonEngine::AndersenPiterbarg,
                AnalyticHestonEngine::Integration::discreteSimpson(256),
                1e-8));

    const ext::shared_ptr<AnalyticHestonEngine>
        andersenPiterbargTrapezoidEngine(
            ext::make_shared<AnalyticHestonEngine>(
                model,
                AnalyticHestonEngine::AndersenPiterbarg,
                AnalyticHestonEngine::Integration::discreteTrapezoid(164),
                1e-8));

    const ext::shared_ptr<AnalyticHestonEngine>
        andersenPiterbargTrapezoidEngine2(
            ext::make_shared<AnalyticHestonEngine>(
                model,
                AnalyticHestonEngine::AndersenPiterbarg,
                AnalyticHestonEngine::Integration::trapezoid(1e-8, 256),
                1e-8));

    const ext::shared_ptr<ExponentialFittingHestonEngine>
        andersenPiterbargExponentialFittingEngine(
            ext::make_shared<ExponentialFittingHestonEngine>(model));

    const ext::shared_ptr<PricingEngine> engines[] = {
        andersenPiterbargLaguerreEngine,
        andersenPiterbargLobattoEngine,
        andersenPiterbargSimpsonEngine,
        andersenPiterbargTrapezoidEngine,
        andersenPiterbargTrapezoidEngine2,
        andersenPiterbargExponentialFittingEngine
    };

    const std::string algos[] = {
          "Gauss-Laguerre", "Gauss-Lobatto",
          "Discrete Simpson", "Discrete Trapezoid", "Trapezoid",
          "Exponential Fitting"
    };

    const ext::shared_ptr<PricingEngine> analyticEngine(
        ext::make_shared<AnalyticHestonEngine>(model, 178));

    const Date maturityDates[] = {
        settlementDate + Period(1, Days),
        settlementDate + Period(1, Weeks),
        settlementDate + Period(1, Years),
        settlementDate + Period(10, Years)
    };

    const Option::Type optionTypes[] = { Option::Call, Option::Put };
    const Real strikes[] = { 50, 75, 90, 100, 110, 130, 150, 200};

    const Real tol = 1e-7;

    for (auto maturityDate : maturityDates) {
        const ext::shared_ptr<Exercise> exercise = ext::make_shared<EuropeanExercise>(maturityDate);

        for (auto optionType : optionTypes) {
            for (Real strike : strikes) {
                VanillaOption option(ext::make_shared<PlainVanillaPayoff>(optionType, strike),
                                     exercise);

                option.setPricingEngine(analyticEngine);
                const Real expected = option.NPV();

                for (Size k=0; k < LENGTH(engines); ++k) {
                    option.setPricingEngine(engines[k]);
                    const Real calculated = option.NPV();

                    const Real error = std::fabs(calculated-expected);

                    if (error > tol) {
                        BOOST_ERROR(" failed to reproduce prices with Andersen-"
                                    "Piterbarg control variate"
                                    << "\n    algorithm      : " << algos[k]
                                    << "\n    strike         : " << strike
                                    << "\n    control variate: " << calculated
                                    << "\n    classic engine : " << expected
                                    << "\n    difference:      " << error);
                    }
                }
            }
        }
    }
}


void HestonModelTest::testAndersenPiterbargControlVariateIntegrand() {
    BOOST_TEST_MESSAGE("Testing Andersen-Piterbarg Integrand "
                        "with control variate...");

    const Date settlementDate(17, April, 2017);
    Settings::instance().evaluationDate() = settlementDate;
    const Date maturityDate = settlementDate + Period(2, Years);

    const DayCounter dayCounter = Actual365Fixed();
    const Rate r = 0.075;
    const Rate q = 0.05;
    const Handle<YieldTermStructure> rTS(flatRate(r, dayCounter));
    const Handle<YieldTermStructure> qTS(flatRate(q, dayCounter));

    const Time maturity = dayCounter.yearFraction(settlementDate, maturityDate);
    const DiscountFactor df = rTS->discount(maturity);

    const Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));
    const Real fwd = s0->value()*qTS->discount(maturity)/df;

    const Real strike = 150;
    const Real sx = std::log(strike);
    const Real dd = std::log(s0->value()*qTS->discount(maturity)/df);

    const Real v0    =  0.08;
    const Real rho   =  -0.8;
    const Real sigma =  0.5;
    const Real kappa =  4.0;
    const Real theta =  0.05;

    const ext::shared_ptr<HestonModel> hestonModel(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, qTS, s0, v0, kappa, theta, sigma, rho)));

    const ext::shared_ptr<COSHestonEngine> cosEngine(
        ext::make_shared<COSHestonEngine>(hestonModel));

    const ext::shared_ptr<AnalyticHestonEngine> engine(
        ext::make_shared<AnalyticHestonEngine>(
            hestonModel,
            AnalyticHestonEngine::AndersenPiterbarg,
            AnalyticHestonEngine::Integration::gaussLaguerre()));

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
        ext::make_shared<EuropeanExercise>(maturityDate));
    option.setPricingEngine(engine);

    const Real refNPV = option.NPV();

    const Volatility implStdDev = blackFormulaImpliedStdDev(
            Option::Call, strike, fwd, refNPV, df);

    const Real var = cosEngine->var(maturity);
    const Real stdDev = std::sqrt(var);

    const Real d = (std::log(s0->value()/strike)
        + (r-q)*maturity+ 0.5*var)/stdDev;

    const Real skew = cosEngine->skew(maturity);
    const Real kurt = cosEngine->kurtosis(maturity);

    const NormalDistribution n;

    const Real q3 = 1/6.*s0->value()*stdDev*(2*stdDev - d)*n(d);
    const Real q4 = 1/24.*s0->value()*stdDev*(d*d - 3*d*stdDev - 1)*n(d);
    const Real q5 = 1/72.*s0->value()*stdDev*(
        d*d*d*d - 5*d*d*d*stdDev - 6*d*d + 15*d*stdDev + 3)*n(d);

    const Real bsNPV = blackFormula(Option::Call, strike, fwd, stdDev, df);

    // different variance values for the control variate
    const Real variances[] = {
        v0*maturity,
        ((1-std::exp(-kappa*maturity))*(v0-theta)/(kappa*maturity) + theta)
            *maturity,
        // second moment as control variate
        var,
        // third and fourth moment pricing based on
        // Corrado C. and T. Su, (1996-b),
        // “Skewness and Kurtosis in S&P 500 IndexReturns Implied by Option Prices”,
        // Journal of Financial Research 19 (2), 175-192.
        squared(blackFormulaImpliedStdDev(
            Option::Call, strike, fwd, bsNPV + skew*q3, df)),
        squared(blackFormulaImpliedStdDev(
            Option::Call, strike, fwd, bsNPV + skew*q3 + kurt*q4, df)),
        // Moment matching based on
        // Rubinstein M., (1998), “Edgeworth Binomial Trees”,
        // Journal of Derivatives 5 (3), 20-27.
        squared(blackFormulaImpliedStdDev(
            Option::Call, strike, fwd,
            bsNPV + skew*q3 + kurt*q4 + skew*skew*q5, df)),
        // implied vol as control variate
        squared(implStdDev),
        // remaining function becomes zero for u -> 0
        -8.0*std::log(engine->chF(std::complex<Real>(0, -0.5), maturity).real())
    };

    for (Size i=0; i < LENGTH(variances); ++i) {
        const Real sigmaBS = std::sqrt(variances[i]/maturity);

        for (Real u =0.001; u < 15; u*=1.05) {
            const std::complex<Real> z(u, -0.5);

            const std::complex<Real> phiBS
                = std::exp(-0.5*sigmaBS*sigmaBS*maturity
                           *(z*z + std::complex<Real>(-z.imag(), z.real())));

            const std::complex<Real> ex
                = std::exp(std::complex<Real>(0.0, u*(dd-sx)));

            const std::complex<Real> chf = engine->chF(z, maturity);

            const Real orig = (-ex*chf / (u*u + 0.25)).real();
            const Real cv = (ex*(phiBS - chf) / (u*u + 0.25)).real();

            if (std::fabs(cv) > 0.03) {
                BOOST_ERROR(" Control variate function is greater "
                        "than original function"
                        << "\n    control variate method  : " << i
                        << "\n    z value                 : " << u
                        << "\n    control variate function: " << cv
                        << "\n    original function       : " << orig);
            }
        }
    }
}

void HestonModelTest::testAndersenPiterbargConvergence() {
    BOOST_TEST_MESSAGE("Testing Andersen-Piterbarg pricing convergence...");

    const Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;
    const Date maturityDate(5, July, 2003);

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> rTS(flatRate(0.01, dayCounter));
    const Handle<YieldTermStructure> qTS(flatRate(0.02, dayCounter));

    const Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));

    const Real v0    =  0.04;
    const Real rho   = -0.5;
    const Real sigma =  1.0;
    const Real kappa =  4.0;
    const Real theta =  0.25;

    const ext::shared_ptr<HestonModel> hestonModel(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, qTS, s0, v0, kappa, theta, sigma, rho)));

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, s0->value()),
        ext::make_shared<EuropeanExercise>(maturityDate));


    // Alan Lewis reference prices posted in
    // http://wilmott.com/messageview.cfm?catid=34&threadid=90957
    const Real reference = 16.070154917028834278213466703938231827658768230714;

    const Real diffs[] = {
            0.0892433814611486298,   0.00013096156482816923,
            1.34107015270501506e-07, 1.22913235145460931e-10,
            1.24344978758017533e-13 };

    for (Size n=10; n <= 50; n+=10) {
        option.setPricingEngine(ext::make_shared<AnalyticHestonEngine>(
            hestonModel, AnalyticHestonEngine::AndersenPiterbarg,
            AnalyticHestonEngine::Integration::discreteTrapezoid(n), 1e-13));

        const Real calculatedDiff = std::fabs(option.NPV()-reference);
        if (calculatedDiff > 1.25*diffs[n/10-1])
            BOOST_ERROR("failed to prove convergence for trapezoid rule "
                    << "\n  calculated difference: " << calculatedDiff
                    << "\n  expected difference:   " << diffs[n/10-1]);
    }
}


void HestonModelTest::testPiecewiseTimeDependentChFvsHestonChF() {
    BOOST_TEST_MESSAGE("Testing piecewise time dependent "
                       "ChF vs Heston ChF...");

    const Date settlementDate(5, July, 2017);
    Settings::instance().evaluationDate() = settlementDate;
    const Date maturityDate(5, July, 2018);

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> rTS(flatRate(0.01, dayCounter));
    const Handle<YieldTermStructure> qTS(flatRate(0.02, dayCounter));

    const Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));

    const Real v0    =  0.04;
    const Real rho   = -0.5;
    const Real sigma =  1.0;
    const Real kappa =  4.0;
    const Real theta =  0.25;

    const ConstantParameter thetaP(theta, PositiveConstraint());
    const ConstantParameter kappaP(kappa, PositiveConstraint());
    const ConstantParameter sigmaP(sigma, PositiveConstraint());
    const ConstantParameter rhoP  (rho, BoundaryConstraint(-1.0, 1.0));

    const ext::shared_ptr<AnalyticHestonEngine> analyticEngine(
        ext::make_shared<AnalyticHestonEngine>(
            ext::make_shared<HestonModel>(
                ext::make_shared<HestonProcess>(
                    rTS, qTS, s0, v0, kappa, theta, sigma, rho))));

    const ext::shared_ptr<AnalyticPTDHestonEngine> ptdHestonEngine(
        ext::make_shared<AnalyticPTDHestonEngine>(
            ext::make_shared<PiecewiseTimeDependentHestonModel>(
                rTS, qTS, s0, v0, thetaP, kappaP, sigmaP, rhoP,
                TimeGrid(dayCounter.yearFraction(settlementDate, maturityDate),
                         10))));

    constexpr double tol = 100 * QL_EPSILON;
    for (Real r = 0.1; r < 4; r+=0.25) {
        for (Real phi = 0; phi < 360; phi+=60) {
            for (Time t=0.1; t <= 1.0; t+=0.3) {
                const std::complex<Real> z
                    = r*std::exp(std::complex<Real>(0, phi));

                const std::complex<Real> a = analyticEngine->chF(z, t);
                const std::complex<Real> b = ptdHestonEngine->chF(z, t);

                if (std::abs(a-b) > tol)
                    BOOST_ERROR("failed to compare characteristic function "
                            << "\n  time dependent model: " << b
                            << "\n  Heston model        : " << a
                            << "\n  Difference          : " << std::abs(a-b));
            }
        }
    }
}


void HestonModelTest::testPiecewiseTimeDependentComparison() {
    BOOST_TEST_MESSAGE("Testing piecewise time dependent "
                       "ChF vs Heston ChF...");

    const Date settlementDate(5, July, 2017);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dc = Actual365Fixed();
    const Date maturityDate(5, July, 2018);
    const Time maturity = dc.yearFraction(settlementDate, maturityDate);

    const Handle<YieldTermStructure> rTS(flatRate(0.05, dc));
    const Handle<YieldTermStructure> qTS(flatRate(0.08, dc));

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    std::vector<Time> modelTimes = {0.25, 0.75, 10.0};
    const TimeGrid modelGrid(modelTimes.begin(), modelTimes.end());

    const Real v0 = 0.1;
    ConstantParameter theta( 0.1, PositiveConstraint());
    ConstantParameter kappa( 1.0, PositiveConstraint());
    ConstantParameter rho( -0.75, BoundaryConstraint(-1.0, 1.0));

    std::vector<Time> pTimes(2);
    pTimes[0] = 0.25;
    pTimes[1] = 0.75;
    PiecewiseConstantParameter sigma(pTimes, PositiveConstraint());

    sigma.setParam(0, 0.30);
    sigma.setParam(1, 0.15);
    sigma.setParam(2, 1.25);

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, s0->value()),
        ext::make_shared<EuropeanExercise>(maturityDate));

    const ext::shared_ptr<PiecewiseTimeDependentHestonModel> ptdModel(
        ext::make_shared<PiecewiseTimeDependentHestonModel>(
            rTS, qTS, s0, v0, theta, kappa, sigma, rho, modelGrid));

    const ext::shared_ptr<AnalyticPTDHestonEngine> ptdHestonEngine(
        ext::make_shared<AnalyticPTDHestonEngine>(ptdModel));

    option.setPricingEngine(ptdHestonEngine);
    const Real calculatedGatheral = option.NPV();

    const ext::shared_ptr<AnalyticPTDHestonEngine> ptdAPEngine(
        ext::make_shared<AnalyticPTDHestonEngine>(
            ptdModel,
            AnalyticPTDHestonEngine::AndersenPiterbarg,
            AnalyticPTDHestonEngine::Integration::discreteTrapezoid(128),
            1e-12));
    option.setPricingEngine(ptdAPEngine);
    const Real calculatedAndersenPiterbarg = option.NPV();

    if (std::fabs(calculatedGatheral - calculatedAndersenPiterbarg) > 1e-10)
        BOOST_ERROR("failed to reproduce npv for time dependent Heston model "
                << "\n  Gatheral ChF         : " << calculatedGatheral
                << "\n  AndersenPiterbarg ChF: " << calculatedAndersenPiterbarg
                << "\n  Difference          : "
                << std::fabs(calculatedGatheral - calculatedAndersenPiterbarg));

    const ext::shared_ptr<HestonProcess> firstPartProcess(
        ext::make_shared<HestonProcess>(
            rTS, qTS, s0, v0, 1.0, 0.1, 0.30, -0.75,
            HestonProcess::QuadraticExponentialMartingale));

    typedef PseudoRandom::rsg_type rsg_type;
    typedef PseudoRandom::urng_type urng_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;

    const MultiPathGenerator<rsg_type> firstPathGen(
        firstPartProcess,
        TimeGrid(pTimes.front(), 6),
        PseudoRandom::make_sequence_generator(12, 1234));

    const urng_type urng(5678);

    Statistics stat;
    const DiscountFactor df = rTS->discount(maturityDate);

    const Size nSims = 10000;
    for (Size i=0; i < nSims; ++i) {
        Real priceS = 0.0;

        for (Size j=0; j < 2; ++j) {
            const sample_type& path1 =
                (j & 1) != 0U ? firstPathGen.antithetic() : firstPathGen.next();
            const Real spot1 = path1.value[0].back();
            const Real v1    = path1.value[1].back();

            const MultiPathGenerator<rsg_type> secondPathGen(
                ext::make_shared<HestonProcess>(
                    rTS, qTS,
                    Handle<Quote>(ext::make_shared<SimpleQuote>(spot1)),
                    v1, 1.0, 0.1, 0.15, -0.75,
                    HestonProcess::QuadraticExponentialMartingale),
                TimeGrid(pTimes[1]-pTimes[0], 12),
                PseudoRandom::make_sequence_generator(24, urng.nextInt32()));

            const sample_type& path2 = secondPathGen.next();
            const Real spot2 = path2.value[0].back();
            const Real v2    = path2.value[1].back();

            const MultiPathGenerator<rsg_type> thirdPathGen(
                ext::make_shared<HestonProcess>(
                    rTS, qTS,
                    Handle<Quote>(ext::make_shared<SimpleQuote>(spot2)),
                    v2, 1.0, 0.1, 1.25, -0.75,
                    HestonProcess::QuadraticExponentialMartingale),
                TimeGrid(maturity-pTimes[1], 6),
                PseudoRandom::make_sequence_generator(12, urng.nextInt32()));
            const sample_type& path3 = thirdPathGen.next();
            const Real spot3 = path3.value[0].back();

            priceS += 0.5*(*option.payoff())(spot3);
        }

        stat.add(priceS*df);
    }

    const Real calculatedMC = stat.mean();
    const Real errorEstimate = stat.errorEstimate();

    if (std::fabs(calculatedMC - calculatedGatheral) > 3.0*errorEstimate)
        BOOST_ERROR("failed to reproduce npv for time dependent Heston model"
                << "\n  Gatheral ChF     : " << calculatedGatheral
                << "\n  Monte-Carlo      : " << calculatedMC
                << "\n  Monte-Carlo error: " << errorEstimate
                << "\n  Difference       : "
                << std::fabs(calculatedGatheral - calculatedMC));
}

void HestonModelTest::testPiecewiseTimeDependentChFAsymtotic() {
    BOOST_TEST_MESSAGE("Testing piecewise time dependent "
                       "ChF vs Heston ChF...");

    const Date settlementDate(5, July, 2017);
    Settings::instance().evaluationDate() = settlementDate;
    const Date maturityDate = settlementDate + Period(13, Months);

    const DayCounter dc = Actual365Fixed();
    const Time maturity = dc.yearFraction(settlementDate, maturityDate);
    const Handle<YieldTermStructure> rTS(flatRate(0.0, dc));

    std::vector<Time> modelTimes = {0.01, 0.5, 2.0};

    const TimeGrid modelGrid(modelTimes.begin(), modelTimes.end());

    const Real v0 = 0.1;
    const std::vector<Time> pTimes(modelTimes.begin(), modelTimes.end()-1);

    PiecewiseConstantParameter sigma(pTimes, PositiveConstraint());
    PiecewiseConstantParameter theta(pTimes, PositiveConstraint());
    PiecewiseConstantParameter kappa(pTimes, PositiveConstraint());
    PiecewiseConstantParameter rho(pTimes, BoundaryConstraint(-1.0, 1.0));

    const Real sigmas[] = { 0.01, 0.2, 0.6 };
    const Real thetas[] = { 0.16, 0.06, 0.36 };
    const Real kappas[] = { 1.0, 0.3, 4.0 };
    const Real rhos[] = { 0.5, -0.75, -0.25 };

    for (Size i=0; i < 3; ++i) {
        sigma.setParam(i, sigmas[i]);
        theta.setParam(i, thetas[i]);
        kappa.setParam(i, kappas[i]);
        rho.setParam(i, rhos[i]);
    }

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));
    const ext::shared_ptr<PiecewiseTimeDependentHestonModel> ptdModel(
        ext::make_shared<PiecewiseTimeDependentHestonModel>(
            rTS, rTS, s0, v0, theta, kappa, sigma, rho, modelGrid));

    const Real eps = 1e-8;

    const ext::shared_ptr<AnalyticPTDHestonEngine> ptdHestonEngine(
        ext::make_shared<AnalyticPTDHestonEngine>(
            ptdModel,
            AnalyticPTDHestonEngine::AndersenPiterbarg,
            AnalyticPTDHestonEngine::Integration::discreteTrapezoid(128),
            eps));

    const std::complex<Real> D_u_inf = -
        std::complex<Real>(std::sqrt(1-rhos[0]*rhos[0]),rhos[0])/sigmas[0];

    const std::complex<Real> dd = std::complex<Real>(kappas[0],
        (2*kappas[0]*rhos[0]-sigmas[0])
        /(2*std::sqrt(1-rhos[0]*rhos[0])))/(sigmas[0]*sigmas[0]);

    std::complex<Real> C_u_inf(0.0, 0.0), cc(0.0, 0.0), clog(0.0, 0.0);

    for (Size i=0; i < 3; ++i) {
        const Real kappa = kappas[i];
        const Real theta = thetas[i];
        const Real sigma = sigmas[i];
        const Real rho = rhos[i];
        const Time tau = std::min(maturity, modelGrid[i+1]) - modelGrid[i];

        C_u_inf += -kappa*theta*tau / sigma
            *std::complex<Real>(std::sqrt(1-rho*rho), rho);

        cc += kappa*std::complex<Real>(2*kappa,(2*kappa*rho-sigma)
            /sqrt(1-rho*rho))*tau*theta/(2*sigma*sigma);

        const std::complex<Real> Di =
            (i < 2) ? sigma/sigmas[i+1]
              *std::complex<Real>(std::sqrt(1-rhos[i+1]*rhos[i+1]), rhos[i+1])
                     : std::complex<Real>(0.0, 0.0);

        clog += 2*kappa*theta/(sigma*sigma)*std::log(1.0 -
            ( Di - std::complex<Real>(std::sqrt(1-rho*rho), rho)) /
            ( Di + std::complex<Real>(std::sqrt(1-rho*rho), -rho)));
    }

    const Real epsilon = eps*M_PI/s0->value();

    const Real uM =
        AnalyticHestonEngine::Integration::andersenPiterbargIntegrationLimit(
            -(C_u_inf + D_u_inf*v0).real(), epsilon, v0, maturity);

    const Real expectedUM = 18.6918883427;
    if (std::fabs(uM - expectedUM) > 1e-5) {
        BOOST_ERROR("failed to reproduce Andersen-Piterbarg "
                    "Integration bounds for piecewise constant "
                    "time dependent Heston Model"
                << "\n  calculated : " << uM
                << "\n  expected   : " << expectedUM
                << "\n  diff       : " << std::fabs(uM - expectedUM)
                << "\n  tolerance  : " << 1e-5);
    }

    const Real u = 1e8;
    const std::complex<Real> expectedlnChF = ptdHestonEngine->lnChF(u, maturity);
    const std::complex<Real> calculatedAsympotic =
        (D_u_inf*u + dd)*v0 + C_u_inf*u + cc + clog;

    if (std::abs(expectedlnChF - calculatedAsympotic) > 0.01) {
        BOOST_ERROR("failed to reproduce asymptotic of characteristic function"
                << "\n  ln(ChF)   : " << expectedlnChF
                << "\n  asymptotic: " << calculatedAsympotic
                << "\n  diff      : "
                << std::abs(expectedlnChF - calculatedAsympotic)
                << "\n  tolerance : " << 0.01);
    }

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, s0->value()),
        ext::make_shared<EuropeanExercise>(maturityDate));
    option.setPricingEngine(ptdHestonEngine);

    const Real expectedNPV = 17.43851162589377;
    const Real calculatedNPV = option.NPV();
    const Real diffNPV = std::fabs(expectedNPV - calculatedNPV);
    if (diffNPV > 1e-9) {
        BOOST_ERROR("failed to reproduce high precision prices for "
                "piecewise constant time dependent Heston model"
                << "\n  expeceted : " << expectedNPV
                << "\n  calclated : " << calculatedNPV
                << "\n  diff      : " << diffNPV
                << "\n  tolerance : " << 1e-9);
    }
}

void HestonModelTest::testSmallSigmaExpansion() {
    BOOST_TEST_MESSAGE("Testing small sigma expansion of "
                       "the characteristic function...");

    const Date settlementDate(20, March, 2020);
    Settings::instance().evaluationDate() = settlementDate;
    const Date maturityDate = settlementDate + Period(2, Years);

    const DayCounter dc = Actual365Fixed();
    const Time t = dc.yearFraction(settlementDate, maturityDate);
    const Handle<YieldTermStructure> rTS(flatRate(0.0, dc));

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(100));

    const Real theta = 0.1 * 0.1;
    const Real v0 = theta + 0.02;
    const Real kappa = 1.25;
    const Real sigma = 1e-9;
    const Real rho = -0.9;

    const ext::shared_ptr<HestonModel> hestonModel =
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, rTS, spot, v0, kappa, theta, sigma, rho));

    const ext::shared_ptr<AnalyticHestonEngine> engine =
        ext::make_shared<AnalyticHestonEngine>(hestonModel);

    const std::complex<Real> expectedChF(
        0.990463578538352651,2.60693475987521132e-12);

    const std::complex<Real> calculatedChF = engine->chF(
        std::complex<Real>(0.55, -0.5), t);

    const Real diffChF = std::abs(expectedChF - calculatedChF);
    const Real tolChF = 1e-12;
    if (diffChF > tolChF) {
        BOOST_ERROR("failed to reproduce normalized characteristic function "
                "value for small sigma"
                << "\n  expeceted : " << expectedChF
                << "\n  calclated : " << calculatedChF
                << "\n  diff      : " << diffChF
                << "\n  tolerance : " << tolChF);
    }

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, 120.0),
        ext::make_shared<EuropeanExercise>(maturityDate));

    option.setPricingEngine(
        ext::make_shared<AnalyticHestonEngine>(
            hestonModel,
            AnalyticHestonEngine::AndersenPiterbarg,
            AnalyticHestonEngine::Integration::gaussLaguerre(192)));

    const Real calculatedNPV = option.NPV();

    const Real stdDev =
        std::sqrt(((1-std::exp(-kappa*t))*(v0-theta)/(kappa*t) + theta)*t);

    const Real expectedNPV =
        blackFormula(Option::Call, 120.0, 100.0, stdDev);

    const Real diffNPV =std::fabs(calculatedNPV - expectedNPV);
    const Real tolNPV = 50*sigma;

    if (diffNPV > tolNPV) {
        BOOST_ERROR("failed to reproduce Black Scholes prices "
                "for Heston model with very small sigma"
                << "\n  expeceted : " << expectedNPV
                << "\n  calclated : " << calculatedNPV
                << "\n  diff      : " << diffNPV
                << "\n  tolerance : " << tolNPV);
    }
}

void HestonModelTest::testSmallSigmaExpansion4ExpFitting() {
    BOOST_TEST_MESSAGE("Testing small sigma expansion for the "
                       "exponential fitting Heston engine...");

    const Date todaysDate(13, March, 2020);
    Settings::instance().evaluationDate() = todaysDate;

    const DayCounter dc = Actual365Fixed();
    const Handle<YieldTermStructure> rTS(flatRate(0.05, dc));
    const Handle<YieldTermStructure> qTS(flatRate(0.075, dc));

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(100.0));

    // special case: reduce sigma
    const Date maturityDate = Date(14, March, 2021);
    const Time maturity = dc.yearFraction(todaysDate, maturityDate);
    const Real fwd =
        spot->value()*qTS->discount(maturity)/rTS->discount(maturity);

    const Real v0 = 0.04;
    const Real rho = -0.5;
    const Real kappa = 4.0;
    const Real theta = 0.04;

    const Real moneyness = 0.1;
    const Real strike = std::exp(-moneyness*std::sqrt(theta*maturity))*fwd;

    const Real expected = blackFormula(
        Option::Call, strike, fwd,
        std::sqrt(v0*maturity), rTS->discount(maturity));

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
        ext::make_shared<EuropeanExercise>(maturityDate));

    for (Real sigma = 1e-4; sigma > 1e-12; sigma*=0.1) {
        option.setPricingEngine(
            ext::make_shared<ExponentialFittingHestonEngine>(
                ext::make_shared<HestonModel>(
                    ext::make_shared<HestonProcess>(
                        rTS, qTS, spot, v0, kappa, theta, sigma, rho))));
        const Real calculated = option.NPV();

        const Real diff = std::fabs(expected - calculated);

        if (diff > 0.01*sigma) {
            BOOST_ERROR("failed to reproduce Black Scholes prices "
                    "for Heston model with very small sigma"
                    << "\n  expeceted : " << expected
                    << "\n  calclated : " << calculated
                    << "\n  sigma     : " << sigma
                    << "\n  diff      : " << diff
                    << "\n  tolerance : " << 10*sigma);
        }
    }


    // generic cases
    const Real kappas[] = { 0.5, 1.0, 4.0 };
    const Real thetas[] = { 0.04, 0.09};
    const Real v0s[]    = { 0.025, 0.20 };
    const Integer maturities[] = { 1, 31, 182, 1850 };

    for (int maturitie : maturities) {
        const Date maturityDate = todaysDate + Period(maturitie, Days);
        const DiscountFactor df = rTS->discount(maturityDate);
        const Real fwd = spot->value() * qTS->discount(maturityDate)/df;

        const ext::shared_ptr<Exercise> exercise =
            ext::make_shared<EuropeanExercise>(maturityDate);

        const Time t = dc.yearFraction(todaysDate, maturityDate);

        Option::Type optionType = Option::Call;

        for (Real kappa : kappas) {
            for (Real theta : thetas) {
                for (Real v0 : v0s) {
                    const ext::shared_ptr<PricingEngine> engine =
                        ext::make_shared<ExponentialFittingHestonEngine>(
                            ext::make_shared<HestonModel>(ext::make_shared<HestonProcess>(
                                rTS, qTS, spot, v0, kappa, theta, 1e-13, -0.8)));

                    const Real stdDev =
                        std::sqrt(((1-std::exp(-kappa*t))*(v0-theta)/(kappa*t) + theta)*t);

                    for (Real strike = spot->value()*exp(-10*stdDev);
                            strike < spot->value()*exp(10*stdDev); strike*= 1.2) {

                        VanillaOption option(
                            ext::make_shared<PlainVanillaPayoff>(
                                optionType, strike), exercise);

                        option.setPricingEngine(engine);
                        const Real calculated = option.NPV();

                        const Real expected =
                            blackFormula(optionType, strike, fwd, stdDev, df);

                        const Real diff = std::fabs(expected - calculated);
                        if (diff > 1e-10) {
                            BOOST_ERROR("failed to reproduce Black Scholes prices "
                                    "for Heston model with very small sigma"
                                    << "\n  expceted  : " << expected
                                    << "\n  calculated: " << calculated
                                    << "\n  diff      : " << diff
                                    << "\n  tolerance : " << 1e-10);
                        }

                        optionType = (optionType == Option::Call)
                            ? Option::Put : Option::Call;
                    }
                }
            }
        }
    }
}

void HestonModelTest::testExponentialFitting4StrikesAndMaturities() {
    BOOST_TEST_MESSAGE("Testing exponential fitting Heston engine "
                       "with high precision results for large moneyness...");

    const Date todaysDate = Date(13, May, 2020);
    Settings::instance().evaluationDate() = todaysDate;

    const DayCounter dc = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(flatRate(0.0507, dc));
    const Handle<YieldTermStructure> qTS(flatRate(0.0469, dc));

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(1.0));

    const Real moneyness[] = { -20, -10, -5, 2.5, 1, 0, 1, 2.5, 5, 10, 20 };
    const Period maturities[] = {
            Period(1, Days),
            Period(1, Months),
            Period(1, Years),
            Period(10, Years)
    };

    const Real v0    =  0.04;
    const Real rho   = -0.6;
    const Real sigma =  0.75;
    const Real kappa =  2.5;
    const Real theta =  0.06;

    // Reference prices are calculated using a boost multi-precision
    // implementation of the AnalyticHestonEngine,
    // https://github.com/klausspanderen/HestonExponentialFitting

    const Real referenceValues[] = {
            1.1631865252540813e-58,
            1.06426822273258466e-49,
            6.92896489110422086e-16,
            8.19515526286263236e-06,
            0.000625608178476390504,
            0.00417261379371945684,
            0.000625608178476390504,
            8.19515526286263236e-06,
            1.92308901296741414e-10,
            1.57327901822368115e-23,
            5.7830515043285098e-58,
            3.56081886910098813e-48,
            2.9489071194212509e-23,
            1.54181757781090727e-11,
            0.000367960011879847279,
            0.00493886106106039818,
            0.0227152343265593776,
            0.00493886106106039818,
            0.000367960011879847279,
            3.06653474407784574e-06,
            8.86665241279348934e-11,
            1.51206812371708868e-20,
            4.18506719865401643e-29,
            2.46637786897559908e-15,
            1.75338784910563671e-08,
            0.00284789176080218294,
            0.0199133097064688458,
            0.0776848755698912041,
            0.0199133097064688458,
            0.00284789176080218294,
            0.00012462190796343504,
            2.59755319566692257e-07,
            1.13853114743124721e-12,
            4.27612073892114211e-39,
            1.08387452075906664e-25,
            4.15179522944463802e-11,
            0.00134157732880653131,
            0.029018582813884912,
            0.176405213088554197,
            0.029018582813884912,
            0.00134157732880653131,
            5.43674074281991917e-06,
            6.51443921040230507e-11,
            9.25756999394709285e-21
    };

    const ext::shared_ptr<HestonModel> model =
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, qTS, s0, v0, kappa, theta, sigma, rho));

    const ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<ExponentialFittingHestonEngine>(model);

    Size idx = 0;
    for (auto maturitie : maturities) {
        const Date maturityDate = todaysDate + maturitie;
        const Time t = dc.yearFraction(todaysDate, maturityDate);

        const ext::shared_ptr<Exercise> exercise =
            ext::make_shared<EuropeanExercise>(maturityDate);

        const DiscountFactor df = rTS->discount(t);
        const Real fwd = s0->value()*qTS->discount(t)/df;

        for (Size j=0; j < LENGTH(moneyness); ++j, ++idx) {
            const Real strike =
                std::exp(-moneyness[j]*std::sqrt(theta*t))*fwd;

            for (Size k=0; k < 2; ++k) {
                const ext::shared_ptr<PlainVanillaPayoff> payoff =
                    ext::make_shared<PlainVanillaPayoff>((k) != 0U ? Option::Put : Option::Call,
                                                         strike);

                VanillaOption option(payoff, exercise);
                option.setPricingEngine(engine);

                const Real calculated = option.NPV();

                Real expected;
                if (payoff->optionType() == Option::Call)
                    if (fwd < strike)
                        expected = referenceValues[idx];
                    else
                        expected = (fwd - strike)*df + referenceValues[idx];
                else
                    if (fwd > strike)
                        expected = referenceValues[idx];
                    else
                        expected = referenceValues[idx] - (fwd - strike)*df;

                const Real diff = std::fabs(calculated - expected);
                if (diff > 1e-12) {
                    BOOST_ERROR("failed to reproduce cached extreme "
                            "Heston model prices with exponential fitted "
                            "Gauss-Laguerre quadrature rule"
                            << "\n  forward   : " << fwd
                            << "\n  strike    : " << strike
                            << "\n  expected  : " << expected
                            << "\n  calculated: " << calculated
                            << "\n  diff      : " << diff
                            << "\n  tolerance : " << 1e-12);
                }
            }
        }
    }
}

namespace {
    class HestonIntegrationMaxBoundTestFct {
      public:
        explicit HestonIntegrationMaxBoundTestFct(Real maxBound)
        : maxBound_(maxBound),
          callCounter_(ext::make_shared<Size>(Size(0))) {}

        Real operator()() {
            ++(*callCounter_);
            return maxBound_;
        }

        Size getCallCounter() const {
            return *callCounter_;
        }
      private:
        const Real maxBound_;
        const ext::shared_ptr<Size> callCounter_;
    };
}

void HestonModelTest::testHestonEngineIntegration() {
    BOOST_TEST_MESSAGE("Testing Heston engine integration signature...");

    auto square = [](Real x) -> Real { return x * x; };

    const AnalyticHestonEngine::Integration integration =
        AnalyticHestonEngine::Integration::gaussLobatto(1e-12, 1e-12);

    const Real c1 = integration.calculate(1.0, square, Real(1.0));

    HestonIntegrationMaxBoundTestFct testFct(1.0);
    const Real c2 = integration.calculate(1.0, square, testFct);

    if (testFct.getCallCounter() == 0 ||
            std::fabs(c1 - 1/3.) > 1e-10 || std::fabs(c2 - 1/3.) > 1e-10) {
        BOOST_ERROR("failed to test Heston engine integration signature");
    }
}


void HestonModelTest::testOptimalControlVariateChoice() {
    BOOST_TEST_MESSAGE(
        "Testing optimal control variate choice for the Heston model...");

    Real v0    =  0.0225;
    Real rho   =  0.5;
    Real sigma =  2.0;
    Real kappa =  0.1;
    Real theta =  0.01;
    Time t = 2.0;

    AnalyticHestonEngine::ComplexLogFormula calculated =
        AnalyticHestonEngine::optimalControlVariate(
            t, v0, kappa, theta, sigma, rho);

    if (calculated != AnalyticHestonEngine::AsymptoticChF) {
        BOOST_ERROR("failed to reproduce optimal control variate choice");
    }

    calculated = AnalyticHestonEngine::optimalControlVariate(
            t, v0, kappa, theta, 0.2, rho);
    if (calculated != AnalyticHestonEngine::AndersenPiterbargOptCV) {
        BOOST_ERROR("failed to reproduce optimal control variate choice");
    }

    calculated = AnalyticHestonEngine::optimalControlVariate(
            t, 0.2, kappa, theta, sigma, rho);
    if (calculated != AnalyticHestonEngine::AndersenPiterbargOptCV) {
        BOOST_ERROR("failed to reproduce optimal control variate choice");
    }

}

void HestonModelTest::testAsymptoticControlVariate() {
    BOOST_TEST_MESSAGE("Testing Heston asymptotic control variate...");

    const Date todaysDate = Date(4, August, 2020);
    Settings::instance().evaluationDate() = todaysDate;

    const DayCounter dc = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(flatRate(0.0, dc));

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(1.0));

    const Real v0    =  0.0225;
    const Real rho   =  0.5;
    const Real sigma =  2.0;
    const Real kappa =  0.1;
    const Real theta =  0.01;

    const ext::shared_ptr<HestonModel> model =
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, rTS, s0, v0, kappa, theta, sigma, rho));

    const Date maturityDate = todaysDate + Period(2, Years);
    const Time t = dc.yearFraction(todaysDate, maturityDate);
    const ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturityDate);

    const Real moneynesses[] = { -15, -10, -5, 0, 5, 10, 15 };

    const Real expected[] = {
        0.0074676425640918,
        0.008680823863233695,
        0.010479611906112223,
        0.023590088942038945,
        0.0019575784806211706,
        0.0005490310253748906,
        0.0001657118753134695
    };

    const ext::shared_ptr<PricingEngine> engines[] = {
        ext::make_shared<AnalyticHestonEngine>(
            model,
            AnalyticHestonEngine::OptimalCV,
            AnalyticHestonEngine::Integration::gaussLobatto(1e-10, 1e-10, 100000)),
        ext::make_shared<AnalyticHestonEngine>(
            model,
            AnalyticHestonEngine::OptimalCV,
            AnalyticHestonEngine::Integration::gaussLaguerre(96)),
        ext::make_shared<ExponentialFittingHestonEngine>(model)
    };

    for (Size j=0; j < LENGTH(engines); ++j) {
        for (Size i=0; i < LENGTH(moneynesses); ++i) {
            const Real moneyness = moneynesses[i];

            const Real strike = std::exp(-moneyness*std::sqrt(theta*t));

            const ext::shared_ptr<PlainVanillaPayoff> payoff =
                ext::make_shared<PlainVanillaPayoff>(
                    strike > 1.0 ? Option::Call : Option::Put, strike);

            const ext::shared_ptr<PricingEngine> engine = engines[j];

            VanillaOption option(payoff, exercise);
            option.setPricingEngine(engine);

            const Real calculated = option.NPV();

            const ext::shared_ptr<AnalyticHestonEngine> analyticHestonEngine =
                ext::dynamic_pointer_cast<AnalyticHestonEngine>(engine);

            if ((analyticHestonEngine != nullptr) &&
                analyticHestonEngine->numberOfEvaluations() > 5000) {
                BOOST_ERROR("too many function valuation needed "
                        << "\n  moneyness      : " << moneyness
                        << "\n  evaluations    : "
                        << analyticHestonEngine->numberOfEvaluations()
                        << "\n  max evaluations: " << 5000);
            }

            const Real diff = std::fabs(calculated - expected[i]);
            if (diff > 5e-8) {
                BOOST_ERROR("failed to reproduce extreme Heston model values for"
                        << "\n  moneyness : " << moneyness
                        << "\n  #engine   : " << j
                        << "\n  calculated: " << calculated
                        << "\n  expected  : " << expected[i]
                        << "\n  difference: " << diff
                        << "\n  tolerance : " << 1e-8);
            }
        }
    }
}

void HestonModelTest::testLocalVolFromHestonModel() {
    BOOST_TEST_MESSAGE("Testing Local Volatility pricing from Heston Model...");

    const auto todaysDate = Date(28, June, 2021);
    Settings::instance().evaluationDate() = todaysDate;

    const auto dc = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(
        ext::make_shared<ZeroCurve>(
            std::vector<Date>{
                todaysDate, todaysDate + Period(90, Days),
                todaysDate + Period(180, Days), todaysDate + Period(1, Years)
            },
            std::vector<Rate>{0.075, 0.05, 0.075, 0.1},
            dc
        )
    );

    const Handle<YieldTermStructure> qTS(
        ext::make_shared<ZeroCurve>(
            std::vector<Date>{
                todaysDate, todaysDate + Period(90, Days), todaysDate + Period(1, Years)
            },
            std::vector<Rate>{0.06, 0.04, 0.12},
            dc
        )
    );

    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.1;
    const Real rho   = -0.75;
    const Real sigma =  0.8;
    const Real kappa =  1.0;
    const Real theta =  0.16;

    const auto hestonModel = ext::make_shared<HestonModel>(
        ext::make_shared<HestonProcess>(
            rTS, qTS, s0, v0, kappa, theta, sigma, rho)
    );

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, 120.0),
        ext::make_shared<EuropeanExercise>(todaysDate + Period(1, Years))
    );

    option.setPricingEngine(
        ext::make_shared<AnalyticHestonEngine>(
            hestonModel,
            AnalyticHestonEngine::OptimalCV,
            AnalyticHestonEngine::Integration::gaussLaguerre(192)
        )
    );

    const Real expected = option.NPV();

    option.setPricingEngine(
         ext::make_shared<FdBlackScholesVanillaEngine>(
             ext::make_shared<BlackScholesMertonProcess>(
                 s0, qTS, rTS,
                 Handle<BlackVolTermStructure>(
                      ext::make_shared<HestonBlackVolSurface>(
                          Handle<HestonModel>(hestonModel),
                          AnalyticHestonEngine::OptimalCV,
                          AnalyticHestonEngine::Integration::gaussLaguerre(24)
                      )
                 )
             ),
             25, 125, 1, FdmSchemeDesc::Douglas(), true, 0.4
         )
    );

    const Real calculated = option.NPV();

    const Real tol = 0.002;
    const Real diff = std::fabs(calculated - expected);
    if (diff > tol) {
        BOOST_ERROR("failed to reproduce Heston model values with "
                    "local volatility pricing"
                << "\n  calculated: " << calculated
                << "\n  expected  : " << expected
                << "\n  difference: " << diff
                << "\n  tolerance : " << tol);
    }
}


test_suite* HestonModelTest::suite(SpeedLevel speed) {
    auto* suite = BOOST_TEST_SUITE("Heston model tests");

    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testBlackCalibration));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testDAXCalibration));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAnalyticVsBlack));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAnalyticVsCached));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testMultipleStrikesEngine));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testMcVsCached));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testFdVanillaVsCached));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testFdAmerican));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAnalyticPiecewiseTimeDependent));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testDAXCalibrationOfTimeDependentModel));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAlanLewisReferencePrices));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testExpansionOnAlanLewisReference));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testExpansionOnFordeReference));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAllIntegrationMethods));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testCosHestonCumulants));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testCosHestonEngine));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testCosHestonEngineTruncation));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testCharacteristicFct));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAndersenPiterbargPricing));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAndersenPiterbargControlVariateIntegrand));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAndersenPiterbargConvergence));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testPiecewiseTimeDependentChFvsHestonChF));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testPiecewiseTimeDependentComparison));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testPiecewiseTimeDependentChFAsymtotic));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testSmallSigmaExpansion));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testSmallSigmaExpansion4ExpFitting));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testExponentialFitting4StrikesAndMaturities));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testHestonEngineIntegration));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testOptimalControlVariateChoice));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAsymptoticControlVariate));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testLocalVolFromHestonModel));

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testDifferentIntegrals));
        suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testFdVanillaWithDividendsVsCached));
        suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testFdBarrierVsCached));
    }

    if (speed == Slow) {
        suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testKahlJaeckelCase));
    }

    return suite;
}

test_suite* HestonModelTest::experimental() {
    auto* suite = BOOST_TEST_SUITE("Heston model experimental tests");
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAnalyticPDFHestonEngine));
    return suite;
}
