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

    FxCalendarTimeVolSurface::FxCalendarTimeVolSurface(ext::shared_ptr<FxVarianceSurface> surface)
    : BlackVarianceTermStructure(surface ? surface->businessDayConvention() : Following,
                                 Actual365Fixed()),
      surface_(std::move(surface)) {
        QL_REQUIRE(surface_, "no variance surface given");
        registerWith(surface_);
        enableExtrapolation(surface_->allowsExtrapolation());
    }

    const Date& FxCalendarTimeVolSurface::referenceDate() const {
        return surface_->referenceDate();
    }
    Calendar FxCalendarTimeVolSurface::calendar() const { return surface_->calendar(); }
    Natural FxCalendarTimeVolSurface::settlementDays() const { return surface_->settlementDays(); }
    Date FxCalendarTimeVolSurface::maxDate() const { return surface_->maxDate(); }
    Real FxCalendarTimeVolSurface::minStrike() const { return surface_->minStrike(); }
    Real FxCalendarTimeVolSurface::maxStrike() const { return surface_->maxStrike(); }

    Real FxCalendarTimeVolSurface::blackVarianceImpl(Time t, Real strike) const {
        // the trading time of the instant, on the surface's own axis
        return surface_->blackVariance(surface_->tradingTimeStructure()->tradingTime(t), strike,
                                       true);
    }

}
