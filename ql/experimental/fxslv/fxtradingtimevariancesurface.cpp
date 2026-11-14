/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

#include <ql/experimental/fxslv/fxtradingtimevariancesurface.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimedaycounter.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        ext::shared_ptr<FxTradingTimeVariance>
        tradingTimeVariance(const ext::shared_ptr<BlackVolTermStructure>& surface) {
            QL_REQUIRE(surface, "no variance surface given");
            auto variance = ext::dynamic_pointer_cast<FxTradingTimeVariance>(surface);
            QL_REQUIRE(variance, "the variance surface is not interpolated in trading time");
            return variance;
        }

    }

    FxTradingTimeVarianceSurface::FxTradingTimeVarianceSurface(
        ext::shared_ptr<BlackVolTermStructure> surface)
    : BlackVarianceTermStructure(surface ? surface->businessDayConvention() : Following,
                                 TradingTimeDayCounter(
                                     tradingTimeVariance(surface)->tradingTimeStructure())),
      surface_(std::move(surface)), variance_(tradingTimeVariance(surface_)) {
        registerWith(surface_);
        enableExtrapolation(surface_->allowsExtrapolation());
    }

    Real FxTradingTimeVarianceSurface::blackVarianceImpl(Time tau, Real strike) const {
        return variance_->blackVarianceInTradingTime(tau, strike);
    }

}
