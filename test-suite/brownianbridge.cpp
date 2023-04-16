/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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

#include "brownianbridge.hpp"
#include "utilities.hpp"
#include <ql/methods/montecarlo/brownianbridge.hpp>
#include <ql/methods/montecarlo/pathgenerator.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <ql/math/randomnumbers/inversecumulativersg.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    template <class ForwardIterator1, class ForwardIterator2>
    Real maxDiff(ForwardIterator1 begin1, ForwardIterator1 end1,
                 ForwardIterator2 begin2) {
        Real diff = 0.0;
        while (begin1 != end1) {
            diff = std::max(diff, std::fabs(*begin1 - *begin2));
            ++begin1; ++begin2;
        }
        return diff;
    }

    template <class ForwardIterator1, class ForwardIterator2>
    Real maxRelDiff(ForwardIterator1 begin1, ForwardIterator1 end1,
                    ForwardIterator2 begin2) {
        Real diff = 0.0;
        while (begin1 != end1) {
            diff = std::max(diff, std::fabs((*begin1 - *begin2)/(*begin2)));
            ++begin1; ++begin2;
        }
        return diff;
    }

}


void BrownianBridgeTest::testVariates() {
    BOOST_TEST_MESSAGE("Testing Brownian-bridge variates...");

    std::vector<Time> times = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 2.0, 5.0};

    Size N = times.size();

    Size samples = 262143;
    unsigned long seed = 42;
    SobolRsg sobol(N, seed);
    InverseCumulativeRsg<SobolRsg,InverseCumulativeNormal> generator(sobol);

    BrownianBridge bridge(times);

    SequenceStatistics stats1(N);
    SequenceStatistics stats2(N);

    std::vector<Real> temp(N);

    for (Size i=0; i<samples; ++i) {
        const std::vector<Real>& sample = generator.nextSequence().value;

        bridge.transform(sample.begin(), sample.end(), temp.begin());
        stats1.add(temp.begin(), temp.end());

        temp[0] = temp[0]*std::sqrt(times[0]);
        for (Size j=1; j<N; ++j)
            temp[j] = temp[j-1] + temp[j]*std::sqrt(times[j]-times[j-1]);
        stats2.add(temp.begin(), temp.end());
    }

    // normalized single variates
    std::vector<Real> expectedMean(N, 0.0);
    Matrix expectedCovariance(N, N, 0.0);
    for (Size i=0; i<N; i++)
        expectedCovariance[i][i] = 1.0;

#ifndef __FAST_MATH__
    Real meanTolerance = 1.0e-16;
#else
    Real meanTolerance = 1.0e-14;
#endif
    Real covTolerance = 2.5e-4;

    std::vector<Real> mean = stats1.mean();
    Matrix covariance = stats1.covariance();

    Real maxMeanError = maxDiff(mean.begin(), mean.end(),
                                expectedMean.begin());
    Real maxCovError = maxDiff(covariance.begin(), covariance.end(),
                               expectedCovariance.begin());

    if (maxMeanError > meanTolerance) {
        Array calculated(N), expected(N);
        std::copy(mean.begin(), mean.end(), calculated.begin());
        std::copy(expectedMean.begin(), expectedMean.end(), expected.begin());
        BOOST_ERROR("failed to reproduce expected mean values"
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << "\n    max error:  " << maxMeanError);
    }

    if (maxCovError > covTolerance) {
        BOOST_ERROR("failed to reproduce expected covariance\n"
                    << "    calculated:\n" << covariance
                    << "    expected:\n" << expectedCovariance
                    << "    max error:  " << maxCovError);
    }

    // denormalized sums along the path
    expectedMean = std::vector<Real>(N, 0.0);
    expectedCovariance = Matrix(N, N);
    for (Size i=0; i<N; ++i)
        for (Size j=i; j<N; ++j)
            expectedCovariance[i][j] = expectedCovariance[j][i] = times[i];

    covTolerance = 6.0e-4;

    mean = stats2.mean();
    covariance = stats2.covariance();

    maxMeanError = maxDiff(mean.begin(), mean.end(),
                           expectedMean.begin());
    maxCovError = maxDiff(covariance.begin(), covariance.end(),
                          expectedCovariance.begin());

    if (maxMeanError > meanTolerance) {
        Array calculated(N), expected(N);
        std::copy(mean.begin(), mean.end(), calculated.begin());
        std::copy(expectedMean.begin(), expectedMean.end(), expected.begin());
        BOOST_ERROR("failed to reproduce expected mean values"
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << "\n    max error:  " << maxMeanError);
    }

    if (maxCovError > covTolerance) {
        BOOST_ERROR("failed to reproduce expected covariance\n"
                    << "    calculated:\n" << covariance
                    << "    expected:\n" << expectedCovariance
                    << "    max error:  " << maxCovError);
    }
}


void BrownianBridgeTest::testPathGeneration() {
    BOOST_TEST_MESSAGE("Testing Brownian-bridge path generation...");

    std::vector<Time> times = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 2.0, 5.0, 7.0, 9.0, 10.0};

    TimeGrid grid(times.begin(), times.end());

    Size N = times.size();

    Size samples = 131071;
    unsigned long seed = 42;
    SobolRsg sobol(N, seed);
    InverseCumulativeRsg<SobolRsg,InverseCumulativeNormal> gsg(sobol);

    Date today = Settings::instance().evaluationDate();
    Handle<Quote> x0(std::shared_ptr<Quote>(new SimpleQuote(100.0)));
    Handle<YieldTermStructure> r(std::shared_ptr<YieldTermStructure>(
                               new FlatForward(today,0.06,Actual365Fixed())));
    Handle<YieldTermStructure> q(std::shared_ptr<YieldTermStructure>(
                               new FlatForward(today,0.03,Actual365Fixed())));
    Handle<BlackVolTermStructure> sigma(
                   std::shared_ptr<BlackVolTermStructure>(
                          new BlackConstantVol(today, NullCalendar(), 0.20,Actual365Fixed())));

    std::shared_ptr<StochasticProcess1D> process(
                              new BlackScholesMertonProcess(x0, q, r, sigma));


    PathGenerator<InverseCumulativeRsg<SobolRsg,InverseCumulativeNormal> >
    generator1(process, grid, gsg, false);
    PathGenerator<InverseCumulativeRsg<SobolRsg,InverseCumulativeNormal> >
    generator2(process, grid, gsg, true);

    SequenceStatistics stats1(N);
    SequenceStatistics stats2(N);

    std::vector<Real> temp(N);

    for (Size i=0; i<samples; ++i) {
        const Path& path1 = generator1.next().value;
        std::copy(path1.begin()+1, path1.end(), temp.begin());
        stats1.add(temp.begin(), temp.end());

        const Path& path2 = generator2.next().value;
        std::copy(path2.begin()+1, path2.end(), temp.begin());
        stats2.add(temp.begin(), temp.end());
    }

    std::vector<Real> expectedMean = stats1.mean();
    Matrix expectedCovariance = stats1.covariance();

    std::vector<Real> mean = stats2.mean();
    Matrix covariance = stats2.covariance();

    Real meanTolerance = 3.0e-5;
    Real covTolerance = 3.0e-3;

    Real maxMeanError = maxRelDiff(mean.begin(), mean.end(),
                                   expectedMean.begin());
    Real maxCovError = maxRelDiff(covariance.begin(), covariance.end(),
                                  expectedCovariance.begin());

    if (maxMeanError > meanTolerance) {
        Array calculated(N), expected(N);
        std::copy(mean.begin(), mean.end(), calculated.begin());
        std::copy(expectedMean.begin(), expectedMean.end(), expected.begin());
        BOOST_ERROR("failed to reproduce expected mean values"
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << "\n    max error:  " << maxMeanError);
    }

    if (maxCovError > covTolerance) {
        BOOST_ERROR("failed to reproduce expected covariance\n"
                    << "    calculated:\n" << covariance
                    << "    expected:\n" << expectedCovariance
                    << "    max error:  " << maxCovError);
    }
}

test_suite* BrownianBridgeTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Brownian bridge tests");
    suite->add(QUANTLIB_TEST_CASE(&BrownianBridgeTest::testVariates));
    suite->add(QUANTLIB_TEST_CASE(&BrownianBridgeTest::testPathGeneration));
    return suite;
}

