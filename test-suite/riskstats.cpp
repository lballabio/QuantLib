
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

#include "riskstats.hpp"

using namespace QuantLib;
using QuantLib::Math::NormalDistribution;
using QuantLib::Math::CumulativeNormalDistribution;

#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

void RiskStatisticsTest::runTest() {

    RiskStatistics s;

    double averages[] = { -100.0, 0.0, 100.0 };
    double sigmas[] = { 0.1, 1.0, 10.0 };
    Size N = 25000,
        numberOfSigma = 15;

    for (Size i=0; i<LENGTH(averages); i++) {
        for (Size j=0; j<LENGTH(sigmas); j++) {

            NormalDistribution normal(averages[i],sigmas[j]);
            CumulativeNormalDistribution cumulative(averages[i],sigmas[j]);

            double dataMin = averages[i] - numberOfSigma*sigmas[j],
                   dataMax = averages[i] + numberOfSigma*sigmas[j];
            double h = (dataMax-dataMin)/(N-1);

            std::vector<double> data(N), weights(N);
            for (Size k=0; k<N; k++)
                data[k] = dataMin + h*k;
            std::transform(data.begin(),data.end(),weights.begin(),normal);

            s.addSequence(data.begin(),data.end(),weights.begin());

            // checks
            double calculated, expected;
            double tolerance;

            if (s.samples() != N)
                CPPUNIT_FAIL(
                    "wrong number of samples\n"
                    "    calculated: "
                    + IntegerFormatter::toString(s.samples()) + "\n"
                    "    expected:   "
                    + IntegerFormatter::toString(N));

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
            expected = dataMin;
            if (calculated != expected)
                CPPUNIT_FAIL(
                    "wrong minimum value\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            calculated = s.max();
            expected = dataMax;
            if (QL_FABS(calculated- expected) > 1.0e-13)
                CPPUNIT_FAIL(
                    "wrong maximum value\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            calculated = s.mean();
            expected = averages[i];
            tolerance = (expected == 0.0 ? 1.0e-13 :
                                           QL_FABS(expected)*1.0e-13);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "wrong mean value\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            calculated = s.variance();
            expected = sigmas[j]*sigmas[j];
            tolerance = expected*1.0e-4;
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "wrong variance\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            calculated = s.standardDeviation();
            expected = sigmas[j];
            tolerance = expected*1.0e-4;
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "wrong standard deviation\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            calculated = s.skewness();
            expected = 0.0;
            tolerance = 1.0e-4;
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "wrong skewness\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            calculated = s.kurtosis();
            expected = 0.0;
            tolerance = 1.0e-1;
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "wrong kurtosis\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            double upper_tail = averages[i]+2.0*sigmas[j],
                   lower_tail = averages[i]-2.0*sigmas[j];
            double twoSigma = cumulative(upper_tail);
            calculated = s.potentialUpside(twoSigma);
            expected = QL_MAX(upper_tail,0.0);
            tolerance = (expected == 0.0 ? 1.0e-3 : expected*1.0e-3);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "wrong potential upside\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            calculated = s.valueAtRisk(twoSigma);
            expected = -QL_MIN(lower_tail,0.0);
            tolerance = (expected == 0.0 ? 1.0e-3 : expected*1.0e-3);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "wrong value at risk\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            calculated = s.expectedShortfall(twoSigma);
            expected = -QL_MIN(averages[i]
                               - sigmas[j]*sigmas[j]
                               * normal(lower_tail)/(1.0-twoSigma),
                               0.0);
            tolerance = (expected == 0.0 ? 1.0e-4
                                         : QL_FABS(expected)*1.0e-4);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "wrong expected shortfall\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            double target = averages[i];
            calculated = s.shortfall(target);
            expected = 0.5;
            tolerance = 1.0e-8;
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "wrong shortfall\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            calculated = s.averageShortfall(target);
            expected = sigmas[j]/QL_SQRT(2.0*M_PI);
            tolerance = expected*1.0e-4;
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "wrong average shortfall\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            s.reset();
        }
    }
}

