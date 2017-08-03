/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007, 2009, 2010, 2012, 2014 Klaus Spanderen

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
#include <ql/instruments/dividendbarrieroption.hpp>
#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/models/equity/hestonmodelhelper.hpp>
#include <ql/models/equity/piecewisetimedependenthestonmodel.hpp>
#include <ql/pricingengines/vanilla/analyticdividendeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/hestonexpansionengine.hpp>
#include <ql/pricingengines/vanilla/coshestonengine.hpp>
#include <ql/pricingengines/vanilla/fdamericanengine.hpp>
#include <ql/pricingengines/vanilla/fddividendeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analyticptdhestonengine.hpp>
#include <ql/pricingengines/barrier/fdhestonbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdblackscholesbarrierengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <ql/pricingengines/vanilla/mceuropeanhestonengine.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/differentialevolution.hpp>
#include <ql/time/period.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/experimental/math/numericaldifferentiation.hpp>
#include <ql/experimental/exoticoptions/analyticpdfhestonengine.hpp>

#include <boost/make_shared.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct CalibrationMarketData {
        Handle<Quote> s0;
        Handle<YieldTermStructure> riskFreeTS, dividendYield;
        std::vector<boost::shared_ptr<CalibrationHelper> > options;
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
        // FLOATING_POINT_EXCEPTION
        Handle<YieldTermStructure> riskFreeTS(
			boost::make_shared<ZeroCurve>(dates, rates, dayCounter));
        
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
        
        Handle<Quote> s0(boost::make_shared<SimpleQuote>(4468.17));
        Real strike[] = { 3400,3600,3800,4000,4200,4400,
                          4500,4600,4800,5000,5200,5400,5600 };
        
        std::vector<boost::shared_ptr<CalibrationHelper> > options;
        
        for (Size s = 0; s < 13; ++s) {
            for (Size m = 0; m < 8; ++m) {
                Handle<Quote> vol(boost::make_shared<SimpleQuote>(v[s*8+m]));
        
                Period maturity((int)((t[m]+3)/7.), Weeks); // round to weeks
                options.push_back(boost::make_shared<HestonModelHelper>(maturity, calendar,
                                              s0, strike[s], vol,
                                              riskFreeTS, dividendYield,
                                          CalibrationHelper::ImpliedVolError));
            }
        }
        
        CalibrationMarketData marketData
                                    ={ s0, riskFreeTS, dividendYield, options };
        
        return marketData;
    }
        
}


void HestonModelTest::testBlackCalibration() {
    BOOST_TEST_MESSAGE(
       "Testing Heston model calibration using a flat volatility surface...");

    SavedSettings backup;

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
    Handle<Quote> s0(boost::make_shared<SimpleQuote>(1.0));
    Handle<Quote> vol(boost::make_shared<SimpleQuote>(0.1));
    Volatility volatility = vol->value();

    for (Size i = 0; i < optionMaturities.size(); ++i) {
        for (Real moneyness = -1.0; moneyness < 2.0; moneyness += 1.0) {
            // FLOATING_POINT_EXCEPTION
            const Time tau = dayCounter.yearFraction(
                                 riskFreeTS->referenceDate(),
                                 calendar.advance(riskFreeTS->referenceDate(),
                                                  optionMaturities[i]));
        const Real fwdPrice = s0->value()*dividendTS->discount(tau)
                            / riskFreeTS->discount(tau);
        const Real strikePrice = fwdPrice * std::exp(-moneyness * volatility
                                                     * std::sqrt(tau));

        options.push_back(boost::make_shared<HestonModelHelper>(optionMaturities[i], calendar,
                                                s0, strikePrice, vol,
                                                riskFreeTS, dividendTS));
        }
    }

    for (Real sigma = 0.1; sigma < 0.7; sigma += 0.2) {
        const Real v0=0.01;
        const Real kappa=0.2;
        const Real theta=0.02;
        const Real rho=-0.75;

        boost::shared_ptr<HestonProcess> process(
			boost::make_shared<HestonProcess>(riskFreeTS, dividendTS,
                              s0, v0, kappa, theta, sigma, rho));

        boost::shared_ptr<HestonModel> model(boost::make_shared<HestonModel>(process));
        boost::shared_ptr<PricingEngine> engine(
			boost::make_shared<AnalyticHestonEngine>(model, 96));

        for (Size i = 0; i < options.size(); ++i)
            options[i]->setPricingEngine(engine);

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

    SavedSettings backup;

    Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;

    CalibrationMarketData marketData = getDAXCalibrationMarketData();
    
    const Handle<YieldTermStructure> riskFreeTS = marketData.riskFreeTS;
    const Handle<YieldTermStructure> dividendTS = marketData.dividendYield;
    const Handle<Quote> s0 = marketData.s0;

    const std::vector<boost::shared_ptr<CalibrationHelper> > options
                                                    = marketData.options;

    const Real v0=0.1;
    const Real kappa=1.0;
    const Real theta=0.1;
    const Real sigma=0.5;
    const Real rho=-0.5;

    const boost::shared_ptr<HestonProcess> process(
        boost::make_shared<HestonProcess>(
            riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

    const boost::shared_ptr<HestonModel> model(
        boost::make_shared<HestonModel>(process));

    const boost::shared_ptr<PricingEngine> engines[] = {
        boost::make_shared<AnalyticHestonEngine>(model, 64),
        boost::make_shared<COSHestonEngine>(model, 12, 75)
    };

    const Array params = model->params();
    for (Size j=0; j < LENGTH(engines); ++j) {
        model->setParams(params);
        for (Size i = 0; i < options.size(); ++i)
            options[i]->setPricingEngine(engines[j]);

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

    SavedSettings backup;

    Date settlementDate = Date::todaysDate();
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual();
    Date exerciseDate = settlementDate + 6*Months;

    boost::shared_ptr<StrikedTypePayoff> payoff(
		boost::make_shared<PlainVanillaPayoff>(Option::Put, 30));
    boost::shared_ptr<Exercise> exercise(
		boost::make_shared<EuropeanExercise>(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.1, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.04, dayCounter));

    Handle<Quote> s0(boost::make_shared<SimpleQuote>(32.0));

    const Real v0=0.05;
    const Real kappa=5.0;
    const Real theta=0.05;
    const Real sigma=1.0e-4;
    const Real rho=0.0;

    boost::shared_ptr<HestonProcess> process(boost::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

    VanillaOption option(payoff, exercise);
    // FLOATING_POINT_EXCEPTION
    boost::shared_ptr<PricingEngine> engine(
		boost::make_shared<AnalyticHestonEngine>(
			boost::make_shared<HestonModel>(process), 144));

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
                   << "\n    error:      " << QL_SCIENTIFIC << error);
    }

    engine = 
		boost::make_shared<FdHestonVanillaEngine>(
			boost::make_shared<HestonModel>(process),
              200,200,100);
    option.setPricingEngine(engine);

    calculated = option.NPV();
    error = std::fabs(calculated - expected);
    tolerance = 1.0e-3;
    if (error > tolerance) {
        BOOST_FAIL("failed to reproduce Black price with FdHestonVanillaEngine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << QL_SCIENTIFIC << error);
    }

}


void HestonModelTest::testAnalyticVsCached() {
    BOOST_TEST_MESSAGE("Testing analytic Heston engine against cached values...");

    SavedSettings backup;

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual();
    Date exerciseDate(28, March, 2005);

    boost::shared_ptr<StrikedTypePayoff> payoff(
		boost::make_shared<PlainVanillaPayoff>(Option::Call, 1.05));
    boost::shared_ptr<Exercise> exercise(
		boost::make_shared<EuropeanExercise>(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.0225, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

    Handle<Quote> s0(boost::make_shared<SimpleQuote>(1.0));
    const Real v0 = 0.1;
    const Real kappa = 3.16;
    const Real theta = 0.09;
    const Real sigma = 0.4;
    const Real rho = -0.2;

    boost::shared_ptr<HestonProcess> process(boost::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));

    VanillaOption option(payoff, exercise);

    boost::shared_ptr<AnalyticHestonEngine> engine(
		boost::make_shared<AnalyticHestonEngine>(
			boost::make_shared<HestonModel>(process), 64));

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
			boost::make_shared<PlainVanillaPayoff>(Option::Call, K[i%3]));
        boost::shared_ptr<Exercise> exercise(
			boost::make_shared<EuropeanExercise>(exerciseDate));

        Handle<YieldTermStructure> riskFreeTS(flatRate(0.05, dayCounter));
        Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

        Real s = riskFreeTS->discount(0.7)/dividendTS->discount(0.7);
        Handle<Quote> s0(boost::make_shared<SimpleQuote>(s));

        boost::shared_ptr<HestonProcess> process(
			boost::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, 0.09, 1.2, 0.08, 1.8, -0.45));

        VanillaOption option(payoff, exercise);

        boost::shared_ptr<PricingEngine> engine(
			boost::make_shared<AnalyticHestonEngine>(
				boost::make_shared<HestonModel>(process)));

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

    SavedSettings backup;

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual();
    Date exerciseDate(28, March, 2005);

    boost::shared_ptr<StrikedTypePayoff> payoff(
		boost::make_shared<PlainVanillaPayoff>(Option::Put, 1.05));
    boost::shared_ptr<Exercise> exercise(
		boost::make_shared<EuropeanExercise>(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.7, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.4, dayCounter));

    Handle<Quote> s0(boost::make_shared<SimpleQuote>(1.05));

    boost::shared_ptr<HestonProcess> process(
		boost::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, 0.3, 1.16, 0.2, 0.8, 0.8,
                   HestonProcess::QuadraticExponentialMartingale));

    VanillaOption option(payoff, exercise);

    boost::shared_ptr<PricingEngine> engine;
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

    SavedSettings backup;

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    Handle<Quote> s0(boost::make_shared<SimpleQuote>(100.0));
    Handle<YieldTermStructure> rTS(flatRate(today, 0.08, dc));
    Handle<YieldTermStructure> qTS(flatRate(today, 0.04, dc));

    Date exDate = today + Integer(0.5*360+0.5);
    boost::shared_ptr<Exercise> exercise(
		boost::make_shared<EuropeanExercise>(exDate));

    boost::shared_ptr<StrikedTypePayoff> payoff(
		boost::make_shared<PlainVanillaPayoff>(Option::Call, 90.0));

    boost::shared_ptr<HestonProcess> process(
		boost::make_shared<HestonProcess>(
            rTS, qTS, s0, 0.25*0.25, 1.0, 0.25*0.25, 0.001, 0.0));

    boost::shared_ptr<PricingEngine> engine;
    engine = boost::make_shared<FdHestonBarrierEngine>(
				boost::make_shared<HestonModel>(process),
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
                   << "\n    error:      " << QL_SCIENTIFIC << error);
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
                   << "\n    error:      " << QL_SCIENTIFIC << error);
    }
}

void HestonModelTest::testFdVanillaVsCached() {
    BOOST_TEST_MESSAGE("Testing FD vanilla Heston engine against cached values...");

    SavedSettings backup;

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual();
    Date exerciseDate(28, March, 2005);

    boost::shared_ptr<StrikedTypePayoff> payoff(
		boost::make_shared<PlainVanillaPayoff>(Option::Put, 1.05));
    boost::shared_ptr<Exercise> exercise(
		boost::make_shared<EuropeanExercise>(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.7, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.4, dayCounter));

    Handle<Quote> s0(boost::make_shared<SimpleQuote>(1.05));

    VanillaOption option(payoff, exercise);

    boost::shared_ptr<HestonProcess> process(
		boost::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, 0.3, 1.16, 0.2, 0.8, 0.8));

    boost::shared_ptr<PricingEngine> engine;
    engine = boost::make_shared<FdHestonVanillaEngine>(
				boost::make_shared<HestonModel>(process),
                    100,200,100);
    option.setPricingEngine(engine);

    Real expected = 0.06325;
    Real calculated = option.NPV();
    Real error = std::fabs(calculated - expected);
    Real tolerance = 1.0e-4;

    if (error > tolerance) {
        BOOST_FAIL("failed to reproduce cached price with FD engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << QL_SCIENTIFIC << error);
    }

    BOOST_TEST_MESSAGE("Testing FD vanilla Heston engine for discrete dividends...");

    payoff = boost::make_shared<PlainVanillaPayoff>(Option::Call, 95.0);
    s0 = Handle<Quote>(boost::make_shared<SimpleQuote>(100.0));

    riskFreeTS = Handle<YieldTermStructure>(flatRate(0.05, dayCounter));
    dividendTS = Handle<YieldTermStructure>(flatRate(0.0, dayCounter));

    exerciseDate = Date(28, March, 2006);
    exercise = boost::make_shared<EuropeanExercise>(exerciseDate);

    std::vector<Date> dividendDates;
    std::vector<Real> dividends;
    for (Date d = settlementDate + 3*Months;
              d < exercise->lastDate();
              d += 6*Months) {
        dividendDates.push_back(d);
        dividends.push_back(1.0);
    }

    DividendVanillaOption divOption(payoff, exercise,
                                    dividendDates, dividends);
    process = boost::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, 0.04, 1.0, 0.04, 0.001, 0.0);
    engine = boost::make_shared<FdHestonVanillaEngine>(
				boost::make_shared<HestonModel>(process),
                    200,400,100);
    divOption.setPricingEngine(engine);
    calculated = divOption.NPV();
    // Value calculated with an independent FD framework, validated with
    // an independent MC framework
    expected = 12.946;
    error = std::fabs(calculated - expected);
    tolerance = 5.0e-3;

    if (error > tolerance) {
        BOOST_FAIL("failed to reproduce discrete dividend price with FD engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << QL_SCIENTIFIC << error);
    }

    BOOST_TEST_MESSAGE("Testing FD vanilla Heston engine for american exercise...");

    dividendTS = Handle<YieldTermStructure>(flatRate(0.03, dayCounter));
    process = boost::make_shared<HestonProcess>(
                   riskFreeTS, dividendTS, s0, 0.04, 1.0, 0.04, 0.001, 0.0);
    engine = boost::make_shared<FdHestonVanillaEngine>(
				boost::make_shared<HestonModel>(process),
                    200,400,100);
    payoff = boost::make_shared<PlainVanillaPayoff>(Option::Put, 95.0);
    exercise = boost::make_shared<AmericanExercise>(
            settlementDate, exerciseDate);
    option = VanillaOption(payoff, exercise);
    option.setPricingEngine(engine);
    calculated = option.NPV();

    Handle<BlackVolTermStructure> volTS(flatVol(settlementDate, 0.2,
                                                  dayCounter));
    boost::shared_ptr<BlackScholesMertonProcess> ref_process(
		boost::make_shared<BlackScholesMertonProcess>(s0, dividendTS, riskFreeTS, volTS));
    boost::shared_ptr<PricingEngine> ref_engine(
		boost::make_shared<FDAmericanEngine<CrankNicolson> >(ref_process, 200, 400));
    option.setPricingEngine(ref_engine);
    expected = option.NPV();

    error = std::fabs(calculated - expected);
    tolerance = 1.0e-3;

    if (error > tolerance) {
        BOOST_FAIL("failed to reproduce american option price with FD engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << QL_SCIENTIFIC << error);
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

    SavedSettings backup;

    Date settlementDate(30, March, 2007);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual();
    Date exerciseDate(30, March, 2017);

    const boost::shared_ptr<StrikedTypePayoff> payoff(
		boost::make_shared<PlainVanillaPayoff>(Option::Call, 200));
    const boost::shared_ptr<Exercise> exercise(
		boost::make_shared<EuropeanExercise>(exerciseDate));

    VanillaOption option(payoff, exercise);


    Handle<YieldTermStructure> riskFreeTS(flatRate(0.0, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.0, dayCounter));

    Handle<Quote> s0(boost::make_shared<SimpleQuote>(100));

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

    const Real tolerance = 0.1;
    const Real expected = 4.95212;

    for (Size i=0; i < LENGTH(descriptions); ++i) {
        const boost::shared_ptr<HestonProcess> process(
			boost::make_shared<HestonProcess>(riskFreeTS, dividendTS, s0, v0,
                              kappa, theta, sigma, rho,
                              descriptions[i].discretization));

        const boost::shared_ptr<PricingEngine> engine =
            MakeMCEuropeanHestonEngine<PseudoRandom>(process)
            .withSteps(descriptions[i].nSteps)
            .withAntitheticVariate()
            .withAbsoluteTolerance(tolerance)
            .withSeed(1234);
        option.setPricingEngine(engine);

        const Real calculated = option.NPV();
        const Real errorEstimate = option.errorEstimate();

        if (std::fabs(calculated - expected) > 2.34*errorEstimate) {
            BOOST_ERROR("Failed to reproduce cached price with MC engine"
                        << "\n    discretization: " << descriptions[i].name
                        << "\n    expected:       " << expected
                        << "\n    calculated:     " << calculated
                        << " +/- " << errorEstimate);
        }

        if (errorEstimate > tolerance) {
            BOOST_ERROR("failed to reproduce error estimate with MC engine"
                        << "\n    discretization: " << descriptions[i].name
                        << "\n    calculated    : " << errorEstimate
                        << "\n    expected      :   " << tolerance);
        }
    }

    option.setPricingEngine(
        MakeMCEuropeanHestonEngine<LowDiscrepancy>(
			boost::make_shared<HestonProcess>(
                    riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho,
                    HestonProcess::BroadieKayaExactSchemeLaguerre))
        .withSteps(1)
        .withSamples(1023));

    Real calculated = option.NPV();
    if (std::fabs(calculated - expected) > tolerance) {
        BOOST_ERROR("Failed to reproduce cached price with MC engine"
                    << "\n    discretization: BroadieKayaExactSchemeLobatto"
                    << "\n    calculated:     " << calculated
                    << "\n    expected:       " << expected
                    << "\n    tolerance:      " << tolerance);
    }


    const boost::shared_ptr<HestonModel> hestonModel(
		boost::make_shared<HestonModel>(
			boost::make_shared<HestonProcess>(
                riskFreeTS, dividendTS, s0, v0,
                kappa, theta, sigma, rho)));

    option.setPricingEngine(
		boost::make_shared<FdHestonVanillaEngine>(hestonModel, 200, 400, 100));

    calculated = option.NPV();
    Real error = std::fabs(calculated - expected);
    if (error > 5.0e-2) {
        BOOST_FAIL("failed to reproduce cached price with FD engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << QL_SCIENTIFIC << error);
    }

    option.setPricingEngine(
		boost::make_shared<AnalyticHestonEngine>(hestonModel, 1e-6, 1000));

    calculated = option.NPV();
    error = std::fabs(calculated - expected);

    if (error > 0.00002) {
        BOOST_FAIL("failed to reproduce cached price with "
                   "GaussLobatto engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << QL_SCIENTIFIC << error);
    }

    option.setPricingEngine(
        boost::make_shared<COSHestonEngine>(hestonModel, 16, 400));
    calculated = option.NPV();
    error = std::fabs(calculated - expected);

    if (error > 0.00002) {
        BOOST_FAIL("failed to reproduce cached price with "
                   "Cosine engine"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected
                   << "\n    error:      " << QL_SCIENTIFIC << error);
    }
}

namespace {
    struct HestonParameter {
        Real v0, kappa, theta, sigma, rho; };
}

void HestonModelTest::testDifferentIntegrals() {
    BOOST_TEST_MESSAGE(
       "Testing different numerical Heston integration algorithms...");

    SavedSettings backup;

    const Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = ActualActual();

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

    std::vector<HestonParameter> params;
    params.push_back(equityfx);
    params.push_back(highCorr);
    params.push_back(lowVolOfVol);
    params.push_back(highVolOfVol);
    params.push_back(kappaEqSigRho);

    const Real tol[] = { 1e-3, 1e-3, 0.2, 0.01, 1e-3 };

    for (std::vector<HestonParameter>::const_iterator iter = params.begin();
         iter != params.end(); ++iter) {

        Handle<Quote> s0(boost::make_shared<SimpleQuote>(1.0));
        boost::shared_ptr<HestonProcess> process(
			boost::make_shared<HestonProcess>(
            riskFreeTS, dividendTS,
            s0, iter->v0, iter->kappa,
            iter->theta, iter->sigma, iter->rho));

        boost::shared_ptr<HestonModel> model(
			boost::make_shared<HestonModel>(process));

        boost::shared_ptr<AnalyticHestonEngine> lobattoEngine(
			boost::make_shared<AnalyticHestonEngine>(model, 1e-10,
                                                       1000000));
        boost::shared_ptr<AnalyticHestonEngine> laguerreEngine(
			boost::make_shared<AnalyticHestonEngine>(model, 128));
        boost::shared_ptr<AnalyticHestonEngine> legendreEngine(
			boost::make_shared<AnalyticHestonEngine>(
                model, AnalyticHestonEngine::Gatheral,
                AnalyticHestonEngine::Integration::gaussLegendre(512)));
        boost::shared_ptr<AnalyticHestonEngine> chebyshevEngine(
			boost::make_shared<AnalyticHestonEngine>(
                model, AnalyticHestonEngine::Gatheral,
                AnalyticHestonEngine::Integration::gaussChebyshev(512)));
        boost::shared_ptr<AnalyticHestonEngine> chebyshev2ndEngine(
			boost::make_shared<AnalyticHestonEngine>(
                model, AnalyticHestonEngine::Gatheral,
                AnalyticHestonEngine::Integration::gaussChebyshev2nd(512)));

        Real maxLegendreDiff    = 0.0;
        Real maxChebyshevDiff   = 0.0;
        Real maxChebyshev2ndDiff= 0.0;
        Real maxLaguerreDiff    = 0.0;

        for (Size i=0; i < LENGTH(maturities); ++i) {
            boost::shared_ptr<Exercise> exercise(
				boost::make_shared<EuropeanExercise>(settlementDate
                                     + Period(maturities[i], Months)));

            for (Size j=0; j < LENGTH(strikes); ++j) {
                for (Size k=0; k < LENGTH(types); ++k) {

                    boost::shared_ptr<StrikedTypePayoff> payoff(
						boost::make_shared<PlainVanillaPayoff>(types[k], strikes[j]));

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

    SavedSettings backup;

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual();
    Date exerciseDate(28, March, 2006);

    boost::shared_ptr<Exercise> exercise(
		boost::make_shared<EuropeanExercise>(exerciseDate));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.06, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

    Handle<Quote> s0(boost::make_shared<SimpleQuote>(1.05));

    boost::shared_ptr<HestonProcess> process(
		boost::make_shared<HestonProcess>(
                     riskFreeTS, dividendTS, s0, 0.16, 2.5, 0.09, 0.8, -0.8));
    boost::shared_ptr<HestonModel> model(
		boost::make_shared<HestonModel>(process));

    std::vector<Real> strikes;
    strikes.push_back(1.0);  strikes.push_back(0.5);
    strikes.push_back(0.75); strikes.push_back(1.5); strikes.push_back(2.0);

    boost::shared_ptr<FdHestonVanillaEngine> singleStrikeEngine(
		boost::make_shared<FdHestonVanillaEngine>(model, 20, 400, 50));
    boost::shared_ptr<FdHestonVanillaEngine> multiStrikeEngine(
		boost::make_shared<FdHestonVanillaEngine>(model, 20, 400, 50));
    multiStrikeEngine->enableMultipleStrikesCaching(strikes);

    Real relTol = 5e-3;
    for (Size i=0; i < strikes.size(); ++i) {
        boost::shared_ptr<StrikedTypePayoff> payoff(
			boost::make_shared<PlainVanillaPayoff>(Option::Put, strikes[i]));

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
                       << "\n    error:      " << QL_SCIENTIFIC << relTol);
        }
        if (std::fabs(deltaCalculated-deltaExpected)/deltaExpected > relTol) {
            BOOST_FAIL("failed to reproduce delta with FD multi strike engine"
                       << "\n    calculated: " << deltaCalculated
                       << "\n    expected:   " << deltaExpected
                       << "\n    error:      " << QL_SCIENTIFIC << relTol);
        }
        if (std::fabs(gammaCalculated-gammaExpected)/gammaExpected > relTol) {
            BOOST_FAIL("failed to reproduce gamma with FD multi strike engine"
                       << "\n    calculated: " << gammaCalculated
                       << "\n    expected:   " << gammaExpected
                       << "\n    error:      " << QL_SCIENTIFIC << relTol);
        }
        if (std::fabs(thetaCalculated-thetaExpected)/thetaExpected > relTol) {
            BOOST_FAIL("failed to reproduce theta with FD multi strike engine"
                       << "\n    calculated: " << thetaCalculated
                       << "\n    expected:   " << thetaExpected
                       << "\n    error:      " << QL_SCIENTIFIC << relTol);
        }
    }
}



void HestonModelTest::testAnalyticPiecewiseTimeDependent() {
    BOOST_TEST_MESSAGE("Testing analytic piecewise time dependent Heston prices...");

    SavedSettings backup;

    Date settlementDate(27, December, 2004);
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual();
    Date exerciseDate(28, March, 2005);

    boost::shared_ptr<StrikedTypePayoff> payoff(
		boost::make_shared<PlainVanillaPayoff>(Option::Call, 1.0));
    boost::shared_ptr<Exercise> exercise(
		boost::make_shared<EuropeanExercise>(exerciseDate));

    std::vector<Date> dates; 
    dates.push_back(settlementDate); dates.push_back(Date(01, January, 2007));
    std::vector<Rate> irates;
    irates.push_back(0.0); irates.push_back(0.2);
    Handle<YieldTermStructure> riskFreeTS(
		boost::make_shared<ZeroCurve>(dates, irates, dayCounter));

    std::vector<Rate> qrates;
    qrates.push_back(0.0); qrates.push_back(0.3);
    Handle<YieldTermStructure> dividendTS(
		boost::make_shared<ZeroCurve>(dates, qrates, dayCounter));
    

    const Real v0 = 0.1;
    Handle<Quote> s0(boost::make_shared<SimpleQuote>(1.0));

    ConstantParameter theta(0.09, PositiveConstraint());
    ConstantParameter kappa(3.16, PositiveConstraint());
    ConstantParameter sigma(4.40, PositiveConstraint());
    ConstantParameter rho  (-0.8, BoundaryConstraint(-1.0, 1.0));

    boost::shared_ptr<PiecewiseTimeDependentHestonModel> model(
		boost::make_shared<PiecewiseTimeDependentHestonModel>(riskFreeTS, dividendTS,
                                              s0, v0, theta, kappa, 
                                              sigma, rho, TimeGrid(20.0, 2)));
    
    VanillaOption option(payoff, exercise);
    option.setPricingEngine(boost::make_shared<AnalyticPTDHestonEngine>(model));

    const Real calculated = option.NPV();
    boost::shared_ptr<HestonProcess> hestonProcess(
		boost::make_shared<HestonProcess>(riskFreeTS, dividendTS, s0, v0,
                          kappa(0.0), theta(0.0), sigma(0.0), rho(0.0)));
    boost::shared_ptr<HestonModel> hestonModel(boost::make_shared<HestonModel>(hestonProcess));
    option.setPricingEngine(boost::make_shared<AnalyticHestonEngine>(hestonModel));
    
    const Real expected = option.NPV();
    
    if (std::fabs(calculated-expected) > 1e-12) {
        BOOST_ERROR("failed to reproduce heston prices "
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected);
    }
}

void HestonModelTest::testDAXCalibrationOfTimeDependentModel() {
    BOOST_TEST_MESSAGE(
             "Testing time-dependent Heston model calibration...");

    SavedSettings backup;

    Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;

    CalibrationMarketData marketData = getDAXCalibrationMarketData();
    
    const Handle<YieldTermStructure> riskFreeTS = marketData.riskFreeTS;
    const Handle<YieldTermStructure> dividendTS = marketData.dividendYield;
    const Handle<Quote> s0 = marketData.s0;

    const std::vector<boost::shared_ptr<CalibrationHelper> > options
                                                    = marketData.options;

    std::vector<Time> modelTimes;
    modelTimes.push_back(0.25);
    modelTimes.push_back(10.0);
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

    boost::shared_ptr<PiecewiseTimeDependentHestonModel> model(
		boost::make_shared<PiecewiseTimeDependentHestonModel>(riskFreeTS, dividendTS,
                                              s0, v0, theta, kappa, 
                                              sigma, rho, modelGrid));
    
    boost::shared_ptr<PricingEngine> engine(
		boost::make_shared<AnalyticPTDHestonEngine>(model));
    for (Size i = 0; i < options.size(); ++i)
        options[i]->setPricingEngine(engine);

    LevenbergMarquardt om(1e-8, 1e-8, 1e-8);
    model->calibrate(options, om, EndCriteria(400, 40, 1.0e-8, 1.0e-8, 1.0e-8));

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

void HestonModelTest::testAlanLewisReferencePrices() {
    BOOST_TEST_MESSAGE("Testing Alan Lewis reference prices...");

    /*
     * testing Alan Lewis reference prices posted in
     * http://wilmott.com/messageview.cfm?catid=34&threadid=90957
     */

    SavedSettings backup;

    const Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;

    const Date maturityDate(5, July, 2003);
    const boost::shared_ptr<Exercise> exercise(
		boost::make_shared<EuropeanExercise>(maturityDate));

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.01, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

    const Handle<Quote> s0(boost::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.04;
    const Real rho   = -0.5;
    const Real sigma =  1.0;
    const Real kappa =  4.0;
    const Real theta =  0.25;

    const boost::shared_ptr<HestonProcess> process(
		boost::make_shared<HestonProcess>(
			riskFreeTS, dividendTS, s0, v0, kappa, theta, sigma, rho));
    const boost::shared_ptr<HestonModel> model(
		boost::make_shared<HestonModel>(process));

    const boost::shared_ptr<PricingEngine> laguerreEngine(
		boost::make_shared<AnalyticHestonEngine>(model, 128u));

    const boost::shared_ptr<PricingEngine> gaussLobattoEngine(
		boost::make_shared<AnalyticHestonEngine>(model, QL_EPSILON, 100000u));

    const boost::shared_ptr<PricingEngine> cosEngine(
		boost::make_shared<COSHestonEngine>(model, 20, 400));

    const Real strikes[] = { 80, 90, 100, 110, 120 };
    const Option::Type types[] = { Option::Put, Option::Call };
    const boost::shared_ptr<PricingEngine> engines[]
        = { laguerreEngine, gaussLobattoEngine, cosEngine };

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
                const boost::shared_ptr<PricingEngine> engine = engines[k];

                const boost::shared_ptr<StrikedTypePayoff> payoff(
					boost::make_shared<PlainVanillaPayoff>(type, strike));

                VanillaOption option(payoff, exercise);
                option.setPricingEngine(engine);

                const Real expected = expectedResults[i][j];
                const Real calculated = option.NPV();
                const Real relError = std::fabs(calculated-expected)/expected;

                if (relError > tol) {
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

    SavedSettings backup;

    const Date settlementDate(5, January, 2014);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.07, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.185, dayCounter));

    const Handle<Quote> s0(boost::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.1;
    const Real rho   = -0.5;
    const Real sigma =  1.0;
    const Real kappa =  4.0;
    const Real theta =  0.05;

    const boost::shared_ptr<HestonModel> model(
        boost::make_shared<HestonModel>(
			boost::make_shared<HestonProcess>(riskFreeTS, dividendTS,
                              s0, v0, kappa, theta, sigma, rho)));

    const Real tol = 1e-6;
    const boost::shared_ptr<AnalyticPDFHestonEngine> pdfEngine(
		boost::make_shared<AnalyticPDFHestonEngine>(model, tol));

    const boost::shared_ptr<PricingEngine> analyticEngine(
		boost::make_shared<AnalyticHestonEngine>(model, 192));

    const Date maturityDate(5, July, 2014);
    const Time maturity = dayCounter.yearFraction(settlementDate, maturityDate);
    const boost::shared_ptr<Exercise> exercise(
		boost::make_shared<EuropeanExercise>(maturityDate));

    // 1. check a plain vanilla call option
    for (Real strike=40; strike < 190; strike+=20) {
        const boost::shared_ptr<StrikedTypePayoff> vanillaPayoff(
			boost::make_shared<PlainVanillaPayoff>(Option::Call, strike));

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
			boost::make_shared<CashOrNothingPayoff>(Option::Call, strike, 1.0),
            exercise);
        digitalOption.setPricingEngine(pdfEngine);
        const Real calculated = digitalOption.NPV();

        const Real eps = 0.01;
        VanillaOption longCall(
			boost::make_shared<PlainVanillaPayoff>(Option::Call, strike-eps),
            exercise);
        longCall.setPricingEngine(analyticEngine);

        VanillaOption shortCall(
			boost::make_shared<PlainVanillaPayoff>(Option::Call, strike+eps),
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

    SavedSettings backup;

    const Date settlementDate(5, July, 2002);
    Settings::instance().evaluationDate() = settlementDate;

    const Date maturityDate(5, July, 2003);
    const boost::shared_ptr<Exercise> exercise =
        boost::make_shared<EuropeanExercise>(maturityDate);

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.01, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));

    const Handle<Quote> s0(boost::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.04;
    const Real rho   = -0.5;
    const Real sigma =  1.0;
    const Real kappa =  4.0;
    const Real theta =  0.25;

    const boost::shared_ptr<HestonProcess> process =
        boost::make_shared<HestonProcess>(riskFreeTS, dividendTS, s0, v0,
                                          kappa, theta, sigma, rho);
    const boost::shared_ptr<HestonModel> model =
        boost::make_shared<HestonModel>(process);

    const boost::shared_ptr<PricingEngine> lpp2Engine =
        boost::make_shared<HestonExpansionEngine>(model,
                                                  HestonExpansionEngine::LPP2);
    //don't test Forde as it does not behave well on this example
    const boost::shared_ptr<PricingEngine> lpp3Engine =
        boost::make_shared<HestonExpansionEngine>(model,
                                                  HestonExpansionEngine::LPP3);

    const Real strikes[] = { 80, 90, 100, 110, 120 };
    const Option::Type types[] = { Option::Put, Option::Call };
    const boost::shared_ptr<PricingEngine> engines[]
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
                const boost::shared_ptr<PricingEngine> engine = engines[k];

                const boost::shared_ptr<StrikedTypePayoff> payoff =
                    boost::make_shared<PlainVanillaPayoff>(type, strike);

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

    SavedSettings backup;

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
        const boost::shared_ptr<HestonExpansion> lpp2 =
            boost::make_shared<LPP2HestonExpansion>(kappa, theta, sigma,
                                                    v0, rho, term);
        const boost::shared_ptr<HestonExpansion> lpp3 =
            boost::make_shared<LPP3HestonExpansion>(kappa, theta, sigma,
                                                    v0, rho, term);
        const boost::shared_ptr<HestonExpansion> forde =
            boost::make_shared<FordeHestonExpansion>(kappa, theta, sigma,
                                                     v0, rho, term);
        const boost::shared_ptr<HestonExpansion> expansions[] = { lpp2, lpp3, forde };
        for (Size i=0; i < LENGTH(strikes); ++i) {
            const Real strike = strikes[i];
            for (Size k=0; k < LENGTH(expansions); ++k) {
                const boost::shared_ptr<HestonExpansion> expansion = expansions[k];

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
    void reportOnIntegrationMethodTest(
        VanillaOption& option,
        const boost::shared_ptr<HestonModel>& model,
        const AnalyticHestonEngine::Integration& integration,
        AnalyticHestonEngine::ComplexLogFormula formula,
        bool isAdaptive, Real expected, Real tol, Size valuations,
        std::string method) {

        if (integration.isAdaptiveIntegration() != isAdaptive)
            BOOST_ERROR(method << " is not an adaptive integration routine");

        const boost::shared_ptr<AnalyticHestonEngine> engine =
            boost::make_shared<AnalyticHestonEngine>(
                model, formula, integration);

        option.setPricingEngine(engine);
        const Real calculated = option.NPV();

        const Real error = std::fabs(calculated - expected);
        if (error > tol) {
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

    SavedSettings backup;

    const Date settlementDate(7, February, 2017);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.05, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.075, dayCounter));

    const Handle<Quote> s0(boost::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.1;
    const Real rho   = -0.75;
    const Real sigma =  0.4;
    const Real kappa =  4.0;
    const Real theta =  0.05;

    const boost::shared_ptr<HestonModel> model =
        boost::make_shared<HestonModel>(
            boost::make_shared<HestonProcess>(
                riskFreeTS, dividendTS,
                s0, v0, kappa, theta, sigma, rho));

    const boost::shared_ptr<StrikedTypePayoff> payoff =
        boost::make_shared<PlainVanillaPayoff>(Option::Put, s0->value());

    const Date maturityDate = settlementDate + Period(1, Years);
    const boost::shared_ptr<Exercise> exercise =
        boost::make_shared<EuropeanExercise>(maturityDate);

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

    // Discrete Simpson rule with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::discreteSimpson(512),
        AnalyticHestonEngine::Gatheral,
        false, expected, tol, 1024,
        "Discrete Simpson rule with Gatheral logarithm");

    // Discrete Simpson rule with branch correction integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::discreteSimpson(512),
        AnalyticHestonEngine::BranchCorrection,
        false, expected, tol, 1024,
        "Discrete Simpson rule with branch correction");

    // Discrete Trapezoid rule with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::discreteTrapezoid(512),
        AnalyticHestonEngine::Gatheral,
        false, expected, 2e-4, 1024,
        "Discrete Trapezoid rule with Gatheral logarithm");

    // Discrete Trapezoid rule with branch correction integration method
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::discreteTrapezoid(512),
        AnalyticHestonEngine::BranchCorrection,
        false, expected, 2e-4, 1024,
        "Discrete Trapezoid rule with branch correction");

    // Gauss-Lobatto with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussLobatto(tol, Null<Real>()),
        AnalyticHestonEngine::Gatheral,
        true, expected, tol, Null<Size>(),
        "Gauss-Lobatto with Gatheral logarithm");

    // Gauss-Konrod with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::gaussKronrod(tol),
        AnalyticHestonEngine::Gatheral,
        true, expected, tol, Null<Size>(),
        "Gauss-Konrod with Gatheral logarithm");

    // Simpson with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::simpson(tol),
        AnalyticHestonEngine::Gatheral,
        true, expected, 1e-6, Null<Size>(),
        "Simpson with Gatheral logarithm");

    // Trapezoid with Gatheral logarithm
    reportOnIntegrationMethodTest(option, model,
        AnalyticHestonEngine::Integration::trapezoid(tol),
        AnalyticHestonEngine::Gatheral,
        true, expected, 1e-6, Null<Size>(),
        "Trapezoid with Gatheral logarithm");
}

namespace {
    class LogCharacteristicFunction
            : public std::unary_function<Real, Real> {
      public:
        LogCharacteristicFunction(
            Size n, Time t,
            const boost::shared_ptr<COSHestonEngine>& engine)
        : t_(t), alpha_(0.0, 1.0), engine_(engine) {
            for (Size i=1; i < n; ++i, alpha_*=std::complex<Real>(0,1));
        }

        Real operator()(Real u) const {
            return (std::log(engine_->characteristicFct(u, t_))/alpha_).real();
        }

      private:
        const Time t_;
        std::complex<Real> alpha_;
        const boost::shared_ptr<COSHestonEngine> engine_;
    };
}

void HestonModelTest::testCosHestonCumulants() {
    BOOST_TEST_MESSAGE("Testing Heston COS cumulants...");

    SavedSettings backup;

    const Date settlementDate(7, February, 2017);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.15, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.075, dayCounter));

    const Handle<Quote> s0(boost::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.1;
    const Real rho   = -0.75;
    const Real sigma =  0.4;
    const Real kappa =  4.0;
    const Real theta =  0.25;

    const boost::shared_ptr<HestonModel> model =
        boost::make_shared<HestonModel>(
            boost::make_shared<HestonProcess>(
                riskFreeTS, dividendTS,
                s0, v0, kappa, theta, sigma, rho));

    const boost::shared_ptr<COSHestonEngine> cosEngine =
        boost::make_shared<COSHestonEngine>(model);

    const Real tol = 1e-7;
    const NumericalDifferentiation::Scheme central(
        NumericalDifferentiation::Central);

    for (Time t=0.01; t < 41.0; t+=t) {
        const Real nc1 = NumericalDifferentiation(
            boost::function<Real(Real)>(
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
            boost::function<Real(Real)>(
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
            boost::function<Real(Real)>(
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
            boost::function<Real(Real)>(
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

    SavedSettings backup;

    const Date settlementDate(7, February, 2017);
    Settings::instance().evaluationDate() = settlementDate;

    const DayCounter dayCounter = Actual365Fixed();
    const Handle<YieldTermStructure> riskFreeTS(flatRate(0.15, dayCounter));
    const Handle<YieldTermStructure> dividendTS(flatRate(0.07, dayCounter));

    const Handle<Quote> s0(boost::make_shared<SimpleQuote>(100.0));

    const Real v0    =  0.1;
    const Real rho   = -0.75;
    const Real sigma =  1.8;
    const Real kappa =  4.0;
    const Real theta =  0.22;

    const boost::shared_ptr<HestonModel> model =
        boost::make_shared<HestonModel>(
            boost::make_shared<HestonProcess>(
                riskFreeTS, dividendTS,
                s0, v0, kappa, theta, sigma, rho));

    const Date maturityDate = settlementDate + Period(1, Years);

    const boost::shared_ptr<Exercise> exercise =
        boost::make_shared<EuropeanExercise>(maturityDate);

    const boost::shared_ptr<PricingEngine> cosEngine(
        boost::make_shared<COSHestonEngine>(model, 25, 600));

    const boost::shared_ptr<StrikedTypePayoff> payoffs[] = {
        boost::make_shared<PlainVanillaPayoff>(Option::Call, s0->value()+20),
        boost::make_shared<PlainVanillaPayoff>(Option::Call, s0->value()+150),
        boost::make_shared<PlainVanillaPayoff>(Option::Put, s0->value()-20),
        boost::make_shared<PlainVanillaPayoff>(Option::Put, s0->value()-90)
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

test_suite* HestonModelTest::suite(SpeedLevel speed) {
    test_suite* suite = BOOST_TEST_SUITE("Heston model tests");

    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testBlackCalibration));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testDAXCalibration));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAnalyticVsBlack));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testAnalyticVsCached));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testDifferentIntegrals));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testFdVanillaVsCached));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testMultipleStrikesEngine));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testMcVsCached));
    suite->add(QUANTLIB_TEST_CASE(
                    &HestonModelTest::testAnalyticPiecewiseTimeDependent));
    suite->add(QUANTLIB_TEST_CASE(
                    &HestonModelTest::testDAXCalibrationOfTimeDependentModel));
    suite->add(QUANTLIB_TEST_CASE(
                    &HestonModelTest::testAlanLewisReferencePrices));
    suite->add(QUANTLIB_TEST_CASE(
                    &HestonModelTest::testExpansionOnAlanLewisReference));
    suite->add(QUANTLIB_TEST_CASE(
                    &HestonModelTest::testExpansionOnFordeReference));
    suite->add(QUANTLIB_TEST_CASE(
                    &HestonModelTest::testAllIntegrationMethods));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testCosHestonCumulants));
    suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testCosHestonEngine));

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(
            &HestonModelTest::testFdBarrierVsCached));
    }

    if (speed == Slow) {
        suite->add(QUANTLIB_TEST_CASE(&HestonModelTest::testKahlJaeckelCase));
    }

    return suite;
}

test_suite* HestonModelTest::experimental() {
    test_suite* suite = BOOST_TEST_SUITE("Heston model tests");
    suite->add(QUANTLIB_TEST_CASE(
        &HestonModelTest::testAnalyticPDFHestonEngine));
    return suite;
}
