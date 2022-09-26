/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Chester Wong
 Copyright (C) 2022 Wojciech Czernous

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

#include "fractionalnoisersg.hpp"
#include "utilities.hpp"
#include <ql/math/matrixutilities/getcovariance.hpp>
#include <ql/math/randomnumbers/fractionalnoisersg.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void FractionalGaussianNoiseTest::testVariates() {
    BOOST_TEST_MESSAGE("Testing fractional Gaussian noise variates...");

    Real HurstParameter = 0.7;
    Size dimension = 1 << 8;
    Size samples = dimension + 3;
    BigNatural seed = 42;
    Real threshold = .005;

    typedef PseudoRandom::rsg_type rsg_type;
    typedef FractionalGaussianNoiseRsg<rsg_type>::sample_type sample_type;
    rsg_type rsg = PseudoRandom::make_sequence_generator(dimension, seed);
    FractionalGaussianNoiseRsg<rsg_type> generator(HurstParameter, rsg);

    Matrix expCov(dimension, dimension);
    for (Size i = 0; i < dimension; ++i)
    {
        expCov[i][i] = generator.gamma(0);
        for (Size j = 0; j < i; ++j)
            expCov[i][j] = expCov[j][i] = generator.gamma(i - j);
    }

    SequenceStatistics stats(dimension);
    for (Size i = 0; i < samples; ++i) {
        const std::vector<Real>& sample = generator.nextSequence().value;
        stats.add(sample.begin(), sample.end());
    }
    
    Real LRT = stats.likelihoodratiotest(Array(dimension, 0.0), expCov);
    CumulativeNormalDistribution cdf; // of N(0,1)

    Real statQuantile = cdf(LRT);

    if (statQuantile < threshold || 1 - threshold < statQuantile) {
        BOOST_ERROR("Rejected hypothesis: fractional Gaussian noise has zero mean"
                    " and given covariance matrix; LRT statistic quantile = "
                    << statQuantile); 
    }
}

test_suite* FractionalGaussianNoiseTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Fractional Gaussian noise generator test");
    suite->add(QUANTLIB_TEST_CASE(&FractionalGaussianNoiseTest::testVariates));
    return suite;
}

