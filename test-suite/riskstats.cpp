
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

#include "riskstats.hpp"
#include "utilities.hpp"
#include <ql/Math/riskstatistics.hpp>
#include <ql/Math/incrementalstatistics.hpp>
#include <ql/Math/sequencestatistics.hpp>
#include <ql/RandomNumbers/sobolrsg.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

typedef GaussianStatistics<IncrementalStatistics> 
    IncrementalGaussianStatistics;

void RiskStatisticsTest::testResults() {

    BOOST_MESSAGE("Testing risk measures...");

    IncrementalGaussianStatistics igs;
    RiskStatistics s;

    Real averages[] = { -100.0, -1.0, 0.0, 1.0, 100.0 };
    Real sigmas[] = { 0.1, 1.0, 100.0 };
    Size i, j, k, N;
    N = Size(QL_POW(2.0,16))-1;
    Real dataMin, dataMax;
    std::vector<Real> data(N), weights(N);

    for (i=0; i<LENGTH(averages); i++) {
        for (j=0; j<LENGTH(sigmas); j++) {

            NormalDistribution normal(averages[i],sigmas[j]);
            CumulativeNormalDistribution cumulative(averages[i],sigmas[j]);
            InverseCumulativeNormal inverseCum(averages[i],sigmas[j]);

            SobolRsg rng(1);
            dataMin = QL_MAX_DOUBLE;
            dataMax = QL_MIN_DOUBLE;
            for (k=0; k<N; k++) {
                data[k] = inverseCum(rng.nextSequence().value[0]);
                dataMin = QL_MIN(dataMin, data[k]);
                dataMax = QL_MAX(dataMax, data[k]);
                weights[k]=1.0;
            }

            igs.addSequence(data.begin(),data.end(),weights.begin());
            s.addSequence(data.begin(),data.end(),weights.begin());

            // checks
            Real calculated, expected;
            Real tolerance;

            if (igs.samples() != N)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong number of samples\n"
                    "    calculated: "
                    + SizeFormatter::toString(igs.samples(),16) + "\n"
                    "    expected:   "
                    + SizeFormatter::toString(N,16));
            if (s.samples() != N)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong number of samples\n"
                    "    calculated: "
                    + SizeFormatter::toString(s.samples(),16) + "\n"
                    "    expected:   "
                    + SizeFormatter::toString(N,16));


            // weightSum()
            tolerance = 1e-10;
            expected = std::accumulate(weights.begin(),weights.end(),0.0);
            calculated = igs.weightSum();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong sum of weights\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.weightSum();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong sum of weights\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // min
            tolerance = 1e-12;
            expected = dataMin;
            calculated = igs.min();
            if (QL_FABS(calculated-expected)>tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong minimum value\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.min();
            if (QL_FABS(calculated-expected)>tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong minimum value\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // max
            expected = dataMax;
            calculated = igs.max();
            if (QL_FABS(calculated-expected)>tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong maximum value\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.max();
            if (QL_FABS(calculated-expected)>tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong maximum value\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // mean
            expected = averages[i];
            tolerance = (expected == 0.0 ? 1.0e-13 :
                                           QL_FABS(expected)*1.0e-13);
            calculated = igs.mean();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong mean value"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.mean();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong mean value"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // variance
            expected = sigmas[j]*sigmas[j];
            tolerance = expected*1.0e-1;
            calculated = igs.variance();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong variance"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.variance();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong variance"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // standardDeviation
            expected = sigmas[j];
            tolerance = expected*1.0e-1;
            calculated = igs.standardDeviation();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong standard deviation"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.standardDeviation();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong standard deviation"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // missing errorEstimate() test

            // skewness
            expected = 0.0;
            tolerance = 1.0e-4;
            calculated = igs.skewness();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong skewness"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.skewness();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong skewness"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // kurtosis
            expected = 0.0;
            tolerance = 1.0e-1;
            calculated = igs.kurtosis();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong kurtosis"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.kurtosis();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong kurtosis"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // percentile
            expected = averages[i];
            tolerance = (expected == 0.0 ? 1.0e-3 : QL_FABS(expected*1.0e-3));
            calculated = igs.gaussianPercentile(0.5);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong Gaussian percentile"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.gaussianPercentile(0.5);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong Gaussian percentile"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.percentile(0.5);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong percentile"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));





            // potential upside
            Real upper_tail = averages[i]+2.0*sigmas[j],
                 lower_tail = averages[i]-2.0*sigmas[j];
            Real twoSigma = cumulative(upper_tail);

            expected = QL_MAX(upper_tail,0.0);
            tolerance = (expected == 0.0 ? 1.0e-3 : QL_FABS(expected*1.0e-3));
            calculated = igs.gaussianPotentialUpside(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong Gaussian potential upside"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.gaussianPotentialUpside(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong Gaussian potential upside"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.potentialUpside(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong potential upside"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // just to check that GaussianStatistics<StatsHolder> does work
            StatsHolder h(s.mean(), s.standardDeviation());
            GaussianStatistics<StatsHolder> test(h);
            expected = s.gaussianPotentialUpside(twoSigma);
            calculated = test.gaussianPotentialUpside(twoSigma);
            if (calculated!=expected)
                BOOST_FAIL("GaussianStatistics<StatsHolder> fails");





            // value-at-risk
            expected = -QL_MIN(lower_tail,0.0);
            tolerance = (expected == 0.0 ? 1.0e-3 : QL_FABS(expected*1.0e-3));
            calculated = igs.gaussianValueAtRisk(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong Gaussian value-at-risk"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.gaussianValueAtRisk(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong Gaussian value-at-risk"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.valueAtRisk(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong value-at-risk"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));

            if (averages[i] > 0.0 && sigmas[j] < averages[i]) {
                // no data will miss the targets:
                // skip the rest of this iteration
                igs.reset();
                s.reset();
                continue;
            }


            // expected shortfall
            expected = -QL_MIN(averages[i]
                               - sigmas[j]*sigmas[j]
                               * normal(lower_tail)/(1.0-twoSigma),
                               0.0);
            tolerance = (expected == 0.0 ? 1.0e-4
                                         : QL_FABS(expected)*1.0e-2);
            calculated = igs.gaussianExpectedShortfall(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong Gaussian expected shortfall"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.gaussianExpectedShortfall(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong Gaussian expected shortfall"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.expectedShortfall(twoSigma);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong expected shortfall"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // shortfall
            expected = 0.5;
            tolerance = (expected == 0.0 ? 1.0e-3 : QL_FABS(expected*1.0e-3));
            calculated = igs.gaussianShortfall(averages[i]);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong Gaussian shortfall"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.gaussianShortfall(averages[i]);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong Gaussian shortfall"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.shortfall(averages[i]);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong shortfall"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // average shortfall
            expected = sigmas[j]/QL_SQRT(2.0*M_PI)*2.0;
            tolerance = expected*1.0e-3;
            calculated = igs.gaussianAverageShortfall(averages[i]);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong Gaussian average shortfall"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.gaussianAverageShortfall(averages[i]);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong Gaussian average shortfall"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.averageShortfall(averages[i]);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong average shortfall"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // regret
            expected = sigmas[j]*sigmas[j];
            tolerance = expected*1.0e-1;
            calculated = igs.gaussianRegret(averages[i]);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong Gaussian regret("
                    + DecimalFormatter::toString(averages[i],2) +
                    ") for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.gaussianRegret(averages[i]);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong Gaussian regret("
                    + DecimalFormatter::toString(averages[i],2) +
                    ") for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = s.regret(averages[i]);
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "Statistics: "
                    "wrong regret("
                    + DecimalFormatter::toString(averages[i],2) +
                    ") for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));


            // downsideVariance
            expected = s.downsideVariance();
            tolerance = (expected == 0.0 ? 1.0e-3 : QL_FABS(expected*1.0e-3));
            calculated = igs.downsideVariance();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong downside variance"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));
            calculated = igs.gaussianDownsideVariance();
            if (QL_FABS(calculated-expected) > tolerance)
                BOOST_FAIL(
                    "IncrementalGaussianStatistics: "
                    "wrong Gaussian downside variance"
                    " for N("
                    + DecimalFormatter::toString(averages[i],2) + ", "
                    + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                    "    calculated: "
                    + DecimalFormatter::toString(calculated,16) + "\n"
                    "    expected:   "
                    + DecimalFormatter::toString(expected,16) + "\n"
                    "    tolerance:   "
                    + DecimalFormatter::toString(tolerance,16));

            // downsideVariance
            if (averages[i]==0.0) {
                expected = sigmas[j]*sigmas[j];
                tolerance = expected*1.0e-3;
                calculated = igs.downsideVariance();
                if (QL_FABS(calculated-expected) > tolerance)
                    BOOST_FAIL(
                        "IncrementalGaussianStatistics: "
                        "wrong downside variance"
                        " for N("
                        + DecimalFormatter::toString(averages[i],2) + ", "
                        + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                        "    calculated: "
                        + DecimalFormatter::toString(calculated,16) + "\n"
                        "    expected:   "
                        + DecimalFormatter::toString(expected,16) + "\n"
                        "    tolerance:   "
                        + DecimalFormatter::toString(tolerance,16));
                calculated = igs.gaussianDownsideVariance();
                if (QL_FABS(calculated-expected) > tolerance)
                    BOOST_FAIL(
                        "IncrementalGaussianStatistics: "
                        "wrong Gaussian downside variance"
                        " for N("
                        + DecimalFormatter::toString(averages[i],2) + ", "
                        + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                        "    calculated: "
                        + DecimalFormatter::toString(calculated,16) + "\n"
                        "    expected:   "
                        + DecimalFormatter::toString(expected,16) + "\n"
                        "    tolerance:   "
                        + DecimalFormatter::toString(tolerance,16));
                calculated = s.downsideVariance();
                if (QL_FABS(calculated-expected) > tolerance)
                    BOOST_FAIL(
                        "Statistics: "
                        "wrong downside variance"
                        " for N("
                        + DecimalFormatter::toString(averages[i],2) + ", "
                        + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                        "    calculated: "
                        + DecimalFormatter::toString(calculated,16) + "\n"
                        "    expected:   "
                        + DecimalFormatter::toString(expected,16) + "\n"
                        "    tolerance:   "
                        + DecimalFormatter::toString(tolerance,16));
                calculated = s.gaussianDownsideVariance();
                if (QL_FABS(calculated-expected) > tolerance)
                    BOOST_FAIL(
                        "Statistics: "
                        "wrong gaussian downside variance"
                        " for N("
                        + DecimalFormatter::toString(averages[i],2) + ", "
                        + DecimalFormatter::toString(sigmas[j],2) + ")\n"
                        "    calculated: "
                        + DecimalFormatter::toString(calculated,16) + "\n"
                        "    expected:   "
                        + DecimalFormatter::toString(expected,16) + "\n"
                        "    tolerance:   "
                        + DecimalFormatter::toString(tolerance,16));
            }

            igs.reset();
            s.reset();

        }
    }
}


test_suite* RiskStatisticsTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Risk statistics tests");
    suite->add(BOOST_TEST_CASE(&RiskStatisticsTest::testResults));
    return suite;
}

