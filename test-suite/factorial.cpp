
/*
 Copyright (C) 2003 Ferdinando Ametrano
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

#include "factorial.hpp"
#include <ql/Math/factorial.hpp>
#include <ql/Math/gammadistribution.hpp>
#include <ql/dataformatters.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;

#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

CppUnit::Test* FactorialTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Factorial tests");
    tests->addTest(new CppUnit::TestCaller<FactorialTest>
                   ("Testing factorial numbers",
                    &FactorialTest::testFactorial));
    tests->addTest(new CppUnit::TestCaller<FactorialTest>
                   ("Testing Gamma function",
                    &FactorialTest::testGammaFunction));
    return tests;
}

void FactorialTest::testFactorial() {

    Size i;
    double expected = 1.0;
    double calculated = Factorial::get(0);
    if (calculated!=expected)
        CPPUNIT_FAIL("Factorial(0)"
                     "    calculated: "
                     + DoubleFormatter::toExponential(calculated,16) + "\n"
                     "    expected:   "
                     + DoubleFormatter::toExponential(expected,16));
    for (i=1; i<28; i++) {
        expected *= i;
        calculated   = Factorial::get(i);
        if (calculated!=expected)
            CPPUNIT_FAIL("Factorial("
                         + IntegerFormatter::toString(i) + ")"
                         "    calculated: "
                         + DoubleFormatter::toExponential(calculated,16) + "\n"
                         "    expected:   "
                         + DoubleFormatter::toExponential(expected,16));
    }
    // Borland cannot stand i>=171
    // ????????????????????????????
    for (i=28; i<171; i++) {
        expected *= i;
        calculated   = Factorial::get(i);
        if (QL_FABS(calculated-expected)/expected > 1.0e-9)
            CPPUNIT_FAIL("Factorial("
                         + IntegerFormatter::toString(i) + ")"
                         "    calculated: "
                         + DoubleFormatter::toExponential(calculated,16) + "\n"
                         "    expected:   "
                         + DoubleFormatter::toExponential(expected,16) + "\n"
                         "    rel error:   "
                         + DoubleFormatter::toExponential(QL_FABS(calculated-expected)/expected,16));
    }
}

void FactorialTest::testGammaFunction() {

    double expected = 0.0;
    double calculated = GammaFunction().logValue(1);
    if (FABS(calculated) > 1.0e-15)
        CPPUNIT_FAIL("GammaFunction("
                     + IntegerFormatter::toString(1) + ")"
                     "    calculated: "
                     + DoubleFormatter::toExponential(calculated,16) + "\n"
                     "    expected:   "
                     + DoubleFormatter::toExponential(expected,16));

    for (Size i=2; i<9000; i++) {
        expected  += QL_LOG(i);
        calculated = GammaFunction().logValue(i+1);
        if (QL_FABS(calculated-expected)/expected > 1.0e-9)
            CPPUNIT_FAIL("GammaFunction("
                         + IntegerFormatter::toString(i) + ")"
                         "    calculated: "
                         + DoubleFormatter::toExponential(calculated,16) + "\n"
                         "    expected:   "
                         + DoubleFormatter::toExponential(expected,16) + "\n"
                         "    rel error:   "
                         + DoubleFormatter::toExponential(QL_FABS(calculated-expected)/expected,16));
    }
}

