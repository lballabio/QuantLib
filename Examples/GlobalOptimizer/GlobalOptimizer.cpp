/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2016 Andres Hernandez

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

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/experimental/math/fireflyalgorithm.hpp>
#include <ql/experimental/math/hybridsimulatedannealing.hpp>
#include <ql/experimental/math/particleswarmoptimization.hpp>

#include <ql/math/optimization/differentialevolution.hpp>
#include <ql/math/optimization/simulatedannealing.hpp>
#include <ql/tuple.hpp>
#include <iomanip>
#include <iostream>
#include <utility>

using namespace QuantLib;

unsigned long seed = 127;

/*
    Some benchmark functions taken from
    https://en.wikipedia.org/wiki/Test_functions_for_optimization

    Global optimizers have generally a lot of hyper-parameters, and one
    * usually requires some hyper-parameter optimization to find appropriate values
*/

Real ackley(const Array& x) {
    //Minimum is found at 0
    Real p1 = 0.0, p2 = 0.0;

    for (Real i : x) {
        p1 += i * i;
        p2 += std::cos(M_TWOPI * i);
    }
    p1 = -0.2*std::sqrt(0.5*p1);
    p2 *= 0.5;
    return M_E + 20.0 - 20.0*std::exp(p1)-std::exp(p2);
}

Array ackleyValues(const Array& x) {
    Array y(x.size());
    for (Size i = 0; i < x.size(); i++) {
        Real p1 = x[i] * x[i];
        p1 = -0.2*std::sqrt(0.5*p1);
        Real p2 = 0.5*std::cos(M_TWOPI*x[i]);
        y[i] = M_E + 20.0 - 20.0*std::exp(p1)-std::exp(p2);
    }
    return y;
}

Real sphere(const Array& x) {
    //Minimum is found at 0
    return DotProduct(x, x);
}

Array sphereValues(const Array& x) {
    Array y(x.size());
    for (Size i = 0; i < x.size(); i++) {
        y[i] = x[i]*x[i];
    }
    return y;
}

Real rosenbrock(const Array& x) {
    //Minimum is found at f(1, 1, ...)
    QL_REQUIRE(x.size() > 1, "Input size needs to be higher than 1");
    Real result = 0.0;
    for (Size i = 0; i < x.size() - 1; i++) {
        Real temp = (x[i + 1] - x[i] * x[i]);
        result += (x[i] - 1.0)*(x[i] - 1.0) + 100.0*temp*temp;
    }
    return result;
}

Real easom(const Array& x) {
    //Minimum is found at f(\pi, \pi, ...)
    Real p1 = 1.0, p2 = 0.0;
    for (Real i : x) {
        p1 *= std::cos(i);
        p2 += (i - M_PI) * (i - M_PI);
    }
    return -p1*std::exp(-p2);
}

Array easomValues(const Array& x) {
    Array y(x.size());
    for (Size i = 0; i < x.size(); i++) {
        Real p1 = std::cos(x[i]);
        Real p2 = (x[i] - M_PI)*(x[i] - M_PI);
        y[i] = -p1*std::exp(-p2);
    }
    return y;
}

Real eggholder(const Array& x) {
    //Minimum is found at f(512, 404.2319)
    QL_REQUIRE(x.size() == 2, "Input size needs to be equal to 2");
    Real p = (x[1] + 47.0);
    return -p*std::sin(std::sqrt(std::abs(0.5*x[0] + p))) -
        x[0] * std::sin(std::sqrt(std::abs(x[0] - p)));
}

Real printFunction(Problem& p, const Array& x) {
    std::cout << " f(" << x[0];
    for (Size i = 1; i < x.size(); i++) {
        std::cout << ", " << x[i];
    }
    Real val = p.value(x);
    std::cout << ") = " << val << std::endl;
    return val;
}

class TestFunction : public CostFunction {
public:
    typedef std::function<Real(const Array&)> RealFunc;
    typedef std::function<Array(const Array&)> ArrayFunc;
    explicit TestFunction(RealFunc f, ArrayFunc fs = ArrayFunc())
    : f_(std::move(f)), fs_(std::move(fs)) {}
    explicit TestFunction(Real (*f)(const Array&), Array (*fs)(const Array&) = nullptr)
    : f_(f), fs_(fs) {}
    ~TestFunction() override = default;
    Real value(const Array& x) const override { return f_(x); }
    Array values(const Array& x) const override {
        if(!fs_)
            throw std::runtime_error("Invalid function");
        return fs_(x);
    }

private:
    RealFunc f_;
    ArrayFunc fs_;
};

int test(OptimizationMethod& method, CostFunction& f, const EndCriteria& endCriteria,
          const Array& start, const Constraint& constraint = Constraint(),
          const Array& optimum = Array()) {
    QL_REQUIRE(!start.empty(), "Input size needs to be at least 1");
    std::cout << "Starting point: ";
    Constraint c;
    if (!constraint.empty())
        c = constraint;
    Problem p(f, c, start);
    printFunction(p, start);
    method.minimize(p, endCriteria);
    std::cout << "End point: ";
    Real val = printFunction(p, p.currentValue());
    if(!optimum.empty())
    {
        std::cout << "Global optimum: ";
        Real optimVal = printFunction(p, optimum);
        if(std::abs(optimVal) < 1e-13)
            return static_cast<int>(std::abs(val - optimVal) < 1e-6);
        else
            return static_cast<int>(std::abs((val - optimVal) / optimVal) < 1e-6);
    }
    return 1;
}

void testFirefly() {
    /*
    The Eggholder function is only in 2 dimensions, it has a multitude
    * of local minima, and they are not symmetric necessarily
    */
    Size n = 2;
    NonhomogeneousBoundaryConstraint constraint(Array(n, -512.0), Array(n, 512.0));
    Array x(n, 0.0);
    Array optimum(n);
    optimum[0] = 512.0;
    optimum[1] = 404.2319;
    Size agents = 150;
    Real vola = 1.5;
    Real intense = 1.0;
    std::shared_ptr<FireflyAlgorithm::Intensity> intensity =
        std::make_shared<ExponentialIntensity>(10.0, 1e-8, intense);
    std::shared_ptr<FireflyAlgorithm::RandomWalk> randomWalk =
        std::make_shared<LevyFlightWalk>(vola, 0.5, 1.0, seed);
    std::cout << "Function eggholder, Agents: " << agents
            << ", Vola: " << vola << ", Intensity: " << intense << std::endl;
    TestFunction f(eggholder);
    FireflyAlgorithm fa(agents, intensity, randomWalk, 40);
    EndCriteria ec(5000, 1000, 1.0e-8, 1.0e-8, 1.0e-8);
    test(fa, f, ec, x, constraint, optimum);
    std::cout << "================================================================" << std::endl;
}

void testSimulatedAnnealing(Size dimension, Size maxSteps, Size staticSteps){

    /*The ackley function has a large amount of local minima, but the
      structure is symmetric, so if one could simply just ignore the
      walls separating the local minima, it would look like almost
      like a parabola

      Andres Hernandez: I could not find a configuration that was able
      to fix the problem
    */

    //global minimum is at 0.0
    TestFunction f(ackley, ackleyValues);

    //Starting point
    Array x(dimension, 1.5);
    Array optimum(dimension, 0.0);

    //Constraint for local optimizer
    Array lower(dimension, -5.0);
    Array upper(dimension, 5.0);
    NonhomogeneousBoundaryConstraint constraint(lower, upper);

    Real lambda = 0.1;
    Real temperature = 350;
    Real epsilon = 0.99;
    Size ms = 1000;
    std::cout << "Function ackley, Lambda: " << lambda
            << ", Temperature: " << temperature
            << ", Epsilon: " << epsilon
            << ", Iterations: " << ms
            << std::endl;

    MersenneTwisterUniformRng rng(seed);
    SimulatedAnnealing<MersenneTwisterUniformRng> sa(lambda, temperature, epsilon, ms, rng);
    EndCriteria ec(maxSteps, staticSteps, 1.0e-8, 1.0e-8, 1.0e-8);
    test(sa, f, ec, x, constraint, optimum);
    std::cout << "================================================================" << std::endl;
}

void testGaussianSA(Size dimension,
                    Size maxSteps,
                    Size staticSteps,
                    Real initialTemp,
                    Real finalTemp,
                    GaussianSimulatedAnnealing::ResetScheme resetScheme =
                        GaussianSimulatedAnnealing::ResetToBestPoint,
                    Size resetSteps = 150,
                    GaussianSimulatedAnnealing::LocalOptimizeScheme optimizeScheme =
                        GaussianSimulatedAnnealing::EveryBestPoint,
                    const std::shared_ptr<OptimizationMethod>& localOptimizer =
                        std::make_shared<LevenbergMarquardt>()) {

    /*The ackley function has a large amount of local minima, but the
     * structure is symmetric, so if one could simply just ignore the
     * walls separating the local minima, it would look like almost like
     * a parabola*/

    //global minimum is at 0.0
    TestFunction f(ackley, ackleyValues);

    std::cout << "Function: ackley, Dimensions: " << dimension
              << ", Initial temp:" << initialTemp
              << ", Final temp:" << finalTemp
              << ", Reset scheme:" << resetScheme
              << ", Reset steps:" << resetSteps
              << std::endl;
    //Starting point
    Array x(dimension, 1.5);
    Array optimum(dimension, 0.0);

    //Constraint for local optimizer
    Array lower(dimension, -5.0);
    Array upper(dimension, 5.0);
    NonhomogeneousBoundaryConstraint constraint(lower, upper);

    //Simulated annealing setup
    SamplerGaussian sampler(seed);
    ProbabilityBoltzmannDownhill probability(seed);
    TemperatureExponential temperature(initialTemp, dimension);
    GaussianSimulatedAnnealing sa(sampler, probability, temperature, ReannealingTrivial(),
                                  initialTemp, finalTemp, 50, resetScheme,
                                  resetSteps, localOptimizer,
                                  optimizeScheme);

    EndCriteria ec(maxSteps, staticSteps, 1.0e-8, 1.0e-8, 1.0e-8);
    test(sa, f, ec, x, constraint, optimum);
    std::cout << "================================================================" << std::endl;
}

void testPSO(Size n){
    /*The Rosenbrock function has a global minima at (1.0, ...) and a local minima at (-1.0, 1.0, ...)
    The difficulty lies in the weird shape of the function*/
    NonhomogeneousBoundaryConstraint constraint(Array(n, -1.0), Array(n, 4.0));
    Array x(n, 0.0);
    Array optimum(n, 1.0);
    Size agents = 100;
    Size kneighbor = 25;
    Size threshold = 500;
    std::cout << "Function: rosenbrock, Dimensions: " << n
            << ", Agents: " << agents << ", K-neighbors: " << kneighbor
            << ", Threshold: " << threshold << std::endl;
    std::shared_ptr<ParticleSwarmOptimization::Topology> topology =
        std::make_shared<KNeighbors>(kneighbor);
    std::shared_ptr<ParticleSwarmOptimization::Inertia> inertia =
        std::make_shared<LevyFlightInertia>(1.5, threshold, seed);
    TestFunction f(rosenbrock);
    ParticleSwarmOptimization pso(agents, topology, inertia, 2.05, 2.05, seed);
    EndCriteria ec(10000, 1000, 1.0e-8, 1.0e-8, 1.0e-8);
    test(pso, f, ec, x, constraint, optimum);
    std::cout << "================================================================" << std::endl;
}

void testDifferentialEvolution(Size n, Size agents){
    /*The Rosenbrock function has a global minima at (1.0, ...) and a local minima at (-1.0, 1.0, ...)
    The difficulty lies in the weird shape of the function*/
    NonhomogeneousBoundaryConstraint constraint(Array(n, -4.0), Array(n, 4.0));
    Array x(n, 0.0);
    Array optimum(n, 1.0);

    TestFunction f(rosenbrock);

    Real probability = 0.3;
    Real stepsizeWeight = 0.6;
    DifferentialEvolution::Strategy strategy = DifferentialEvolution::BestMemberWithJitter;

    std::cout << "Function: rosenbrock, Dimensions: " << n << ", Agents: " << agents
              << ", Probability: " << probability
              << ", StepsizeWeight: " << stepsizeWeight
              << ", Strategy: BestMemberWithJitter" << std::endl;
    DifferentialEvolution::Configuration config;
    config.withBounds(true)
          .withCrossoverProbability(probability)
          .withPopulationMembers(agents)
          .withStepsizeWeight(stepsizeWeight)
          .withStrategy(strategy)
          .withSeed(seed);

    DifferentialEvolution de(config);
    EndCriteria ec(5000, 1000, 1.0e-8, 1.0e-8, 1.0e-8);
    test(de, f, ec, x, constraint, optimum);
    std::cout << "================================================================" << std::endl;
}


int main(int, char* []) {

    try {
        std::cout << std::endl;

        std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
        std::cout << "Firefly Algorithm Test" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        testFirefly();


        std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
        std::cout << "Hybrid Simulated Annealing Test" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        testGaussianSA(3, 500, 200, 100.0, 0.1, GaussianSimulatedAnnealing::ResetToBestPoint, 150, GaussianSimulatedAnnealing::EveryNewPoint);
        testGaussianSA(10, 500, 200, 100.0, 0.1, GaussianSimulatedAnnealing::ResetToBestPoint, 150, GaussianSimulatedAnnealing::EveryNewPoint);
        testGaussianSA(30, 500, 200, 100.0, 0.1, GaussianSimulatedAnnealing::ResetToBestPoint, 150, GaussianSimulatedAnnealing::EveryNewPoint);


        std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
        std::cout << "Particle Swarm Optimization Test" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        testPSO(3);
        testPSO(10);
        testPSO(30);


        std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
        std::cout << "Simulated Annealing Test" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        testSimulatedAnnealing(3, 10000, 4000);
        testSimulatedAnnealing(10, 10000, 4000);
        testSimulatedAnnealing(30, 10000, 4000);


        std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
        std::cout << "Differential Evolution Test" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        testDifferentialEvolution(3, 50);
        testDifferentialEvolution(10, 150);
        testDifferentialEvolution(30, 450);

        return 0;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}
