
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "solvers.hpp"
#include "utilities.hpp"
#include <ql/Solvers1D/brent.hpp>
#include <ql/Solvers1D/bisection.hpp>
#include <ql/Solvers1D/falseposition.hpp>
#include <ql/Solvers1D/ridder.hpp>
#include <ql/Solvers1D/secant.hpp>
#include <ql/Solvers1D/newton.hpp>
#include <ql/Solvers1D/newtonsafe.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    class Foo {
      public:
        double operator()(double x) const { return x*x-1.0; }
        double derivative(double x) const { return 2.0*x; }
    };

    template <class S>
    void test(const S& solver, const std::string& name) {
        double accuracy[] = { 1.0e-4, 1.0e-6, 1.0e-8 };
        double expected = 1.0;
        for (Size i=0; i<LENGTH(accuracy); i++) {
            double root = solver.solve(Foo(),accuracy[i],1.5,0.1);
            if (QL_FABS(root-expected) > accuracy[i]) {
                BOOST_FAIL(
                    name + " solver:\n"
                    "    expected:   " 
                    + DoubleFormatter::toString(expected) + "\n"
                    "    calculated: "
                    + DoubleFormatter::toString(root) + "\n"
                    "    accuracy:   "
                    + DoubleFormatter::toString(accuracy[i]));
            }
            root = solver.solve(Foo(),accuracy[i],1.5,0.0,1.0);
            if (QL_FABS(root-expected) > accuracy[i]) {
                BOOST_FAIL(
                    name + " solver (bracketed):\n"
                    "    expected:   " 
                    + DoubleFormatter::toString(expected) + "\n"
                    "    calculated: "
                    + DoubleFormatter::toString(root) + "\n"
                    "    accuracy:   "
                    + DoubleFormatter::toString(accuracy[i]));
            }
        }
    }

}

void Solver1DTest::testResults() {

    BOOST_MESSAGE("Testing 1-D solvers...");

    test(Brent(),"Brent");
    test(Bisection(),"Bisection");
    test(FalsePosition(),"FalsePosition");
    test(Ridder(),"Ridder");
    test(Secant(),"Secant");
    test(Newton(),"Newton");
    test(NewtonSafe(),"NewtonSafe");

}


test_suite* Solver1DTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("1-D solver tests");
    suite->add(BOOST_TEST_CASE(&Solver1DTest::testResults));
    return suite;
}

