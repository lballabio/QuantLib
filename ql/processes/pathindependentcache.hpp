/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Peter Caspers

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

/*! \file stochasticprocess.hpp
    \brief stochastic processes
*/

#ifndef quantlib_pathindependent_cache_hpp
#define quantlib_pathindependent_cache_hpp

#include <ql/time/date.hpp>

namespace QuantLib {

    template <typename U>
    class PathIndependentCache {
      public:
        PathIndependentCache() = default;
        explicit PathIndependentCache(const Size timeSteps) : timeSteps_(timeSteps) {}
        bool ready() const { return ready_; }
        template <typename T>
        void add(T&& v, const Real t, const Real dt = 0.0) const {
            if (timeSteps_ > 0) {
                if (!times_.empty() && t == times_.back()) {
                    QL_REQUIRE(dt == dts_.back(), "PathIndependentCache::add("
                                                      << t << "," << dt
                                                      << "): time step is repeatedly added, but "
                                                         "with different dt, last dt was "
                                                      << dts_.back() << ". This is not supported.");
                } else {
                    data_.push_back(std::forward<T>(v));
                    times_.push_back(t);
                    dts_.push_back(dt);
                }
                if (data_.size() == timeSteps_)
                    ready_ = true;
            }
        }
        const U& get(const Real t, const Real dt = 0.0) const {
            QL_REQUIRE(ready_, "PathIndependentCache::get(): cache not ready.");
            // usually one or zero increments, so no need to search smarter
            while (t != times_[timeStep_] && timeStep_ < times_.size()) {
                ++timeStep_;
            }
            if (timeStep_ == timeSteps_)
                timeStep_ = 0;
            QL_REQUIRE(t == times_[timeStep_] && dt == dts_[timeStep_],
                       "PathIndependentCache::get("
                           << t << "," << dt << ") at time step " << timeStep_
                           << " does not match cached (t,dt) = (" << times_[timeStep_] << ","
                           << dts_[timeStep_] << ")");
            return data_[timeStep_];
        }

      private:
        Size timeSteps_ = 0;
        mutable Size timeStep_ = 0;
        mutable std::vector<U> data_;
        mutable std::vector<Real> times_;
        mutable std::vector<Real> dts_;
        mutable bool ready_ = false;
    };

}

#endif
