/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010, 2011, 2012 Klaus Spanderen

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

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/finitedifferences/dynprogvppintrinsicvalueengine.hpp>
#include <ql/experimental/finitedifferences/fdklugeextouspreadengine.hpp>
#include <ql/experimental/finitedifferences/fdmklugeextouop.hpp>
#include <ql/experimental/finitedifferences/fdmspreadpayoffinnervalue.hpp>
#include <ql/experimental/finitedifferences/fdmvppstepconditionfactory.hpp>
#include <ql/experimental/finitedifferences/fdsimpleextoustorageengine.hpp>
#include <ql/experimental/finitedifferences/fdsimpleklugeextouvppengine.hpp>
#include <ql/experimental/finitedifferences/vanillavppoption.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/experimental/processes/gemanroncoroniprocess.hpp>
#include <ql/experimental/processes/klugeextouprocess.hpp>
#include <ql/instruments/basketoption.hpp>
#include <ql/instruments/vanillastorageoption.hpp>
#include <ql/instruments/vanillaswingoption.hpp>
#include <ql/math/generallinearleastsquares.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/statistics/generalstatistics.hpp>
#include <ql/math/functional.hpp>
#include <ql/methods/finitedifferences/meshers/exponentialjump1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmsimpleprocess1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/montecarlo/lsmbasissystem.hpp>
#include <ql/methods/montecarlo/multipathgenerator.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <deque>
#include <utility>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(VppTests)

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

class linear {
    Real alpha, beta;
  public:
    linear(Real alpha, Real beta) : alpha(alpha), beta(beta) {}
    Real operator()(Real x) const {
        return alpha + beta*x;
    }
};

// for a "real" gas and power forward curve
// please see. e.g. http://www.kyos.com/?content=64
const std::vector<Real> fuelPrices = {
    20.74,21.65,20.78,21.58,21.43,20.82,22.02,21.52,
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

const std::vector<Real> powerPrices = {
    40.40,36.71,31.87,25.81,31.61,35.00,46.22,60.68,
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

class PathFuelPrice : public FdmInnerValueCalculator {
  public:
    typedef FdSimpleKlugeExtOUVPPEngine::Shape Shape;

    PathFuelPrice(const MultiPathGenerator<PseudoRandom>::sample_type::value_type& path,
                  ext::shared_ptr<Shape> shape)
    : path_(path), shape_(std::move(shape)) {}
    Real innerValue(const FdmLinearOpIterator&, Time t) override {
        QL_REQUIRE(t-std::sqrt(QL_EPSILON) <=  shape_->back().first,
                   "invalid time");

        const Size i = Size(t * 365U * 24U);
        const Real f = std::lower_bound(shape_->begin(), shape_->end(),
                                        std::pair<Time, Real>(t-std::sqrt(QL_EPSILON), 0.0))->second;

        return std::exp(path_[2][i] + f);
    }
    Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override {
        return innerValue(iter, t);
    }

  private:
    const MultiPathGenerator<PseudoRandom>::sample_type::value_type& path_;
    const ext::shared_ptr<Shape> shape_;
};

class PathSparkSpreadPrice : public FdmInnerValueCalculator {
  public:
    typedef FdSimpleKlugeExtOUVPPEngine::Shape Shape;

    PathSparkSpreadPrice(Real heatRate,
                         const MultiPathGenerator<PseudoRandom>::sample_type::value_type& path,
                         ext::shared_ptr<Shape> fuelShape,
                         ext::shared_ptr<Shape> powerShape)
    : heatRate_(heatRate), path_(path), fuelShape_(std::move(fuelShape)),
      powerShape_(std::move(powerShape)) {}

    Real innerValue(const FdmLinearOpIterator&, Time t) override {
        QL_REQUIRE(t-std::sqrt(QL_EPSILON) <=  powerShape_->back().first,
                   "invalid time");

        const Size i = Size(t * 365U * 24U);
        const Real f = std::lower_bound(
                               powerShape_->begin(), powerShape_->end(),
                               std::pair<Time, Real>(t-std::sqrt(QL_EPSILON), 0.0))->second;
        const Real g = std::lower_bound(
                               fuelShape_->begin(),fuelShape_->end(),
                               std::pair<Time, Real>(t-std::sqrt(QL_EPSILON), 0.0))->second;

        return std::exp(f + path_[0][i]+path_[1][i])
            - heatRate_*std::exp(g + path_[2][i]);
    }
    Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override {
        return innerValue(iter, t);
    }

  private:
    const Real heatRate_;
    const MultiPathGenerator<PseudoRandom>::sample_type::value_type& path_;
    const ext::shared_ptr<Shape> fuelShape_;
    const ext::shared_ptr<Shape> powerShape_;
};

ext::shared_ptr<KlugeExtOUProcess> createKlugeExtOUProcess() {
    // model definition
    const Real beta         = 200;
    const Real eta          = 1.0/0.2;
    const Real lambda       = 4.0;
    const Real alpha        = 7.0;
    const Real volatility_x = 1.4;
    const Real kappa        = 4.45;
    const Real volatility_u = std::sqrt(1.3);
    const Real rho          = 0.7;

    Array x0(2);
    x0[0] = 0.0; x0[1] = 0.0;

    const ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> ouProcess(
            new ExtendedOrnsteinUhlenbeckProcess(alpha, volatility_x, x0[0],
                                                 constant_b(x0[0])));
    const ext::shared_ptr<ExtOUWithJumpsProcess> lnPowerProcess(
            new ExtOUWithJumpsProcess(ouProcess, x0[1], beta, lambda, eta));

    const Real u=0.0;
    const ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> lnGasProcess(
            new ExtendedOrnsteinUhlenbeckProcess(kappa, volatility_u, u,
                                                 constant_b(u)));

    ext::shared_ptr<KlugeExtOUProcess> klugeOUProcess(
            new KlugeExtOUProcess(rho, lnPowerProcess, lnGasProcess));

    return klugeOUProcess;
}


BOOST_AUTO_TEST_CASE(testGemanRoncoroniProcess) {

    BOOST_TEST_MESSAGE("Testing Geman-Roncoroni process...");

    /* Example induced by H. Geman, A. Roncoroni,
       "Understanding the Fine Structure of Electricity Prices",
       http://papers.ssrn.com/sol3/papers.cfm?abstract_id=638322
       Results are verified against the authors MatLab-Code.
       http://semeq.unipmn.it/files/Ch19_spark_spread.zip
    */

    const Date today = Date(18, December, 2011);
    Settings::instance().evaluationDate() = today;
    const DayCounter dc = ActualActual(ActualActual::ISDA);

    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, 0.03, dc);

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

    ext::shared_ptr<GemanRoncoroniProcess> grProcess(
                new GemanRoncoroniProcess(x0, alpha, beta, gamma, delta,
                                          eps, zeta, d, k, tau, sig2, a, b,
                                          theta1, theta2, theta3, psi));


    const Real speed     = 5.0;
    const Volatility vol = std::sqrt(1.4);
    const Real betaG     = 0.08;
    const Real alphaG    = 1.0;
    const Real x0G       = 1.1;

    ext::function<Real (Real)> f = linear(alphaG, betaG);

    ext::shared_ptr<StochasticProcess1D> eouProcess(
        new ExtendedOrnsteinUhlenbeckProcess(speed, vol, x0G, f,
                           ExtendedOrnsteinUhlenbeckProcess::Trapezodial));

    std::vector<ext::shared_ptr<StochasticProcess1D> > processes = {grProcess, eouProcess};

    Matrix correlation(2, 2, 1.0);
    correlation[0][1] = correlation[1][0] = 0.25;

    ext::shared_ptr<StochasticProcess> pArray(
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

            const Real fuelPrice         = std::exp(path.value[1][i]);
            const Real electricityPrice = std::exp(path.value[0][i]);

            const Real sparkSpread = electricityPrice - heatRate*fuelPrice;
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

BOOST_AUTO_TEST_CASE(testSimpleExtOUStorageEngine) {

    BOOST_TEST_MESSAGE("Testing simple-storage option based on ext. OU model...");

    Date settlementDate = Date(18, December, 2011);
    Settings::instance().evaluationDate() = settlementDate;
    DayCounter dayCounter = ActualActual(ActualActual::ISDA);
    Date maturityDate = settlementDate + Period(12, Months);

    std::vector<Date> exerciseDates(1, settlementDate+Period(1, Days));
    while (exerciseDates.back() < maturityDate) {
        exerciseDates.push_back(exerciseDates.back()+Period(1, Days));
    }
    ext::shared_ptr<BermudanExercise> bermudanExercise(
                                        new BermudanExercise(exerciseDates));

    const Real x0 = 3.0;
    const Real speed = 1.0;
    const Real volatility = 0.5;
    const Rate irRate = 0.1;

    ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> ouProcess(
        new ExtendedOrnsteinUhlenbeckProcess(speed, volatility, x0,
                                             constant_b(x0)));

    ext::shared_ptr<YieldTermStructure> rTS(
                                flatRate(settlementDate, irRate, dayCounter));

    ext::shared_ptr<PricingEngine> storageEngine(
               new FdSimpleExtOUStorageEngine(ouProcess, rTS, 1, 25));

    VanillaStorageOption storageOption(bermudanExercise, 50, 0, 1);

    storageOption.setPricingEngine(storageEngine);

    const Real expected = 69.5755;
    const Real calculated = storageOption.NPV();

    if (std::fabs(expected - calculated) > 5e-2) {
        BOOST_ERROR("Failed to reproduce cached values" <<
                    "\n calculated: " << calculated <<
                    "\n   expected: " << expected);
    }
}

BOOST_AUTO_TEST_CASE(testKlugeExtOUSpreadOption) {

    BOOST_TEST_MESSAGE("Testing simple Kluge ext-Ornstein-Uhlenbeck spread option...");

    Date settlementDate = Date(18, December, 2011);
    Settings::instance().evaluationDate() = settlementDate;

    DayCounter dayCounter = ActualActual(ActualActual::ISDA);
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

    ext::shared_ptr<ExtOUWithJumpsProcess>
                                           klugeProcess = createKlugeProcess();
    ext::function<Real (Real)> f = linear(alphaG, betaG);

    ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> extOUProcess(
        new ExtendedOrnsteinUhlenbeckProcess(speed, vol, x0G, f,
                           ExtendedOrnsteinUhlenbeckProcess::Trapezodial));

    ext::shared_ptr<YieldTermStructure> rTS(
                                flatRate(settlementDate, irRate, dayCounter));

    ext::shared_ptr<KlugeExtOUProcess> klugeOUProcess(
                    new KlugeExtOUProcess(rho, klugeProcess, extOUProcess));


    ext::shared_ptr<Payoff> payoff(new PlainVanillaPayoff(Option::Call, 0.0));

    Array spreadFactors(2);
    spreadFactors[0] = 1.0; spreadFactors[1] = -heatRate;
    ext::shared_ptr<BasketPayoff> basketPayoff(
                               new AverageBasketPayoff(payoff, spreadFactors));

    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(maturityDate));

    BasketOption option(basketPayoff, exercise);
    option.setPricingEngine(ext::shared_ptr<PricingEngine>(
        new FdKlugeExtOUSpreadEngine(klugeOUProcess, rTS,
                                     5, 200, 50, 20)));

    TimeGrid grid(maturity, 50);
    typedef PseudoRandom::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;

    rsg_type rsg = PseudoRandom::make_sequence_generator(
        klugeOUProcess->factors() * (grid.size() - 1), 1234UL);

    MultiPathGenerator<rsg_type> generator(klugeOUProcess, grid, rsg, false);


    GeneralStatistics npv;
    const Size nTrails = 20000;
    for (Size i=0; i < nTrails; ++i) {
        const sample_type& path = generator.next();

        Array p(2);
        p[0] = path.value[0].back() + path.value[1].back();
        p[1] = path.value[2].back();
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

BOOST_AUTO_TEST_CASE(testVPPIntrinsicValue) {

    BOOST_TEST_MESSAGE("Testing VPP step condition...");

    const Date today = Date(18, December, 2011);
    const DayCounter dc = ActualActual(ActualActual::ISDA);
    Settings::instance().evaluationDate() = today;

    // vpp parameters
    const Real pMin           = 8;
    const Real pMax           = 40;
    const Size tMinUp         = 2;
    const Size tMinDown       = 2;
    const Real startUpFuel    = 20;
    const Real startUpFixCost = 100;
    const Real fuelCostAddon    = 3.0;

    const ext::shared_ptr<SwingExercise> exercise(new SwingExercise(today, today + 6, 3600U));

    // Expected values are calculated using mixed integer programming
    // based on the gnu linear programming toolkit. For details please see:
    // http://spanderen.de/
    //        2011/06/23/vpp-pricing-ii-mixed-integer-linear-programming/
    const Real efficiency[] = { 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.9 };
    const Real expected[] = { 0.0, 2056.04, 11145.577778, 26452.04,
                              44512.461818, 62000.626667, 137591.911111};

    for (Size i=0; i < LENGTH(efficiency); ++i) {
        const Real heatRate = 1.0/efficiency[i];

        VanillaVPPOption option(heatRate, pMin, pMax, tMinUp, tMinDown,
                                startUpFuel, startUpFixCost, exercise);

        option.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new DynProgVPPIntrinsicValueEngine(fuelPrices, powerPrices,
                                               fuelCostAddon, flatRate(0.0, dc))));

        const Real calculated = option.NPV();

        if (std::fabs(expected[i] - calculated) > 1e-4) {
            BOOST_ERROR("Failed to reproduce reference values"
                       << "\n    calculated: " << calculated
                       << "\n    expected:   " << expected[i]);

        }
    }
}

BOOST_AUTO_TEST_CASE(testVPPPricing, *precondition(if_speed(Slow))) {
    BOOST_TEST_MESSAGE("Testing VPP pricing using perfect foresight or FDM...");

    const Date today = Date(18, December, 2011);
    const DayCounter dc = ActualActual(ActualActual::ISDA);
    Settings::instance().evaluationDate() = today;

    // vpp parameter
    const Real heatRate       = 2.5;
    const Real pMin           = 8;
    const Real pMax           = 40;
    const Size tMinUp         = 6;
    const Size tMinDown       = 2;
    const Real startUpFuel    = 20;
    const Real startUpFixCost = 100;

    const ext::shared_ptr<SwingExercise> exercise(new SwingExercise(today, today + 6, 3600U));

    VanillaVPPOption vppOption(heatRate, pMin, pMax, tMinUp, tMinDown,
                               startUpFuel, startUpFixCost, exercise);

    const ext::shared_ptr<KlugeExtOUProcess> klugeOUProcess
        = createKlugeExtOUProcess();
    const ext::shared_ptr<ExtOUWithJumpsProcess> lnPowerProcess
        = klugeOUProcess->getKlugeProcess();
    const ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> ouProcess
        = lnPowerProcess->getExtendedOrnsteinUhlenbeckProcess();
    const ext::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> lnGasProcess
        = klugeOUProcess->getExtOUProcess();

    const Real beta         = lnPowerProcess->beta();
    const Real eta          = lnPowerProcess->eta();
    const Real lambda       = lnPowerProcess->jumpIntensity();
    const Real alpha        = ouProcess->speed();
    const Real volatility_x = ouProcess->volatility();
    const Real kappa        = lnGasProcess->speed();
    const Real volatility_u = lnGasProcess->volatility();

    const Rate irRate       = 0.00;
    const Real fuelCostAddon  = 3.0;

    const ext::shared_ptr<YieldTermStructure> rTS
        = flatRate(today, irRate, dc);

    const Size nHours = powerPrices.size();

    typedef FdSimpleKlugeExtOUVPPEngine::Shape Shape;
    ext::shared_ptr<Shape> fuelShape(new Shape(nHours));
    ext::shared_ptr<Shape> powerShape(new Shape(nHours));

    for (Size i=0; i < nHours; ++i) {
        const Time t = (i+1)/(365*24.);

        const Real fuelPrice = fuelPrices[i];
        const Real gs = std::log(fuelPrice)-squared(volatility_u)
                               /(4*kappa)*(1-std::exp(-2*kappa*t));
        (*fuelShape)[i] = Shape::value_type(t, gs);

        const Real powerPrice = powerPrices[i];
        const Real ps = std::log(powerPrice)-squared(volatility_x)
                 /(4*alpha)*(1-std::exp(-2*alpha*t))
                -lambda/beta*std::log((eta-std::exp(-beta*t))/(eta-1.0));

        (*powerShape)[i] = Shape::value_type(t, ps);
    }

    // Test: intrinsic value
    vppOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        new DynProgVPPIntrinsicValueEngine(fuelPrices, powerPrices,
                                           fuelCostAddon, flatRate(0.0, dc))));

    const Real intrinsic = vppOption.NPV();
    const Real expectedIntrinsic = 2056.04;
    if (std::fabs(intrinsic - expectedIntrinsic) > 0.1) {
        BOOST_ERROR("Failed to reproduce intrinsic value"
                   << "\n    calculated: " << intrinsic
                   << "\n    expected  : " << expectedIntrinsic);
    }

    // Test: finite difference price
    const ext::shared_ptr<PricingEngine> engine(
        new FdSimpleKlugeExtOUVPPEngine(klugeOUProcess, rTS,
                                        fuelShape, powerShape, fuelCostAddon,
                                        1, 25, 11, 10));

    vppOption.setPricingEngine(engine);

    const Real fdmPrice = vppOption.NPV();
    const Real expectedFdmPrice = 5217.68;
    if (std::fabs(fdmPrice - expectedFdmPrice) > 0.1) {
       BOOST_ERROR("Failed to reproduce finite difference price"
                   << "\n    calculated: " << fdmPrice
                   << "\n    expected  : " << expectedFdmPrice);
    }

    // Test: Monte-Carlo perfect foresight price
    VanillaVPPOption::arguments args;
    vppOption.setupArguments(&args);

    const FdmVPPStepConditionFactory stepConditionFactory(args);

    const ext::shared_ptr<FdmMesher> oneDimMesher(new FdmMesherComposite(
        stepConditionFactory.stateMesher()));
    const Size nStates = oneDimMesher->layout()->dim()[0];

    const FdmVPPStepConditionMesher vppMesh = {0U, oneDimMesher};

    const TimeGrid grid(dc.yearFraction(today, exercise->lastDate()+1),
                        exercise->dates().size());
    typedef PseudoRandom::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;

    rsg_type rsg = PseudoRandom::make_sequence_generator(
        klugeOUProcess->factors() * (grid.size() - 1), 1234UL);
    MultiPathGenerator<rsg_type> generator(klugeOUProcess, grid, rsg, false);

    GeneralStatistics npv;
    const Size nTrails = 2500;

    for (Size i=0; i < nTrails; ++i) {
        const sample_type& path = generator.next();
        const ext::shared_ptr<FdmVPPStepCondition> stepCondition(
            stepConditionFactory.build(
                vppMesh, fuelCostAddon,
                ext::shared_ptr<FdmInnerValueCalculator>(
                    new PathFuelPrice(path.value, fuelShape)),
                ext::shared_ptr<FdmInnerValueCalculator>(
                    new PathSparkSpreadPrice(heatRate, path.value,
                                             fuelShape, powerShape))));

        Array state(nStates, 0.0);
        for (Size j=exercise->dates().size(); j > 0; --j) {
            stepCondition->applyTo(state, grid.at(j));
            state*=rTS->discount(grid.at(j))/rTS->discount(grid.at(j-1));
        }

        npv.add(state.back());
    }
    Real npvMC = npv.mean();
    Real errorMC = npv.errorEstimate();
    const Real expectedMC = 5250.0;
    if (std::fabs(npvMC-expectedMC) > 3*errorMC) {
        BOOST_ERROR("Failed to reproduce Monte-Carlo price"
                   << "\n    calculated: " << npvMC
                   << "\n    error     ; " << errorMC
                   << "\n    expected  : " << expectedMC);
    }
    npv.reset();

    // Test: Longstaff Schwartz least squares Monte-Carlo
    // implementation is not strictly correct but saves some coding
    const Size nCalibrationTrails = 1000U;
    std::vector<sample_type> calibrationPaths;
    std::vector<ext::shared_ptr<FdmVPPStepCondition> > stepConditions;
    std::vector<ext::shared_ptr<FdmInnerValueCalculator> > sparkSpreads;

    sparkSpreads.reserve(nCalibrationTrails);
    stepConditions.reserve(nCalibrationTrails);
    calibrationPaths.reserve(nCalibrationTrails);

    for (Size i=0; i < nCalibrationTrails; ++i) {
        calibrationPaths.push_back(generator.next());

        sparkSpreads.push_back(ext::shared_ptr<FdmInnerValueCalculator>(
            new PathSparkSpreadPrice(heatRate, calibrationPaths.back().value,
                                     fuelShape, powerShape)));
        stepConditions.push_back(stepConditionFactory.build(
            vppMesh, fuelCostAddon,
            ext::shared_ptr<FdmInnerValueCalculator>(
                new PathFuelPrice(calibrationPaths.back().value, fuelShape)),
            sparkSpreads.back()));
    }


    const FdmLinearOpIterator iter = oneDimMesher->layout()->begin();

    // prices of all calibration paths for all states
    std::vector<Array> prices(nCalibrationTrails, Array(nStates, 0.0));

    // regression coefficients for all states and all exercise dates
    std::vector<std::vector<Array> > coeff(
        nStates, std::vector<Array>(exercise->dates().size(), Array()));

    // regression functions
    const Size dim = 1U;
    std::vector<ext::function<Real(Array)> > v(
        LsmBasisSystem::multiPathBasisSystem(dim, 5U, LsmBasisSystem::Monomial));

    for (Size i = exercise->dates().size(); i > 0U; --i) {
        const Time t = grid.at(i);

        std::vector<Array> x(nCalibrationTrails, Array(dim));

        for (Size j=0; j < nCalibrationTrails; ++j) {
            x[j][0] = sparkSpreads[j]->innerValue(iter, t);
        }

        for (Size k=0; k < nStates; ++k) {
            std::vector<Real> y(nCalibrationTrails);

            for (Size j=0; j < nCalibrationTrails; ++j) {
                y[j] = prices[j][k];
            }
            coeff[k][i-1] = GeneralLinearLeastSquares(x, y, v).coefficients();

            for (Size j=0; j < nCalibrationTrails; ++j) {
                prices[j][k] = 0.0;
                for (Size l=0; l < v.size(); ++l) {
                    prices[j][k] += coeff[k][i-1][l]*v[l](x[j]);
                }
            }
        }

        for (Size j=0; j < nCalibrationTrails; ++j) {
            stepConditions[j]->applyTo(prices[j], grid.at(i));
        }
    }

    Real tmpValue = 0.0;
    for (Size i=0; i < nTrails; ++i) {
        Array x(dim), state(nStates, 0.0), contState(nStates, 0.0);

        const sample_type& path = (i % 2) != 0U ? generator.antithetic() : generator.next();

        const ext::shared_ptr<FdmInnerValueCalculator> fuelPrices(
            new PathFuelPrice(path.value, fuelShape));

        const ext::shared_ptr<FdmInnerValueCalculator> sparkSpreads(
            new PathSparkSpreadPrice(heatRate, path.value,
                                     fuelShape, powerShape));

        for (Size j = exercise->dates().size(); j > 0U; --j) {
            const Time t = grid.at(j);
            const Real fuelPrice = fuelPrices->innerValue(iter, t);
            const Real sparkSpread = sparkSpreads->innerValue(iter, t);
            const Real startUpCost
                    = startUpFixCost + (fuelPrice + fuelCostAddon)*startUpFuel;

            x[0] = sparkSpread;
            for (Size k=0; k < nStates; ++k) {
                contState[k] = 0.0;
                for (Size l=0; l < v.size(); ++l) {
                    contState[k] += coeff[k][j-1][l]*v[l](x);
                }
            }

            const Real pMinFlow = pMin*(sparkSpread - heatRate*fuelCostAddon);
            const Real pMaxFlow = pMax*(sparkSpread - heatRate*fuelCostAddon);

            // rollback continuation states and the path states
            for (Size i=0; i < 2*tMinUp; ++i) {
                if (i < tMinUp) {
                    state[i]    += pMinFlow;
                    contState[i]+= pMinFlow;
                }
                else {
                    state[i]    += pMaxFlow;
                    contState[i]+= pMaxFlow;
                }
            }

            // dynamic programming using the continuation values
            Array retVal(nStates);
            for (Size i=0; i < tMinUp-1; ++i) {
                retVal[i] = retVal[tMinUp + i]
                          = (contState[i+1] > contState[tMinUp + i+1])?
                                          state[i+1] : state[tMinUp + i+1];
            }

            if (contState[2*tMinUp] >=
                std::max(contState[tMinUp-1], contState[2*tMinUp-1])) {
                retVal[tMinUp-1] = retVal[2*tMinUp-1] = state[2*tMinUp];
            }
            else if (contState[tMinUp-1] >= contState[2*tMinUp-1]) {
                retVal[tMinUp-1] = retVal[2*tMinUp-1] = state[tMinUp-1];
            }
            else {
                retVal[tMinUp-1] = retVal[2*tMinUp-1] = state[2*tMinUp-1];
            }

            for (Size i=0; i < tMinDown-1; ++i) {
                retVal[2*tMinUp + i] = state[2*tMinUp + i+1];
            }

            if (contState.back() >=
                std::max(contState.front(), contState[tMinUp]) - startUpCost) {
                retVal.back() = state.back();
            }
            else if (contState.front() >  contState[tMinUp]) {
                retVal.back() = state.front()-startUpCost;
            }
            else {
                retVal.back() = state[tMinUp]-startUpCost;
            }
            state = retVal;
        }
        tmpValue+=0.5*state.back();
        if ((i % 2) != 0U) {
            npv.add(tmpValue, 1.0);
            tmpValue = 0.0;
        }
    }

    npvMC = npv.mean();
    errorMC = npv.errorEstimate();
    if (std::fabs(npvMC-fdmPrice) > 3*errorMC) {
        BOOST_ERROR("Failed to reproduce Least Square Monte-Carlo price"
                   << "\n    calculated   : " << npvMC
                   << "\n    error        : " << errorMC
                   << "\n    expected FDM : " << fdmPrice);
    }
}

BOOST_AUTO_TEST_CASE(testKlugeExtOUMatrixDecomposition) {
    BOOST_TEST_MESSAGE("Testing KlugeExtOU matrix decomposition...");

    const Date today = Date(18, December, 2011);
    Settings::instance().evaluationDate() = today;

    const ext::shared_ptr<KlugeExtOUProcess> klugeOUProcess
        = createKlugeExtOUProcess();

    const Size xGrid = 50;
    const Size yGrid = 20;
    const Size uGrid = 20;
    const Time maturity = 1;

    const ext::shared_ptr<ExtOUWithJumpsProcess> klugeProcess
        = klugeOUProcess->getKlugeProcess();
    const ext::shared_ptr<StochasticProcess1D> ouProcess
        = klugeProcess->getExtendedOrnsteinUhlenbeckProcess();

    const ext::shared_ptr<FdmMesher> mesher(
        new FdmMesherComposite(
            ext::shared_ptr<Fdm1dMesher>(
                new FdmSimpleProcess1dMesher(xGrid, ouProcess, maturity)),
            ext::shared_ptr<Fdm1dMesher>(
                new ExponentialJump1dMesher(yGrid,
                                            klugeProcess->beta(),
                                            klugeProcess->jumpIntensity(),
                                            klugeProcess->eta())),
            ext::shared_ptr<Fdm1dMesher>(
                new FdmSimpleProcess1dMesher(uGrid,
                                             klugeOUProcess->getExtOUProcess(),
                                             maturity))));

    const ext::shared_ptr<FdmLinearOpComposite> op(
        new FdmKlugeExtOUOp(mesher, klugeOUProcess,
                            flatRate(today, 0.0, ActualActual(ActualActual::ISDA)),
                            FdmBoundaryConditionSet(), 16));
    op->setTime(0.1, 0.2);

    Array x(mesher->layout()->size());

    PseudoRandom::rng_type rng(PseudoRandom::urng_type(12345UL));
    for (Real& i : x) {
        i = rng.next().value;
    }

    const Real tol = 1e-9;
    const Array applyExpected = op->apply(x);
    const Array applyExpectedMixed = op->apply_mixed(x);

    const std::vector<SparseMatrix> matrixDecomp(op->toMatrixDecomp());
    const Array applyCalculated = prod(op->toMatrix(), x);
    const Array applyCalculatedMixed = prod(matrixDecomp.back(), x);

    for (Size i=0; i < x.size(); ++i) {
        const Real diffApply = std::fabs(applyExpected[i]-applyCalculated[i]);
        if (diffApply > tol && diffApply > std::fabs(applyExpected[i])*tol) {
            BOOST_ERROR("Failed to reproduce apply operation" <<
                     "\n    expected  : " << applyExpected[i] <<
                     "\n    calculated: " << applyCalculated[i] <<
                     "\n    diff      : " << diffApply);
        }

        const Real diffMixed = std::fabs(applyExpectedMixed[i]-applyCalculatedMixed[i]);
        if (diffMixed > tol && diffMixed > std::fabs(applyExpected[i])*tol) {
            BOOST_ERROR("Failed to reproduce apply operation" <<
                     "\n    expected  : " << applyExpectedMixed[i] <<
                     "\n    calculated: " << applyCalculatedMixed[i] <<
                     "\n    diff      : " << diffMixed);
        }
    }


    for (Size i=0; i < 3; ++i) {
        const Array applyExpectedDir = op->apply_direction(i, x);
        const Array applyCalculatedDir = prod(matrixDecomp[i], x);

        for (Size j=0; j < x.size(); ++j) {
            const Real diff
                = std::fabs((applyExpectedDir[j] - applyCalculatedDir[j]));

            if (diff > tol && diff > std::fabs(applyExpectedDir[j]*tol)) {
                BOOST_ERROR("Failed to reproduce apply operation" <<
                         "\n    expected  : " << applyExpectedDir[i] <<
                         "\n    calculated: " << applyCalculatedDir[i] <<
                         "\n    diff : " << diff);
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()


