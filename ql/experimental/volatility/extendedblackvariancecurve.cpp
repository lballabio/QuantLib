/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Frank Hövermann

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

#include <ql/experimental/volatility/extendedblackvariancecurve.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <utility>

namespace QuantLib {

    ExtendedBlackVarianceCurve::ExtendedBlackVarianceCurve(const Date& referenceDate,
                                                           const std::vector<Date>& dates,
                                                           std::vector<Handle<Quote> > volatilities,
                                                           DayCounter dayCounter,
                                                           bool forceMonotoneVariance)
    : BlackVarianceTermStructure(referenceDate), dayCounter_(std::move(dayCounter)),
      maxDate_(dates.back()), volatilities_(std::move(volatilities)),
      forceMonotoneVariance_(forceMonotoneVariance) {
        QL_REQUIRE(dates.size() == volatilities_.size(),
                   "size mismatch between dates and volatilities");

        QL_REQUIRE(dates[0] > referenceDate,
                   "cannot have dates_[0] <= referenceDate");

        variances_ = std::vector<Real>(dates.size()+1);
        times_ = std::vector<Time>(dates.size()+1);

        times_[0] = 0.0;
        for (Size j=1; j<=dates.size(); ++j) {
            times_[j] = timeFromReference(dates[j-1]);
            QL_REQUIRE(times_[j]>times_[j-1],
                       "dates must be sorted unique!");
        }

        setVariances();
        setInterpolation<Linear>();

        for (auto& volatilitie : volatilities_)
            registerWith(volatilitie);
    }

    void ExtendedBlackVarianceCurve::setVariances() {
        variances_[0] = 0.0;
        for (Size j=1; j<=volatilities_.size(); j++) {
            Volatility sigma = volatilities_[j-1]->value();
            variances_[j] = times_[j] * sigma * sigma;
            QL_REQUIRE(variances_[j]>=variances_[j-1]
                       || !forceMonotoneVariance_,
                       "variance must be non-decreasing");
        }
    }

    void ExtendedBlackVarianceCurve::update() {
        setVariances();
        varianceCurve_.update();
        notifyObservers();
    }

    Real ExtendedBlackVarianceCurve::blackVarianceImpl(Time t, Real) const {
        if (t<=times_.back()) {
            return varianceCurve_(t, true);
        } else {
            return varianceCurve_(times_.back(), true)*t/times_.back();
        }
    }

}

