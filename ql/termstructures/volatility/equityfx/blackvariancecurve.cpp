/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003 StatPro Italia srl

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

#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancetimeextrapolation.hpp>
#include <utility>

namespace QuantLib {

    BlackVarianceCurve::BlackVarianceCurve(const Date& referenceDate,
                                           const std::vector<Date>& dates,
                                           const std::vector<Volatility>& blackVolCurve,
                                           DayCounter dayCounter,
                                           bool forceMonotoneVariance,
                                           BlackVolTimeExtrapolation timeExtrapolation)
    : BlackVarianceTermStructure(referenceDate), dayCounter_(std::move(dayCounter)),
      maxDate_(dates.back()), timeExtrapolation_(timeExtrapolation) {

        QL_REQUIRE(dates.size()==blackVolCurve.size(),
                   "mismatch between date vector and black vol vector");

        // cannot have dates[0]==referenceDate, since the
        // value of the vol at dates[0] would be lost
        // (variance at referenceDate must be zero)
        QL_REQUIRE(dates[0]>referenceDate,
                   "cannot have dates[0] <= referenceDate");

        variances_ = std::vector<Real>(dates.size()+1);
        times_ = std::vector<Time>(dates.size()+1);
        variances_[0] = 0.0;
        times_[0] = 0.0;
        Size j;
        for (j=1; j<=blackVolCurve.size(); j++) {
            times_[j] = timeFromReference(dates[j-1]);
            QL_REQUIRE(times_[j]>times_[j-1],
                       "dates must be sorted unique!");
            variances_[j] = times_[j] *
                blackVolCurve[j-1]*blackVolCurve[j-1];
            QL_REQUIRE(variances_[j]>=variances_[j-1]
                       || !forceMonotoneVariance,
                       "variance must be non-decreasing");
        }

        // default: linear interpolation
        setInterpolation<Linear>();
    }

    Real BlackVarianceCurve::blackVarianceImpl(Time t, Real) const {
        if (t <= times_.back() || timeExtrapolation_ == BlackVolTimeExtrapolation::UseInterpolatorVariance) {
            return std::max(varianceCurve_(t, true), 0.0);
        } else if (timeExtrapolation_ == BlackVolTimeExtrapolation::FlatVolatility) {
            // extrapolate with flat vol
            return timeExtrapolationBlackVarianceFlat(t, times_, varianceCurve_);
        } else if (timeExtrapolation_ == BlackVolTimeExtrapolation::UseInterpolatorVolatility) {
            return timeExtrapolationBlackVarianceInVolatility(t, times_, varianceCurve_);
        } else {
            QL_FAIL("Unknown time extrapolation method");
        }
    }

}

