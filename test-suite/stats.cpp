
/*
 Copyright (C) 2003 Ferdinando Ametrano
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
using namespace QuantLib::Math;

#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

void StatisticsTest::runTest() {

    double data[] = { 3.0, 4.0, 5.0, 2.0, 3.0, 4.0, 5.0, 6.0, 4.0, 7.0 };
    std::vector<double> weights(LENGTH(data),1.0);

    Statistics s;
    s.addSequence(data,data+LENGTH(data),weights.begin());

    HStatistics hs;
    hs.addSequence(data,data+LENGTH(data),weights.begin());

    unsigned long dimension = 5;
    std::vector<std::vector<double > > sequenceData(LENGTH(data));
    SequenceStatistics<> ss(dimension);
    SequenceStatistics<HStatistics> hss(dimension);
    for (Size i = 0; i<LENGTH(data); i++) {
        std::vector<double> temp(dimension, data[i]);
        ss.add(temp, weights[i]);
        hss.add(temp, weights[i]);
    }

    double calculated, expected;
    double tolerance;
    std::vector<double> calculatedSequence;


    
    
    
    if (s.samples() != LENGTH(data))
        CPPUNIT_FAIL(
            "Statistics: "
            "wrong number of samples\n"
            "    calculated: "
            + IntegerFormatter::toString(s.samples()) + "\n"
            "    expected:   "
            + IntegerFormatter::toString(LENGTH(data)));
    if (hs.samples() != LENGTH(data))
        CPPUNIT_FAIL(
            "HStatistics: "
            "wrong number of samples\n"
            "    calculated: "
            + IntegerFormatter::toString(s.samples()) + "\n"
            "    expected:   "
            + IntegerFormatter::toString(LENGTH(data)));
    if (ss.samples() != LENGTH(data))
        CPPUNIT_FAIL(
            "SequenceStatistics<Statistics>: "
            "wrong number of samples\n"
            "    calculated: "
            + IntegerFormatter::toString(s.samples()) + "\n"
            "    expected:   "
            + IntegerFormatter::toString(LENGTH(data)));
    if (hss.samples() != LENGTH(data))
        CPPUNIT_FAIL(
            "SequenceStatistics<HStatistics>: "
            "wrong number of samples\n"
            "    calculated: "
            + IntegerFormatter::toString(s.samples()) + "\n"
            "    expected:   "
            + IntegerFormatter::toString(LENGTH(data)));






    expected = std::accumulate(weights.begin(),weights.end(),0.0);
    calculated = s.weightSum();
    if (calculated != expected)
        CPPUNIT_FAIL(
            "Statistics: "
            "wrong sum of weights\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = hs.weightSum();
    if (calculated != expected)
        CPPUNIT_FAIL(
            "HStatistics: "
            "wrong sum of weights\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = ss.weightSum();
    if (calculated != expected)
        CPPUNIT_FAIL(
            "SequenceStatistics<Statistics>: "
            "wrong sum of weights\n"
            "    calculated: "
            + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = hss.weightSum();
    if (calculated != expected)
        CPPUNIT_FAIL(
            "SequenceStatistics<HStatistics>: "
            "wrong sum of weights\n"
            "    calculated: "
            + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));




    expected = *std::min_element(data,data+LENGTH(data));
    calculated = s.min();
    if (calculated != expected)
        CPPUNIT_FAIL(
            "Statistics: "
            "wrong minimum value\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = hs.min();
    if (calculated != expected)
        CPPUNIT_FAIL(
            "HStatistics: "
            "wrong minimum value\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculatedSequence = ss.min();
    for (i=0; i<dimension; i++) {
        if (calculatedSequence[i] != expected)
            CPPUNIT_FAIL(
                "SequenceStatistics<Statistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong minimum value\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }
    calculatedSequence = hss.min();
    for (i=0; i<dimension; i++) {
        if (calculatedSequence[i] != expected)
            CPPUNIT_FAIL(
                "SequenceStatistics<HStatistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong minimum value\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }




    expected = *std::max_element(data,data+LENGTH(data));
    calculated = s.max();
    if (calculated != expected)
        CPPUNIT_FAIL(
            "Statistics: "
            "wrong maximum value\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = hs.max();
    if (calculated != expected)
        CPPUNIT_FAIL(
            "HStatistics: "
            "wrong maximum value\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculatedSequence = ss.max();
    for (i=0; i<dimension; i++) {
        if (calculatedSequence[i] != expected)
            CPPUNIT_FAIL(
                "SequenceStatistics<Statistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong maximun value\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }
    calculatedSequence = hss.max();
    for (i=0; i<dimension; i++) {
        if (calculatedSequence[i] != expected)
            CPPUNIT_FAIL(
                "SequenceStatistics<HStatistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong maximun value\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }




    expected = 4.3;
    tolerance = 1.0e-9;
    calculated = s.mean();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "Statistics: "
            "wrong mean value\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = hs.mean();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "HStatistics: "
            "wrong mean value\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculatedSequence = ss.mean();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<Statistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong mean value\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }
    calculatedSequence = hss.mean();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<HStatistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong mean value\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }


    
    expected = 2.23333333333;
    calculated = s.variance();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "Statistics: "
            "wrong variance\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = hs.variance();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "HStatistics: "
            "wrong variance\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculatedSequence = ss.variance();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<Statistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong variance\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }
    calculatedSequence = hss.variance();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<HStatistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong variance\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }
            

    
    expected = 1.4944341181;
    calculated = s.standardDeviation();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "Statistics: "
            "wrong standard deviation\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = hs.standardDeviation();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "HStatistics: "
            "wrong standard deviation\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculatedSequence = ss.standardDeviation();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<Statistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong standard deviation\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }
    calculatedSequence = hss.standardDeviation();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<HStatistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong standard deviation\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }


    
    
    expected = 0.359543071407;
    calculated = s.skewness();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "Statistics: "
            "wrong skewness\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = hs.skewness();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "HStatistics: "
            "wrong skewness\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculatedSequence = ss.skewness();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<Statistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong skewness\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }
    calculatedSequence = hss.skewness();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<HStatistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong skewness\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }



    expected = -0.151799637209;
    calculated = s.kurtosis();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "Statistics: "
            "wrong kurtosis\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = hs.kurtosis();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "HStatistics: "
            "wrong kurtosis\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculatedSequence = ss.kurtosis();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<Statistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong kurtosis\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }
    calculatedSequence = hss.kurtosis();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<HStatistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong kurtosis\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }


    

    
    s.reset();
    hs.reset();
    ss.reset();
    hss.reset();


    
    std::vector<double> data2(LENGTH(data));
    std::transform(data,data+LENGTH(data),data2.begin(),
                   std::bind2nd(std::minus<double>(),3.0));
    s.addSequence(data2.begin(),data2.end(),weights.begin());
    hs.addSequence(data2.begin(),data2.end(),weights.begin());

    for (i = 0; i<LENGTH(data); i++) {
        std::vector<double> temp(dimension, data2[i]);
        ss.add(temp, weights[i]);
        hss.add(temp, weights[i]);
    }



    expected = 0.111111111;
    calculated = s.downsideVariance();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "Statistics: "
            "wrong downside variance\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = hs.downsideVariance();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "HStatistics: "
            "wrong downside variance\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculatedSequence = ss.downsideVariance();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<Statistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong downside variance\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }
    calculatedSequence = hss.downsideVariance();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<HStatistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong downside variance\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }

    
    
    
    
    
    
    expected = 0.333333333;
    calculated = s.downsideDeviation();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "Statistics: "
            "wrong downside deviation\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculated = hs.downsideDeviation();
    if (QL_FABS(calculated-expected) > tolerance)
        CPPUNIT_FAIL(
            "HStatistics: "
            "wrong downside deviation\n"
            "    calculated: "
            + DoubleFormatter::toString(calculated) + "\n"
            "    expected:   "
            + DoubleFormatter::toString(expected));
    calculatedSequence = ss.downsideDeviation();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<Statistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong downside deviation\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }
    calculatedSequence = hss.downsideDeviation();
    for (i=0; i<dimension; i++) {
        if (QL_FABS(calculatedSequence[i]-expected) > tolerance)
            CPPUNIT_FAIL(
                "SequenceStatistics<HStatistics>: "
                + IntegerFormatter::toOrdinal(i+1) + " dimension: "
                "wrong downside deviation\n"
                "    calculated: "
                + DoubleFormatter::toString(calculatedSequence[i]) + "\n"
                "    expected:   "
                + DoubleFormatter::toString(expected));
    }

}

