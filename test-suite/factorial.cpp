
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
#include <ql/dataformatters.hpp>
#include <ql/Math/poissondistribution.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void FactorialTest::testFactorial() {

    BOOST_MESSAGE("Testing factorial numbers...");

    unsigned int i;
    double expected = 1.0;
    double calculated = Factorial::get(0);
    if (calculated!=expected)
        BOOST_FAIL("Factorial(0)"
                   "    calculated: "
                   + DoubleFormatter::toExponential(calculated,16) + "\n"
                   "    expected:   "
                   + DoubleFormatter::toExponential(expected,16));
    for (i=1; i<28; i++) {
        expected *= i;
        calculated   = Factorial::get(i);
        if (calculated!=expected)
            BOOST_FAIL("Factorial("
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
            BOOST_FAIL("Factorial("
                       + IntegerFormatter::toString(i) + ")"
                       "    calculated: "
                       + DoubleFormatter::toExponential(calculated,16) + "\n"
                       "    expected:   "
                       + DoubleFormatter::toExponential(expected,16) + "\n"
                       "    rel error:   "
                       + DoubleFormatter::toExponential(
                                   QL_FABS(calculated-expected)/expected,16));
    }
}

void FactorialTest::testGammaFunction() {

    BOOST_MESSAGE("Testing Gamma function...");

    double expected = 0.0;
    double calculated = GammaFunction().logValue(1);
    if (QL_FABS(calculated) > 1.0e-15)
        BOOST_FAIL("GammaFunction("
                   + IntegerFormatter::toString(1) + ")"
                   "    calculated: "
                   + DoubleFormatter::toExponential(calculated,16) + "\n"
                   "    expected:   "
                   + DoubleFormatter::toExponential(expected,16));

    for (Size i=2; i<9000; i++) {
        expected  += QL_LOG(double(i));
        calculated = GammaFunction().logValue(i+1);
        if (QL_FABS(calculated-expected)/expected > 1.0e-9)
            BOOST_FAIL("GammaFunction("
                       + SizeFormatter::toString(i) + ")"
                       "    calculated: "
                       + DoubleFormatter::toExponential(calculated,16) + "\n"
                       "    expected:   "
                       + DoubleFormatter::toExponential(expected,16) + "\n"
                       "    rel error:   "
                       + DoubleFormatter::toExponential(
                                   QL_FABS(calculated-expected)/expected,16));
    }
}




void FactorialTest::testPoissonDistribution() {

    BOOST_MESSAGE("Testing Poisson distribution...");

    double mean = 0.0;
    unsigned long i = 0;
    PoissonDistribution pdf(mean);
    double calculated = pdf(i);
    double expected = 1.0;
    double error = QL_FABS(calculated-expected);
    if (error>1.0e-16)
        BOOST_FAIL("Poisson pdf("
                   + DoubleFormatter::toString(mean) + ")("
                   + IntegerFormatter::toString(i) + ")\n"
                   "    calculated: "
                   + DoubleFormatter::toString(calculated,16) + "\n"
                   "    expected:   "
                   + DoubleFormatter::toString(expected,16)+
                   "    error:   "
                   + DoubleFormatter::toExponential(error));

    CumulativePoissonDistribution cdf(mean);
    double cumCalculated = cdf(i);
    double cumExpected = 1.0;
    if (cumCalculated!=cumExpected)
        BOOST_FAIL("Poisson cdf("
                   + DoubleFormatter::toString(mean) + ")("
                   + IntegerFormatter::toString(i) + ")\n"
                   "    calculated: "
                   + DoubleFormatter::toString(cumCalculated,16) + "\n"
                   "    expected:   "
                   + DoubleFormatter::toString(cumExpected,16)+
                   "    error:   "
                   + DoubleFormatter::toExponential(error));

    for (i=1; i<25; i++) {
        calculated = pdf(i);
        expected = 0.0;
        if (calculated!=expected)
            BOOST_FAIL("Poisson pdf("
                       + DoubleFormatter::toString(mean) + ")("
                       + IntegerFormatter::toString(i) + ")\n"
                       "    calculated: "
                       + DoubleFormatter::toString(calculated,16) + "\n"
                       "    expected:   "
                       + DoubleFormatter::toString(expected,16)+
                       "    error:   "
                       + DoubleFormatter::toExponential(error));
        cumCalculated = cdf(i);
        cumExpected = 1.0;
        if (cumCalculated!=cumExpected)
            BOOST_FAIL("Poisson cdf("
                       + DoubleFormatter::toString(mean) + ")("
                       + IntegerFormatter::toString(i) + ")\n"
                       "    calculated: "
                       + DoubleFormatter::toString(cumCalculated,16) + "\n"
                       "    expected:   "
                       + DoubleFormatter::toString(cumExpected,16)+
                       "    error:   "
                       + DoubleFormatter::toExponential(error));
    }


    for (mean=0.5; mean<=10; mean+=0.5) {
        i = 0;
        pdf = PoissonDistribution(mean);
        calculated = pdf(i);
        double logHelper = -mean;
        expected = QL_EXP(logHelper);
        error = QL_FABS(calculated-expected);
        if (error>1e-16)
            BOOST_FAIL("Poisson pdf("
                       + DoubleFormatter::toString(mean) + ")("
                       + IntegerFormatter::toString(i) + ")\n"
                       "    calculated: "
                       + DoubleFormatter::toString(calculated,16) + "\n"
                       "    expected:   "
                       + DoubleFormatter::toString(expected,16)+
                       "    error:   "
                       + DoubleFormatter::toExponential(error));

        cdf = CumulativePoissonDistribution(mean);
        cumCalculated = cdf(i);
        cumExpected = expected;
        error = QL_FABS(cumCalculated-cumExpected);
        if (error>1.0e-13)
            BOOST_FAIL("Poisson cdf("
                       + DoubleFormatter::toString(mean) + ")("
                       + IntegerFormatter::toString(i) + ")\n"
                       "    calculated: "
                       + DoubleFormatter::toString(cumCalculated,16) + "\n"
                       "    expected:   "
                       + DoubleFormatter::toString(cumExpected,16)+
                       "    error:   "
                       + DoubleFormatter::toExponential(error));
        for (unsigned long i=1; i<25; i++) {
            calculated = pdf(i);
            logHelper = logHelper+QL_LOG(mean)-QL_LOG(double(i));
            expected = QL_EXP(logHelper);
            error = QL_FABS(calculated-expected);
            if (error>1.0e-13)
                BOOST_FAIL("Poisson pdf("
                           + DoubleFormatter::toString(mean) + ")("
                           + IntegerFormatter::toString(i) + ")\n"
                           "    calculated: "
                           + DoubleFormatter::toString(calculated,16) + "\n"
                           "    expected:   "
                           + DoubleFormatter::toString(expected,16)+
                           "    error:   "
                           + DoubleFormatter::toExponential(error));
            cumCalculated = cdf(i);
            cumExpected += expected;
            error = QL_FABS(cumCalculated-cumExpected);
            if (error>1.0e-12)
                BOOST_FAIL("Poisson cdf("
                           + DoubleFormatter::toString(mean) + ")("
                           + IntegerFormatter::toString(i) + ")\n"
                           "    calculated: "
                           + DoubleFormatter::toString(cumCalculated,16) + "\n"
                           "    expected:   "
                           + DoubleFormatter::toString(cumExpected,16)+
                           "    error:   "
                           + DoubleFormatter::toExponential(error));
        }
    }
}


test_suite* FactorialTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Factorial tests");
    suite->add(BOOST_TEST_CASE(&FactorialTest::testFactorial));
    suite->add(BOOST_TEST_CASE(&FactorialTest::testGammaFunction));
    suite->add(BOOST_TEST_CASE(&FactorialTest::testPoissonDistribution));
    return suite;
}

