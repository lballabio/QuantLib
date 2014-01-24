/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#include "factorial.hpp"
#include "utilities.hpp"
#include <ql/math/factorial.hpp>
#include <ql/math/distributions/gammadistribution.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void FactorialTest::testFactorial() {

    BOOST_TEST_MESSAGE("Testing factorial numbers...");

    Real expected = 1.0;
    Real calculated = Factorial::get(0);
    if (calculated!=expected)
        BOOST_FAIL("Factorial(0) = " << calculated);

    for (Natural i=1; i<171; ++i) {
        expected *= i;
        calculated = Factorial::get(i);
        if (std::fabs(calculated-expected)/expected > 1.0e-9)
            BOOST_FAIL("Factorial(" << i << ")" <<
                       std::setprecision(16) << QL_SCIENTIFIC <<
                       "\n calculated: " << calculated <<
                       "\n   expected: " << expected <<
                       "\n rel. error: " <<
                       std::fabs(calculated-expected)/expected);
    }
}

void FactorialTest::testGammaFunction() {

    BOOST_TEST_MESSAGE("Testing Gamma function...");

    Real expected = 0.0;
    Real calculated = GammaFunction().logValue(1);
    if (std::fabs(calculated) > 1.0e-15)
        BOOST_ERROR("GammaFunction(1)\n"
                    << std::setprecision(16) << QL_SCIENTIFIC
                    << "    calculated: " << calculated << "\n"
                    << "    expected:   " << expected);

    for (Size i=2; i<9000; i++) {
        expected  += std::log(Real(i));
        calculated = GammaFunction().logValue(static_cast<Real>(i+1));
        if (std::fabs(calculated-expected)/expected > 1.0e-9)
            BOOST_ERROR("GammaFunction(" << i << ")\n"
                        << std::setprecision(16) << QL_SCIENTIFIC
                        << "    calculated: " << calculated << "\n"
                        << "    expected:   " << expected << "\n"
                        << "    rel. error: "
                        << std::fabs(calculated-expected)/expected);
    }
}

void FactorialTest::testGammaValues() {

    BOOST_TEST_MESSAGE("Testing Gamma values...");

    // reference results are calculated with R
    Real tasks[][3] = {
    		{ 0.0001, 9999.422883231624, 1e3},
    		{ 1.2, 0.9181687423997607, 1e3},
    		{ 7.3, 1271.4236336639089586, 1e3},
    		{-1.1, 9.7148063829028946, 1e3},
    		{-4.001,-41.6040228304425312, 1e3},
    		{-4.999, -8.347576090315059, 1e3},
    		{-19.000001, 8.220610833201313e-12, 1e8},
    		{-19.5, 5.811045977502255e-18, 1e3},
    		{-21.000001, 1.957288098276488e-14, 1e8},
    		{-21.5, 1.318444918321553e-20, 1e6}
    };

    for (Size i=0; i < LENGTH(tasks); ++i) {
    	const Real x = tasks[i][0];
    	const Real expected = tasks[i][1];
    	const Real calculated = GammaFunction().value(x);
    	const Real tol = tasks[i][2] * QL_EPSILON*std::fabs(expected);

    	if (std::fabs(calculated - expected) > tol) {
            BOOST_ERROR("GammaFunction(" << x << ")\n"
                        << std::setprecision(16) << QL_SCIENTIFIC
                        << "    calculated: " << calculated << "\n"
                        << "    expected:   " << expected << "\n"
                        << "    rel. error: "
                        << std::fabs(calculated-expected)/expected);
    	}
    }
}

test_suite* FactorialTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Factorial tests");
    suite->add(QUANTLIB_TEST_CASE(&FactorialTest::testFactorial));
    suite->add(QUANTLIB_TEST_CASE(&FactorialTest::testGammaFunction));
    suite->add(QUANTLIB_TEST_CASE(&FactorialTest::testGammaValues));
    return suite;
}

