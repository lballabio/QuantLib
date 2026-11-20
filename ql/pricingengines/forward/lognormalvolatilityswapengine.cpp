/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yang Ye

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

#include <ql/pricingengines/forward/lognormalvolatilityswapengine.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    LognormalVolatilitySwapEngine::LognormalVolatilitySwapEngine(
        Handle<YieldTermStructure> discountCurve,
        Handle<Quote> fairVarianceStrike,
        Handle<Quote> varianceLogVolatility)
    : discountCurve_(std::move(discountCurve)),
      fairVarianceStrike_(std::move(fairVarianceStrike)),
      varianceLogVolatility_(std::move(varianceLogVolatility)) {
        registerWith(discountCurve_);
        registerWith(fairVarianceStrike_);
        registerWith(varianceLogVolatility_);
    }

    void LognormalVolatilitySwapEngine::calculate() const {
        QL_REQUIRE(arguments_.startDate >= discountCurve_->referenceDate(),
                   "seasoned volatility swaps are not supported: start date ("
                   << arguments_.startDate << ") precedes the reference date ("
                   << discountCurve_->referenceDate() << ")");

        Real variance = fairVarianceStrike_->value();
        QL_REQUIRE(variance > 0.0, "negative or null fair variance strike (" << variance << ")");
        Real logVol = varianceLogVolatility_->value();
        QL_REQUIRE(logVol >= 0.0, "negative variance log-volatility (" << logVol << ")");

        Real fairVolatility = std::sqrt(variance) * std::exp(-logVol * logVol / 8.0);
        results_.fairVolatility = fairVolatility;

        Real sign = (arguments_.position == Position::Long ? 1.0 : -1.0);
        results_.value = sign * arguments_.notional *
                         discountCurve_->discount(arguments_.maturityDate) *
                         (fairVolatility - arguments_.strike);
    }

}
