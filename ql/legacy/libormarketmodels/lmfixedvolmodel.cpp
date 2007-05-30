/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/legacy/libormarketmodels/lmfixedvolmodel.hpp>

namespace QuantLib {

    LmFixedVolatilityModel::LmFixedVolatilityModel(
        const Array& volatilities,
        const std::vector<Time>& startTimes)
    : LmVolatilityModel(startTimes.size(), 0),
      volatilities_    (volatilities),
      startTimes_      (startTimes) {
        QL_REQUIRE(startTimes_.size()>1, "too few dates");
        QL_REQUIRE(volatilities_.size() == startTimes_.size(),
                   "volatility array and fixing time array have to have "
                   "the same size");
        for (Size i = 1; i < startTimes_.size(); i++) {
            QL_REQUIRE(startTimes_[i] > startTimes_[i-1],
                       "invalid time (" << startTimes_[i] << ", vs "
                       << startTimes_[i-1] << ")");
        }
    }

    Disposable<Array> LmFixedVolatilityModel::volatility(
                                                 Time t, const Array&) const {
        QL_REQUIRE(t >= startTimes_.front() && t <= startTimes_.back(),
                   "invalid time given for volatility model");

        const Size ti = std::upper_bound(startTimes_.begin(),
                                         startTimes_.end()-1, t)
                      - startTimes_.begin()-1;

        Array tmp(size_, 0.0);

        for (Size i=ti; i<size_; ++i) {
            tmp[i] = volatilities_[i-ti];
        }

        return tmp;
    }

    Volatility LmFixedVolatilityModel::volatility(
                                         Size i, Time t, const Array&) const {
        QL_REQUIRE(t >= startTimes_.front() && t <= startTimes_.back(),
                   "invalid time given for volatility model");

        const Size ti = std::upper_bound(startTimes_.begin(),
                                         startTimes_.end()-1, t)
                      - startTimes_.begin()-1;

        return volatilities_[i-ti];
    }

    void LmFixedVolatilityModel::generateArguments() {
        return;
    }

}

