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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "solvers.hpp"
#include "utilities.hpp"
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/solvers1d/bisection.hpp>
#include <ql/math/solvers1d/falseposition.hpp>
#include <ql/math/solvers1d/ridder.hpp>
#include <ql/math/solvers1d/secant.hpp>
#include <ql/math/solvers1d/newton.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    class F1 {
      public:
        Real operator()(Real x) const { return x*x-1.0; }
        Real derivative(Real x) const { return 2.0*x; }
    };

    class F2 {
      public:
        Real operator()(Real x) const { return 1.0-x*x; }
        Real derivative(Real x) const { return -2.0*x; }
    };

    template <class S, class F>
    void test_not_bracketed(const S& solver, const std::string& name,
                            const F& f, Real guess) {
        Real accuracy[] = { 1.0e-4, 1.0e-6, 1.0e-8 };
        Real expected = 1.0;
        for (Size i=0; i<LENGTH(accuracy); i++) {
            Real root = solver.solve(f,accuracy[i],guess,0.1);
            if (std::fabs(root-expected) > accuracy[i]) {
                BOOST_FAIL(name << " solver (not bracketed):\n"
                           << "    expected:   " << expected << "\n"
                           << "    calculated: " << root << "\n"
                           << "    accuracy:   " << accuracy[i]);
            }
        }
    }

    template <class S, class F>
    void test_bracketed(const S& solver, const std::string& name,
                        const F& f, Real guess) {
        Real accuracy[] = { 1.0e-4, 1.0e-6, 1.0e-8 };
        Real expected = 1.0;
        for (Size i=0; i<LENGTH(accuracy); i++) {
            // guess on the left side of the root, increasing function
            Real root = solver.solve(f,accuracy[i],guess,0.0,2.0);
            if (std::fabs(root-expected) > accuracy[i]) {
                BOOST_FAIL(name << " solver (bracketed):\n"
                           << "    expected:   " << expected << "\n"
                           << "    calculated: " << root << "\n"
                           << "    accuracy:   " << accuracy[i]);
            }
        }
    }


    template <class S>
    void test_solver(const S& solver, const std::string& name) {
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
    }
}


void Solver1DTest::testBrent() {
    BOOST_MESSAGE("Testing Brent solver...");
    test_solver(Brent(),"Brent");
}

void Solver1DTest::testBisection() {
    BOOST_MESSAGE("Testing bisection solver...");
    test_solver(Bisection(),"Bisection");
}

void Solver1DTest::testFalsePosition() {
    BOOST_MESSAGE("Testing false-position solver...");
    test_solver(FalsePosition(),"FalsePosition");
}

void Solver1DTest::testNewton() {
    BOOST_MESSAGE("Testing Newton solver...");
    test_solver(Newton(),"Newton");
}

void Solver1DTest::testNewtonSafe() {
    BOOST_MESSAGE("Testing Newton-safe solver...");
    test_solver(NewtonSafe(),"NewtonSafe");
}

void Solver1DTest::testRidder() {
    BOOST_MESSAGE("Testing Ridder solver...");
    test_solver(Ridder(),"Ridder");
}

void Solver1DTest::testSecant() {
    BOOST_MESSAGE("Testing secant solver...");
    test_solver(Secant(),"Secant");
}


test_suite* Solver1DTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("1-D solver tests");
    suite->add(QUANTLIB_TEST_CASE(&Solver1DTest::testBrent));
    suite->add(QUANTLIB_TEST_CASE(&Solver1DTest::testBisection));
    suite->add(QUANTLIB_TEST_CASE(&Solver1DTest::testFalsePosition));
    suite->add(QUANTLIB_TEST_CASE(&Solver1DTest::testNewton));
    suite->add(QUANTLIB_TEST_CASE(&Solver1DTest::testNewtonSafe));
    suite->add(QUANTLIB_TEST_CASE(&Solver1DTest::testRidder));
    suite->add(QUANTLIB_TEST_CASE(&Solver1DTest::testSecant));
    return suite;
}

