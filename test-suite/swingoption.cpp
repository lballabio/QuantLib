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
#include <ql/experimental/finitedifferences/fdextoujumpvanillaengine.hpp>
#include <ql/experimental/finitedifferences/fdsimpleextoujumpswingengine.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/instruments/vanillaswingoption.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/factorial.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/richardsonextrapolation.hpp>
#include <ql/math/statistics/generalstatistics.hpp>
#include <ql/methods/finitedifferences/meshers/exponentialjump1dmesher.hpp>
#include <ql/methods/montecarlo/multipathgenerator.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdsimplebsswingengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <utility>


using namespace QuantLib;
using namespace boost::unit_test_framework;


namespace swing_option_test {

    ext::function<Real(Real)> constant_b(Real b) {
        return [=](Real x){ return b; };
    }

    ext::shared_ptr<ExtOUWithJumpsProcess> createKlugeProcess() {
        Array x0(2);
        x0[0] = 3.0; x0[1] = 0.0;

        const Real beta = 5.0;
        const Real eta  = 2.0;
        const Real jumpIntensity = 1.0;
        const Real speed = 1.0;
        const Real volatility = 2.0;

        ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> ouProcess(
            new ExtendedOrnsteinUhlenbeckProcess(speed, volatility, x0[0],
                                                 constant_b(x0[0])));
        return ext::make_shared<ExtOUWithJumpsProcess>(
            ouProcess, x0[1], beta, jumpIntensity, eta);
    }
}

void SwingOptionTest::testExtendedOrnsteinUhlenbeckProcess() {

    BOOST_TEST_MESSAGE("Testing extended Ornstein-Uhlenbeck process...");

    const Real speed = 2.5;
    const Volatility vol = 0.70;
    const Real level = 1.43;

    ExtendedOrnsteinUhlenbeckProcess::Discretization discr[] = {
        ExtendedOrnsteinUhlenbeckProcess::MidPoint,
        ExtendedOrnsteinUhlenbeckProcess::Trapezodial,
        ExtendedOrnsteinUhlenbeckProcess::GaussLobatto};

    ext::function<Real (Real)> f[] 
        = { [=](Real x) -> Real { return level; },
            [ ](Real x) -> Real { return x + 1.0; },
            [ ](Real x) -> Real { return std::sin(x); }}; 

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

            PseudoRandom::rng_type rng(PseudoRandom::urng_type(1234U));

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



void SwingOptionTest::testFdmExponentialJump1dMesher() {

    BOOST_TEST_MESSAGE("Testing finite difference mesher for the Kluge model...");

    using namespace swing_option_test;

    Array x(2, 1.0);
    const Real beta = 100.0;
    const Real eta  = 1.0/0.4;
    const Real jumpIntensity = 4.0;
    const Size dummySteps  = 2;

    ExponentialJump1dMesher mesher(dummySteps, beta, jumpIntensity, eta);

    ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> ouProcess(
        new ExtendedOrnsteinUhlenbeckProcess(1.0, 1.0, x[0],
                                             constant_b(1.0)));
    ext::shared_ptr<ExtOUWithJumpsProcess> jumpProcess(
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

        auto iter = std::lower_bound(path.begin(), path.end(), x);
        const Real q = std::distance(path.begin(), iter)/Real(n);
        QL_REQUIRE(std::fabs(q - v) < relTol1
                   || ((v < threshold) && std::fabs(q-v) < relTol2),
                    "can not reproduce jump distribution");
    }
}

void SwingOptionTest::testExtOUJumpVanillaEngine() {

    BOOST_TEST_MESSAGE("Testing finite difference pricer for the Kluge model...");

    using namespace swing_option_test;

    ext::shared_ptr<ExtOUWithJumpsProcess> jumpProcess = createKlugeProcess();

    const Date today = Date::todaysDate();
    Settings::instance().evaluationDate() = today;

    const DayCounter dc = ActualActual(ActualActual::ISDA);
    const Date maturityDate = today + Period(12, Months);
    const Time maturity = dc.yearFraction(today, maturityDate);

    const Rate irRate = 0.1;
    ext::shared_ptr<YieldTermStructure> rTS(flatRate(today, irRate, dc));
    ext::shared_ptr<StrikedTypePayoff> payoff(
                                     new PlainVanillaPayoff(Option::Call, 30));
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(maturityDate));

    ext::shared_ptr<PricingEngine> engine(
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

    BOOST_TEST_MESSAGE("Testing Black-Scholes vanilla swing option pricing...");

    Date settlementDate = Date::todaysDate();
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual(ActualActual::ISDA);
    Date maturityDate = settlementDate + Period(12, Months);

    Real strike = 30;
    ext::shared_ptr<StrikedTypePayoff> payoff(
        new PlainVanillaPayoff(Option::Put, strike));
    ext::shared_ptr<StrikedTypePayoff> forward(
        new VanillaForwardPayoff(Option::Put, strike));

    std::vector<Date> exerciseDates(1, settlementDate+Period(1, Months));
    while (exerciseDates.back() < maturityDate) {
        exerciseDates.push_back(exerciseDates.back()+Period(1, Months));
    }

    ext::shared_ptr<SwingExercise> swingExercise(
                                            new SwingExercise(exerciseDates));

    Handle<YieldTermStructure> riskFreeTS(flatRate(0.14, dayCounter));
    Handle<YieldTermStructure> dividendTS(flatRate(0.02, dayCounter));
    Handle<BlackVolTermStructure> volTS(
                                    flatVol(settlementDate, 0.4, dayCounter));

    Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(30.0)));

    ext::shared_ptr<BlackScholesMertonProcess> process(
            new BlackScholesMertonProcess(s0, dividendTS, riskFreeTS, volTS));
    ext::shared_ptr<PricingEngine> engine(
                                new FdSimpleBSSwingEngine(process, 50, 200));
    
    VanillaOption bermudanOption(payoff, swingExercise);
    bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                          new FdBlackScholesVanillaEngine(process, 50, 200)));
    const Real bermudanOptionPrices = bermudanOption.NPV();
    
    for (Size i=0; i < exerciseDates.size(); ++i) {
        const Size exerciseRights = i+1;
        
        VanillaSwingOption swingOption(forward, swingExercise,
        		                       0, exerciseRights);
        swingOption.setPricingEngine(engine);
        const Real swingOptionPrice = swingOption.NPV();

        const Real upperBound = exerciseRights*bermudanOptionPrices;

        if (swingOptionPrice - upperBound > 0.01) {
            BOOST_ERROR("Failed to reproduce upper bounds"
                        << "\n    upper Bound: " << upperBound
                        << "\n    Price:       " << swingOptionPrice
                        << "\n    diff:        " << swingOptionPrice - upperBound);
        }
        
        Real lowerBound = 0.0;
        for (Size j=exerciseDates.size()-i-1; j < exerciseDates.size(); ++j) {
            VanillaOption europeanOption(payoff, ext::shared_ptr<Exercise>(
                                     new EuropeanExercise(exerciseDates[j])));
            europeanOption.setPricingEngine(
                ext::shared_ptr<PricingEngine>(
                                          new AnalyticEuropeanEngine(process)));
            lowerBound += europeanOption.NPV();
        }

        if (lowerBound - swingOptionPrice > 4e-2) {
            BOOST_ERROR("Failed to reproduce lower bounds"
                        << "\n    lower Bound: " << lowerBound
                        << "\n    Price:       " << swingOptionPrice
                        << "\n    diff:        " << lowerBound - swingOptionPrice);
        }
    }
}


void SwingOptionTest::testExtOUJumpSwingOption() {

    BOOST_TEST_MESSAGE("Testing simple swing option pricing for Kluge model...");

    using namespace swing_option_test;

    Date settlementDate = Date::todaysDate();
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual(ActualActual::ISDA);
    Date maturityDate = settlementDate + Period(12, Months);

    Real strike = 30;
    ext::shared_ptr<StrikedTypePayoff> payoff(
        new PlainVanillaPayoff(Option::Put, strike));
    ext::shared_ptr<StrikedTypePayoff> forward(
        new VanillaForwardPayoff(Option::Put, strike));

    std::vector<Date> exerciseDates(1, settlementDate+Period(1, Months));
    while (exerciseDates.back() < maturityDate) {
        exerciseDates.push_back(exerciseDates.back()+Period(1, Months));
    }
    ext::shared_ptr<SwingExercise> swingExercise(
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

    ext::shared_ptr<ExtOUWithJumpsProcess> jumpProcess = createKlugeProcess();

    const Rate irRate = 0.1;
    ext::shared_ptr<YieldTermStructure> rTS(
                                flatRate(settlementDate, irRate, dayCounter));

    ext::shared_ptr<PricingEngine> swingEngine(
                new FdSimpleExtOUJumpSwingEngine(jumpProcess, rTS, 25, 50, 25));

    ext::shared_ptr<PricingEngine> vanillaEngine(
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

        VanillaSwingOption swingOption(forward, swingExercise,
                                       0, exerciseRights);
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
            VanillaOption europeanOption(payoff, ext::shared_ptr<Exercise>(
                                     new EuropeanExercise(exerciseDates[j])));
            europeanOption.setPricingEngine(
                ext::shared_ptr<PricingEngine>(vanillaEngine));
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
            std::sort(exerciseValues.begin(), exerciseValues.end(), std::greater<>());

            Real npCashFlows
                = std::accumulate(exerciseValues.begin(),
                                  exerciseValues.begin()+exerciseRights, Real(0.0));
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

namespace swing_option_test {
    class SwingPdePricing {
      public:
        typedef FdSimpleExtOUJumpSwingEngine::Shape Shape;

        SwingPdePricing(ext::shared_ptr<ExtOUWithJumpsProcess> process,
                        ext::shared_ptr<VanillaOption> option,
                        ext::shared_ptr<Shape> shape)
        : process_(std::move(process)), option_(std::move(option)), shape_(std::move(shape)) {}

        Real operator()(Real x) const {
            const ext::shared_ptr<YieldTermStructure> rTS(
                flatRate(0.0, Actual365Fixed()));

            const Size gridX = 200;
            const Size gridY = 100;
            const Size gridT = 100;

            option_->setPricingEngine(
                ext::make_shared<FdExtOUJumpVanillaEngine>(
                    process_, rTS,
                    Size(gridT/x), Size(gridX/x), Size(gridY/x), shape_));

            return option_->NPV();
        }

      private:
        const ext::shared_ptr<ExtOUWithJumpsProcess> process_;
        const ext::shared_ptr<VanillaOption> option_;
        const ext::shared_ptr<Shape> shape_;
    };
}

void SwingOptionTest::testKlugeChFVanillaPricing() {
    BOOST_TEST_MESSAGE("Testing Kluge PDE Vanilla Pricing in"
            " comparison to moment matching...");

    using namespace swing_option_test;

    Date settlementDate = Date(22, November, 2019);
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = Actual365Fixed();
    Date maturityDate = settlementDate + Period(6, Months);
    const Time t = dayCounter.yearFraction(settlementDate, maturityDate);

    const Real f0 = 30;

    const Real x0 = 0.0;
    const Real y0 = 0.0;

    const Real beta = 5.0;
    const Real eta  = 5.0;
    const Real lambda = 4.0;
    const Real alpha = 4.0;
    const Real sig = 1.0;

    const ext::shared_ptr<ExtOUWithJumpsProcess> klugeProcess =
        ext::make_shared<ExtOUWithJumpsProcess>(
            ext::make_shared<ExtendedOrnsteinUhlenbeckProcess>(
                    alpha, sig, x0, constant_b(0.0)),
            y0, beta, lambda, eta);

    const Real strike = f0;

    const ext::shared_ptr<VanillaOption> option =
        ext::make_shared<VanillaOption>(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
            ext::make_shared<EuropeanExercise>(maturityDate));

    typedef FdSimpleExtOUJumpSwingEngine::Shape Shape;
    const ext::shared_ptr<Shape> shape(ext::make_shared<Shape>());

    const Real ps = std::log(f0)
        - sig*sig/(4*alpha)*(1-std::exp(-2*alpha*t))
        - lambda/beta*std::log((eta-std::exp(-beta*t))/(eta-1.0));

    shape->emplace_back(t, ps);

    const Real expected =
        RichardsonExtrapolation(
            SwingPdePricing(klugeProcess, option, shape), 4.0)(2.0, 1.5);

    const Real stdDev = std::sqrt((((2 - 2*std::exp(-2*beta*t))*lambda)
            /(beta*eta*eta) + ((1 - std::exp(-2*alpha*t))*sig*sig)/alpha)/2.);

    const Real bsNPV = blackFormula(Option::Call, strike, f0, stdDev);

    const Real g1 = ((2 - 2*std::exp(-3*beta*t))*lambda)/(beta*eta*eta*eta)
        / (stdDev*stdDev*stdDev);

    const Real g2 = 3*(std::exp((alpha + beta)*t)
        *  squared(2*alpha*std::exp(2*alpha*t)*(-1 + std::exp(2*beta*t))
                  *lambda + beta*std::exp(2*beta*t)*(-1 + std::exp(2*alpha*t))
                  *eta*eta*sig*sig)
            + 16*alpha*alpha*beta*std::exp((5*alpha + 3*beta)*t)*lambda
                *std::sinh(2*beta*t))
        / (4.*alpha*alpha*beta*beta
                *std::exp(5*(alpha + beta)*t)*eta*eta*eta*eta)
        / (stdDev*stdDev*stdDev*stdDev) - 3.0;

    const Real d = (std::log(f0/strike) + 0.5*stdDev*stdDev)/stdDev;

    // Jurczenko E., Maillet B. and Negrea B.,
    // Multi-Moment Approximate Option Pricing Models:
    // A General Comparison (Part 1)
    // https://papers.ssrn.com/sol3/papers.cfm?abstract_id=300922
    const NormalDistribution n;
    const Real q3 = 1/Factorial::get(3)*f0*stdDev*(2*stdDev - d)*n(d);
    const Real q4 = 1/Factorial::get(4)*f0*stdDev*(d*d - 3*d*stdDev - 1)*n(d);
    const Real q5 = 10/Factorial::get(6)*f0*stdDev*(
        d*d*d*d - 5*d*d*d*stdDev - 6*d*d + 15*d*stdDev + 3)*n(d);

    // Corrado C. and T. Su, (1996-b),
    // “Skewness and Kurtosis in S&P 500 IndexReturns Implied by Option Prices”,
    // Journal of Financial Research 19 (2), 175-192.
    const Real ccs3 = bsNPV + g1*q3;
    const Real ccs4 = ccs3 + g2*q4;

    // Rubinstein M., (1998), “Edgeworth Binomial Trees”,
    // Journal of Derivatives 5 (3), 20-27.
    const Real cr = ccs4 + g1*g1*q5;

    const Volatility expectedImplVol = blackFormulaImpliedStdDevLiRS(
        Option::Call, strike, f0, expected, 1.0)/std::sqrt(t);

    const Volatility bsImplVol = blackFormulaImpliedStdDevLiRS(
        Option::Call, strike, f0, bsNPV, 1.0)/std::sqrt(t);

    const Volatility ccs3ImplVol = blackFormulaImpliedStdDevLiRS(
        Option::Call, strike, f0, ccs3, 1.0)/std::sqrt(t);

    const Volatility ccs4ImplVol = blackFormulaImpliedStdDevLiRS(
        Option::Call, strike, f0, ccs4, 1.0)/std::sqrt(t);

    const Volatility crImplVol = blackFormulaImpliedStdDevLiRS(
        Option::Call, strike, f0, cr, 1.0)/std::sqrt(t);

    const Real tol[] = {0.01, 0.0075, 0.005, 0.004};
    const std::string methods[] = {
        "Second Order", "Third Order", "Fourth Order", "Rubinstein"};

    const Real calculated[] = {bsImplVol, ccs3ImplVol, ccs4ImplVol, crImplVol};

    for (Size i=0; i < 4; ++i) {
        const Real diff = std::fabs(calculated[i] - expectedImplVol);
        if (diff > tol[i]) {
            BOOST_ERROR("failed to reproduce vanilla option implied volatility "
                    "with moment matching"
                    << "\n    calculated: " << calculated[i]
                    << "\n    expected:   " << expectedImplVol
                    << "\n    difference: " << diff
                    << "\n    tolerance:  " << tol[i]
                    << "\n    method:     " << methods[i]);
        }
    }
}

test_suite* SwingOptionTest::suite(SpeedLevel speed) {
    auto* suite = BOOST_TEST_SUITE("Swing-Option Test");

    suite->add(QUANTLIB_TEST_CASE(
        &SwingOptionTest::testExtendedOrnsteinUhlenbeckProcess));
    suite->add(QUANTLIB_TEST_CASE(&SwingOptionTest::testFdBSSwingOption));
    suite->add(QUANTLIB_TEST_CASE(
                          &SwingOptionTest::testFdmExponentialJump1dMesher));
    suite->add(QUANTLIB_TEST_CASE(
                          &SwingOptionTest::testKlugeChFVanillaPricing));

    if (speed <= Fast) {
        suite->add(QUANTLIB_TEST_CASE(
            &SwingOptionTest::testExtOUJumpVanillaEngine));
        suite->add(QUANTLIB_TEST_CASE(
            &SwingOptionTest::testExtOUJumpSwingOption));
    }

    return suite;
}


