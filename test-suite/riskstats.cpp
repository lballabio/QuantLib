
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

#include "riskstats.hpp"

using namespace QuantLib;
using namespace QuantLib::Math;
using namespace QuantLib::RandomNumbers;

#define LENGTH(a) (sizeof(a)/sizeof(a[0]))

void RiskStatisticsTest::runTest() {


    Statistics   s;
    HStatistics hs;

    unsigned long dimension = 5;
    SequenceStatistics<>             ss(dimension);
    SequenceStatistics<HStatistics> hss(dimension);
            
    double averages[] = { -100.0, -1.0, 0.0, 1.0, 100.0 };
    double sigmas[] = { 0.1, 1.0, 100.0 };
//    Size N = Size(QL_POW(2,18))-1;
    Size N = Size(QL_POW(2.0,14)), numberOfSigma = 15;

    for (Size i=0; i<LENGTH(averages); i++) {
        for (Size j=0; j<LENGTH(sigmas); j++) {

            
            std::vector<double> data(N), weights(N);
            NormalDistribution               normal(averages[i],sigmas[j]);
            CumulativeNormalDistribution cumulative(averages[i],sigmas[j]);
            InverseCumulativeNormal      inverseCum(averages[i],sigmas[j]);
            SobolRsg rng(1);


            double dataMin = averages[i] - numberOfSigma*sigmas[j],
                   dataMax = averages[i] + numberOfSigma*sigmas[j];
            double h = (dataMax-dataMin)/(N-1);

            for (Size k=0; k<N; k++)
                data[k] = dataMin + h*k;
            std::transform(data.begin(),data.end(),weights.begin(),normal);


            /*
            double dataMin = QL_MAX_DOUBLE;
            double dataMax = QL_MIN_DOUBLE;
            for (Size k=0; k<N; k++) {
                data[k] = inverseCum(rng.nextSequence().value[0]);
                dataMin = QL_MIN(dataMin, data[k]);
                dataMax = QL_MAX(dataMax, data[k]);
                weights[k]=1.0;
//                weights[k]=normal(data[k]);
            }
            */

            s.addSequence(data.begin(),data.end(),weights.begin());
            hs.addSequence(data.begin(),data.end(),weights.begin());

            // checks
            double calculated, expected;
            double tolerance;

            expected = N;
            calculated = s.samples();
            if (calculated != expected)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong number of samples\n"
                    "    calculated: "
                    + IntegerFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + IntegerFormatter::toString(expected));
            calculated = hs.samples();
            if (calculated != expected)
                CPPUNIT_FAIL(
                    "HStatistics: "
                    "wrong number of samples\n"
                    "    calculated: "
                    + IntegerFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + IntegerFormatter::toString(expected));






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


            tolerance = 1e-12;
            expected = dataMin;
            calculated = s.min();
            if (QL_FABS(calculated-expected)>tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong minimum value\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));
            calculated = hs.min();
            if (QL_FABS(calculated-expected)>tolerance)
                CPPUNIT_FAIL(
                    "HStatistics: "
                    "wrong minimum value\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));







            expected = dataMax;
            calculated = s.max();
            if (QL_FABS(calculated-expected)>tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong maximum value\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));
            calculated = hs.max();
            if (QL_FABS(calculated-expected)>tolerance)
                CPPUNIT_FAIL(
                    "HStatistics: "
                    "wrong maximum value\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));





            expected = averages[i];
            tolerance = (expected == 0.0 ? 1.0e-13 :
                                           QL_FABS(expected)*1.0e-13);
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






            expected = sigmas[j]*sigmas[j];
            tolerance = expected*1.0e-4;
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






            expected = sigmas[j];
            tolerance = expected*1.0e-4;
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





            expected = 0.0;
            tolerance = 1.0e-4;
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


            expected = 0.0;
            tolerance = 1.0e-1;
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


            double upper_tail = averages[i]+2.0*sigmas[j],
                   lower_tail = averages[i]-2.0*sigmas[j];
            double twoSigma = cumulative(upper_tail);
            expected = QL_MAX(upper_tail,0.0);
            tolerance = (expected == 0.0 ? 1.0e-3 : expected*1.0e-3);
            calculated = s.gaussianPotentialUpside(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong potential upside\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));
            calculated = hs.gaussianPotentialUpside(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "HStatistics: "
                    "wrong potential upside\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));





            expected = -QL_MIN(lower_tail,0.0);
            tolerance = (expected == 0.0 ? 1.0e-3 : expected*1.0e-3);
            calculated = s.gaussianValueAtRisk(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong value at risk\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));
            calculated = hs.gaussianValueAtRisk(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "HStatistics: "
                    "wrong value at risk\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));


            expected = -QL_MIN(averages[i]
                               - sigmas[j]*sigmas[j]
                               * normal(lower_tail)/(1.0-twoSigma),
                               0.0);
            tolerance = (expected == 0.0 ? 1.0e-4
                                         : QL_FABS(expected)*1.0e-4);
            calculated = s.gaussianExpectedShortfall(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong expected shortfall\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));
            calculated = hs.gaussianExpectedShortfall(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "HStatistics: "
                    "wrong expected shortfall\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));


            double target = averages[i];
            expected = 0.5;
            tolerance = 1.0e-8;
            calculated = s.gaussianShortfall(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong shortfall\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));
            calculated = hs.gaussianShortfall(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "HStatistics: "
                    "wrong shortfall\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));


            expected = sigmas[j]/QL_SQRT(2.0*M_PI);
            tolerance = expected*1.0e-4;
            calculated = s.gaussianAverageShortfall(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong average shortfall\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));
            calculated = hs.gaussianAverageShortfall(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "HStatistics: "
                    "wrong average shortfall\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected));

            s.reset();
            hs.reset();
        }
    }
}

