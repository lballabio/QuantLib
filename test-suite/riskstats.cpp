/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2003 RiskMap srl

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

#include "riskstats.hpp"
#include "utilities.hpp"
#include <ql/math/statistics/riskstatistics.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <ql/math/comparison.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void RiskStatisticsTest::testResults() {

    BOOST_TEST_MESSAGE("Testing risk measures...");

    GenericGaussianStatistics<IncrementalStatistics> igs;
    RiskStatistics s;

    Real averages[] = { -100.0, -1.0, 0.0, 1.0, 100.0 };
    Real sigmas[] = { 0.1, 1.0, 100.0 };
    Size i, j, k, N;
    N = Size(std::pow(2.0,16))-1;
    Real dataMin, dataMax;
    std::vector<Real> data(N), weights(N);

    for (i=0; i<LENGTH(averages); i++) {
        for (j=0; j<LENGTH(sigmas); j++) {

            NormalDistribution normal(averages[i],sigmas[j]);
            CumulativeNormalDistribution cumulative(averages[i],sigmas[j]);
            InverseCumulativeNormal inverseCum(averages[i],sigmas[j]);

            SobolRsg rng(1);
            dataMin = QL_MAX_REAL;
            dataMax = QL_MIN_REAL;
            for (k=0; k<N; k++) {
                data[k] = inverseCum(rng.nextSequence().value[0]);
                dataMin = std::min(dataMin, data[k]);
                dataMax = std::max(dataMax, data[k]);
                weights[k]=1.0;
            }

            igs.addSequence(data.begin(),data.end(),weights.begin());
            s.addSequence(data.begin(),data.end(),weights.begin());

            // checks
            Real calculated, expected;
            Real tolerance;

            if (igs.samples() != N)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong number of samples\n"
                           << "    calculated: " << igs.samples() << "\n"
                           << "    expected:   " << N);
            if (s.samples() != N)
                BOOST_FAIL("RiskStatistics: wrong number of samples\n"
                           << "    calculated: " << s.samples() << "\n"
                           << "    expected:   " << N);


            // weightSum()
            tolerance = 1e-10;
            expected = std::accumulate(weights.begin(),weights.end(),Real(0.0));
            calculated = igs.weightSum();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong sum of weights\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.weightSum();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong sum of weights\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // min
            tolerance = 1e-12;
            expected = dataMin;
            calculated = igs.min();
            if (std::fabs(calculated-expected)>tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong minimum value\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.min();
            if (std::fabs(calculated-expected)>tolerance)
                BOOST_FAIL("RiskStatistics: "
                           << "wrong minimum value\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // max
            expected = dataMax;
            calculated = igs.max();
            if (std::fabs(calculated-expected)>tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong maximum value\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.max();
            if (std::fabs(calculated-expected)>tolerance)
                BOOST_FAIL("RiskStatistics: "
                           << "wrong maximum value\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // mean
            expected = averages[i];
            tolerance = (expected == 0.0 ? 1.0e-13 :
                                           std::fabs(expected)*1.0e-13);
            calculated = igs.mean();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong mean value"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.mean();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong mean value"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // variance
            expected = sigmas[j]*sigmas[j];
            tolerance = expected*1.0e-1;
            calculated = igs.variance();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong variance"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.variance();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong variance"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // standardDeviation
            expected = sigmas[j];
            tolerance = expected*1.0e-1;
            calculated = igs.standardDeviation();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong standard deviation"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.standardDeviation();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong standard deviation"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // missing errorEstimate() test

            // skewness
            expected = 0.0;
            tolerance = 1.0e-4;
            calculated = igs.skewness();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong skewness"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.skewness();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong skewness"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // kurtosis
            expected = 0.0;
            tolerance = 1.0e-1;
            calculated = igs.kurtosis();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong kurtosis"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.kurtosis();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong kurtosis"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // percentile
            expected = averages[i];
            tolerance = (expected == 0.0 ? 1.0e-3 :
                                           std::fabs(expected*1.0e-3));
            calculated = igs.gaussianPercentile(0.5);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong Gaussian percentile"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.gaussianPercentile(0.5);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong Gaussian percentile"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.percentile(0.5);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong percentile"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);



            // potential upside
            Real upper_tail = averages[i]+2.0*sigmas[j],
                 lower_tail = averages[i]-2.0*sigmas[j];
            Real twoSigma = cumulative(upper_tail);

            expected = std::max<Real>(upper_tail,0.0);
            tolerance = (expected == 0.0 ? 1.0e-3 :
                                           std::fabs(expected*1.0e-3));
            calculated = igs.gaussianPotentialUpside(twoSigma);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong Gaussian potential upside"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.gaussianPotentialUpside(twoSigma);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong Gaussian potential upside"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.potentialUpside(twoSigma);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong potential upside"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // just to check that GaussianStatistics<StatsHolder> does work
            StatsHolder h(s.mean(), s.standardDeviation());
            GenericGaussianStatistics<StatsHolder> test(h);
            expected = s.gaussianPotentialUpside(twoSigma);
            calculated = test.gaussianPotentialUpside(twoSigma);
            if (!close(calculated,expected))
                BOOST_FAIL("GenericGaussianStatistics<StatsHolder> fails"
                           << std::setprecision(16)
                           << "\n  calculated: " << calculated
                           << "\n  expected: " << expected);

            // value-at-risk
            expected = -std::min<Real>(lower_tail,0.0);
            tolerance = (expected == 0.0 ? 1.0e-3 :
                                           std::fabs(expected*1.0e-3));
            calculated = igs.gaussianValueAtRisk(twoSigma);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong Gaussian value-at-risk"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.gaussianValueAtRisk(twoSigma);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong Gaussian value-at-risk"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.valueAtRisk(twoSigma);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong value-at-risk"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);

            if (averages[i] > 0.0 && sigmas[j] < averages[i]) {
                // no data will miss the targets:
                // skip the rest of this iteration
                igs.reset();
                s.reset();
                continue;
            }


            // expected shortfall
            expected = -std::min<Real>(averages[i]
                                       - sigmas[j]*sigmas[j]
                                       * normal(lower_tail)/(1.0-twoSigma),
                                       0.0);
            tolerance = (expected == 0.0 ? 1.0e-4
                                         : std::fabs(expected)*1.0e-2);
            calculated = igs.gaussianExpectedShortfall(twoSigma);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong Gaussian expected shortfall"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.gaussianExpectedShortfall(twoSigma);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong Gaussian expected shortfall"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.expectedShortfall(twoSigma);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong expected shortfall"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // shortfall
            expected = 0.5;
            tolerance = (expected == 0.0 ? 1.0e-3 :
                                           std::fabs(expected*1.0e-3));
            calculated = igs.gaussianShortfall(averages[i]);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong Gaussian shortfall"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.gaussianShortfall(averages[i]);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong Gaussian shortfall"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.shortfall(averages[i]);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong shortfall"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // average shortfall
            expected = sigmas[j]/std::sqrt(2.0*M_PI)*2.0;
            tolerance = expected*1.0e-3;
            calculated = igs.gaussianAverageShortfall(averages[i]);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong Gaussian average shortfall"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.gaussianAverageShortfall(averages[i]);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong Gaussian average shortfall"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.averageShortfall(averages[i]);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: wrong average shortfall"
                           << " for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // regret
            expected = sigmas[j]*sigmas[j];
            tolerance = expected*1.0e-1;
            calculated = igs.gaussianRegret(averages[i]);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong Gaussian regret(" << averages[i] << ") "
                           << "for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.gaussianRegret(averages[i]);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: "
                           << "wrong Gaussian regret(" << averages[i] << ") "
                           << "for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = s.regret(averages[i]);
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("RiskStatistics: "
                           << "wrong regret(" << averages[i] << ") "
                           << "for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);


            // downsideVariance
            expected = s.downsideVariance();
            tolerance = (expected == 0.0 ? 1.0e-3 :
                                           std::fabs(expected*1.0e-3));
            calculated = igs.downsideVariance();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong downside variance"
                           << "for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);
            calculated = igs.gaussianDownsideVariance();
            if (std::fabs(calculated-expected) > tolerance)
                BOOST_FAIL("IncrementalGaussianStatistics: "
                           << "wrong Gaussian downside variance"
                           << "for N(" << averages[i] << ", "
                           << sigmas[j] << ")\n"
                           << std::setprecision(16)
                           << "    calculated: " << calculated << "\n"
                           << "    expected:   " << expected << "\n"
                           << "    tolerance:  " << tolerance);

            // downsideVariance
            if (averages[i]==0.0) {
                expected = sigmas[j]*sigmas[j];
                tolerance = expected*1.0e-3;
                calculated = igs.downsideVariance();
                if (std::fabs(calculated-expected) > tolerance)
                    BOOST_FAIL("IncrementalGaussianStatistics: "
                               << "wrong downside variance"
                               << "for N(" << averages[i] << ", "
                               << sigmas[j] << ")\n"
                               << std::setprecision(16)
                               << "    calculated: " << calculated << "\n"
                               << "    expected:   " << expected << "\n"
                               << "    tolerance:  " << tolerance);
                calculated = igs.gaussianDownsideVariance();
                if (std::fabs(calculated-expected) > tolerance)
                    BOOST_FAIL("IncrementalGaussianStatistics: "
                               << "wrong Gaussian downside variance"
                               << "for N(" << averages[i] << ", "
                               << sigmas[j] << ")\n"
                               << std::setprecision(16)
                               << "    calculated: " << calculated << "\n"
                               << "    expected:   " << expected << "\n"
                               << "    tolerance:  " << tolerance);
                calculated = s.downsideVariance();
                if (std::fabs(calculated-expected) > tolerance)
                    BOOST_FAIL("RiskStatistics: wrong downside variance"
                               << "for N(" << averages[i] << ", "
                               << sigmas[j] << ")\n"
                               << std::setprecision(16)
                               << "    calculated: " << calculated << "\n"
                               << "    expected:   " << expected << "\n"
                               << "    tolerance:  " << tolerance);
                calculated = s.gaussianDownsideVariance();
                if (std::fabs(calculated-expected) > tolerance)
                    BOOST_FAIL("RiskStatistics: wrong Gaussian downside variance"
                               << "for N(" << averages[i] << ", "
                               << sigmas[j] << ")\n"
                               << std::setprecision(16)
                               << "    calculated: " << calculated << "\n"
                               << "    expected:   " << expected << "\n"
                               << "    tolerance:  " << tolerance);
            }

            igs.reset();
            s.reset();

        }
    }
}


test_suite* RiskStatisticsTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Risk statistics tests");
    suite->add(QUANTLIB_TEST_CASE(&RiskStatisticsTest::testResults));
    return suite;
}

