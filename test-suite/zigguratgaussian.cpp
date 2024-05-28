/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Ralf Konrad Eckel

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

#include "toplevelfixture.hpp"
#include <ql/math/randomnumbers/xoshiro256starstaruniformrng.hpp>
#include <ql/math/randomnumbers/zigguratgaussianrng.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/kurtosis.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/skewness.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <iostream>
#include <numeric>

using namespace QuantLib;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(ZigguratGaussianTests)

BOOST_AUTO_TEST_CASE(testMeanAndStdDevOfNextReal) {
    BOOST_TEST_MESSAGE("Testing ZigguratGaussianRng<Xoshiro256StarStarUniformRng>::nextReal() for "
                       "mean, variance, skewness and kurtosis...");
    using namespace boost::accumulators;

    auto seed = 42UL;
    auto uniformRandom = Xoshiro256StarStarUniformRng(seed);
    auto random = ZigguratGaussianRng<Xoshiro256StarStarUniformRng>(uniformRandom);

    const auto iterations = 10'000'000;

    accumulator_set<Real,
                    features<tag::count, tag::mean, tag::variance, tag::skewness, tag::kurtosis>>
        randoms;

    for (auto j = 0; j < iterations; ++j) {
        Real next = random.next().value;
        randoms(next);
    }

    Real mean = boost::accumulators::mean(randoms);
    Real variance = boost::accumulators::variance(randoms);
    Real skewness = boost::accumulators::skewness(randoms);
    Real kurtosis = boost::accumulators::kurtosis(randoms);

    if (std::abs(mean) > 0.001) {
        BOOST_ERROR("Mean " << mean << " for seed " << seed << " is not close to 0.");
    }
    if (std::abs(1.0 - variance) > 0.005) {
        BOOST_ERROR("Variance " << variance << " for seed " << seed << " is not close to 1.");
    }
    if (std::abs(skewness) > 0.001) {
        BOOST_ERROR("Skewness " << skewness << " for seed " << seed << " is not close to 0.");
    }
    if (std::abs(kurtosis) > 0.03) {
        BOOST_ERROR("Kurtosis " << kurtosis << " for seed " << seed << " is not close to 0.");
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
