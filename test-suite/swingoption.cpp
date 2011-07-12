/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010, 2011 Klaus Spanderen

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

#include "swingoption.hpp"
#include "utilities.hpp"

#include <ql/math/functional.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/instruments/basketoption.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/instruments/vanillaswingoption.hpp>
#include <ql/instruments/vanillastorageoption.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/statistics/generalstatistics.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/methods/montecarlo/multipathgenerator.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/experimental/processes/gemanroncoroniprocess.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/uniform1dmesher.hpp>
#include <ql/experimental/finitedifferences/fdmmeshercomposite.hpp>
#include <ql/experimental/finitedifferences/fdsimplebsswingengine.hpp>
#include <ql/experimental/finitedifferences/fdextoujumpvanillaengine.hpp>
#include <ql/experimental/finitedifferences/fdsimpleextoustorageengine.hpp>
#include <ql/experimental/finitedifferences/fdsimpleextoujumpswingengine.hpp>
#include <ql/experimental/finitedifferences/exponentialjump1dmesher.hpp>
#include <ql/experimental/finitedifferences/fdblackscholesvanillaengine.hpp>
#include <ql/experimental/finitedifferences/fdklugeextouspreadengine.hpp>
#include <ql/experimental/finitedifferences/fdmvppstepcondition.hpp>

#include <boost/lambda/lambda.hpp>
#include <deque>

using namespace QuantLib;
using namespace boost::unit_test_framework;


namespace {
    boost::shared_ptr<ExtOUWithJumpsProcess> createKlugeProcess() {
        Array x0(2);
        x0[0] = 3.0; x0[1] = 0.0;

        const Real beta = 5.0;
        const Real eta  = 2.0;
        const Real jumpIntensity = 1.0;
        const Real speed = 1.0;
        const Real volatility = 2.0;

        boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> ouProcess(
            new ExtendedOrnsteinUhlenbeckProcess(speed, volatility, x0[0],
                                                 constant<Real, Real>(x0[0])));
        return boost::shared_ptr<ExtOUWithJumpsProcess>(
            new ExtOUWithJumpsProcess(ouProcess, x0[1], beta,
                                      jumpIntensity, eta));
    }
}

void SwingOptionTest::testExtendedOrnsteinUhlenbeckProcess() {

    BOOST_MESSAGE("Testing extended Ornstein-Uhlenbeck process...");

    SavedSettings backup;

	const Real speed = 2.5;
	const Volatility vol = 0.70;
	const Real level = 1.43;

	ExtendedOrnsteinUhlenbeckProcess::Discretization discr[] = {
		ExtendedOrnsteinUhlenbeckProcess::MidPoint,
		ExtendedOrnsteinUhlenbeckProcess::Trapezodial,
		ExtendedOrnsteinUhlenbeckProcess::GaussLobatto};

	boost::function<Real (Real)> f[] 
		= { constant<Real, Real>(level),
			std::bind1st(std::plus<Real>(), 1.0),
			std::ptr_fun<Real, Real>(std::sin) }; 

	for (Size n=0; n < LENGTH(f); ++n) {
		ExtendedOrnsteinUhlenbeckProcess refProcess(
			speed, vol, 0.0, f[n], 
			ExtendedOrnsteinUhlenbeckProcess::GaussLobatto, 1e-6);

		for (Size i=0; i < LENGTH(discr)-1; ++i) {
			ExtendedOrnsteinUhlenbeckProcess eouProcess(
									  speed, vol, 0.0, f[n], discr[i]);

			const Time T = 10;
			const Size nTimeSteps = 10000;

			const Time dt = T/nTimeSteps;
			Time t  = 0.0;
			Real q = 0.0;
			Real p = 0.0;
			
			PseudoRandom::rng_type rng(PseudoRandom::urng_type(1234u));

			for (Size j=0; j < nTimeSteps; ++j) {
				const Real dw = rng.next().value;
				q=eouProcess.evolve(t,q,dt,dw);
				p=refProcess.evolve(t,p,dt,dw);

				if (std::fabs(q-p) > 1e-6) {
					BOOST_FAIL("invalid process evaluation " 
					            << n << " " << i << " " << j << " " << q-p);
				}
				t+=dt;
			}
		}
	}
}


void SwingOptionTest::testGemanRoncoroniProcess() {

    BOOST_MESSAGE("Testing Geman Roncoroni process...");

    /* Example induced by H. Geman, A. Roncoroni,
       "Understanding the Fine Structure of Electricity Prices",
       http://papers.ssrn.com/sol3/papers.cfm?abstract_id=638322
       Results are verified against the authors MatLab-Code.
       http://semeq.unipmn.it/files/Ch19_spark_spread.zip
    */

    SavedSettings backup;

    const Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;
    const DayCounter dc = ActualActual();
    
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, 0.03, dc);
        
	const Real x0     = 3.3;
	const Real beta   = 0.05;
    const Real alpha  = 3.1;
	const Real gamma  = -0.09;
	const Real delta  = 0.07;
	const Real eps    = -0.40;
	const Real zeta   = -0.40;
	const Real d      = 1.6;
	const Real k      = 1.0;
	const Real tau    = 0.5;
	const Real sig2   = 10.0;
	const Real a      =-7.0;
	const Real b      =-0.3;
	const Real theta1 = 35.0;
	const Real theta2 = 9.0;
	const Real theta3 = 0.10;
	const Real psi    = 1.9;

    boost::shared_ptr<GemanRoncoroniProcess> grProcess(
                new GemanRoncoroniProcess(x0, alpha, beta, gamma, delta,
        								  eps, zeta, d, k, tau, sig2, a, b, 
        								  theta1, theta2, theta3, psi));


    const Real speed     = 5.0;
    const Volatility vol = std::sqrt(1.4);
    const Real betaG     = 0.08;
    const Real alphaG    = 1.0;
    const Real x0G       = 1.1;

    boost::function<Real (Real)> f = alphaG + betaG*boost::lambda::_1;

    boost::shared_ptr<StochasticProcess1D> eouProcess(
        new ExtendedOrnsteinUhlenbeckProcess(speed, vol, x0G, f,
                           ExtendedOrnsteinUhlenbeckProcess::Trapezodial));

    std::vector<boost::shared_ptr<StochasticProcess1D> > processes;
    processes.push_back(grProcess);
    processes.push_back(eouProcess);
    
    Matrix correlation(2, 2, 1.0);
    correlation[0][1] = correlation[1][0] = 0.25;
    
    boost::shared_ptr<StochasticProcess> pArray(
                           new StochasticProcessArray(processes, correlation));
    
    const Time T = 10.0;
    const Size stepsPerYear = 250;
    const Size steps = Size(T*Real(stepsPerYear));
    
    TimeGrid grid(T, steps);

    typedef PseudoRandom::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;
    rsg_type rsg = PseudoRandom::make_sequence_generator(
                               pArray->size()*(grid.size()-1), BigNatural(421));

    GeneralStatistics npv, onTime;    
    MultiPathGenerator<rsg_type> generator(pArray, grid, rsg, false);
        
    const Real heatRate = 8.0;
    const Size nrTrails = 250;
    
    for (Size n=0; n < nrTrails; ++n) {
        Real plantValue = 0.0;
        sample_type path = generator.next();

        for (Size i=1; i <= steps; ++i) {
            const Time t = Real(i)/stepsPerYear;
            const DiscountFactor df = rTS->discount(t);
            
            const Real gasPrice         = std::exp(path.value[1][i]);
            const Real electricityPrice = std::exp(path.value[0][i]);
            
            const Real sparkSpread = electricityPrice - heatRate*gasPrice;
            plantValue += std::max(0.0, sparkSpread)*df;
            onTime.add((sparkSpread > 0.0) ? 1.0 : 0.0);            
        }
                
        npv.add(plantValue);
    }
    
    const Real expectedNPV = 12500;
    const Real calculatedNPV = npv.mean();
    const Real errorEstimateNPV = npv.errorEstimate();
    
    if (std::fabs(calculatedNPV - expectedNPV) > 3.0*errorEstimateNPV) {
        BOOST_ERROR("Failed to reproduce cached price with MC engine"
                    << "\n    calculated: " << calculatedNPV
                    << "\n    expected:   " << expectedNPV
                    << " +/- " << errorEstimateNPV);
    }
    
    const Real expectedOnTime = 0.43;
    const Real calculatedOnTime = onTime.mean();
    const Real errorEstimateOnTime 
        = std::sqrt(calculatedOnTime*(1-calculatedOnTime))/nrTrails;
    
    if (std::fabs(calculatedOnTime - expectedOnTime)>3.0*errorEstimateOnTime) {
        BOOST_ERROR("Failed to reproduce cached price with MC engine"
                    << "\n    calculated: " << calculatedNPV
                    << "\n    expected:   " << expectedNPV
                    << " +/- " << errorEstimateNPV);
    }
}

void SwingOptionTest::testFdmExponentialJump1dMesher() {

    BOOST_MESSAGE("Testing finite difference mesher for the Kluge model ...");

    SavedSettings backup;

    Array x(2, 1.0);
    const Real beta = 100.0;
    const Real eta  = 1.0/0.4;
    const Real jumpIntensity = 4.0;
    const Size dummySteps  = 2;

    ExponentialJump1dMesher mesher(dummySteps, beta, jumpIntensity, eta);

    boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> ouProcess(
        new ExtendedOrnsteinUhlenbeckProcess(1.0, 1.0, x[0],
                                             constant<Real, Real>(1.0)));
    boost::shared_ptr<ExtOUWithJumpsProcess> jumpProcess(
        new ExtOUWithJumpsProcess(ouProcess, x[1], beta, jumpIntensity, eta));

    const Time dt = 1.0/(10.0*beta);
    const Size n = 1000000;

    std::vector<Real> path(n);
    PseudoRandom::rng_type mt(PseudoRandom::urng_type(123));
    Array dw(3);
    for (Size i=0; i < n; ++i) {
        dw[0] = mt.next().value;
        dw[1] = mt.next().value;
        dw[2] = mt.next().value;
        path[i] = (x = jumpProcess->evolve(0.0, x, dt, dw))[1];
    }
    std::sort(path.begin(), path.end());

    const Real relTol1 = 2e-3;
    const Real relTol2 = 2e-2;
    const Real threshold = 0.9;

    for (Real x=1e-12; x < 1.0; x*=10) {
        const Real v = mesher.jumpSizeDistribution(x);

        std::vector<Real>::iterator iter
            = std::lower_bound(path.begin(), path.end(), x);
        const Real q = std::distance(path.begin(), iter)/Real(n);
        QL_REQUIRE(std::fabs(q - v) < relTol1
                    || (v < threshold) && std::fabs(q-v) < relTol2,
                    "can not reproduce jump distribution");
    }
}

void SwingOptionTest::testExtOUJumpVanillaEngine() {

    BOOST_MESSAGE("Testing finite difference pricer for the Kluge model ...");

    SavedSettings backup;

    boost::shared_ptr<ExtOUWithJumpsProcess> jumpProcess = createKlugeProcess();

    const Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    const DayCounter dc = ActualActual();
    const Date maturityDate = today + Period(12, Months);
    const Time maturity = dc.yearFraction(today, maturityDate);

    const Rate irRate = 0.1;
    boost::shared_ptr<YieldTermStructure> rTS(flatRate(today, irRate, dc));
    boost::shared_ptr<StrikedTypePayoff> payoff(
                                     new PlainVanillaPayoff(Option::Call, 30));
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(maturityDate));

    boost::shared_ptr<PricingEngine> engine(
                 new FdExtOUJumpVanillaEngine(jumpProcess, rTS, 25, 200, 50));

    VanillaOption option(payoff, exercise);
    option.setPricingEngine(engine);
    const Real fdNPV = option.NPV();

    const Size steps = 100;
    const Size nrTrails = 200000;
    TimeGrid grid(maturity, steps);

    typedef PseudoRandom::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;
    rsg_type rsg = PseudoRandom::make_sequence_generator(
                    jumpProcess->factors()*(grid.size()-1), BigNatural(421));

    GeneralStatistics npv;
    MultiPathGenerator<rsg_type> generator(jumpProcess, grid, rsg, false);

    for (Size n=0; n < nrTrails; ++n) {
        sample_type path = generator.next();

        const Real x = path.value[0].back();
        const Real y = path.value[1].back();

        const Real cashflow = (*payoff)(std::exp(x+y));
        npv.add(cashflow*rTS->discount(maturity));
    }

    const Real mcNPV = npv.mean();
    const Real mcError = npv.errorEstimate();

    if ( std::fabs(fdNPV - mcNPV) > 3.0*mcError) {
        BOOST_ERROR("Failed to reproduce FD and MC prices"
                    << "\n    FD NPV: " << fdNPV
                    << "\n    MC NPV: " << mcNPV
                    << " +/- " << mcError);
    }
}

void SwingOptionTest::testFdBSSwingOption() {

    BOOST_MESSAGE("Testing Black-Scholes Vanilla Swing option pricing ...");

    SavedSettings backup;

    Date settlementDate = Date::todaysDate();
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual();
    Date maturityDate = settlementDate + Period(12, Months);

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                     new PlainVanillaPayoff(Option::Put, 30));

    std::vector<Date> exerciseDates(1, settlementDate+Period(1, Months));
    while (exerciseDates.back() < maturityDate) {
        exerciseDates.push_back(exerciseDates.back()+Period(1, Months));
    }

    boost::shared_ptr<SwingExercise> swingExercise(
                                            new SwingExercise(exerciseDates));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.14, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));
    Handle<BlackVolTermStructure> volTS(
                                    flatVol(settlementDate, 0.4, dayCounter));

    Handle<Quote> s0(boost::shared_ptr<Quote>(new SimpleQuote(30.0)));

    boost::shared_ptr<BlackScholesMertonProcess> process(
            new BlackScholesMertonProcess(s0, dividendTS, riskFreeTS, volTS));
    boost::shared_ptr<PricingEngine> engine(
                                new FdSimpleBSSwingEngine(process, 50, 200));
    
    VanillaOption bermudanOption(payoff, swingExercise);
    bermudanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                          new FdBlackScholesVanillaEngine(process, 50, 200)));
    const Real bermudanOptionPrices = bermudanOption.NPV();
    
    for (Size i=0; i < exerciseDates.size(); ++i) {
        const Size exerciseRights = i+1;
        
        VanillaSwingOption swingOption(payoff, swingExercise,
                                       exerciseRights, exerciseRights);
        swingOption.setPricingEngine(engine);
        const Real swingOptionPrice = swingOption.NPV();

        const Real upperBound = exerciseRights*bermudanOptionPrices;

        if (swingOptionPrice - upperBound > 2e-2) {
            BOOST_ERROR("Failed to reproduce upper bounds"
                        << "\n    upper Bound: " << upperBound
                        << "\n    Price:       " << swingOptionPrice);
        }
        
        Real lowerBound = 0.0;
        for (Size j=exerciseDates.size()-i-1; j < exerciseDates.size(); ++j) {
            VanillaOption europeanOption(payoff, boost::shared_ptr<Exercise>(
                                     new EuropeanExercise(exerciseDates[j])));
            europeanOption.setPricingEngine(
                boost::shared_ptr<PricingEngine>(
                                          new AnalyticEuropeanEngine(process)));
            lowerBound += europeanOption.NPV();
        }

        if (lowerBound - swingOptionPrice > 2e-2) {
            BOOST_ERROR("Failed to reproduce lower bounds"
                        << "\n    lower Bound: " << lowerBound
                        << "\n    Price:       " << swingOptionPrice);
        }
    }
}


void SwingOptionTest::testExtOUJumpSwingOption() {

    BOOST_MESSAGE("Testing Simple Swing option pricing for Kluge model...");

    SavedSettings backup;

    Date settlementDate = Date::todaysDate();
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual();
    Date maturityDate = settlementDate + Period(12, Months);

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                     new PlainVanillaPayoff(Option::Put, 30));

    std::vector<Date> exerciseDates(1, settlementDate+Period(1, Months));
    while (exerciseDates.back() < maturityDate) {
        exerciseDates.push_back(exerciseDates.back()+Period(1, Months));
    }
    boost::shared_ptr<SwingExercise> swingExercise(
                                            new SwingExercise(exerciseDates));

    std::vector<Time> exerciseTimes(exerciseDates.size());
    for (Size i=0; i < exerciseTimes.size(); ++i) {
        exerciseTimes[i]
                 = dayCounter.yearFraction(settlementDate, exerciseDates[i]);
    }

    TimeGrid grid(exerciseTimes.begin(), exerciseTimes.end(), 60);
    std::vector<Size> exerciseIndex(exerciseDates.size());
    for (Size i=0; i < exerciseIndex.size(); ++i) {
        exerciseIndex[i] = grid.closestIndex(exerciseTimes[i]);
    }

    boost::shared_ptr<ExtOUWithJumpsProcess> jumpProcess = createKlugeProcess();

    const Rate irRate = 0.1;
    boost::shared_ptr<YieldTermStructure> rTS(
                                flatRate(settlementDate, irRate, dayCounter));

    boost::shared_ptr<PricingEngine> swingEngine(
                new FdSimpleExtOUJumpSwingEngine(jumpProcess, rTS, 25, 50, 25));

    boost::shared_ptr<PricingEngine> vanillaEngine(
                new FdExtOUJumpVanillaEngine(jumpProcess, rTS, 25, 50, 25));

    VanillaOption bermudanOption(payoff, swingExercise);
    bermudanOption.setPricingEngine(vanillaEngine);
    const Real bermudanOptionPrices = bermudanOption.NPV();

    const Size nrTrails = 16000;
    typedef PseudoRandom::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;
    rsg_type rsg = PseudoRandom::make_sequence_generator(
                    jumpProcess->factors()*(grid.size()-1), BigNatural(421));

    MultiPathGenerator<rsg_type> generator(jumpProcess, grid, rsg, false);

    for (Size i=0; i < exerciseDates.size(); ++i) {
        const Size exerciseRights = i+1;

        VanillaSwingOption swingOption(payoff, swingExercise,
                                       exerciseRights, exerciseRights);
        swingOption.setPricingEngine(swingEngine);
        const Real swingOptionPrice = swingOption.NPV();

        const Real upperBound = exerciseRights*bermudanOptionPrices;

        if (swingOptionPrice - upperBound > 2e-2) {
            BOOST_ERROR("Failed to reproduce upper bounds"
                        << "\n    upper Bound: " << upperBound
                        << "\n    Price:       " << swingOptionPrice);
        }

        Real lowerBound = 0.0;
        for (Size j=exerciseDates.size()-i-1; j < exerciseDates.size(); ++j) {
            VanillaOption europeanOption(payoff, boost::shared_ptr<Exercise>(
                                     new EuropeanExercise(exerciseDates[j])));
            europeanOption.setPricingEngine(
                boost::shared_ptr<PricingEngine>(vanillaEngine));
            lowerBound += europeanOption.NPV();
        }

        if (lowerBound - swingOptionPrice > 2e-2) {
            BOOST_ERROR("Failed to reproduce lower bounds"
                       << "\n    lower Bound: " << lowerBound
                       << "\n    Price:       " << swingOptionPrice);
        }

        // use MC plus perfect forecast to find an upper bound
        GeneralStatistics npv;
        for (Size n=0; n < nrTrails; ++n) {
            sample_type path = generator.next();

            std::vector<Real> exerciseValues(exerciseTimes.size());
            for (Size k=0; k < exerciseTimes.size(); ++k) {
                const Real x = path.value[0][exerciseIndex[k]];
                const Real y = path.value[1][exerciseIndex[k]];
                const Real s = std::exp(x+y);

                exerciseValues[k] =(*payoff)(s)*rTS->discount(exerciseDates[k]);
            }
            std::sort(exerciseValues.begin(), exerciseValues.end(),
                      std::greater<Real>());

            Real npCashFlows
                = std::accumulate(exerciseValues.begin(),
                                  exerciseValues.begin()+exerciseRights, 0.0);
            npv.add(npCashFlows);
        }

        const Real mcUpperBound = npv.mean();
        const Real mcErrorUpperBound = npv.errorEstimate();
        if (swingOptionPrice - mcUpperBound > 2.36*mcErrorUpperBound) {
            BOOST_ERROR("Failed to reproduce mc upper bounds"
                       << "\n    mc upper Bound: " << mcUpperBound
                       << "\n    Price:          " << swingOptionPrice);
        }
    }
}

void SwingOptionTest::testSimpleExtOUStorageEngine() {

    BOOST_MESSAGE("Testing Simple Storage option based on ext. OU  model...");

    SavedSettings backup;

    Date settlementDate = Date::todaysDate();
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual();
    Date maturityDate = settlementDate + Period(12, Months);

    std::vector<Date> exerciseDates(1, settlementDate+Period(1, Days));
    while (exerciseDates.back() < maturityDate) {
        exerciseDates.push_back(exerciseDates.back()+Period(1, Days));
    }
    boost::shared_ptr<BermudanExercise> bermudanExercise(
                                        new BermudanExercise(exerciseDates));

    const Real x0 = 3.0;
    const Real speed = 1.0;
    const Real volatility = 0.5;
    const Rate irRate = 0.1;

    boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> ouProcess(
        new ExtendedOrnsteinUhlenbeckProcess(speed, volatility, x0,
                                             constant<Real, Real>(x0)));

    boost::shared_ptr<YieldTermStructure> rTS(
                                flatRate(settlementDate, irRate, dayCounter));

    boost::shared_ptr<PricingEngine> storageEngine(
               new FdSimpleExtOUStorageEngine(ouProcess, rTS, 1, 25));

    VanillaStorageOption storageOption(bermudanExercise, 50, 0, 1);

    storageOption.setPricingEngine(storageEngine);

    const Real expected = 69.6686;
    const Real calculated = storageOption.NPV();

    if (std::fabs(expected - calculated) > 2e-2) {
        BOOST_ERROR("Failed to reproduce cached values"
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected);
    }
}


void SwingOptionTest::testKlugeExtOUSpreadOption() {

    BOOST_MESSAGE("Testing Simple Kluge ext-Ornstein-Uhlenbeck spread option");

    SavedSettings backup;

    Date settlementDate = Date::todaysDate();
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual();
    Date maturityDate = settlementDate + Period(1, Years);
    Time maturity = dayCounter.yearFraction(settlementDate, maturityDate);

    const Real speed     = 1.0;
    const Volatility vol = std::sqrt(1.4);
    const Real betaG     = 0.0;
    const Real alphaG    = 3.0;
    const Real x0G       = 3.0;

    const Rate irRate      = 0.0;
    const Real heatRate    = 2.0;
    const Real rho         = 0.5;

    boost::shared_ptr<ExtOUWithJumpsProcess>
                                           klugeProcess = createKlugeProcess();
    boost::function<Real (Real)> f = alphaG + betaG*boost::lambda::_1;

    boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> extOUProcess(
        new ExtendedOrnsteinUhlenbeckProcess(speed, vol, x0G, f,
                           ExtendedOrnsteinUhlenbeckProcess::Trapezodial));

    boost::shared_ptr<YieldTermStructure> rTS(
                                flatRate(settlementDate, irRate, dayCounter));

    boost::shared_ptr<Payoff> payoff(new PlainVanillaPayoff(Option::Call, 0.0));

    Array spreadFactors(2);
    spreadFactors[0] = 1.0; spreadFactors[1] = -heatRate;
    boost::shared_ptr<BasketPayoff> basketPayoff(
                               new AverageBasketPayoff(payoff, spreadFactors));

    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(maturityDate));

    BasketOption option(basketPayoff, exercise);
    option.setPricingEngine(boost::shared_ptr<PricingEngine>(
        new FdKlugeExtOUSpreadEngine(rho,
                                     klugeProcess, extOUProcess, rTS,
                                     5, 200, 50, 20)));

    TimeGrid grid(maturity, 50);
    PseudoRandom::rng_type rng(PseudoRandom::urng_type(1234ul));

    GeneralStatistics npv;
    const Size nTrails = 20000;
    for (Size i=0; i < nTrails; ++i) {
        Real u = extOUProcess->x0();
        Array x = klugeProcess->initialValues();

        for (Size k=0; k < grid.size()-1; ++k) {
            const Time t = grid.at(k);
            const Time dt = grid.dt(k);

            Array dw(3);
            dw[0] = rng.next().value;
            dw[1] = rng.next().value;
            dw[2] = rng.next().value;
            x = klugeProcess->evolve(t, x, dt, dw);

            const Real du = std::sqrt(1-rho*rho) * rng.next().value + rho*dw[0];
            u = extOUProcess->evolve(t, u, dt, du);
        }

        Array p(2);
        p[0] = x[0] + x[1]; p[1] = u;
        npv.add((*basketPayoff)(Exp(p)));
    }

    const Real calculated = option.NPV();
    const Real expectedMC = npv.mean();
    const Real mcError = npv.errorEstimate();
    if (std::fabs(expectedMC - calculated) > 3*mcError) {
            BOOST_ERROR("Failed to reproduce referenc values"
                       << "\n    calculated:   " << calculated
                       << "\n    expected(MC): " << expectedMC
                       << "\n    mc error    : " << mcError);

    }
}

namespace {
    // for a "real" gas and power forward curve
    // please see. e.g. http://www.kyos.com/?content=64
    const Real gasPrices[] = {20.74,21.65,20.78,21.58,21.43,20.82,22.02,21.52,
                              21.02,21.46,21.75,20.69,22.16,20.38,20.82,20.68,
                              20.57,21.92,22.04,20.45,20.75,21.92,20.53,20.67,
                              20.88,21.02,20.82,21.67,21.82,22.12,20.45,20.74,
                              22.39,20.95,21.71,20.70,20.94,21.59,22.33,21.13,
                              21.50,21.42,20.56,21.23,21.37,21.90,20.62,21.17,
                              21.86,22.04,22.05,21.00,20.70,21.12,21.26,22.40,
                              21.31,22.24,21.96,21.02,21.71,20.48,21.36,21.75,
                              21.90,20.44,21.26,22.29,20.34,21.79,21.66,21.50,
                              20.76,20.27,20.84,20.24,21.97,20.52,20.98,21.40,
                              20.39,20.71,20.78,20.30,21.56,21.72,20.27,21.57,
                              21.82,20.57,21.33,20.51,22.32,21.99,20.57,22.11,
                              21.56,22.24,20.62,21.70,21.11,21.19,21.79,20.46,
                              22.21,20.82,20.52,22.29,20.71,21.45,22.40,20.63,
                              20.95,21.97,22.20,20.67,21.01,22.25,20.76,21.33,
                              20.49,20.33,21.94,20.64,20.99,21.09,20.97,22.17,
                              20.72,22.06,20.86,21.40,21.75,20.78,21.79,20.47,
                              21.19,21.60,20.75,21.36,21.61,20.37,21.67,20.28,
                              22.33,21.37,21.33,20.87,21.25,22.01,22.08,20.81,
                              20.70,21.84,21.82,21.68,21.24,22.36,20.83,20.64,
                              21.03,20.57,22.34,20.96,21.54,21.26,21.43,22.39};

    const Real powerPrices[]={40.40,36.71,31.87,25.81,31.61,35.00,46.22,60.68,
                              42.45,38.01,33.84,29.79,31.84,38.53,49.23,59.92,
                              43.85,37.47,34.89,29.99,30.85,29.19,29.25,38.67,
                              36.90,25.93,22.12,20.19,17.19,19.29,13.51,18.14,
                              33.76,30.48,25.63,18.01,23.86,32.41,48.56,64.69,
                              38.42,39.31,32.73,29.97,31.41,35.02,46.85,58.12,
                              39.14,35.42,32.61,28.76,29.41,35.83,46.73,61.41,
                              61.01,59.43,60.43,66.29,62.79,62.66,57.66,51.63,
                              62.18,60.53,61.94,64.86,59.57,58.15,53.74,48.36,
                              45.64,51.21,51.54,50.79,54.50,49.92,41.58,39.81,
                              28.86,37.42,39.78,42.36,45.67,36.84,33.91,28.75,
                              62.97,63.84,62.91,68.77,64.33,61.95,59.12,54.89,
                              63.62,60.90,66.57,69.51,64.71,59.89,57.28,57.10,
                              65.09,63.82,67.52,70.51,65.59,59.36,58.22,54.64,
                              52.17,53.02,57.12,53.50,53.16,49.21,52.21,40.96,
                              49.01,47.94,49.89,53.83,52.96,50.33,51.72,46.99,
                              39.06,47.99,47.91,52.35,48.51,47.39,50.45,43.66,
                              25.62,35.76,42.76,46.51,45.62,46.79,48.76,41.00,
                              52.65,55.57,57.67,56.79,55.15,54.74,50.31,47.49,
                              53.72,55.62,55.89,58.11,54.46,52.92,49.61,44.68,
                              51.59,57.44,56.50,55.12,57.22,54.61,49.92,45.20};

    class GasPrice : public FdmInnerValueCalculator {
      public:
        Real innerValue(const FdmLinearOpIterator&, Time t) {
            return gasPrices[(Size) t]; }
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) {
            return innerValue(iter, t);
        }
    };
    class SparkSpreadPrice : public FdmInnerValueCalculator {
      public:
        SparkSpreadPrice(Real heatRate) : heatRate_(heatRate) {}
        Real innerValue(const FdmLinearOpIterator&, Time t) {
            Size i = (Size) t;
            return powerPrices[i] - heatRate_*gasPrices[i];
        }
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) {
            return innerValue(iter, t);
        }
      private:
        const Real heatRate_;
    };
}

void SwingOptionTest::testVPPStepCondition() {

    BOOST_MESSAGE("Testing VPP step condition");

    SavedSettings backup;

    const Real pMin           = 8;
    const Real pMax           = 40;
    const Size tMinUp         = 6;
    const Size tMinDown       = 2;
    const Real startUpFuel    = 20;
    const Real startUpFixCost = 100;
    const Real carbonPrice    = 3.0;
    const Size stateDirection = 0;

    const Size nStates = 2*tMinUp + tMinDown;
    const std::vector<Size> dim(1, nStates);
    const boost::shared_ptr<FdmLinearOpLayout> layout(
                                                   new FdmLinearOpLayout(dim));

    const boost::shared_ptr<FdmMesher> mesher(
        new FdmMesherComposite(layout,
             std::vector<boost::shared_ptr<Fdm1dMesher> >(1,
                                boost::shared_ptr<Fdm1dMesher>(
                                    new Uniform1dMesher(0.0, 1.0, nStates)))));

    const boost::shared_ptr<FdmInnerValueCalculator> gasPrice(new GasPrice());

    const Real efficiency[] = { 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.9 };

    // Expected values are calculated using mixed integer programming
    // based on the gnu linear programming toolkit. For details please see:
    // http://spanderen.de/
    //        2011/06/23/vpp-pricing-ii-mixed-integer-linear-programming/
    const Real expected[] = { 0.0, 2056.04, 11145.577778, 26418.24,
                              44512.461818, 62000.626667, 137591.911111};

    for (Size i=0; i < LENGTH(efficiency); ++i) {
        const Real heatRate = 1.0/efficiency[i];
        const boost::shared_ptr<FdmInnerValueCalculator> sparkSpreadPrice(
                                              new SparkSpreadPrice(heatRate));

        boost::shared_ptr<StepCondition<Array> > stepCondition(
            new FdmVPPStepCondition(heatRate, pMin, pMax, tMinUp, tMinDown,
                                    startUpFuel, startUpFixCost,
                                    carbonPrice, stateDirection,
                                    mesher, gasPrice, sparkSpreadPrice));

        Array state(nStates, 0.0);
        for (Size j=LENGTH(gasPrices); j > 0; --j) {
            stepCondition->applyTo(state, (Time) j-1);
        }
        const Real calculated = *std::max_element(state.begin(), state.end());

        if (std::fabs(expected[i] - calculated) > 1e-6) {
            BOOST_ERROR("Failed to reproduce reference values"
                       << "\n    calculated:   " << calculated
                       << "\n    expected(MC): " << expected[i]);

        }
    }
}


test_suite* SwingOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Spark-Option Test");
    suite->add(QUANTLIB_TEST_CASE(
		&SwingOptionTest::testExtendedOrnsteinUhlenbeckProcess));
	suite->add(QUANTLIB_TEST_CASE(
		&SwingOptionTest::testGemanRoncoroniProcess));
    suite->add(QUANTLIB_TEST_CASE(&SwingOptionTest::testFdBSSwingOption));
    suite->add(QUANTLIB_TEST_CASE(
                          &SwingOptionTest::testFdmExponentialJump1dMesher));
    suite->add(QUANTLIB_TEST_CASE(&SwingOptionTest::testExtOUJumpVanillaEngine));
    suite->add(QUANTLIB_TEST_CASE(
                            &SwingOptionTest::testExtOUJumpSwingOption));
    suite->add(QUANTLIB_TEST_CASE(
                            &SwingOptionTest::testSimpleExtOUStorageEngine));
    suite->add(QUANTLIB_TEST_CASE(
                            &SwingOptionTest::testKlugeExtOUSpreadOption));
    suite->add(QUANTLIB_TEST_CASE(
                            &SwingOptionTest::testVPPStepCondition));
    return suite;
}


