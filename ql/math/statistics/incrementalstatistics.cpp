/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
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

#include <ql/math/statistics/incrementalstatistics.hpp>
#include <cmath>
#include <limits>

namespace QuantLib {

    IncrementalStatistics::IncrementalStatistics() {
        reset();
    }

    void IncrementalStatistics::reset() {
        n_           = 0;
        wSum_        = 0.0;
        sum_         = 0.0;
        weightedVar_ = 0.0;
        sum2_        = 0.0;
        sum3_        = 0.0;
        sum4_        = 0.0;
        min_         =  std::numeric_limits<Real>::max();
        max_         = -std::numeric_limits<Real>::max();

        nd_          = 0;
        wSumd_       = 0.0;
        sum2d_       = 0.0;
    }

    void IncrementalStatistics::add(Real value, Real weight) {
        QL_REQUIRE(weight >= 0.0,
                   "negative weight (" << weight << ") not allowed");

        ++n_;

        if (value < min_) min_ = value;
        if (value > max_) max_ = value;

        if (weight == 0.0)
            return;

        if (value < 0.0) {
            ++nd_;
            wSumd_ += weight;
            sum2d_ += weight * value * value;
        }

        Real wPrev = wSum_;
        wSum_     += weight;
        sum_      += weight * value;

        if (wSum_ == 0.0)
            return;

        Real mean = sum_ / wSum_;

        if (n_ == 1) {
            weightedVar_ = 0.0;
        } else if (wPrev > 0.0) {
            Real tmp = value - mean;
            weightedVar_ = (weightedVar_ * wPrev) / wSum_ + (tmp * tmp * weight) / wPrev;
        }

        Real v2 = value * value;
        Real v3 = v2 * value;
        Real v4 = v3 * value;
        sum2_ += weight * v2;
        sum3_ += weight * v3;
        sum4_ += weight * v4;
    }

    Size IncrementalStatistics::samples() const {
        return n_;
    }

    Real IncrementalStatistics::weightSum() const {
        return wSum_;
    }

    Real IncrementalStatistics::mean() const {
        QL_REQUIRE(wSum_ > 0.0, "sampleWeight_= 0, unsufficient");
        return sum_ / wSum_;
    }

    Real IncrementalStatistics::variance() const {
        QL_REQUIRE(wSum_ > 0.0, "sampleWeight_= 0, unsufficient");
        QL_REQUIRE(n_ > 1, "sample number <= 1, unsufficient");
        Real n = static_cast<Real>(n_);
        return (n / (n - 1.0)) * weightedVar_;
    }

    Real IncrementalStatistics::standardDeviation() const {
        return std::sqrt(variance());
    }

    Real IncrementalStatistics::errorEstimate() const {
        return std::sqrt(variance() / static_cast<Real>(n_));
    }

    Real IncrementalStatistics::skewness() const {
        QL_REQUIRE(n_ > 2, "sample number <= 2, unsufficient");
        Real m = mean();
        Real mom2 = sum2_ / wSum_;
        Real mom3 = sum3_ / wSum_;
        Real num = mom3 - 3.0 * mom2 * m + 2.0 * m * m * m;
        Real denVar = mom2 - m * m;
        if (denVar <= 0.0)
            return 0.0;
        Real den = denVar * std::sqrt(denVar);
        Real n = static_cast<Real>(n_);
        Real r1 = n / (n - 2.0);
        Real r2 = (n - 1.0) / (n - 2.0);
        return std::sqrt(r1 * r2) * (num / den);
    }

    Real IncrementalStatistics::kurtosis() const {
        QL_REQUIRE(n_ > 3, "sample number <= 3, unsufficient");
        Real m = mean();
        Real mom2 = sum2_ / wSum_;
        Real mom3 = sum3_ / wSum_;
        Real mom4 = sum4_ / wSum_;
        Real num = mom4 - 4.0 * mom3 * m + 6.0 * mom2 * m * m - 3.0 * m * m * m * m;
        Real denVar = mom2 - m * m;
        if (denVar <= 0.0)
            return 0.0;
        Real den = denVar * denVar;
        Real rawKurt = (num / den) - 3.0;

        Real n = static_cast<Real>(n_);
        Real r1 = (n - 1.0) / (n - 2.0);
        Real r2 = (n + 1.0) / (n - 3.0);
        Real r3 = (n - 1.0) / (n - 3.0);
        return ((3.0 + rawKurt) * r2 - 3.0 * r3) * r1;
    }

    Real IncrementalStatistics::min() const {
        QL_REQUIRE(n_ > 0, "empty sample set");
        return min_;
    }

    Real IncrementalStatistics::max() const {
        QL_REQUIRE(n_ > 0, "empty sample set");
        return max_;
    }

    Size IncrementalStatistics::downsideSamples() const {
        return nd_;
    }

    Real IncrementalStatistics::downsideWeightSum() const {
        return wSumd_;
    }

    Real IncrementalStatistics::downsideVariance() const {
        QL_REQUIRE(wSumd_ > 0.0, "sampleWeight_= 0, unsufficient");
        QL_REQUIRE(nd_ > 1, "sample number <= 1, unsufficient");
        Real nd = static_cast<Real>(nd_);
        return (nd / (nd - 1.0)) * (sum2d_ / wSumd_);
    }

    Real IncrementalStatistics::downsideDeviation() const {
        return std::sqrt(downsideVariance());
    }

}
