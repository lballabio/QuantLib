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

#include <ql/experimental/fxslv/fxcalendartimevolsurface.hpp>
#include <ql/experimental/fxslv/fxvariancesurface.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        Handle<TradingTimeTermStructure>
        clockOf(const ext::shared_ptr<BlackVolTermStructure>& surface) {
            QL_REQUIRE(surface, "no variance surface given");
            const auto* fx = dynamic_cast<const FxVolSurface*>(surface.get());
            QL_REQUIRE(fx, "the variance surface is not an FX variance surface");
            return fx->tradingTimeStructure();
        }

    }

    FxCalendarTimeVolSurface::FxCalendarTimeVolSurface(
        ext::shared_ptr<BlackVolTermStructure> surface)
    : BlackVarianceTermStructure(surface ? surface->businessDayConvention() : Following,
                                 Actual365Fixed()),
      surface_(std::move(surface)), clock_(clockOf(surface_)) {
        registerWith(surface_);
        enableExtrapolation(surface_->allowsExtrapolation());
    }

    Real FxCalendarTimeVolSurface::blackVarianceImpl(Time t, Real strike) const {
        // the trading time of the instant, on the surface's own axis
        return surface_->blackVariance(clock_->tradingTime(t), strike, true);
    }

}
