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

#include <ql/errors.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimedaycounter.hpp>
#include <utility>

namespace QuantLib {

    TradingTimeDayCounter::Impl::Impl(Handle<TradingTimeTermStructure> tradingTime)
    : tradingTime_(std::move(tradingTime)) {
        QL_REQUIRE(!tradingTime_.empty(), "no trading time given");
    }

    Time TradingTimeDayCounter::Impl::yearFraction(const Date& d1,
                                                   const Date& d2,
                                                   const Date&,
                                                   const Date&) const {
        return tradingTime_->tradingTime(d1, d2);
    }

}
