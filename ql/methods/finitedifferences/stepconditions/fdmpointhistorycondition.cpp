/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Jogi Sidhu

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

#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmpointhistorycondition.hpp>
#include <algorithm>
#include <iterator>
#include <utility>

namespace QuantLib {

    FdmPointHistoryCondition::FdmPointHistoryCondition(
        const ext::shared_ptr<FdmMesher>& mesher, Real x, std::vector<Time> solutionTimes)
    : x_(x), solutionTimes_(std::move(solutionTimes)) {
        QL_REQUIRE(mesher->layout()->dim().size() == 1,
                   "one-dimensional mesher required");

        const Array& locations = mesher->locations(0);
        locations_.assign(locations.begin(), locations.end());
        QL_REQUIRE(x_ >= locations_.front() && x_ <= locations_.back(),
                   "point " << x_ << " outside the mesher range ["
                   << locations_.front() << ", " << locations_.back() << "]");
    }

    void FdmPointHistoryCondition::applyTo(Array& a, Time t) const {
        QL_REQUIRE(a.size() == locations_.size(), "inconsistent array dimensions");

        // as in Fdm1DimSolver
        const MonotonicCubicNaturalSpline interpolation(
            locations_.begin(), locations_.end(), a.begin());
        values_[t] = interpolation(x_);

        if (std::find(solutionTimes_.begin(), solutionTimes_.end(), t) != solutionTimes_.end())
            solutions_[t] = a;
    }

    Real FdmPointHistoryCondition::valueAt(Time t) const {
        QL_REQUIRE(!values_.empty(), "no values recorded");

        const auto upper = values_.lower_bound(t);
        if (upper == values_.end())
            return std::prev(upper)->second;
        if (upper == values_.begin() || upper->first == t)
            return upper->second;

        const auto lower = std::prev(upper);
        const Real w = (t - lower->first) / (upper->first - lower->first);
        return (1.0 - w) * lower->second + w * upper->second;
    }

    const Array& FdmPointHistoryCondition::solutionAt(Time t) const {
        const auto solution = solutions_.find(t);
        QL_REQUIRE(solution != solutions_.end(), "no solution recorded at time " << t);
        return solution->second;
    }

}
