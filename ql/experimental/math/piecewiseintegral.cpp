/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/experimental/math/piecewiseintegral.hpp>
#include <algorithm>
#include <utility>

namespace QuantLib {

    PiecewiseIntegral::PiecewiseIntegral(std::shared_ptr<Integrator> integrator,
                                         std::vector<Real> criticalPoints,
                                         const bool avoidCriticalPoints)
    : Integrator(1.0, 1), integrator_(std::move(integrator)),
      criticalPoints_(std::move(criticalPoints)),
      eps_(avoidCriticalPoints ? (1.0 + QL_EPSILON) : 1.0) {

        std::sort(criticalPoints_.begin(), criticalPoints_.end());
        std::vector<Real>::const_iterator end =
            std::unique(criticalPoints_.begin(), criticalPoints_.end(),
                        static_cast<bool (*)(Real, Real)>(close_enough));
        criticalPoints_.resize(end - criticalPoints_.begin());
    }

} // namespace QuantLib
