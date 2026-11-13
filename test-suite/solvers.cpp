/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2012 StatPro Italia srl

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
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/solvers1d/bisection.hpp>
#include <ql/math/solvers1d/falseposition.hpp>
#include <ql/math/solvers1d/ridder.hpp>
#include <ql/math/solvers1d/secant.hpp>
#include <ql/math/solvers1d/newton.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/math/solvers1d/halley.hpp>
#include <ql/math/solvers1d/finitedifferencenewtonsafe.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(Solver1DTests)

class F1 {
  public:
    Real operator()(Real x) const { return x*x-1.0; }
    Real derivative(Real x) const { return 2.0*x; }
    Real secondDerivative(Real x) const { return 2.0;}
};

class F2 {
  public:
    Real operator()(Real x) const { return 1.0-x*x; }
    Real derivative(Real x) const { return -2.0*x; }
    Real secondDerivative(Real x) const { return -2.0;}
};

class F3 {
  public:
    Real operator()(Real x) const { return std::atan(x-1); }
    Real derivative(Real x) const { return 1.0 / (1.0+(x-1.0)*(x-1.0)); }
    Real secondDerivative(Real x) const {
        const Real u = x-1.0;
        return -2*u/((1.0+u*u)*(1.0+u*u));
    }
};

template <class S, class F>
void test_not_bracketed(const S& solver, const std::string& name,
                        const F& f, Real guess) {
    Real accuracy[] = { 1.0e-4, 1.0e-6, 1.0e-8 };
    Real expected = 1.0;
    for (Real& i : accuracy) {
        Real root = solver.solve(f, i, guess, 0.1);
        if (std::fabs(root - expected) > i) {
            BOOST_FAIL(name << " solver (not bracketed):\n"
                       << "    expected:   " << expected << "\n"
                       << "    calculated: " << root << "\n"
                       << "    accuracy:   " << i);
        }
    }
}

template <class S, class F>
void test_bracketed(const S& solver, const std::string& name,
                    const F& f, Real guess) {
    Real accuracy[] = { 1.0e-4, 1.0e-6, 1.0e-8 };
    Real expected = 1.0;
    for (Real& i : accuracy) {
        // guess on the left side of the root, increasing function
        Real root = solver.solve(f, i, guess, 0.0, 2.0);
        if (std::fabs(root - expected) > i) {
            BOOST_FAIL(name << " solver (bracketed):\n"
                       << "    expected:   " << expected << "\n"
                       << "    calculated: " << root << "\n"
                       << "    accuracy:   " << i);
        }
    }
}

class Probe {
  public:
    Probe(Real& result, Real offset)
    : result_(result), previous_(result), offset_(offset) {}
    Real operator()(Real x) const {
        result_ = x;
        return previous_ + offset_ - x*x;
    }
    Real derivative(Real x) const { return 2.0*x; }
    Real secondDerivative(Real x) const { return 2.0; }
  private:
    Real& result_;
    Real previous_;
    Real offset_;
};

template <class S>
void test_last_call_with_root(const S& solver, const std::string& name,
                              bool bracketed,  Real accuracy) {

    Real mins[] = { 3.0, 2.25, 1.5, 1.0 };
    Real maxs[] = { 7.0, 5.75, 4.5, 3.0 };
    Real steps[] = { 0.2, 0.2, 0.1, 0.1 };
    Real offsets[] = { 25.0, 11.0, 5.0, 1.0 };
    Real guesses[] = { 4.5, 4.5, 2.5, 2.5 };
    //Real expected[] = { 5.0, 4.0, 3.0, 2.0 };

    Real argument = 0.0;
    Real result;

    for (Size i=0; i<4; ++i) {
        if (bracketed) {
            result = solver.solve(Probe(argument, offsets[i]), accuracy,
                                  guesses[i], mins[i], maxs[i]);
        } else {
            result = solver.solve(Probe(argument, offsets[i]), accuracy,
                                  guesses[i], steps[i]);
        }

        Real error = std::fabs(result-argument);
        // the solver should have called the function with
        // the very same value it's returning. But the internal
        // 80bit length of the x87 FPU register might lead to
        // a very small glitch when compiled with -mfpmath=387 on gcc
        if (error > 2*QL_EPSILON) {
            BOOST_FAIL(name << " solver ("
                       << (bracketed ? "" : "not ")
                       << "bracketed):\n"
                       << "    index:   " << i << "\n"
                       << "    expected:   " << result << "\n"
                       << "    calculated: " << argument << "\n"
                       << "    error: " << error);
        }
    }
}        

template <class S>
void test_solver(const S& solver, const std::string& name, Real accuracy) {
    // guess on the left side of the root, increasing function
    test_not_bracketed(solver, name, F1(), 0.5);
    test_bracketed(solver, name, F1(), 0.5);
    // guess on the right side of the root, increasing function
    test_not_bracketed(solver, name, F1(), 1.5);
    test_bracketed(solver, name, F1(), 1.5);
    // guess on the left side of the root, decreasing function
    test_not_bracketed(solver, name, F2(), 0.5);
    test_bracketed(solver, name, F2(), 0.5);
    // guess on the right side of the root, decreasing function
    test_not_bracketed(solver, name, F2(), 1.5);
    test_bracketed(solver, name, F2(), 1.5);
    // situation where bisection is used in the finite difference
    // newton solver as the first step and where the initial
    // guess is equal to the next estimate (which causes an infinite
    // derivative if we do not handle this case with special care)
    test_not_bracketed(solver, name, F3(), 1.00001);
    // check that the last function call is made with the root value
    if(accuracy != Null<Real>()) {
        test_last_call_with_root(solver, name, false, accuracy);
        test_last_call_with_root(solver, name, true, accuracy);
    }
}


BOOST_AUTO_TEST_CASE(testBrent) {
    BOOST_TEST_MESSAGE("Testing Brent solver...");
    test_solver(Brent(), "Brent", 1.0e-6);
}

BOOST_AUTO_TEST_CASE(testBisection) {
    BOOST_TEST_MESSAGE("Testing bisection solver...");
    test_solver(Bisection(), "Bisection", 1.0e-6);
}

BOOST_AUTO_TEST_CASE(testFalsePosition) {
    BOOST_TEST_MESSAGE("Testing false-position solver...");
    test_solver(FalsePosition(), "FalsePosition", 1.0e-6);
}

BOOST_AUTO_TEST_CASE(testNewton) {
    BOOST_TEST_MESSAGE("Testing Newton solver...");
    test_solver(Newton(), "Newton", 1.0e-12);
}

// f(x) = x^3 - 8 has the root 2 and a derivative that vanishes at 0
class CubicMinusEight {
  public:
    Real operator()(Real x) const { return x*x*x - 8.0; }
    Real derivative(Real x) const { return 3.0*x*x; }
    Real secondDerivative(Real x) const { return 6.0*x; }
};

// f(x) = tanh(x - 2) has root 2; at 25, tanh(x - 2) rounds to 1,
// so the computed derivative 1 - tanh(x - 2)^2 is zero.
class ShiftedTanh {
  public:
    Real operator()(Real x) const { return std::tanh(x-2.0); }
    Real derivative(Real x) const {
        const Real t = std::tanh(x-2.0);
        return 1.0-t*t;
    }
    Real secondDerivative(Real x) const {
        const Real t = std::tanh(x-2.0);
        return -2.0*t*(1.0-t*t);
    }
};

// records the arguments of the function calls
template <class F>
class Recorder {
  public:
    Real operator()(Real x) const {
        arguments_.push_back(x);
        return f_(x);
    }
    Real derivative(Real x) const { return f_.derivative(x); }
    Real secondDerivative(Real x) const { return f_.secondDerivative(x); }
    mutable std::vector<Real> arguments_;
  private:
    F f_;
};

template <class S, class F>
void test_fallback(const S& solver, const std::string& name, const F& f,
                   Real expected, Real guess, Real xMin, Real xMax) {
    Real accuracy = 1.0e-10;
    BOOST_TEST_CONTEXT(name << " solver, guess " << guess) {
        try {
            Real root = solver.solve(f, accuracy, guess, xMin, xMax);
            QL_CHECK_SMALL(root - expected, accuracy);
        } catch (std::exception& e) {
            BOOST_ERROR("solver failed: " << e.what());
        }
    }
}

BOOST_AUTO_TEST_CASE(testNewtonAndHalleyFallback) {
    BOOST_TEST_MESSAGE("Testing fallback of Newton and Halley solvers to Newton-safe solver...");

    // the derivative is zero at the guess: Newton's step is infinite and
    // Halley's is not a number, so they have to give up on them
    test_fallback(Newton(), "Newton", CubicMinusEight(), 2.0, 0.0, -1.0, 5.0);
    test_fallback(Halley(), "Halley", CubicMinusEight(), 2.0, 0.0, -1.0, 5.0);
    test_fallback(Newton(), "Newton", ShiftedTanh(), 2.0, 25.0, -20.0, 30.0);
    test_fallback(Halley(), "Halley", ShiftedTanh(), 2.0, 25.0, -20.0, 30.0);

    // The derivative is tiny, so the step is finite but takes the iterate far
    // out of the bracket. The Newton-safe solver must start again from the
    // guess, not from an iterate reconstructed by adding the step back.
    Real guess = 1.0e-9;
    Recorder<CubicMinusEight> f;
    test_fallback(Newton(), "Newton", f, 2.0, guess, -1.0, 5.0);
    // the first call is made by the Newton solver, the second by the other one
    BOOST_CHECK_GE(std::count(f.arguments_.begin(), f.arguments_.end(), guess), 2);
}

BOOST_AUTO_TEST_CASE(testNewtonSafe) {
    BOOST_TEST_MESSAGE("Testing Newton-safe solver...");
    test_solver(NewtonSafe(), "NewtonSafe", 1.0e-9);
}

BOOST_AUTO_TEST_CASE(testFiniteDifferenceNewtonSafe) {
    BOOST_TEST_MESSAGE("Testing finite-difference Newton-safe solver...");
    test_solver(FiniteDifferenceNewtonSafe(), "FiniteDifferenceNewtonSafe", 1.0e-6);
}

BOOST_AUTO_TEST_CASE(testFiniteDifferenceNewtonSafeSmallStep) {
    BOOST_TEST_MESSAGE("Testing finite-difference Newton-safe convergence after a small step...");

    auto f = [](Real x) -> Real { return 20.0*x*x*x*x*x - 0.01; };
    Real accuracy = 0.001;
    Real expected = std::pow(0.01/20.0, 1.0/5.0);
    Real root = FiniteDifferenceNewtonSafe().solve(f, accuracy, 0.35, -1.75, 5.25);
    QL_CHECK_SMALL(root - expected, accuracy);
}

BOOST_AUTO_TEST_CASE(testFiniteDifferenceNewtonSafeResidualReduction) {
    BOOST_TEST_MESSAGE("Testing finite-difference Newton-safe convergence "
                       "after residual reduction...");

    Real expected = 0.05;
    auto f = [=](Real x) -> Real {
        return 1000.0*(x*x*x - expected*expected*expected) + (x - expected)
            + 0.6*(std::tanh(10000.0*x) - std::tanh(10000.0*expected));
    };
    Real accuracy = 0.001;
    Real root = FiniteDifferenceNewtonSafe().solve(f, accuracy, 0.0, -1.0, 5.0);
    QL_CHECK_SMALL(root - expected, accuracy);
}

BOOST_AUTO_TEST_CASE(testFiniteDifferenceNewtonSafeEvaluationLimit) {
    BOOST_TEST_MESSAGE("Testing finite-difference Newton-safe function evaluation limits...");

    for (Size budget = 2; budget <= 6; ++budget) {
        BOOST_TEST_CONTEXT("evaluation limit " << budget) {
            Size calls = 0;
            auto f = [&](Real x) -> Real {
                ++calls;
                return 20.0*x*x*x*x*x - 0.01;
            };
            FiniteDifferenceNewtonSafe solver;
            solver.setMaxEvaluations(budget);
            BOOST_CHECK_EXCEPTION(solver.solve(f, 0.001, 0.35, -1.75, 5.25), Error,
                                  ExpectedErrorMessage("maximum number of function evaluations"));
            BOOST_CHECK_EQUAL(calls, budget);
        }
    }

    // A zero initial slope forces bisection and the extra endpoint evaluation.
    const Real scale = 8*QL_EPSILON;
    Size calls = 0;
    auto f = [&](Real x) -> Real {
        ++calls;
        Real t = (x-1.0)/scale;
        return (t+1.0)*(t+1.0)-4.0;
    };
    FiniteDifferenceNewtonSafe solver;
    solver.setMaxEvaluations(4);
    BOOST_CHECK_EXCEPTION(solver.solve(f, QL_EPSILON, 1.0, 1.0-2*scale, 1.0+4*scale),
                          Error, ExpectedErrorMessage("maximum number of function evaluations"));
    BOOST_CHECK_EQUAL(calls, Size(4));
}

BOOST_AUTO_TEST_CASE(testFiniteDifferenceNewtonSafeConvergenceAtLimit) {
    BOOST_TEST_MESSAGE("Testing finite-difference Newton-safe convergence "
                       "at the evaluation limit...");

    struct Case {
        Real accuracy, guess, xMin, xMax;
        Size budget;
    };
    const Case cases[] = {
        {0.01,   0.9999, 0.999, 1.001, 3}, // an already narrow bracket
        {1.0e-8, 1.0,    0.0,   2.0,   3}, // an exact initial root
        {0.1,    1.1,    0.0,   2.0,   5}  // a successful probe on the last call
    };
    for (const auto& c : cases) {
        BOOST_TEST_CONTEXT("accuracy " << c.accuracy << ", evaluation limit " << c.budget) {
            Size calls = 0;
            Real last = Null<Real>();
            auto f = [&](Real x) -> Real {
                ++calls;
                last = x;
                return x*x-1.0;
            };
            FiniteDifferenceNewtonSafe solver;
            solver.setMaxEvaluations(c.budget);
            Real root = solver.solve(f, c.accuracy, c.guess, c.xMin, c.xMax);
            QL_CHECK_SMALL(root-1.0, c.accuracy);
            QL_CHECK_SMALL(root-last, 2*QL_EPSILON);
            BOOST_CHECK_EQUAL(calls, c.budget);
        }
    }
}

BOOST_AUTO_TEST_CASE(testFiniteDifferenceNewtonSafeRoundedProbe) {
    BOOST_TEST_MESSAGE("Testing finite-difference Newton-safe convergence with rounded probes...");

    const Real shift = 1.0/QL_EPSILON;
    const Real accuracy = 0.6;
    const ExpectedErrorMessage exhausted("maximum number of function evaluations");
    for (Real expected : std::vector<Real>{0.2, 0.8}) {
        BOOST_TEST_CONTEXT("root offset " << expected) {
            auto f = [=](Real x) -> Real {
                Real t = x-shift;
                return t*t*t-expected*expected*expected;
            };
            try {
                Real root = FiniteDifferenceNewtonSafe().solve(
                    f, accuracy, shift, shift-2.0, shift+4.0);
                // Compare offsets so rounding the expected root cannot hide the error.
                QL_CHECK_SMALL((root-shift)-expected, accuracy);
            } catch (const Error& e) {
                BOOST_CHECK(exhausted(e));
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testRidder) {
    BOOST_TEST_MESSAGE("Testing Ridder solver...");
    test_solver(Ridder(), "Ridder", 1.0e-6);
}

BOOST_AUTO_TEST_CASE(testSecant) {
    BOOST_TEST_MESSAGE("Testing secant solver...");
    test_solver(Secant(), "Secant", 1.0e-6);
}

BOOST_AUTO_TEST_CASE(testHalley) {
    BOOST_TEST_MESSAGE("Testing Halley solver...");
    test_solver(Halley(), "Halley", 1.0e-6);
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
