
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

    IncrementalGaussianStatistics   igs;
    Statistics s;

    unsigned long dimension = 5;
    SequenceStatistics<IncrementalGaussianStatistics> igss(dimension);
    SequenceStatistics<Statistics> ss(dimension);
            
    double averages[] = { -100.0, -1.0, 0.0, 1.0, 100.0 };
    double sigmas[] = { 0.1, 1.0, 100.0 };
    Size i, j, k, N;
    N = Size(QL_POW(2.0,16))-1;
    double dataMin, dataMax;
    std::vector<double> data(N), weights(N);

    for (i=0; i<LENGTH(averages); i++) {
        for (j=0; j<LENGTH(sigmas); j++) {

            
            NormalDistribution               normal(averages[i],sigmas[j]);
            CumulativeNormalDistribution cumulative(averages[i],sigmas[j]);

/*
            Size numberOfSigma = 15;
            dataMin = averages[i] - numberOfSigma*sigmas[j];
            dataMax = averages[i] + numberOfSigma*sigmas[j];
            double h = (dataMax-dataMin)/(N-1);
            for (k=0; k<N; k++)
                data[k] = dataMin + h*k;
            std::transform(data.begin(),data.end(),weights.begin(),normal);
*/

            InverseCumulativeNormal inverseCum(averages[i],sigmas[j]);
            SobolRsg rng(1);
            dataMin = QL_MAX_DOUBLE;
            dataMax = QL_MIN_DOUBLE;
            for (k=0; k<N; k++) {
                data[k] = inverseCum(rng.nextSequence().value[0]);
                dataMin = QL_MIN(dataMin, data[k]);
                dataMax = QL_MAX(dataMax, data[k]);
                weights[k]=1.0;
//                weights[k]=normal(data[k]);
            }


            igs.addSequence(data.begin(),data.end(),weights.begin());
            s.addSequence(data.begin(),data.end(),weights.begin());

            // checks
            double calculated, expected;
            double tolerance;

            if (igs.samples() != N)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong number of samples\n"
                    "    calculated: "
                    + IntegerFormatter::toString(igs.samples(),16) + "\n"
                    "    expected:   "
                    + IntegerFormatter::toString(N,16));
            if (s.samples() != N)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong number of samples\n"
                    "    calculated: "
                    + IntegerFormatter::toString(s.samples(),16) + "\n"
                    "    expected:   "
                    + IntegerFormatter::toString(N,16));


            // weightSum()
            tolerance = 1e-10;
            expected = std::accumulate(weights.begin(),weights.end(),0.0);
            calculated = igs.weightSum();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong sum of weights\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.weightSum();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong sum of weights\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));


            // min
            tolerance = 1e-12;
            expected = dataMin;
            calculated = igs.min();
            if (QL_FABS(calculated-expected)>tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong minimum value\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.min();
            if (QL_FABS(calculated-expected)>tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong minimum value\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));


            // max
            expected = dataMax;
            calculated = igs.max();
            if (QL_FABS(calculated-expected)>tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong maximum value\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.max();
            if (QL_FABS(calculated-expected)>tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong maximum value\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));


            // mean
            expected = averages[i];
            tolerance = (expected == 0.0 ? 1.0e-13 :
                                           QL_FABS(expected)*1.0e-13);
            calculated = igs.mean();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong mean value"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.mean();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong mean value"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));


            // variance
            expected = sigmas[j]*sigmas[j];
            tolerance = expected*1.0e-1;
            calculated = igs.variance();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong variance"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.variance();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong variance"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));


            // standardDeviation
            expected = sigmas[j];
            tolerance = expected*1.0e-1;
            calculated = igs.standardDeviation();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong standard deviation"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.standardDeviation();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong standard deviation"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));


            // missing errorEstimate() test

            // skewness
            expected = 0.0;
            tolerance = 1.0e-4;
            calculated = igs.skewness();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong skewness"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.skewness();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong skewness"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));


            // kurtosis
            expected = 0.0;
            tolerance = 1.0e-1;
            calculated = igs.kurtosis();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong kurtosis"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.kurtosis();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong kurtosis"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));


            // percentile
            expected = averages[i];
            tolerance = (expected == 0.0 ? 1.0e-3 : QL_FABS(expected*1.0e-3));
            calculated = igs.gaussianPercentile(0.5);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong gaussian percentile"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.gaussianPercentile(0.5);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong gaussian percentile"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.percentile(0.5);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong percentile"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));





            // potential upside
            double upper_tail = averages[i]+2.0*sigmas[j],
                   lower_tail = averages[i]-2.0*sigmas[j];
            double twoSigma = cumulative(upper_tail);
            expected = QL_MAX(upper_tail,0.0);
            tolerance = (expected == 0.0 ? 1.0e-3 : QL_FABS(expected*1.0e-3));
            calculated = igs.gaussianPotentialUpside(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong gaussian potential upside"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.gaussianPotentialUpside(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong gaussian potential upside"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.potentialUpside(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong gaussian potential upside"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));


            // value-at-risk
            expected = -QL_MIN(lower_tail,0.0);
            tolerance = (expected == 0.0 ? 1.0e-3 : QL_FABS(expected*1.0e-3));
            calculated = igs.gaussianValueAtRisk(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong gaussian value-at-risk"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.gaussianValueAtRisk(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong gaussian value-at-risk"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.valueAtRisk(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong gaussian value-at-risk"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));

            
            
            // expected shortfall
            expected = -QL_MIN(averages[i]
                               - sigmas[j]*sigmas[j]
                               * normal(lower_tail)/(1.0-twoSigma),
                               0.0);
            tolerance = (expected == 0.0 ? 1.0e-4
                                         : QL_FABS(expected)*1.0e-2);
            calculated = igs.gaussianExpectedShortfall(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong gaussian expected shortfall"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.gaussianExpectedShortfall(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong gaussian expected shortfall"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.expectedShortfall(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong expected shortfall"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));




            // shortfall
            double target = averages[i];
            expected = 0.5;
            tolerance = (expected == 0.0 ? 1.0e-3 : QL_FABS(expected*1.0e-3));
            calculated = igs.gaussianShortfall(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong gaussian shortfall"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.gaussianShortfall(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong gaussian shortfall"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.shortfall(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong shortfall"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));




            // average shortfall
            expected = sigmas[j]/QL_SQRT(2.0*M_PI);
            tolerance = expected*1.0e-3;
            calculated = igs.gaussianAverageShortfall(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong gaussian average shortfall"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.gaussianAverageShortfall(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong gaussian average shortfall"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.averageShortfall(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong average shortfall"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));


            // regret
            expected = sigmas[j]*sigmas[j];
            tolerance = expected*1.0e-1;
            calculated = igs.gaussianRegret(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong gaussian regret("
                    + DoubleFormatter::toString(target,2) +
                    ") for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.gaussianRegret(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong gaussian regret("
                    + DoubleFormatter::toString(target,2) +
                    ") for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = s.regret(target);
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "Statistics: "
                    "wrong regret("
                    + DoubleFormatter::toString(target,2) +
                    ") for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));


            // downsideVariance
            expected = s.downsideVariance();
            tolerance = (expected == 0.0 ? 1.0e-3 : QL_FABS(expected*1.0e-3));
            calculated = igs.downsideVariance();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong downside variance"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            calculated = igs.gaussianDownsideVariance();
            if (QL_FABS(calculated-expected) > tolerance)
                CPPUNIT_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong gaussian downside variance"
                    " for N("
                    + DoubleFormatter::toString(averages[i],2) + ", "
                    + DoubleFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DoubleFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DoubleFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DoubleFormatter::toString(tolerance,16));
            
            igs.reset();
            s.reset();
        }
    }
}

