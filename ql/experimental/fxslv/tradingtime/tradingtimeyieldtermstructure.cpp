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

#include <ql/experimental/fxslv/tradingtime/tradingtimedaycounter.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimeyieldtermstructure.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    TradingTimeYieldTermStructure::TradingTimeYieldTermStructure(
        Handle<YieldTermStructure> curve, Handle<TradingTimeTermStructure> tradingTime)
    : YieldTermStructure(TradingTimeDayCounter(tradingTime)), curve_(std::move(curve)),
      tradingTime_(std::move(tradingTime)) {
        QL_REQUIRE(!curve_.empty(), "no yield curve given");
        registerWith(curve_);
        registerWith(tradingTime_);
        checkTradingTime();
    }

    void TradingTimeYieldTermStructure::update() {
        // the event weights or the reference date may have changed
        checked_ = false;
        YieldTermStructure::update();
    }

    void TradingTimeYieldTermStructure::checkTradingTime() const {
        if (checked_)
            return;
        QL_REQUIRE(tradingTime_->referenceDate() == curve_->referenceDate(),
                   "trading-time reference date (" << tradingTime_->referenceDate()
                       << ") differs from the curve's (" << curve_->referenceDate() << ")");
        checked_ = true;
    }

    DiscountFactor TradingTimeYieldTermStructure::discountImpl(Time tau) const {
        checkTradingTime();
        // calendar days from the reference date, then the curve's own time
        // for them, linear within the day so that dates are exact
        const Real days = tradingTime_->calendarTime(tau) * 365.0;
        const Real whole = std::floor(days);
        const Date d0 = referenceDate() + static_cast<Date::serial_type>(whole);
        const Time t0 = curve_->timeFromReference(d0);
        const Time t = days > whole ? t0 + (days - whole) * (curve_->timeFromReference(d0 + 1) - t0)
                                    : t0;
        return curve_->discount(t, true);
    }

}
