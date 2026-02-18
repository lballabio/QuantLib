/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026

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

#include <ql/termstructures/volatility/equityfx/blackvariancesurfacefromsmilesections.hpp>
#include <algorithm>
#include <utility>

namespace QuantLib {

    BlackVarianceSurfaceFromSmileSections::BlackVarianceSurfaceFromSmileSections(
            const Date& referenceDate,
            const std::vector<Date>& dates,
            std::vector<ext::shared_ptr<SmileSection>> smileSections,
            DayCounter dayCounter)
    : BlackVarianceTermStructure(referenceDate),
      dayCounter_(std::move(dayCounter)),
      smileSections_(std::move(smileSections)) {

        QL_REQUIRE(!dates.empty(),
                   "at least one date is required");
        QL_REQUIRE(dates.size() == smileSections_.size(),
                   "mismatch between " << dates.size() << " dates and "
                   << smileSections_.size() << " smile sections");

        maxDate_ = dates.back();
        times_.resize(dates.size());

        times_[0] = timeFromReference(dates[0]);
        QL_REQUIRE(times_[0] > 0.0,
                   "first date (" << dates[0]
                   << ") must be after reference date ("
                   << referenceDate << ")");

        for (Size i = 1; i < dates.size(); ++i) {
            times_[i] = timeFromReference(dates[i]);
            QL_REQUIRE(times_[i] > times_[i-1],
                       "dates must be sorted and unique, but date "
                       << dates[i] << " (t=" << times_[i]
                       << ") is not after date " << dates[i-1]
                       << " (t=" << times_[i-1] << ")");
        }

        for (Size i = 0; i < smileSections_.size(); ++i) {
            QL_REQUIRE(smileSections_[i],
                       "null smile section at index " << i);
            registerWith(smileSections_[i]);
        }
    }

    Real BlackVarianceSurfaceFromSmileSections::blackVarianceImpl(
            Time t, Real strike) const {

        if (t == 0.0)
            return 0.0;

        if (t <= times_.front()) {
            // linear interpolation from (0, 0) to first tenor
            Real var1 = smileSections_.front()->variance(strike);
            return var1 * t / times_.front();
        }

        if (t >= times_.back()) {
            // flat vol extrapolation beyond last tenor
            Real varN = smileSections_.back()->variance(strike);
            return varN * t / times_.back();
        }

        // find enclosing interval
        auto it = std::upper_bound(times_.begin(), times_.end(), t);
        Size hi = std::distance(times_.begin(), it);
        Size lo = hi - 1;

        Real varLo = smileSections_[lo]->variance(strike);
        Real varHi = smileSections_[hi]->variance(strike);
        Real alpha = (t - times_[lo]) / (times_[hi] - times_[lo]);

        return varLo + (varHi - varLo) * alpha;
    }

    void BlackVarianceSurfaceFromSmileSections::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<BlackVarianceSurfaceFromSmileSections>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BlackVarianceTermStructure::accept(v);
    }

}
