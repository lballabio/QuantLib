/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Gang Liang

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

#include <ql/errors.hpp>
#include <ql/math/statistics/histogram.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/math/comparison.hpp>
#include <algorithm>

namespace QuantLib {

    namespace {

        /* The discontinuous quantiles use the method (type 8) as
           recommended by Hyndman and Fan (1996). The resulting
           quantile estimates are approximately median-unbiased
           regardless of the distribution of 'samples'.

           If quantile function is called multiple times for the same
           dataset, it is recommended to pre-sort the sample vector.
        */
        Real quantile(const std::vector<Real>& samples, Real prob) {
            Size nsample = samples.size();
            QL_REQUIRE(prob >= 0.0 && prob <= 1.0,
                       "Probability has to be in [0,1].");
            QL_REQUIRE(nsample > 0, "The sample size has to be positive." );

            if (nsample == 1)
                return samples[0];

            // two special cases: close to boundaries
            const Real a = 1. / 3, b = 2*a / (nsample+a);
            if (prob < b)
                return *std::min_element(samples.begin(), samples.end());
            else if (prob > 1-b)
                return *std::max_element(samples.begin(), samples.end());

            // general situation: middle region and nsample >= 2
            Size index = static_cast<Size>(std::floor((nsample+a)*prob+a));
            std::vector<Real> sorted(index+1);
            std::partial_sort_copy(samples.begin(), samples.end(),
                                   sorted.begin(), sorted.end());

            // use "index & index+1"th elements to interpolate the quantile
            Real weight = nsample*prob + a - index;
            return (1-weight) * sorted[index-1] + weight * sorted[index];
        }

    }


    Size Histogram::bins() const {
        return bins_;
    }

    const std::vector<Real>& Histogram::breaks() const {
        return breaks_;
    }

    Histogram::Algorithm Histogram::algorithm() const {
        return algorithm_;
    }

    bool Histogram::empty() const {
        return bins_ == 0;
    }

    Size Histogram::counts(Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        return counts_.at(i);
        #else
        return counts_[i];
        #endif
    }

    Real Histogram::frequency(Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        return frequency_.at(i);
        #else
        return frequency_[i];
        #endif
    }

    void Histogram::calculate() {
        QL_REQUIRE(!data_.empty(), "no data given");

        Real min = *std::min_element(data_.begin(), data_.end());
        Real max = *std::max_element(data_.begin(), data_.end());

        // calculate number of bins if necessary
        if (bins_ == Null<Size>()) {
            switch (algorithm_) {
              case Sturges: {
                  bins_ = static_cast<Size>(
                           std::ceil(std::log(static_cast<Real>(data_.size()))
                                     /std::log(2.0) + 1));
                  break;
              }
              case FD: {
                  Real r1 = quantile(data_, 0.25);
                  Real r2 = quantile(data_, 0.75);
                  Real h = 2.0 * (r2-r1) * std::pow(static_cast<Real>(data_.size()), -1.0/3.0);
                  bins_ = static_cast<Size>(std::ceil((max-min)/h));
                  break;
              }
              case Scott: {
                  IncrementalStatistics summary;
                  summary.addSequence(data_.begin(), data_.end());
                  Real variance = summary.variance();
                  Real h = 3.5 * std::sqrt(variance)
                         * std::pow(static_cast<Real>(data_.size()), -1.0/3.0);
                  bins_ = static_cast<Size>(std::ceil((max-min)/h));
                  break;
              }
              case None:
                QL_FAIL("a bin-partition algorithm is required");
              default:
                QL_FAIL("unknown bin-partition algorithm");
            };
            bins_ = std::max<Size>(bins_,1);
        }

        if (breaks_.empty()) {
            // set breaks if not provided
            breaks_.resize(bins_-1);

            // ensure breaks_ evenly span over the range of data_
            // TODO: borrow the idea of pretty in R.
            Real h = (max-min)/bins_;
            for (Size i=0; i<breaks_.size(); ++i) {
                breaks_[i] = min + (i+1)*h;
            }
        } else {
            // or ensure they're sorted if given
            std::sort(breaks_.begin(), breaks_.end());
            auto end = std::unique(breaks_.begin(), breaks_.end(),
                                   static_cast<bool (*)(Real, Real)>(close_enough));
            breaks_.resize(end - breaks_.begin());
        }

        // finally, calculate counts and frequencies
        counts_.resize(bins_);
        std::fill(counts_.begin(), counts_.end(), 0);

        for (Real p : data_) {
            bool processed = false;
            for (Size i=0; i<breaks_.size(); ++i) {
                if (p < breaks_[i]) {
                    ++counts_[i];
                    processed = true;
                    break;
                }
            }
            if (!processed)
                ++counts_[bins_-1];
        }

        frequency_.resize(bins_);

        Size totalCounts = data_.size();
        for (Size i=0; i<bins_; ++i)
            frequency_[i] = static_cast<Real>(counts_[i])/totalCounts;
    }

}
