
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#include "factorial.hpp"
#include <ql/Math/factorial.hpp>
#include <ql/Math/gammadistribution.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void FactorialTest::testFactorial() {

    BOOST_MESSAGE("Testing factorial numbers...");

    Natural i;
    Real expected = 1.0;
    Real calculated = Factorial::get(0);
    if (calculated!=expected)
        BOOST_FAIL("Factorial(0)\n"
                   << std::setprecision(16) << std::scientific
                   << "    calculated: " << calculated << "\n"
                   << "    expected:   " << expected);
    for (i=1; i<28; i++) {
        expected *= i;
        calculated   = Factorial::get(i);
        if (calculated!=expected)
            BOOST_FAIL("Factorial(" << i << ")\n"
                       << std::setprecision(16) << std::scientific
                       << "    calculated: " << calculated << "\n"
                       << "    expected:   " << expected);
    }
    // Borland cannot stand i>=171
    // ????????????????????????????
    for (i=28; i<171; i++) {
        expected *= i;
        calculated   = Factorial::get(i);
        if (std::fabs(calculated-expected)/expected > 1.0e-9)
            BOOST_FAIL("Factorial(" << i << ")\n"
                       << std::setprecision(16) << std::scientific
                       << "    calculated: " << calculated << "\n"
                       << "    expected:   " << expected << "\n"
                       << "    rel. error: "
                       << std::fabs(calculated-expected)/expected);
    }
}

void FactorialTest::testGammaFunction() {

    BOOST_MESSAGE("Testing Gamma function...");

    Real expected = 0.0;
    Real calculated = GammaFunction().logValue(1);
    if (std::fabs(calculated) > 1.0e-15)
        BOOST_FAIL("GammaFunction(1)\n"
                   << std::setprecision(16) << std::scientific
                   << "    calculated: " << calculated << "\n"
                   << "    expected:   " << expected);

    for (Size i=2; i<9000; i++) {
        expected  += std::log(Real(i));
        calculated = GammaFunction().logValue(i+1);
        if (std::fabs(calculated-expected)/expected > 1.0e-9)
            BOOST_FAIL("GammaFunction(" << i << ")\n"
                       << std::setprecision(16) << std::scientific
                       << "    calculated: " << calculated << "\n"
                       << "    expected:   " << expected << "\n"
                       << "    rel. error: "
                       << std::fabs(calculated-expected)/expected);
    }
}


test_suite* FactorialTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Factorial tests");
    suite->add(BOOST_TEST_CASE(&FactorialTest::testFactorial));
    suite->add(BOOST_TEST_CASE(&FactorialTest::testGammaFunction));
    return suite;
}

