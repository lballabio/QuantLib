
/*
 Copyright (C) 2003 RiskMap.

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/TermStructures/zerocurve.hpp>

namespace QuantLib {

    ZeroCurve::ZeroCurve(const Date &todaysDate,
                         const std::vector < Date > &dates,
                         const std::vector < Rate > &yields,
                         const DayCounter & dayCounter)
    : todaysDate_(todaysDate), dates_(dates), yields_(yields),
      dayCounter_(dayCounter) {

        QL_REQUIRE(dates_.size()>1, "too few dates");
        QL_REQUIRE(yields_.size()==dates_.size(),
                   "dates/yields mismatch");

        times_.resize(dates_.size());
        times_[0]=0.0;
        for(Size i = 1; i < dates_.size(); i++) {
            QL_REQUIRE(dates_[i]>dates_[i-1], "invalid date");
            #if !defined(QL_NEGATIVE_RATES)
            QL_REQUIRE(yields_[i] >= 0.0, "invalid yield");
            #endif
            times_[i] = dayCounter_.yearFraction(dates_[0],
                                                 dates_[i]);
        }

        interpolation_ = LinearInterpolation(times_.begin(), times_.end(),
                                             yields_.begin());
    }

    Rate ZeroCurve::zeroYieldImpl(Time t, bool extrapolate) const {
        QL_REQUIRE(t >= 0.0,
                   "negative time (" + DoubleFormatter::toString(t) +
                   ") not allowed");
        return interpolation_(t, extrapolate);
    }

}
