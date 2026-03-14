/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2005 Gary Kennedy
 Copyright (C) 2015 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/math/statistics/statistics.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/math/statistics/gaussianstatistics.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/math/statistics/convergencestatistics.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/randomnumbers/inversecumulativerng.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/comparison.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(StatisticsTests)

Real data[] =    { 3.0, 4.0, 5.0, 2.0, 3.0, 4.0, 5.0, 6.0, 4.0, 7.0 };
Real weights[] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

template <class S>
void check(const std::string& name) {

    S s;
    for (Size i=0; i<std::size(data); i++)
        s.add(data[i],weights[i]);

    Real calculated, expected;
    Real tolerance;

    if (s.samples() != std::size(data))
        BOOST_FAIL(name << ": wrong number of samples\n"
                   << "    calculated: " << s.samples() << "\n"
                   << "    expected:   " << std::size(data));

    expected = std::accumulate(weights,weights+std::size(weights),Real(0.0));
    calculated = s.weightSum();
    if (calculated != expected)
        BOOST_FAIL(name << ": wrong sum of weights\n"
                   << "    calculated: " << calculated << "\n"
                   << "    expected:   " << expected);

    expected = *std::min_element(data,data+std::size(data));
    calculated = s.min();
    if (calculated != expected)
        BOOST_FAIL(name << ": wrong minimum value\n"
                   << "    calculated: " << calculated << "\n"
                   << "    expected:   " << expected);

    expected = *std::max_element(data,data+std::size(data));
    calculated = s.max();
    if (calculated != expected)
        BOOST_FAIL(name << ": wrong maximum value\n"
                   << "    calculated: " << calculated << "\n"
                   << "    expected:   " << expected);

    expected = 4.3;
    tolerance = 1.0e-9;
    calculated = s.mean();
    if (std::fabs(calculated-expected) > tolerance)
        BOOST_FAIL(name << ": wrong mean value\n"
                   << "    calculated: " << calculated << "\n"
                   << "    expected:   " << expected);

    expected = 2.23333333333;
    calculated = s.variance();
    if (std::fabs(calculated-expected) > tolerance)
        BOOST_FAIL(name << ": wrong variance\n"
                   << "    calculated: " << calculated << "\n"
                   << "    expected:   " << expected);

    expected = 1.4944341181;
    calculated = s.standardDeviation();
    if (std::fabs(calculated-expected) > tolerance)
        BOOST_FAIL(name << ": wrong standard deviation\n"
                   << "    calculated: " << calculated << "\n"
                   << "    expected:   " << expected);

    expected = 0.359543071407;
    calculated = s.skewness();
    if (std::fabs(calculated-expected) > tolerance)
        BOOST_FAIL(name << ": wrong skewness\n"
                   << "    calculated: " << calculated << "\n"
                   << "    expected:   " << expected);

    expected = -0.151799637209;
    calculated = s.kurtosis();
    if (std::fabs(calculated-expected) > tolerance)
        BOOST_FAIL(name << ": wrong kurtosis\n"
                   << "    calculated: " << calculated << "\n"
                   << "    expected:   " << expected);
}

template <class S>
void checkSequence(const std::string& name, Size dimension) {

    GenericSequenceStatistics<S> ss(dimension);
    Size i;
    for (i = 0; i<std::size(data); i++) {
        std::vector<Real> temp(dimension, data[i]);
        ss.add(temp, weights[i]);
    }

    std::vector<Real> calculated;
    Real expected, tolerance;

    if (ss.samples() != std::size(data))
        BOOST_FAIL("SequenceStatistics<" << name << ">: "
                   << "wrong number of samples\n"
                   << "    calculated: " << ss.samples() << "\n"
                   << "    expected:   " << std::size(data));

    expected = std::accumulate(weights,weights+std::size(weights),Real(0.0));
    if (ss.weightSum() != expected)
        BOOST_FAIL("SequenceStatistics<" << name << ">: "
                   << "wrong sum of weights\n"
                   << "    calculated: " << ss.weightSum() << "\n"
                   << "    expected:   " << expected);

    expected = *std::min_element(data,data+std::size(data));
    calculated = ss.min();
    for (i=0; i<dimension; i++) {
        if (calculated[i] != expected)
            BOOST_FAIL("SequenceStatistics<" << name << ">: "
                       << io::ordinal(i+1) << " dimension: "
                       << "wrong minimum value\n"
                       << "    calculated: " << calculated[i] << "\n"
                       << "    expected:   " << expected);
    }

    expected = *std::max_element(data,data+std::size(data));
    calculated = ss.max();
    for (i=0; i<dimension; i++) {
        if (calculated[i] != expected)
            BOOST_FAIL("SequenceStatistics<" << name << ">: "
                       << io::ordinal(i+1) << " dimension: "
                       << "wrong maximun value\n"
                       << "    calculated: " << calculated[i] << "\n"
                       << "    expected:   " << expected);
    }

    expected = 4.3;
    tolerance = 1.0e-9;
    calculated = ss.mean();
    for (i=0; i<dimension; i++) {
        if (std::fabs(calculated[i]-expected) > tolerance)
            BOOST_FAIL("SequenceStatistics<" << name << ">: "
                       << io::ordinal(i+1) << " dimension: "
                       << "wrong mean value\n"
                       << "    calculated: " << calculated[i] << "\n"
                       << "    expected:   " << expected);
    }

    expected = 2.23333333333;
    calculated = ss.variance();
    for (i=0; i<dimension; i++) {
        if (std::fabs(calculated[i]-expected) > tolerance)
            BOOST_FAIL("SequenceStatistics<" << name << ">: "
                       << io::ordinal(i+1) << " dimension: "
                       << "wrong variance\n"
                       << "    calculated: " << calculated[i] << "\n"
                       << "    expected:   " << expected);
    }

    expected = 1.4944341181;
    calculated = ss.standardDeviation();
    for (i=0; i<dimension; i++) {
        if (std::fabs(calculated[i]-expected) > tolerance)
            BOOST_FAIL("SequenceStatistics<" << name << ">: "
                       << io::ordinal(i+1) << " dimension: "
                       << "wrong standard deviation\n"
                       << "    calculated: " << calculated[i] << "\n"
                       << "    expected:   " << expected);
    }

    expected = 0.359543071407;
    calculated = ss.skewness();
    for (i=0; i<dimension; i++) {
        if (std::fabs(calculated[i]-expected) > tolerance)
            BOOST_FAIL("SequenceStatistics<" << name << ">: "
                       << io::ordinal(i+1) << " dimension: "
                       << "wrong skewness\n"
                       << "    calculated: " << calculated[i] << "\n"
                       << "    expected:   " << expected);
    }

    expected = -0.151799637209;
    calculated = ss.kurtosis();
    for (i=0; i<dimension; i++) {
        if (std::fabs(calculated[i]-expected) > tolerance)
            BOOST_FAIL("SequenceStatistics<" << name << ">: "
                       << io::ordinal(i+1) << " dimension: "
                       << "wrong kurtosis\n"
                       << "    calculated: " << calculated[i] << "\n"
                       << "    expected:   " << expected);
    }
}

template <class S>
void checkConvergence(const std::string& name) {

    ConvergenceStatistics<S> stats;

    stats.add(1.0);
    stats.add(2.0);
    stats.add(3.0);
    stats.add(4.0);
    stats.add(5.0);
    stats.add(6.0);
    stats.add(7.0);
    stats.add(8.0);

    const Size expectedSize1 = 3;
    Size calculatedSize = stats.convergenceTable().size();
    if (calculatedSize != expectedSize1)
        BOOST_FAIL("ConvergenceStatistics<" << name << ">: "
                   << "\nwrong convergence-table size"
                   << "\n    calculated: " << calculatedSize
                   << "\n    expected:   " << expectedSize1);

    const Real expectedValue1 = 4.0;
    const Real tolerance = 1.0e-9;
    Real calculatedValue = stats.convergenceTable().back().second;
    if (std::fabs(calculatedValue-expectedValue1) > tolerance)
        BOOST_FAIL("wrong last value in convergence table"
                   << "\n    calculated: " << calculatedValue
                   << "\n    expected:   " << expectedValue1);

    const Size expectedSampleSize1 = 7;
    Size calculatedSamples = stats.convergenceTable().back().first;
    if (calculatedSamples != expectedSampleSize1)
        BOOST_FAIL("wrong number of samples in convergence table"
                   << "\n    calculated: " << calculatedSamples
                   << "\n    expected:   " << expectedSampleSize1);

    stats.reset();
    stats.add(1.0);
    stats.add(2.0);
    stats.add(3.0);
    stats.add(4.0);

    const Size expectedSize2 = 2;
    calculatedSize = stats.convergenceTable().size();
    if (calculatedSize != expectedSize2)
        BOOST_FAIL("wrong convergence-table size"
                   << "\n    calculated: " << calculatedSize
                   << "\n    expected:   " << expectedSize2);

    const Real expectedValue2 = 2.0;
    calculatedValue = stats.convergenceTable().back().second;
    if (std::fabs(calculatedValue-expectedValue2) > tolerance)
        BOOST_FAIL("wrong last value in convergence table"
                   << "\n    calculated: " << calculatedValue
                   << "\n    expected:   " << expectedValue2);

    const Size expectedSampleSize2 = 3;
    calculatedSamples = stats.convergenceTable().back().first;
    if (calculatedSamples != expectedSampleSize2)
        BOOST_FAIL("wrong number of samples in convergence table"
                   << "\n    calculated: " << calculatedSamples
                   << "\n    expected:   " << expectedSampleSize2);
}


BOOST_AUTO_TEST_CASE(testStatistics) {

    BOOST_TEST_MESSAGE("Testing statistics...");

    check<IncrementalStatistics>(
        std::string("IncrementalStatistics"));
    check<Statistics>(std::string("Statistics"));
}

BOOST_AUTO_TEST_CASE(testSequenceStatistics) {

    BOOST_TEST_MESSAGE("Testing sequence statistics...");

    checkSequence<IncrementalStatistics>(
        std::string("IncrementalStatistics"),5);
    checkSequence<Statistics>(std::string("Statistics"),5);
}

BOOST_AUTO_TEST_CASE(testConvergenceStatistics) {

    BOOST_TEST_MESSAGE("Testing convergence statistics...");

    checkConvergence<IncrementalStatistics>(
                              std::string("IncrementalStatistics"));
    checkConvergence<Statistics>(std::string("Statistics"));
}

#define TEST_INC_STAT(expr, expected)                                          \
    if (!close_enough(expr, expected))                                         \
        BOOST_ERROR(std::setprecision(16)                                      \
                    << std::scientific << #expr << " (" << expr                \
                    << ") can not be reproduced against cached result ("       \
                    << expected << ")");

BOOST_AUTO_TEST_CASE(testIncrementalStatistics) {

    BOOST_TEST_MESSAGE("Testing incremental statistics...");

    // With QuantLib 1.7 IncrementalStatistics was changed to
    // a wrapper to the boost accumulator library. This is
    // a test of the new implementation against results from
    // the old one.

    MersenneTwisterUniformRng mt(42);

    IncrementalStatistics stat;

    for (Size i = 0; i < 500000; ++i) {
        Real x = 2.0 * (mt.nextReal() - 0.5) * 1234.0;
        Real w = mt.nextReal();
        stat.add(x, w);
    }

    if (stat.samples() != 500000)
        BOOST_ERROR("stat.samples()  (" << stat.samples()
                    << ") can not be reproduced against cached result ("
                    << 500000 << ")");
    TEST_INC_STAT(stat.weightSum(), 2.5003623600676749e+05);
    TEST_INC_STAT(stat.mean(), 4.9122325964293845e-01);
    TEST_INC_STAT(stat.variance(),  5.0706503959683329e+05);
    TEST_INC_STAT(stat.standardDeviation(),  7.1208499464378076e+02);
    TEST_INC_STAT(stat.errorEstimate(), 1.0070402569876076e+00);
    TEST_INC_STAT(stat.skewness(), -1.7360169326720038e-03);
    TEST_INC_STAT(stat.kurtosis(), -1.1990742562085395e+00);
    TEST_INC_STAT(stat.min(), -1.2339945045639761e+03);
    TEST_INC_STAT(stat.max(),  1.2339958308008499e+03);
    TEST_INC_STAT(stat.downsideVariance(), 5.0786776146975247e+05);
    TEST_INC_STAT(stat.downsideDeviation(),  7.1264841364431061e+02);

    // This is a test for numerical stability,
    // where the old implementation fails

    InverseCumulativeRng<MersenneTwisterUniformRng,InverseCumulativeNormal> normal_gen(mt);

    IncrementalStatistics stat2;

    for (Size i = 0; i < 500000; ++i) {
        Real x = normal_gen.next().value * 1E-1 + 1E8;
        Real w = 1.0;
        stat2.add(x, w);
    }

    Real tol = 1E-5;

    if(std::fabs( stat2.variance() - 1E-2 ) > tol)
        BOOST_ERROR("variance (" << stat2.variance()
                                 << ") out of expected range " << 1E-2 << " +- "
                                 << tol);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
