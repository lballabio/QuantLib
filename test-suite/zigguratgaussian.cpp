/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Ralf Konrad Eckel

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
#include <ql/math/randomnumbers/xoshiro256starstaruniformrng.hpp>
#include <ql/math/randomnumbers/zigguratgaussianrng.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

using namespace QuantLib;

namespace {

    /*! Deterministic stand-in for the uniform source.

        ZigguratGaussianRng is a template over its RNG and documents the
        interface it needs (nextReal / nextInt64), so a scripted source lets a
        test drive one exact draw instead of relying on aggregate statistics.
    */
    class ScriptedRng {
      public:
        ScriptedRng(std::vector<std::uint64_t> ints, std::vector<Real> reals)
        : ints_(std::move(ints)), reals_(std::move(reals)) {}

        std::uint64_t nextInt64() const {
            BOOST_REQUIRE_MESSAGE(intPos_ < ints_.size(), "scripted nextInt64() exhausted");
            return ints_[intPos_++];
        }

        Real nextReal() const {
            BOOST_REQUIRE_MESSAGE(realPos_ < reals_.size(), "scripted nextReal() exhausted");
            return reals_[realPos_++];
        }

      private:
        std::vector<std::uint64_t> ints_;
        std::vector<Real> reals_;
        mutable std::size_t intPos_ = 0;
        mutable std::size_t realPos_ = 0;
    };

    /*! nextReal() splits a single 64-bit draw into the ordinate and the layer,

            u = 2*((r >> 11) + 0.5)/2^53 - 1,      i = r & 0xff

        so inverting it lets a test ask for a specific (layer, u) pair.

        The preconditions are load-bearing rather than decorative: a layer of
        256 or more would alias through `r & 0xff` (256 becomes layer 0, the
        tail case), and u = 1 would round the bucket up to 2^53, whose shift
        overflows to zero and decodes back as u = -1. Either would silently
        encode a different draw than the one asked for.
    */
    std::uint64_t drawFor(int layer, Real u) {
        QL_REQUIRE(layer >= 0 && layer <= 0xFF, "layer " << layer << " outside [0, 255]");
        QL_REQUIRE(u >= -1.0 && u < 1.0, "u " << u << " outside [-1, 1)");

        const Real twoPow53 = std::ldexp(1.0, 53);
        const auto bucket =
            static_cast<std::uint64_t>(std::llround((u + 1.0) / 2.0 * twoPow53 - 0.5));
        QL_REQUIRE(bucket < (1ULL << 53), "bucket " << bucket << " overflows for u = " << u);

        return (bucket << 11) | static_cast<std::uint64_t>(layer);
    }

}

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(ZigguratGaussianTests)

BOOST_AUTO_TEST_CASE(testStatisticsOfNextReal) {
    BOOST_TEST_MESSAGE("Testing ZigguratGaussianRng<Xoshiro256StarStarUniformRng>::nextReal() for "
                       "mean, variance, skewness and kurtosis...");
    auto seed = 42UL;
    auto uniformRandom = Xoshiro256StarStarUniformRng(seed);
    auto random = ZigguratGaussianRng<Xoshiro256StarStarUniformRng>(uniformRandom);

    auto randoms = IncrementalStatistics();

    auto iterations = 10'000'000;
    for (auto j = 0; j < iterations; ++j) {
        Real next = random.next().value;
        randoms.add(next);
    }

    auto mean = randoms.mean();
    auto variance = randoms.variance();
    auto skewness = randoms.skewness();
    auto kurtosis = randoms.kurtosis();

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

BOOST_AUTO_TEST_CASE(testWedgeDrawIsAccepted) {
    BOOST_TEST_MESSAGE("Testing ZigguratGaussianRng<ScriptedRng>::nextReal() accepts a draw "
                       "that falls in a wedge...");

    // Layer 1 runs from normX(1) = 3.654152885361008796 down to
    // normX(2) = 3.449278298560964462. A draw whose |x| lands between the two
    // misses the rectangle fast path and must go through the wedge acceptance
    // test, which is the only way such a value can be produced from layer 1.
    //
    // Take x = 3.5, comfortably inside that band, and feed the wedge test
    // u = 0.5. Marsaglia-Tsang acceptance is
    //
    //     normF(i+1) + (normF(i) - normF(i+1)) * u  <  pdf(x)
    //
    // and with normF(1) = 0.001260285930498598,
    //          normF(2) = 0.002609072746106363,
    //          pdf(3.5) = 0.002187491118182889:
    //
    //     lhs = 0.001934679338302481  <  pdf(x)     -> accept, margin 2.5e-04
    //
    // so the generator must return the wedge draw itself.
    const int wedgeLayer = 1;
    const Real normX1 = 3.654152885361008796;
    const Real wedgeX = 3.5;

    // The generator consumes this second draw only if it rejected the first:
    // layer 200 with |x| = 0.1035, well inside normX(201), so the rectangle
    // fast path returns it immediately. It sits 3.4 away from the wedge value,
    // which makes the two outcomes unambiguous.
    const int fastPathLayer = 200;
    const Real fastPathU = 0.1;

    auto scripted =
        ScriptedRng({drawFor(wedgeLayer, wedgeX / normX1), drawFor(fastPathLayer, fastPathU)},
                    {0.5});
    auto random = ZigguratGaussianRng<ScriptedRng>(scripted);

    const Real sample = random.next().value;

    if (std::abs(sample - wedgeX) > 1.0e-12) {
        BOOST_ERROR("ZigguratGaussianRng rejected a wedge draw that the Marsaglia-Tsang "
                    "acceptance test accepts.\n"
                    << "    expected " << wedgeX << " (the wedge draw)\n"
                    << "    returned " << sample << " (the following fast-path draw)");
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
