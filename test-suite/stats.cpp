
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
// $Id$

#include "stats.hpp"

using namespace QuantLib;
using QuantLib::Math::Statistic;

#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

void StatisticsTest::runTest() {

    double data[] = { 3.0, 4.0, 5.0, 2.0, 3.0, 4.0, 5.0, 6.0, 4.0, 7.0 };
    std::vector<double> weights(LENGTH(data),1.0);

    Statistic s;
    s.addSequence(data,data+LENGTH(data),weights.begin());

    double calculated, expected;
    double tolerance;

    if (s.samples() != LENGTH(data))
        CPPUNIT_FAIL(
            "wrong number of samples\n"
            "    calculated: "
            + IntegerFormatter::toString(s.samples()) + "\n"
            "    expected:   "
            + IntegerFormatter::toString(LENGTH(data)));

    calculated = s.weightSum();
    expected = std::accumulate(weights.begin(),weights.end(),0.0);
    if (calculated != expected)
        CPPUNIT_FAIL(
            "wrong sum of weights\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));

    calculated = s.min();
    expected = *std::min_element(data,data+LENGTH(data));
    if (calculated != expected)
        CPPUNIT_FAIL(
            "wrong minimum value\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));

    calculated = s.max();
    expected = *std::max_element(data,data+LENGTH(data));
    if (calculated != expected)
        CPPUNIT_FAIL(
            "wrong maximum value\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));

    calculated = s.mean();
    expected = 4.3;
    tolerance = 1.0e-9;
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "wrong mean value\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));

    calculated = s.variance();
    expected = 2.23333333333;
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "wrong variance\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
            
    calculated = s.standardDeviation();
    expected = 1.4944341181;
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "wrong standard deviation\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));

    calculated = s.skewness();
    expected = 0.359543071407;
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "wrong skewness\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));

    calculated = s.kurtosis();
    expected = -0.151799637209;
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "wrong kurtosis\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));

    s.reset();

    std::vector<double> data2(LENGTH(data));
    std::transform(data,data+LENGTH(data),data2.begin(),
                   std::bind2nd(std::minus<double>(),3.0));
    s.addSequence(data2.begin(),data2.end(),weights.begin());

    calculated = s.downsideDeviation();
    expected = 0.333333333;
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "wrong downside deviation\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));

}

