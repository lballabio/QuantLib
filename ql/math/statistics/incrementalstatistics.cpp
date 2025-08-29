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
#include <iomanip>

namespace QuantLib {

    IncrementalStatistics::IncrementalStatistics() {
        reset();
    }

    Size IncrementalStatistics::samples() const {
        return boost::accumulators::extract_result<
            boost::accumulators::tag::count>(acc_);
    }

    Real IncrementalStatistics::weightSum() const {
        return boost::accumulators::extract_result<
            boost::accumulators::tag::sum_of_weights>(acc_);
    }

    Real IncrementalStatistics::mean() const {
        QL_REQUIRE(weightSum() > 0.0, "sampleWeight_= 0, unsufficient");
        return boost::accumulators::extract_result<
            boost::accumulators::tag::weighted_mean>(acc_);
    }

    Real IncrementalStatistics::variance() const {
        QL_REQUIRE(weightSum() > 0.0, "sampleWeight_= 0, unsufficient");
        QL_REQUIRE(samples() > 1, "sample number <= 1, unsufficient");
        Real n = static_cast<Real>(samples());
        return n / (n - 1.0) *
               boost::accumulators::extract_result<
                   boost::accumulators::tag::weighted_variance>(acc_);
    }

    Real IncrementalStatistics::standardDeviation() const {
        return std::sqrt(variance());
    }

    Real IncrementalStatistics::errorEstimate() const {
        return std::sqrt(variance() / (samples()));
    }

    Real IncrementalStatistics::skewness() const {
        QL_REQUIRE(samples() > 2, "sample number <= 2, unsufficient");
        Real n = static_cast<Real>(samples());
        Real r1 = n / (n - 2.0);
        Real r2 = (n - 1.0) / (n - 2.0);
        return std::sqrt(r1 * r2) * 
               boost::accumulators::extract_result<
                   boost::accumulators::tag::weighted_skewness>(acc_);
    }

    Real IncrementalStatistics::kurtosis() const {
        QL_REQUIRE(samples() > 3,
                   "sample number <= 3, unsufficient");
        boost::accumulators::extract_result<
            boost::accumulators::tag::weighted_kurtosis>(acc_);
        Real n = static_cast<Real>(samples());
        Real r1 = (n - 1.0) / (n - 2.0);
        Real r2 = (n + 1.0) / (n - 3.0);
        Real r3 = (n - 1.0) / (n - 3.0);
        return ((3.0 + boost::accumulators::extract_result<
                           boost::accumulators::tag::weighted_kurtosis>(acc_)) *
                    r2 -
                3.0 * r3) *
               r1;
    }

    Real IncrementalStatistics::min() const {
        QL_REQUIRE(samples() > 0, "empty sample set");
        return boost::accumulators::extract_result<
            boost::accumulators::tag::min>(acc_);
    }

    Real IncrementalStatistics::max() const {
        QL_REQUIRE(samples() > 0, "empty sample set");
        return boost::accumulators::extract_result<
            boost::accumulators::tag::max>(acc_);
    }

    Size IncrementalStatistics::downsideSamples() const {
        return boost::accumulators::extract_result<
            boost::accumulators::tag::count>(downsideAcc_);
    }

    Real IncrementalStatistics::downsideWeightSum() const {
        return boost::accumulators::extract_result<
            boost::accumulators::tag::sum_of_weights>(downsideAcc_);
    }

    Real IncrementalStatistics::downsideVariance() const {
        QL_REQUIRE(downsideWeightSum() > 0.0, "sampleWeight_= 0, unsufficient");
        QL_REQUIRE(downsideSamples() > 1, "sample number <= 1, unsufficient");
        Real n = static_cast<Real>(downsideSamples());
        Real r1 = n / (n - 1.0);
        return r1 *
               boost::accumulators::extract_result<
                   boost::accumulators::tag::moment<2> >(downsideAcc_);
    }

    Real IncrementalStatistics::downsideDeviation() const {
        return std::sqrt(downsideVariance());
    }

    void IncrementalStatistics::add(Real value, Real valueWeight) {
        QL_REQUIRE(valueWeight >= 0.0, "negative weight (" << valueWeight
                                                           << ") not allowed");
        acc_(value, boost::accumulators::weight = valueWeight);
        if(value < 0.0)
            downsideAcc_(value, boost::accumulators::weight = valueWeight);
    }

    void IncrementalStatistics::reset() {
        acc_ = accumulator_set();
        downsideAcc_ = downside_accumulator_set();
    }

}
